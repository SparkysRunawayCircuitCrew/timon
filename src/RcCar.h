/**
 * RcChar class definition.
 */

#ifndef __avc_RcCar_h
#define __avc_RcCar_h

#include "CommandParallel.h"
#include "Servo.h"

namespace avc {
  /**
   * Definition of the RC car to control.
   */
  class RcCar : public CommandParallel {

  private:
    // The "map" of GPIO pins (how things need to be wired)
    static const BlackLib::pwmName pwmSteer = BlackLib::P9_14;
    static const BlackLib::pwmName pwmEsc = BlackLib::P9_21;

  private:
    Servo _steer;
    Servo _esc;
    int _wayPoint;

  public:

    /**
     * Constructs a new instance and adds all of the commands to run.
     */
    RcCar();
    
    ~RcCar();

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
     * Helper method to "slowly" move towards a target power level.
     *
     * @param power The goal we eventually want to reach.
     *
     * @param maxStep The maximum change we are allowed to make to the
     * current power level each time this method is called.
     */
    void seekSpeed(float power, float maxStep = 0.05) {
      _esc.seek(power, maxStep);
    }

    /**
     * Helper method to "slowly" move towards a target steer setting.
     *
     * @param ang The goal (in signed decimal degrees) we eventually
     * want to reach (where 0 is straight).
     *
     * @param maxStep The maximum change we are allowed to make to the
     * current steering as we seek the new angle.
     */
    void seekSteer(float ang, float maxStep = 2.0) {
      _steer.seek(ang, maxStep);
    }

    /**
     * Sets the drive power to 0 allowing the car to coast to a stop.
     */
    void coast() {
      _esc.set(0.0);
    }

    /**
     * Sets the steering angle to a fixed value (note servo may take a
     * little time to physically reach the angle).
     *
     * @param angle The angle to 
     */
    void setSteer(float angle) {
      _steer.set(angle);
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

  };

  /**
   * Command to drive straight down the road until the turn point is detected.
   */
  class DriveToTurn : public Command {

  public:

    DriveToTurn(RcCar& car, float power, float timeout = 10.0);

    void doInitialize();

    Command::State doExecute();

    void doEnd(Command::State reason);

  private:
    RcCar& _car;
    float _power;
  };

  /**
   * Command to make a turn.
   */
  class MakeTurn : public Command {

  public:

    MakeTurn(RcCar& car, float relativeAng = 90.0);
    ~MakeTurn();

    void doInitialize();

    Command::State doExecute();

    void doEnd(Command::State reason);

  private:
    RcCar& _car;
    // How much to turn (in signed degrees)
    float _relativeAng;
    float _targetAng;
  };
}

#endif
