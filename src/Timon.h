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

namespace avc {
  /**
   * Definition of the RC car to control.
   */
  class Timon : public CommandParallel {

  private:
    // The "map" of GPIO pins (how things need to be wired)

    // P9 pin 14 (GPIO 40)
    static const BlackLib::pwmName LEFT_PWM = BlackLib::P9_14;
    // P9 pin 11 (GPIO 30)
    static const BlackLib::gpioName LEFT_GPIO_FWD = BlackLib::GPIO_30;
    // P9 pin 12 (GPIO 60)
    static const BlackLib::gpioName LEFT_GPIO_REV = BlackLib::GPIO_60;

    // P9 pin 21 (GPIO 3)
    static const BlackLib::pwmName RIGHT_PWM = BlackLib::P9_21;
    // P9 pin 13 (GPIO 31)
    static const BlackLib::gpioName RIGHT_GPIO_FWD = BlackLib::GPIO_31;
    // P9 pin 15 (GPIO 48)
    static const BlackLib::gpioName RIGHT_GPIO_REV = BlackLib::GPIO_48;

  private:
#if USE_SERVOS
    Servo _left;
    Servo _right;
#else
    HBridge _left;
    HBridge _right;
#endif
    int _wayPoint;

  public:

    /**
     * Constructs a new instance and adds all of the commands to run.
     */
    Timon();
    
    ~Timon();

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
     * Returns the gyro reading (TODO - not implemented yet).
     *
     * @return Absolute value of the gyro reading in signed decimal
     * degrees wher e 0 is from the direction the car was facing when
     * the gyro was zeroed.
     */
    float getAngle() {
      // TODO - Need gyro reading or some other way to get angle of car
      return 0.0;
    }

    /**
     * Will go true once we have detected that we've reached a corner
     * in the course and need to make a turn (TODO - not implemented
     * yet).
     */
    bool detectedCorner() {
      // TODO - Need to figure out this from vision
      return false;
    }

    /**
     * Dumps some information about the current state of the vehicle
     * on the course.
     */
    std::ostream& print(std::ostream& out, const Command& cmd) const;

  private:

    /**
     * Method to read the sensors attached to the car and check for
     * catastrophic failure indicators.
     *
     * @return true if state is OK (we can continue to drive), false
     * if catastrophic failure and we need to abort. NOTE: This method
     * is called once per execute cycle by {@link #doExecute} before
     * allowing commands to continue running.
     */
    bool checkState();

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
  };

  /**
   * Command to make a turn.
   */
  class MakeTurn : public Command {

  public:

    MakeTurn(Timon& car, float relativeAng = 90.0);
    ~MakeTurn();

    void doInitialize();

    Command::State doExecute();

    void doEnd(Command::State reason);

  private:
    Timon& _car;
    // How much to turn (in signed degrees)
    float _relativeAng;
    float _targetAng;
  };
}

#endif
