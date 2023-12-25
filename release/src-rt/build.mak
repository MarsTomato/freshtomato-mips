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

ifeq ($(LINKSYS_E),y)
	@rm -f image/freshtomato-E??00$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(branch_rev)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
endif
ifeq ($(BELKIN),y)
 ifneq ($(NVRAM_SIZE),60)
	@rm -f image/freshtomato-F7D????-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
	@rm -f image/freshtomato-F5D8235v3-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin
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
 ifeq ($(LINKSYS_E),y)
	# Linksys E-series build plus generic TRX image
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
		-l 4200,image/freshtomato-E4200-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 61XN,image/freshtomato-E3000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-l 32XN,image/freshtomato-E2000-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
 else
  ifeq ($(BELKIN),y)
   ifneq ($(NVRAM_SIZE),60)
	# Create Belkin images
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
		-b 0x20100322,image/freshtomato-F7D3301-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-b 0x20090928,image/freshtomato-F7D3302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-b 0x20091006,image/freshtomato-F7D4302-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		-b 0x00017116,image/freshtomato-F5D8235v3-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).bin \
		| tee -a fpkg.log
   endif
  else
	# Create generic TRX image
	@echo "Creating Generic TRX Firmware"
	@btools/fpkg -i lzma-loader/loader.gz -i $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz -a 1024 -i router/mipsel-uclibc/target.image \
		-t image/freshtomato$(if $(filter-out $(BUILD_FN),),$(shell echo -$(BUILD_FN)))-$(branch_rev)$(fn_BUILD_USB)$(fn_NVRAM_SIZE)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).trx \
		| tee -a fpkg.log
  endif
 endif
endif

	@cp fpkg.log image/fpkg-$(branch_rev)$(fn_BUILD_USB)-$(current_TOMATO_VER)$(beta)$(current_V2)-$(current_BUILD_DESC).log