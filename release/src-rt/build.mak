build_all:
	@echo ""
	@echo "Building FreshTomato $(branch_rev) $(current_BUILD_USB) $(current_TOMATO_VER)$(beta)$(current_V2) $(current_BUILD_DESC) $(current_BUILD_NAME) with $(TOMATO_PROFILE_NAME) Profile"
	@echo ""
	@echo ""

	@-mkdir image
	@$(MAKE) -C router all
	@$(MAKE) -C router install
	@$(MAKE) -C btools

	@echo "\033[41;1m   Creating image \033[0m\033]2;Creating image\007"

	@rm -f image/freshtomato-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	@rm -f image/freshtomato-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin

	$(eval current_BUILD_FN := $(if $(BUILD_FN),-$(BUILD_FN)))

ifeq ($(WNR3500LV2),1)
	@echo "Creating Firmware for Netgear WNR3500L v2 ..."
	mipsel-uclibc-objcopy -O binary -g $(LINUXDIR)/vmlinux image/vmlinux.bin
	$(WNRTOOL)/lzma e image/vmlinux.bin image/vmlinux.lzma
	$(call CREATE_INJECT_MODEL_WNR3500LV2,freshtomato-Netgear-3500Lv2-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk)
	cd image && touch rootfs
	cd image && $(WNRTOOL)/packet -k freshtomato-wnr3500lv2.trx -f rootfs -b $(BOARD_FILE) -ok kernel_image -oall kernel_rootfs_image -or rootfs_image -i $(fw_cfg_file) && rm -f rootfs && \
	cp kernel_rootfs_image.chk $(SAN_IMAGE)
	@echo "Cleanup ..."
	rm -f image/*image.chk image/*.trx
endif

ifeq ($(WRT54),y)
 ifneq ($(MIPS32),r2)
	@rm -f image/freshtomato-WRT54*-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-WRTSL54*-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-WR850G-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
 endif
endif
ifeq ($(LINKSYS_E),y)
	@rm -f image/freshtomato-E??????-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(LINKSYS_E_64k),y)
	@rm -f image/freshtomato-E??????-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(LINKSYS_E1200v1),y)
	@rm -f image/freshtomato-E1200v1-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(LINKSYS_E1000v2),y)
	@rm -f image/freshtomato-E1000v2-v21-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-Cisco-M10v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(LINKSYS_E2500),y)
	@rm -f image/freshtomato-E2500-$(branch_rev)-$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(LINKSYS_E3200),y)
	@rm -f image/freshtomato-E3200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-E2500v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(BELKIN_F5D),y)
	@rm -f image/freshtomato-F5D8235v3-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(BELKIN_F7D),y)
	@rm -f image/freshtomato-F7D????-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif

ifeq ($(WRT54),y)
ifneq ($(MIPS32),r2)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-WRT54G_WRT54GL-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,W54G)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-WRT54GS-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,W54S)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-WRT54GSv4-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,W54s)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-WRTSL54GS-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,W54U)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-WR850G-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-m,0x10577050)
endif
endif
ifeq ($(LINKSYS_E),y)
	# Linksys E-series(60k Nvram) images
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E1550-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,1550)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E4200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,4200)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E3000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,61XN)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E2000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,32XN)
endif
ifeq ($(LINKSYS_E2500),y)
	# Linksys E2500(60k Nvram) image
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E2500-$(branch_rev)-$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E25X)
endif
ifeq ($(LINKSYS_E3200),y)
	# Linksys E3200/E2500v3(60k Nvram) image
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E2500v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,25V3)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E3200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,3200)
endif
ifeq ($(LINKSYS_E_64k),y)
	# Linksys E-series(64k Nvram) images
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E800-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E800)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E900-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E900)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E1200v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E122)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E1500-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E150)
endif
ifeq ($(LINKSYS_E1000v2),y)
	# Linksys E1000v2/v2.1 images
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E1000v2-v21-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E100)
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-Cisco-M10v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,M010)
endif
ifeq ($(LINKSYS_E1200v1),y)
	# Linksys E1200v1 images
	$(call CREATE_INJECT_MODEL_LINKSYS,freshtomato-E1200v1-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,-l,E120)
endif
ifeq ($(BELKIN_F5D),y)
	# Create Belkin F5D8235v3 image
	$(call CREATE_INJECT_MODEL_BELKIN,freshtomato-F5D8235v3-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,0x00017116)
endif
ifeq ($(BELKIN_F7D),y)
	# Create Belkin F7D3301, F7D3302, F7D4302 images
	$(call CREATE_INJECT_MODEL_BELKIN,freshtomato-F7D3301-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,0x20100322)
	$(call CREATE_INJECT_MODEL_BELKIN,freshtomato-F7D3302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,0x20090928)
	$(call CREATE_INJECT_MODEL_BELKIN,freshtomato-F7D4302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin,0x20091006)
endif
ifeq ($(WNDR),y)
	@echo "Creating Firmware for Netgear WNDR Routers ..."
	# For mkchkimg, have to redirect stderr to stdout ... for some reason mkchkimg outputs to stderr (confirmed in source code!), 
	# and tee only reads from stdout (not stderr)
	@echo "*********************** Convert TRX to CHK (add Netgear Checksum) ************************" >>fpkg.log
	# Make multiple versions / files, as file is HW specific (HW information is captured in the .chk file itself!)
 ifeq ($(USBAP),y)
	# Make WNDR3400v2, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	$(call CREATE_INJECT_MODEL_WNDR,freshtomato-WNDR3400v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk,U12H187T00_NETGEAR)
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3400v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3400v3, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	$(call CREATE_INJECT_MODEL_WNDR,freshtomato-WNDR3400v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk,U12H208T00_NETGEAR)
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3400v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
 else
	# Make WNDR4000, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	$(call CREATE_INJECT_MODEL_WNDR,freshtomato-WNDR4000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk,U12H181T00_NETGEAR)
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR4000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3700v3, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	$(call CREATE_INJECT_MODEL_WNDR,freshtomato-WNDR3700v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk,U12H194T00_NETGEAR)
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3700v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3400, Checksum starts at 0x6CFFF8 => Max size (to not touch the last 64kB block) = 7077888
	$(call CREATE_INJECT_MODEL_WNDR,freshtomato-WNDR3400-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk,U12H155T00_NETGEAR)
	@$(MAKE) netgear-check MAXFSIZE=7077888 NG_FNAME=image/freshtomato-WNDR3400-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
 endif
else
 ifeq ($(ASUS_TRX),0)
	@echo "Creating Generic TRX Firmware (RT-N)"
	$(call CREATE_INJECT_MODEL_GEN,freshtomato$(current_BUILD_FN)-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx)
 endif
endif

netgear-check:
	$(eval FSIZE = $(shell stat -c %s $(NG_FNAME)))
	@if [ ${FSIZE} -gt ${MAXFSIZE} ] ; then \
		echo "************ Router Filesize exceeds Netgear Hardware limits - file will be deleted! ************"; \
		echo "             -> File to be deleted: " $(NG_FNAME); \
		rm $(NG_FNAME); \
		echo; \
	else \
		echo "Router Filesize meets Netgear Hardware limits - no action required."; \
		echo; \
	fi
