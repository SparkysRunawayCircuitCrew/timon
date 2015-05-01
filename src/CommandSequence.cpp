/**
 * CommandSequence implementation.
 */

#include "CommandSequence.h"

#include <sstream>

using namespace avc;
using namespace std;

CommandSequence::~CommandSequence() {
  int n = _commands.size();
  for (int i = 0; i < n; i++) {
    delete _commands[i];
  }
}

CommandSequence::CommandSequence(const std::string& name) :
  Command(name),
  _commands(),
  _currentIdx(0)
{
  setTimeout(0);
}

void CommandSequence::add(Command* command) {
  _commands.push_back(command);
  setTimeout(getTimeout() + command->getTimeout());
}

void CommandSequence::doInitialize() {
  _currentIdx = 0;
  _needInitialize = true;
}

Command::State CommandSequence::doExecute() {
  State state = Command::NORMAL_END;
  int n = _commands.size();
  if (_currentIdx >= n) {
    return state;
  }

  do {
    Command* cmd = _commands[_currentIdx];

    if (_needInitialize) {
      _needInitialize = false;
      cmd->initialize();
      cout << "Seq initialized: " << *this << "\n";
    }

    state = cmd->execute();
    if (state == Command::STILL_RUNNING) {
      // cout << "Seq running: " << *this << "\n";
      break;
    }
    cmd->end(state);
    cout << "Seq ended: " << *this << "\n";

    if (state != NORMAL_END) {
      break;
    }

    // Command ended normally, move to next command in list
    _needInitialize = true;
    _currentIdx++;

  } while (_currentIdx < n);

  
  return state;
}

void CommandSequence::doEnd(State reason) {
  _currentIdx = _commands.size();
  _needInitialize = true;
}

std::ostream& CommandSequence::print(std::ostream& out) const {
  //  Command::print(out) << "\n";
  int n = _commands.size();
  out << getName() << '[' << _currentIdx << " of " << n 
      << "]";
  if (_currentIdx < n) {
    out << " = " << *_commands[_currentIdx];
  }
  return out;
}
