/**
 * TimonDriveStraight class implementation.
 */

#include "TimonDriveStraight.h"

using namespace avc;
using namespace std;

const float DriveStraight::DRIVE_POWER = 0.15;
const float DriveStraight::MAX_DRIVE_POWER = DRIVE_POWER * 1.5;
const float DriveStraight::P = 0.04;
const float DriveStraight::D = 0.025;

DriveStraight::DriveStraight(Timon& car, float heading, float minTime, bool relative) :
    Command("DriveStraight", 8.0),
    _car(car),
    _heading(heading),
    _desiredHeading(heading),
    _leftPower(0),
    _rightPower(0),
    _lastAngErr(0),
    _minTimeToDrive(minTime),
    _relative(relative)
{

}

DriveStraight::~DriveStraight() {

}

void DriveStraight::doInitialize() {
  if (_relative) {
	_desiredHeading = _car.getHeading() + _heading;
	if (_desiredHeading > 360.0) {
	    _desiredHeading -= 360.0;
	}
    } else {
	_desiredHeading = _heading;
    }
    // Hmmmm, should we compute last angle error or just init to 0?
    _lastAngErr = 0;
    
    _rightPower = _leftPower = DRIVE_POWER;
}

Command::State DriveStraight::doExecute() {
    float curHeading = _car.getHeading();
    float headingCorrection = 0; // TODO use red stanchions to correct car

    float angErr = computeAngDiff(_desiredHeading + headingCorrection, curHeading);
    float angErrChange = angErr - _lastAngErr;

    // Compute a % adjustment to power to "correct" for turn
    float adjPower = P * angErr + D * angErrChange;
    
    //_rightPower = _rightPower * (1 - adjPower);
    //_leftPower = _leftPower * (1 + adjPower);

    _rightPower = DRIVE_POWER * (1 - adjPower);
    _leftPower = DRIVE_POWER * (1 + adjPower);

    // TODO: We should adjust power based on how far/near to the
    // side we are as well (need red stanchion info)

    // Don't let power get too high
    _rightPower = min(_rightPower, MAX_DRIVE_POWER);
    _leftPower = min(_leftPower, MAX_DRIVE_POWER);

    _car.print(cout, *this) << "\n  desiredHeading: " << _desiredHeading
			    << "  heading: " << curHeading
			    << "  angErr: " << angErr
			    << "  left: " << _leftPower
			    << "  right: " << _rightPower << "\n";

    _car.drive(_leftPower, _rightPower);

    _lastAngErr = angErr;

    // If we found the yellow stanchion after driving at least 0.5 seconds
    // then consider ourselves at the end
    if (_car.atCorner() && (getElapsedTime() > _minTimeToDrive)) {
        _car.print(cout, *this) << "  FOUND YELLOW STANCHION!\n";
	return Command::NORMAL_END;
    }

    return Command::STILL_RUNNING;
}

void DriveStraight::doEnd(Command::State reason) {
    // Stop driving motors
    _car.coast();
}
