# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# To rebuild everything and all configurations:
#  make distclean
#  make V1=whatever V2=sub-whatever VPN=vpn3.6 X
#
# The 1st "whatever" would be the build number - use only when you want the build number to be added 
# to the tomato version; the sub-whatever would be the version note (both optional).
#
# Example:
# make V1=8516 V2="my_build" X
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


export ac_cv_func_malloc_0_nonnull=yes
export SRCBASE := $(shell pwd)
export SRCBASEDIR := $(shell pwd | sed 's/.*release\///g')
RELEASEDIR := $(shell (cd $(SRCBASE)/.. && pwd -P))
CC_LINUX_DIR  ?= $(abspath ../../tools/brcm/hndtools-mipsel-linux/bin/)
CC_UCLIBC_DIR ?= $(abspath ../../tools/brcm/hndtools-mipsel-uclibc/bin/)
export PATH := $(RELEASEDIR)/tools:$(PATH):$(CC_LINUX_DIR):$(CC_UCLIBC_DIR)

include target.mak

export TPROFILE := N
V1 ?= "--def"
VPN ?= "VPN"
PPTPD ?= "n"
KERN_SIZE_OPT ?= y

# Asus firmware version in the TRX header (prevents downgrading to an incompatible firmware at AsusWRT)
FORCE_SN = 386
FORCE_EN = 40000

ifeq ($(NVRAM_SIZE),)
 NVRAM_SIZE = 0
endif

ifeq ($(ASUS_TRX),)
 ASUS_TRX = 0
endif

ifeq ($(WNR3500LV2),)
 WNR3500LV2 = 0
else
 WNRTOOL=$(SRCBASE)/wnrtool
 BOARD_FILE=$(WNRTOOL)/comp-wnr3500lv2.txt
 fw_cfg_file := $(WNRTOOL)/ambitCfg-wnr3500lv2.h
endif

ifeq ($(NVRAM_64K),y)
 EXTRA_64KDESC = ' -64K'
 EXTRA_64KCFLAG = '-DTCONFIG_NVRAM_64K'
else
 EXTRA_64KDESC = ''
 EXTRA_64KCFLAG = ''
endif

-include tomato_profile.mak

# This could be simpler by just using $(TOMATO_PROFILE_NAME) like it used to be,
# but that's fragile.  If you make one certain innocuous change elsewhere in the
# makefile(s), the build will silently be wrong. This way it works properly every time.
current_BUILD_NAME = $(strip $(shell grep "^TOMATO_BUILD_NAME" tomato_profile.mak | cut -d"=" -f2))
current_BUILD_DESC = $(strip $(shell grep "^TOMATO_BUILD_DESC" tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_BUILD_USB  = $(strip $(shell grep "^TOMATO_BUILD_USB"  tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_TOMATO_VER = $(strip $(shell grep "TOMATO_MAJOR" router/shared/tomato_version.h | cut -d"\"" -f2)).$(strip $(shell grep "TOMATO_MINOR" router/shared/tomato_version.h | cut -d"\"" -f2))$(if $(filter $(V1),"--def"),,.$(strip $(shell grep -w "TOMATO_BUILD" router/shared/tomato_version.h | cut -d"\"" -f2)))

uppercase_N = $(shell echo $(N) | tr a-z A-Z)
lowercase_N = $(shell echo $(N) | tr A-Z a-z)
uppercase_B = $(shell echo $(B) | tr a-z A-Z)
lowercase_B = $(shell echo $(B) | tr A-Z a-z)

beta = $(if $(filter $(TOMATO_EXPERIMENTAL),1),-beta,)

revision = $(if $(filter $(MIPS32),r2),R2,R1)
ifneq ($(CONFIG_RT),)
 ND = "RT"
else
 ifeq ($(CONFIG_BCMWL6),y)
  ND = "RTAC"
 else
  ifeq ($(CONFIG_BCMWL5),y)
   ND = "RTN"
  endif
 endif
endif
branch_rev = $(shell echo K26MIPS$(revision)_$(ND))

ifneq ($(V2),)
	current_V2 = $(shell echo -$(V2))
else
	current_V2 = ""
endif

# prepare variables for filename
fn_BUILD_USB = $(if $(filter $(current_BUILD_USB),"USB"),$(shell echo -$(current_BUILD_USB)))
fn_NVRAM_SIZE = $(if $(filter $(NVRAM_SIZE),0),,$(shell echo -NVRAM$(NVRAM_SIZE)K))

tomato_ver:
	@echo ""
	@btools/uversion.pl --gen $(V1) $(branch_rev) $(beta) $(V2) $(current_BUILD_USB) $(current_BUILD_DESC)

include build.mak

all: tomato_ver build_all
	@echo ""
	@echo "-----------------"
	@echo `cat router/shared/tomato_version` " ready"
	@echo "-----------------"
	@echo "\033[41;1m   FreshTomato build done! \033[0m\033]2;FreshTomato build done!\007"

clean:
	@touch router/.config
	@rm -f router/config_[a-z]
	@rm -f router/busybox/config_[a-z]
	@$(MAKE) -C router $@
	@-rmdir router/mipsel-uclibc

cleanimage:
	@rm -f fpkg.log
	@rm -fr image/*
	@rm -f router/.config
	@touch router/.config 
	@-mkdir image

cleantools:
	@$(MAKE) -C $(LINUXDIR)/scripts/squashfs clean
	@$(MAKE) -C btools clean

cleankernel:
	@cd $(LINUXDIR) && \
	mv .config save-config && \
	$(MAKE) distclean || true; \
	cp -p save-config .config || true

kernel:
	@$(MAKE) -C router kernel
	@ls -l $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/zImage

distclean: clean cleanimage cleankernel cleantools
ifneq ($(INSIDE_MAK),1)
	@$(MAKE) -C router $@ INSIDE_MAK=1
endif
	mv router/busybox/.config busybox-saved-config || true
	@$(MAKE) -C router/busybox distclean
	@rm -f router/busybox/config_current
	@cp -p busybox-saved-config router/busybox/.config || true
	@cp -p router/busybox/.config  router/busybox/config_current || true
	@rm -f router/config_current
	@rm -f router/.config.cmd router/.config.old router/.config
	@rm -f router/libfoo_xref.txt
	@rm -f tomato_profile.mak router/shared/tomato_profile.h
	@touch tomato_profile.mak
	@touch router/shared/tomato_profile.h

prepk:
	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_base config_current ; \
		cp -f config_current .config
	$(MAKE) -C $(LINUXDIR) oldconfig

what:
	@echo ""
	@echo "$(current_BUILD_DESC)-$(current_BUILD_NAME)-$(TOMATO_PROFILE_NAME) Profile"
	@echo ""

# The methodology for making the different builds is to
# copy the "base" config file to the "target" config file in
# the appropriate directory, and then edit it by removing and
# inserting the desired configuration lines.
# You can't just delete the "whatever=y" line, you must have
# a "...is not set" line, or the make oldconfig will stop and ask
# what to do.

# Options for "make bin" :
# BUILD_DESC (Std|Lite|Ext|...)
# MIPS32 (r2|r1)
# WNR3500LV2 (0|1)
# KERN_SIZE_OPT KERN_SIZE_OPT_MORE
# USB ("USB"|"")
# JFFSv1 | NO_JFFS
# NO_CIFS, NO_SSH, NO_ZEBRA, NO_SAMBA, NO_HTTPS, NO_XXTP, NO_LIBOPT, NO_FTP, NO_BLINK
# SAMBA3, OPENVPN, DONT_OPTIMIZE_SIZE, OPTIMIZE_SIZE_MORE, IPV6SUPP, EBTABLES, NTFS, UFSD, MEDIASRV, BBEXTRAS, USBEXTRAS, BCM57, SLIM, NOCAT, CTF, NGINX, NANO, BCMNAT, PROXY
# NFS, BTCLIENT, BTGUI, TR_EXTRAS, SNMP, SDHC, HFS, UPS, DNSCRYPT, STUBBY, PPTPD, TOR, IPSEC, RAID, MICROSD, USBAP, NO_USBAPP, IPERF, KEYGEN, NO_KEYGEN, TERMLIB, ADVTHEMES, BRCM_NAND_JFFS2, BONDING, MDNS, SDK6MIPS, ROAM

define RouterOptions
	@( \
	sed -i "/TCONFIG_EMF/d" $(1); \
	if [ "$(SLIM)" = "y" ]; then \
		echo "# TCONFIG_EMF is not set" >>$(1); \
	else \
		echo "TCONFIG_EMF=y" >>$(1); \
	fi; \
	sed -i "/TCONFIG_JFFSV1/d" $(1); \
	if [ "$(JFFSv1)" = "y" ]; then \
		echo "TCONFIG_JFFSV1=y" >>$(1); \
	else \
		echo "# TCONFIG_JFFSV1 is not set" >>$(1); \
	fi; \
	if [ "$(IPSEC)" = "y" ]; then \
		echo "TCONFIG_IPSEC=y" >>$(1); \
	else \
		echo "# TCONFIG_IPSEC is not set" >>$(1); \
	fi; \
	if [ "$(RAID)" = "y" ]; then \
		echo "TCONFIG_RAID=y" >>$(1); \
	else \
		echo "# TCONFIG_RAID is not set" >>$(1); \
	fi; \
	if [ "$(MIPS32)" = "r2" ]; then \
		echo "TCONFIG_MIPSR2=y" >>$(1); \
	else \
		echo "# TCONFIG_MIPSR2 is not set" >>$(1); \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		sed -i "/TCONFIG_USB is not set/d" $(1); \
		echo "TCONFIG_USB=y" >>$(1); \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/TCONFIG_USB_EXTRAS/d" $(1); \
			echo "TCONFIG_USB_EXTRAS=y" >>$(1); \
		fi; \
		if [ "$(NTFS)" = "y" ]; then \
			sed -i "/TCONFIG_NTFS/d" $(1); \
			echo "TCONFIG_NTFS=y" >>$(1); \
		fi; \
		if [ "$(UFSD)" = "y" ]; then \
			sed -i "/TCONFIG_UFSD/d" $(1); \
			echo "TCONFIG_UFSD=y" >>$(1); \
		fi; \
		if [ "$(HFS)" = "y" ]; then \
			sed -i "/TCONFIG_HFS/d" $(1); \
			echo "TCONFIG_HFS=y" >>$(1); \
		fi; \
		if [ "$(MEDIASRV)" = "y" ]; then \
			sed -i "/TCONFIG_MEDIA_SERVER/d" $(1); \
			echo "TCONFIG_MEDIA_SERVER=y" >>$(1); \
		fi; \
	else \
		sed -i "/TCONFIG_USB=y/d" $(1); \
		echo "# TCONFIG_USB is not set" >>$(1); \
	fi; \
	if [ "$(NO_FTP)" = "y" ]; then \
		sed -i "/TCONFIG_FTP/d" $(1); \
		echo "# TCONFIG_FTP is not set" >>$(1); \
	fi; \
	if [ "$(NO_SAMBA)" = "y" ]; then \
		sed -i "/TCONFIG_SAMBASRV/d" $(1); \
		echo "# TCONFIG_SAMBASRV is not set" >>$(1); \
	fi; \
	if [ "$(NO_ZEBRA)" = "y" ]; then \
		sed -i "/TCONFIG_ZEBRA/d" $(1); \
		echo "# TCONFIG_ZEBRA is not set" >>$(1); \
	fi; \
	if [ "$(NO_JFFS)" = "y" ]; then \
		sed -i "/TCONFIG_JFFS2/d" $(1); \
		echo "# TCONFIG_JFFS2 is not set" >>$(1); \
		sed -i "/TCONFIG_JFFSV1/d" $(1); \
		echo "# TCONFIG_JFFSV1 is not set" >>$(1); \
	fi; \
	if [ "$(NO_CIFS)" = "y" ]; then \
		sed -i "/TCONFIG_CIFS/d" $(1); \
		echo "# TCONFIG_CIFS is not set" >>$(1); \
	fi; \
	if [ "$(NO_SSH)" = "y" ]; then \
		sed -i "/TCONFIG_SSH/d" $(1); \
		echo "# TCONFIG_SSH is not set" >>$(1); \
	fi; \
	if [ "$(NO_HTTPS)" = "y" ]; then \
		sed -i "/TCONFIG_HTTPS/d" $(1); \
		echo "# TCONFIG_HTTPS is not set" >>$(1); \
	fi; \
	if [ "$(NO_XXTP)" = "y" ]; then \
		sed -i "/TCONFIG_L2TP/d" $(1); \
		echo "# TCONFIG_L2TP is not set" >>$(1); \
		sed -i "/TCONFIG_PPTP/d" $(1); \
		echo "# TCONFIG_PPTP is not set" >>$(1); \
	fi; \
	if [ "$(NO_LIBOPT)" = "y" ]; then \
		sed -i "/TCONFIG_OPTIMIZE_SHARED_LIBS/d" $(1); \
		echo "# TCONFIG_OPTIMIZE_SHARED_LIBS is not set" >>$(1); \
	fi; \
	if [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/TCONFIG_EBTABLES/d" $(1); \
		echo "TCONFIG_EBTABLES=y" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/TCONFIG_IPV6/d" $(1); \
		echo "TCONFIG_IPV6=y" >>$(1); \
	fi; \
	if [ "$(NOCAT)" = "y" ]; then \
		sed -i "/TCONFIG_NOCAT/d" $(1); \
		echo "TCONFIG_NOCAT=y" >>$(1); \
	fi; \
	if [ "$(NGINX)" = "y" ]; then \
		sed -i "/TCONFIG_NGINX/d" $(1); \
		echo "TCONFIG_NGINX=y" >>$(1); \
	fi; \
	sed -i "/TCONFIG_MULTIWAN/d" $(1); \
	if [ "$(NVRAM_SIZE)" = "60" ] || [ "$(NVRAM_SIZE)" = "64" ] || [ "$(NVRAM_64K)" = "y" ]; then \
		echo "TCONFIG_MULTIWAN=y" >>$(1); \
	else \
		echo "# TCONFIG_MULTIWAN is not set" >>$(1); \
	fi; \
	if [ "$(OPENVPN)" = "y" ]; then \
		sed -i "/TCONFIG_LZO/d" $(1); \
		echo "TCONFIG_LZO=y" >>$(1); \
		sed -i "/# TCONFIG_OPENVPN is not set/d" $(1); \
		echo "TCONFIG_OPENVPN=y" >>$(1); \
		sed -i "/TCONFIG_FTP_SSL/d" $(1); \
		echo "TCONFIG_FTP_SSL=y" >>$(1); \
	fi; \
	if [ "$(PPTPD)" = "y" ]; then \
		sed -i "/TCONFIG_PPTPD/d" $(1); \
		echo "TCONFIG_PPTPD=y" >>$(1); \
	fi; \
	if [ "$(BTCLIENT)" = "y" ]; then \
		sed -i "/TCONFIG_BT/d" $(1); \
		echo "TCONFIG_BT=y" >>$(1); \
		sed -i "/TCONFIG_BBT/d" $(1); \
		echo "TCONFIG_BBT=y" >>$(1); \
	fi; \
	if [ "$(BTGUI)" = "y" ]; then \
		sed -i "/TCONFIG_BT/d" $(1); \
		echo "TCONFIG_BT=y" >>$(1); \
	fi; \
	if [ "$(TR_EXTRAS)" = "y" ]; then \
		sed -i "/TCONFIG_TR_EXTRAS/d" $(1); \
		echo "TCONFIG_TR_EXTRAS=y" >>$(1); \
	fi; \
	if [ "$(NFS)" = "y" ]; then \
		sed -i "/TCONFIG_NFS/d" $(1); \
		echo "TCONFIG_NFS=y" >>$(1); \
	fi; \
	if [ "$(SNMP)" = "y" ]; then \
		sed -i "/TCONFIG_SNMP/d" $(1); \
		echo "TCONFIG_SNMP=y" >>$(1); \
	fi; \
	if [ "$(SDHC)" = "y" ]; then \
		sed -i "/TCONFIG_SDHC/d" $(1); \
		echo "TCONFIG_SDHC=y" >>$(1); \
	fi; \
	if [ "$(DNSSEC)" = "y" ]; then \
		sed -i "/TCONFIG_DNSSEC/d" $(1); \
		echo "TCONFIG_DNSSEC=y" >>$(1); \
	fi; \
	if [ "$(UPS)" = "y" ]; then \
		sed -i "/TCONFIG_UPS/d" $(1); \
		echo "TCONFIG_UPS=y" >>$(1); \
	fi; \
	if [ "$(NANO)" = "y" ]; then \
		sed -i "/TCONFIG_NANO/d" $(1); \
		echo "TCONFIG_NANO=y" >>$(1); \
	fi; \
	if [ "$(DNSCRYPT)" = "y" ]; then \
		sed -i "/TCONFIG_DNSCRYPT/d" $(1); \
		echo "TCONFIG_DNSCRYPT=y" >>$(1); \
	fi; \
	if [ "$(STUBBY)" = "y" ]; then \
		sed -i "/TCONFIG_STUBBY/d" $(1); \
		echo "TCONFIG_STUBBY=y" >>$(1); \
	fi; \
	if [ "$(NVRAM_64K)" = "y" ]; then \
		sed -i "/TCONFIG_NVRAM_64K/d" $(1); \
		echo "TCONFIG_NVRAM_64K=y" >>$(1); \
	fi; \
	if [ "$(TOR)" = "y" ]; then \
		sed -i "/TCONFIG_TOR/d" $(1); \
		echo "TCONFIG_TOR=y" >>$(1); \
	fi; \
	if [ "$(MICROSD)" = "y" ]; then \
		sed -i "/TCONFIG_MICROSD/d" $(1); \
		echo "TCONFIG_MICROSD=y" >>$(1); \
	fi; \
	if [ "$(USBAP)" = "y" ]; then \
		sed -i "/TCONFIG_USBAP/d" $(1); \
		echo "TCONFIG_USBAP=y" >>$(1); \
		if [ "$(NO_USBAPP)" = "y" ]; then \
			sed -i "/TCONFIG_REMOVE_USBAPP/d" $(1); \
			echo "TCONFIG_REMOVE_USBAPP=y" >>$(1); \
			sed -i "/TCONFIG_FTP/d" $(1); \
			echo "# TCONFIG_FTP is not set" >>$(1); \
			echo "# TCONFIG_FTP_SSL is not set" >>$(1); \
			sed -i "/TCONFIG_SAMBA/d" $(1); \
			echo "# TCONFIG_SAMBASRV is not set" >>$(1); \
			echo "# TCONFIG_SAMBA3 is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(ASUS_TRX)" = "RT-AC66U" ]; then \
		sed -i "/TCONFIG_AC66U/d" $(1); \
		echo "TCONFIG_AC66U=y" >>$(1); \
	else \
		sed -i "/TCONFIG_AC66U/d" $(1); \
		echo "# TCONFIG_AC66U is not set" >>$(1); \
	fi; \
	if [ "$(ASUS_TRX)" = "R6300V1" ]; then \
		sed -i "/TCONFIG_R6300V1/d" $(1); \
		echo "TCONFIG_R6300V1=y" >>$(1); \
	fi; \
	if [ "$(ASUS_TRX)" = "WNDR4500V1" ]; then \
		sed -i "/TCONFIG_WNDR4500V1/d" $(1); \
		echo "TCONFIG_WNDR4500V1=y" >>$(1); \
	fi; \
	if [ "$(ASUS_TRX)" = "WNDR4500V2" ]; then \
		sed -i "/TCONFIG_WNDR4500V2/d" $(1); \
		echo "TCONFIG_WNDR4500V2=y" >>$(1); \
	fi; \
	if [ "$(BUILD_DESC)" = "AIO" ]; then \
		sed -i "/TCONFIG_AIO/d" $(1); \
		echo "TCONFIG_AIO=y" >>$(1); \
	fi; \
	if [ "$(DONT_OPTIMIZE_SIZE)" = "y" ]; then \
		sed -i "/TCONFIG_OPTIMIZE_SIZE=y/d" $(1); \
		echo "# TCONFIG_OPTIMIZE_SIZE is not set" >>$(1); \
	fi; \
	if [ "$(CTF)" = "y" ]; then \
		sed -i "/TCONFIG_CTF/d" $(1); \
		echo "TCONFIG_CTF=y" >>$(1); \
	fi; \
	if [ "$(NAND)" = "y" ]; then \
		sed -i "/TCONFIG_NAND/d" $(1); \
		echo "TCONFIG_NAND=y" >>$(1); \
	fi; \
	if [ "$(SDK6MIPS)" = "y" ]; then \
		sed -i "/TCONFIG_BCMWL6/d" $(1); \
		echo "TCONFIG_BCMWL6=y" >>$(1); \
	fi; \
	if [ "$(ROAM)" = "y" ]; then \
		sed -i "/TCONFIG_ROAM/d" $(1); \
		echo "TCONFIG_ROAM=y" >>$(1); \
	fi; \
	if [ "$(BCMNAT)" = "y" ]; then \
		sed -i "/TCONFIG_BCMNAT/d" $(1); \
		echo "TCONFIG_BCMNAT=y" >>$(1); \
	fi; \
	if [ "$(TINC)" = "y" ]; then \
		sed -i "/TCONFIG_TINC/d" $(1); \
		echo "TCONFIG_TINC=y" >>$(1); \
	fi; \
	if [ "$(OPENSSL11)" = "y" ]; then \
		sed -i "/TCONFIG_OPENSSL11/d" $(1); \
		echo "TCONFIG_OPENSSL11=y" >>$(1); \
	fi; \
	if [ "$(IPERF)" = "y" ]; then \
		sed -i "/TCONFIG_IPERF/d" $(1); \
		echo "TCONFIG_IPERF=y" >>$(1); \
	fi; \
	if [ "$(KEYGEN)" = "y" ] && [ "$(NO_KEYGEN)" = "" ]; then \
		sed -i "/TCONFIG_KEYGEN/d" $(1); \
		echo "TCONFIG_KEYGEN=y" >>$(1); \
	fi; \
	if [ "$(TERMLIB)" = "y" ]; then \
		sed -i "/TCONFIG_TERMLIB/d" $(1); \
		echo "TCONFIG_TERMLIB=y" >>$(1); \
	fi; \
	if [ "$(ADVTHEMES)" = "y" ]; then \
		sed -i "/TCONFIG_ADVTHEMES/d" $(1); \
		echo "TCONFIG_ADVTHEMES=y" >>$(1); \
	fi; \
	if [ "$(NVRAM_SIZE)" = "32" ]; then \
		sed -i "/TCONFIG_NVRAM_32K/d" $(1); \
		echo "TCONFIG_NVRAM_32K=y" >>$(1); \
	fi; \
	if [ "$(OPTIMIZE_SIZE_MORE)" = "y" ]; then \
		sed -i "/TCONFIG_OPTIMIZE_SIZE_MORE/d" $(1); \
		echo "TCONFIG_OPTIMIZE_SIZE_MORE=y" >>$(1); \
	fi; \
	if [ "$(BRCM_NAND_JFFS2)" = "y" ]; then \
		sed -i "/TCONFIG_BRCM_NAND_JFFS2/d" $(1); \
		echo "TCONFIG_BRCM_NAND_JFFS2=y" >>$(1); \
	fi; \
	if [ "$(BONDING)" = "y" ]; then \
		sed -i "s/# TCONFIG_BONDING is not set/TCONFIG_BONDING=y/g" $(1); \
	fi; \
	if [ "$(MDNS)" = "y" ]; then \
		sed -i "s/# TCONFIG_MDNS is not set/TCONFIG_MDNS=y/g" $(1); \
	fi; \
	if [ "$(PROXY)" = "y" ]; then \
		sed -i "s/# TCONFIG_PROXY is not set/TCONFIG_PROXY=y/g" $(1); \
	fi; \
	if [ "$(QRCODE)" = "y" ]; then \
		sed -i "/TCONFIG_QRCODE/d" $(1); \
		echo "TCONFIG_QRCODE=y" >>$(1); \
	fi; \
	if [ "$(NO_BLINK)" = "y" ]; then \
		sed -i "s/TCONFIG_BLINK=y/# TCONFIG_BLINK is not set/g" $(1); \
	fi; \
	)
endef

define BusyboxOptions
	@( \
	sed -i "/CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT/d" $(1); \
	echo "CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT=y" >>$(1); \
	sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
	echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
	sed -i "/CONFIG_MKNOD/d" $(1); \
	echo "CONFIG_MKNOD=y" >>$(1); \
	if [ "$(NO_CIFS)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_MOUNT_CIFS/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_CIFS is not set" >>$(1); \
	fi; \
	if [ "$(BBEXTRAS)" = "y" ]; then \
		sed -i "/CONFIG_SENDMAIL/d" $(1); \
		echo "CONFIG_SENDMAIL=y" >>$(1); \
		sed -i "/CONFIG_WHOIS/d" $(1); \
		echo "CONFIG_WHOIS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_SORT_BIG/d" $(1); \
		echo "CONFIG_FEATURE_SORT_BIG=y" >>$(1); \
		sed -i "/CONFIG_CLEAR/d" $(1); \
		echo "CONFIG_CLEAR=y" >>$(1); \
		sed -i "/CONFIG_NICE/d" $(1); \
		echo "CONFIG_NICE=y" >>$(1); \
		sed -i "/CONFIG_SETCONSOLE/d" $(1); \
		echo "CONFIG_SETCONSOLE=y" >>$(1); \
		if [ "$(CONFIG_RT)" = "" ]; then \
			sed -i "/CONFIG_MKFIFO/d" $(1); \
			echo "CONFIG_MKFIFO=y" >>$(1); \
		fi; \
		sed -i "/CONFIG_SEQ/d" $(1); \
		echo "CONFIG_SEQ=y" >>$(1); \
		sed -i "/CONFIG_STTY/d" $(1); \
		echo "CONFIG_STTY=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/CONFIG_FDISK/d" $(1); \
			echo "CONFIG_FDISK=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_FDISK_WRITABLE/d" $(1); \
			echo "CONFIG_FEATURE_FDISK_WRITABLE=y" >>$(1); \
			sed -i "/CONFIG_MKFS_VFAT/d" $(1); \
			echo "CONFIG_MKFS_VFAT=y" >>$(1); \
			sed -i "/CONFIG_MKSWAP/d" $(1); \
			echo "CONFIG_MKSWAP=y" >>$(1); \
			sed -i "/CONFIG_FLOCK/d" $(1); \
			echo "CONFIG_FLOCK=y" >>$(1); \
			sed -i "/CONFIG_FSYNC/d" $(1); \
			echo "CONFIG_FSYNC=y" >>$(1); \
			sed -i "/CONFIG_LSUSB/d" $(1); \
			echo "CONFIG_LSUSB=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_WGET_STATUSBAR/d" $(1); \
			echo "CONFIG_FEATURE_WGET_STATUSBAR=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_VERBOSE_USAGE/d" $(1); \
			echo "CONFIG_FEATURE_VERBOSE_USAGE=y" >>$(1); \
		fi; \
	else \
		sed -i "/CONFIG_FEATURE_MOUNT_LOOP/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LOOP is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_LABEL/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LABEL is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_FSTAB/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_FSTAB is not set" >>$(1); \
		sed -i "/CONFIG_VOLUMEID/d" $(1); \
		echo "# CONFIG_VOLUMEID is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_SWAPON/d" $(1); \
		echo "# CONFIG_SWAPON is not set" >>$(1); \
		sed -i "/CONFIG_SWAPOFF/d" $(1); \
		echo "# CONFIG_SWAPOFF is not set" >>$(1); \
		sed -i "/CONFIG_CHROOT/d" $(1); \
		echo "# CONFIG_CHROOT is not set" >>$(1); \
		sed -i "/CONFIG_PIVOT_ROOT/d" $(1); \
		echo "# CONFIG_PIVOT_ROOT is not set" >>$(1); \
		sed -i "/CONFIG_TRUE/d" $(1); \
		echo "# CONFIG_TRUE is not set" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_IPV6/d" $(1); \
		echo "CONFIG_FEATURE_IPV6=y" >>$(1); \
		sed -i "/CONFIG_PING6/d" $(1); \
		echo "CONFIG_PING6=y" >>$(1); \
		sed -i "/CONFIG_TRACEROUTE6/d" $(1); \
		echo "CONFIG_TRACEROUTE6=y" >>$(1); \
	fi; \
	if [ "$(SLIM)" = "y" ]; then \
		sed -i "/CONFIG_AWK/d" $(1); \
		echo "# CONFIG_AWK is not set" >>$(1); \
		sed -i "/CONFIG_BASENAME/d" $(1); \
		echo "# CONFIG_BASENAME is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_TELNET=y/d" $(1); \
		echo "# CONFIG_TELNET is not set" >>$(1); \
		sed -i "/CONFIG_ARPING/d" $(1); \
		echo "# CONFIG_ARPING is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR/d" $(1); \
		echo "# CONFIG_FEATURE_LS_COLOR is not set" >>$(1); \
		sed -i "/CONFIG_CHOWN/d" $(1); \
		echo "# CONFIG_CHOWN is not set" >>$(1); \
	else \
		sed -i "/CONFIG_FEATURE_LS_COLOR/d" $(1); \
		echo "CONFIG_FEATURE_LS_COLOR=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR_IS_DEFAULT/d" $(1); \
		echo "CONFIG_FEATURE_LS_COLOR_IS_DEFAULT=y" >>$(1); \
	fi; \
	if [ "$(BUILD_DESC)" = "AIO" ] || [ "$(BUILD_DESC)" = "Mega-VPN" ]; then \
		sed -i "s/# CONFIG_TIME is not set/CONFIG_TIME=y/g" $(1); \
		sed -i "s/# CONFIG_GETOPT is not set/CONFIG_GETOPT=y/g" $(1); \
		sed -i "s/# CONFIG_FEATURE_GETOPT_LONG is not set/CONFIG_FEATURE_GETOPT_LONG=y/g" $(1); \
		sed -i "s/# CONFIG_DIFF is not set/CONFIG_DIFF=y/g" $(1); \
		sed -i "s/# CONFIG_LSOF is not set/CONFIG_LSOF=y/g" $(1); \
	fi; \
	)
endef

define KernelConfig
	@( \
	sed -i "/CONFIG_NVRAM_SIZE/d" $(1); \
	echo "CONFIG_NVRAM_SIZE="$(NVRAM_SIZE) >>$(1); \
	sed -i "/CONFIG_CC_OPTIMIZE_FOR_SIZE/d" $(1); \
	if [ "$(KERN_SIZE_OPT)" = "y" ]; then \
		echo "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" >>$(1); \
	else \
		echo "# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set" >>$(1); \
	fi; \
	if [ "$(KERN_SIZE_OPT_MORE)" = "y" ]; then \
		sed -i "/CONFIG_BUG=y/d" $(1); \
		echo "# CONFIG_BUG is not set" >>$(1); \
	else \
		sed -i "/# CONFIG_BUG is not set/d" $(1); \
		echo "CONFIG_BUG=y" >>$(1); \
	fi; \
	if [ "$(MIPS32)" = "r2" ]; then \
		sed -i "/CONFIG_CPU_MIPS32_R1/d" $(1); \
		echo "# CONFIG_CPU_MIPS32_R1 is not set" >>$(1); \
		sed -i "/CONFIG_CPU_MIPS32_R2/d" $(1); \
		echo "CONFIG_CPU_MIPS32_R2=y" >>$(1); \
		sed -i "/CONFIG_CPU_MIPSR1/d" $(1); \
		echo "CONFIG_CPU_MIPSR2=y" >>$(1); \
		if [ "$(NAND)" = "y" ]; then \
			sed -i "/CONFIG_MTD_NFLASH/d" $(1); \
			echo "CONFIG_MTD_NFLASH=y" >>$(1); \
			sed -i "/CONFIG_MTD_NAND/d" $(1); \
			echo "CONFIG_MTD_NAND=y" >>$(1); \
			echo "CONFIG_MTD_NAND_IDS=y" >>$(1); \
			echo "# CONFIG_MTD_NAND_VERIFY_WRITE is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_ECC_SMC is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_MUSEUM_IDS is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_DISKONCHIP is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_CAFE is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_NANDSIM is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_PLATFORM is not set" >>$(1); \
			echo "# CONFIG_MTD_NAND_ONENAND is not set" >>$(1); \
			sed -i "/CONFIG_MTD_BRCMNAND/d" $(1); \
			echo "CONFIG_MTD_BRCMNAND=y" >>$(1); \
		fi; \
		if [ "$(WNR3500LV2)" = "1" ]; then \
			sed -i "/CONFIG_MTD_NFLASH/d" $(1); \
			echo "CONFIG_MTD_NFLASH=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(UPS)" = "y" ]; then \
		sed -i "/CONFIG_USB_HIDDEV=m/d" $(1); \
		echo "CONFIG_USB_HIDDEV=y" >>$(1); \
		sed -i "/CONFIG_USB_DEVICEFS=m/d" $(1); \
		echo "CONFIG_USB_DEVICEFS=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "" ]; then \
		sed -i "/CONFIG_EFI_PARTITION/d" $(1); \
		echo "# CONFIG_EFI_PARTITION is not set" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/CONFIG_IPV6 is not set/d" $(1); \
		echo "CONFIG_IPV6=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_IPTABLES/d" $(1); \
		echo "CONFIG_IP6_NF_IPTABLES=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MATCH_RT/d" $(1); \
		echo "CONFIG_IP6_NF_MATCH_RT=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_FILTER/d" $(1); \
		echo "CONFIG_IP6_NF_FILTER=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_LOG/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_LOG=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_REJECT/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_REJECT=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MANGLE/d" $(1); \
		echo "CONFIG_IP6_NF_MANGLE=m" >>$(1); \
		sed -i "/CONFIG_NF_CONNTRACK_IPV6/d" $(1); \
		echo "CONFIG_NF_CONNTRACK_IPV6=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_HL/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_HL=m" >>$(1); \
		sed -i "/CONFIG_IPV6_ROUTER_PREF/d" $(1); \
		echo "CONFIG_IPV6_ROUTER_PREF=y" >>$(1); \
		sed -i "/CONFIG_IPV6_SIT/d" $(1); \
		echo "CONFIG_IPV6_SIT=m" >>$(1); \
		sed -i "/CONFIG_IPV6_SIT_6RD/d" $(1); \
		echo "CONFIG_IPV6_SIT_6RD=y" >>$(1); \
		sed -i "/CONFIG_IPV6_MULTIPLE_TABLES/d" $(1); \
		echo "CONFIG_IPV6_MULTIPLE_TABLES=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_RAW/d" $(1); \
		echo "CONFIG_IP6_NF_RAW=m" >>$(1); \
		sed -i "/CONFIG_IPV6_OPTIMISTIC_DAD/d" $(1); \
		echo "CONFIG_IPV6_OPTIMISTIC_DAD=y" >>$(1); \
		sed -i "/CONFIG_IPV6_MROUTE/d" $(1); \
		echo "CONFIG_IPV6_MROUTE=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_ROUTE/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_ROUTE=m" >>$(1); \
		sed -i "/CONFIG_INET6_XFRM_TUNNEL/d" $(1); \
		echo "CONFIG_INET6_XFRM_TUNNEL=m" >>$(1); \
		sed -i "/CONFIG_INET6_AH/d" $(1); \
		echo "CONFIG_INET6_AH=m" >>$(1); \
		sed -i "/CONFIG_INET6_ESP/d" $(1); \
		echo "CONFIG_INET6_ESP=m" >>$(1); \
		sed -i "/CONFIG_INET6_IPCOMP/d" $(1); \
		echo "CONFIG_INET6_IPCOMP=m" >>$(1); \
		sed -i "/CONFIG_INET6_XFRM_MODE_TRANSPORT/d" $(1); \
		echo "CONFIG_INET6_XFRM_MODE_TRANSPORT=m" >>$(1); \
		sed -i "/CONFIG_INET6_XFRM_MODE_TUNNEL/d" $(1); \
		echo "CONFIG_INET6_XFRM_MODE_TUNNEL=m" >>$(1); \
		sed -i "/CONFIG_INET6_XFRM_MODE_BEET/d" $(1); \
		echo "CONFIG_INET6_XFRM_MODE_BEET=m" >>$(1); \
	fi; \
	sed -i "/CONFIG_BCM57XX/d" $(1); \
	if [ "$(BCM57)" = "y" ]; then \
		echo "CONFIG_BCM57XX=m" >>$(1); \
	else \
		echo "# CONFIG_BCM57XX is not set" >>$(1); \
	fi; \
	if [ "$(CONFIG_RT)" = "" ]; then \
		if [ "$(USBAP)" = "y" ]; then \
			echo "CONFIG_WL_USBAP=y" >>$(1); \
			echo 'CONFIG_WL_APSTA="wlconfig_lx_router_high"' >>$(1); \
			sed -i "/CONFIG_USB_USBNET/d" $(1); \
			echo "# CONFIG_USB_USBNET is not set" >>$(1); \
			sed -i "/CONFIG_USB_NET_CDCETHER/d" $(1); \
			echo "# CONFIG_USB_NET_CDCETHER is not set" >>$(1); \
			sed -i "/CONFIG_USB_NET_CDC_NCM/d" $(1); \
			echo "# CONFIG_USB_NET_CDC_NCM is not set" >>$(1); \
			sed -i "/CONFIG_USB_NET_HUAWEIETHER/d" $(1); \
			echo "# CONFIG_USB_NET_HUAWEIETHER is not set" >>$(1); \
			sed -i "/CONFIG_USB_NET_QMI_WWAN/d" $(1); \
			echo "# CONFIG_USB_NET_QMI_WWAN is not set" >>$(1); \
			sed -i "/CONFIG_USB_IPHETH/d" $(1); \
			echo "# CONFIG_USB_IPHETH is not set" >>$(1); \
			sed -i "/CONFIG_USB_NET_RNDIS_HOST/d" $(1); \
			echo "# CONFIG_USB_NET_RNDIS_HOST is not set" >>$(1); \
			sed -i "/CONFIG_USB_WDM/d" $(1); \
			echo "# CONFIG_USB_WDM is not set" >>$(1); \
			sed -i "/CONFIG_USB_ACM/d" $(1); \
			echo "# CONFIG_USB_ACM is not set" >>$(1); \
		else \
			echo "# CONFIG_WL_USBAP is not set" >>$(1); \
			echo 'CONFIG_WL_APSTA="wlconfig_lx_router_apsta"' >>$(1); \
		fi; \
	fi; \
	if [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/CONFIG_BRIDGE_NF_EBTABLES/d" $(1); \
		echo "CONFIG_BRIDGE_NF_EBTABLES=m" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_BRIDGE_EBT_IP6/d" $(1); \
			echo "CONFIG_BRIDGE_EBT_IP6=m" >>$(1); \
		fi; \
	fi; \
	if [ "$(CONFIG_RT)" = "" ]; then \
		sed -i "/CONFIG_NVRAM_64K/d" $(1); \
		if [ "$(NVRAM_64K)" = "y" ]; then \
			echo "CONFIG_NVRAM_64K=y" >>$(1); \
		else \
			echo "# CONFIG_NVRAM_64K is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(CONFIG_RT)" = "" ]; then \
		if [ "$(BUILD_DESC)" = "AIO" ] || [ "$(BUILD_DESC)" = "Mega-VPN" ]; then \
			sed -i "s/# CONFIG_NETFILTER_XT_MATCH_STATISTIC is not set/CONFIG_NETFILTER_XT_MATCH_STATISTIC=m/g" $(1); \
			sed -i "s/# CONFIG_NETFILTER_XT_MATCH_COMMENT is not set/CONFIG_NETFILTER_XT_MATCH_COMMENT=y/g" $(1); \
		fi; \
	fi; \
	if [ "$(BONDING)" = "y" ]; then \
		sed -i "s/# CONFIG_BONDING is not set/CONFIG_BONDING=m/g" $(1); \
	fi; \
	)
endef


bin:
ifeq ($(B),)
	@echo $@" is not a valid target!"
	@false
endif
	@cp router/config_base router/config_$(lowercase_B)
	@cp router/busybox/config_base router/busybox/config_$(lowercase_B)
	@cp $(LINUXDIR)/config_base $(LINUXDIR)/config_$(lowercase_B)

	$(call RouterOptions, router/config_$(lowercase_B))
	$(call KernelConfig, $(LINUXDIR)/config_$(lowercase_B))
	$(call BusyboxOptions, router/busybox/config_$(lowercase_B))

	@$(MAKE) setprofile N=$(TPROFILE) B=$(B) DESC="$(BUILD_DESC)" USB="$(USB)"
	@$(MAKE) all

include targets.mak

setprofile:
	echo '#ifndef TOMATO_PROFILE' > router/shared/tomato_profile.h
	echo '#define TOMATO_$(N) 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_G 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_N 2' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE PROFILE_$(N)' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE_NAME "$(N)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_NAME "$(B)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_DESC "$(DESC)$(EXTRA_64KDESC)"' >> router/shared/tomato_profile.h
	echo '#ifndef CONFIG_NVRAM_SIZE' >> router/shared/tomato_profile.h
	echo '#define CONFIG_NVRAM_SIZE $(NVRAM_SIZE)' >> router/shared/tomato_profile.h
	echo '#endif' >> router/shared/tomato_profile.h
	echo '#endif' >> router/shared/tomato_profile.h

	echo 'TOMATO_$(N) = 1' > tomato_profile.mak
	echo 'PROFILE_G = 1' >> tomato_profile.mak
	echo 'PROFILE_N = 2' >> tomato_profile.mak
	echo 'TOMATO_PROFILE = $$(PROFILE_$(N))' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_NAME = "$(N)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD = "$(B)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD_NAME = "$(B)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD_DESC = "$(DESC)$(EXTRA_64KDESC)"' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_L = $(lowercase_N)' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_U = $(uppercase_N)' >> tomato_profile.mak
	echo 'TOMATO_BUILD_USB = "$(USB)"' >> tomato_profile.mak

	echo 'export EXTRACFLAGS := $(EXTRA_CFLAGS) -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign $(if $(filter $(MIPS32),r2),-march=mips32r2 -mips32r2 -mtune=mips32r2,-march=mips32 -mips32 -mtune=mips32) $(if $(filter $(NVRAM_SIZE),0),,-DCONFIG_NVRAM_SIZE=$(NVRAM_SIZE)) $(EXTRA_64KCFLAG)' >> tomato_profile.mak

# Note that changes to variables in tomato_profile.mak don't
# get propogated to this invocation of make!
	@echo ""
	@echo "Using $(N) profile, $(B) build config."
	@echo ""

	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp -f config_current .config

	@cd router/busybox && \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@cd router ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@$(MAKE) -C router oldconfig

include help.mak

help: help_main
	@echo ""
	@echo "..etc.. - other build configs"
	@echo " clean          -C router clean"
	@echo " cleanimage     rm -rf image"
	@echo " cleantools     clean btools, mksquashfs"
	@echo " cleankernel    -C Linux distclean (but preserves .config)"
	@echo " distclean      distclean of Linux & busybox (but preserve .configs)"
	@echo " prepk          -C Linux oldconfig dep"

.PHONY: all clean distclean cleanimage cleantools cleankernel prepk what setprofile help
.PHONY: a b c d m nc Makefile allversions tomato_profile.mak
