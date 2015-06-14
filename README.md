# timone
R-ROBOT

our rbot.

## BeagleBone Black (BBB) Set Up

* You will need at least 4 GB of disk (this means if you have an old
  Rev B board with a 2GB MMC, you will need to boot from a micro SD
  card).

* This build was done on the standard Debian (wheezy) distribution
  that ships with the Rev C boards. You will need to make adjustments
  if you are using a different distribution.

* The following packages will need to be apt-get installed:
  * unzip
  * g++-4.7 (the stock g++ on wheezy is 4.6)
  * chkconfig

```sh
apt-get install unzip g++-4.7 chkconfig
```

* You will need to build/install the BlackLib libraries:

```sh
make -C libs/BlackLib CXX=g++-4.7 install
```

* If we use the Pixy camera, you will need to build/install the Pixy
  libraries (see the README.md file in the examplws/pixy-usb
  directory). We are still deciding on this.

* At this point you should be able to build and install the Autonomous
  Vehicle Competition code using the following commands:

```sh
make -C src
sudo make -C src install
```

* NOTE: The installation does NOT install the necessary device tree
  overlay by default, you must run the following build rule to install
  the device tree overlay (and you will need to reboot after
  installing):

```
make -C src dts
sudo make -C src dts-install
```

* The device tree overlay file for the BBB will be installed if
  required when the "avc" service is started (either at boot or using
  the service command). You can use the following to check and see if
  the timon_gpio overlay is loaded (or load it if not)

```
sudo -i
SLOTS=/sys/devices/bone_capemgr.9/slots
grep timon-gpio ${SLOTS} || echo timon-gpio > ${SLOTS}
cat ${SLOTS}
```

* NOTE: The installation will install a copy of the code at
  /usr/sbin/avc AND a start up script (/etc/init.d/avc) that will run
  the code at boot time. If you want to disable the code from
  launching at boot time, run the following to change the start up
  behavior and verify the new settings:

```sh
chkconfig avc off
chkconfig --list avc
```

* Alternatively, if you want to disable and uninstall, you can run:

```sh
make -C src uninstall
```

* When the program is installed and running, it waits for the start
  button on the robot to be pressed and released before launching into
  the autonmous driving code.

* Any output from the autonomous program (when run as a service) can
  be found in the /var/log/avc.log file. This file is cleared
  everytime you restart the service (or reboot the machine).

## Startup Sequence
1. Battery power must be connected, motors should be UNPLUGGED.
2. Flip power switch, wait for BBB to boot up.
3. Ordered LED sequence indicates ready state.
4. Plug in motors
5. Two push bottons on BBB are two auton modes

## Shutdown Sequence
1. Disconnect motors.
2. Hit tiny "power" button on BBB. (One press, if no shutdown in ~1 minute, hold for 10 secs.)
3. Flip power switch.

## How to Charge the Battery

1. Connect red lead of left side of charger into the +12V terminal of the Power Supply
2. Connect black lead of left side of charger into the GND terminal of the Power Supply
3. Connect the right charger leads to the battery, matching the colors
4. Plug the colorful wires of the battery into the charger, into the port that fits
5. Plug power supply into the wall to power on everything. The complete circuit diagram should look like this:

```
   |x|
   x_x
   | |
   | |  _________________________
   | |_|          o    o    o----|--                  _______________________                   ______________________
   |___|         +3.3 +5   +12   | |                 |     ______________    |--------><-------|                      |
       |   O      o--  o    o    | |                 |    |              |   |--------><-------|    I AM A BATTERY    |
       |         GND| -5   -12   | |                 |    |______________|   |                 |                      |
       |            |            | |________\/_______|                       |________\/_______|                      |
       | | | | | | ||| | | | | | |          /\       |                       |        /\       |                      |
       |____________|____________|  ________\/_______|    (||) (v) (^) (o)   |________\/_______|                      |
                    |______________|        /\       |_______________________|        /\       |______________________|
``` 
6. Press the increment (^) button on the charger so that the word "Balance" shows up on the screen
7. Press and hold the green Enter/Start on the charger
8. If the screen says R: 4SER  S: 4SER you are good to go. The charger expected 4 cells in the battery and found 4 cells
9. Press the green Enter/Start once more to start charging the battery
10. When it is finished, the charger will beep at you
11. Unplug Power Supply from the wall
12. Disconnect the other stuff
