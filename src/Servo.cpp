/**
 * Implmentation of a Servo class to manipulate a PWM controlled servo.
 */

#include "Servo.h"

using namespace avc;

namespace {
  // How many pulses per second (from Wikipedia/net seems like 40 - 60 range
  // is OK).
  const uint32_t servoHz = 50;

  // Servos expect 60 Hz (60 times per billion nanoseconds)
  const uint64_t servoPeriodNanos = (1000000000 / 50);

  // Servo expects pulses to be 1 millisecond to 2 millisecond with
  // 1.5 milliseconds being "center"
  const uint64_t servoMinNanos = 1000000;
  const uint64_t servoMaxNanos = 2000000;

  // Now that we have the time of the full period and the range
  // of times we want the pulse high, we can set our duty percent range limits
  const float minDutyPercent = (100.0 * servoMinNanos) / servoPeriodNanos;
  const float maxDutyPercent = (100.0 * servoMaxNanos) / servoPeriodNanos;
  const float dutyPercentRange = (maxDutyPercent - minDutyPercent);

}


Servo::Servo(BlackLib::pwmName name, float minValue, float maxValue) :
  pwm(name),
  curVal((minValue + maxValue) / 2),
  minVal(minValue),
  maxVal(maxValue),
  // Compute scale factor (slope) when converting from angle to duty percent
  m((maxDutyPercent - minDutyPercent) / (maxValue - minValue)),
  enabled(false)
{
  
}

Servo::~Servo() {
  disable();
}

bool Servo::set(float newVal) {
  if ((newVal < minVal) || (newVal > maxVal)) {
    // Out of range, ignore request
    return false;
  }

  float dutyPercent = 100.0 - ((newVal - minVal) * m + minDutyPercent);
  bool ok = (enabled ? pwm.setDutyPercent(dutyPercent) : enable(dutyPercent));

  /*
  std::cout
    << "newVal: " << newVal
    << "  minVal: " << minVal
    << "  maxVal: " << maxVal
    << "  dutyPercent: " << (100.0 - dutyPercent)
    << "  duty: " << pwm.getDutyValue() << "\n";
  */

  if (ok == false) {
    // Something bad has happened
    disable();
  }

  return ok;
}

bool Servo::enable(float dutyPercent) {
  if (enabled == false) {
    enabled = pwm.setPeriodTime(servoPeriodNanos) &&
      pwm.setPolarity(BlackLib::straight) &&
      pwm.setDutyPercent(dutyPercent) &&
      pwm.setRunState(BlackLib::run);
  }
  return enabled;
}

std::ostream& Servo::dumpInfo(std::ostream& out) const {
  out << "       Cycle (Hz): " << (1.0e9 / servoPeriodNanos)
      << "\n    Period (nanos): " << servoPeriodNanos
      << "\n  Min Duty (nanos): " << (servoPeriodNanos * minDutyPercent / 100)
      << " (" << minDutyPercent << "%)"
      << "\n  Cen Duty (nanos): " << (servoPeriodNanos * ((maxDutyPercent + minDutyPercent) / 200))
      << " (" << ((minDutyPercent + maxDutyPercent) / 2) << "%)"
      << "\n  Max Duty (nanos): " << (servoPeriodNanos * maxDutyPercent / 100)
      << " (" << maxDutyPercent << "%)"
      << "\n         Min Value: " << minVal
      << "\n         Cen Value: " << ((minVal + maxVal) / 2)
      << "\n         Max Value: " << maxVal
      << "\n";
  return out;
}

