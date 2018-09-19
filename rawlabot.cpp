#include <WalabotAPI.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#ifndef __LINUX__
// I'm not sure if this even works..
#define strtok_r strtok_s
#endif

// Macro to report an error for a bad WALABOT_RESULT.
#define wala_screech() {                                                       \
  const char* errorStr = Walabot_GetErrorString();                             \
  printf("Error at %s:%d:\n >%s\n", __FILE__, __LINE__, errorStr);             \
}

// Macro to check if a WALABOT_RESULT is successfully or calls wala_screech
// and returns false otherwise. Used in the setup function.
#define setup_check(res) {                                                     \
  if (res != WALABOT_SUCCESS) {                                                \
    wala_screech();                                                            \
    return false;                                                              \
  }                                                                            \
}

// State variable which will be changed by SIGINT (handleSigint)
bool RUNNING = true;

// Setup and starts the Walabot. If something wents wrong, an error will be
// printed and the function returns false.
bool setup(APP_PROFILE profile, int *arenaR, int *arenaTheta, int *arenaPhi) {
  // Connection to an available Walabot
  setup_check(Walabot_ConnectAny());

  // Setting scan profile
  setup_check(Walabot_SetProfile(profile));

  // Setting arena
  if (arenaR != NULL) {
    setup_check(Walabot_SetArenaR(
          arenaR[0], arenaR[1], arenaR[2]));
  } else {
    setup_check(Walabot_SetArenaR(10, 60, 1));
  }

  if (arenaTheta != NULL) {
    setup_check(Walabot_SetArenaTheta(
          arenaTheta[0], arenaTheta[1], arenaTheta[2]));
  } else {
    setup_check(Walabot_SetArenaTheta(-15, 15, 5));
  }

  if (arenaPhi != NULL) {
    setup_check(Walabot_SetArenaPhi(
          arenaPhi[0], arenaPhi[1], arenaPhi[2]));
  } else {
    setup_check(Walabot_SetArenaPhi(-60, 60, 5));
  }

  // Start sensor
  setup_check(Walabot_Start());

  // Start calibration and trigger the sensor until calibration is finished.
  setup_check(Walabot_StartCalibration());

  APP_STATUS status;
  double caliPercentage;

  do {
    setup_check(Walabot_GetStatus(&status, &caliPercentage));
    setup_check(Walabot_Trigger());
  } while (status == STATUS_CALIBRATING);

  return true;
}

// Prints --help message
void printHelp() {
  printf("rawlabot\n\n");
  printf("  --help\n");
  printf("    prints this message\n\n");
  printf("  --print-antennas\n");
  printf("    prints list of antennas (TX/RX)\n\n");
  printf("  --antennas TX1:RX1,TX2:RX2,..\n");
  printf("    dump data from antenna pairs as in --print-antennas\n\n");
  printf("  --profile short-range|sensor|sensor-narrow\n");
  printf("    use one of those three profiles (defaults to sensor)\n\n");
  printf("  --arena-r START,END,RES\n");
  printf("    set arena's radial range (defaults to 10,60,1)\n\n");
  printf("  --arena-theta START,END,RES\n");
  printf("    set arena's polar range (default to -15,15,5)\n\n");
  printf("  --arena-phi START,END,RES\n");
  printf("    set arena's azimuth range (defaults to -60,60,5)\n\n");
  printf("  --runs NUMB\n");
  printf("    query the antenna pairs NUMB times and exit\n\n");
}

// Prints all available antenna pairs (TX/RX).
void printAntennaPairs() {
  AntennaPair *antennas;
  int num;

  if (Walabot_GetAntennaPairs(&antennas, &num) != WALABOT_SUCCESS) {
    wala_screech();
    return;
  }

  printf("List of antenna pairs (TX/RX):\n");
  for (int i = 0; i < num; i++) {
    printf("  %d:%d\n", antennas[i].txAntenna, antennas[i].rxAntenna);
  }
}

// Parses an comma-separated int-string like (3,0,-20).
// [in]  input  : string to be parsed
// [out] triple : array of three ints
void parseTriple(char *input, int **triple) {
  char *tok;

  *triple = new int [3];
  tok = strtok(input, ",");

  for (int i = 0; i < 3; i++) {
    (*triple)[i] = atoi(tok);
    tok = strtok(NULL, ",");
  }
}

// Parse the --antennas parameter to an array of AntennaPair*
//  [in]  antennas : command line parameter
//  [out] num      : amount of AntennaPairs in pairs
//  [out] pairs    : array of pointers to AntennaPair
void parseAntennas(char *antennas, int *num, AntennaPair **pairs) {
  // Count pairs in string
  *num = 1;
  for (int i = 0; antennas[i]; i++) {
    *num += antennas[i] == ',';
  }

  // Create array and tokenize string
  *pairs = new AntennaPair [*num];

  int c = 0;
  char *tokOuter, *tokOuterNext = NULL;

  tokOuter = strtok_r(antennas, ",", &tokOuterNext);
  while (tokOuter != NULL) {
    char *tokInner, *tokInnerNext = NULL;
    int tx, rx;

    tokInner = strtok_r(tokOuter, ":", &tokInnerNext);
    tx = atoi(tokInner);
    tokInner = strtok_r(NULL, ":", &tokInnerNext);
    rx = atoi(tokInner);

    (*pairs)[c++] = AntennaPair{tx, rx};

    tokOuter = strtok_r(NULL, ",", &tokOuterNext);
  }
}

// Handle Ctrl+C (SIGINT) to stop at next iteration.
void handleSigint(int sig) {
  if (sig != SIGINT) {
    printf("¯\\_(ツ)_/¯\tSIGINT handler has wrong another signal: %d\n", sig);
    return;
  }

  printf("\rSIGINT received, will stop soon..\n");
  RUNNING = false;
}

// Appends the Walabot's data for tx/rx antennas to filename.
void writeAntennaData(
    double *signal, int signalNums, long long numb, char *filename) {
  FILE *f = fopen(filename, "a");
  for (int i = 0; i < signalNums; i++) {
    fprintf(f, "%lld,%d,%f\n", numb, i, signal[i]);
  }
  fclose(f);
}

int main(int argc, char **argv) {
  bool showHelp = false;
  bool forceHelp = true;
  bool printAntennas = false;

  int *arenaR = NULL;
  int *arenaTheta = NULL;
  int *arenaPhi = NULL;

  char *antennas = NULL;

  long long *runLimit = NULL;

  APP_PROFILE profile = PROF_SENSOR;

  if (argc > 1) {
    forceHelp = false;

    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--help") == 0) {
        showHelp = true;
      } else if (strcmp(argv[i], "--print-antennas") == 0) {
        printAntennas = true;
      } else if (strcmp(argv[i], "--antennas") == 0) {
        antennas = argv[++i];
      } else if (strcmp(argv[i], "--profile") == 0) {
        i++;

        if (strcmp(argv[i], "short-range") == 0) {
          profile = PROF_SHORT_RANGE_IMAGING;
        } else if (strcmp(argv[i], "sensor") == 0) {
          profile = PROF_SENSOR;
        } else if (strcmp(argv[i], "sensor-narrow") == 0) {
          profile = PROF_SENSOR_NARROW;
        } else {
          printf("Unknown profile: %s\n", argv[i]);
          forceHelp = true;
          break;
        }
      } else if (strcmp(argv[i], "--arena-r") == 0) {
        parseTriple(argv[++i], &arenaR);
      } else if (strcmp(argv[i], "--arena-theta") == 0) {
        parseTriple(argv[++i], &arenaTheta);
      } else if (strcmp(argv[i], "--arena-phi") == 0) {
        parseTriple(argv[++i], &arenaPhi);
      } else if (strcmp(argv[i], "--runs") == 0) {
        runLimit = new long long;
        *runLimit = atoll(argv[++i]);
      } else {
        printf("Unknown parameter: %s\n", argv[i]);
        forceHelp = true;
        break;
      }
    }
  }

  if (showHelp || forceHelp) {
    printHelp();
    return forceHelp ? 1 : 0;
  }

  // Start up Walabot
  if (!setup(profile, arenaR, arenaTheta, arenaPhi)) {
    return 1;
  }

  if (printAntennas) {
    printAntennaPairs();
  } else {
    int antennaNum;
    AntennaPair *antennaPairs;
    char **antennaPairFilenames;

    if (antennas == NULL) {
      printHelp();

      Walabot_Disconnect();
      return 1;
    }

    parseAntennas(antennas, &antennaNum, &antennaPairs);

    antennaPairFilenames = new char* [antennaNum];
    for (int i = 0; i < antennaNum; i++) {
      antennaPairFilenames[i] = new char [10];
      sprintf(antennaPairFilenames[i], "%d-%d.csv",
          antennaPairs[i].txAntenna, antennaPairs[i].rxAntenna);

      FILE *f = fopen(antennaPairFilenames[i], "w");
      fprintf(f, "run,no,signal\n");
      fclose(f);
    }

    printf("rawlabot\nStart recording..\n");

    // Register SIGINT handler to stop after Ctrl+C
    signal(SIGINT, handleSigint);

    long long numb;
    for (numb = 0; RUNNING && (!runLimit || numb < *runLimit); numb++) {
      std::thread *threads = new std::thread [antennaNum];

      if (Walabot_Trigger() != WALABOT_SUCCESS) {
        wala_screech();

        Walabot_Disconnect();
        return 1;
      }

      // Write each CSV in an own thread
      for (int i = 0; i < antennaNum; i++) {
        int signalNums;
        double *signal, *time;
        WALABOT_RESULT res;

        res = Walabot_GetSignal(
            antennaPairs[i].txAntenna, antennaPairs[i].rxAntenna,
            &signal, &time, &signalNums);

        if (res != WALABOT_SUCCESS) {
          wala_screech();

          Walabot_Disconnect();
          return 1;
        }

        threads[i] = std::thread(writeAntennaData,
            signal, signalNums, numb, antennaPairFilenames[i]);
      }

      // Synchronize threads
      for (int i = 0; i < antennaNum; i++) {
        threads[i].join();
      }
    }

    printf("Wrote %lld data sets for each antenna pair\n", numb);
  }

  Walabot_Disconnect();
  return 0;
}
