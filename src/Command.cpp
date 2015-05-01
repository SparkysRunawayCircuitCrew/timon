/**
 * Implementation of the Command interface and several supporting classes.
 */

#include "Command.h"

#include <sstream>

using namespace avc;
using namespace std;

//
// Command class
//

Command::~Command() {
}

Command::Command(const std::string& name, float timeout) :
  _name(name),
  _timer(),
  _timeout(timeout),
  _state(NEVER_STARTED)
{
  _timer.zero();
}

void Command::doInitialize() {
}

Command::State Command::doExecute() {
  return NORMAL_END;
}

void Command::doEnd(State reason) {
}


void Command::initialize() {
  _state = STILL_RUNNING;
  _timer.start();
}

Command::State Command::execute() {
  if (_timer.secsElapsed() > getTimeout()) {
    return TIMED_OUT;
  }
  return doExecute();
}

void Command::end(State reason) {
  _timer.pause();
  if (reason == STILL_RUNNING) {
    reason = INTERRUPTED;
  }
  _state = reason;
  doEnd(reason);
}


void Command::run(Command& command, int executeRateHz) {
  Timer timer;
  command.initialize();

  // How long between each execution loop
  int period = 1000000000 / executeRateHz;

  // Assume we will end due to normal termination
  State tc = NORMAL_END;
  float timeout = command.getTimeout();

  while ((tc = command.execute()) == STILL_RUNNING) {
    timer.sleepUntilNextNano(period);
  }
  command.end(tc);
}

std::ostream& Command::print(std::ostream& out) const {
  out << _name << "(state=" << stateToString(_state)
      << ", secsElapsed=" << _timer.secsElapsed();

  return out << ')';
}

std::string Command::toString() const {
  ostringstream obuf;
  print(obuf);
  return obuf.str();
}

const string& Command::stateToString(State tc) {
  static string names[] = {
    "completed-ok", "still-running", "never-started", "was-interrupted", "timed-out", "unknown"
  };
  const int n = sizeof(names) / sizeof(names[0]);
  int idx = (int) tc;
  if (idx < 0 || idx >= n) {
    idx = n - 1;
  }
  return names[idx];
}
