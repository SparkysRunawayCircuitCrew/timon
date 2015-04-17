/**
 * PixyTokens implementation.
 */

#include "PixyTokens.h"

using namespace std;

PixyTokens::PixyTokens(int mb) :
  status(PIXY_ERROR_USB_NOT_FOUND),
  versionMaj(0),
  versionMin(0),
  versionBuild(0),
  frames(0),
  maxBlocks(mb),
  numBlocks(0),
  blocks(0)
{
  blocks = new Block[maxBlocks];
}

PixyTokens::~PixyTokens() {
  delete[] blocks;
}

string PixyTokens::getVersion() const {
  char buf[200];
  sprintf(buf, "%d.%d.%d", versionMaj, versionMin, versionBuild);
  return buf;
}

void PixyTokens::printErrorMessage() const {
  pixy_error(status);
}

bool PixyTokens::open() {
  int initStatus = pixy_init();

  if (initStatus == 0) {
    uint16_t major;
    uint16_t minor;
    uint16_t build;

    int firmwareStatus = pixy_get_firmware_version(&major, &minor, &build);

    if (firmwareStatus == 0) {
      versionMaj = major;
      versionMin = minor;
      versionBuild = build;
    }
    status = firmwareStatus;
  } else {
    status = initStatus;
  }
  return isOpen();
}

void PixyTokens::close() {
  status = PIXY_ERROR_USB_NOT_FOUND;
  pixy_close();
}

bool PixyTokens::hasNewData() {
  int rc = pixy_blocks_are_new();
  return (rc != 0);
}

int PixyTokens::readData() {
  int cnt = pixy_get_blocks(maxBlocks, blocks);
  if (cnt < 0) {
    numBlocks = 0;
    status = cnt;

    printf("pixy_get_blocks(): ");
    pixy_error(cnt);

    return -1;
  }
  frames++;
  numBlocks = cnt;
  return cnt;
}
