#!/bin/bash
#
# Sets up Linux system so BeagleBone Black connected via USB can reach
# the Internet.
#
# Usage:
#
#   sudo bbb-usb-to-internet.bash [OUT_INTERFACE] [BBB_INTERFACE]
#


# Try to guess at appropriate interfaces based on routing table
eval $(/usr/sbin/route -n | awk -- '
$1 == "0.0.0.0" { printf("outDev=%s\n", $8); }
$1 == "192.168.7.0" { printf("usbDev=%s\n", $8); }
')

# Interface on laptop that can reach the Internet
declare outDev="${1:-$outDev}";

# USB interface used to communicate with BBB
declare usbDev="${2:-$usbDev}";

# Make sure two interfaces are found
declare ok="true";
for net in "${outDev}" "${usbDev}"; do
  if ! grep -q "^${outDev}:" /proc/net/dev; then
    ok="false";
    echo -e "\n***ERROR*** Failed to find network device \"${net}\"\n";
  fi
done

if [ "${ok}" != "true" ]; then
  echo -e "
Usage:

  $0 [NET_INTERFACE [BBB_USB_INTERFACE]]

Example:

  $0 eth0 usb0

NOTE: We will try to automatically determine the interfaces from
the output of 'route -n'. If that fails, you may need to determine
these interface names yourself.
";
  exit 1;
fi

#
# Things look OK at this point, set up system to do NAT for BBB
#

echo "Setting up iptable rules to do NAT for BBB ...";
sudo iptables --table nat --delete POSTROUTING --out-interface ${outDev} -j MASQUERADE &>/dev/null
sudo iptables --table nat --append POSTROUTING --out-interface ${outDev} -j MASQUERADE;
sudo iptables --delete FORWARD --in-interface ${usbDev} -j ACCEPT &>/dev/null;
sudo iptables --append FORWARD --in-interface ${usbDev} -j ACCEPT;

echo "Enabling IP forwarding ...";
sudo sh -c 'echo 1 > /proc/sys/net/ipv4/ip_forward';

# Now that NAT has been set up on desktop/laptop, update routing and
# name resolving on BBB

echo "Adding route to BBB ..."
ssh root@192.168.7.2 route add -net 0.0.0.0/0 gw 192.168.7.1 usb0 &>/dev/null;
echo "Installing /etc/resolv.conf on BBB ...";
scp /etc/resolv.conf root@192.168.7.2:/etc/resolv.conf;
echo "Updating time on BBB ...";
ssh root@192.168.7.2 ntpdate 0.fedora.pool.ntp.org;

echo "All done";
