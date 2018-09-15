#include <WalabotAPI.h>
#include <cstdio>

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

int main() {
  if (!setup()) {
    return 1;
  }

  AntennaPair *antennas;
  int antennaNum;

  if (Walabot_GetAntennaPairs(&antennas, &antennaNum) != WALABOT_SUCCESS) {
    wala_screech();
    return 1;
  } else {
    printf("Detected %d antenna pairs\n", antennaNum);
  }

  for (;;) {
    if (Walabot_Trigger() != WALABOT_SUCCESS) {
      wala_screech();

      Walabot_Disconnect();
      return 1;
    }

    for (int i = 0; i < antennaNum; i++) {
      double *signal, *time;
      int nums, tx, rx;

      tx = antennas[i].txAntenna;
      rx = antennas[i].rxAntenna;

      if (Walabot_GetSignal(tx, rx, &signal, &time, &nums) != WALABOT_SUCCESS) {
        wala_screech();

        Walabot_Disconnect();
        return 1;
      }

      for (int j = 0; j < nums; j++) {
        printf("%d\t%d\t%f\t%f\n", tx, rx, signal[j], time[j]);
      }
    }
  }

  return 0;
}
