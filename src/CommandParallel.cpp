/**
 * CommandParallel implementation.
 */

#include "CommandParallel.h"

#include <sstream>

using namespace avc;
using namespace std;

CommandParallel::~CommandParallel() {
  clear();
}

CommandParallel::CommandParallel(const std::string& name, bool stopWhenOneDone) :
  Command(name),
  _commands(),
  _states(),
  _stopWhenOneDone(stopWhenOneDone)
{
  setTimeout(0);
}

void CommandParallel::add(Command* command) {
  _commands.push_back(command);
  _states.push_back(Command::NEVER_STARTED);
  setTimeout(max(getTimeout(), command->getTimeout()));
}

void CommandParallel::clear() {
  int n = _commands.size();
  for (int i = 0; i < n; i++) {
    delete _commands[i];
  }
  _commands.clear();
  _states.clear();
}

void CommandParallel::doInitialize() {
  int n = _commands.size();
  for (int i = 0; i < n; i++) {
    _states[i] = Command::STILL_RUNNING;
    _commands[i]->initialize();
  }
}

Command::State CommandParallel::doExecute() {
  Command::State worstState = Command::NORMAL_END;

  int n = _commands.size();
  int cntDone = 0;

  for (int i = 0; i < n; i++) {
    // If child command is still running
    if (_states[i] == Command::STILL_RUNNING) {
      // Go execute her periodic code
      State state = _commands[i]->execute();
      _states[i] = state;

      // If terminated, update info and see if we need to stop
      if (state != Command::STILL_RUNNING) {
	cntDone++;
	worstState = Command::worstState(worstState, state);

	if (_stopWhenOneDone) {
	  break;
	}
      }
    }
  }

  return ((cntDone != n) ? Command::STILL_RUNNING : worstState);
}

void CommandParallel::doEnd(State reason) {
  int n = _commands.size();
  for (int i = 0; i < n; i++) {
    _commands[i]->end(reason);
  }
}

std::ostream& CommandParallel::print(std::ostream& out) const {
  //  Command::print(out) << "\n";
  int n = _commands.size();
  int running = 0;
  for (int i = 0; i < n; i++) {
    if (_states[i] == Command::STILL_RUNNING) {
      running++;
    }
  }
  out << getName() << '[' << running << " running of " << n 
      << "]: {";
  for (int i = 0; i < n; i++) {
    if (_states[i] == Command::STILL_RUNNING) {
      out << " " << _commands[i]->getName();
    }
  }
  return out << " }";
}
