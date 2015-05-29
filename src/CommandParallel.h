/**
 * CommandParallel definition.
 */
#ifndef __avc_CommandParallel_h
#define __avc_CommandParallel_h

#include "Command.h"

#include <vector>

namespace avc {

  /**
   * Class which can execute 0 or more child Command objects in parallel.
   */
  class CommandParallel : public Command {
  public:
    CommandParallel(const std::string& name, bool stopWhenOneDone = false);
    ~CommandParallel();

    void add(Command* command);

    void clear();

  protected:
    void doInitialize();
    State doExecute();
    void doEnd(State reason);
    std::ostream& print(std::ostream& out) const;

  private:
    std::vector<Command*> _commands;
    std::vector<Command::State> _states;
    bool _stopWhenOneDone;
  };
}

#endif
