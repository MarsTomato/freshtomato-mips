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

	@echo "" >>fpkg.log
	@echo "***********************" `date` "************************" >>fpkg.log
	@cat router/shared/tomato_version >>fpkg.log
	@echo "" >>fpkg.log
	@cat router/target.info >>fpkg.log

ifneq ($(WNDR),y)
 ifeq ($(ASUS_TRX),0)
	# Create generic TRX image
	@echo "Creating Generic TRX Firmware"
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
		| tee -a fpkg.log

	@cp fpkg.log image/fpkg-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).log
 endif
endif
