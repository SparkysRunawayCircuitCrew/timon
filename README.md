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
