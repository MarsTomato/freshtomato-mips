include ../common.mak

CFLAGS	= -Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections -Wsign-compare
LIBS =
LDFLAGS	= -ffunction-sections -fdata-sections -Wl,--gc-sections

ifneq ($(TCONFIG_HTTPS),)
ifeq ($(TCONFIG_OPENSSL11),y)
CFLAGS	+= -I$(TOP)/openssl-1.1/include -DUSE_OPENSSL
LIBS += -L$(TOP)/openssl-1.1 -lssl -lcrypto
else
CFLAGS	+= -I$(TOP)/openssl/include -DUSE_OPENSSL
LIBS += -L$(TOP)/openssl -lssl -lcrypto
endif
else
CFLAGS	+= -I$(TOP)/cyassl/include -I$(TOP)/cyassl/ctaocrypt/include
LIBS += $(TOP)/cyassl/src/.libs/libcyassl.a
endif

LIBS += -lpthread
OBJ = mssl.o

all: libmssl.so libmssl.a

libmssl.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS) $(LIBS)
	$(STRIP) $@
	$(CPTMP)

libmssl.a: $(OBJ)
	rm -f $@
	$(AR) cruv $@ $^

clean:
	rm -f .*.depend *.o *.so *.a

install: all
	@echo " [mssl] Installing..."
	install -D libmssl.so $(INSTALLDIR)/usr/lib/libmssl.so
	$(STRIP) $(INSTALLDIR)/usr/lib/libmssl.so

%.o: %.c .%.depend
	@echo " [mssl] CC $@"
	@$(CC) $(CFLAGS) -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
