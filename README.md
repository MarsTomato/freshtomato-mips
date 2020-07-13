# **FreshTomato-MIPS** #
.  
  
**Forked off from Tomato by Shibby, builds compiled by pedro**
.  
  
For the following **MIPSR1** and **MIPSR2** routers: **Asus** WL500GP, N10U, N12 A1/B1/C1/D1/VP/K/HP, N15U, N16, N53, N66U, AC66U **Netgear** WNR3500LV1, WNR3500LV2, R6300V1, WNDR4500V1, WNDR4500V2, WNDR3400/v2/v3, WNDR3700v3 **Linksys** WRT54 series, E800, E900, E1000v2/v2.1, E1200V1, E1200V2, E1500, E2000, E2500, E3000, E3200, E4200 **Tenda** W1800R, N80 **Dlink** DIR-320 **Belkin** F5D8235v3, F7D3301, F7D3302, F7D4302, F9K1102v1/v3.  
.  
  
***Disclaimer: I am not responsible for any bricked routers, nor do I encourage other people to flash alternative firmwares on their routers. Use at your own risk!***  
.  
  
- [**Project page**](https://freshtomato.org/)
- [**Source code**](https://bitbucket.org/pedro311/freshtomato-mips/commits/all)
- [**Changelog**](https://bitbucket.org/pedro311/freshtomato-mips/src/mips-master/CHANGELOG)
- [**Downloads**](https://freshtomato.org/downloads)
- [**Issue tracker**](https://bitbucket.org/pedro311/freshtomato-mips/issues?status=new&status=open)
- [**Forum EN**](https://www.linksysinfo.org/)
- [**Forum PL**](https://openlinksys.info/forum/)
- **Donations**: [**PayPal**](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=B4FDH9TH6Z8FU)  or  BTC: **`1JDxBBQvcJ9XxgagJRNVrqC1nysq8F8B1Y`**  
  
.  
**HOW TO PREPARE A WORK ENVIRONMENT FOR FRESHTOMATO COMPILATION (on Debian 9.x/64bit or Debian 10.x/64bit)**
  
1. Install Debian via the graphical interface (for simplicity); install the SSH server, choose default [username]; the rest may be the default
  
2. Login as root
  
3. Update system:
    ```sh
    $ apt-get update
    $ apt-get dist-upgrade
    ```
  
4. Install basic packages:
    ```sh
    $ apt-get install build-essential net-tools
    ```
  
5. NOT NECESSARY (depends if sys is on vmware); install vmware-tools:
    ```sh
    $ mkdir /mnt/cd
    $ mount /dev/cdrom /mnt/cd
    ```
    unpack  
    ```sh
    $ ./vmware-install.pl
    ```
    Or from debian package:  
    ```sh
    $ apt-get install open-vm-tools
    ```
  
6. Set proper date/time:
    ```sh
    $ dpkg-reconfigure tzdata
    ```
    In case of problems here:
    ```sh
    $ export PATH=$PATH:/usr/sbin
    ```
  
7. Add your [username] to sudo group:
    ```sh
    $ apt-get install sudo
    $ adduser [username] sudo
    $ reboot
    ```
  
8. Login as [username], install base packages with all dependencies:
    ```sh
    $ sudo apt-get install autoconf m4 bison flex g++ libtool sqlite gcc binutils patch bzip2 make gettext unzip zlib1g-dev libc6 gperf automake groff
    $ sudo apt-get install lib32stdc++6 libncurses5 libncurses5-dev gawk gitk zlib1g-dev autopoint shtool autogen mtd-utils gcc-multilib gconf-editor lib32z1-dev pkg-config libssl-dev automake1.11
    $ sudo apt-get install libmnl-dev libxml2-dev intltool libglib2.0-dev libstdc++5 texinfo dos2unix xsltproc libnfnetlink0 libcurl4-openssl-dev libgtk2.0-dev libnotify-dev libevent-dev git
    $ sudo apt-get install re2c texlive libelf1 nodejs zip mc cmake curl
    $ sudo apt-get install linux-headers-$(uname -r)
    ```
  
9. Remove libicu-dev if it's installed, it stopped PHP compilation:
    ```sh
    $ sudo apt-get remove libicu-dev
    ```
  
10. Remove uuid-dev if it's installed, it stopped miniupnpd compilation:
    ```sh
    $ sudo apt-get remove uuid-dev
    ```
  
11. Install i386 elf1 packages:
    ```sh
    $ sudo dpkg --add-architecture i386
    $ sudo apt-get update
    $ sudo apt-get install libelf1:i386 libelf-dev:i386
    ```
  
12. Clone/download repository:
    ```sh
    $ git clone https://bitbucket.org/pedro311/freshtomato-mips.git
    ```
  
13. Edit .profile (or .bashrc) file, add:
    ```text
    PATH="$PATH:/home/[username]/freshtomato-mips/tools/brcm/hndtools-mipsel-linux/bin"
    PATH="$PATH:/home/[username]/freshtomato-mips/tools/brcm/hndtools-mipsel-uclibc/bin"
    PATH="$PATH:/bin:/sbin:/usr/bin:/usr/X11R6/bin"
    ```
  
14. Reboot system
  
15. Add your email address to git config:
    ```sh
    $ cd freshtomato-mips
    $ git config --global user.email "[email-address]"
    ```
  
16. Add your username to git config:
    ```sh
    $ cd freshtomato-mips
    $ git config --global user.name [name]
    ```
  
.  
**HOW TO COMPILE**
  
1. Change dir to git repository ie: ```$ cd freshtomato-mips```
2. Before every compilation, use ```$ git clean -fdxq && git reset --hard```, and possibly ```$ git pull``` to pull recent changes from remote
3. To compile RT image, use: ```$ git checkout mips-master``` then: ```$ cd release/src-rt```, check for possible targets: ```$ make help```, use one (Mini for ie. WRT54G): ```$ make f```
4. To compile RT-N image, use: ```$ git checkout mips-RT-AC``` then: ```$ cd release/src-rt```, check for possible targets: ```$ make help```, use one (RT-N66U build AIO): ```$ make r64z```
5. To compile RT-AC image, use: ```$ git checkout mips-RT-AC``` then: ```$ cd release/src-rt-6.x```, check for possible targets: ```$ make help```, use one (RT-AC66U build AIO): ```$ make ac66z```
  
.  
**WARNING**
  
To compile n6, n60, rtn53, e2500, e3200, wndr3400v2, wndr3400v2-vpn and f9k targets (RT-N), before compilation you have to revert patch to the kernel:  
```sh
$ cd release/src-rt/linux
$ patch -R -p4 < fix4usbap.patch
```
  