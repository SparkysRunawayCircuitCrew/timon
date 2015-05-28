/**
 * This is the main entry point in the SparkFun Autonomous Vehicle Competition (AVC)
 * for the 2015 robot.
 */

#include "CommandSequence.h"
#include "Timon.h"
#include "UserLeds.h"

#include <BlackGPIO.h>

#include <cmath>
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

  // Create instance of vehicle
  Timon timon;

  // Start button connected to P9 18 (GPIO_4)
  BlackLib::BlackGPIO startButton(BlackLib::GPIO_4, BlackLib::input);
  // Extra mode start button connected to P9 24 (GPIO_15)
  //BlackLib::BlackGPIO startButton(BlackLib::GPIO_15, BlackLib::input);
  // Start button connected to P9 pin 23 (GPIO_49)
  //BlackLib::BlackGPIO startButton(BlackLib::GPIO_49, BlackLib::input);
  bool startWasHigh = startButton.isHigh();

  cout << "Entering main loop - waiting for trigger ...\n";

  while (hasBeenInterrupted == false) {

    bool startIsHigh = startButton.isHigh();

    // Run auton when button is pressed and then released
    if ((startWasHigh == true) && (startIsHigh == false)) {
      leds.setState(0xf);
      cout << "Button released, starting auton\n";

      Timer autonTimer;
      Command::run(timon);

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
    // Save prior state
    startWasHigh = startIsHigh;
  }

  cout << "Terminated by external signal\n";

  return 0;
}

//
// Implementation of the Timon class methods
//

Timon::Timon() :
  CommandParallel("Timon", true),
#if USE_SERVOS
  _left(LEFT_PWM, -1.0, +1.0),
  _right(RIGHT_PWM, -1.0, +1.0),
#else
  _left(LEFT_PWM, LEFT_GPIO_FWD, LEFT_GPIO_REV),
  _right(RIGHT_PWM, RIGHT_GPIO_FWD, RIGHT_GPIO_REV),
#endif
  _gyro(),
  _initHeading(0),
  _heading(0),
  _wayPoint(1),
  _crashed(false),
  _done(false)
{
  if (!_gyro.reset()) {
    cerr << "**ERROR*** Failed to reset gyro\n";
    _crashed = true;
  }

  CommandSequence* drive = new CommandSequence("Drive");
  // Short drive to first corner
  drive->add(new DriveToTurn(*this, 0.2, 2.0));
  // Give 1/2 second to slow down
  drive->add(new DrivePowerTime(*this, 0, 0, 0.5));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Long drive to second corner
  drive->add(new DriveToTurn(*this, 0.2, 5.0));
  // Give 1/2 second to slow down
  drive->add(new DrivePowerTime(*this, 0, 0, 0.5));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Medium drive to third corner
  drive->add(new DriveToTurn(*this, 0.2, 4.0));
  // Give 1/2 second to slow down
  drive->add(new DrivePowerTime(*this, 0, 0, 0.5));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Long drive to fourth corner
  drive->add(new DriveToTurn(*this, 0.2, 5.0));
  // Give 1/2 second to slow down
  drive->add(new DrivePowerTime(*this, 0, 0, 0.5));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Short drive to finish line
  drive->add(new DriveToTurn(*this, 0.2, 2.0));
  // And stop (NOTE: this is optional now as the Timon class should
  // automatically disable everything after finishing an auton run)
  drive->add(DrivePowerTime::createStopCommand(*this));
  add(drive);
}

Timon::~Timon() { 
  _left.set(0);
  _right.set(0);
  _left.disable();
  _right.disable();
}

void Timon::doInitialize() {
  _crashed = _done = false;
  _wayPoint = 1;

  if (!_gyro.getHeading(_initHeading)) {
    _crashed = true;
    cerr << "***ERROR*** Gyro not responding (unable to read heading)\n";
  }

  CommandParallel::doInitialize();
}

void Timon::readSensors() {
  float heading;
  if (_gyro.getHeading(heading)) {
    heading -= _initHeading;
    if (heading < 0) {
      heading += 360.0;
    }
    _heading = heading;
  } else {
    _crashed = true;
    cerr << "***ERROR*** Gyro not responding (unable to read heading)\n";
  }
}

float Timon::getRelativeHeading(float initHeading) const {
  float relHeading = _heading - initHeading;
  // Put in range of [0, 360]
  if (relHeading < 0) {
    relHeading += 360;
  }
  // Now adjust to range of [-180.0, +180.0]
  if (relHeading > 180) {
    relHeading -= 360;
  }
  return relHeading;
}

Command::State Timon::doExecute() {
  readSensors();
  if (hasCrashed()) {
    // Make sure motors are turned off
    coast();
    // Catastrophic failure!
    return Command::INTERRUPTED;
  }

  return CommandParallel::doExecute();
}

void Timon::doEnd(Command::State reason) {
  CommandParallel::doEnd(reason);
  disable();
}

void Timon::disable() {
  coast();
}

void Timon::nextWaypoint() {
  UserLeds& leds = UserLeds::getInstance();
  leds.setState(_wayPoint++);
}

float Timon::rangeCheckPower(float power) {
  return max(-1.0f, min(+1.0f, power));
}

ostream& Timon::print(std::ostream& out, const Command& cmd) const {
  out << cmd << "  Timon(left=" << _left.get() << ", right="
      << _right.get() << ", heading=" << _heading << ")";
  return out;
}

//
// Implementation of the DriveToTurn class methods
//

DriveToTurn::DriveToTurn(Timon& car, float power, float timeout) :
  Command("DriveToTurn", timeout),
  _car(car),
  _power(power),
  _initialHeading(0)
{
}

void DriveToTurn::doInitialize() {
  _initialHeading = _car.getHeading();
  _car.print(cout, *this) << "\n";
}

Command::State DriveToTurn::doExecute() {
  float turned = _car.getRelativeHeading(_initialHeading);
  float turnedMag = abs(turned);

  float powerCorrect = 0;
  if (turnedMag > 1.0) {
    // TODO: Figure out a power correction (basically P of PID) based
    // on how far turn is off
    powerCorrect = turnedMag / 180 + 0.005;
    powerCorrect = (turned > 0) ? powerCorrect : -powerCorrect;
    cout << "Off course: " << turned << " degrees, power correct: "
	 << powerCorrect << "\n";
  }

  float powerLeft = _power - powerCorrect;
  float powerRight = _power + powerCorrect;

  // Seek new drive power levels with power correction for steering
  powerLeft = Timon::rangeCheckPower(powerLeft);
  powerRight = Timon::rangeCheckPower(powerRight);
  _car.seekDrive(powerLeft, powerRight);

  bool foundCorner = _car.detectedCorner();
  // TODO: Remove this time out override once we can detect the corner
  foundCorner = ((getElapsedTime() / getTimeout()) >= 0.5);
  _car.print(cout, *this) << "\n";
  return (foundCorner ? Command::NORMAL_END : Command::STILL_RUNNING);
}

void DriveToTurn::doEnd(Command::State reason) {
  _car.print(cout, *this) << "\n";
  _car.nextWaypoint();
}

//
// Implementation of the MakeTurn class methods
//

MakeTurn::MakeTurn(Timon& car, float turn) :
  Command("MakeTurn", 1.0 + abs(turn) / 50),
  _car(car),
  _turn(turn),
  _initialHeading(0)
{
}

MakeTurn::~MakeTurn() { 
}

void MakeTurn::doInitialize() {
  _car.print(cout, *this) << "\n";
  _initialHeading = _car.getHeading();
}

Command::State MakeTurn::doExecute() {
  float carTurned = _car.getRelativeHeading(_initialHeading);
  float err = _turn - carTurned;

  // TODO: Delete once we figure out how to get angle (the
  // statement below makes 
  //float percentTimeElapsed = getElapsedTime() / getTimeout();
  //err = (percentTimeElapsed >= 0.5) ? 0.0 : (1.1 - percentTimeElapsed) * _turn;

  // Limit to .2 power level
  float steer = min(0.2f, max(-0.2f, err / 15));
  
  steer = Timon::rangeCheckPower(steer);
  _car.seekDrive(steer, -steer);

  // TODO: NOTE, this implementation does not take into account a minimum
  // power to turn the car (for example, if we get within 10 degrees and
  // drop the power too low, the car may stop turning and never reach
  // the final target).

  _car.print(cout, *this) << "  err: " << err << "\n";

  // Done if within 3 degrees
  return ((abs(err) < 3.0) ? Command::NORMAL_END : Command::STILL_RUNNING);
}

void MakeTurn::doEnd(Command::State reason) {
  _car.print(cout, *this) << "\n";
}

//
// Implementation of the DrivePowerTime class methods
//

DrivePowerTime::DrivePowerTime(Timon& car, float powerLeft, float powerRight,
			       float howLong) :
  Command("DrivePowerTime", howLong + 1.0),
  _car(car),
  _powerLeft(powerLeft),
  _powerRight(powerRight),
  _runTime(howLong)
{
}

DrivePowerTime::~DrivePowerTime() { 
}

Command::State DrivePowerTime::doExecute() {
  if (getElapsedTime() >= _runTime) {
    return Command::NORMAL_END;
  }

  _car.seekDrive(_powerLeft, _powerRight);
  return Command::STILL_RUNNING;
}

void DrivePowerTime::doEnd(Command::State reason) {
  // If command was to bring car to a stop and the 
  if (reason == Command::NORMAL_END) {
    if (_powerLeft == 0 && _powerRight == 0) {
      _car.drive(0, 0);
    }
  }
}
