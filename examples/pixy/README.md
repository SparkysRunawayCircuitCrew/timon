# CMUcam5 Pixy on BeagleBone Black

The files contained in this area are related to working with the Pixy
camera on a BeagleBone Black (however, much of what is here will work
on any Linux based system).

## libpixy

The Pixy camera has a USB interface and there is a library that makes
many Pixy functions easily available to C code. This library is
typically installed under /usr/local. You can check to see if the
library is installed using the following _ls_ command.

```sh
root@salsa:~# ls /usr/local/*/*pixy*
/usr/local/include/pixydefs.h  /usr/local/lib/libpixyusb.a
/usr/local/include/pixy.h
root@salsa:~# 
```

If the library is not installed on your system, follow the direction
at the Pixy wiki:

http://cmucam.org/projects/cmucam5/wiki

## Example C++ Code

There is an example C++ wrapper class for the Pixy USB library
functions provided under the pixy-usb directory.

An example of using this class is provided in pixy-usb/pixy-usb.cpp.

If your system is properly configured, you should be able to build and
run this example by typing in the following commands in the same
directory as this README.md file:

```
make && bin/pixy-usb
```


