/**
 * To compile:
 * 
 *   g++-4.7 -o /tmp/blink-led -lBlackLib blink-led.cpp
 *
 * To run:
 *
 *   /tmp/blink-led
 */

#include <BlackGPIO.h>

#include <unistd.h>

using namespace BlackLib;

int main(int argc, char** argv) {
  // P9 pin 11
  BlackGPIO led1(BlackLib::GPIO_30, BlackLib::output, BlackLib::FastMode);
  // P9 pin 12
  BlackGPIO led2(BlackLib::GPIO_60, BlackLib::output);

  //
  // Using pointers
  //
  // P9 pin 13
  BlackGPIO* led3 = new BlackGPIO(BlackLib::GPIO_31, BlackLib::output, BlackLib::FastMode);
  // P9 pin 15
  BlackGPIO* led4 = new BlackGPIO(BlackLib::GPIO_48, BlackLib::output);

  for (int i = 0; i < 10; i++) {
    led1.setValue((i & 1) == 0 ? BlackLib::high : BlackLib::low);
    led2.setValue((i & 1) == 0 ? BlackLib::low : BlackLib::high);

    led4->setValue((i & 1) == 0 ? BlackLib::high : BlackLib::low);
    led3->setValue((i & 1) == 0 ? BlackLib::low : BlackLib::high);
    sleep(1); 
  }

  // Turn off all four LEDs
  led1.setValue(BlackLib::low);
  led2.setValue(BlackLib::low);
  led3->setValue(BlackLib::low);
  led4->setValue(BlackLib::low);

  // When you use new, you allocate memory and are responsible for deleting the
  // objects when done.

  // HOWEVER, the BlackLib implementation "unexports" the pin settings in the
  // destructor which can leave the pins in a random/unpredictable state upon
  // exit. If we don't delete the objects, we can avoid this unexport issue,
  // but at the cost of a memory leak (which is only an issue if you continually
  // create the objects in your code)
  //
 
 // If you don't delete these, you should see /sys/class/gpio/gpio31
 // and /sys/class/gpio/gpio48 left in the filesystem were as the
 // non-pointer entries gpio 30 and 60 will be cleaned up (removed) by
 // the led1 and led2 destructors.

  //delete led3;
  //delete led4;
}
