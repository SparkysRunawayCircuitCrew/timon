/**
 * CommandSequence definition.
 */
#ifndef __avc_CommandSequence_h
#define __avc_CommandSequence_h

#include "Command.h"

#include <vector>

namespace avc {

  class CommandSequence : public Command {
  public:
    CommandSequence(const std::string& name);
    ~CommandSequence();

    void add(Command* command);

  protected:
    void doInitialize();
    State doExecute();
    void doEnd(State reason);
    std::ostream& print(std::ostream& out) const;

  private:
    std::vector<Command*> _commands;
    int _currentIdx;
    bool _needInitialize;
  };
}

#endif
