# FreshTomato MIPS

> Alternative open-source firmware for Broadcom MIPS-based routers.

## Supported routers

FreshTomato MIPS supports both **MIPSR1** and **MIPSR2** devices.

| Manufacturer | Models |
|---|---|
| **ASUS** | WL500GP, N10U, N12 A1/B1/C1/D1/VP/K/HP, N15U, N16, N53, N66U, AC66U |
| **Netgear** | WNR3500Lv1, WNR3500Lv2, R6300v1, WNDR4500v1, WNDR4500v2, WNDR3400/v2/v3, WNDR3700v3, WNDR4000 |
| **Linksys** | WRT54 series, E800, E900, E1000v2/v2.1, E1200v1, E1200v2, E1500, E2000, E2500, E3000, E3200, E4200 |
| **Tenda** | W1800R, N80 |
| **D-Link** | DIR-320, DIR-865L |
| **Belkin** | F5D8235v3, F7D3301, F7D3302, F7D4302, F9K1102v1/v3 |

> [!CAUTION]
> Flashing alternative firmware can permanently damage your router. The author is not responsible for bricked devices. Proceed entirely at your own risk.

## Project resources

| Resource | Link |
|---|---|
| Project website | [freshtomato.org](https://freshtomato.org/) |
| Source code | [GitHub](https://github.com/FreshTomato-Project/freshtomato-mips) · [GitLab mirror](https://gitlab.com/pedro311/freshtomato-mips) |
| Changelog | [CHANGELOG](https://github.com/FreshTomato-Project/freshtomato-mips/blob/mips-master/CHANGELOG) |
| Downloads | [Download images](https://freshtomato.org/downloads) |
| Issue tracker | [GitHub Issues](https://github.com/FreshTomato-Project/freshtomato-mips/issues) |
| Pull requests | [GitHub Pull Requests](https://github.com/FreshTomato-Project/freshtomato-mips/pulls) |
| English forum | [LinksysInfo](https://www.linksysinfo.org/index.php?forums/tomato-firmware.33/) |
| Polish forum | [OpenLinksys](https://openlinksys.info/forum/) |
| Donations | [Support FreshTomato](https://freshtomato.org/donations.html) |

## Preparing the build environment

The following instructions target **Debian 13 (64-bit)**.

### 1. Install Debian

Install Debian using the graphical installer. For simplicity:

- enable the SSH server;
- create a standard user account;
- keep the remaining options at their defaults.

In the commands below, replace `<username>`, `<name>`, and `<email-address>` with your own values.

### 2. Update the system

Log in as `root`, then run:

```sh
apt-get update
apt-get dist-upgrade
```

### 3. Install basic packages

```sh
apt-get install build-essential net-tools
```

### 4. Configure the time zone

```sh
dpkg-reconfigure tzdata
```

If the command is unavailable because of the current `PATH`, run:

```sh
export PATH="$PATH:/usr/sbin"
```

### 5. Grant sudo access

```sh
apt-get install sudo
adduser <username> sudo
reboot
```

After the reboot, log in as `<username>`.

### 6. Install build dependencies

```sh
sudo apt-get install \
  autoconf autoconf-archive m4 bison flex g++ libtool gcc binutils patch \
  bzip2 make gettext unzip zlib1g-dev libc6 gperf automake groff minisign

sudo apt-get install \
  lib32stdc++6 libncurses6 libncurses-dev gawk gitk zlib1g-dev autopoint \
  shtool autogen mtd-utils gcc-multilib lib32z1-dev pkg-config libssl-dev \
  automake

sudo apt-get install \
  libmnl-dev libxml2-dev intltool libglib2.0-dev texinfo dos2unix xsltproc \
  libnfnetlink0 libcurl4-openssl-dev libgtk2.0-dev libnotify-dev \
  libevent-dev git

sudo apt-get install \
  re2c texlive libelf1 nodejs zip mc cmake ninja-build curl \
  libglib2.0-dev-bin libglib2.0-dev sqlite3 dconf-editor python3-dev \
  python3-setuptools

sudo apt-get install "linux-headers-$(uname -r)"
```

### 7. Clone the repository

```sh
git clone https://github.com/FreshTomato-Project/freshtomato-mips.git
```

### 8. Reboot

```sh
sudo reboot
```

### 9. Configure Git

```sh
cd freshtomato-mips
git config --global user.email "<email-address>"
git config --global user.name "<name>"
```

## Compiling FreshTomato

### Prepare the repository

Enter the repository:

```sh
cd freshtomato-mips
```

Before each compilation, clean and reset the working tree:

```sh
git clean -fdxq
git reset --hard
```

Optionally, download the latest changes:

```sh
git pull
```

> [!WARNING]
> `git clean -fdxq` and `git reset --hard` permanently remove untracked files and local changes.

### RT image

Example: build a **Mini** image for a WRT54G-class device.

```sh
git checkout mips-master
cd release/src-rt
make help
make f
```

### RT-N image

Example: build an AIO image for **RT-N66U**.

```sh
git checkout mips-RT-AC
cd release/src-rt
make help
make r64z
```

### RT-AC image

Example: build an AIO image for **RT-AC66U**.

```sh
git checkout mips-RT-AC
cd release/src-rt-6.x
make help
make ac66z
```

`make help` lists the build targets available in the selected source tree.
