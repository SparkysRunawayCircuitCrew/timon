#/usr/bin/python

# Example of opening serial port UART1 (/dev/ttyO1) on BBB and reading serial
# data from Pixy
#
# /dev/ttyO1 pins on BBB:
#   P9_26 (Rx), P9_24 (Tx), P9_20 (CTS), P9_19 (RTS)
#
# Requires cape_enable=capemgr.enable_partno=BB-UART1 in /boot/uboot/uEnv.txt


import serial

ser = serial.Serial("/dev/ttyO1", baudrate=38400, bytesize=8, parity='N', stopbits=1, timeout=1, xonxoff=0, rtscts=0)

print ser.isOpen()
n = ser.inWaiting()
print n
b = ser.read(n)

for i in range(0, n):
  print b[i]

ser.close()
