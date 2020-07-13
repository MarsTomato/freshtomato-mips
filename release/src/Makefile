#
# Toplevel Makefile for the BCM947xx Linux Router release
#
# Copyright 2005, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.53 2005/04/25 03:54:37 tallest Exp $
#

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# To rebuild everything and all configurations:
#  make distclean
#  make V1=whatever V2=sub-whatever VPN=vpn3.6 a b c d m n o
#
# The 1st "whatever" would be the build number - use only when you want the build number to be added 
# to the tomato version; the sub-whatever would be the version note (both optional).
#
# Example:
# make V1=8516 V2="my_build" a b c d m s n o
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


export ac_cv_func_malloc_0_nonnull=yes
export SRCBASE := $(shell pwd)
RELEASEDIR := $(shell (cd $(SRCBASE)/.. && pwd -P))
PATH := $(RELEASEDIR)/tools:$(PATH)
export TPROFILE := N

include ./target.mak

V1 ?= "--def"
VPN ?= "VPN"
ND = "K26"
RT_SUFFIX="_RT"

PPTPD ?= "n"

KERN_SIZE_OPT ?= y

ifeq ($(NVRAM_SIZE),)
NVRAM_SIZE = 0
endif

-include tomato_profile.mak

# This could be simpler by just using $(TOMATO_PROFILE_NAME) like it used to be,
# but that's fragile.  If you make one certain innocuous change elsewhere in the
# makefile(s), the build will silently be wrong.  This way it works properly every time.
current_BUILD_NAME = $(strip $(shell grep "^TOMATO_BUILD_NAME" tomato_profile.mak | cut -d"=" -f2))
current_BUILD_DESC = $(strip $(shell grep "^TOMATO_BUILD_DESC" tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_BUILD_USB  = $(strip $(shell grep "^TOMATO_BUILD_USB"  tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_TOMATO_VER = $(strip $(shell grep "TOMATO_MAJOR" router/shared/tomato_version.h | cut -d"\"" -f2)).$(strip $(shell grep "TOMATO_MINOR" router/shared/tomato_version.h | cut -d"\"" -f2))$(if $(filter $(V1),"--def"),,.$(strip $(shell grep -w "TOMATO_BUILD" router/shared/tomato_version.h | cut -d"\"" -f2)))

uppercase_N = $(shell echo $(N) | tr a-z  A-Z)
lowercase_N = $(shell echo $(N) | tr A-Z a-z)
uppercase_B = $(shell echo $(B) | tr a-z  A-Z)
lowercase_B = $(shell echo $(B) | tr A-Z a-z)

mips_rev = $(if $(filter $(MIPS32),r2),MIPSR2,MIPSR1)

beta = $(if $(filter $(TOMATO_EXPERIMENTAL),1),-beta,)

tomato_ver:
	@echo ""
	@btools/uversion.pl --gen $(V1) $(mips_rev)$(beta)$(V2) $(ND) $(current_BUILD_USB) $(current_BUILD_DESC)

ifeq ($(TOMATO_BUILD),)

all:
	$(MAKE) z

else

all: tomato_ver
	@echo ""
	@echo "Building FreshTomato $(ND) $(current_BUILD_USB)-$(mips_rev) $(current_TOMATO_VER)$(beta)$(V2) $(current_BUILD_DESC) $(current_BUILD_NAME) with $(TOMATO_PROFILE_NAME) Profile"
	@echo ""
	@echo ""

	@-mkdir image
	@$(MAKE) -C router all
	@$(MAKE) -C router install
	@$(MAKE) -C btools

	@echo "\033[41;1m   Creating image \033[0m\033]2;Creating image\007"

	@rm -f image/freshtomato-$(ND)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).trx
	@rm -f image/freshtomato-$(ND)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin
ifeq ($(LINKSYS_E),1)
	@rm -f image/freshtomato-E??00$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(BELKIN),y)
ifneq ($(NVRAM_SIZE),60)
	@rm -f image/freshtomato-F7D????$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-F5D8235v3$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin
endif
endif

	@echo "" >>fpkg.log
	@echo "***********************" `date` "************************" >>fpkg.log
	@cat router/shared/tomato_version >>fpkg.log
	@echo "" >>fpkg.log
	@cat router/target.info >>fpkg.log

ifeq ($(wildcard include/bcm20xx.h),)
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato.trx \
		-l W54G,image/WRT54G_WRT54GL.bin \
		-l W54S,image/WRT54GS.bin \
		-l W54s,image/WRT54GSv4.bin \
		-l W54U,image/WRTSL54GS.bin \
		-m 0x10577050,image/WR850G.bin \
		| tee -a fpkg.log
else
ifeq ($(LINKSYS_E),1)
	# Linksys E-series build plus generic TRX image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato-$(ND)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).trx \
		-l 4200,image/freshtomato-E4200$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		-l 61XN,image/freshtomato-E3000$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		-l 32XN,image/freshtomato-E2000$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
else
ifeq ($(BELKIN),y)
ifneq ($(NVRAM_SIZE),60)
	# Create Belkin images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato-$(ND)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).trx \
		-b 0x20100322,image/freshtomato-F7D3301$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		-b 0x20090928,image/freshtomato-F7D3302$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		-b 0x20091006,image/freshtomato-F7D4302$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		-b 0x00017116,image/freshtomato-F5D8235v3$(current_BUILD_USB)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
else
	# Create generic TRX image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato-$(ND)$(current_BUILD_USB)$(if $(filter $(NVRAM_SIZE),0),,-NVRAM$(NVRAM_SIZE)K)$(RT_SUFFIX)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2)-$(current_BUILD_DESC).trx \
		| tee -a fpkg.log
endif
endif
endif

	@cp fpkg.log image/fpkg-$(current_BUILD_USB)-$(mips_rev)-$(current_TOMATO_VER)$(beta)$(V2).log
	@echo ""
	@echo "-----------------"
	@echo `cat router/shared/tomato_version` " ready"
	@echo "-----------------"
	@echo "\033[41;1m   FreshTomato build done! \033[0m\033]2;FreshTomato build done!\007"
endif



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
# KERN_SIZE_OPT KERN_SIZE_OPT_MORE
# USB ("USB"|"")
# JFFSv1 | NO_JFFS
# NO_CIFS, NO_SSH, NO_ZEBRA, NO_SAMBA, NO_HTTPS, NO_XXTP, NO_LIBOPT
# SAMBA3, OPENVPN, DONT_OPTIMIZE_SIZE, OPTIMIZE_SIZE_MORE, IPV6SUPP, EBTABLES, NTFS, MEDIASRV, BBEXTRAS, USBEXTRAS, BCM57, SLIM, NOCAT, CTF, NGINX NANO BCMNAT
# NFS BTCLIENT BTGUI TR_EXTRAS SNMP SDHC HFS UPS DNSCRYPT STUBBY PPTPD TOR IPSEC RAID IPERF KEYGEN TERMLIB ADVTHEMES BRCM_NAND_JFFS2

define RouterOptions
	@( \
	sed -i "/TCONFIG_SAMBA3/d" $(1); \
	echo "TCONFIG_SAMBA3=y" >>$(1); \
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
		if [ "$(MEDIASRV)" = "y" ]; then \
			sed -i "/TCONFIG_MEDIA_SERVER/d" $(1); \
			echo "TCONFIG_MEDIA_SERVER=y" >>$(1); \
		fi; \
	else \
		sed -i "/TCONFIG_USB=y/d" $(1); \
		echo "# TCONFIG_USB is not set" >>$(1); \
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
	if [ "$(HFS)" = "y" ]; then \
		sed -i "/TCONFIG_HFS/d" $(1); \
		echo "TCONFIG_HFS=y" >>$(1); \
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
	if [ "$(TOR)" = "y" ]; then \
		sed -i "/TCONFIG_TOR/d" $(1); \
		echo "TCONFIG_TOR=y" >>$(1); \
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
	if [ "$(KEYGEN)" = "y" ]; then \
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
	)
endef

define BusyboxOptions
	@( \
	sed -i "/CONFIG_FEATURE_2_4_MODULES/d" $(1); \
	echo "# CONFIG_FEATURE_2_4_MODULES is not set" >>$(1); \
	sed -i "/CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT/d" $(1); \
	echo "CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT=y" >>$(1); \
	sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
	echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
	sed -i "/CONFIG_MKNOD/d" $(1); \
	echo "CONFIG_MKNOD=y" >>$(1); \
	sed -i "/CONFIG_FEATURE_SYSLOGD_READ_BUFFER_SIZE/d" $(1); \
	echo "CONFIG_FEATURE_SYSLOGD_READ_BUFFER_SIZE=512" >>$(1); \
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
		sed -i "/CONFIG_SEQ/d" $(1); \
		echo "CONFIG_SEQ=y" >>$(1); \
		sed -i "/CONFIG_STTY/d" $(1); \
		echo "CONFIG_STTY=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/CONFIG_E2FSCK/d" $(1); \
			echo "CONFIG_E2FSCK=y" >>$(1); \
			sed -i "/CONFIG_MKE2FS/d" $(1); \
			echo "CONFIG_MKE2FS=y" >>$(1); \
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
			sed -i "/CONFIG_TUNE2FS/d" $(1); \
			echo "CONFIG_TUNE2FS=y" >>$(1); \
			sed -i "/CONFIG_E2LABEL/d" $(1); \
			echo "CONFIG_E2LABEL=y" >>$(1); \
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
		sed -i "/CONFIG_SWAPONOFF/d" $(1); \
		echo "# CONFIG_SWAPONOFF is not set" >>$(1); \
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
		sed -i "/CONFIG_FEATURE_UDHCP_RFC5969/d" $(1); \
		echo "CONFIG_FEATURE_UDHCP_RFC5969=y" >>$(1); \
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
	fi; \
	if [ "$(UPS)" = "y" ]; then \
		sed -i "/CONFIG_INPUT=m/d" $(1); \
		echo "CONFIG_INPUT=y" >>$(1); \
		sed -i "/CONFIG_USB=m/d" $(1); \
		echo "CONFIG_USB=y" >>$(1); \
		sed -i "/CONFIG_HID=m/d" $(1); \
		echo "CONFIG_HID=y" >>$(1); \
		sed -i "/CONFIG_USB_HID=m/d" $(1); \
		echo "CONFIG_USB_HID=y" >>$(1); \
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
	if [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/CONFIG_BRIDGE_NF_EBTABLES/d" $(1); \
		echo "CONFIG_BRIDGE_NF_EBTABLES=m" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_BRIDGE_EBT_IP6/d" $(1); \
			echo "CONFIG_BRIDGE_EBT_IP6=m" >>$(1); \
		fi; \
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



#################################


## targets with USB support

## VPN
e:
	@$(MAKE) bin USB="USB" B=E NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y STUBBY=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="$(VPN)"


## BTgui-VPN
c:
	@$(MAKE) bin USB="USB" B=M NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y BTGUI=y STUBBY=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="BTgui-VPN"


## Nocat-VPN
d:
	@$(MAKE) bin USB="USB" B=M NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y BTGUI=y STUBBY=y NOCAT=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="Nocat-VPN"


## BT
u:
	@$(MAKE) bin USB="USB" B=M NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y \
	    BTCLIENT=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="BT"


## BT-VPN
t:
	@$(MAKE) bin USB="USB" B=M NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y PPTPD=y \
	    NO_JFFS=y BTCLIENT=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="BT-VPN"


## Big-VPN
b:
	@$(MAKE) bin USB="USB" B=M NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y BTGUI=y STUBBY=y NO_CIFS=y NO_JFFS=y NFS=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="Big-VPN"


## Mega-VPN
o:
	@$(MAKE) bin USB="USB" B=E NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y MEDIASRV=y DNSCRYPT=y STUBBY=y NOCAT=y TOR=y BTCLIENT=y NFS=y SNMP=y UPS=y TR_EXTRAS=y IPSEC=y DNSSEC=y TINC=y NANO=y HFS=y IPERF=y TERMLIB=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="Mega-VPN"


## All-In-One (AIO) - 8MB+
z:
	@$(MAKE) bin USB="USB" B=E NTFS=y EBTABLES=y MULTIWAN=y IPV6SUPP=y USBEXTRAS=y BCMNAT=y OPENVPN=y KEYGEN=y PPTPD=y \
	    BBEXTRAS=y MEDIASRV=y DNSCRYPT=y STUBBY=y NOCAT=y TOR=y BTCLIENT=y NFS=y SNMP=y UPS=y TR_EXTRAS=y IPSEC=y DNSSEC=y TINC=y NANO=y HFS=y NGINX=y IPERF=y TERMLIB=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="AIO"


#################################


## targets with NO USB support

## Mini - targeted for Netgear routers
f:
	@$(MAKE) bin USB="" B=F MULTIWAN=y NO_CIFS=y JFFSv1=y NO_ZEBRA=y BCMNAT=y BUILD_DESC="Mini"


## MiniIPv6 - for 4MB routers
i:
	@$(MAKE) bin USB="" B=I MULTIWAN=y NO_CIFS=y NO_ZEBRA=y NO_JFFS=y IPV6SUPP=y NO_HTTPS=y BCMNAT=y BUILD_DESC="MiniIPv6"


## MiniVPN - for 4MB routers
j:
	@$(MAKE) bin USB="" B=V MULTIWAN=y NO_CIFS=y NO_ZEBRA=y NO_JFFS=y OPENVPN=y OPTIMIZE_SIZE_MORE=y KERN_SIZE_OPT_MORE=y BUILD_DESC="MiniVPN"


## MiniVPN2 - for 4MB routers
j2:
	@$(MAKE) bin USB="" B=V MULTIWAN=y NO_CIFS=y NO_ZEBRA=y NO_JFFS=y PPTPD=y OPTIMIZE_SIZE_MORE=y KERN_SIZE_OPT_MORE=y BUILD_DESC="MiniVPN2"


## IPv6-VPN with no USB support
v:
	@$(MAKE) bin USB="" B=V MULTIWAN=y NO_JFFS=y IPV6SUPP=y OPENVPN=y PPTPD=y OPTIMIZE_SIZE_MORE=y KERN_SIZE_OPT_MORE=y BUILD_DESC="IPv6-$(VPN)"


## Max build for router with 8MB flash without USB Support (like WRT54GS v1)
m:
	@$(MAKE) bin USB="" B=E MULTIWAN=y IPV6SUPP=y OPENVPN=y KEYGEN=y PPTPD=y NOCAT=y NFS=y SNMP=y STUBBY=y TOR=y IPSEC=y BBEXTRAS=y EBTABLES=y BCMNAT=y ADVTHEMES=y OPENSSL11=y DONT_OPTIMIZE_SIZE=y BUILD_DESC="Max"



########## Mips32r2 builds ##########


## AIO MIPS2 for RT-N16
r2z:
	@$(MAKE) z MIPS32=r2 NVRAM_SIZE=32

r2e:
	@$(MAKE) e MIPS32=r2 NVRAM_SIZE=32

r2c:
	@$(MAKE) c MIPS32=r2 NVRAM_SIZE=32

r2u:
	@$(MAKE) u MIPS32=r2 NVRAM_SIZE=32

r2t:
	@$(MAKE) t MIPS32=r2 NVRAM_SIZE=32

r2d:
	@$(MAKE) d MIPS32=r2 NVRAM_SIZE=32

r2b:
	@$(MAKE) b MIPS32=r2 NVRAM_SIZE=32

r2o:
	@$(MAKE) o MIPS32=r2 NVRAM_SIZE=32

r2f:
	@$(MAKE) f MIPS32=r2 BELKIN=y NVRAM_SIZE=32

r2i:
	@$(MAKE) i MIPS32=r2 NVRAM_SIZE=32

r2j:
	@$(MAKE) j MIPS32=r2 NVRAM_SIZE=32

r2j2:
	@$(MAKE) j2 MIPS32=r2 NVRAM_SIZE=32

r2v:
	@$(MAKE) v MIPS32=r2 NVRAM_SIZE=32

r2m:
	@$(MAKE) m MIPS32=r2 NVRAM_SIZE=32


## Linksys
n60e:
	@$(MAKE) e MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60c:
	@$(MAKE) c MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60u:
	@$(MAKE) u MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60t:
	@$(MAKE) t MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60b:
	@$(MAKE) b MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60d:
	@$(MAKE) d MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60o:
	@$(MAKE) o MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60v:
	@$(MAKE) v MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

n60m:
	@$(MAKE) m MIPS32=r2 NVRAM_SIZE=60 LINKSYS_E=1

setprofile:
	echo '#ifndef TOMATO_PROFILE' > router/shared/tomato_profile.h
	echo '#define TOMATO_$(N) 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_G 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_N 2' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE PROFILE_$(N)' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE_NAME "$(N)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_NAME "$(B)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_DESC "$(DESC)"' >> router/shared/tomato_profile.h
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
	echo 'TOMATO_BUILD_DESC = "$(DESC)"' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_L = $(lowercase_N)' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_U = $(uppercase_N)' >> tomato_profile.mak
	echo 'TOMATO_BUILD_USB = "$(USB)"' >> tomato_profile.mak

	echo 'export EXTRACFLAGS := $(EXTRA_CFLAGS) -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign $(if $(filter $(MIPS32),r2),-march=mips32r2 -mips32r2 -mtune=mips32r2,-march=mips32 -mips32 -mtune=mips32) $(if $(filter $(NVRAM_SIZE),0),,-DCONFIG_NVRAM_SIZE=$(NVRAM_SIZE))' >> tomato_profile.mak

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

help:
	@echo "e            VPN - (standard plus VPN, extra utilities and NTFS support)"
	@echo "c            BTgui-VPN - (Ext plus BT gui)"
	@echo "d            Nocat-VPN - (BTgui plus VPN plus Nocat)"
	@echo "u            BT"
	@echo "t            BT-VPN - (BT plus VPN)"
	@echo "b            Big-VPN - (Big plus VPN)"
	@echo "o            Mega-VPN - (Mega plus VPN plus NOCAT minus NFS)"
	@echo "z            MIPS Release 15F AIO (for routers +8MB flash)"
	@echo "i            MiniIPv6 - (IPv6 with no USB support minus CIFS and RIPv1/2)"
	@echo "j            MiniVPN - (OpenVPN with no USB support minus CIFS and RIPv1/2)"
	@echo "j2           MiniVPN - (PPTPD with no USB support minus CIFS and RIPv1/2)"
	@echo "f            Mini - (no USB support minus CIFS and RIPv1/2)"
	@echo "v            IPv6-VPN (no usb) - (VPN with no USB support)"
	@echo "m            Max (no usb) - (Max with IPv6 support and no USB)"
	@echo ""
	@echo "r2e          MIPS Release 2 VPN"
	@echo "r2c          MIPS Release 2 BTgui-VPN"
	@echo "r2d          MIPS Release 2 Nocat-VPN"
	@echo "r2u          MIPS Release 2 BT"
	@echo "r2t          MIPS Release 2 BT-VPN"
	@echo "r2b          MIPS Release 2 Big-VPN"
	@echo "r2o          MIPS Release 2 Mega-VPN (for routers +8MB flash)"
	@echo "r2z          MIPS Release 2 AIO (for routers +8MB flash)"
	@echo "r2v          MIPS Release 2 IPv6-VPN (no usb)"
	@echo "r2m          MIPS Release 2 Max (no usb)"
	@echo "r2i          MIPS Release 2 MiniIPv6 (for 4MB flash)"
	@echo "r2j          MIPS Release 2 MiniVPN (OpenVPN; for 4MB flash)"
	@echo "r2j2         MIPS Release 2 MiniVPN (PPTPD; for 4MB flash)"
	@echo "r2f          MIPS Release 2 Mini (for netgear)"
	@echo ""
	@echo "n60e         Linksys E-series build VPN"
	@echo "n60c         Linksys E-series build BTGui-VPN"
	@echo "n60d         Linksys E-series build Nocat-VPN"
	@echo "n60t         Linksys E-series build BT-VPN"
	@echo "n60b         Linksys E-series build Big-VPN"
	@echo "n60o         Linksys E-series build Mega-VPN"
	@echo "n60v         Linksys E-series build IPv6-VPN for E2000"
	@echo "n60m         Linksys E-series build Max for E2000"
	@echo ""
	@echo "..etc..      other build configs"
	@echo "clean        -C router clean"
	@echo "cleanimage   rm -rf image"
	@echo "cleantools   clean btools, mksquashfs"
	@echo "cleankernel  -C Linux distclean (but preserves .config)"
	@echo "distclean    distclean of Linux & busybox (but preserve .configs)"
	@echo "prepk        -C Linux oldconfig dep"

.PHONY: all clean distclean cleanimage cleantools cleankernel prepk what setprofile help
.PHONY: a b c d m nc Makefile allversions tomato_profile.mak
