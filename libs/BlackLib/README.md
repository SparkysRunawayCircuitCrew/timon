# BlackLib Library

This area contains a simple makefile that should allow you to
download, build and install the BlackLib library
(http://blacklib.yigityuce.com/index.html).

To download and build on a BeagleBone Black, you may need to specify
the location of the 4.7 or newer GNU compiler (for stdc++11 support)

```
make CXX=g++-4.7
```

If you are building on a system other than a BeagleBone Black, you
will need to have a cross compiler installed. If your cross compiler
has a prefix of "arm-linux-gnueabihf-", you should be able to type:

```
make
```

If your cross compilation toolset has a different prefix (for example:
"arm-bbb-g++"), then you will need to specify the prefix:

```
make CCPREFIX=arg-bbb-
```

The following targets are available:

* "make clear" - Removes everything (including the downloaded file).

* "make clean" - Removes the intermediate files (but leaves the download and final tar.gz).

* "make" - Attempts to download and build a binary tar.gz file for installing on your BBB

* "sudo make install" - Attempts to build and install the library and header files under the /usr/local area.

* "sudo make uninstall" - Attempts to remove the library and header files from /usr/local

Here is a sample program with instructions on compiling and running that should work once the library has been properly installed on your BBB:

```c++
//
// Example of using BlackLib to read digital input GPIO_22 (Pin 19 on P8 header)
//
// To compile: g++ -o /tmp/gpio22 -lBlackLib bl.cpp
//
// To run:     /tmp/gpio22
//
// Requires version 4.7 of g++ compiler (you might need to specify
// g++-4.7 instead of g++ depending on your system))

#include <BlackGPIO.h>
#include <iostream>

using namespace std;
using namespace BlackLib;

int main(int argc, const char** argv) {
  BlackGPIO myGpio(GPIO_22, input);

  cout << "GPIO_22 state: " << myGpio.getValue() << "\n";
  return 0;
}
```