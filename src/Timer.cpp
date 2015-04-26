/**
 * Implementation of non-inline Timer methods.
 */

#include "Timer.h"
#include <cmath>

using namespace avc;

float Timer::sleep(float seconds) {
  float secsToNanos = 1e9;
  float isecs = std::floor(seconds);
  float nsecs = (seconds - isecs) * secsToNanos;

  timespec sleepTime;
  sleepTime.tv_sec = (time_t) isecs;
  sleepTime.tv_nsec = (long) nsecs;

  timespec timeRemaining;
  timeRemaining.tv_sec = 0;
  timeRemaining.tv_nsec = 0;

  int rc = nanosleep(&sleepTime, &timeRemaining);
  if (rc == 0) {
    //std::cout << "Slept: " << sleepTime.tv_sec << " seconds and "
    //	      << sleepTime.tv_nsec << " nanoseconds\n";
    return 0;
  }

  float secsLeftToSleep = timeRemaining.tv_sec + (timeRemaining.tv_nsec / secsToNanos);

  return secsLeftToSleep;
}

int Timer::sleepNanos(int nsecs) {
  timespec sleepTime;
  sleepTime.tv_sec = (time_t) 0;
  sleepTime.tv_nsec = (long) nsecs;

  timespec timeRemaining;
  timeRemaining.tv_sec = 0;
  timeRemaining.tv_nsec = 0;

  int rc = nanosleep(&sleepTime, &timeRemaining);
  if (rc == 0) {
    return 0;
  }

  int nanosLeftToSleep = timeRemaining.tv_nsec;

  return nanosLeftToSleep;
}

float Timer::diffSecs(const timespec& fromHere, const timespec& toHere) {
  int secDelta = toHere.tv_sec - fromHere.tv_sec;
  int nanoDelta = (int) toHere.tv_nsec - (int) fromHere.tv_nsec;

  float diffSecs = 1e-9;
  diffSecs *= nanoDelta;
  diffSecs += secDelta;
  return diffSecs;
}

int Timer::sleepUntilNextNano(int nanoPeriod) const {
  timespec now;
  if (!Timer::getTime(now)) {
    return nanoPeriod;
  }

  // Get a delta nanosecond from start time
  int nanosDelta = (int) now.tv_nsec - (int) startTime.tv_nsec;
  if (nanosDelta < 0) {
    nanosDelta += 1000000000;
  }

  // Compute how far into the current time period
  int intoTimePeriod = nanosDelta % nanoPeriod;
  int nanoSleepTime = nanoPeriod - intoTimePeriod;

  // Uncomment to see details on sleep computation
  /*
  std::cout
    << "nanosDelta: " << nanosDelta
    << "  intoTimePeriod: " << intoTimePeriod
    << "  nanoPeriod: " << nanoPeriod
    << "  nanoSleepTime: " << nanoSleepTime << "\n";
  */
  return Timer::sleepNanos(nanoSleepTime);
}

float Timer::secsElapsed() const {
  if (running) {
    timespec now;
    Timer::getTime(now);
    return diffSecs(startTime, now);
  }
  return diffSecs(startTime, endTime);
}

std::ostream& Timer::printJson(std::ostream& out) const {
  out << "{ startTime: { epochSecs: " << startTime.tv_sec
      << ", nano: " << startTime.tv_nsec
      << "}, endTime: { epochSecs: " << endTime.tv_sec
      << ", nano: " << endTime.tv_nsec
      << "}, running: " << (running ? "true" : "false")
      << ", secsElapsed: " << secsElapsed()
      << ", clocksPerSec: " << CLOCKS_PER_SEC
      << " }";
  return out;
}



