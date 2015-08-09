
#pragma once

#include "Command.h"
#include "Timon.h"
#include "Timer.h"

namespace avc {
	class Brake : public Command {

	public:
		Brake(Timon& car, float duration);

		void doInitialize();
		Command::State doExecute();
		void doEnd(Command::State reason);

	private:
		Timon& timon;
		Timer timer;	
		float duration;
	};
}
