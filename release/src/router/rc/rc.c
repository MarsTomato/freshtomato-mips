/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/


#include "rc.h"


#ifdef DEBUG_RCTEST
/* used for various testing */
static int rctest_main(int argc, char *argv[])
{
	if (argc < 2)
		printf("test what?\n");
	else {
		printf("%s\n", argv[1]);

		if (strcmp(argv[1], "foo") == 0) {
		}
		else if (strcmp(argv[1], "qos") == 0)
			start_qos();

#ifdef TCONFIG_FANCTRL
		else if (strcmp(argv[1], "phy_tempsense") == 0) {
			stop_phy_tempsense();
			start_phy_tempsense();
		}
#endif
#ifdef TCONFIG_IPV6
		else if (strcmp(argv[1], "6rd") == 0) {
			stop_6rd_tunnel();
			start_6rd_tunnel();
		}
#endif
#ifdef DEBUG_IPTFILE
		else if (strcmp(argv[1], "iptfile") == 0)
			create_test_iptfile();
#endif
		else
			printf("what?\n");
	}
	return 0;
}
#endif /* DEBUG_RCTEST */

static int hotplug_main(int argc, char *argv[])
{
	if (argc >= 2) {
		if (strcmp(argv[1], "net") == 0)
			hotplug_net();
#ifdef TCONFIG_USB
		else if (strcmp(argv[1], "usb") == 0)
			hotplug_usb();
		else if (strcmp(argv[1], "block") == 0)
			hotplug_usb();
#endif
	}
	return 0;
}

static int rc_main(int argc, char *argv[])
{
	if (argc < 2)
		return 0;
	if (strcmp(argv[1], "start") == 0)
		return kill(1, SIGUSR2);
	if (strcmp(argv[1], "stop") == 0)
		return kill(1, SIGINT);
	if (strcmp(argv[1], "restart") == 0)
		return kill(1, SIGHUP);

	return 0;
}

typedef struct {
	const char *name;
	int (*main)(int argc, char *argv[]);
} applets_t;

static const applets_t applets[] = {
#ifdef TCONFIG_BCMARM
	{ "preinit",			init_main			},
#endif
	{ "init",			init_main			},
	{ "console",			console_main			},
	{ "rc",				rc_main				},
	{ "ip-up",			ipup_main			},
	{ "ip-down",			ipdown_main			},
	{ "ip-pre-up",			ippreup_main			},
#ifdef TCONFIG_IPV6
	{ "ipv6-up",			ip6up_main			},
	{ "ipv6-down",			ip6down_main			},
#endif
#ifdef TCONFIG_PPTPD
	{ "pptpc_ip-up",		pptpc_ipup_main			},
	{ "pptpc_ip-down",		pptpc_ipdown_main		},
#endif
	{ "ppp_event",			pppevent_main			},
	{ "hotplug",			hotplug_main			},
	{ "redial",			redial_main			},
	{ "mwanroute",			mwan_route_main			},
	{ "listen",			listen_main			},
	{ "service",			service_main			},
	{ "sched",			sched_main			},
#ifdef TCONFIG_BCMARM
	{ "mtd-write",			mtd_write_main_old		},
	{ "mtd-erase",			mtd_unlock_erase_main_old	},
	{ "mtd-unlock",			mtd_unlock_erase_main_old	},
#else
	{ "mtd-write",			mtd_write_main			},
	{ "mtd-erase",			mtd_unlock_erase_main		},
	{ "mtd-unlock",			mtd_unlock_erase_main		},
#endif
	{ "buttons",			buttons_main			},
#if defined(TCONFIG_BCMARM) || defined(CONFIG_BLINK)
	{ "blink",			blink_main			},
	{ "blink_br",			blink_br_main			},
#endif
#ifdef TCONFIG_FANCTRL
	{ "phy_tempsense",		phy_tempsense_main		},
#endif
	{ "rcheck",			rcheck_main			},
	{ "dhcpc-event",		dhcpc_event_main		},
	{ "dhcpc-release",		dhcpc_release_main		},
	{ "dhcpc-renew",		dhcpc_renew_main		},
#ifdef TCONFIG_IPV6
	{ "dhcp6c-state",		dhcp6c_state_main		},
#endif
	{ "radio",			radio_main			},
	{ "led",			led_main			},
	{ "halt",			reboothalt_main			},
	{ "reboot",			reboothalt_main			},
	{ "gpio",			gpio_main			},
	{ "wldist",			wldist_main			},
#ifdef TCONFIG_CIFS
	{ "mount-cifs",			mount_cifs_main			},
#endif
#ifdef TCONFIG_DDNS
	{ "ddns-update",		ddns_update_main		},
#endif
#ifdef DEBUG_RCTEST
	{ "rctest",			rctest_main			},
#endif
	{NULL, NULL}
};

#ifdef TCONFIG_BCMARM
void erase_nvram(void)
{
	eval("mtd-erase2", "nvram");
}
#endif

int main(int argc, char **argv)
{
	char *base;
	int f;

	/*
	 * Make sure std* are valid since several functions attempt to close these
	 * handles. If nvram_*() runs first, nvram=0, nvram gets closed
	*/
	if ((f = open("/dev/null", O_RDWR)) < 3) {
		dup(f);
		dup(f);
	}
	else
		close(f);

	base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];

	if (strcmp(base, "rc") == 0) {
		if (argc < 2)
			return 1;
		if (strcmp(argv[1], "start") == 0)
			return kill(1, SIGUSR2);
		if (strcmp(argv[1], "stop") == 0)
			return kill(1, SIGINT);
		if (strcmp(argv[1], "restart") == 0)
			return kill(1, SIGHUP);

		++argv;
		--argc;
		base = argv[0];
	}

#ifdef DEBUG_NOISY
	if (nvram_match("debug_logrc", "1")) {
		int i;

		cprintf("[rc %d] ", getpid());
		for (i = 0; i < argc; ++i) {
			cprintf("%s ", argv[i]);
		}
		cprintf("\n");

	}

	if (nvram_match("debug_ovrc", "1")) {
		char tmp[256];
		char *a[32];

		realpath(argv[0], tmp);
		if ((strncmp(tmp, "/tmp/", 5) != 0) && (argc < 32)) {
			memset(tmp, 0, 256);
			sprintf(tmp, "%s%s", "/tmp/", base);
			if (f_exists(tmp)) {
				cprintf("[rc] override: %s\n", tmp);
				memcpy(a, argv, argc * sizeof(a[0]));
				a[argc] = 0;
				a[0] = tmp;
				execvp(tmp, a);
				exit(0);
			}
		}
	}
#endif /* DEBUG_NOISY */

	const applets_t *a;
	for (a = applets; a->name; ++a) {
		if (strcmp(base, a->name) == 0) {
			openlog(base, LOG_PID, LOG_USER);
			return a->main(argc, argv);
		}
	}

#ifdef TCONFIG_BCMARM
	if (!strcmp(base, "nvram_erase")) {
		erase_nvram();
		return 0;
	}
	/* mtd-erase2 [device] */
	else if (!strcmp(base, "mtd-erase2")) {
		if (argv[1] && ((!strcmp(argv[1], "boot")) ||
				(!strcmp(argv[1], "linux")) ||
				(!strcmp(argv[1], "linux2")) ||
				(!strcmp(argv[1], "rootfs")) ||
				(!strcmp(argv[1], "rootfs2")) ||
				(!strcmp(argv[1], "brcmnand")) ||
				(!strcmp(argv[1], "nvram")) ||
				(!strcmp(argv[1], "crash")))) {
			return mtd_erase(argv[1]);
		}
		else {
			fprintf(stderr, "usage: mtd-erase2 [device]\n");
			return EINVAL;
		}
	}
	/* mtd-write2 [path] [device] */
	else if (!strcmp(base, "mtd-write2")) {
		if (argc >= 3)
			return mtd_write(argv[1], argv[2]);
		else {
			fprintf(stderr, "usage: mtd-write2 [path] [device]\n");
			return EINVAL;
		}
	}
#endif

	printf("Unknown applet: %s\n", base);
	return 0;
}
