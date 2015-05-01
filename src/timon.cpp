/**
 * This is the main entry point in the SparkFun Autonomous Vehicle Competition (AVC)
 * for the 2015 robot.
 */

#include "CommandSequence.h"
#include "UserLeds.h"

#include <BlackGPIO.h>

#include <iostream>

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

  // Start button connected to P9 pin 23 (GPIO_49)
  BlackLib::BlackGPIO startButton(BlackLib::GPIO_49, BlackLib::input);
  bool startWasHigh = startButton.isHigh();

  cout << "Entering main loop - waiting for trigger ...\n";

  while (hasBeenInterrupted == false) {

    bool startIsHigh = startButton.isHigh();

    // Run auton when button is pressed and then released
    if ((startWasHigh == true) && (startIsHigh == false)) {
      leds.setState(0xf);
      cout << "Button released, starting auton\n";
	      
      Timer autonTimer;

      cout << "TODO: Need timon implemenation - this is just a place holder\n";

      cout << "Auton completed in " << autonTimer.secsElapsed() << " seconds\n";

    } else {
      Timer::sleep(0.05);
      int ledState = waitCnt & 0xf;
      leds.setState(ledState);
      if (ledState == 0) {
	      waitCnt = 1;
      } else {
	      waitCnt <<= 1;
      }
    }
    leds.setState(0x0);
    // Save prior state
    startWasHigh = startIsHigh;
  }

  cout << "Terminated by external signal\n";

  return 0;
}
