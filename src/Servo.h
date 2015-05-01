/**
 * Definition of a Servo class to manipulate a PWM controlled servo or ESC.
 */
#ifndef __avc_Servo_h
#define __avc_Servo_h

#include <BlackPWM.h>

namespace avc {

  /**
   * Servo wrapper around a BlackLib BlackPWM object used to control
   * RC Servos and ESC (Electronic Speed Controls).
   */
  class Servo {

  public:
    /**
     * Construct a new Servo instance.
     *
     * <p>NOTE: By default, this assumes you are using this object for
     * a servo which rotates from -45.0 to +45.0 degrees. If you are
     * using a servo with a different amount or rotation, you want to
     * map the min/max to a different range, you can set the desired
     * range at time of construction. For example, if you are
     * constructing a ESC instance you may want to set the range to
     * -1.0 to +1.0.</p>
     *
     * @param pwm The PWM name (see BlackLib::BlackPWM).
     *
     * @param minVal The minimum value your servo can move to (-45.0
     * by default).
     *
     * @param maxVal The maximum value your servo can move to (+45.0
     * by default).
     */
    Servo(BlackLib::pwmName pwm, float minVal = -45.0, float maxVal = +45.0);

    /**
     * Destructor will disable the servo.
     */
    ~Servo();

    /**
     * Gets the current setting of the servo (last value set).
     */
    float get() const {
      return curVal;
    }

    /**
     * Sets the target value for the servo and enables the servo
     * (method returns immediately as servo continues to move to
     * desired target).
     *
     * @param val The new value to set within the min/max range
     * limits specified.
     *
     * @return true if value accepted and servo set, false if value
     * out of range and request ignored.
     */
    bool set(float val);

    /**
     * Seeks the target value, but limits the maximum adjustment for
     * the servo and enables the servo (method returns immediately as
     * servo continues to move to desired target).
     *
     * @param val The new value that you are trying to reach.
     *
     * @param maxStep The maximum adjustment that can be made to the
     * current setting.
     *
     * @return true if value accepted and servo set, false if value
     * out of range and request ignored.
     */
    bool seek(float val, float maxStep);

    /**
     * Returns true if servo is being controlled, false otherwise.
     */
    bool isEnabled() const {
      return enabled;
    }

    /**
     * Disables the servo motor (turns off servo leaving it at its
     * current position until the next time set() is called).
     */
    void disable() {
      pwm.setRunState(BlackLib::stop);
      enabled = false;
    }

    /**
     * The minimum value that you can move the servo to (defined at
     * construction).
     */
    float getMin() const {
      return minVal;
    }

    /**
     * The maximum value that you can move the servo to (defined at
     * construction).
     */
    float getMax() const {
      return maxVal;
    }

    /**
     * Dumps debug information about the servo to the output stream provided.
     */
    std::ostream& dumpInfo(std::ostream& out) const;

  private:
    BlackLib::BlackPWM pwm;
    float curVal;
    float minVal;
    float maxVal;
    // Scalar used in converting value to duty cycle
    float m;
    bool enabled;

    /**
     * Enables the servo to be operated (you do not need to call this
     * directly, as set() calls when you try to set a value).
     *
     * @param dutyPercent Initial duty percent to set if enabling is required.
     *
     * @return false if the servo needed to be initialized and there
     * was a problem.
     */
    bool enable(float dutyPercent);
  };

}

#endif
