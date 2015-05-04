/**
 * HBridge definition.
 */
#ifndef __avc_HBridge_h
#define __avc_HBridge_h

#include <BlackGPIO.h>
#include <BlackPWM.h>

namespace avc {

  /**
   * HBridge wrapper around a BlackLib BlackPWM and BlackGPIO objects
   * used to control power output to motors via a PWM signal and two
   * control lines with four states (forward, reverse, coast and
   * brake).
   */
  class HBridge {

  public:
    /**
     * Construct a new HBridge instance.
     *
     * @param pwmPower The PWM name used to control the power output
     * (see BlackLib::BlackPWM).
     *
     * @param gpioFwd The GPIO name to the pin used to "shift into" forward.
     *
     * @param gpioRev The GPIO name to the pin used to "shift into" reverse.
     *
     * @param period The period (in nanoseconds) of the PWM signal
     * (defaults to 1000000 if omitted).
     */
    HBridge(BlackLib::pwmName pwmPower, BlackLib::gpioName gpioFwd,
	    BlackLib::gpioName gpioRev, uint64_t period = 1000000);

    /**
     * Destructor will disable power and shift it into neutral (coast).
     */
    ~HBridge();

    /**
     * Gets the current power level (last value set - see {@link #setPower}).
     */
    float get() const {
      return curVal;
    }

    /**
     * Sets the power output value.
     *
     * @param val The new power value in the range of [-1.0, +1.0]
     * where positive values will shift into forward, negative values
     * will shift into reverse and a 0 value will put it into coast
     * mode.
     *
     * @return true if value accepted and new power level set.
     */
    bool set(float val);

    /**
     * Seeks the target value, but limits the maximum power adjustment.
     *
     * @param val The new value that you are trying to reach.
     *
     * @param maxStep The maximum adjustment that can be made to the
     * current setting.
     *
     * @return true if value set or moved towards, false if value
     * out of range and request ignored.
     */
    bool seek(float val, float maxStep);

    /**
     * Returns true if motor is being controlled, false otherwise.
     */
    bool isEnabled() const {
      return enabled;
    }

    /**
     * Disables the motor (sets power to 0 and shifts into neutral and
     * then turns off PWM signal).
     */
    void disable();

    /**
     * Dumps debug information about the motor to the output stream provided.
     */
    std::ostream& dumpInfo(std::ostream& out) const;

  private:
    BlackLib::BlackPWM pwmPower;

    // NOTE: We used pointers so we can allocate but never deallocate
    // the GPIO pins as BlackLib will unexport them in the destrutor
    // (and when they are unexported they can go to strange states)
    BlackLib::BlackGPIO* gpioFwd;
    BlackLib::BlackGPIO* gpioRev;
    uint64_t period;
    float curVal;
    bool enabled;
    bool fwdState;
    bool revState;

    /**
     * Enables the motor to be operated (you do not need to call this
     * directly, as set() calls when you try to set a value).
     *
     * @param dutyPercent Initial duty percent to set if enabling is required.
     *
     * @return false if the motor needed to be initialized and there
     * was a problem.
     */
    bool enable(float dutyPercent);
  };

}

#endif
