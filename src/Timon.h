/**
 * Timon class definition.
 */

#ifndef __avc_Timon_h
#define __avc_Timon_h

#define USE_SERVOS 0

#include "CommandParallel.h"
#if USE_SERVOS
#include "Servo.h"
#else
#include "HBridge.h"
#endif

#include "GyroBNO055.h"

#include <fstream>

enum Found: int {
    None,
        Red,
        Yellow,
        };

struct FileData {
    int frameCount; 
    Found found;

    int boxWidth, boxHeight;
    int xMid, yBot;

    int safetyFrameCount;

    // Zeros contents (which results in Found::None)
    void clear() { memset((char*) this, 0, sizeof(FileData)); }

    // Let constructor clear contents
    FileData() { clear(); }
};

namespace avc {
    /**
     * Definition of the RC car to control.
     */
    class Timon : public CommandParallel {

    private:
        // The "map" of GPIO pins (how things need to be wired)

        // P9 pin 16 (GPIO 51)
        static const BlackLib::pwmName RIGHT_PWM = BlackLib::P9_16;
        // P9 pin 17 (GPIO 5)
        static const BlackLib::gpioName RIGHT_GPIO_FWD = BlackLib::GPIO_5;
        // P9 pin 23 (GPIO 49)
        static const BlackLib::gpioName RIGHT_GPIO_REV = BlackLib::GPIO_49;

        // P9 pin 21 (GPIO 3)
        static const BlackLib::pwmName LEFT_PWM = BlackLib::P9_21;
        // P9 pin 15 (GPIO 48)
        static const BlackLib::gpioName LEFT_GPIO_FWD = BlackLib::GPIO_48;
        // P9 pin 12 (GPIO 60)
        static const BlackLib::gpioName LEFT_GPIO_REV = BlackLib::GPIO_60;

    private:
#if USE_SERVOS
        Servo _left;
        Servo _right;
#else
        HBridge _left;
        HBridge _right;
#endif
        // Gyro to track direction of car
        GyroBNO055 _gyro;

        // Initial reading of the gyro at the start of the run
        float _initHeading;

        // Current heading of the car since the start of auton.
        float _heading;

        // Used to count how far we've progressed
        int _wayPoint;

	// Used to store stanchions counts
	int _stanchionCounts[3];

        // Will be true if something terrible happens
        bool _crashed;

	// Will be true if in turn
	bool _inTurn;

        // Will be true once we've reached the final point in our drive
        bool _done;

        // A file handle to the stanchion data
        std::ifstream _stanchionsFile;

	// Frame ID on vision record of last time we saw a stanchion
	int _lastStanchionFrame;
	// Timer used to track how long it's been since we've seen a stanchion
	Timer _lastStanchionTimer;

        // Vision information record read from avc-vision file
        FileData _fileData;

        // Previous vision information record (in case you want to compare)
        FileData _fileDataPrev;

    public:

        /**
         * Constructs a new instance and adds all of the commands to run.
         */
        Timon();
    
        ~Timon();

        /**
         * Load in commands to drive the long way around the road.
         */
        void setAutonLongWay();

        /**
         * Load in commands to drive the short way around the road.
         */
        void setAutonShortWay();

        /**
         * Returns true if we've detected a crashed situation along the way.
         */
        bool hasCrashed() const { return _crashed; }

        /**
         * Returns true once we've reached the final destination.
         */
        bool isDone() const { return _done; }

        /**
         * Reads the state of all of the sensors attached to the vehicle.
         */
        void readSensors();

	/**
	 * Gets the specified counter
	 */
	int getCounter(Found counter) const { return _stanchionCounts[counter]; }

        /**
         * Returns whether or not the last detection resulted in a yellow stanchion
         */
        bool atCorner() const {
            return _fileData.found == Found::Yellow;
        }

	void enterTurn() { _inTurn = true; }
	void exitTurn();

        /**
         * Returns the last reported heading from the gyro (from last
         * "readSensors()" invocation).
         *
         * <p>NOTE: This is relative to the starting point (where the
         * angle is "zeroed" by {@link #doInitialization}).</p>
         *
         * @return Absolute heading in the range of [0, 360] degrees.
         */
        float getHeading() const { return _heading; }

        /**
         * Returns the relative heading based on the last reported heading
         * from the gyro (from last "readSensors()" invocation) and some
         * previous value.
         *
         * @param initHeading The initial starting point (typically from
         * some earliar invocation of {@link #getHeading}).
         *
         * @return Relative heading in the range of [-180, +180] degrees.
         */
        float getRelativeHeading(float initHeading) const;

        /**
         * Used to provide an indication that the car has reached the next
         * way point in the path.
         */
        void nextWaypoint();

        /**
         * Initializes the car for a new run through the course.
         */
        void doInitialize();

        /**
         * Periodic routine that checks the state of the car (updating
         * sensor readings) and then passes control to the executing
         * commands.
         */
        Command::State doExecute();

        /**
         * At end of autonomous command, we will make sure to invoke the
         * disable command to turn everything off.
         */
        void doEnd(Command::State reason);

        /**
         * Method which disables all actuators (motors) - should be called
         * after autonomous run to make sure everything is "off" (is
         * called by {@link #doEnd}).
         */
        void disable();

        /**
         * Forces a power value within the legal range.
         *
         * @param power The power value to check.
         *
         * @param Power value to apply (will be power or closest legal value).
         */
        static float rangeCheckPower(float power);

        /**
         * Helper method to "slowly" move towards a target power level.
         *
         * @param power The goal we eventually want to reach.
         *
         * @param maxStep The maximum change we are allowed to make to the
         * current power level each time this method is called.
         */
        void seekSpeed(float power, float maxStep = 0.05) {
            _left.seek(power, maxStep);
            _right.seek(power, maxStep);
        }

        /**
         * Helper method to "slowly" move towards a target steer setting.
         *
         * @param leftPower The desired power output for the left side
         * (range of [-1, +1].
         *
         * @param rightPower The desired power output for the right side
         * (range of [-1, +1].
         *
         * @param maxStep The maximum change we are allowed to make to the
         * current power output to the motors.
         */
        void seekDrive(float leftPower, float rightPower, float maxStep = 0.05) {
            _left.seek(leftPower, maxStep);
            _right.seek(rightPower, maxStep);
        }

        /**
         * Sets the pwoer output to left and right side independent of
         * each other (allows "turning" and "spinning".
         *
         * @param leftPower The power output for the left side (range of [-1, +1].
         * @param rightPower The power output for the right side (range of [-1, +1].
         */
        void drive(float leftPower, float rightPower) {
            _left.set(leftPower);
            _right.set(rightPower);
        }

        /**
         * Sets the drive power to 0 allowing the car to coast to a stop.
         */
        void coast() {
            drive(0.0, 0.0);
        }

        /**
         * Dumps some information about the current state of the vehicle
         * on the course.
         */
        std::ostream& print(std::ostream& out, const Command& cmd) const;

    private:
        
    };

    /**
     * Command to drive at preset power levels for a specific amount of time.
     */
    class DrivePowerTime : public Command {

    public:

        DrivePowerTime(Timon& car, float powerLeft, float powerRight, float howLong);
        ~DrivePowerTime();

        static Command* createStopCommand(Timon& car) {
            return new DrivePowerTime(car, 0, 0, 0);
        }

        Command::State doExecute();

        void doEnd(Command::State reason);

    private:
        Timon& _car;
        float _powerLeft;
        float _powerRight;
        float _runTime;
    };

    /**
     * Command to drive straight down the road until the turn point is detected.
     */
    class DriveToTurn : public Command {

    public:

        DriveToTurn(Timon& car, float power, float timeout = 10.0);

        void doInitialize();

        Command::State doExecute();

        void doEnd(Command::State reason);

    private:
        Timon& _car;
        float _power;
        float _initialHeading;
    };

    /**
     * Command to make a relative turn in the range of [-150.0, +150.0]
     * degrees.
     *
     * <p>This command will likely "blow up" with spastic spins if you
     * try to make too large of a turn. If you keep your turns under 150
     * degrees in either direction and you will probably be OK.</p>
     */
    class MakeTurn : public Command {

    public:

        MakeTurn(Timon& car, float turn = 90.0);
        ~MakeTurn();

        void doInitialize();

        Command::State doExecute();

        void doEnd(Command::State reason);

    private:
        Timon& _car;
        // How much to turn (in signed degrees)
        float _turn;
        float _initialHeading;
        float _lastErr;
	float _lastCarTurned;
        int _inRangeCnt;
    };

    /**
     * Command to make a relative turn in the range of [-150.0, +150.0]
     * degrees.
     *
     * <p>This command will likely "blow up" with spastic spins if you
     * try to make too large of a turn. If you keep your turns under 150
     * degrees in either direction and you will probably be OK.</p>
     */
    class MakeSmoothTurn : public Command {

    public:

        MakeSmoothTurn(Timon& car, float turn = 90.0);
        ~MakeSmoothTurn();

        void doInitialize();

        Command::State doExecute();

        void doEnd(Command::State reason);

    private:
        Timon& _car;
        // How much to turn (in signed degrees)
        float _turn;
        float _initialHeading;
        float _lastErr;
        int _inRangeCnt;
    };
}

#endif
