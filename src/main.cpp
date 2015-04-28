#include "Servo.h"
#include "Timer.h"

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

  steer.set(0);
  Timer::sleep(0.5);

  // Set speed close to dead zone (0.135)
  speed.set(0.125);
  rampSpeed(speed, .16);
  Timer::sleep(1.0);

  // Slow down for corner
  rampSpeed(speed, .14);

  // Turn for 2 seconds at slow speed
  steer.set(-20);
  Timer::sleep(2.0);

  // Straighten out and speed up on straight away
  steer.set(0);
  rampSpeed(speed, 0.16);

  // Let it go straight for 1 second, then shut her down
  Timer::sleep(1.0);
  rampSpeed(speed, 0);

  // Get a dump of the total run time as JSON (just for kicks)
  runTime.printJson(cout << "var runTimeTotal = ") << "\n";
}

int main(int argc, const char** argv) {
  signal(SIGINT, interrupted);
  signal(SIGTERM, interrupted);

  // Left tread used PWM on P8_13 (EHRPWM2B) and direction 
  // controls with P8_11 (GPIO_45) and P8_15 (GPIO_47)
  //HBridge leftTread(BlackLib::P8_13, BlackLib::P8_11, BlackLib::P8_15);

  // Left tread used PWM on P9_14 (EHRPWM1A) and direction 
  // controls with P9_12 (GPIO_60) and P9_15 (GPIO_48)
  //HBridge rightTread(BlackLib::P9_14, BlackLib::P9_12, BlackLib::P9_15);
  Servo steer(BlackLib::P8_13, -40.0, 40.0);
  Servo speed(BlackLib::P9_14, -1.0, +1.0);

  // Start button connected to P8 pin 19 (GPIO_22)
  BlackLib::BlackGPIO startButton(BlackLib::GPIO_22, BlackLib::input);
  bool startWasHigh = startButton.isHigh();

  cout << "Entering main loop - waiting for trigger ...\n";

  while (hasBeenInterrupted == false) {
    bool startIsHigh = startButton.isHigh();

    // Run auton when button is pressed and then released
    if ((startWasHigh == true) && (startIsHigh == false)) {
      cout << "Button released, starting auton\n";
      Timer autonTimer;
      autonA(steer, speed);
      cout << "Auton completed in " << autonTimer.secsElapsed() << " seconds\n";
    } else {
      Timer::sleep(0.05);
    }
    
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

