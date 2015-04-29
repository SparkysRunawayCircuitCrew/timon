/**
 * Implementation of UserLeds class.
 */

#include "UserLeds.h"

#include <iostream>
#include <fstream>

using namespace avc;
using namespace std;

namespace {
  string SYS_PATH_PREFIX("/sys/class/leds/beaglebone:green:usr");
  char ledToChar[] = { '0', '1', '2', '3' };

  inline bool isLedOk(int led) {
    return ((unsigned) led < 4);
  }

  inline bool isBitSet(int bits, int bit) {
    return ((bits >> bit) & 1) == 1;
  }

  inline void setBit(int& bits, int bit) {
    bits |= (1 << bit);
  }

  inline void clearBit(int& bits, int bit) {
    bits &= ~(1 << bit);
  }

  bool writeSysFile(int led, const string& fileName, const string& value) {
    if (!isLedOk(led)) {
      return false;
    }

    string path(SYS_PATH_PREFIX);
    path += ledToChar[led];
    path += '/';
    path += fileName;

    ofstream out(path, std::ofstream::out);
    bool ok = out.write(value.c_str(), value.size());
    out.close();

    return ok;
  }
}

// This is the single instance
UserLeds UserLeds::instance;

UserLeds::UserLeds() :
  initialized(0),
  state(0) {
}

bool UserLeds::setState(int newState) {
  bool ok = true;
  for (int i = 0; i < 4; i++) {
    ok = ok && setLed(i, (newState & 1) == 1);
    newState >>= 1;
  }

  return ok;
}

bool UserLeds::setLed(int led, bool turnOn) {
  bool ok = isLedOk(led);
  if (ok) {
    bool force = false;

    // See if we still need to initialize
    if (isBitSet(initialized, led) == false) {
      setBit(initialized, led);
      // Take control of user LED
      ok = writeSysFile(led, "trigger", "none");
      force = true;
    }

    // See if we need to update the system file
    if (ok && (force || (isBitSet(state, led) != turnOn))) {
      string fileName("brightness");

      if (turnOn) {
	      setBit(state, led);
	      ok = writeSysFile(led, fileName, "1");
      } else {
	      clearBit(state, led);
	      ok = writeSysFile(led, fileName, "0");
      }
    }
  }
  return ok;
}


bool UserLeds::isLedOn(int led) const {
  bool on = isLedOk(led) && isBitSet(state, led);
  return on;
}


