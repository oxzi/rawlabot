#include <WalabotAPI.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef __LINUX__
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

// Setup and starts the Walabot. If something wents wrong, an error will be
// printed and the function returns false.
bool setup() {
  // Connection to an available Walabot
  setup_check(Walabot_ConnectAny());

  // Setting scan profile
  // TODO: make this configurable
  setup_check(Walabot_SetProfile(PROF_SENSOR));

  // Setting arena
  // TODO: make this configurable
  setup_check(Walabot_SetArenaR(10, 60, 1));
  setup_check(Walabot_SetArenaTheta(-15, 15, 5));
  setup_check(Walabot_SetArenaPhi(-60, 60, 5));

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

int main(int argc, char **argv) {
  bool showHelp = true;
  bool printAntennas = false;
  char *antennas = NULL;

  if (argc > 1) {
    showHelp = false;

    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--help") == 0) {
        showHelp = true;
      } else if (strcmp(argv[i], "--print-antennas") == 0) {
        printAntennas = true;
      } else if (strcmp(argv[i], "--antennas") == 0) {
        antennas = argv[++i];
      } else {
        printf("Unknown parameter: %s\n", argv[i]);
        showHelp = true;
        break;
      }
    }
  }

  if (showHelp) {
    printHelp();
    return 0;
  }

  // Start up Walabot
  if (!setup()) {
    return 1;
  }

  if (printAntennas) {
    printAntennaPairs();
  } else {
    int antennaNum;
    AntennaPair *antennaPairs;

    parseAntennas(antennas, &antennaNum, &antennaPairs);

    for (;;) {
      if (Walabot_Trigger() != WALABOT_SUCCESS) {
        wala_screech();

        Walabot_Disconnect();
        return 1;
      }

      for (int i = 0; i < antennaNum; i++) {
        double *signal, *time;
        int nums, tx, rx;

        tx = antennaPairs[i].txAntenna;
        rx = antennaPairs[i].rxAntenna;

        if (Walabot_GetSignal(tx, rx, &signal, &time, &nums) != WALABOT_SUCCESS) {
          wala_screech();

          Walabot_Disconnect();
          return 1;
        }

        for (int j = 0; j < nums; j++) {
          printf("%d-%d,%f\n", tx, rx, signal[j]);
        }
      }
    }
  }

  Walabot_Disconnect();
  return 0;
}
