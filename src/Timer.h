/**
 * Definition of a Timer encapsulate timing and delays.
 */
#ifndef __avc_Timer_h
#define __avc_Timer_h

#include <iostream>

#include <time.h>

namespace avc {

  /**
   * Timer class wrapper the clock_gettime function that gets a
   * monotonic time (see: "man 3 clock_gettime").
   */
  class Timer {

  public:

    /**
     * Wrapper around the nanosleep function to sleep for a precise
     * amount of time.
     *
     * @param seconds The number of seconds to sleep (you can specify
     * very small amounts of time).
     *
     * @return Will return 0.0 if the operation performed normally
     * (slept the desired time period). If the operation failed (was
     * interrupted for example), this method returns the number of
     * seconds NOT slept (still needing to sleep).
     */
    static float sleep(float seconds);

    /**
     * Used for short sleep times (less than a second).
     *
     * @param nanos How many nanoseconds to sleep in the range of [0,
     * 999,999,999].
     *
     * @return 0 If sleep successful, otherwise, the number of nanos
     * still needing to sleep.
     */
    static int sleepNanos(int nanos);
    
    /**
     * Gets a high precision time stamp that ignores real-time adjustments.
     *
     * @param storeIn Where to store the results.
     *
     * @return true If successfully got time, false if not (check
     * errno - see "man clock_gettime" for details).
     */
    static bool getTime(timespec& storeIn) {
      storeIn.tv_sec = 0;
      storeIn.tv_nsec = 0;
      return clock_gettime(CLOCK_MONOTONIC_RAW, &storeIn) == 0;
    }

    /**
     * Computes the difference between two time stamps in seconds.
     *
     * @param fromHere The starting point in time.
     * @param toHere The ending point in time.
     *
     * @returns (toHere - fromHere) as a number of seconds.
     */
    static float diffSecs(const timespec& fromHere, const timespec& toHere);

    /**
     * Indicates whether the timer is running or paused.
     */
    bool isRunning() const {
      return running;
    }

    /**
     * Resets and starts the timer (sets start time to current time
     * and puts timer in a running state).
     */
    bool start() {
      Timer::getTime(startTime);
      endTime = startTime;
      running = true;
    }

    /**
     * Pauses the timer (records the pause time and takes timer out of
     * running state).
     */
    bool pause() {
      Timer::getTime(endTime);
      running = false;
    }

    /**
     * Puts the timer back in a running state (without resetting the
     * start time).
     */
    bool unpause() {
      running = true;
    }

    /**
     * Attempts to sleep until the next nano second period.
     *
     * <p>This method is useful if you want to fire something off at a
     * constant rate. For example if you want to run a function
     * "foo()" 60 times a second:</p>
     *
     * <pre>
     * int nanoPeriod = 1000000000 / 60; // Period for 60 Hz rate
     * Timer runTime;
     *
     * while (continueRunning()) {
     *   runTime.sleepUntilNextNano(nanoPeriod);
     *   foo();
     * }
     * </pre>
     *
     * @param nanoPeriod How long the time period should be (in the
     * range of [0, 999,999,999]).
     */
    int sleepUntilNextNano(int nanoPeriod) const;

    /**
     * Determines the number of seconds which have elapsed.
     *
     * @return If the timer is paused, this method returns the number
     * of seconds elapsed from when the timer was started and then
     * paused. If the timer is still running, this method returns the
     * number of seconds from when the timer was last started to now.
     */
    float secsElapsed() const;

    /**
     * Construct a new Timer instance and "start" it.
     *
     * <p>This will create a new instance of the Timer object and
     * "press the start button" (initializes timer with current time
     * stamp).</p>
     */
    Timer() {
      start();
    }

    /**
     * Construct a new Timer instance that is a copy of another.
     *
     * @param t The other Timer to copy values from.
     */
    Timer(const Timer& t) :
      startTime(t.startTime),
      endTime(t.endTime),
      running(t.running) {
    }

    /**
     * Dumps debug information about the servo to the output stream provided.
     */
    std::ostream& printJson(std::ostream& out) const;

  private:
    // processor clock timestamp when timer started
    timespec startTime;
    // processor clock timestamp when timer paused
    timespec endTime;
    // Whether we are in a runing or paused state
    bool running;
  };

}

#endif
