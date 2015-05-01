#include "Servo.h"
#include "Timer.h"
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

void rampSpeed(Servo& esc, double newSpeed) {
  float curSpeed = esc.get();
  bool speedUp = (newSpeed > curSpeed);
  float step =  speedUp ? 0.005 : -0.005;
  int stepPeriod = 1000000000 / 20; // 20 Hz (50 milliseconds between steps)
  float nextSpeed = curSpeed + step;

  // Used to make nice sleep steps
  Timer start;
  
  while ((newSpeed > nextSpeed) == speedUp) {
    // Pause timer now to see how close we are to time edges in
    // diagnostic output
    start.pause();
    esc.set(nextSpeed);
    cout << "Adjusted speed to: " << nextSpeed << " (timer: "
	 << start.secsElapsed() << ")\n";
    nextSpeed += step;
    start.unpause();
    start.sleepUntilNextNano(stepPeriod);
  }

  if (newSpeed != nextSpeed) {
    esc.set(newSpeed);
    cout << "Reached final speed: " << newSpeed << " (timer: "
	 << start.secsElapsed() << ")\n";
  }
}

/**
 * Sample stupid auton that just drives straight, does a turn, then
 * drives straight and finally stops.
 */
void autonA(Servo& steer, Servo& speed) {
  Timer runTime;
  // Use user LEDs to provide indication how far we make it through the code
  int wayPoint = 1;
  UserLeds& leds = UserLeds::getInstance();

  steer.set(0);
  Timer::sleep(0.5);
  leds.setState(wayPoint++);

  // Set speed close to dead zone (0.135)
  speed.set(0.125);
  rampSpeed(speed, .16);
  Timer::sleep(1.0);
  leds.setState(wayPoint++);

  // Slow down for corner
  rampSpeed(speed, .14);

  // Turn for 2 seconds at slow speed
  steer.set(-20);
  Timer::sleep(2.0);
  leds.setState(wayPoint++);

  // Straighten out and speed up on straight away
  steer.set(0);
  rampSpeed(speed, 0.16);

  // Let it go straight for 1 second, then shut her down
  Timer::sleep(1.0);
  rampSpeed(speed, 0);
  leds.setState(wayPoint++);

  // Get a dump of the total run time as JSON (just for kicks)
  runTime.printJson(cout << "var runTimeTotal = ") << "\n";
}

int main(int argc, const char** argv) {
  signal(SIGINT, interrupted);
  signal(SIGTERM, interrupted);
  UserLeds& leds = UserLeds::getInstance();
  int waitCnt = 0;

  // PWM on P9 at pin 14
  Servo steer(BlackLib::P9_14, -40.0, 40.0);
  // PWM on P9 at pin 16
  Servo speed(BlackLib::P9_16, -1.0, +1.0);

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
      autonA(steer, speed);
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


/*
int main(int argc, const char** argv) {
	BlackLib::BlackPWM pwm(P8_13);

	pwm.setPeriodTime(1 / 60, BlackLib::milisecond);
	pwm.setDutyPercent(4.0);

	printf("%s\n", pwm.getDutyFilePath().c_str());

	pwm.setRunState(BlackLib::run);

	sleep(15);

	pwm.setRunState(stop);

	return 0;
}
*/

