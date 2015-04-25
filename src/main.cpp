#include <BlackGPIO.h>
#include <BlackPWM.h>
#include <iostream>
#include <unistd.h>

int main(int argc, const char** argv) {
	BlackLib::BlackPWM pwm(P8_13);

	pwm.setPeriodTime(1 / 60, BlackLib::milisecond);
	pwm.setDutyPercent(4.0);

	printf("%s\n", pwm.getDutyFilePath().c_str());

	pwm.setRunState(BlackLib::run);

	sleep(15);

	pwm.setRunState(stop);

	return 0;
}
