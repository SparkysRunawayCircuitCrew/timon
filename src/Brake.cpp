
#include "Brake.h"

using namespace avc;

Brake::Brake(Timon& car, float dur) 
	: Command("Brake", dur + 1), timon(car), duration(dur) {}

void Brake::doInitialize() { 
	timer.start();
}

Command::State Brake::doExecute() {
	timon.brake();

	if (timer.secsElapsed() > duration) {
		return Command::State::NORMAL_END;
	}

	return Command::State::STILL_RUNNING;
}

void Brake::doEnd(Command::State reason) {
	timon.coast();
}
