/*

	Copyright 2003, CyberTAN  Inc.  All Rights Reserved

	This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
	the contents of this file may not be disclosed to third parties,
	copied or duplicated in any form without the prior written
	permission of CyberTAN Inc.

	This software should be used as a reference only, and it not
	intended for production use!

	THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE

*/
/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

 */
/*

	Modified for Tomato Firmware
	Portions, Copyright (C) 2006-2009 Jonathan Zarate

*/


#include "rc.h"

#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

/* needed by logmsg() */
#define LOGMSG_DISABLE	DISABLE_SYSLOG_OS
#define LOGMSG_NVDEBUG	"dhcp_debug"


static void expires(unsigned int seconds, char *prefix)
{
	struct sysinfo info;
	char s[32];
	char expires_file[64];

	sysinfo(&info);
	memset(s, 0, 32);
	sprintf(s, "%u", (unsigned int)info.uptime + seconds);

	memset(expires_file, 0, 64);
	sprintf(expires_file, "/var/lib/misc/dhcpc-%s.expires", prefix);
	f_write_string(expires_file, s, 0, 0);
}

static void do_renew_file(unsigned int renew, char *prefix)
{
	char buf[64];

	memset(buf, 0, 64);
	sprintf(buf, "/var/lib/misc/%s_dhcpc.renewing", prefix);

	if (renew)
		f_write(buf, NULL, 0, 0, 0);
	else
		unlink(buf);
}

void do_connect_file(unsigned int connect, char *prefix)
{
	char buf[64];

	memset(buf, 0, 64);
	sprintf(buf, "/var/lib/misc/%s.connecting", prefix);

	if (connect)
		f_write(buf, NULL, 0, 0, 0);
	else
		unlink(buf);
}

/* copy env to nvram
 * returns 1 if new/changed, 0 if not changed/no env
 */
static int env2nv(char *env, char *nv)
{
	char *value;
	if ((value = getenv(env)) != NULL) {
		if (!nvram_match(nv, value)) {
			nvram_set(nv, value);
			return 1;
		}
	}
	return 0;
}

static int env2nv_gateway(const char *nv)
{
	char *v, *g;
	char *b;
	int r = 0;

	if ((v = getenv("router")) != NULL) {
		if ((b = strdup(v)) != NULL) {
			if ((v = strchr(b, ' ')) != NULL) /* truncate multiple entries */
				*v = 0;
			if (!nvram_match((char *)nv, b)) {
				nvram_set(nv, b);
				r = 1;
			}
			free(b);
		}
	}
	else if ((v = getenv("staticroutes")) != NULL) {
		if ((b = strdup(v)) == NULL)
			return 0;

		v = b;
		while ((g = strsep(&v, " ")) != NULL) {
			if (strcmp(g, "0.0.0.0/0") == 0) {
				if ((g = strsep(&v, " ")) && *g) {
					if (!nvram_match((char *)nv, g)) {
						nvram_set(nv, g);
						r = 1;
					}
					break;
				}
			}
		}
		free(b);
	}

	return r;
}

static int deconfig(char *ifname, char *prefix)
{
	char tmp[32];

	ifconfig(ifname, IFUP, "0.0.0.0", NULL);

	if (using_dhcpc(prefix)) {
		nvram_set(strcat_r(prefix, "_ipaddr", tmp), "0.0.0.0");
		nvram_set(strcat_r(prefix, "_netmask", tmp), "0.0.0.0");
		nvram_set(strcat_r(prefix, "_gateway", tmp), "0.0.0.0");
	}
	nvram_set(strcat_r(prefix, "_lease", tmp), "0");
	nvram_set(strcat_r(prefix, "_routes1", tmp), "");
	nvram_set(strcat_r(prefix, "_routes2", tmp), "");
	expires(0, prefix);

	if ((get_wanx_proto(prefix) == WP_DHCP) || (get_wanx_proto(prefix) == WP_LTE)) {
		nvram_set(strcat_r(prefix, "_netmask", tmp), "0.0.0.0");
		nvram_set(strcat_r(prefix, "_gateway_get", tmp), "0.0.0.0");
		nvram_set(strcat_r(prefix, "_get_dns", tmp), "");
	}

#ifdef TCONFIG_IPV6
	nvram_set("wan_6rd", "");
#endif

	return 0;
}

static int bound(char *ifname, int renew, char *prefix)
{
	char tmp [32];
	char *netmask, *dns, *gw;
	int wan_proto = get_wanx_proto(prefix);

	do_renew_file(0, prefix);

	logmsg(LOG_DEBUG, "*** IN %s: interface=%s, wan_prefix=%s, renew=%d, proto=%d", __FUNCTION__, ifname, prefix, renew, wan_proto);

	nvram_set(strcat_r(prefix, "_routes1", tmp), "");
	nvram_set(strcat_r(prefix, "_routes2", tmp), "");
	env2nv("ip", strcat_r(prefix, "_ipaddr", tmp));
	env2nv_gateway(strcat_r(prefix, "_gateway", tmp));
	env2nv("dns", strcat_r(prefix, "_get_dns", tmp));
	env2nv("domain", strcat_r(prefix, "_get_domain", tmp));
	env2nv("lease", strcat_r(prefix, "_lease", tmp));
	netmask = getenv("subnet") ? : "255.255.255.255";
	if ((wan_proto == WP_DHCP) || (wan_proto == WP_LTE) || (using_dhcpc(prefix))) { /* netmask for DHCP MAN */
		nvram_set(strcat_r(prefix, "_netmask", tmp), netmask);
		nvram_set(strcat_r(prefix, "_gateway_get", tmp), nvram_safe_get(strcat_r(prefix, "_gateway", tmp)));
	}

	/* RFC3442: If the DHCP server returns both a Classless Static Routes option
	 * and a Router option, the DHCP client MUST ignore the Router option.
	 * Similarly, if the DHCP server returns both a Classless Static Routes
	 * option and a Static Routes option, the DHCP client MUST ignore the
	 * Static Routes option.
	 * Ref: http://www.faqs.org/rfcs/rfc3442.html
	 */
	/* Classless Static Routes (option 121) */
	if (!env2nv("staticroutes", strcat_r(prefix, "_routes1", tmp)))
		/* or MS Classless Static Routes (option 249) */
		env2nv("msstaticroutes", strcat_r(prefix, "_routes1", tmp));

	/* Static Routes (option 33) */
	env2nv("routes", strcat_r(prefix, "_routes2", tmp));

	expires(atoi(safe_getenv("lease")), prefix);

#ifdef TCONFIG_IPV6
	env2nv("ip6rd", "wan_6rd");
#endif

	logmsg(LOG_DEBUG, "*** %s: %s_ipaddr=%s", __FUNCTION__, prefix, nvram_safe_get(strcat_r(prefix, "_ipaddr", tmp)));
	logmsg(LOG_DEBUG, "*** %s: %s_netmask=%s", __FUNCTION__, prefix, netmask);
	logmsg(LOG_DEBUG, "*** %s: %s_gateway=%s", __FUNCTION__, prefix, nvram_safe_get(strcat_r(prefix, "_gateway", tmp)));
	logmsg(LOG_DEBUG, "*** %s: %s_get_dns=%s", __FUNCTION__, prefix, nvram_safe_get(strcat_r(prefix, "_get_dns", tmp)));
	logmsg(LOG_DEBUG, "*** %s: %s_routes1=%s", __FUNCTION__, prefix, nvram_safe_get(strcat_r(prefix, "_routes1", tmp)));
	logmsg(LOG_DEBUG, "*** %s: %s_routes2=%s", __FUNCTION__, prefix, nvram_safe_get(strcat_r(prefix, "_routes2", tmp)));

	ifconfig(ifname, IFUP, "0.0.0.0", NULL);
	ifconfig(ifname, IFUP, nvram_safe_get(strcat_r(prefix, "_ipaddr", tmp)), netmask);

	if ((wan_proto != WP_DHCP) && (wan_proto != WP_LTE)) {

		/* setup dnsmasq and routes to dns / access servers */
		gw = nvram_safe_get(strcat_r(prefix, "_gateway", tmp));
		if ((*gw) && (strcmp(gw, "0.0.0.0") != 0)) {
			logmsg(LOG_DEBUG, "*** %s: do preset_wan ... ifname=%s gateway=%s netmask=%s prefix=%s", __FUNCTION__, ifname, gw, netmask, prefix);
			preset_wan(ifname, gw, netmask, prefix);
		}
		else {
			logmsg(LOG_DEBUG, "*** %s: NO gateway! Just do DHCP DNS stuff ...", __FUNCTION__);
			dns_to_resolv();
		}
		/* don't clear dns servers for PPTP/L2TP wans, required for pptp/l2tp server name resolution */
		dns = nvram_safe_get(strcat_r(prefix, "_get_dns", tmp));
		if (wan_proto != WP_PPTP && wan_proto != WP_L2TP) {
			nvram_set(strcat_r(prefix, "_get_dns", tmp), renew ? dns : "");
			logmsg(LOG_DEBUG, "*** %s: clear / set dns to resolv.conf", __FUNCTION__);
		}
		switch (wan_proto) {
		case WP_PPTP:
			logmsg(LOG_DEBUG, "*** %s: start_pptp(%s) ...", __FUNCTION__, prefix);
			start_pptp(prefix);
			break;
		case WP_L2TP:
			logmsg(LOG_DEBUG, "*** %s: start_l2tp(%s) ...", __FUNCTION__, prefix);
			start_l2tp(prefix);
			break;

		case WP_PPPOE:
			logmsg(LOG_DEBUG, "*** %s: start_pppoe(%s) ...", __FUNCTION__, prefix);

			if (!strcmp(prefix, "wan"))
				start_pppoe(PPPOEWAN, prefix);
			else if (!strcmp(prefix, "wan2"))
				start_pppoe(PPPOEWAN2, prefix);
#ifdef TCONFIG_MULTIWAN
			else if (!strcmp(prefix, "wan3"))
				start_pppoe(PPPOEWAN3, prefix);
			else if (!strcmp(prefix, "wan4"))
				start_pppoe(PPPOEWAN4, prefix);
#endif
			break;
		}
	}
	else {
		logmsg(LOG_DEBUG, "*** OUT %s: to start_wan_done, ifname=%s prefix=%s ...", __FUNCTION__, ifname, prefix);
		start_wan_done(ifname,prefix);
	}

	return 0;
}

static int renew(char *ifname, char *prefix)
{
	char *a;
	int changed = 0, routes_changed = 0;
	int wan_proto = get_wanx_proto(prefix);
	char tmp[32];

	logmsg(LOG_DEBUG, "*** %s: interface=%s, wan_prefix=%s", __FUNCTION__, ifname, prefix);

	do_renew_file(0, prefix);

	if ((env2nv("ip", strcat_r(prefix, "_ipaddr", tmp))) ||
	    (env2nv_gateway(strcat_r(prefix, "_gateway", tmp))) ||
	    (wan_proto == WP_LTE && env2nv("subnet", strcat_r(prefix, "_netmask", tmp))) ||
	    (wan_proto == WP_DHCP && env2nv("subnet", strcat_r(prefix, "_netmask", tmp)))) {
		/* WAN IP or gateway changed, restart/reconfigure everything */
		logmsg(LOG_DEBUG, "*** %s: WAN IP or gateway changed, restart/reconfigure everything", __FUNCTION__);

		return bound(ifname, 1, prefix);
	}

	if ((wan_proto == WP_DHCP) || (wan_proto == WP_LTE)) {
		changed |= env2nv("domain", strcat_r(prefix, "_get_domain", tmp));
		changed |= env2nv("dns", strcat_r(prefix, "_get_dns", tmp));
	}

	nvram_set(strcat_r(prefix, "_routes1_save", tmp), nvram_safe_get(strcat_r(prefix, "_routes1", tmp)));
	nvram_set(strcat_r(prefix, "_routes2_save", tmp), nvram_safe_get(strcat_r(prefix, "_routes2", tmp)));

	/* Classless Static Routes (option 121) or MS Classless Static Routes (option 249) */
	if (getenv("staticroutes"))
		routes_changed |= env2nv("staticroutes", strcat_r(prefix, "_routes1_save", tmp));
	else
		routes_changed |= env2nv("msstaticroutes", strcat_r(prefix, "_routes1_save", tmp));
	/* Static Routes (option 33) */
	routes_changed |= env2nv("routes", strcat_r(prefix, "_routes2_save", tmp));

	changed |= routes_changed;

	if ((a = getenv("lease")) != NULL) {
		nvram_set(strcat_r(prefix, "_lease", tmp), a);
		expires(atoi(a), prefix);
	}

	if (changed) {
		set_host_domain_name();
		start_dnsmasq();
	}

	if (routes_changed) {
		do_wan_routes(ifname, 0, 0, prefix);
		nvram_set(strcat_r(prefix, "_routes1", tmp), nvram_safe_get(strcat_r(prefix, "_routes1_save", tmp)));
		nvram_set(strcat_r(prefix, "_routes2", tmp), nvram_safe_get(strcat_r(prefix, "_routes2_save", tmp)));
		do_wan_routes(ifname, 0, 1, prefix);
	}
	nvram_unset(strcat_r(prefix, "_routes1_save", tmp));
	nvram_unset(strcat_r(prefix, "_routes2_save", tmp));

	return 0;
}


int dhcpc_event_main(int argc, char **argv)
{
	char *ifname;
	ifname = getenv("interface");
	char prefix[] = "wanXX";

	if (nvram_match("wan2_ifname", ifname))
		strcpy(prefix, "wan2");
	else if (nvram_match("wan2_iface", ifname))
		strcpy(prefix, "wan2");
#ifdef TCONFIG_MULTIWAN
	else if (nvram_match("wan3_ifname", ifname))
		strcpy(prefix, "wan3");
	else if (nvram_match("wan3_iface", ifname))
		strcpy(prefix, "wan3");
	else if (nvram_match("wan4_ifname", ifname))
		strcpy(prefix, "wan4");
	else if (nvram_match("wan4_iface", ifname))
		strcpy(prefix, "wan4");
#endif
	else
		strcpy(prefix, "wan");

	if (!wait_action_idle(10))
		return 1;

	logmsg(LOG_DEBUG, "*** %s: interface=%s wan_prefix=%s event=%s", __FUNCTION__, ifname, prefix, argv[1]);

	if ((argc == 2) && (ifname = getenv("interface")) != NULL) {
		if (strcmp(argv[1], "deconfig") == 0)
			return deconfig(ifname, prefix);
		if (strcmp(argv[1], "bound") == 0)
			return bound(ifname, 0, prefix);
		if ((strcmp(argv[1], "renew") == 0) || (strcmp(argv[1], "update") == 0))
			return renew(ifname,prefix);
	}

	return 1;
}

int dhcpc_release_main(int argc, char **argv)
{
	char prefix[] = "wanXX";
	char dhcpc_file[64];

	if (argc > 1)
		strcpy(prefix, argv[1]);
	else
		strcpy(prefix, "wan");

	logmsg(LOG_DEBUG, "*** %s: argc=%d wan_prefix=%s", __FUNCTION__, argc, prefix);

	mwan_table_del(prefix); /* for dual WAN and multi WAN */

	if (!using_dhcpc(prefix))
		return 1;

	memset(dhcpc_file, 0, 64);
	sprintf(dhcpc_file, "/var/run/udhcpc-%s.pid", prefix);
	if (kill_pidfile_s(dhcpc_file, SIGUSR2) == 0)
		sleep(2);

	do_renew_file(0, prefix);

	do_connect_file(0, prefix);

	mwan_load_balance(); /* for dual WAN and multi WAN */

	/* WAN LED control */
	wan_led_off(prefix);

	return 0;
}

int dhcpc_renew_main(int argc, char **argv)
{
	char prefix[] = "wanXX";
	char dhcpcpid_file[64];

	if (argc > 1)
		strcpy(prefix, argv[1]);
	else
		strcpy(prefix, "wan");

	logmsg(LOG_DEBUG, "*** %s: argc=%d wan_prefix=%s", __FUNCTION__, argc, prefix);

	if (!using_dhcpc(prefix))
		return 1;

	memset(dhcpcpid_file, 0, 64);
	sprintf(dhcpcpid_file, "/var/run/udhcpc-%s.pid", prefix);
	if (kill_pidfile_s(dhcpcpid_file, SIGUSR1) == 0)
		do_renew_file(1, prefix);
	else {
		stop_dhcpc(prefix);
		start_dhcpc(prefix);
	}

	return 0;
}

void start_dhcpc(char *prefix)
{
	char dhcpcpid_file[64];
	char cmd[256];
	char tmp[32];
	char *ifname;
	int proto;

	nvram_set(strcat_r(prefix, "_get_dns", tmp), "");

	do_renew_file(1, prefix);

	proto = get_wanx_proto(prefix);

	if (proto == WP_LTE)
		ifname = nvram_safe_get(strcat_r(prefix, "_modem_if", tmp));
	else
		ifname = nvram_safe_get(strcat_r(prefix, "_ifname", tmp));

	if ((proto == WP_DHCP) || (proto == WP_LTE))
		nvram_set(strcat_r(prefix, "_iface", tmp), ifname);

	memset(dhcpcpid_file, 0, 64);
	sprintf(dhcpcpid_file, "/var/run/udhcpc-%s.pid", prefix);
	snprintf(cmd, sizeof(cmd), "udhcpc -i %s -b -s dhcpc-event %s %s %s %s %s %s %s -p %s",
	                           ifname,
	                           nvram_invmatch("wan_hostname", "") ? "-H" : "", nvram_safe_get("wan_hostname"),
	                           /* This params required to get static / classless routes from DHCP server */
	                           nvram_get_int("dhcp_routes") ? "-O 33 -O 121 -O 249" : "",
	                           nvram_get_int("dhcpc_minpkt") ? "-m" : "",
	                           nvram_contains_word("log_events", "dhcpc") ? "-S" : "",
	                           nvram_safe_get("dhcpc_custom"),
#ifdef TCONFIG_IPV6
	                           (get_ipv6_service() == IPV6_6RD_DHCP) ? "-O ip6rd" : ""
#else
	                           ""
#endif
	                           ,
	                           dhcpcpid_file
	);

	logmsg(LOG_DEBUG, "*** %s: prefix=%s cmd = /bin/sh -c %s", __FUNCTION__, prefix, cmd);

	xstart("/bin/sh", "-c", cmd);
}

void stop_dhcpc(char *prefix)
{
	char dhcpcpid_file[64];

	killall("dhcpc-event", SIGTERM);

	memset(dhcpcpid_file, 0, 64);
	sprintf(dhcpcpid_file, "/var/run/udhcpc-%s.pid", prefix);
	if (kill_pidfile_s(dhcpcpid_file, SIGUSR2) == 0) /* release */
		sleep(2);

	kill_pidfile_s(dhcpcpid_file, SIGTERM);
	unlink(dhcpcpid_file);

	do_renew_file(0, prefix);

	/* WAN LED control */
	wan_led_off(prefix);
}

#ifdef TCONFIG_IPV6
int dhcp6c_state_main(int argc, char **argv)
{
	char prefix[INET6_ADDRSTRLEN];
	const char *lanif;
	struct in6_addr addr;
	int i, r;

	if (!wait_action_idle(10))
		return 1;

	lanif = getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, 0);

	/* check IPv6 addr - change/new ? */
	if (!nvram_match("ipv6_rtr_addr", (char *) lanif)) {
		nvram_set("ipv6_rtr_addr", lanif);
		/* extract prefix from configured IPv6 address */
		if (inet_pton(AF_INET6, nvram_safe_get("ipv6_rtr_addr"), &addr) > 0) {
			r = nvram_get_int("ipv6_prefix_length") ? : 64;
			for (r = 128 - r, i = 15; r > 0; r -= 8) {
				if (r >= 8) {
					addr.s6_addr[i--] = 0;
				} else {
					addr.s6_addr[i--] &= (0xff << r);
				}
			}
			inet_ntop(AF_INET6, &addr, prefix, sizeof(prefix));
			nvram_set("ipv6_prefix", prefix);
		}
		/* (re)start dnsmasq and httpd */
		set_host_domain_name();
		start_dnsmasq();
		start_httpd();
	}

	/* check DNS */
	if (env2nv("new_domain_name_servers", "ipv6_get_dns"))
		dns_to_resolv();

	return 0;
}

void start_dhcp6c(void)
{
	FILE *f;
	int prefix_len;
	char *wan6face;
	char *argv[] = { "dhcp6c", "-T", "LL", NULL, NULL, NULL }; /* DUID type 3 (DUID-LL) */
	int argc;
	int ipv6_vlan = 0; /* bit 0 = IPv6 enabled for LAN1, bit 1 = IPv6 enabled for LAN2, bit 2 = IPv6 enabled for LAN3, 1 == TRUE, 0 == FALSE */

	/* Check if turned on */
	if (get_ipv6_service() != IPV6_NATIVE_DHCP)
		return;

	prefix_len = 64 - (nvram_get_int("ipv6_prefix_length") ? : 64);
	if (prefix_len < 0)
		prefix_len = 0;

	wan6face = nvram_safe_get("wan_iface");
	ipv6_vlan = nvram_get_int("ipv6_vlan");

	nvram_set("ipv6_get_dns", "");
	nvram_set("ipv6_rtr_addr", "");
	nvram_set("ipv6_prefix", "");

	nvram_set("ipv6_pd_pltime", "0"); /* reset preferred and valid lifetime */
	nvram_set("ipv6_pd_vltime", "0");

	/* Create dhcp6c.conf */
	unlink("/var/dhcp6c_duid");
	if ((f = fopen("/etc/dhcp6c.conf", "w"))) {
		fprintf(f, "interface %s {\n", wan6face);

		if (nvram_get_int("ipv6_pdonly") == 0)
			fprintf(f, " send ia-na 0;\n");

		fprintf(f, " send ia-pd 0;\n"
		           " request domain-name-servers;\n"
		           " script \"/sbin/dhcp6c-state\";\n"
		           "};\n"
		           "id-assoc pd 0 {\n"
		           " prefix ::/%d infinity;\n"
		           " prefix-interface %s {\n"
		           "  sla-id 0;\n"
		           "  sla-len %d;\n"
		           "  ifid 1;\n" /* override the default EUI-64 address selection and create a very userfriendly address --> ::1 */
		           " };\n",
		           nvram_get_int("ipv6_prefix_length"),
		           nvram_safe_get("lan_ifname"),
		           prefix_len);

		/* check IPv6 for LAN1 */
		if ((ipv6_vlan & 0x01) && (prefix_len >= 1) && (strcmp(nvram_safe_get("lan1_ipaddr"), "") != 0)) /* 2x IPv6 /64 networks possible --> for LAN and LAN1 */
			fprintf(f, " prefix-interface %s {\n"
			           "  sla-id 1;\n"
			           "  sla-len %d;\n"
			           "  ifid 1;\n" /* override the default EUI-64 address selection and create a very userfriendly address --> ::1 */
			           " };\n", nvram_safe_get("lan1_ifname"), prefix_len);

		/* check IPv6 for LAN2 */
		if ((ipv6_vlan & 0x02) && (prefix_len >= 2) && (strcmp(nvram_safe_get("lan2_ipaddr"), "") != 0)) /* 4x IPv6 /64 networks possible --> for LAN to LAN2 */
			fprintf(f, " prefix-interface %s {\n"
		                   "  sla-id 2;\n"
		                   "  sla-len %d;\n"
		                   "  ifid 1;\n" /* override the default EUI-64 address selection and create a very userfriendly address --> ::1 */
		                   " };\n", nvram_safe_get("lan2_ifname"), prefix_len);

		/* check IPv6 for LAN3 */
		if ((ipv6_vlan & 0x04) && (prefix_len >= 2) && (strcmp(nvram_safe_get("lan3_ipaddr"), "") != 0)) /* 4x IPv6 /64 networks possible --> for LAN to LAN3 */
			fprintf(f, " prefix-interface %s {\n"
			           "  sla-id 3;\n"
			           "  sla-len %d;\n"
			           "  ifid 1;\n" /* override the default EUI-64 address selection and create a very userfriendly address --> ::1 */
			           " };\n", nvram_safe_get("lan3_ifname"), prefix_len);

		fprintf(f, "};\n"
		           "id-assoc na 0 { };\n");

		fclose(f);
	}

	argc = 3;
	if (nvram_get_int("debug_ipv6"))
		argv[argc++] = "-D";

	argv[argc++] = wan6face;
	argv[argc] = NULL;
	_eval(argv, NULL, 0, NULL);
}

void stop_dhcp6c(void)
{
	killall("dhcp6c-event", SIGTERM);
	killall_tk_period_wait("dhcp6c", 50);

	nvram_set("ipv6_pd_pltime", "0"); /* reset preferred and valid lifetime */
	nvram_set("ipv6_pd_vltime", "0");
}
#endif /* TCONFIG_IPV6 */
