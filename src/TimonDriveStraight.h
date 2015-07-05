/**
 * TimonDriveStraight class definition.
 */

#ifndef __avc_TimonDriveStraight_h
#define __avc_TimonDriveStraight_h

#ifndef __avc_Timon_h
#include "Timon.h"
#endif

namespace avc {

    /**
     * Command to drive straight down the road until the turn point is
     * detected (uses desired gryo heading and feed back from detected
     * red stanchions).
     */
    class DriveStraight : public Command {

    public:

	/**
	 * Construct a new instance of the command.
	 *
	 * @param car Reference to the vehicle to control.
	 *
	 * @param heading The heading to maintain on the gyro.
	 *
	 * @param relative Is the heading relative (true) to the
	 * current heading, or is an absolute (false) gyro heading.
	 */
	DriveStraight(Timon& car, float heading, float minTimeToDrive, bool relative);

        ~DriveStraight();

        void doInitialize();

        Command::State doExecute();

        void doEnd(Command::State reason);

    private:
	static float computeAngDiff(float a1, float a2) {
	    float diff = a1 - a2;
	    if (diff < -180) {
		diff += 360.0;
	    } else if (diff > 180.0) {
		diff -= 360.0;
	    }
	    return diff;
	}

	int getRedCount() const { return _car.getCounter(Found::Red) - _initialRed; }
	int getYellowCount() const { return _car.getCounter(Found::Yellow) - _initialYellow; }

	// Constant values
	static const float DRIVE_POWER;
	static const float MAX_DRIVE_POWER;

	// PID values (no I)
	static const float P;
	static const float D;

	// Member variables
        Timon& _car;
	float _heading;
        float _desiredHeading;
	float _leftPower;
	float _rightPower;
        float _lastAngErr;
	float _minTimeToDrive;
	bool _relative;

	int _initialRed;
	int _initialYellow;
    };
}

#endif
