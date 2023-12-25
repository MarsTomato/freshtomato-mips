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

ifneq ($(ASUS_TRX),0)
 ifeq ($(ASUS_TRX),R6300V1)
	$(MAKE) -C ctools
	ctools/objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/mipsel-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	# For mkchkimg, have to redirect stderr to stdout ... for some reason mkchkimg outputs to stderr (confirmed in source code!), 
	# and tee only reads from stdout (not stderr)
	@echo "*********************** Convert TRX to CHK (add Netgear Checksum) ************************" >>fpkg.log
	@echo "Creating Firmware for Netgear R6300 v1 .... "
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-Netgear-R6300V1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/linux-lzma.trx \
	-b U12H218T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@rm -f image/linux-lzma.trx
	@echo ""
 else
  ifeq ($(ASUS_TRX),WNDR4500V1)
	$(MAKE) -C ctools
	ctools/objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/mipsel-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	# For mkchkimg, have to redirect stderr to stdout ... for some reason mkchkimg outputs to stderr (confirmed in source code!), 
	# and tee only reads from stdout (not stderr)
	@echo "*********************** Convert TRX to CHK (add Netgear Checksum) ************************" >>fpkg.log
	@echo "Creating Firmware for Netgear WNDR4500 v1 .... "
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-Netgear-WNDR4500V1-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/linux-lzma.trx \
	-b U12H189T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@rm -f image/linux-lzma.trx
	@echo ""
  else
   ifeq ($(ASUS_TRX),WNDR4500V2)
	$(MAKE) -C ctools
	ctools/objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/mipsel-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	# For mkchkimg, have to redirect stderr to stdout ... for some reason mkchkimg outputs to stderr (confirmed in source code!), 
	# and tee only reads from stdout (not stderr)
	@echo "*********************** Convert TRX to CHK (add Netgear Checksum) ************************" >>fpkg.log
	@echo "Creating Firmware for Netgear WNDR4500 v2 .... "
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-Netgear-WNDR4500V2-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/linux-lzma.trx \
	-b U12H224T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@rm -f image/linux-lzma.trx
	@echo ""
   else
	$(MAKE) -C ctools
	ctools/objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux ctools/piggy
	ctools/lzma_4k e ctools/piggy  ctools/vmlinuz-lzma
	ctools/mksquashfs router/mipsel-uclibc/target ctools/target.squashfs -noappend -all-root
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
	ctools/trx_asus -i image/linux-lzma.trx -r $(ASUS_TRX),3.0.0.4,$(FORCE_SN),$(FORCE_EN),image/freshtomato-$(ASUS_TRX)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	@rm -f image/linux-lzma.trx
	@echo ""
   endif #WNDR4500V2
  endif  #WNDR4500V1
 endif   #R6300v1
endif    #ASUS_TRX

ifeq ($(WNR3500LV2),1)
	@echo "Creating Firmware for Netgear WNR3500L v2 .... "
	mipsel-uclibc-objcopy -O binary -g $(LINUXDIR)/vmlinux image/vmlinux.bin
	$(WNRTOOL)/lzma e image/vmlinux.bin image/vmlinux.lzma
	$(WNRTOOL)/trx -o image/freshtomato-wnr3500lv2.trx image/vmlinux.lzma $(SRCBASE)/router/mipsel-uclibc/target.image
	rm -f image/vmlinux.bin image/vmlinux.lzma
	cd image && touch rootfs
	cd image && $(WNRTOOL)/packet -k freshtomato-wnr3500lv2.trx -f rootfs -b $(BOARD_FILE) -ok kernel_image -oall kernel_rootfs_image -or rootfs_image -i $(fw_cfg_file) && rm -f rootfs && \
	cp kernel_rootfs_image.chk freshtomato-Netgear-3500Lv2-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
	@echo "Cleanup ...."
#	rm -f image/*image.chk image/*.trx
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

	@echo "" >>fpkg.log
	@echo "***********************" `date` "************************" >>fpkg.log
	@cat router/shared/tomato_version >>fpkg.log
	@echo "" >>fpkg.log
	@cat router/target.info >>fpkg.log

ifeq ($(WRT54),y)
ifneq ($(MIPS32),r2)
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l W54G,image/freshtomato-WRT54G_WRT54GL-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
		-l W54S,image/freshtomato-WRT54GS-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
		-l W54s,image/freshtomato-WRT54GSv4-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
		-l W54U,image/freshtomato-WRTSL54GS-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
		-m 0x10577050,image/freshtomato-WR850G-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
		| tee -a fpkg.log
endif
endif
ifeq ($(LINKSYS_E),y)
	# Linksys E-series(60k Nvram) images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l 1550,image/freshtomato-E1550-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 4200,image/freshtomato-E4200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 61XN,image/freshtomato-E3000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 32XN,image/freshtomato-E2000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(LINKSYS_E2500),y)
	# Linksys E2500(60k Nvram) image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l E25X,image/freshtomato-E2500-$(branch_rev)-$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(LINKSYS_E3200),y)
	# Linksys E3200/E2500v3(60k Nvram) image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l 25V3,image/freshtomato-E2500v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 3200,image/freshtomato-E3200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(LINKSYS_E_64k),y)
	# Linksys E-series(64k Nvram) images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l E800,image/freshtomato-E800-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l E900,image/freshtomato-E900-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l E122,image/freshtomato-E1200v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l E150,image/freshtomato-E1500-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(LINKSYS_E1000v2),y)
	# Linksys E1000v2/v2.1 images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l E100,image/freshtomato-E1000v2-v21-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l M010,image/freshtomato-Cisco-M10v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(LINKSYS_E1200v1),y)
	# Linksys E1200v1 images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-l E120,image/freshtomato-E1200v1-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(BELKIN_F5D),y)
	# Create Belkin F5D8235v3 image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-b 0x00017116,image/freshtomato-F5D8235v3-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(BELKIN_F7D),y)
	# Create Belkin F7D3301, F7D3302, F7D4302 images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-b 0x20100322,image/freshtomato-F7D3301-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-b 0x20090928,image/freshtomato-F7D3302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-b 0x20091006,image/freshtomato-F7D4302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
endif
ifeq ($(WNDR),y)
	@echo "Creating Firmware for Netgear WNDR Routers ..."
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
	-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	| tee -a fpkg.log
	# For mkchkimg, have to redirect stderr to stdout ... for some reason mkchkimg outputs to stderr (confirmed in source code!), 
	# and tee only reads from stdout (not stderr)
	@echo "*********************** Convert TRX to CHK (add Netgear Checksum) ************************" >>fpkg.log
	# Make multiple versions / files, as file is HW specific (HW information is captured in the .chk file itself!)
 ifeq ($(USBAP),y)
	# Make WNDR3400v2, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-WNDR3400v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	-b U12H187T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3400v2-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3400v3, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-WNDR3400v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	-b U12H208T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3400v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
 else
	# Make WNDR4000, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-WNDR4000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	-b U12H181T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR4000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3700v3, Checksum starts at 0x6FFFF8 => Max size (to not touch the last 64kB block) = 7274496
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-WNDR3700v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	-b U12H194T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@$(MAKE) netgear-check MAXFSIZE=7274496 NG_FNAME=image/freshtomato-WNDR3700v3-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk

	# Make WNDR3400, Checksum starts at 0x6CFFF8 => Max size (to not touch the last 64kB block) = 7077888
	@$(SRCBASE)/wnrtool/mkchkimg -o image/freshtomato-WNDR3400-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk \
	-k image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
	-b U12H155T00_NETGEAR -r 2 2>&1 | tee -a fpkg.log
	@$(MAKE) netgear-check MAXFSIZE=7077888 NG_FNAME=image/freshtomato-WNDR3400-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).chk
 endif
	@echo "Cleanup ...."
	@cp fpkg.log image/fpkg-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2).log
	@rm -f image/freshtomato-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx
	@rm -f fpkg.log
else
 ifeq ($(ASUS_TRX),0)
	# Create generic TRX image
	@echo "Creating Generic TRX Firmware"
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
		| tee -a fpkg.log

	@cp fpkg.log image/fpkg-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).log
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
