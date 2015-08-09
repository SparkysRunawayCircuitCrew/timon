/**
 * This is the main entry point in the SparkFun Autonomous Vehicle Competition (AVC)
 * for the 2015 robot.
 */

#include "CommandSequence.h"
#include "Timon.h"
#include "TimonDriveStraight.h"
#include "Brake.h"

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
    BlackLib::BlackGPIO longButton(BlackLib::GPIO_4, BlackLib::input);
    // Extra mode start button connected to P9 24 (GPIO_15)
    BlackLib::BlackGPIO shortButton(BlackLib::GPIO_15, BlackLib::input);

    bool longWasHigh = longButton.isHigh();
    bool shortWasHigh = shortButton.isHigh();

    cout << "Entering main loop - waiting for trigger ...\n";

    while (hasBeenInterrupted == false) {

        bool shortIsHigh = shortButton.isHigh();
        bool longIsHigh = longButton.isHigh();

        // Run auton when button is pressed and then released
        if ((longWasHigh == true) && (longIsHigh == false)) {

            leds.setState(0xf);
            cout << "Starting auton for long path around track\n";
            timon.setAutonLongWay();

            Timer autonTimer;
            Command::run(timon);

            cout << "Long path auton completed in " << autonTimer.secsElapsed() << " seconds\n";
            timon.disable();

        } else if ((shortWasHigh == true) && (shortIsHigh == false)) {

            leds.setState(0xf);
            cout << "Starting auton for short path around track\n";
            timon.setAutonShortWay();

            Timer autonTimer;
            Command::run(timon);

            cout << "Short path auton completed in " << autonTimer.secsElapsed() << " seconds\n";
            timon.disable();

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
        longWasHigh = longIsHigh;
        shortWasHigh = shortIsHigh;
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
    _done(false),
    _stanchionsFile("/dev/shm/stanchions"),
    _lastStanchionFrame(0),
    _lastStanchionTimer(),
    _fileData(),
    _fileDataPrev(),
    _inTurn(false)
{
    if (!_gyro.reset()) {
        cerr << "**ERROR*** Failed to reset gyro\n";
        _crashed = true;
    }
}

void doTurns(Timon& timon, CommandSequence* drive) {
    // Give .25 seconds to let user move hand away
    drive->add(new DrivePowerTime(timon, 0, 0, 0.25));

    // How many turns to make
    const float numberOfTurns = 4;

    for (int i = 0; i < numberOfTurns; i++) {
		// Make a right hand turn
		drive->add(new MakeTurn(timon, 90.0));
    }

}

void doStop(Timon& timon, CommandSequence* drive) {
	drive->add(new DriveStraight(timon, 0, 2.0, true));
	drive->add(new Brake(timon, 1.0));
}

void Timon::setAutonLongWay() {
    clear();

	auto drive = new CommandSequence("Drive");

	//doTurns(*this, drive);
	doStop(*this, drive);

    drive->print(cout);

    add(drive);
}

void Timon::setAutonShortWay() {
    clear();

    CommandSequence* drive = new CommandSequence("Drive");
    // Give .25 seconds to let user move hand away
    drive->add(new DrivePowerTime(*this, 0, 0, 0.25));

    // Uncomment to spin left side forward one second followed
    // by right side forward for a second to verify logic is right
    //drive->add(new DrivePowerTime(*this, 0.2, 0, 1.0));

    // Make a left hand turn
    //    drive->add(new DriveToTurn(*this, 0.2, 10.0));
    const float numberOfTurns = 4;
	float curAng = 0;

    for (int i = 0; i < numberOfTurns; i++) {
	    // Experiment with "driving straight" command
	    drive->add(new DriveStraight(*this, curAng, 2.0, false));

		// Brake
		drive->add(new Brake(*this, 1.0));

		// Reverse
		drive->add(new DrivePowerTime(*this, -0.15, -0.15, 0.6));

		// And brake again, just to make sure
		drive->add(new Brake(*this, 0.5));

	    // Make a right hand turn
	    drive->add(new MakeTurn(*this, 90.0));

		curAng += 90;
    }

    drive->print(cout);

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

    // Used to keep track of how long since we've seen a stanchion
    _lastStanchionTimer.start();
    _lastStanchionFrame = 0;

    // Clear vision record (0 values and set found to None)
    _fileData.clear();
    _fileDataPrev.clear();

    memset(_stanchionCounts, 0, sizeof(_stanchionCounts));

    if (!_gyro.getHeading(_initHeading)) {
        _crashed = true;
        cerr << "***ERROR*** Gyro not responding (unable to read heading)\n";
    }

    CommandParallel::doInitialize();
}

void Timon::readSensors() {
    int ledsState = 0;

    // If process interrupted, consider car as crashed
    if (hasBeenInterrupted) {
	_crashed = true;
	cerr << "***ERROR*** Interrupted process\n";
    }

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

    //
    // Try to read in current vision status from sensors
    //
    FileData data;

    bool fileOk = false;

    for (int i = 0; i < 2; i++) {
        _stanchionsFile.seekg(0);
        _stanchionsFile.read((char*)&data, sizeof(data));

        if (data.frameCount == data.safetyFrameCount) {
	    // If this is a new frame, store previous info
	    if (_fileData.frameCount != data.frameCount) {
		_fileDataPrev = _fileData;

	    	if (data.found != _fileDataPrev.found) {
		    _stanchionCounts[_fileData.found]++;
	    	}
	    }
	    _fileData = data;
	    fileOk = true;
            break;
        }

        cerr << "WARNING: Framecount mismatch: " << data.frameCount
	     	 << ", " << data.safetyFrameCount << "\n";
        Timer::sleep(0.001);
    }

    if (fileOk == false) {
		_fileData.found = Found::None;
		_crashed = true;
		cerr << "***ERROR*** Failed to read valid record from stanchion file\n";
    }

    //
    // Check how long it's been since we've had a new image of a stanchion
    //
    if ((_fileData.found != Found::None) &&
	(_fileData.frameCount != _lastStanchionFrame)) {
	_lastStanchionFrame = _fileData.frameCount;
	_lastStanchionTimer.start();
    } else {
	//float maxTimeToWait = 2.0;
	const float maxTimeToWait = 5.0;

	if ( !_inTurn && (_lastStanchionTimer.secsElapsed() > maxTimeToWait) ) {
	    _crashed = true;
	    cerr << "***ERROR*** Failed to find a stanchion in last "
		 << maxTimeToWait << " seconds (frames: "
		 << _fileData.frameCount
		 << " last stanchion frame: " << _lastStanchionFrame
		 << ")\n";
	}
    }

    if (_fileData.found == Found::Yellow) {
	// Light 4th LED if yellow found (next to Ethernet)
	ledsState |= 0x8;
    } else if (_fileData.found == Found::Red) {
	// Light 3rd LED if red found
	ledsState |= 0x4;
    }
    if ((_fileData.boxHeight >= 80) && (_fileData.boxHeight <= 120)) {
	// Light 1st LED if last height was within range
	ledsState |= 0x1;
    }

    UserLeds& leds = UserLeds::getInstance();
    leds.setState(ledsState);
}

void Timon::exitTurn() {
	_inTurn = false;
	_lastStanchionTimer.start();
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
    // Let's use leds for reporting status now
    //    UserLeds& leds = UserLeds::getInstance();
    //    leds.setState(_wayPoint++);
}

float Timon::rangeCheckPower(float power) {
    return max(-1.0f, min(+1.0f, power));
}

ostream& Timon::print(std::ostream& out, const Command& cmd) const {
    out << cmd << "  Timon(left=" << _left.get() 
	<< ", right=" << _right.get() << ", heading=" << _heading 
	<< ", frameCount=" << _fileData.frameCount 
	<< ", found=" << _fileData.found << ", box_height=" << _fileData.boxHeight 
	<< ", inTurn=" << _inTurn << ")";
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

    bool foundCorner = _car.atCorner();

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
    Command("MakeTurn", 1.0 + abs(turn) / 25),
    _car(car),
    _turn(turn),
    _initialHeading(0),
    _lastErr(0),
    _lastCarTurned(0),
    _inRangeCnt(0)
{
}

MakeTurn::~MakeTurn() { 
}

void MakeTurn::doInitialize() {
    _car.print(cout, *this) << "MAKING TURN\n";
    _car.enterTurn();

    _initialHeading = _car.getHeading();
    _lastErr = _turn;
    _inRangeCnt = 0;
}

Command::State MakeTurn::doExecute() {
    float carTurned = _car.getRelativeHeading(_initialHeading);
    float err = _turn - carTurned;
    float deltaErr = _lastErr - err;
    const float P = (0.10f * 10.0f / 360.0f);
    const float D = (0.1f * 10.0f / 360.0f);
    //const float D = 0.0f;

    float steer = err * P + deltaErr * D;

    // Limit maximum range
    const float maxMag = 0.35f;
    steer = min(maxMag, max(-maxMag, steer));

    // Then shift to minimum power level
    // float steerMag = abs(steer);
    //const float minMag = 0.15f;
    //steer = (steer < 0) ? steer - minMag : steer + minMag;
    
    const float baseMinMag = 0.05f;

    float minMag = baseMinMag;
    if (abs(carTurned - _lastCarTurned) < 0.4) {
		minMag = maxMag;
    } 
    
    if (steer > -minMag && steer < minMag) {
		steer = (steer < 0) ? -minMag : minMag;
    }
  
    steer = Timon::rangeCheckPower(steer);
    _car.drive(steer, -steer);

    // TODO: NOTE, this implementation does not take into account a minimum
    // power to turn the car (for example, if we get within 10 degrees and
    // drop the power too low, the car may stop turning and never reach
    // the final target).

    _car.print(cout, *this) << "  turned: " << carTurned << "  err: " << err << "\n";

    _lastErr = err;

    // Done if within 3 degrees
    if (abs(err) < 6.0) {
        _inRangeCnt++;
    } else {
        _inRangeCnt = 0;
    }

    _lastCarTurned = carTurned;
    return ((_inRangeCnt >= 2) ? Command::NORMAL_END : Command::STILL_RUNNING);
}

void MakeTurn::doEnd(Command::State reason) {
    _car.print(cout, *this) << "\n";
    _car.exitTurn();
}

//
// Implementation of the MakeSmoothTurn class methods
//

MakeSmoothTurn::MakeSmoothTurn(Timon& car, float turn) :
    Command("MakeSmoothTurn", 1.0 + abs(turn) / 25),
    _car(car),
    _turn(turn),
    _initialHeading(0),
    _lastErr(0),
    _inRangeCnt(0)
{
}

MakeSmoothTurn::~MakeSmoothTurn() { 
}

void MakeSmoothTurn::doInitialize() {
    _car.print(cout, *this) << "\n";
    _initialHeading = _car.getHeading();
    _lastErr = _turn;
    _inRangeCnt = 0;
}

Command::State MakeSmoothTurn::doExecute() {
    float carTurned = _car.getRelativeHeading(_initialHeading);

    _car.drive(.25, .2);

    _car.print(cout, *this) << "  turned: " << carTurned << "\n";

    return ((abs(carTurned) >= abs(_turn)) ? Command::NORMAL_END : Command::STILL_RUNNING);
}

void MakeSmoothTurn::doEnd(Command::State reason) {
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

    _car.drive(_powerLeft, _powerRight);
    _car.print(cout, *this);

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
