/**
 * Definition of the Command interface and several supporting classes.
 */
#ifndef __avc_Command_h
#define __avc_Command_h

#include "Timer.h"

#include <iostream>
#include <string>
#include <vector>

namespace avc {

  /**
   * Command class interface similar to the Command classes in the WPI
   * library used by FRC.
   */
  class Command {

  public:
    /**
     * Possible reasons why a command ended.
     */
    enum State {
      /** Normal termination (command ran as expected without problems). */
      NORMAL_END = 0,

      /** Command is still running. */
      STILL_RUNNING = 1,

      /** Command has never been started. */
      NEVER_STARTED = 2,

      /** Command was running, but was interrupted by some external cause. */
      INTERRUPTED = 3,

      /** Command was running, but failed to run in maximum time allowed. */
      TIMED_OUT = 4
    };

  protected:
    /**
     * Called one time (by {@link #initalize} to allow command
     * specific initialization - default implementation does nothing).
     */
    virtual void doInitialize();

    /**
     * This method is called repeatedly while the command still needs
     * to be run.
     *
     * @return State The state of the command (STILL_RUNNING until
     * command is done and then typically NORMAL_END). The default
     * implemetation returns NORMAL_END immediately.
     */
    virtual State doExecute();

    /**
     * Derived classes can implement this method if they need to do
     * any clean up after running.
     *
     * @param reason The reason why the command is ending.
     */
    virtual void doEnd(State reason);

  public:
    /**
     * Construct a new command object with an assigned name.
     *
     * @param name The name used to identify the command.
     *
     * @param timeout The maximum number of seconds this command
     * should take to complete (defaults to 1.0 seconds).
     */
    Command(const std::string& name, float timeout = 1.0);

    /**
     * Destructor.
     */
    virtual ~Command();

    /**
     * Initializes command to starting state.
     */
    void initialize();

    /**
     * This method is called repeatedly while the command still needs
     * to be run.
     *
     * @return State The state of the command (STILL_RUNNING until
     * command is done and then typically NORMAL_END). The default
     * implemetation returns NORMAL_END immediately.
     */
    State execute();

    /**
     * Called when the command has ended normally or needs to be stopped.
     *
     * @param reason The reason why the command is ending.
     */
    void end(State reason);

    /**
     * Peforms a full execution cycle on a command (blocks until
     * command is done).
     *
     * @param command The Command object to run through.
     *
     * @param executeRateHz How many times a second the {@link
     * #execute} method should be called.
     */
    static void run(Command& command, int executeRateHz = 20);

  public:

    /**
     * Gets the ASCII name associated with the command.
     *
     * @return Short name to identify the command.
     */
    const std::string& getName() const {
      return _name;
    }

    /**
     * Set the number of seconds which the command is allowed to run.
     *
     * @param The maximum duration that the {@link #execute} method
     * can be called after starting (command should complete before
     * this time).
     */
    void setTimeout(float secs) {
      _timeout = secs;
    }

    /**
     * Get the number of seconds which  the command is allowed to run.
     *
     * @return The maximum number of seconds the command should take
     * to complete.
     */
    float getTimeout() const {
      return _timeout;
    }

    /**
     * Returns the number of seconds elapsed since the command was
     * started or time of last run.
     *
     * @return The number of seconds since the {@link #initialize} was
     * invoked or total time of last run.
     */
    float getElapsedTime() const {
      return _timer.secsElapsed();
    }

    /**
     * Dumps information about the state of the command to standard output.
     *
     * @param out The output stream to write to.
     * @return The output stream written to.
     */
    virtual std::ostream& print(std::ostream& out) const;

    /**
     * Dumps information about the state of the command to a string.
     *
     * @return A string representation (same format as {@link #print}
     * by default).
     */
    virtual std::string toString() const;

    /**
     * Convert the current run state to a human readable string.
     */
    static const std::string& stateToString(State tc);

  private:
    // Disable default constructor
    Command();

    /** Will be null pointer until command is running. */
    Timer _timer;

    /** The name of the command. */
    std::string _name;

    /** Maximum time command ever takes to run. */
    float _timeout;

    /** The current state of the command. */
    State _state;
  };

  //
  // inline methods
  //

  inline std::ostream& operator <<(std::ostream& out, const Command& command) {
    return command.print(out);
  }
}

#endif
