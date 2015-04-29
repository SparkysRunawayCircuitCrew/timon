/**
 * Definition of UserLeds class.
 */
#ifndef __avc_UserLeds_h
#define __avc_UserLeds_h

#include <iostream>

#include <time.h>

namespace avc {

  /**
   * UserLeds is a singleton used to control the 4 onboard LEDs on a BBB.
   */
  class UserLeds {

  public:

    /**
     * Get access to the single instance of the object used to
     * manipulate all 4 of the LEDs.
     */
    static UserLeds& getInstance() { return instance; }

    /**
     * Set the state of ALL four LEDs in a single shot.
     *
     * @param newState - Low bit goes with USER0, 0xf all on, 0x0 all off.
     *
     * @return true if successfully set state, false if file permission issue.
     */
    bool setState(int newState);

    /**
     * Return the last state set on ALL four LEDs in a single shot.
     */
    int getState() const { return state; }

    /**
     * Set the state of a specific LED.
     *
     * @param led The LED to modify in the range: [0, 3].
     * @param turnOn Pass true to turn on, false to turn off.
     *
     * @return true if successfully set state, false if file
     * permission issue or out of range led value passed.
     */
    bool setLed(int led, bool turnOn);

    /**
     * Query the last state set of individual LED.
     *
     * @param led The LED to get the state of in the range of [0, 3].
     *
     * @return true if LED is on, false if off.
     */
    bool isLedOn(int led) const;

  private:
    // Hide constructors (force use of getInstance)
    UserLeds();
    UserLeds(const UserLeds&);

    // Single instance of object
    static UserLeds instance;

    // Whether or not the LED has been initialized
    int initialized;

    // Current state of USER LEDs2
    int state;
  };

}

#endif
