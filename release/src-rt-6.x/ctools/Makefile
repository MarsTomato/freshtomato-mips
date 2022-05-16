include ../router/.config

CC = gcc
CFLAGS = -O3 -fno-delete-null-pointer-checks -Wall
ifeq ($(TCONFIG_BCMWL6),y)
CFLAGS += -DBCMWL6
endif

all: trx_asus

trx_asus: trx_asus.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o trx_asus piggy target.squashfs vmlinuz-lzma

%.o: %.c
	$(CC) $(CFLAGS) -c $<
