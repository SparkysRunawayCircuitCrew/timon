#include "PixyTokens.h"

#include <iostream>

#include <signal.h>
#include <unistd.h>

using namespace std;

namespace {
  // Flag will be set when user terminates via ^C or uses kill on process
  bool hasBeenInterrupted = false;

  void interrupted(int sig) {
    hasBeenInterrupted = true;
  }
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupted);
  signal(SIGTERM, interrupted);

  PixyTokens pt;

  while (hasBeenInterrupted == false) {
    if (pt.isOpen() == false) {
      if (pt.open()) {
	cout << "Initialized pixy, version: " << pt.getVersion() << "\n";
      } else {
	cerr << "Failed to initialize pixy (will try again in 3 seconds)\n";
	pt.printErrorMessage();
	sleep(3);
      }
    } else if (pt.hasNewData()) {
      int cnt = pt.readData();
      if (cnt < 0) {
	pt.printErrorMessage();
	pt.close();
	sleep(3);
      } else if (cnt > 0) {
	cout << "\nFrame: " << pt.getFrames() << "  Blocks: " << cnt << "\n";
	for (int i = 0; i < cnt; i++) {
	  cout << "  " << pt.formatBlock(i) << "\n";
	}
	// Uncomment to slow down output
	//sleep(1);
      } else {
	usleep(5000);
      }
    }
  }

  pt.close();

  return 0;
}
