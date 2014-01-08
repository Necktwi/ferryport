RemoteDeviceController
======================

supporting platforms:
linux

tested platforms:
CentOS, ubuntu

Installation instructions:
-------------------------------------------------
1. Copy application folder to desktop.
2. Open config.xml in the application folder with text editor.
3. Set app-name, namespace, server-addr, server-port, stream-addr and stream-port values. Save and close the file.
4. Open terminal.
5. Enter following commands.
6. cd ~/Desktop/remotedevicecontroller
7. make
8. sudo make install


Installation instructions for ubuntu
--------------------------------------------------
1. cd ubuntu-13.04-console-armhf-2013-09-26
2. sudo ./setup_sdcard.sh --probe-mmc
3. sudo ./setup_sdcard.sh --mmc /dev/sdX --uboot bone
4. insert sd card in beaglebone, connect to system and ssh ubuntu@192.168.7.2
5. password: temppwd
6. add "deb http://deb.newmeksolutions.com/ $(ARCH)/" to top of /etc/apt/sources.list without quotes
7. sudo route add default gw 192.168.7.1
8. sudo vi /etc/resolv.conf
9. change nameserver 192.168.1.1 to 192.168.7.1
10. sudo ntpdate ntp.ubuntu.com
11. sudo vi /etc/udev/rules.d/70-persistent-net.rules add at EOF

# USB device 0x0bda:0x8189 (usb)
SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{dev_id}=="0x0", ATTR{type}=="1", KERNEL=="wlan*", NAME="wlan0"


11. sudo vi /etc/network/interfaces  and replace #wifi example section with the below

# WiFi Example
auto wlan0
iface wlan0 inet static
    wpa-ssid "NEWMEK"
    wpa-psk  "ekmaster"
    address 192.168.2.121
    netmask 255.255.255.0
    network 192.168.2.0
#    gateway 192.168.2.1
    dns-nameservers 192.168.2.21 8.8.8.8

11. sudo apt-get update
12. sudo apt-get dist-upgrade
13. sudo apt-get install remotedevicecontroller
14. sudo remotedevicecontroller -i
15. exit, connect wifi modem, connect 3g modem, connect camera, connect gps-device and ssh using wifi
16. ssh ubuntu@192.168.2.121  #TO CHECK WIFI CONNECTION. SKIP IF NOT REQUIRED
17. sudo vi /etc/wvdial.conf
# for reliance netconnect+ modem
[Dialer Defaults]
Init1 = ATZ
Init2 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
Modem Type = Analog Modem
ISDN = 0
New PPPD = yes
Phone = #777
Modem = /dev/CDMAModem
Username = net
Password = net
Baud = 9600

18. sudo init 6


Copying image:
----------------------------------------------------------------------------------------------------------
1. sudo dd if=BBB_img/bbb_mbr.img of=/dev/sdb bs=446 count=1
2. partition /dev/sdb into boot(65MB,fat16) and rootfs(remaining,ext4) using gparted.
3. mount all cards
3. sudo cp -Rrf --preserve=all BBB_img/boot/* /media/boot
4. sudo cp -Rrf --preserve=all BBB_img/rootfs/* /media/rootfs
5. insert sd card, reliance modem in bbb and plug in to computer via usb.
6. connect beaglebone with ssh ubuntu@192.168.7.2
7. sudo vi /etc/wvdial.conf  and change username and password to the phone no. of the modem.
8. sudo wvdial &   and test internet connection with ping www.google.com
9. sudo remotedevicecontroller -i
10. fetch system-id.
11. $ sudo remotedevicecontroller -c
    gps-device
    device_name

