/**
 * To compile:
 * 
 *
   g++-4.7 -std=c++11 -o /tmp/read_uart -lBlackLib read_uart.cpp 
 *
 * To run:
 *
 *   /tmp/read_uart
 */

#include <unistd.h>
#include <iostream>
#include <BlackUART.h>

using namespace BlackLib;
using namespace std;

int main() {
  BlackUART gyro(UART1,
		 Baud38400,
		 ParityNo,
		 StopOne,
		 Char8);

  gyro.open(ReadWrite | NonBlock);
  
  char readBuffer[100];

  while(1) {
    cout << gyro.read() << "\n";
    usleep(50);
  }
  gyro.close();
  return 0;
}
