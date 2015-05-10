/**
 * Implmentation of GyroBNO055 class to get direction from gyro.
 */

#include "GyroBNO055.h"
#include "Timer.h"

#include <iostream>

using namespace avc;
using namespace std;

namespace {
  // Address of register to read to get chip address
  const uint8_t chipIdAddr = 0x0;

  // ID byte expected to be returned by board
  const uint8_t chipIdByte = 0xa0;

  // Register to set the mode of operation of the board in
  const uint8_t operationModeAddr = 0x3d;

  // Address to write to when triggering configuration change
  const uint8_t sysTriggerAddr = 0x3f;

  // Address of power control register
  const uint8_t powerModeAddr = 0x3e;

  // Set to normal power mode
  const uint8_t powerModeNormal = 0x0;

  // Address of page ID register (not sure what that means)
  const uint8_t pageIdAddr = 0x7;

  // Configuration mode
  const uint8_t configMode = 0x00;

  // 9 DOF mode plus absolute angles
  const uint8_t ndofMode = 0x0C;

  // The 6 bytes containing the euler values (pitch, roll and heading) 
  const uint8_t eulerAddr = 0x1a;

  // The 2 heading registers (LSB, MSB)
  const uint8_t headingAddr = 0x1a;

  bool setMode(BlackLib::BlackI2C& i2c, uint8_t mode) {
    if (!i2c.writeByte(operationModeAddr, mode)) {
      return false;
    }
    Timer::sleepNanos(30000000);
    return true;
  }
}


GyroBNO055::GyroBNO055(BlackLib::i2cName i2cDev, int i2cAddr) :
  i2cGyro(i2cDev, i2cAddr)
{
}

GyroBNO055::~GyroBNO055() {
  if (i2cGyro.isOpen()) {
    i2cGyro.close();
  }
}

bool GyroBNO055::reset() {
  if (i2cGyro.isOpen()) {
    i2cGyro.close();
  }

  i2cGyro.open(BlackLib::ReadWrite);
  if (i2cGyro.fail()) {
    cerr << "Failed to open Gyro I2C device\n";
    return false;
  }

  uint8_t idByte = i2cGyro.readByte(chipIdAddr);
  if (idByte != chipIdByte) {
    cerr << "Is BNO055 chip connected? ID byte returned was: 0x" <<
      std::hex << (idByte & 0xff) << " (expected 0x"
	 << (chipIdByte & 0xff) << ")\n" << std::dec;
    i2cGyro.close();
    return false;
  }

  // Reset to configuration mode (in case it was not in this mode)
  if (!setMode(i2cGyro, configMode)) {
    cerr << "Failed to reset BNO055 to configuration mode\n";
    i2cGyro.close();
    return false;
  }

  // Reset (which may take a bit)
  i2cGyro.writeByte(sysTriggerAddr, 0x20);

  for (int i = 0; i < 10; i++) {
    Timer::sleepNanos(100000000);
    idByte = i2cGyro.readByte(chipIdAddr);
    cerr << "Chip ID reported: 0x" << std::hex
	 << (idByte & 0xff) << " (looking for: 0x"
	 << (chipIdByte & 0xff) << ")\n" << std::dec;

    if (idByte == chipIdByte) {
      cerr << "Chip ID OK after reset\n";
      break;
    }
  }

  if (idByte != chipIdByte) {
    cerr << "Failed to reset BNO055 board\n";
    i2cGyro.close();
    return false;
  }

  // Give 50 more milliseconds for chip to settle
  Timer::sleepNanos(50000000);

  // Set normal power mode
  i2cGyro.writeByte(powerModeAddr, powerModeNormal);
  Timer::sleepNanos(10000000);

  // Configure to use Adafruit added onboard crystal oscillator
  i2cGyro.writeByte(pageIdAddr, 0);
  i2cGyro.writeByte(sysTriggerAddr, 0x80);
  Timer::sleepNanos(10000000);

  if (!setMode(i2cGyro, ndofMode)) {
    cerr << "Failed to reset BNO055 to NDOF mode\n";
    i2cGyro.close();
    return false;
  }
  Timer::sleepNanos(20000000);

  return true;
}

bool GyroBNO055::getHeading(float& angDeg) {
  if (i2cGyro.isOpen() == false) {
    return false;
  }

  uint8_t rawBytes[2];
  int len = i2cGyro.readBlock(eulerAddr, rawBytes, sizeof(rawBytes));

  if (len != sizeof(rawBytes)) {
    cerr << "Failed to read in " << sizeof(rawBytes) << " bytes from BNO055\n";
    i2cGyro.close();
    return false;
  }

  angDeg = (((rawBytes[1] & 0xff) << 8) + (rawBytes[0] & 0xff)) / 16.0;
  return true;
}

