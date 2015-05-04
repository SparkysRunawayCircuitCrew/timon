/**
 * This is the main entry point in the SparkFun Autonomous Vehicle Competition (AVC)
 * for the 2015 robot.
 */

#include "CommandSequence.h"
#include "Timon.h"
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

  Timon timon;

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
    leds.setState(0x0);
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
  _wayPoint(1)
{
  // TODO: Need real sensor update and catastrophic check command
  //Command* check = new Command("Check");
  //add(check);

  CommandSequence* drive = new CommandSequence("Drive");
  // Short drive to first corner
  drive->add(new DriveToTurn(*this, 0.2, 1.0));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Long drive to second corner
  drive->add(new DriveToTurn(*this, 0.2, 3.0));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Medium drive to third corner
  drive->add(new DriveToTurn(*this, 0.2, 2.0));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Long drive to fourth corner
  drive->add(new DriveToTurn(*this, 0.2, 3.0));
  // Make a right hand turn
  drive->add(new MakeTurn(*this, 90.0));
  // Short drive to finish line
  drive->add(new DriveToTurn(*this, 0.2, 1.0));
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
  _wayPoint = 1;
  CommandParallel::doInitialize();
}

Command::State Timon::doExecute() {
  if (checkState()) {
    return CommandParallel::doExecute();
  }
  // Catastrophic failure!
  return Command::INTERRUPTED;
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

ostream& Timon::print(std::ostream& out, const Command& cmd) const {
  out << cmd << "  Timon(left=" << _left.get() << ", right="
      << _right.get() << ")";
  return out;
}

bool Timon::checkState() {
  // TODO: Need to start adding sensors and feed back
  // For now, assume everything is OK
  return true;
}

//
// Implementation of the DriveToTurn class methods
//

DriveToTurn::DriveToTurn(Timon& car, float power, float timeout) :
  Command("DriveToTurn", timeout),
  _car(car),
  _power(power)
{
}

void DriveToTurn::doInitialize() {
  _car.print(cout, *this) << "\n";
}

Command::State DriveToTurn::doExecute() {
  _car.seekSpeed(_power);
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

MakeTurn::MakeTurn(Timon& car, float relativeAng) :
  Command("MakeTurn", abs(relativeAng) / 50 + 0.1),
  _car(car),
  _relativeAng(relativeAng),
  _targetAng(0)
{
}

MakeTurn::~MakeTurn() { 
}

void MakeTurn::doInitialize() {
  _car.print(cout, *this) << "\n";
  _targetAng = _car.getAngle() + _relativeAng;
}

Command::State MakeTurn::doExecute() {
  float carAng = _car.getAngle();
  float err = _targetAng - carAng;

  // TODO: Delete once we figure out how to get angle (the
  // statement below makes 
  float percentTimeElapsed = getElapsedTime() / getTimeout();
  err = (percentTimeElapsed >= 0.5) ? 0.0 : (1.1 - percentTimeElapsed) * _relativeAng;

  // Limit to .2 power level
  float steer = min(0.2f, max(-0.2f, err / 15));
  _car.seekDrive(steer, -steer);

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
