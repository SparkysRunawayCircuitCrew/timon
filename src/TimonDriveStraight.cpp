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
    Command("DriveStraight", 3600),
    _car(car),
    _heading(heading),
    _desiredHeading(heading),
	_headingCorrection(0),
    _leftPower(0),
    _rightPower(0),
    _lastAngErr(0),
    _lastHeadingCorrection(0),
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

	resetCounts();
}

Command::State DriveStraight::doExecute() {
    float curHeading = _car.getHeading();

    FileData& fileData = _car.getFileData();

    const float targetHeight = 60;
    const float maxCorrection = 10;
	const float maxTimeToCorrectWithoutRed = 2.0f;

    // We found red, lets use it to correct heading
    if (fileData.found == Found::Red) {
		_correctionTimer.start();
		_headingCorrection = (targetHeight - fileData.boxHeight) * 0.2f;
    } else if (fileData.found == Found::None) {
		// We've corrected long enough wihout a new red, reset!
		if (_correctionTimer.isRunning() && _correctionTimer.secsElapsed() >= maxTimeToCorrectWithoutRed) {
			_headingCorrection = 0;
			_correctionTimer.pause();
		}
	}

	const float maxCorrectionDiff = 360;

	// If the change in heading correction is too great, assume something went wrong
	float headingCorrectionDiff = abs(_headingCorrection - _lastHeadingCorrection);
	if (headingCorrectionDiff > maxCorrectionDiff) {
		cout << "HEADING CORRECTION IS TOO GREAT: " << headingCorrectionDiff << "\n"
			 << "CUR CORRECTION: " << _headingCorrection << "\n"
			 << "LAST CORRECTION: " << _lastHeadingCorrection << "\n";

		//return Command::INTERRUPTED;
	}

    _headingCorrection = max(min(_headingCorrection, maxCorrection), -maxCorrection);

    float angErr = computeAngDiff(_desiredHeading + _headingCorrection, curHeading);
    float angErrChange = angErr - _lastAngErr;

    // Compute a % adjustment to power to "correct" for turn
    float adjPower = P * angErr + D * angErrChange;
    
    //_rightPower = _rightPower * (1 - adjPower);
    //_leftPower = _leftPower * (1 + adjPower);

	// NOTE PLUS AND MINUS ARE REVERESED HERE.
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
							<< "  right: " << _rightPower
                            << "  redCnt: " << getRedCount()
                            << "  yelCnt: " << getYellowCount()
							<< "  correction:  " << _headingCorrection
                            << "\n";

    _car.drive(_leftPower, _rightPower);

    _lastAngErr = angErr;
	_lastHeadingCorrection = _headingCorrection;

    if (getElapsedTime() < _minTimeToDrive) {
		resetCounts();
		_car.print(cout, *this) << "\nResetting counts to zero" << "\n";

		return Command::STILL_RUNNING;
    }
	
	if (getYellowCount() >= 1 && getRedCount() >= 1) {
        _car.print(cout, *this) << "\nFOUND YELLOW STANCHION!\n";
		return Command::NORMAL_END;
    }

    return Command::STILL_RUNNING;
}

void DriveStraight::doEnd(Command::State reason) {
    // Stop driving motors
    _car.coast();
}
