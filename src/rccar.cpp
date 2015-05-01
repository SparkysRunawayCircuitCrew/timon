#include "CommandSequence.h"
#include "Servo.h"
#include "UserLeds.h"

#include <BlackGPIO.h>

#include <cmath>
#include <iostream>

#include <signal.h>

using namespace avc;
using namespace std;

/**
 * Definition of the RC car to control.
 */
class RcCar : public CommandSequence {

private:
  Servo _steer;
  Servo _esc;
  int _wayPoint;

public:
  RcCar() :
    CommandSequence("RcCar"),
    _steer(BlackLib::P9_14, -40.0, 40.0),
    _esc(BlackLib::P9_16, -1.0, +1.0),
    _wayPoint(1)
  {
  }

  ~RcCar() { 
    _steer.set(0);
    _esc.set(0);
    _steer.disable();
    _esc.disable();
  }

  void nextWaypoint() {
    UserLeds& leds = UserLeds::getInstance();
    leds.setState(_wayPoint++);
  }

  void doInitialize() {
    _wayPoint = 1;
    CommandSequence::doInitialize();
  }

  void seekSpeed(float power, float maxStep = 0.05) {
    _esc.seek(power, maxStep);
  }

  void seekSteer(float ang, float maxStep = 2.0) {
    _steer.seek(ang, maxStep);
  }

  void coast() {
    _esc.set(0.0);
  }

  void setSteer(float angle) {
    _steer.set(0.0);
  }

  float getAngle() {
    // TODO - Need gyro reading or some other way to get angle of car
    return 0.0;
  }

  bool detectedCorner() {
    // TODO - Need to figure out this from vision
    return false;
  }

  std::ostream& print(std::ostream& out, const Command& cmd) const {
    out << cmd << "  Car(steer=" << _steer.get() << ", power="
	<< _esc.get() << ")";
    return out;
  }

};

/**
 * Command to drive straight down the road until the turn point is detected.
 */
class DriveToTurn : public Command {

public:
  DriveToTurn(RcCar& car, float power, float timeout = 10.0) :
    Command("DriveToTurn", timeout),
    _car(car),
    _power(power)
  {
  }

  void doInitialize() {
    _car.print(cout, *this) << "\n";
  }

  Command::State doExecute() {
    _car.seekSpeed(_power);
    _car.setSteer(0.0);
    bool foundCorner = _car.detectedCorner();
    // TODO: Remove this time out override once we can detect the corner
    foundCorner = ((getElapsedTime() / getTimeout()) >= 0.5);
    _car.print(cout, *this) << "\n";
    return (foundCorner ? Command::NORMAL_END : Command::STILL_RUNNING);
  }

  void doEnd(Command::State reason) {
    _car.print(cout, *this) << "\n";
    _car.coast();
    _car.nextWaypoint();
  }

private:
  RcCar& _car;
  float _power;
};

/**
 * Command to make a turn.
 */
class MakeTurn : public Command {

public:

  MakeTurn(RcCar& car, float relativeAng = 90.0) :
    Command("MakeTurn", 1.0),
    _car(car),
    _relativeAng(relativeAng),
    _targetAng(0)
  {
  }

  ~MakeTurn() { }

  void doInitialize() {
    _car.print(cout, *this) << "\n";
    _targetAng = _car.getAngle() + _relativeAng;
  }

  Command::State doExecute() {
    _car.seekSpeed(0.15);

    float carAng = _car.getAngle();
    float err = _targetAng - carAng;

    // TODO: Delete once we figure out how to get angle (the
    // statement below makes 
    float percentTimeElapsed = getElapsedTime() / getTimeout();
    err = (percentTimeElapsed >= 0.5) ? 0.0 : (1.1 - percentTimeElapsed) * _relativeAng;

    // Limit to 25 degree steer in either direction
    float steer = min(25.0f, max(-25.0f, err));
    _car.seekSteer(steer);

    _car.print(cout, *this) << "  err: " << err << "\n";

    // Done if within 3 degrees
    return ((abs(err) < 3.0) ? Command::NORMAL_END : Command::STILL_RUNNING);
  }

  void doEnd(Command::State reason) {
    _car.print(cout, *this) << "\n";
    _car.setSteer(0);
    _car.coast();
  }

private:
  RcCar& _car;
  // How much to turn (in signed degrees)
  float _relativeAng;
  float _targetAng;
};

namespace {
  // Flag will be set when user terminates via ^C or uses kill on process
  bool hasBeenInterrupted = false;

  void interrupted(int sig) {
    hasBeenInterrupted = true;
  }
}

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
      RcCar car;

      // Short drive to first corner
      car.add(new DriveToTurn(car, 0.5, 1.0));
      // Make a right hand turn
      car.add(new MakeTurn(car, 90.0));
      // Long drive to second corner
      car.add(new DriveToTurn(car, 0.5, 3.0));
      // Make a right hand turn
      car.add(new MakeTurn(car, 90.0));
      // Medium drive to third corner
      car.add(new DriveToTurn(car, 0.5, 2.0));
      // Make a right hand turn
      car.add(new MakeTurn(car, 90.0));
      // Long drive to fourth corner
      car.add(new DriveToTurn(car, 0.5, 3.0));
      // Make a right hand turn
      car.add(new MakeTurn(car, 90.0));
      // Short drive to finish line
      car.add(new DriveToTurn(car, 0.5, 1.0));
	      
      Timer autonTimer;
      Command::run(car);
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
