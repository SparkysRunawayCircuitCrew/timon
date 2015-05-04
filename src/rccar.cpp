#include "RcCar.h"
#include "CommandSequence.h"

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

//
// Implementation of the RcCar class methods
//

RcCar::RcCar() :
  CommandParallel("RcCar", true),
  _steer(pwmSteer, -40.0, 40.0),
  _esc(pwmEsc, -1.0, +1.0),
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
  // And stop (should probably just have a command for this)
  drive->add(new DriveToTurn(*this, 0.0, 1.0));
  add(drive);
}

RcCar::~RcCar() { 
  _steer.set(0);
  _esc.set(0);
  _steer.disable();
  _esc.disable();
}

void RcCar::doInitialize() {
  _wayPoint = 1;
  CommandParallel::doInitialize();
}

void RcCar::nextWaypoint() {
  UserLeds& leds = UserLeds::getInstance();
  leds.setState(_wayPoint++);
}

ostream& RcCar::print(std::ostream& out, const Command& cmd) const {
  out << cmd << "  Car(steer=" << _steer.get() << ", power="
      << _esc.get() << ")";
  return out;
}

//
// Implementation of the DriveToTurn class methods
//

DriveToTurn::DriveToTurn(RcCar& car, float power, float timeout) :
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
  _car.setSteer(0.0);
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

MakeTurn::MakeTurn(RcCar& car, float relativeAng) :
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

void MakeTurn::doEnd(Command::State reason) {
  _car.print(cout, *this) << "\n";
  _car.setSteer(0);
}
