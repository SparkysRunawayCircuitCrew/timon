/**
 * Simple test of using the GyroBNO055 class. Displays relative angle
 * and lights up LEDs based on rotation from starting point.
 *
 * To compile/run:
 *
 *   make name=gyro && sudo build/gryo
 *
 * Use ^C to terminate.
 */

#include "UserLeds.h"
#include "GyroBNO055.h"
#include "Timer.h"

#include <BlackGPIO.h>

#include <iostream>
#include <cmath>

#include <signal.h>

using namespace avc;
using namespace std;

namespace {
  // Flag will be set when user terminates via ^C or uses kill on process
  bool hasBeenInterrupted = false;

  void interrupted(int sig) {
    hasBeenInterrupted = true;
  }
}

//
// Main entry point into the code basically waits for user
// to press button on BBB then runs the autonomous code
// 
int main(int argc, const char** argv) {
  signal(SIGINT, interrupted);
  signal(SIGTERM, interrupted);
  UserLeds& leds = UserLeds::getInstance();
  int waitCnt = 0;

  // Create and reset the gyro
  GyroBNO055 gyro;
  if (gyro.reset()) {
    cout << "Successfully initialized the Gyro\n";
  } else {
    cout << "Failed to reset Gyro\n";
  }

  while (hasBeenInterrupted == false) {
    Timer::sleep(0.5);
    float heading;
    Timer gyroReadTime;

    if (gyro.getHeading(heading)) {
      gyroReadTime.pause();
      // Convert to signed value
      if (heading > 180.0) {
	heading = heading - 360.0;
      }
      float degOff = abs(heading);
      int ledState = 0xf;

      if (degOff > 30.0) {
	ledState = (heading < 0) ? 0xc : 0x3;
      } else if (degOff > 10.0) {
	ledState = (heading < 0) ? 0x8 : 0x1;
      } else if (degOff > 3.0) {
	ledState = (heading < 0) ? 0x4 : 0x2;
      } else if (degOff > 1) {
	ledState = 0;
      } else if (degOff > 0.25) {
	ledState = 0x6;
      }
      leds.setState(ledState);

      cout << heading 
	   << " degrees off (" << (gyroReadTime.secsElapsed() * 1000.0)
	   << " msecs to read)\n";
    } else {
      cerr << "Problem reading gyro data\n";
      break;
    }
  }

  leds.setState(0);

  return 0;
}
