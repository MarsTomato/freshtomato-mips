include ../common.mak

CFLAGS = -Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC -Wsign-compare
CFLAGS += -I. -I$(SRCBASE)/include -I$(SRCBASE)/

LDFLAGS = --gc-sections -fPIC

OBJS = shutils.o wl.o wl_linux.o linux_timer.o defaults.o id.o misc.o \
       led.o version.o base64.o files.o strings.o process.o ifaddrs.o nvparse.o

ifeq ($(TCONFIG_USB),y)
OBJS += usb.o \
        $(TOP)/busybox/util-linux/volume_id/ext.o \
        $(TOP)/busybox/util-linux/volume_id/util2.o \
        $(TOP)/busybox/util-linux/volume_id/fat.o \
        $(TOP)/busybox/util-linux/volume_id/ntfs.o \
        $(TOP)/busybox/util-linux/volume_id/linux_swap.o
endif

ifeq ($(CONFIG_BCMWL5),y)
OBJS += wlif_utils.o UdpLib.o
endif

all: libshared.so libshared.a

libshared.so: $(OBJS)
	@echo " [shared] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^

	$(SIZECHECK)
	$(CPTMP)

libshared.a: $(OBJS)
	@echo " [shared] AR $@"
	@rm -f $@
	@$(AR) cruv $@ $^

install: all
	@echo " [shared] Installing as $(INSTALLDIR)/usr/lib/libshared.so"
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libshared.so $(INSTALLDIR)/usr/lib
	@$(STRIP) $(INSTALLDIR)/usr/lib/libshared.so

clean:
	rm -f *.o *.so *.a .*.depend

%.o: %.c .%.depend
	@echo " [shared] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
