/**
 * Implmentation of a HBridge class to manipulate a PWM/GPIO controlled motor.
 */

#include "HBridge.h"

using namespace avc;

namespace {
  const float minPower = -1.0;
  const float maxPower = +1.0;
}


HBridge::HBridge(BlackLib::pwmName power, BlackLib::gpioName fwd,
		 BlackLib::gpioName rev, uint64_t periodNanos) :
  pwmPower(power),
  gpioFwd(new BlackLib::BlackGPIO(fwd, BlackLib::output, BlackLib::FastMode)),
  gpioRev(new BlackLib::BlackGPIO(rev, BlackLib::output, BlackLib::FastMode)),
  period(periodNanos),
  curVal(0),
  enabled(false)
{
  disable();
}

HBridge::~HBridge() {
  disable();

  // Normally we would want to clean up these objects, but the BlackLib
  // destructor will unexport them and leave our GPIO objects in a bad state.
  //delete gpioFwd;
  //delete gpioRev;
}

bool HBridge::set(float newVal) {
  if ((newVal < minPower) || (newVal > maxPower)) {
    // Out of range, ignore request
    return false;
  }

  bool fwd = (newVal > 0);
  bool rev = (newVal < 0);
  // BlackLib duty percent is opposite of how I think about it
  float duty = 100.0 * (1 - (rev ? -newVal : newVal));

  if (fwd != fwdState) {
    if (gpioFwd->setValue(fwd ? BlackLib::high : BlackLib::low)) {
      fwdState = fwd;
    } else {
      disable();
      return false;
    }
  }

  if (rev != revState) {
    if (gpioRev->setValue(rev ? BlackLib::high : BlackLib::low)) {
      revState = rev;
    } else {
      disable();
      return false;
    }
  }

  bool ok = (enabled ? pwmPower.setDutyPercent(duty) : enable(duty));
  if (ok == true) {
    curVal = newVal;
  } else {
    // Something bad has happened
    disable();
  }
  return ok;
}

bool HBridge::seek(float newVal, float maxStep) {
  float desiredStep = newVal - curVal;
  if (desiredStep < -maxStep) {
    newVal = curVal - maxStep;
  } else if (desiredStep > maxStep) {
    newVal = curVal + maxStep;
  }
  return set(newVal);
}

void HBridge::disable() {
  enabled = false;
  curVal = 0.0;
  fwdState = false;
  revState = false;
  gpioFwd->setValue(fwdState ? BlackLib::high : BlackLib::low);
  gpioRev->setValue(revState ? BlackLib::high : BlackLib::low);
  pwmPower.setDutyPercent(100.0);

  pwmPower.setRunState(BlackLib::stop);
}

bool HBridge::enable(float dutyPercent) {
  if (enabled == false) {
    enabled = pwmPower.setDutyPercent(100.0) &&
      pwmPower.setPeriodTime(period) &&
      pwmPower.setPolarity(BlackLib::straight) &&
      pwmPower.setDutyPercent(dutyPercent) &&
      pwmPower.setRunState(BlackLib::run);
  }
  return enabled;
}

std::ostream& HBridge::dumpInfo(std::ostream& out) const {
  out << "       Cycle (Hz): " << (1.0e9 / period)
      << "\n    Period (nanos): " << period
      << "\n  Power ([-1, +1]): " << curVal
      << "\n      Forward Gear: " << (fwdState ? "true" : "false")
      << "\n      Reverse Gear: " << (revState ? "true" : "false")
      << "\n";
  return out;
}

