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

#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <wlutils.h>

#include <sys/mount.h>
#include <mntent.h>
#include <dirent.h>
#include <linux/version.h>


#define dnslog(level,x...) if (nvram_get_int("dns_debug")>=level) syslog(level, x)

// Pop an alarm to recheck pids in 500 msec.
static const struct itimerval pop_tv = { {0,0}, {0, 500 * 1000} };

// Pop an alarm to reap zombies.
static const struct itimerval zombie_tv = { {0,0}, {307, 0} };

// -----------------------------------------------------------------------------

static const char dmhosts[] = "/etc/dnsmasq";
static const char dmdhcp[] = "/etc/dnsmasq";
static const char dmresolv[] = "/etc/resolv.dnsmasq";

static pid_t pid_dnsmasq = -1;




static int is_wet(int idx, int unit, int subunit, void *param)
{
	return nvram_match(wl_nvname("mode", unit, subunit), "wet");
}

void start_dnsmasq()
{
	FILE *f;
	const char *nv;
	char buf[512];
	char lan[24];
	const char *router_ip;
	char sdhcp_lease[32];
	char *e;
	int n;
	char *mac, *ip, *name;
	char *p;
	int ipn;
	char ipbuf[32];
	char tmp[128];
	FILE *hf, *df;
	int dhcp_start;
	int dhcp_count;
	int dhcp_lease;
	int do_dhcpd;
	int do_dns;
	int do_dhcpd_hosts;

#ifdef TCONFIG_IPV6
	int ipv6_lease; /* DHCP IPv6 lease time */
#endif

	char wan_prefix[] = "wanXX";
	int wan_unit, mwan_num;
	const dns_list_t *dns;

	TRACE_PT("begin\n");

	if (getpid() != 1) {
		start_service("dnsmasq");
		return;
	}

	stop_dnsmasq();

	if (foreach_wif(1, NULL, is_wet)) {
		syslog(LOG_INFO, "Starting dnsmasq is skipped due to the WEB mode enabled\n");
		return;
	}

	if ((f = fopen("/etc/dnsmasq.conf", "w")) == NULL) {
		perror("/etc/dnsmasq.conf");
		return;
	}

	router_ip = nvram_safe_get("lan_ipaddr");

	fprintf(f,
		"pid-file=/var/run/dnsmasq.pid\n");

	if (((nv = nvram_get("wan_domain")) != NULL) || ((nv = nvram_get("wan_get_domain")) != NULL)) {
		if (*nv) fprintf(f, "domain=%s\n", nv);
	}

	mwan_num = nvram_get_int("mwan_num");
	if (mwan_num < 1 || mwan_num > MWAN_MAX) {
		mwan_num = 1;
	}

	if (((nv = nvram_get("dns_minport")) != NULL) && (*nv)) n = atoi(nv);
		else n = 4096;
	fprintf(f,
		"resolv-file=%s\n"		// the real stuff is here
		"addn-hosts=%s\n"		// directory with additional hosts files
		"dhcp-hostsfile=%s\n"		// directory with dhcp hosts files
		"expand-hosts\n"		// expand hostnames in hosts file
		"min-port=%u\n", 		// min port used for random src port
		dmresolv, dmhosts, dmdhcp, n);

	do_dns = nvram_match("dhcpd_dmdns", "1");

	// DNS rebinding protection, will discard upstream RFC1918 responses
	if (nvram_get_int("dns_norebind")) {
		fprintf(f,
			"stop-dns-rebind\n"
			"rebind-localhost-ok\n");
	}

#ifdef TCONFIG_DNSCRYPT
	if (nvram_match("dnscrypt_proxy", "1")) {
		fprintf(f, "server=127.0.0.1#%s\n", nvram_safe_get("dnscrypt_port") );
	}
#endif
#ifdef TCONFIG_STUBBY
	if (nvram_match("stubby_proxy", "1")) {
		fprintf(f, "server=127.0.0.1#5453\n");
	}
#endif
#ifdef TCONFIG_TOR
	if ((nvram_match("tor_enable", "1")) && (nvram_match("dnsmasq_onion_support", "1"))) {
		char *t_ip = nvram_safe_get("lan_ipaddr");

		if (nvram_match("tor_iface", "br1")) t_ip = nvram_safe_get("lan1_ipaddr");
		if (nvram_match("tor_iface", "br2")) t_ip = nvram_safe_get("lan2_ipaddr");
		if (nvram_match("tor_iface", "br3")) t_ip = nvram_safe_get("lan3_ipaddr");

		fprintf(f, "server=/onion/%s#%s\n", t_ip, nvram_safe_get("tor_dnsport"));
	}
#endif
	for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {
		get_wan_prefix(wan_unit, wan_prefix);

		/* allow RFC1918 responses for server domain (fix connect PPTP/L2TP WANs) */
		switch (get_wanx_proto(wan_prefix)) {
		case WP_PPTP:
			nv = nvram_safe_get(strcat_r(wan_prefix, "_pptp_server_ip", tmp));
			break;
		case WP_L2TP:
			nv = nvram_safe_get(strcat_r(wan_prefix, "_l2tp_server_ip", tmp));
			break;
		default:
			nv = NULL;
			break;
		}
		if (nv && *nv) fprintf(f, "rebind-domain-ok=%s\n", nv);

		dns = get_dns(wan_prefix);	// this always points to a static buffer

		/* check dns entries only for active connections (checkConnect might have false-negative response) */
		if ((check_wanup(wan_prefix) == 0) && (dns->count == 0))
			continue;
		/* dns list with non-standart ports */
		for (n = 0 ; n < dns->count; ++n) {
			if (dns->dns[n].port != 53)
				fprintf(f, "server=%s#%u\n", inet_ntoa(dns->dns[n].addr), dns->dns[n].port);
		}
	} /* end for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) */

	if (nvram_get_int("dhcpd_static_only")) {
		fprintf(f, "dhcp-ignore=tag:!known\n");
	}

	if ((n = nvram_get_int("dnsmasq_q"))) {	/* process quiet flags */
		if (n & 1) fprintf(f, "quiet-dhcp\n");
#ifdef TCONFIG_IPV6
		if (n & 2) fprintf(f, "quiet-dhcp6\n");
		if (n & 4) fprintf(f, "quiet-ra\n");
#endif
	}

	// dhcp
	do_dhcpd_hosts=0;
	char lanN_proto[] = "lanXX_proto";
	char lanN_ifname[] = "lanXX_ifname";
	char lanN_ipaddr[] = "lanXX_ipaddr";
	char lanN_netmask[] = "lanXX_netmask";
	char dhcpdN_startip[] = "dhcpdXX_startip";
	char dhcpdN_endip[] = "dhcpdXX_endip";
	char dhcpN_start[] = "dhcpXX_start";
	char dhcpN_num[] = "dhcpXX_num";
	char dhcpN_lease[] = "dhcpXX_lease";
	char br;
	for (br = 0 ; br <= 3 ; br++) {
		char bridge[2] = "0";
		if (br != 0)
			bridge[0] += br;
		else
			strcpy(bridge, "");

		sprintf(lanN_proto, "lan%s_proto", bridge);
		sprintf(lanN_ifname, "lan%s_ifname", bridge);
		sprintf(lanN_ipaddr, "lan%s_ipaddr", bridge);
		do_dhcpd = nvram_match(lanN_proto, "dhcp");
		if (do_dhcpd) {
			do_dhcpd_hosts++;

			router_ip = nvram_safe_get(lanN_ipaddr);
			strlcpy(lan, router_ip, sizeof(lan));
			if ((p = strrchr(lan, '.')) != NULL) *(p + 1) = 0;

			fprintf(f,
				"interface=%s\n",
				nvram_safe_get(lanN_ifname));

			sprintf(dhcpN_lease, "dhcp%s_lease", bridge);
			dhcp_lease = nvram_get_int(dhcpN_lease);

			if (dhcp_lease <= 0) dhcp_lease = 1440;

			if ((e = nvram_get("dhcpd_slt")) != NULL) n = atoi(e); else n = 0;
			if (n < 0) strcpy(sdhcp_lease, "infinite");
				else sprintf(sdhcp_lease, "%dm", (n > 0) ? n : dhcp_lease);

			if (!do_dns) {	/* if not using dnsmasq for dns */

				for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {

				get_wan_prefix(wan_unit, wan_prefix);
				/* skip inactive WAN connections (checkConnect might have false-negative response)
				   TBD: need to check if there is no WANs active do we need skip here also?!? */
				if (check_wanup(wan_prefix) == 0)
					continue;

				dns = get_dns(wan_prefix);	/* static buffer */

				if ((dns->count == 0) && (nvram_get_int("dhcpd_llndns"))) {
					/* no DNS might be temporary. use a low lease time to force clients to update. */
					dhcp_lease = 2;
					strcpy(sdhcp_lease, "2m");
					do_dns = 1;
				}
				else {
					/* pass the dns directly */
					buf[0] = 0;
					for (n = 0 ; n < dns->count; ++n) {
						if (dns->dns[n].port == 53) {	/* check: option 6 doesn't seem to support other ports */
							sprintf(buf + strlen(buf), ",%s", inet_ntoa(dns->dns[n].addr));
						}
					}
					fprintf(f, "dhcp-option=tag:%s,6%s\n", nvram_safe_get(lanN_ifname), buf);
				}

				} /* end for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) */
			}

			sprintf(dhcpdN_startip, "dhcpd%s_startip", bridge);
			sprintf(dhcpdN_endip, "dhcpd%s_endip", bridge);
			sprintf(lanN_netmask, "lan%s_netmask", bridge);

			if ((p = nvram_get(dhcpdN_startip)) && (*p) && (e = nvram_get(dhcpdN_endip)) && (*e)) {
				fprintf(f, "dhcp-range=tag:%s,%s,%s,%s,%dm\n", nvram_safe_get(lanN_ifname), p, e, nvram_safe_get(lanN_netmask), dhcp_lease);
			}
			else {
				/* for compatibility */
				sprintf(dhcpN_start, "dhcp%s_start", bridge);
				sprintf(dhcpN_num, "dhcp%s_num", bridge);
				sprintf(lanN_netmask, "lan%s_netmask", bridge);
				dhcp_start = nvram_get_int(dhcpN_start);
				dhcp_count = nvram_get_int(dhcpN_num);
				fprintf(f, "dhcp-range=tag:%s,%s%d,%s%d,%s,%dm\n",
					nvram_safe_get(lanN_ifname), lan, dhcp_start, lan, dhcp_start + dhcp_count - 1, nvram_safe_get(lanN_netmask), dhcp_lease);
			}

			nv = nvram_safe_get(lanN_ipaddr);
			if ((nvram_get_int("dhcpd_gwmode") == 1) && (get_wan_proto() == WP_DISABLED)) {
				p = nvram_safe_get("lan_gateway");
				if ((*p) && (strcmp(p, "0.0.0.0") != 0)) nv = p;
			}

			fprintf(f,
				"dhcp-option=tag:%s,3,%s\n",	/* gateway */
				nvram_safe_get(lanN_ifname), nv);

			if (((nv = nvram_get("wan_wins")) != NULL) && (*nv) && (strcmp(nv, "0.0.0.0") != 0)) {
				fprintf(f, "dhcp-option=tag:%s,44,%s\n", nvram_safe_get(lanN_ifname), nv);
			}
#ifdef TCONFIG_SAMBASRV
			else if (nvram_get_int("smbd_enable") && nvram_invmatch("lan_hostname", "") && nvram_get_int("smbd_wins")) {
				if ((nv == NULL) || (*nv == 0) || (strcmp(nv, "0.0.0.0") == 0)) {
					/* Samba will serve as a WINS server */
					fprintf(f, "dhcp-option=tag:%s,44,%s\n", nvram_safe_get(lanN_ifname), nvram_safe_get(lanN_ipaddr));
				}
			}
#endif
		} else {
			if (strcmp(nvram_safe_get(lanN_ifname), "") != 0) {
				fprintf(f, "interface=%s\n", nvram_safe_get(lanN_ifname));
				/* if no dhcp range is set then no dhcp service will be offered so following line is superflous */
				// fprintf(f, "no-dhcp-interface=%s\n", nvram_safe_get(lanN_ifname));
			}
		}
	}

	/* write static lease entries & create hosts file */
	mkdir_if_none(dmhosts);
	snprintf(buf, sizeof(buf), "%s/hosts", dmhosts);
	if ((hf = fopen(buf, "w")) != NULL) {
		if (((nv = nvram_get("wan_hostname")) != NULL) && (*nv))
			fprintf(hf, "%s %s\n", router_ip, nv);
#ifdef TCONFIG_SAMBASRV
		else if (((nv = nvram_get("lan_hostname")) != NULL) && (*nv))
			fprintf(hf, "%s %s\n", router_ip, nv);
#endif
		p = (char *)get_wanip("wan");
		if ((*p == 0) || strcmp(p, "0.0.0.0") == 0)
			p = "127.0.0.1";
		fprintf(hf, "%s wan1-ip\n", p);

		p = (char *)get_wanip("wan2");
		if ((*p == 0) || strcmp(p, "0.0.0.0") == 0)
			p = "127.0.0.1";
		fprintf(hf, "%s wan2-ip\n", p);

#ifdef TCONFIG_MULTIWAN
		p = (char *)get_wanip("wan3");
		if ((*p == 0) || strcmp(p, "0.0.0.0") == 0)
			p = "127.0.0.1";
		fprintf(hf, "%s wan3-ip\n", p);

		p = (char *)get_wanip("wan4");
		if ((*p == 0) || strcmp(p, "0.0.0.0") == 0)
			p = "127.0.0.1";
		fprintf(hf, "%s wan4-ip\n", p);
#endif

	}

	mkdir_if_none(dmdhcp);
	snprintf(buf, sizeof(buf), "%s/dhcp-hosts", dmdhcp);
	df = fopen(buf, "w");

	/* PREVIOUS/OLD FORMAT:
	 * 00:aa:bb:cc:dd:ee<123<xxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 53 w/ delim
	 * 00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 85 w/ delim
	 * 00:aa:bb:cc:dd:ee,00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz> = 106 w/ delim
	 *
	 * NEW FORMAT (+static ARP binding after hostname):
	 * 00:aa:bb:cc:dd:ee<123<xxxxxxxxxxxxxxxxxxxxxxxxxx.xyz<a> = 55 w/ delim
	 * 00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz<a> = 87 w/ delim
	 * 00:aa:bb:cc:dd:ee,00:aa:bb:cc:dd:ee<123.123.123.123<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xyz<a> = 108 w/ delim
	 */

	p = nvram_safe_get("dhcpd_static");
	while ((e = strchr(p, '>')) != NULL) {
		n = (e - p);
		if (n > 107) {
			p = e + 1;
			continue;
		}

		strncpy(buf, p, n);
		buf[n] = 0;
		p = e + 1;

		if ((e = strchr(buf, '<')) == NULL) continue;
		*e = 0;
		mac = buf;

		ip = e + 1;
		if ((e = strchr(ip, '<')) == NULL) continue;
		*e = 0;
		if (strchr(ip, '.') == NULL) {
			ipn = atoi(ip);
			if ((ipn <= 0) || (ipn > 255)) continue;
			sprintf(ipbuf, "%s%d", lan, ipn);
			ip = ipbuf;
		}
		else {
			if (inet_addr(ip) == INADDR_NONE) continue;
		}

		name = e + 1;

		if ((e = strchr(name, '<')) != NULL) {
			*e = 0;
		}

		if ((hf) && (*name != 0)) {
			fprintf(hf, "%s %s\n", ip, name);
		}

		if ((do_dhcpd_hosts > 0) && (*mac != 0) && (strcmp(mac, "00:00:00:00:00:00") != 0)) {
			fprintf(f, "dhcp-host=%s,%s", mac, ip);
			if (nvram_get_int("dhcpd_slt") != 0) {
				fprintf(f, ",%s", sdhcp_lease);
			}
			fprintf(f, "\n");
		}
	}

	if (df) fclose(df);
	if (hf) fclose(hf);

	n = nvram_get_int("dhcpd_lmax");
	fprintf(f, "dhcp-lease-max=%d\n", (n > 0) ? n : 255);
	if (nvram_get_int("dhcpd_auth") >= 0) {
	fprintf(f,
		"dhcp-option=lan,252,\"\\n\"\n"
		"dhcp-authoritative\n");
	}

	if (nvram_match("dnsmasq_debug", "1")) {
		fprintf(f, "log-queries\n");
	}

	if ((nvram_get_int("adblock_enable")) && (f_exists("/etc/dnsmasq.adblock"))) {
		fprintf(f, "conf-file=/etc/dnsmasq.adblock\n");
	}

#ifdef TCONFIG_DNSSEC
	if (nvram_match("dnssec_enable", "1")) {
		fprintf(f,
			"conf-file=/etc/trust-anchors.conf\n"
			"dnssec\n"
			"dnssec-no-timecheck\n");
	}
#endif

#ifdef TCONFIG_DNSCRYPT
	if (nvram_match("dnscrypt_proxy", "1")) {
		if (nvram_match("dnscrypt_priority", "1"))
			fprintf(f, "strict-order\n");

		if (nvram_match("dnscrypt_priority", "2"))
			fprintf(f, "no-resolv\n");
	}
#endif

#ifdef TCONFIG_STUBBY
	if (nvram_match("stubby_proxy", "1")) {
		if (nvram_match("stubby_priority", "1"))
			fprintf(f, "strict-order\n");

		if (nvram_match("stubby_priority", "2"))
			fprintf(f, "no-resolv\n");
	}
#endif

	//

#ifdef TCONFIG_OPENVPN
	write_ovpn_dnsmasq_config(f);
#endif

#ifdef TCONFIG_PPTPD
	write_pptpd_dnsmasq_config(f);
#endif

#ifdef TCONFIG_IPV6
	if (ipv6_enabled()) {

		ipv6_lease = nvram_get_int("ipv6_lease_time"); /* get DHCP IPv6 lease time */
		if ((ipv6_lease < 1) || (ipv6_lease > 720)) { /* check lease time and limit the range (1...720 hours, 30 days should be enough) */
			ipv6_lease = 12;
		}

		/* enable-ra should be enabled in both cases */
		if (nvram_get_int("ipv6_radvd") || nvram_get_int("ipv6_dhcpd")) {
			fprintf(f,"enable-ra\n");
		}

		/* Only SLAAC and NO DHCPv6 */
		if (nvram_get_int("ipv6_radvd") && !nvram_get_int("ipv6_dhcpd")) {
			fprintf(f,"dhcp-range=::, constructor:br*, ra-names, ra-stateless, 64, %dh\n", ipv6_lease);
		}

		/* Only DHCPv6 and NO SLAAC */
		if (nvram_get_int("ipv6_dhcpd") && !nvram_get_int("ipv6_radvd")) {
			fprintf(f,"dhcp-range=::2, ::FFFF:FFFF, constructor:br*, 64, %dh\n", ipv6_lease);
		}

		/* SLAAC and DHCPv6 (2 IPv6 IPs) */
		if (nvram_get_int("ipv6_radvd") && nvram_get_int("ipv6_dhcpd")) {
			fprintf(f,"dhcp-range=::2, ::FFFF:FFFF, constructor:br*, ra-names, 64, %dh\n", ipv6_lease);
		}
	}
#endif

	fprintf(f, "%s\n", nvram_safe_get("dnsmasq_custom"));

	fappend(f, "/etc/dnsmasq.custom");
	fappend(f, "/etc/dnsmasq.ipset");

	fclose(f);

	if (do_dns) {
		unlink("/etc/resolv.conf");
		symlink("/rom/etc/resolv.conf", "/etc/resolv.conf");	/* nameserver 127.0.0.1 */
	}

	TRACE_PT("run dnsmasq\n");

	/* Default to some values we like, but allow the user to override them */
	eval("dnsmasq", "-c", "4096", "--log-async");

	if (!nvram_contains_word("debug_norestart", "dnsmasq")) {
		pid_dnsmasq = -2;
	}

	TRACE_PT("end\n");

#ifdef TCONFIG_DNSCRYPT
	/* start dnscrypt-proxy */
	if (nvram_match("dnscrypt_proxy", "1")) {
		char dnscrypt_local[30];
		char *dnscrypt_ekeys;
		sprintf(dnscrypt_local, "127.0.0.1:%s", nvram_safe_get("dnscrypt_port") );
		dnscrypt_ekeys = nvram_match("dnscrypt_ephemeral_keys", "1") ? "-E" : "";

		eval("ntp2ip");

		if (nvram_match("dnscrypt_manual", "1")) {
			eval("dnscrypt-proxy", "-d", dnscrypt_ekeys,
			     "-a", dnscrypt_local,
			     "-m", nvram_safe_get("dnscrypt_log"),
			     "-N", nvram_safe_get("dnscrypt_provider_name"),
			     "-k", nvram_safe_get("dnscrypt_provider_key"),
			     "-r", nvram_safe_get("dnscrypt_resolver_address") );
		} else {
			eval("dnscrypt-proxy", "-d", dnscrypt_ekeys,
			     "-a", dnscrypt_local,
			     "-m", nvram_safe_get("dnscrypt_log"),
			     "-R", nvram_safe_get("dnscrypt_resolver"),
			     "-L", "/etc/dnscrypt-resolvers.csv" );
		}

#ifdef TCONFIG_IPV6
		char dnscrypt_local_ipv6[30];
		sprintf(dnscrypt_local_ipv6, "::1:%s", nvram_safe_get("dnscrypt_port") );

		if (get_ipv6_service() != *("NULL")) {	/* when ipv6 enabled */
			if (nvram_match("dnscrypt_manual", "1")) {
				eval("dnscrypt-proxy", "-d", dnscrypt_ekeys,
				     "-a", dnscrypt_local,
				     "-m", nvram_safe_get("dnscrypt_log"),
				     "-N", nvram_safe_get("dnscrypt_provider_name"),
				     "-k", nvram_safe_get("dnscrypt_provider_key"),
				     "-r", nvram_safe_get("dnscrypt_resolver_address") );
			} else {
				eval("dnscrypt-proxy", "-d", dnscrypt_ekeys,
				     "-a", dnscrypt_local,
				     "-m", nvram_safe_get("dnscrypt_log"),
				     "-R", nvram_safe_get("dnscrypt_resolver"),
				     "-L", "/etc/dnscrypt-resolvers.csv" );
			}
		}
#endif
	}
#endif

#ifdef TCONFIG_STUBBY
	if (nvram_match("stubby_proxy", "1")) {
		eval("ntp2ip");
		eval("stubby", "-g", "-v", nvram_safe_get("stubby_log"), "-C", "/etc/stubby.yml");
	}
#endif

#ifdef TCONFIG_DNSSEC
	if ((time(0) > Y2K) && nvram_match("dnssec_enable", "1")) {
		sleep(1);
		killall("dnsmasq", SIGINT);
	}
#endif

}

void stop_dnsmasq(void)
{
	TRACE_PT("begin\n");

	if (getpid() != 1) {
		stop_service("dnsmasq");
		return;
	}

	pid_dnsmasq = -1;

	unlink("/etc/resolv.conf");
	symlink(dmresolv, "/etc/resolv.conf");

	killall_tk("dnsmasq");
#ifdef TCONFIG_DNSCRYPT
	killall_tk("dnscrypt-proxy");
#endif

#ifdef TCONFIG_STUBBY
	killall_tk("stubby");
#endif

	TRACE_PT("end\n");
}

void clear_resolv(void)
{
#ifndef TCONFIG_OPTIMIZE_SIZE
	dnslog(LOG_DEBUG, "*** clear_resolv, clear all DNS entries\n");
#endif
	f_write(dmresolv, NULL, 0, 0, 0);	/* blank */
}

void start_adblock(int update)
{
	if (nvram_match("adblock_enable", "1")) {
		killall("adblock", SIGTERM);
		sleep(1);
		if (update) {
			xstart("/usr/sbin/adblock", "update");
		} else {
			xstart("/usr/sbin/adblock");
		}
	}
}

void stop_adblock()
{
	xstart("/usr/sbin/adblock", "stop");
}

#ifdef TCONFIG_IPV6
static int write_ipv6_dns_servers(FILE *f, const char *prefix, char *dns, const char *suffix, int once)
{
	char p[INET6_ADDRSTRLEN + 1], *next = NULL;
	struct in6_addr addr;
	int cnt = 0;

	foreach(p, dns, next) {
		/* verify that this is a valid IPv6 address */
		if (inet_pton(AF_INET6, p, &addr) == 1) {
			fprintf(f, "%s%s%s", (once && cnt) ? "" : prefix, p, suffix);
			++cnt;
		}
	}

	return cnt;
}
#endif

void dns_to_resolv(void)
{
	FILE *f;
	const dns_list_t *dns;
	char *trig_ip;
	int i;
	mode_t m;
	char wan_prefix[] = "wanXX";
	int wan_unit, mwan_num;
	int append = 0;
	int exclusive = 0;
	char tmp[64];

	mwan_num = nvram_get_int("mwan_num");
	if (mwan_num < 1 || mwan_num > MWAN_MAX) {
		mwan_num = 1;
	}

	for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {
		get_wan_prefix(wan_unit, wan_prefix);

		/* skip inactive WAN connections (checkConnect might have false-negative response) */
		if ((check_wanup(wan_prefix) == 0) &&
		    get_wanx_proto(wan_prefix) != WP_DISABLED &&
		    get_wanx_proto(wan_prefix) != WP_PPTP &&
		    get_wanx_proto(wan_prefix) != WP_L2TP &&
		    !nvram_get_int(strcat_r(wan_prefix, "_ppp_demand", tmp)))
		{
#ifndef TCONFIG_OPTIMIZE_SIZE
			dnslog(LOG_DEBUG, "*** dns_to_resolv: %s (proto:%d) is not UP, not P-t-P or On Demand, SKIP ADD\n", wan_prefix, get_wanx_proto(wan_prefix));
#endif
			continue;
		}
		else {
#ifndef TCONFIG_OPTIMIZE_SIZE
			dnslog(LOG_DEBUG, "*** dns_to_resolv: %s (proto:%d) is OK to ADD\n", wan_prefix, get_wanx_proto(wan_prefix));
#endif
			append++;
		}
		m = umask(022);	/* 077 from pppoecd */
		if ((f = fopen(dmresolv, (append == 1) ? "w" : "a")) != NULL) {	/* write / append */
			if (append == 1)
				/* Check for VPN DNS entries */
				exclusive = (write_pptp_client_resolv(f)
#ifdef TCONFIG_OPENVPN
				             || write_ovpn_resolv(f)
#endif
				);

#ifndef TCONFIG_OPTIMIZE_SIZE
			dnslog(LOG_DEBUG, "exclusive: %d", exclusive);
#endif
			if (!exclusive) { /* exclusive check */
#ifdef TCONFIG_IPV6
				if (write_ipv6_dns_servers(f, "nameserver ", nvram_safe_get("ipv6_dns"), "\n", 0) == 0 || nvram_get_int("dns_addget"))
					if (append == 1) /* only once */
						write_ipv6_dns_servers(f, "nameserver ", nvram_safe_get("ipv6_get_dns"), "\n", 0);
#endif
				dns = get_dns(wan_prefix);	/* static buffer */
				if (dns->count == 0) {
					/* Put a pseudo DNS IP to trigger Connect On Demand */
					if (nvram_match(strcat_r(wan_prefix, "_ppp_demand", tmp), "1")) {
						switch (get_wanx_proto(wan_prefix)) {
						case WP_PPPOE:
						case WP_PPP3G:
						case WP_PPTP:
						case WP_L2TP:
							/* The nameserver IP specified below used to be 1.1.1.1, however this became an legit IP address of a public recursive DNS server,
							 * defeating the purpose of specifying a bogus DNS server in order to trigger Connect On Demand.
							 * An IP address from TEST-NET-2 block was chosen here, as RFC 5737 explicitly states this address block
							 * should be non-routable over the public internet. In effect since January 2010.
							 * Further info: http://linksysinfo.org/index.php?threads/tomato-using-1-1-1-1-for-pppoe-connect-on-demand.74102
							 * Also add possibility to change that IP (198.51.100.1) in GUI by the user
							 */
							trig_ip = nvram_safe_get(strcat_r(wan_prefix, "_ppp_demand_dnsip", tmp));
#ifndef TCONFIG_OPTIMIZE_SIZE
							dnslog(LOG_DEBUG, "*** dns_to_resolv: no servers for %s: put a pseudo DNS (non-routable on public internet) IP %s to trigger Connect On Demand", wan_prefix, trig_ip);
#endif
							fprintf(f, "nameserver %s\n", trig_ip);
							break;
						}
					}
				} else {
					fprintf(f, "# dns for %s:\n", wan_prefix);
					for (i = 0; i < dns->count; i++) {
						if (dns->dns[i].port == 53) {	/* resolv.conf doesn't allow for an alternate port */
							fprintf(f, "nameserver %s\n", inet_ntoa(dns->dns[i].addr));
#ifndef TCONFIG_OPTIMIZE_SIZE
							dnslog(LOG_DEBUG, "*** dns_to_resolv, %s DNS %s to %s [%s]", (append == 1) ? "write" : "append", inet_ntoa(dns->dns[i].addr), dmresolv, wan_prefix);
#endif
						}
					}
				}
			}
			fclose(f);
		}
		umask(m);

	} /* end for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) */
}

void start_httpd(void)
{
	if (getpid() != 1) {
		start_service("httpd");
		return;
	}

	if (nvram_match("web_css", "online")) {
		xstart("/usr/sbin/ttb");
	}

	stop_httpd();
	/* wait to exit gracefully */
	sleep(1);

	/* set www dir */
	if      (nvram_match("web_dir", "jffs")) { chdir("/jffs/www"); }
	else if (nvram_match("web_dir", "opt"))  { chdir("/opt/www"); }
	else if (nvram_match("web_dir", "tmp"))  { chdir("/tmp/www"); }
	else                                     { chdir("/www"); }

	eval("httpd");
	chdir("/");
}

void stop_httpd(void)
{
	if (getpid() != 1) {
		stop_service("httpd");
		return;
	}

	killall_tk("httpd");
}

#ifdef TCONFIG_IPV6
static void add_ip6_lanaddr(void)
{
	char ip[INET6_ADDRSTRLEN + 4];
	const char *p;

	p = ipv6_router_address(NULL);
	if (*p) {
		snprintf(ip, sizeof(ip), "%s/%d", p, nvram_get_int("ipv6_prefix_length") ? : 64);
		eval("ip", "-6", "addr", "add", ip, "dev", nvram_safe_get("lan_ifname"));
	}
}

void start_ipv6_tunnel(void)
{
	char ip[INET6_ADDRSTRLEN + 4];
	struct in_addr addr4;
	struct in6_addr addr;
	char *wanip, *mtu, *tun_dev;
	int service;
	char wan_prefix[] = "wanXX";
	int wan_unit, mwan_num;

	mwan_num = nvram_get_int("mwan_num");
	if (mwan_num < 1 || mwan_num > MWAN_MAX) {
		mwan_num = 1;
	}
	for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {
		get_wan_prefix(wan_unit, wan_prefix);
		if (check_wanup(wan_prefix)) break;
	}

	service = get_ipv6_service();
	tun_dev = (char *)get_wan6face();
	wanip = (char *)get_wanip(wan_prefix);

	mtu = (nvram_get_int("ipv6_tun_mtu") > 0) ? nvram_safe_get("ipv6_tun_mtu") : "1480";
	modprobe("sit");

	eval("ip", "tunnel", "add", tun_dev, "mode", "sit",
	     "remote", (service == IPV6_ANYCAST_6TO4) ? "any" : nvram_safe_get("ipv6_tun_v4end"),
	     "local", wanip,
	     "ttl", nvram_safe_get("ipv6_tun_ttl"));
	eval("ip", "link", "set", tun_dev, "mtu", mtu, "up");

	nvram_set("ipv6_ifname", tun_dev);

	if (service == IPV6_ANYCAST_6TO4) {
		int prefixlen = 16;
		int mask4size = 0;

		addr4.s_addr = 0;
		memset(&addr, 0, sizeof(addr));
		inet_aton(wanip, &addr4);
		addr.s6_addr16[0] = htons(0x2002);
		ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
		addr.s6_addr16[7] = htons(0x0001);
		inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
		snprintf(ip, sizeof(ip), "%s/%d", ip, prefixlen);
		add_ip6_lanaddr();
	}
	/* static tunnel 6to4 */
	else {
		snprintf(ip, sizeof(ip), "%s/%d",
			nvram_safe_get("ipv6_tun_addr"),
			nvram_get_int("ipv6_tun_addrlen") ? : 64);
	}

	eval("ip", "-6", "addr", "add", ip, "dev", tun_dev);

	if (service == IPV6_ANYCAST_6TO4) {
		snprintf(ip, sizeof(ip), "::192.88.99.%d", nvram_get_int("ipv6_relay"));
		eval("ip", "-6", "route", "add", "2000::/3", "via", ip, "dev", tun_dev, "metric", "1");
	}
	else {
		eval("ip", "-6", "route", "add", "::/0", "dev", tun_dev, "metric", "1");
	}

	/* (re)start dnsmasq */
	if (service == IPV6_ANYCAST_6TO4)
		start_dnsmasq();
}

void stop_ipv6_tunnel(void)
{
	eval("ip", "tunnel", "del", (char *)get_wan6face());
	if (get_ipv6_service() == IPV6_ANYCAST_6TO4) {
		/* get rid of old IPv6 address from lan iface */
		eval("ip", "-6", "addr", "flush", "dev", nvram_safe_get("lan_ifname"), "scope", "global");
	}
	modprobe_r("sit");
}

void start_6rd_tunnel(void)
{
	const char *tun_dev, *wanip;
	int service, mask_len, prefix_len, local_prefix_len;
	char mtu[10], prefix[INET6_ADDRSTRLEN], relay[INET_ADDRSTRLEN];
	struct in_addr netmask_addr, relay_addr, relay_prefix_addr, wanip_addr;
	struct in6_addr prefix_addr, local_prefix_addr;
	char local_prefix[INET6_ADDRSTRLEN];
	char tmp_ipv6[INET6_ADDRSTRLEN + 4], tmp_ipv4[INET_ADDRSTRLEN + 4];
	char tmp[256];
	FILE *f;
	char wan_prefix[] = "wanXX";
	int wan_unit,mwan_num;

	mwan_num = nvram_get_int("mwan_num");
	if (mwan_num < 1 || mwan_num > MWAN_MAX) {
		mwan_num = 1;
	}
	for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {
		get_wan_prefix(wan_unit, wan_prefix);
		if (check_wanup(wan_prefix)) break;
	}

	service = get_ipv6_service();
	wanip = get_wanip(wan_prefix);
	tun_dev = get_wan6face();
	sprintf(mtu, "%d", (nvram_get_int("wan_mtu") > 0) ? (nvram_get_int("wan_mtu") - 20) : 1280);

	/* maybe we can merge the ipv6_6rd_* variables into a single ipv_6rd_string (ala wan_6rd) to save nvram space? */
	if (service == IPV6_6RD) {
		_dprintf("starting 6rd tunnel using manual settings.\n");
		mask_len = nvram_get_int("ipv6_6rd_ipv4masklen");
		prefix_len = nvram_get_int("ipv6_6rd_prefix_length");
		strcpy(prefix, nvram_safe_get("ipv6_6rd_prefix"));
		strcpy(relay, nvram_safe_get("ipv6_6rd_borderrelay"));
	}
	else {
		_dprintf("starting 6rd tunnel using automatic settings.\n");
		char *wan_6rd = nvram_safe_get("wan_6rd");
		if (sscanf(wan_6rd, "%d %d %s %s", &mask_len,  &prefix_len, prefix, relay) < 4) {
			_dprintf("wan_6rd string is missing or invalid (%s)\n", wan_6rd);
			return;
		}
	}

	/* validate values that were passed */
	if (mask_len < 0 || mask_len > 32) {
		_dprintf("invalid mask_len value (%d)\n", mask_len);
		return;
	}
	if (prefix_len < 0 || prefix_len > 128) {
		_dprintf("invalid prefix_len value (%d)\n", prefix_len);
		return;
	}
	if ((32 - mask_len) + prefix_len > 128) {
		_dprintf("invalid combination of mask_len and prefix_len!\n");
		return;
	}

	sprintf(tmp, "ping -q -c 2 %s | grep packet", relay);
	if ((f = popen(tmp, "r")) == NULL) {
		_dprintf("error obtaining data\n");
		return;
	}
	fgets(tmp, sizeof(tmp), f);
	pclose(f);
	if (strstr(tmp, " 0% packet loss") == NULL) {
		_dprintf("failed to ping border relay\n");
		return;
	}

	/* get relay prefix from border relay address and mask */
	netmask_addr.s_addr = htonl(0xffffffff << (32 - mask_len));
	inet_aton(relay, &relay_addr);
	relay_prefix_addr.s_addr = relay_addr.s_addr & netmask_addr.s_addr;

	/* calculate the local prefix */
	inet_pton(AF_INET6, prefix, &prefix_addr);
	inet_pton(AF_INET, wanip, &wanip_addr);
	if (calc_6rd_local_prefix(&prefix_addr, prefix_len, mask_len,
	    &wanip_addr, &local_prefix_addr, &local_prefix_len) == 0) {
		_dprintf("error calculating local prefix.");
		return;
	}
	inet_ntop(AF_INET6, &local_prefix_addr, local_prefix, sizeof(local_prefix));

	snprintf(tmp_ipv6, sizeof(tmp_ipv6), "%s1", local_prefix);
	nvram_set("ipv6_rtr_addr", tmp_ipv6);
	nvram_set("ipv6_prefix", local_prefix);

	/* load sit module needed for the 6rd tunnel */
	modprobe("sit");

	/* create the 6rd tunnel */
	eval("ip", "tunnel", "add", (char *)tun_dev, "mode", "sit", "local", (char *)wanip, "ttl", nvram_safe_get("ipv6_tun_ttl"));

	snprintf(tmp_ipv6, sizeof(tmp_ipv6), "%s/%d", prefix, prefix_len);
	snprintf(tmp_ipv4, sizeof(tmp_ipv4), "%s/%d", inet_ntoa(relay_prefix_addr), mask_len);
	eval("ip", "tunnel" "6rd", "dev", (char *)tun_dev, "6rd-prefix", tmp_ipv6, "6rd-relay_prefix", tmp_ipv4);

	/* bring up the link */
	eval("ip", "link", "set", "dev", (char *)tun_dev, "mtu", (char *)mtu, "up");

	/* set the WAN address Note: IPv6 WAN CIDR should be: ((32 - ip6rd_ipv4masklen) + ip6rd_prefixlen) */
	snprintf(tmp_ipv6, sizeof(tmp_ipv6), "%s1/%d", local_prefix, local_prefix_len);
	eval("ip", "-6", "addr", "add", tmp_ipv6, "dev", (char *)tun_dev);

	/* set the LAN address Note: IPv6 LAN CIDR should be 64 */
	snprintf(tmp_ipv6, sizeof(tmp_ipv6), "%s1/%d", local_prefix, nvram_get_int("ipv6_prefix_length") ? : 64);
	eval("ip", "-6", "addr", "add", tmp_ipv6, "dev", nvram_safe_get("lan_ifname"));

	/* add default route via the border relay */
	snprintf(tmp_ipv6, sizeof(tmp_ipv6), "::%s", relay);
	// eval("ip", "-6", "route", "add", "default", "via", tmp_ipv6, "dev", (char *)tun_dev);
	eval("ip", "-6", "route", "add", "::/0", "via", tmp_ipv6, "dev", (char *)tun_dev);

	nvram_set("ipv6_ifname", (char *)tun_dev);

	/* (re)start dnsmasq */
	start_dnsmasq();

	printf("6rd end\n");
}

void stop_6rd_tunnel(void)
{
	eval("ip", "tunnel", "del", (char *)get_wan6face());
	eval("ip", "-6", "addr", "flush", "dev", nvram_safe_get("lan_ifname"), "scope", "global");
	modprobe_r("sit");
}

void start_ipv6(void)
{
	int service, i;
	char buffer[16];

	service = get_ipv6_service();
	enable_ip6_forward();

	/* Check if turned on */
	switch (service) {
	case IPV6_NATIVE:
	case IPV6_6IN4:
	case IPV6_MANUAL:
		add_ip6_lanaddr();
		break;
	case IPV6_NATIVE_DHCP:
	case IPV6_ANYCAST_6TO4:
		nvram_set("ipv6_rtr_addr", "");
		nvram_set("ipv6_prefix", "");
		break;
	}

	if (service != IPV6_DISABLED) {
		/* Check if "ipv6_accept_ra" (bit 1) for lan is enabled (via GUI, basic-ipv6.asp) and "ipv6_radvd" AND "ipv6_dhcpd" (SLAAC and/or DHCP with dnsmasq) is disabled (via GUI, advanced-dhcpdns.asp) */
		/* HINT: "ipv6_accept_ra" bit 0 ==> used for wan, "ipv6_accept_ra" bit 1 ==> used for lan interfaces (br0...br3) */
		if ((nvram_get_int("ipv6_accept_ra") & 0x02) != 0 && !nvram_get_int("ipv6_radvd") && !nvram_get_int("ipv6_dhcpd")) {
			/* Check lanX / brX - If available then accept_ra for brX */
			for (i = 0; i < 4; i++) {
				sprintf(buffer, (i == 0 ? "lan_ipaddr" : "lan%d_ipaddr"), i);
				if (strcmp(nvram_safe_get(buffer), "") != 0) {
					sprintf(buffer, (i == 0 ? "lan_ifname" : "lan%d_ifname"), i);
					accept_ra(nvram_safe_get(buffer));
				}
			}
		}
		else {
			/* Check lanX / brX - If available then set accept_ra default value for brX */
			for (i = 0; i < 4; i++) {
				sprintf(buffer, (i == 0 ? "lan_ipaddr" : "lan%d_ipaddr"), i);
				if (strcmp(nvram_safe_get(buffer), "") != 0) {
					sprintf(buffer, (i == 0 ? "lan_ifname" : "lan%d_ifname"), i);
					accept_ra_reset(nvram_safe_get(buffer));
				}
			}
		}
	}
}

void stop_ipv6(void)
{
	stop_ipv6_tunnel();
	stop_dhcp6c();
	eval("ip", "-6", "addr", "flush", "scope", "global");
	eval("ip", "-6", "route", "flush", "scope", "global");
}
#endif /* #ifdef TCONFIG_IPV6 */

void start_upnp(void)
{
	if (getpid() != 1) {
		start_service("upnp");
		return;
	}

	if (get_wan_proto() == WP_DISABLED) return;

	int enable;
	FILE *f;
	int upnp_port;

	if (((enable = nvram_get_int("upnp_enable")) & 3) != 0) {
		mkdir("/etc/upnp", 0777);
		if (f_exists("/etc/upnp/config.alt")) {
			xstart("miniupnpd", "-f", "/etc/upnp/config.alt");
		}
		else {
			if ((f = fopen("/etc/upnp/config", "w")) != NULL) {
				upnp_port = nvram_get_int("upnp_port");
				if ((upnp_port < 0) || (upnp_port >= 0xFFFF)) upnp_port = 0;

				if (check_wanup("wan2")) {
					fprintf(f, "ext_ifname=%s\n", get_wanface("wan2"));
				}

#ifdef TCONFIG_MULTIWAN
				if (check_wanup("wan3")) {
					fprintf(f, "ext_ifname=%s\n", get_wanface("wan3"));
				}
				if (check_wanup("wan4")) {
					fprintf(f, "ext_ifname=%s\n", get_wanface("wan4"));
				}
#endif

				fprintf(f,
					"ext_ifname=%s\n"
					"port=%d\n"
					"enable_upnp=%s\n"
					"enable_natpmp=%s\n"
					"secure_mode=%s\n"
					"upnp_forward_chain=upnp\n"
					"upnp_nat_chain=upnp\n"
					"upnp_nat_postrouting_chain=pupnp\n"
					"notify_interval=%d\n"
					"system_uptime=yes\n"
					"friendly_name=%s"" Router\n"
					"model_name=%s\n"
					"model_url=http://linksysinfo.org/index.php?forums/tomato-firmware.33/\n"
					"manufacturer_name=Tomato Firmware\n"
					"manufacturer_url=http://linksysinfo.org/index.php?forums/tomato-firmware.33/\n"
					"\n"
					,
					get_wanface("wan"),
					upnp_port,
					(enable & 1) ? "yes" : "no",			/* upnp enable */
					(enable & 2) ? "yes" : "no",			/* natpmp enable */
					nvram_get_int("upnp_secure") ? "yes" : "no",	/* secure_mode (only forward to self) */
					nvram_get_int("upnp_ssdp_interval"),
					nvram_safe_get("router_name"),
					nvram_safe_get("t_model_name")
				);

				if (nvram_get_int("upnp_clean")) {
					int interval = nvram_get_int("upnp_clean_interval");
					if (interval < 60) interval = 60;
					fprintf(f,
						"clean_ruleset_interval=%d\n"
						"clean_ruleset_threshold=%d\n",
						interval,
						nvram_get_int("upnp_clean_threshold")
					);
				}
				else
					fprintf(f, "clean_ruleset_interval=0\n");

				if (nvram_match("upnp_mnp", "1")) {
					int https = nvram_get_int("https_enable");
					fprintf(f, "presentation_url=http%s://%s:%s/forward-upnp.asp\n",
						https ? "s" : "", nvram_safe_get("lan_ipaddr"),
						nvram_safe_get(https ? "https_lanport" : "http_lanport"));
				}
				else {
					/* Empty parameters are not included into XML service description */
					fprintf(f, "presentation_url=\n");
				}

				char uuid[45];
				f_read_string("/proc/sys/kernel/random/uuid", uuid, sizeof(uuid));
				fprintf(f, "uuid=%s\n", uuid);

				/* shibby - move custom configuration before "allow" statements */
				/* discussion: http://www.linksysinfo.org/index.php?threads/miniupnpd-custom-config-syntax.70863/#post-256291 */
				fappend(f, "/etc/upnp/config.custom");
				fprintf(f, "%s\n", nvram_safe_get("upnp_custom"));

				char lanN_ipaddr[] = "lanXX_ipaddr";
				char lanN_netmask[] = "lanXX_netmask";
				char lanN_ifname[] = "lanXX_ifname";
				char upnp_lanN[] = "upnp_lanXX";
				char br;

				for (br = 0 ; br < 4 ; br++) {
					char bridge[2] = "0";
					if (br != 0)
						bridge[0] += br;
					else
						strcpy(bridge, "");

					sprintf(lanN_ipaddr, "lan%s_ipaddr", bridge);
					sprintf(lanN_netmask, "lan%s_netmask", bridge);
					sprintf(lanN_ifname, "lan%s_ifname", bridge);
					sprintf(upnp_lanN, "upnp_lan%s", bridge);

					char *lanip = nvram_safe_get(lanN_ipaddr);
					char *lanmask = nvram_safe_get(lanN_netmask);
					char *lanifname = nvram_safe_get(lanN_ifname);
					char *lanlisten = nvram_safe_get(upnp_lanN);

					if ((strcmp(lanlisten, "1") == 0) && (strcmp(lanifname, "") != 0)) {
						fprintf(f,
							"listening_ip=%s\n",
							lanifname);
						int ports[4];
						if ((ports[0] = nvram_get_int("upnp_min_port_ext")) > 0 &&
						    (ports[1] = nvram_get_int("upnp_max_port_ext")) > 0 &&
						    (ports[2] = nvram_get_int("upnp_min_port_int")) > 0 &&
						    (ports[3] = nvram_get_int("upnp_max_port_int")) > 0) {
							fprintf(f,
								"allow %d-%d %s/%s %d-%d\n",
								ports[0], ports[1],
								lanip, lanmask,
								ports[2], ports[3]
							);
						}
						else {
							/* by default allow only redirection of ports above 1024 */
							fprintf(f, "allow 1024-65535 %s/%s 1024-65535\n", lanip, lanmask);
						}
					}
				}
				fprintf(f, "\ndeny 0-65535 0.0.0.0/0 0-65535\n");
				fclose(f);

				xstart("miniupnpd", "-f", "/etc/upnp/config");
			}
		}
	}
}

void stop_upnp(void)
{
	if (getpid() != 1) {
		stop_service("upnp");
		return;
	}

	killall_tk("miniupnpd");
}

static pid_t pid_crond = -1;

void start_cron(void)
{
	stop_cron();

	eval("crond", nvram_contains_word("log_events", "crond") ? NULL : "-l", "9");
	if (!nvram_contains_word("debug_norestart", "crond")) {
		pid_crond = -2;
	}
}

void stop_cron(void)
{
	pid_crond = -1;
	killall_tk("crond");
}

static pid_t pid_hotplug2 = -1;

void start_hotplug2()
{
	stop_hotplug2();

	f_write_string("/proc/sys/kernel/hotplug", "", FW_NEWLINE, 0);
	xstart("hotplug2", "--persistent", "--no-coldplug");
	/* FIXME: Don't remember exactly why I put "sleep" here - but it was not for a race with check_services()... - TB */
	sleep(1);

	if (!nvram_contains_word("debug_norestart", "hotplug2")) {
		pid_hotplug2 = -2;
	}
}

void stop_hotplug2(void)
{
	pid_hotplug2 = -1;
	killall_tk("hotplug2");
}

// Written by Sparq in 2002/07/16
#ifdef TCONFIG_ZEBRA
void start_zebra(void)
{
	if (getpid() != 1) {
		start_service("zebra");
		return;
	}

	FILE *fp;

	char *lan_tx = nvram_safe_get("dr_lan_tx");
	char *lan_rx = nvram_safe_get("dr_lan_rx");
	char *lan1_tx = nvram_safe_get("dr_lan1_tx");
	char *lan1_rx = nvram_safe_get("dr_lan1_rx");
	char *lan2_tx = nvram_safe_get("dr_lan2_tx");
	char *lan2_rx = nvram_safe_get("dr_lan2_rx");
	char *lan3_tx = nvram_safe_get("dr_lan3_tx");
	char *lan3_rx = nvram_safe_get("dr_lan3_rx");
	char *wan_tx = nvram_safe_get("dr_wan_tx");
	char *wan_rx = nvram_safe_get("dr_wan_rx");

	if ((*lan_tx == '0') && (*lan_rx == '0') &&
	    (*lan1_tx == '0') && (*lan1_rx == '0') &&
	    (*lan2_tx == '0') && (*lan2_rx == '0') &&
	    (*lan3_tx == '0') && (*lan3_rx == '0') &&
	    (*wan_tx == '0') && (*wan_rx == '0')) {
		return;
	}

	/* empty */
	if ((fp = fopen("/etc/zebra.conf", "w")) != NULL) {
		fclose(fp);
	}

	if ((fp = fopen("/etc/ripd.conf", "w")) != NULL) {
		char *lan_ifname = nvram_safe_get("lan_ifname");
		char *lan1_ifname = nvram_safe_get("lan1_ifname");
		char *lan2_ifname = nvram_safe_get("lan2_ifname");
		char *lan3_ifname = nvram_safe_get("lan3_ifname");
		char *wan_ifname = nvram_safe_get("wan_ifname");

		fprintf(fp, "router rip\n");
		if (strcmp(lan_ifname, "") != 0)
			fprintf(fp, "network %s\n", lan_ifname);
		if (strcmp(lan1_ifname, "") != 0)
			fprintf(fp, "network %s\n", lan1_ifname);
		if (strcmp(lan2_ifname, "") != 0)
			fprintf(fp, "network %s\n", lan2_ifname);
		if (strcmp(lan3_ifname, "") != 0)
			fprintf(fp, "network %s\n", lan3_ifname);
		fprintf(fp, "network %s\n", wan_ifname);
		fprintf(fp, "redistribute connected\n");
		// fprintf(fp, "redistribute static\n");

		/* 43011: modify by zg 2006.10.18 for cdrouter3.3 item 173(cdrouter_rip_30) bug */
		// fprintf(fp, "redistribute kernel\n"); /* 1.11: removed, redistributes indirect -- zzz */

		if (strcmp(lan_ifname, "") != 0) {
			fprintf(fp, "interface %s\n", lan_ifname);
			if (*lan_tx != '0') fprintf(fp, "ip rip send version %s\n", lan_tx);
			if (*lan_rx != '0') fprintf(fp, "ip rip receive version %s\n", lan_rx);
		}
		if (strcmp(lan1_ifname, "") != 0) {
			fprintf(fp, "interface %s\n", lan1_ifname);
			if (*lan1_tx != '0') fprintf(fp, "ip rip send version %s\n", lan1_tx);
			if (*lan1_rx != '0') fprintf(fp, "ip rip receive version %s\n", lan1_rx);
		}
		if (strcmp(lan2_ifname, "") != 0) {
			fprintf(fp, "interface %s\n", lan2_ifname);
			if (*lan2_tx != '0') fprintf(fp, "ip rip send version %s\n", lan2_tx);
			if (*lan2_rx != '0') fprintf(fp, "ip rip receive version %s\n", lan2_rx);
		}
		if (strcmp(lan3_ifname, "") != 0) {
			fprintf(fp, "interface %s\n", lan3_ifname);
			if (*lan3_tx != '0') fprintf(fp, "ip rip send version %s\n", lan3_tx);
			if (*lan3_rx != '0') fprintf(fp, "ip rip receive version %s\n", lan3_rx);
		}
		fprintf(fp, "interface %s\n", wan_ifname);
		if (*wan_tx != '0') fprintf(fp, "ip rip send version %s\n", wan_tx);
		if (*wan_rx != '0') fprintf(fp, "ip rip receive version %s\n", wan_rx);

		fprintf(fp, "router rip\n");
		if (strcmp(lan_ifname, "") != 0) {
			if (*lan_tx == '0') fprintf(fp, "distribute-list private out %s\n", lan_ifname);
			if (*lan_rx == '0') fprintf(fp, "distribute-list private in %s\n", lan_ifname);
		}
		if (strcmp(lan1_ifname, "") != 0) {
			if (*lan1_tx == '0') fprintf(fp, "distribute-list private out %s\n", lan1_ifname);
			if (*lan1_rx == '0') fprintf(fp, "distribute-list private in %s\n", lan1_ifname);
		}
		if (strcmp(lan2_ifname, "") != 0) {
			if (*lan2_tx == '0') fprintf(fp, "distribute-list private out %s\n", lan2_ifname);
			if (*lan2_rx == '0') fprintf(fp, "distribute-list private in %s\n", lan2_ifname);
		}
		if (strcmp(lan3_ifname, "") != 0) {
			if (*lan3_tx == '0') fprintf(fp, "distribute-list private out %s\n", lan3_ifname);
			if (*lan3_rx == '0') fprintf(fp, "distribute-list private in %s\n", lan3_ifname);
		}
		if (*wan_tx == '0') fprintf(fp, "distribute-list private out %s\n", wan_ifname);
		if (*wan_rx == '0') fprintf(fp, "distribute-list private in %s\n", wan_ifname);
		fprintf(fp, "access-list private deny any\n");

		// fprintf(fp, "debug rip events\n");
		// fprintf(fp, "log file /etc/ripd.log\n");
		fclose(fp);
	}

	xstart("zebra", "-d");
	xstart("ripd",  "-d");
}

void stop_zebra(void)
{
	if (getpid() != 1) {
		stop_service("zebra");
		return;
	}

	killall("zebra", SIGTERM);
	killall("ripd", SIGTERM);

	unlink("/etc/zebra.conf");
	unlink("/etc/ripd.conf");
}
#endif /* #ifdef TCONFIG_ZEBRA */

void start_syslog(void)
{
	char *argv[16];
	int argc;
	char *nv;
	char *b_opt = "";
	char rem[256];
	int n;
	char s[64];
	char cfg[256];
	char *rot_siz = "50";
	char *rot_keep = "1";
	char *log_file_path;
	char log_default[] = "/var/log/messages";

	argv[0] = "syslogd";
	argc = 1;

	if (nvram_match("log_remote", "1")) {
		nv = nvram_safe_get("log_remoteip");
		if (*nv) {
			snprintf(rem, sizeof(rem), "%s:%s", nv, nvram_safe_get("log_remoteport"));
			argv[argc++] = "-R";
			argv[argc++] = rem;
		}
	}

	if (nvram_match("log_file", "1")) {
		argv[argc++] = "-L";

		if (strcmp(nvram_safe_get("log_file_size"), "") != 0) {
			rot_siz = nvram_safe_get("log_file_size");
		}
		if (nvram_get_int("log_file_size") > 0) {
			rot_keep = nvram_safe_get("log_file_keep");
		}

		/* log to custom path - shibby */
		if (nvram_match("log_file_custom", "1")) {
			log_file_path = nvram_safe_get("log_file_path");
			argv[argc++] = "-s";
			argv[argc++] = rot_siz;
			argv[argc++] = "-O";
			argv[argc++] = log_file_path;
			if (strcmp(nvram_safe_get("log_file_path"), log_default) != 0) {
				remove(log_default);
				symlink(log_file_path, log_default);
			}
		}
		else

		/* Read options:    rotate_size(kb)    num_backups    logfilename.
		 * Ignore these settings and use defaults if the logfile cannot be written to.
		 */
		if (f_read_string("/etc/syslogd.cfg", cfg, sizeof(cfg)) > 0) {
			if ((nv = strchr(cfg, '\n')))
				*nv = 0;

			if ((nv = strtok(cfg, " \t"))) {
				if (isdigit(*nv))
					rot_siz = nv;
			}

			if ((nv = strtok(NULL, " \t")))
				b_opt = nv;

			if ((nv = strtok(NULL, " \t")) && *nv == '/') {
				if (f_write(nv, cfg, 0, FW_APPEND, 0) >= 0) {
					argv[argc++] = "-O";
					argv[argc++] = nv;
				}
				else {
					rot_siz = "50";
					b_opt = "";
				}
			}
		}

		if (nvram_match("log_file_custom", "0")) {
			argv[argc++] = "-s";
			argv[argc++] = rot_siz;
			struct stat sb;
			if (lstat(log_default, &sb) != -1)
				if (S_ISLNK(sb.st_mode))
					remove(log_default);
		}

		if (isdigit(*b_opt)) {
			argv[argc++] = "-b";
			argv[argc++] = b_opt;
		} else if (nvram_get_int("log_file_size") > 0) {
			argv[argc++] = "-b";
			argv[argc++] = rot_keep;
		}
	}

	if (argc > 1) {
		argv[argc] = NULL;
		_eval(argv, NULL, 0, NULL);

		argv[0] = "klogd";
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);

		/* used to be available in syslogd -m */
		n = nvram_get_int("log_mark");
		if (n > 0) {
			/* n is in minutes */
			if (n < 60)
				sprintf(rem, "*/%d * * * *", n);
			else if (n < 60 * 24)
				sprintf(rem, "0 */%d * * *", n / 60);
			else
				sprintf(rem, "0 0 */%d * *", n / (60 * 24));
			sprintf(s, "%s logger -p syslog.info -- -- MARK --", rem);
			eval("cru", "a", "syslogdmark", s);
		}
		else {
			eval("cru", "d", "syslogdmark");
		}
	}
}

void stop_syslog(void)
{
	killall("klogd", SIGTERM);
	killall("syslogd", SIGTERM);
}

static pid_t pid_igmp = -1;

void start_igmp_proxy(void)
{
	FILE *fp;
	char igmp_buffer[32];
	char wan_prefix[] = "wanXX";
	int wan_unit, mwan_num, count = 0;

	mwan_num = nvram_get_int("mwan_num");
	if (mwan_num < 1 || mwan_num > MWAN_MAX) {
		mwan_num = 1;
	}

	pid_igmp = -1;
	if (nvram_match("multicast_pass", "1")) {
		int ret = 0;

		if (f_exists("/etc/igmp.alt")) {
			ret = eval("igmpproxy", "/etc/igmp.alt");
		}
		else if ((fp = fopen("/etc/igmp.conf", "w")) != NULL) {
			/* check that lan, lan1, lan2 and lan3 are not selected and use custom config */
			/* The configuration file must define one (or more) upstream interface(s) and one or more downstream interfaces,
			 * see https://github.com/pali/igmpproxy/commit/b55e0125c79fc9dbc95c6d6ab1121570f0c6f80f and
			 * see https://github.com/pali/igmpproxy/blob/master/igmpproxy.conf
			 */
			if (nvram_match("multicast_lan", "0") && nvram_match("multicast_lan1", "0") && nvram_match("multicast_lan2", "0") && nvram_match("multicast_lan3", "0")) {
				fprintf(fp, "%s\n", nvram_safe_get("multicast_custom"));
				fclose(fp);
				ret = eval("igmpproxy", "/etc/igmp.conf");
			}
			/* create default config for upstream/downstream interface(s) */
			else {
				if (nvram_match("multicast_quickleave", "1")) {
					fprintf(fp,
						"quickleave\n");
				}
				for (wan_unit = 1; wan_unit <= mwan_num; ++wan_unit) {
					get_wan_prefix(wan_unit, wan_prefix);
					if ((check_wanup(wan_prefix)) && (get_wanx_proto(wan_prefix) != WP_DISABLED)) {
						count++;
						/*
						 * Configuration for Upstream Interface
						 * Example:
						 * phyint ppp0 upstream ratelimit 0 threshold 1
						 * altnet 193.158.35.0/24
						 */
						fprintf(fp,
							"phyint %s upstream ratelimit 0 threshold 1\n",
							get_wanface(wan_prefix));
						if ((nvram_get("multicast_altnet_1") != NULL) ||
						    (nvram_get("multicast_altnet_2") != NULL) ||
						    (nvram_get("multicast_altnet_3") != NULL)) { /* check for allowed remote network address, see note at GUI advanced-firewall.asp */
							if (nvram_get("multicast_altnet_1") != NULL) {
								memset(igmp_buffer, 0, sizeof(igmp_buffer)); /* reset */
								snprintf(igmp_buffer, sizeof(igmp_buffer),"%s", nvram_safe_get("multicast_altnet_1")); /* copy to buffer */
								fprintf(fp,
									"\taltnet %s\n", igmp_buffer); /* with the following format: a.b.c.d/n - Example: altnet 10.0.0.0/16 */
								syslog(LOG_INFO, "igmpproxy: multicast_altnet_1 = %s\n", igmp_buffer);
							}

							if (nvram_get("multicast_altnet_2") != NULL) {
								memset(igmp_buffer, 0, sizeof(igmp_buffer)); /* reset */
								snprintf(igmp_buffer, sizeof(igmp_buffer),"%s", nvram_safe_get("multicast_altnet_2")); /* copy to buffer */
								fprintf(fp,
									"\taltnet %s\n", igmp_buffer); /* with the following format: a.b.c.d/n - Example: altnet 10.0.0.0/16 */
								syslog(LOG_INFO, "igmpproxy: multicast_altnet_2 = %s\n", igmp_buffer);
							}

							if (nvram_get("multicast_altnet_3") != NULL) {
								memset(igmp_buffer, 0, sizeof(igmp_buffer)); /* reset */
								snprintf(igmp_buffer, sizeof(igmp_buffer),"%s", nvram_safe_get("multicast_altnet_3")); /* copy to buffer */
								fprintf(fp,
									"\taltnet %s\n", igmp_buffer); /* with the following format: a.b.c.d/n - Example: altnet 10.0.0.0/16 */
								syslog(LOG_INFO, "igmpproxy: multicast_altnet_3 = %s\n", igmp_buffer);
							}
						}
						else {
							fprintf(fp,
								"\taltnet 0.0.0.0/0\n"); /* default, allow all! */
						}
					}
				}
				if (!count) {
					fclose(fp);
					unlink("/etc/igmp.conf");
					return;
				}
					char lanN_ifname[] = "lanXX_ifname";
					char multicast_lanN[] = "multicast_lanXX";
					char br;

					for (br = 0 ; br < 4 ; br++) {
						char bridge[2] = "0";
						if (br != 0)
							bridge[0]+=br;
						else
							strcpy(bridge, "");

						sprintf(lanN_ifname, "lan%s_ifname", bridge);
						sprintf(multicast_lanN, "multicast_lan%s", bridge);

						if ((strcmp(nvram_safe_get(multicast_lanN), "1") == 0) && (strcmp(nvram_safe_get(lanN_ifname), "") != 0)) {
						/*
						 * Configuration for Downstream Interface
						 * Example:
						 * phyint br0 downstream ratelimit 0 threshold 1
						 */
							fprintf(fp,
								"phyint %s downstream ratelimit 0 threshold 1\n",
								nvram_safe_get(lanN_ifname));
						}
					}
				fclose(fp);
				ret = eval("igmpproxy", "/etc/igmp.conf");
			}
		}
		else {
			return;
		}
		if (!nvram_contains_word("debug_norestart", "igmprt")) {
			pid_igmp = -2;
		}
		if (ret) {
			syslog(LOG_INFO, "starting igmpproxy failed ...\n");
		} else {
			syslog(LOG_INFO, "igmpproxy is started\n");
		}
	}
}

void stop_igmp_proxy(void)
{
	pid_igmp = -1;
	killall_tk("igmpproxy");

	syslog(LOG_INFO, "igmpproxy is stopped\n");
}

void start_udpxy(void)
{
	char wan_prefix[] = "wan";	/* not yet mwan ready, use wan for now */
	char buffer[32], buffer2[16];
	int i, bind_lan = 0;

	/* check if udpxy is enabled via GUI, advanced-firewall.asp */
	if (nvram_match("udpxy_enable", "1")) {
		if ((check_wanup(wan_prefix)) && (get_wanx_proto(wan_prefix) != WP_DISABLED)) {
			memset(buffer, 0, sizeof(buffer));					/* reset */
			snprintf(buffer, sizeof(buffer), "%s", get_wanface(wan_prefix));	/* copy wanface to buffer */

			/* check interface to listen on */
			/* check udpxy enabled/selected for br0 - br3 */
			for (i = 0; i < 4; i++) {
				int ret1 = 0, ret2 = 0;
				sprintf(buffer2, (i == 0 ? "udpxy_lan" : "udpxy_lan%d"), i);
				ret1 = nvram_match(buffer2, "1");
				sprintf(buffer2, (i == 0 ? "lan_ipaddr" : "lan%d_ipaddr"), i);
				ret2 = strcmp(nvram_safe_get(buffer2), "") != 0;
				if (ret1 && ret2) {
					sprintf(buffer2, (i == 0 ? "lan_ifname" : "lan%d_ifname"), i);
					eval("udpxy", (nvram_get_int("udpxy_stats") ? "-S" : ""), "-p", nvram_safe_get("udpxy_port"), "-c", nvram_safe_get("udpxy_clients"), "-a", nvram_safe_get(buffer2), "-m", buffer);
					bind_lan = 1;
					break;	/* start udpxy only once and only for one lanX */
				}
			}
			/* address/interface to listen on: default = 0.0.0.0 */
			if (!bind_lan) {
				eval("udpxy", (nvram_get_int("udpxy_stats") ? "-S" : ""), "-p", nvram_safe_get("udpxy_port"), "-c", nvram_safe_get("udpxy_clients"), "-m", buffer);
			}
		}
		else {
			/* do nothing */
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "udpxy not started!\n");
#endif
		}
	}
}

void stop_udpxy(void)
{
	killall_tk("udpxy");
}

#ifdef TCONFIG_NOCAT
static pid_t pid_splashd = -1;
void start_splashd(void)
{
	pid_splashd = -1;
	start_nocat();
	if (!nvram_contains_word("debug_norestart", "splashd")) {
		pid_splashd = -2;
	}
}

void stop_splashd(void)
{
	pid_splashd = -1;
	stop_nocat();
	start_wan(BOOT);
}
#endif	/* #ifdef TCONFIG_NOCAT */

#ifdef TCONFIG_NGINX
static pid_t pid_nginx = -1;
void start_enginex(void)
{
	pid_nginx =-1;
	start_nginx();
	if (!nvram_contains_word("debug_norestart", "enginex")) {
		pid_nginx = -2;
	}
}

void stop_enginex(void)
{
	pid_nginx = -1;
	stop_nginx();
}

void start_nginxfastpath(void)
{
	pid_nginx =-1;
	start_nginxfp();
	if (!nvram_contains_word("debug_norestart", "nginxfp")) {
		pid_nginx = -2;
	}
}
void stop_nginxfastpath(void)
{
	pid_nginx = -1;
	stop_nginxfp();
}
#endif	/* #ifdef TCONFIG_NGINX */

void set_tz(void)
{
	f_write_string("/etc/TZ", nvram_safe_get("tm_tz"), FW_CREATE|FW_NEWLINE, 0644);
}

void start_ntpd(void)
{
	char *servers, *ptr;
	int servers_len = 0, ntp_updates_int = 0;
	FILE *f;

	if (getpid() != 1) {
		start_service("ntpd");
		return;
	}

	set_tz();

	stop_ntpd();

	if (nvram_match("dnscrypt_proxy", "1") || nvram_match("stubby_proxy", "1")) {
		eval("ntp2ip");
	}

	/* This is the nvram var defining how the server should be run / how often to sync */
	ntp_updates_int = nvram_get_int("ntp_updates");

	/* The Tomato GUI allows the User to select an NTP Server region, and then string concats 1. 2. and 3. as prefix */
	/* Therefore, the nvram variable contains a string of 3 NTP servers - This code separates them and passes them to */
	/* ntpd as separate parameters.  This code should continue to work if GUI is changed to only store 1 value in the NVRAM var */
	if (ntp_updates_int >= 0) {
		servers_len = strlen(nvram_safe_get("ntp_server"));

		/* Allocating memory dynamically both so we don't waste memory, and in case of unanticipatedly long server name in nvram */
		if ((servers = malloc(servers_len + 1)) == NULL) {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "ntpd: failed allocating memory, exiting\n");
#endif
			return;			/* Just get out if we couldn't allocate memory */
		}
		memset(servers, 0, sizeof(servers));

		/* Get the space separated list of ntp servers */
		strcpy(servers, nvram_safe_get("ntp_server"));

		/* Put the servers into the ntp config file */
		if ((f = fopen("/etc/ntp.conf", "w")) != NULL) {
			ptr = strtok(servers, " ");
			while(ptr) {
				fprintf(f, "server %s\n", ptr);
				ptr = strtok(NULL, " ");
			}
		}
		fclose(f);
		free(servers);

		if (ntp_updates_int == 0) {		/* Only at startup */
			xstart("ntpd", "-q");
		}
		else if (ntp_updates_int >= 1) {	/* Auto adjusted timing by ntpd since it doesn't currently implement minpoll and maxpoll */
			xstart("ntpd", "-l");
		}
	}
}

void stop_ntpd(void)
{
	if (getpid() != 1) {
		stop_service("ntpd");
		return;
	}

	killall_tk("ntpd");
}

static void stop_rstats(void)
{
	int n, m;
	int pid;
	int pidz;
	int ppidz;
	int w = 0;

	n = 60;
	m = 15;
	while ((n-- > 0) && ((pid = pidof("rstats")) > 0)) {
		w = 1;
		pidz = pidof("gzip");
		if (pidz < 1) pidz = pidof("cp");
		ppidz = ppid(ppid(pidz));
		if ((m > 0) && (pidz > 0) && (pid == ppidz)) {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "rstats(PID %d) shutting down, waiting for helper process to complete (PID %d, PPID %d).\n", pid, pidz, ppidz);
#endif
			--m;
		} else {
			kill(pid, SIGTERM);
		}
		sleep(1);
	}
#ifndef TCONFIG_OPTIMIZE_SIZE
	if ((w == 1) && (n > 0))
		syslog(LOG_DEBUG, "rstats stopped.\n");
#endif
}

static void start_rstats(int new)
{
	if (nvram_match("rstats_enable", "1")) {
		stop_rstats();
		if (new) {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "starting rstats (new datafile).\n");
#endif
			xstart("rstats", "--new");
		} else {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "starting rstats.\n");
#endif
			xstart("rstats");
		}
	}
}

static void stop_cstats(void)
{
	int n, m;
	int pid;
	int pidz;
	int ppidz;
	int w = 0;

	n = 60;
	m = 15;
	while ((n-- > 0) && ((pid = pidof("cstats")) > 0)) {
		w = 1;
		pidz = pidof("gzip");
		if (pidz < 1) pidz = pidof("cp");
		ppidz = ppid(ppid(pidz));
		if ((m > 0) && (pidz > 0) && (pid == ppidz)) {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "cstats(PID %d) shutting down, waiting for helper process to complete (PID %d, PPID %d).\n", pid, pidz, ppidz);
#endif
			--m;
		} else {
			kill(pid, SIGTERM);
		}
		sleep(1);
	}
#ifndef TCONFIG_OPTIMIZE_SIZE
	if ((w == 1) && (n > 0))
		syslog(LOG_DEBUG, "cstats stopped.\n");
#endif
}

static void start_cstats(int new)
{
	if (nvram_match("cstats_enable", "1")) {
		stop_cstats();
		if (new) {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "starting cstats (new datafile).\n");
#endif
			xstart("cstats", "--new");
		} else {
#ifndef TCONFIG_OPTIMIZE_SIZE
			syslog(LOG_DEBUG, "starting cstats.\n");
#endif
			xstart("cstats");
		}
	}
}

/* !!TB - FTP Server */
#ifdef TCONFIG_FTP
static char *get_full_storage_path(char *val)
{
	static char buf[128];
	int len;

	if (val[0] == '/')
		len = sprintf(buf, "%s", val);
	else
		len = sprintf(buf, "%s/%s", MOUNT_ROOT, val);

	if (len > 1 && buf[len - 1] == '/')
		buf[len - 1] = 0;

	return buf;
}

static char *nvram_storage_path(char *var)
{
	char *val = nvram_safe_get(var);
	return get_full_storage_path(val);
}

char vsftpd_conf[] =  "/etc/vsftpd.conf";
char vsftpd_users[] = "/etc/vsftpd.users";
char vsftpd_passwd[] = "/etc/vsftpd.passwd";

static void start_ftpd(void)
{
	char tmp[256];
	FILE *fp, *f;
	char *buf;
	char *p, *q;
	char *user, *pass, *rights, *root_dir;
	int i;

	if (getpid() != 1) {
		start_service("ftpd");
		return;
	}

	if (!nvram_get_int("ftp_enable")) return;

	mkdir_if_none(vsftpd_users);
	mkdir_if_none("/var/run/vsftpd");

	if ((fp = fopen(vsftpd_conf, "w")) == NULL)
		return;

	if (nvram_get_int("ftp_super")) {
		/* rights */
		sprintf(tmp, "%s/%s", vsftpd_users, "admin");
		if ((f = fopen(tmp, "w"))) {
			fprintf(f,
				"dirlist_enable=yes\n"
				"write_enable=yes\n"
				"download_enable=yes\n");
			fclose(f);
		}
	}

	if (nvram_invmatch("ftp_anonymous", "0")) {
		fprintf(fp,
			"anon_allow_writable_root=yes\n"
			"anon_world_readable_only=no\n"
			"anon_umask=022\n");

		/* rights */
		sprintf(tmp, "%s/ftp", vsftpd_users);
		if ((f = fopen(tmp, "w"))) {
			if (nvram_match("ftp_dirlist", "0"))
				fprintf(f, "dirlist_enable=yes\n");
			if (nvram_match("ftp_anonymous", "1") ||
			    nvram_match("ftp_anonymous", "3"))
				fprintf(f, "write_enable=yes\n");
			if (nvram_match("ftp_anonymous", "1") ||
			    nvram_match("ftp_anonymous", "2"))
				fprintf(f, "download_enable=yes\n");
			fclose(f);
		}
		if (nvram_match("ftp_anonymous", "1") ||
		    nvram_match("ftp_anonymous", "3"))
			fprintf(fp,
				"anon_upload_enable=yes\n"
				"anon_mkdir_write_enable=yes\n"
				"anon_other_write_enable=yes\n");
	}
	else {
		fprintf(fp, "anonymous_enable=no\n");
	}

	fprintf(fp,
		"dirmessage_enable=yes\n"
		"download_enable=no\n"
		"dirlist_enable=no\n"
		"hide_ids=yes\n"
		"syslog_enable=yes\n"
		"local_enable=yes\n"
		"local_umask=022\n"
		"chmod_enable=no\n"
		"chroot_local_user=yes\n"
		"check_shell=no\n"
		"log_ftp_protocol=%s\n"
		"user_config_dir=%s\n"
		"passwd_file=%s\n"
		"listen%s=yes\n"
		"listen%s=no\n"
		"listen_port=%s\n"
		"background=yes\n"
		"isolate=no\n"
		"max_clients=%d\n"
		"max_per_ip=%d\n"
		"max_login_fails=1\n"
		"idle_session_timeout=%s\n"
		"use_sendfile=no\n"
		"anon_max_rate=%d\n"
		"local_max_rate=%d\n"
		"%s\n",
		nvram_get_int("log_ftp") ? "yes" : "no",
		vsftpd_users, vsftpd_passwd,
#ifdef TCONFIG_IPV6
		ipv6_enabled() ? "_ipv6" : "",
		ipv6_enabled() ? "" : "_ipv6",
#else
		"",
		"_ipv6",
#endif
		nvram_get("ftp_port") ? : "21",
		nvram_get_int("ftp_max"),
		nvram_get_int("ftp_ipmax"),
		nvram_get("ftp_staytimeout") ? : "300",
		nvram_get_int("ftp_anonrate") * 1024,
		nvram_get_int("ftp_rate") * 1024,
		nvram_safe_get("ftp_custom"));

	fclose(fp);

	/* prepare passwd file and default users */
	if ((fp = fopen(vsftpd_passwd, "w")) == NULL)
		return;

	if (((user = nvram_get("http_username")) == NULL) || (*user == 0)) user = "admin";
	if (((pass = nvram_get("http_passwd")) == NULL) || (*pass == 0)) pass = "admin";

	fprintf(fp, /* anonymous, admin, nobody */
		"ftp:x:0:0:ftp:%s:/sbin/nologin\n"
		"%s:%s:0:0:root:/:/sbin/nologin\n"
		"nobody:x:65534:65534:nobody:%s/:/sbin/nologin\n",
		nvram_storage_path("ftp_anonroot"), user,
		nvram_get_int("ftp_super") ? crypt(pass, "$1$") : "x",
		MOUNT_ROOT);

	if ((buf = strdup(nvram_safe_get("ftp_users"))) != NULL) {
		/*
		username<password<rights[<root_dir>]
		rights:
			Read/Write
			Read Only
			View Only
			Private
		*/
		p = buf;
		while ((q = strsep(&p, ">")) != NULL) {
			i = vstrsep(q, "<", &user, &pass, &rights, &root_dir);
			if (i < 3 || i > 4) continue;
			if (!user || !pass) continue;

			if (i == 3 || !root_dir || !(*root_dir))

			root_dir = nvram_safe_get("ftp_pubroot");

			/* directory */
			if (strncmp(rights, "Private", 7) == 0) {
				sprintf(tmp, "%s/%s", nvram_storage_path("ftp_pvtroot"), user);
				mkdir_if_none(tmp);
			}
			else
				sprintf(tmp, "%s", get_full_storage_path(root_dir));

			fprintf(fp, "%s:%s:0:0:%s:%s:/sbin/nologin\n",
				user, crypt(pass, "$1$"), user, tmp);

			/* rights */
			sprintf(tmp, "%s/%s", vsftpd_users, user);
			if ((f = fopen(tmp, "w"))) {
				tmp[0] = 0;
				if (nvram_invmatch("ftp_dirlist", "1"))
					strcat(tmp, "dirlist_enable=yes\n");
				if (strstr(rights, "Read") || !strcmp(rights, "Private"))
					strcat(tmp, "download_enable=yes\n");
				if (strstr(rights, "Write") || !strncmp(rights, "Private", 7))
					strcat(tmp, "write_enable=yes\n");

				fputs(tmp, f);
				fclose(f);
			}
		}
		free(buf);
	}

	fclose(fp);
	killall("vsftpd", SIGHUP);

	/* start vsftpd if it's not already running */
	if (pidof("vsftpd") <= 0) {
		int ret = 0;

		ret = xstart("vsftpd");
		if (ret) {
			syslog(LOG_INFO, "starting vsftpd failed ...\n");
		}
		else {
			syslog(LOG_INFO, "vsftpd is started\n");
		}
	}
}

static void stop_ftpd(void)
{
	if (getpid() != 1) {
		stop_service("ftpd");
		return;
	}

	killall_tk("vsftpd");
	unlink(vsftpd_passwd);
	unlink(vsftpd_conf);
	eval("rm", "-rf", vsftpd_users);

	syslog(LOG_INFO, "vsftpd is stopped\n");
}
#endif	/* #ifdef TCONFIG_FTP */

/* !!TB - Samba */
#ifdef TCONFIG_SAMBASRV
static void kill_samba(int sig)
{
	if (sig == SIGTERM) {
		killall_tk("smbd");
		killall_tk("nmbd");
	}
	else {
		killall("smbd", sig);
		killall("nmbd", sig);
	}
}

static void start_samba(void)
{
	FILE *fp;
	DIR *dir = NULL;
	struct dirent *dp;
	char nlsmod[15];
	int mode;
	char *nv;
	char *si;

	if (getpid() != 1) {
		start_service("smbd");
		return;
	}

	mode = nvram_get_int("smbd_enable");
	if (!mode || !nvram_invmatch("lan_hostname", ""))
		return;

	if ((fp = fopen("/etc/smb.conf", "w")) == NULL)
		return;

	si = nvram_safe_get("smbd_ifnames");

	fprintf(fp, "[global]\n"
		" interfaces = %s\n"
		" bind interfaces only = yes\n"
		" enable core files = no\n"
		" deadtime = 30\n"
		" smb encrypt = disabled\n"
		" min receivefile size = 16384\n"
		" workgroup = %s\n"
		" netbios name = %s\n"
		" server string = %s\n"
		" dos charset = ASCII\n"
		" unix charset = UTF8\n"
		" display charset = UTF8\n"
		" guest account = nobody\n"
		" security = user\n"
		" %s\n"
		" guest ok = %s\n"
		" guest only = no\n"
		" browseable = yes\n"
		" syslog only = yes\n"
		" timestamp logs = no\n"
		" syslog = 1\n"
		" passdb backend = smbpasswd\n"
		" encrypt passwords = yes\n"
		" preserve case = yes\n"
		" short preserve case = yes\n",
		strlen(si) ? si : nvram_safe_get("lan_ifname"),
		nvram_get("smbd_wgroup") ? : "WORKGROUP",
		nvram_safe_get("lan_hostname"),
		nvram_get("router_name") ? : "FreshTomato",
		mode == 2 ? "" : "map to guest = Bad User",
		mode == 2 ? "no" : "yes"	// guest ok
	);

	fprintf(fp, " load printers = no\n"	/* add for Samba printcap issue */
		" printing = bsd\n"
		" printcap name = /dev/null\n"
		" map archive = no\n"
		" map hidden = no\n"
		" map read only = no\n"
		" map system = no\n"
		" store dos attributes = no\n"
		" dos filemode = yes\n"
		" strict locking = no\n"
		" oplocks = yes\n"
		" level2 oplocks = yes\n"
		" kernel oplocks = no\n"
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
		" use sendfile = no\n");
#else
		" use sendfile = yes\n");
#endif

	if (nvram_get_int("smbd_wins")) {
		nv = nvram_safe_get("wan_wins");
		if ((*nv == 0) || (strcmp(nv, "0.0.0.0") == 0)) {
			fprintf(fp, " wins support = yes\n");
		}
	}

	/* 0 - smb1, 1 - smb2, 2 - smb1 + smb2 */
	if (nvram_get_int("smbd_protocol") == 0)
		fprintf(fp, " max protocol = NT1\n");
	else
		fprintf(fp, " max protocol = SMB2\n");
	if (nvram_get_int("smbd_protocol") == 1)
		fprintf(fp, " min protocol = SMB2\n");

	if (nvram_get_int("smbd_master")) {
		fprintf(fp,
			" domain master = yes\n"
			" local master = yes\n"
			" preferred master = yes\n"
			" os level = 255\n");
	}

	nv = nvram_safe_get("smbd_cpage");
	if (*nv) {
#ifndef TCONFIG_SAMBA3
		fprintf(fp, " client code page = %s\n", nv);
#endif
		sprintf(nlsmod, "nls_cp%s", nv);

		nv = nvram_safe_get("smbd_nlsmod");
		if ((*nv) && (strcmp(nv, nlsmod) != 0))
			modprobe_r(nv);

		modprobe(nlsmod);
		nvram_set("smbd_nlsmod", nlsmod);
	}

#ifndef TCONFIG_SAMBA3
	if (nvram_match("smbd_cset", "utf8"))
		fprintf(fp, " coding system = utf8\n");
	else if (nvram_invmatch("smbd_cset", ""))
		fprintf(fp, " character set = %s\n", nvram_safe_get("smbd_cset"));
#endif

	nv = nvram_safe_get("smbd_custom");
	/* add socket options unless overriden by the user */
	if (strstr(nv, "socket options") == NULL) {
		fprintf(fp, " socket options = TCP_NODELAY SO_KEEPALIVE IPTOS_LOWDELAY SO_RCVBUF=65536 SO_SNDBUF=65536\n");
	}
	fprintf(fp, "%s\n", nv);

	/* configure shares */

	char *buf;
	char *p, *q;
	char *name, *path, *comment, *writeable, *hidden;
	int cnt = 0;

	if ((buf = strdup(nvram_safe_get("smbd_shares"))) != NULL) {
		/* sharename<path<comment<writeable[0|1]<hidden[0|1] */

		p = buf;
		while ((q = strsep(&p, ">")) != NULL) {
			if (vstrsep(q, "<", &name, &path, &comment, &writeable, &hidden) != 5) continue;
			if (!path || !name) continue;

			/* share name */
			fprintf(fp, "\n[%s]\n", name);

			/* path */
			fprintf(fp, " path = %s\n", path);

			/* access level */
			if (!strcmp(writeable, "1"))
				fprintf(fp, " writable = yes\n delete readonly = yes\n force user = root\n");
			if (!strcmp(hidden, "1"))
				fprintf(fp, " browseable = no\n");

			/* comment */
			if (comment)
				fprintf(fp, " comment = %s\n", comment);

			cnt++;
		}
		free(buf);
	}

	/* Share every mountpoint below MOUNT_ROOT */
	if (nvram_get_int("smbd_autoshare") && (dir = opendir(MOUNT_ROOT))) {
		while ((dp = readdir(dir))) {
			if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {

				/* Only if is a directory and is mounted */
				if (!dir_is_mountpoint(MOUNT_ROOT, dp->d_name))
					continue;

				/* smbd_autoshare: 0 - disable, 1 - read-only, 2 - writable, 3 - hidden writable */
				fprintf(fp, "\n[%s]\n path = %s/%s\n comment = %s\n",
					dp->d_name, MOUNT_ROOT, dp->d_name, dp->d_name);
				if (nvram_match("smbd_autoshare", "3"))	// Hidden
					fprintf(fp, "\n[%s$]\n path = %s/%s\n browseable = no\n",
						dp->d_name, MOUNT_ROOT, dp->d_name);
				if (nvram_match("smbd_autoshare", "2") || nvram_match("smbd_autoshare", "3"))	// RW
					fprintf(fp, " writable = yes\n delete readonly = yes\n force user = root\n");

				cnt++;
			}
		}
	}
	if (dir) closedir(dir);

	if (cnt == 0) {
		/* by default share MOUNT_ROOT as read-only */
		fprintf(fp, "\n[share]\n"
			" path = %s\n"
			" writable = no\n",
			MOUNT_ROOT);
	}

	fclose(fp);

	mkdir_if_none("/var/run/samba");
	mkdir_if_none("/etc/samba");

	/* write smbpasswd */
#ifdef TCONFIG_SAMBA3
	eval("smbpasswd", "nobody", "\"\"");
#else
	eval("smbpasswd", "-a", "nobody", "\"\"");
#endif
	if (mode == 2) {
		char *smbd_user;
		if (((smbd_user = nvram_get("smbd_user")) == NULL) || (*smbd_user == 0) || !strcmp(smbd_user, "root"))
			smbd_user = "nas";
#ifdef TCONFIG_SAMBA3
		eval("smbpasswd", smbd_user, nvram_safe_get("smbd_passwd"));
#else
		eval("smbpasswd", "-a", smbd_user, nvram_safe_get("smbd_passwd"));
#endif
	}

	kill_samba(SIGHUP);
	int ret1 = 0, ret2 = 0;

	/* start samba if it's not already running */
	if (pidof("nmbd") <= 0)
		ret1 = xstart("nmbd", "-D");

	if (pidof("smbd") <= 0) {
		ret2 = xstart("smbd", "-D");
	}

	if (ret1 || ret2) {
		kill_samba(SIGTERM);
		syslog(LOG_INFO, "starting Samba daemon failed ...\n");
	}
	else {
		start_wsdd();
		syslog(LOG_INFO, "Samba daemon is started\n");
	}
}

static void stop_samba(void)
{
	if (getpid() != 1) {
		stop_service("smbd");
		return;
	}

	stop_wsdd();
	kill_samba(SIGTERM);
	/* clean up */
	unlink("/var/log/smb");
	unlink("/var/log/nmb");
	eval("rm", "-rf", "/var/run/samba");

	syslog(LOG_INFO, "samba daemon is stopped\n");
}

void start_wsdd()
{
	unsigned char ea[ETHER_ADDR_LEN];
	char serial[18];
	pid_t pid;
	char bootparms[64];
	char *wsdd_argv[] = { "/usr/sbin/wsdd2",
				"-d",
				"-w",
				"-i",	/* no multi-interface binds atm */
				nvram_safe_get("lan_ifname"),
				"-b",
				NULL,	/* boot parameters */
				NULL
			};
	stop_wsdd();

	if (!ether_atoe(nvram_safe_get("lan_hwaddr"), ea))
		f_read("/dev/urandom", ea, sizeof(ea));

	snprintf(serial, sizeof(serial), "%02x%02x%02x%02x%02x%02x",
		ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);

	snprintf(bootparms, sizeof(bootparms), "sku:%s,serial:%s", (nvram_get("odmpid") ? : "FreshTomato"), serial);
	wsdd_argv[6] = bootparms;

	_eval(wsdd_argv, NULL, 0, &pid);
}

void stop_wsdd() {
	if (pidof("wsdd2") > 0)
		killall_tk("wsdd2");
}
#endif	/* #ifdef TCONFIG_SAMBASRV */

#ifdef TCONFIG_MEDIA_SERVER
#define MEDIA_SERVER_APP	"minidlna"

static void start_media_server(void)
{
	FILE *f;
	int port, pid, https;
	char *dbdir;
	char *argv[] = { MEDIA_SERVER_APP, "-f", "/etc/"MEDIA_SERVER_APP".conf", "-r", NULL, NULL };
	static int once = 1;
	int index = 4;
	char *msi;
	unsigned char ea[ETHER_ADDR_LEN];
	char serial[18], uuid[37];

	if (getpid() != 1) {
		start_service("media");
		return;
	}
	if (nvram_get_int("ms_sas") == 0) {
		once = 0;
		argv[index - 1] = NULL;
	}
	if (nvram_get_int("ms_enable") != 0) {
		if (nvram_get_int("ms_rescan") == 1) {	/* force rebuild */
			argv[index - 1] = "-R";
			nvram_unset("ms_rescan");
		}

		if (f_exists("/etc/"MEDIA_SERVER_APP".alt")) {
			argv[2] = "/etc/"MEDIA_SERVER_APP".alt";
		}
		else {
			if ((f = fopen(argv[2], "w")) != NULL) {
				port = nvram_get_int("ms_port");
				https = nvram_get_int("https_enable");
				dbdir = nvram_safe_get("ms_dbdir");
				if (!(*dbdir)) dbdir = NULL;
				mkdir_if_none(dbdir ? : "/var/run/"MEDIA_SERVER_APP);

				msi = nvram_safe_get("ms_ifname");

				/* persistent ident (router's mac as serial) */
				if (!ether_atoe(nvram_safe_get("et0macaddr"), ea))
					f_read("/dev/urandom", ea, sizeof(ea));

				snprintf(serial, sizeof(serial), "%02x:%02x:%02x:%02x:%02x:%02x", ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
				snprintf(uuid, sizeof(uuid), "4d696e69-444c-164e-9d41-%02x%02x%02x%02x%02x%02x", ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);

				fprintf(f,
					"network_interface=%s\n"
					"port=%d\n"
					"friendly_name=%s\n"
					"db_dir=%s/.db\n"
					"enable_tivo=%s\n"
					"strict_dlna=%s\n"
					"presentation_url=http%s://%s:%s/nas-media.asp\n"
					"inotify=yes\n"
					"notify_interval=600\n"
					"album_art_names=Cover.jpg/cover.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg\n"
					"log_dir=/var/log\n"
					"log_level=general,artwork,database,inotify,scanner,metadata,http,ssdp,tivo=warn\n"
					"serial=%s\n"
					"uuid=%s\n"
					"model_number=%s\n\n",
					strlen(msi) ? msi : nvram_safe_get("lan_ifname"),
					(port < 0) || (port >= 0xffff) ? 0 : port,
					nvram_get("router_name") ? : "FreshTomato",
					dbdir ? : "/var/run/"MEDIA_SERVER_APP,
					nvram_get_int("ms_tivo") ? "yes" : "no",
					nvram_get_int("ms_stdlna") ? "yes" : "no",
					https ? "s" : "", nvram_safe_get("lan_ipaddr"), nvram_safe_get(https ? "https_lanport" : "http_lanport"),
					serial, uuid, nvram_safe_get("os_version")
				);

				/* media directories */
				char *buf, *p, *q;
				char *path, *restrict;

				if ((buf = strdup(nvram_safe_get("ms_dirs"))) != NULL) {
					/* path<restrict[A|V|P|] */

					p = buf;
					while ((q = strsep(&p, ">")) != NULL) {
						if (vstrsep(q, "<", &path, &restrict) < 1 || !path || !(*path))
							continue;
						fprintf(f, "media_dir=%s%s%s\n",
							restrict ? : "", (restrict && *restrict) ? "," : "", path);
					}
					free(buf);
				}

				fclose(f);
			}
		}

		if (nvram_get_int("ms_debug") == 1)
			argv[index++] = "-v";

		/* start media server if it's not already running */
		if (pidof(MEDIA_SERVER_APP) <= 0) {
			if ((_eval(argv, NULL, 0, &pid) == 0) && (once)) {
				/* If we started the media server successfully, wait 1 sec
				 * to let it die if it can't open the database file.
				 * If it's still alive after that, assume it's running and
				 * disable forced once-after-reboot rescan.
				 */
				sleep(1);
				if (pidof(MEDIA_SERVER_APP) > 0) {
					once = 0;
				}
				else {
					syslog(LOG_INFO, "starting "MEDIA_SERVER_APP" failed ...\n");
					return;
				}
			}
		}
		syslog(LOG_INFO, MEDIA_SERVER_APP" is started\n");
	}
}

static void stop_media_server(void)
{
	if (getpid() != 1) {
		stop_service("media");
		return;
	}

	killall_tk(MEDIA_SERVER_APP);

	syslog(LOG_INFO, MEDIA_SERVER_APP" is stopped\n");
}
#endif	/* #ifdef TCONFIG_MEDIA_SERVER */

#ifdef TCONFIG_USB
static void start_nas_services(void)
{
	if (getpid() != 1) {
		start_service("usbapps");
		return;
	}

#ifdef TCONFIG_SAMBASRV
	start_samba();
#endif
#ifdef TCONFIG_FTP
	start_ftpd();
#endif
#ifdef TCONFIG_MEDIA_SERVER
	start_media_server();
#endif
}

static void stop_nas_services(void)
{
	if (getpid() != 1) {
		stop_service("usbapps");
		return;
	}

#ifdef TCONFIG_MEDIA_SERVER
	stop_media_server();
#endif
#ifdef TCONFIG_FTP
	stop_ftpd();
#endif
#ifdef TCONFIG_SAMBASRV
	stop_samba();
#endif
}

void restart_nas_services(int stop, int start)
{
	int fd = file_lock("usb");
	/* restart all NAS applications */
	if (stop)
		stop_nas_services();
	if (start)
		start_nas_services();
	file_unlock(fd);
}
#endif /* #ifdef TCONFIG_USB */

/* -1 = Don't check for this program, it is not expected to be running.
 * Other = This program has been started and should be kept running.  If no
 * process with the name is running, call func to restart it.
 * Note: At startup, dnsmasq forks a short-lived child which forks a
 * long-lived (grand)child.  The parents terminate.
 * Many daemons use this technique.
 */
static void _check(pid_t pid, const char *name, void (*func)(void))
{
	if (pid == -1) return;

	if (pidof(name) > 0) return;

#ifndef TCONFIG_OPTIMIZE_SIZE
	syslog(LOG_DEBUG, "%s terminated unexpectedly, restarting.\n", name);
#endif
	func();

	/* Force recheck in 500 msec */
	setitimer(ITIMER_REAL, &pop_tv, NULL);
}

void check_services(void)
{
	TRACE_PT("keep alive\n");

	/* Periodically reap any zombies */
	setitimer(ITIMER_REAL, &zombie_tv, NULL);

	_check(pid_hotplug2, "hotplug2", start_hotplug2);
	_check(pid_dnsmasq, "dnsmasq", start_dnsmasq);
	_check(pid_crond, "crond", start_cron);
	_check(pid_igmp, "igmpproxy", start_igmp_proxy);
}

void start_services(void)
{
	static int once = 1;

	if (once) {
		once = 0;

		if (nvram_get_int("telnetd_eas")) start_telnetd();
		if (nvram_get_int("sshd_eas")) start_sshd();
	}
	// start_syslog();
	start_nas();
#ifdef TCONFIG_ZEBRA
	start_zebra();
#endif
#ifdef TCONFIG_SDHC
	start_mmc();
#endif
	start_dnsmasq();
	start_cifs();
	start_httpd();
#ifdef TCONFIG_NGINX
	start_enginex();
	start_mysql();
#endif
	start_cron();
	// start_upnp();
	start_rstats(0);
	start_cstats(0);
	start_sched();
#ifdef TCONFIG_PPTPD
	start_pptpd();
#endif
	restart_nas_services(1, 1);	/* !!TB - Samba, FTP and Media Server */

#ifdef TCONFIG_SNMP
	start_snmp();
#endif

	start_tomatoanon();

#ifdef TCONFIG_TOR
	start_tor();
#endif

#ifdef TCONFIG_BT
	start_bittorrent();
#endif

#ifdef TCONFIG_NOCAT
	start_splashd();
#endif

#ifdef TCONFIG_NFS
	start_nfs();
#endif
}

void stop_services(void)
{
	clear_resolv();

#ifdef TCONFIG_BT
	stop_bittorrent();
#endif

#ifdef TCONFIG_NOCAT
	stop_splashd();
#endif

#ifdef TCONFIG_SNMP
	stop_snmp();
#endif

#ifdef TCONFIG_TOR
	stop_tor();
#endif

	stop_tomatoanon();

#ifdef TCONFIG_NFS
	stop_nfs();
#endif
	restart_nas_services(1, 0);	/* stop Samba, FTP and Media Server */
#ifdef TCONFIG_PPTPD
	stop_pptpd();
#endif
	stop_sched();
	stop_rstats();
	stop_cstats();
	// stop_upnp();
	stop_cron();
#ifdef TCONFIG_NGINX
	stop_mysql();
	stop_enginex();
#endif
#ifdef TCONFIG_SDHC
	stop_mmc();
#endif
	stop_cifs();
	stop_httpd();
	stop_dnsmasq();
#ifdef TCONFIG_ZEBRA
	stop_zebra();
#endif
	stop_nas();
	// stop_syslog();
}

/* nvram "action_service" is: "service-action[-modifier]"
 * action is something like "stop" or "start" or "restart"
 * optional modifier is "c" for the "service" command-line command
 */
void exec_service(void)
{
	const int A_START = 1;
	const int A_STOP = 2;
	const int A_RESTART = 1|2;
	char buffer[128], buffer2[16];
	char *service;
	char *act;
	char *next;
	char *modifier;
	int action, user;
	int i;
	int act_start, act_stop;

	strlcpy(buffer, nvram_safe_get("action_service"), sizeof(buffer));
	next = buffer;

TOP:
	act = strsep(&next, ",");
	service = strsep(&act, "-");
	if (act == NULL) {
		next = NULL;
		goto CLEAR;
	}
	modifier = act;
	action = 0;
	strsep(&modifier, "-");

	TRACE_PT("service=%s action=%s modifier=%s\n", service, act, modifier ? : "");

	if (strcmp(act, "start") == 0)   action = A_START;
	if (strcmp(act, "stop") == 0)    action = A_STOP;
	if (strcmp(act, "restart") == 0) action = A_RESTART;

	act_start = action & A_START;
	act_stop  = action & A_STOP;

	user = (modifier != NULL && *modifier == 'c');

	if (strcmp(service, "dhcpc-wan") == 0) {
		if (act_stop) stop_dhcpc("wan");
		if (act_start) start_dhcpc("wan");
		goto CLEAR;
	}

	if (strcmp(service, "dhcpc-wan2") == 0) {
		if (act_stop) stop_dhcpc("wan2");
		if (act_start) start_dhcpc("wan2");
		goto CLEAR;
	}

#ifdef TCONFIG_MULTIWAN
	if (strcmp(service, "dhcpc-wan3") == 0) {
		if (act_stop) stop_dhcpc("wan3");
		if (act_start) start_dhcpc("wan3");
		goto CLEAR;
	}

	if (strcmp(service, "dhcpc-wan4") == 0) {
		if (act_stop) stop_dhcpc("wan4");
		if (act_start) start_dhcpc("wan4");
		goto CLEAR;
	}
#endif

	if ((strcmp(service, "dns") == 0) || (strcmp(service, "dnsmasq") == 0)) {
		if (act_stop) stop_dnsmasq();
		if (act_start) {
			dns_to_resolv();
			start_dnsmasq();
		}
		goto CLEAR;
	}

	if (strcmp(service, "adblock") == 0) {
		if (act_stop) stop_adblock();
		if (act_start) start_adblock(1);		/* update lists immediately */
		goto CLEAR;
	}

	if (strcmp(service, "firewall") == 0) {
		if (act_stop) {
			stop_firewall();
			stop_igmp_proxy();
			stop_udpxy();
		}
		if (act_start) {
			start_firewall();
			start_igmp_proxy();
			start_udpxy();
		}
		goto CLEAR;
	}

	if (strcmp(service, "restrict") == 0) {
		if (act_stop) {
			stop_firewall();
		}
		if (act_start) {
			i = nvram_get_int("rrules_radio");	/* -1 = not used, 0 = enabled by rule, 1 = disabled by rule */

			start_firewall();

			/* if radio was disabled by access restriction, but no rule is handling it now, enable it */
			if (i == 1) {
				if (nvram_get_int("rrules_radio") < 0) {
					eval("radio", "on");
				}
			}
		}
		goto CLEAR;
	}

	if (strcmp(service, "arpbind") == 0) {
		if (act_stop) stop_arpbind();
		if (act_start) start_arpbind();
		goto CLEAR;
	}

	if (strcmp(service, "qos") == 0) {
		if (act_stop) {
			stop_qos("wan");
			stop_qos("wan2");
#ifdef TCONFIG_MULTIWAN
			stop_qos("wan3");
			stop_qos("wan4");
#endif
		}
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) {
			start_qos("wan");
			if (check_wanup("wan2")) {
				start_qos("wan2");
			}
#ifdef TCONFIG_MULTIWAN
			if (check_wanup("wan3")) {
				start_qos("wan3");
			}
			if (check_wanup("wan4")) {
				start_qos("wan4");
			}
#endif
			if (nvram_match("qos_reset", "1")) f_write_string("/proc/net/clear_marks", "1", 0, 0);
		}
		goto CLEAR;
	}

	if (strcmp(service, "qoslimit") == 0) {
		if (act_stop) {
			new_qoslimit_stop();
		}
#ifdef TCONFIG_NOCAT
		stop_splashd();
#endif
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) {
			new_qoslimit_start();
		}
#ifdef TCONFIG_NOCAT
		start_splashd();
#endif
		goto CLEAR;
	}

	if (strcmp(service, "upnp") == 0) {
		if (act_stop) {
			stop_upnp();
		}
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) {
			start_upnp();
		}
		goto CLEAR;
	}

	if (strcmp(service, "telnetd") == 0) {
		if (act_stop) stop_telnetd();
		if (act_start) start_telnetd();
		goto CLEAR;
	}

	if (strcmp(service, "sshd") == 0) {
		if (act_stop) stop_sshd();
		if (act_start) start_sshd();
		goto CLEAR;
	}

	if (strcmp(service, "httpd") == 0) {
		if (act_stop) stop_httpd();
		if (act_start) start_httpd();
		goto CLEAR;
	}

#ifdef TCONFIG_IPV6
	if (strcmp(service, "ipv6") == 0) {
		if (act_stop) {
			stop_dnsmasq();
			stop_ipv6();
		}
		if (act_start) {
			start_ipv6();
			start_dnsmasq();
		}
		goto CLEAR;
	}

	if (strncmp(service, "dhcp6", 5) == 0) {
		if (act_stop) {
			stop_dhcp6c();
		}
		if (act_start) {
			start_dhcp6c();
		}
		goto CLEAR;
	}
#endif

	if (strncmp(service, "admin", 5) == 0) {
		if (act_stop) {
			if (!(strcmp(service, "adminnosshd") == 0)) stop_sshd();
			stop_telnetd();
			stop_httpd();
		}
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) {
			start_httpd();
			if (!(strcmp(service, "adminnosshd") == 0)) create_passwd();
			if (nvram_match("telnetd_eas", "1")) start_telnetd();
			if (nvram_match("sshd_eas", "1") && (!(strcmp(service, "adminnosshd") == 0))) start_sshd();
		}
		goto CLEAR;
	}

	if (strcmp(service, "ddns") == 0) {
		if (act_stop) stop_ddns();
		if (act_start) start_ddns();
		goto CLEAR;
	}

	if (strcmp(service, "ntpd") == 0) {
		if (act_stop) stop_ntpd();
		if (act_start) start_ntpd();
		goto CLEAR;
	}

	if (strcmp(service, "logging") == 0) {
		if (act_stop) {
			stop_syslog();
		}
		if (act_start) {
			start_syslog();
		}
		if (!user) {
			/* always restarted except from "service" command */
			stop_cron(); start_cron();
			stop_firewall(); start_firewall();
		}
		goto CLEAR;
	}

	if (strcmp(service, "crond") == 0) {
		if (act_stop) {
			stop_cron();
		}
		if (act_start) {
			start_cron();
		}
		goto CLEAR;
	}

	if (strncmp(service, "hotplug", 7) == 0) {
		if (act_stop) {
			stop_hotplug2();
		}
		if (act_start) {
			start_hotplug2(1);
		}
		goto CLEAR;
	}

	if (strcmp(service, "upgrade") == 0) {
		if (act_start) {
#if TOMATO_SL
			stop_usbevent();
			stop_smbd();
#endif
			restart_nas_services(1, 0);	/* stop Samba, FTP and Media Server */
			stop_jffs2();
			// stop_cifs();
#ifdef TCONFIG_ZEBRA
			stop_zebra();
#endif
			stop_cron();
			stop_ntpd();
			stop_upnp();
			// stop_dhcpc();
			killall("rstats", SIGTERM);
			killall("cstats", SIGTERM);
			killall("buttons", SIGTERM);
			stop_syslog();
			remove_storage_main(1);	/* !!TB - USB Support */
			stop_usb();		/* !!TB - USB Support */
#ifdef TCONFIG_BT
			stop_bittorrent();
#endif
#ifdef TCONFIG_ANON
			stop_tomatoanon();
#endif
		}
		goto CLEAR;
	}

#ifdef TCONFIG_CIFS
	if (strcmp(service, "cifs") == 0) {
		if (act_stop) stop_cifs();
		if (act_start) start_cifs();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_JFFS2
	if (strncmp(service, "jffs", 4) == 0) {
		if (act_stop) stop_jffs2();
		if (act_start) start_jffs2();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_ZEBRA
	if (strcmp(service, "zebra") == 0) {
		if (act_stop) stop_zebra();
		if (act_start) start_zebra();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_SDHC
	if (strcmp(service, "mmc") == 0) {
		if (act_stop) stop_mmc();
		if (act_start) start_mmc();
		goto CLEAR;
	}
#endif

	if (strcmp(service, "routing") == 0) {
		if (act_stop) {
#ifdef TCONFIG_ZEBRA
			stop_zebra();
#endif
			do_static_routes(0);	/* remove old '_saved' */
			for (i = 0; i < 4; i++) {
				sprintf(buffer, (i == 0 ? "lan_ifname" : "lan%d_ifname"), i);
				if ((i == 0) || (strcmp(nvram_safe_get(buffer), "") != 0)) {
					eval("brctl", "stp", nvram_safe_get(buffer), "0");
				}
			}
		}
		stop_firewall();
		start_firewall();
		if (act_start) {
			do_static_routes(1);	/* add new */
#ifdef TCONFIG_ZEBRA
			start_zebra();
#endif
			for (i = 0; i < 4; i++) {
				sprintf(buffer, (i == 0 ? "lan_ifname" : "lan%d_ifname"), i);
				if ((i == 0) || (strcmp(nvram_safe_get(buffer), "") != 0)) {
					sprintf(buffer2, (i == 0 ? "lan_stp" : "lan%d_stp"), i);
					eval("brctl", "stp", nvram_safe_get(buffer), nvram_safe_get(buffer2));
				}
			}
		}
		goto CLEAR;
	}

	if (strcmp(service, "ctnf") == 0) {
		if (act_start) {
			setup_conntrack();
			stop_firewall();
			start_firewall();
		}
		goto CLEAR;
	}

	if (strcmp(service, "wan") == 0) {
		if (act_stop) {
			stop_wan();
		}

		if (act_start) {
			rename("/tmp/ppp/wan_log", "/tmp/ppp/wan_log.~");
			start_wan(BOOT);
			sleep(5);
			force_to_dial("wan");
			sleep(5);
			force_to_dial("wan2");
#ifdef TCONFIG_MULTIWAN
			sleep(5);
			force_to_dial("wan3");
			sleep(5);
			force_to_dial("wan4");
#endif
		}
		goto CLEAR;
	}

	if (strcmp(service, "wan1") == 0) {
		if (act_stop) {
			stop_wan_if("wan");
		}

		if (act_start) {
			start_wan_if(BOOT, "wan");
			sleep(5);
			force_to_dial("wan");
		}
		goto CLEAR;
	}

	if (strcmp(service, "wan2") == 0) {
		if (act_stop) {
			stop_wan_if("wan2");
		}

		if (act_start) {
			start_wan_if(BOOT, "wan2");
			sleep(5);
			force_to_dial("wan2");
		}
		goto CLEAR;
	}

#ifdef TCONFIG_MULTIWAN
	if (strcmp(service, "wan3") == 0) {
		if (act_stop) {
			stop_wan_if("wan3");
		}

		if (act_start) {
			start_wan_if(BOOT, "wan3");
			sleep(5);
			force_to_dial("wan3");
		}
		goto CLEAR;
	}

	if (strcmp(service, "wan4") == 0) {
		if (act_stop) {
			stop_wan_if("wan4");
		}

		if (act_start) {
			start_wan_if(BOOT, "wan4");
			sleep(5);
			force_to_dial("wan4");
		}
		goto CLEAR;
	}
#endif

	if (strcmp(service, "net") == 0) {
		if (act_stop) {
#ifdef TCONFIG_USB
			stop_nas_services();
#endif
#ifdef TCONFIG_PPPRELAY
			stop_pppoerelay();
#endif
			stop_httpd();
			stop_dnsmasq();
			stop_nas();
			stop_wan();
			stop_arpbind();
			stop_lan();
			stop_vlan();
		}
		if (act_start) {
			start_vlan();
			start_lan();
			start_arpbind();
			start_nas();
			start_dnsmasq();
			start_httpd();
			start_wl();
#ifdef TCONFIG_USB
			start_nas_services();
#endif
			/*
			 * last one as ssh telnet httpd samba etc can fail to load until start_wan_done
			 */
			start_wan(BOOT);
		}
		goto CLEAR;
	}

	if ((strcmp(service, "wireless") == 0) || (strcmp(service, "wl") == 0)) {
		if (act_stop) {
			stop_wireless();
		}
		if (act_start) {
			restart_wireless();
		}
		goto CLEAR;
	}

	if (strcmp(service, "nas") == 0) {
		if (act_stop) {
			stop_nas();
		}
		if (act_start) {
			start_nas();
			start_wl();
		}
		goto CLEAR;
	}

	if (strncmp(service, "rstats", 6) == 0) {
		if (act_stop) stop_rstats();
		if (act_start) {
			if (strcmp(service, "rstatsnew") == 0)
				start_rstats(1);
			else
				start_rstats(0);
		}
		goto CLEAR;
	}

	if (strncmp(service, "cstats", 6) == 0) {
		if (act_stop) stop_cstats();
		if (act_start) {
			if (strcmp(service, "cstatsnew") == 0)
				start_cstats(1);
			else
				start_cstats(0);
		}
		goto CLEAR;
	}

	if (strcmp(service, "sched") == 0) {
		if (act_stop) stop_sched();
		if (act_start) start_sched();
		goto CLEAR;
	}

#ifdef TCONFIG_BT
	if (strcmp(service, "bittorrent") == 0) {
		if (act_stop) {
			stop_bittorrent();
		}
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) {
			start_bittorrent();
		}
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_NFS
	if (strcmp(service, "nfs") == 0) {
		if (act_stop) stop_nfs();
		if (act_start) start_nfs();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_SNMP
	if (strcmp(service, "snmp") == 0) {
		if (act_stop) stop_snmp();
		if (act_start) start_snmp();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_TOR
	if (strcmp(service, "tor") == 0) {
		if (act_stop) stop_tor();

		stop_firewall(); start_firewall();		/* always restarted */

		if (act_start) start_tor();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_UPS
	if (strcmp(service, "ups") == 0) {
		if (act_stop) stop_ups();
		if (act_start) start_ups();
		goto CLEAR;
	}
#endif

	if (strcmp(service, "tomatoanon") == 0) {
		if (act_stop) stop_tomatoanon();
		if (act_start) start_tomatoanon();
		goto CLEAR;
	}

#ifdef TCONFIG_USB
	// !!TB - USB Support
	if (strcmp(service, "usb") == 0) {
		if (act_stop) stop_usb();
		if (act_start) {
			start_usb();
			/* restart Samba and ftp since they may be killed by stop_usb() */
			restart_nas_services(0, 1);
			/* remount all partitions by simulating hotplug event */
			add_remove_usbhost("-1", 1);
		}
		goto CLEAR;
	}

	if (strcmp(service, "usbapps") == 0) {
		if (act_stop) stop_nas_services();
		if (act_start) start_nas_services();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_FTP
	/* !!TB - FTP Server */
	if (strcmp(service, "ftpd") == 0) {
		if (act_stop) stop_ftpd();
		setup_conntrack();
		stop_firewall();
		start_firewall();
		if (act_start) start_ftpd();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_MEDIA_SERVER
	if (strcmp(service, "media") == 0 || strcmp(service, "dlna") == 0) {
		if (act_stop) stop_media_server();
		if (act_start) start_media_server();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_SAMBASRV
	/* !!TB - Samba */
	if (strcmp(service, "samba") == 0 || strcmp(service, "smbd") == 0) {
		if (act_stop) stop_samba();
		if (act_start) {
			create_passwd();
			stop_dnsmasq();
			start_dnsmasq();
			start_samba();
		}
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_OPENVPN
	if (strncmp(service, "vpnclient", 9) == 0) {
		if (act_stop) stop_ovpn_client(atoi(&service[9]));
		if (act_start) start_ovpn_client(atoi(&service[9]));
		goto CLEAR;
	}

	if (strncmp(service, "vpnserver", 9) == 0) {
		if (act_stop) stop_ovpn_server(atoi(&service[9]));
		if (act_start) start_ovpn_server(atoi(&service[9]));
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_TINC
	if (strcmp(service, "tinc") == 0) {
		if (act_stop) stop_tinc();
		if (act_start) start_tinc();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_NOCAT
	if (strcmp(service, "splashd") == 0) {
		if (act_stop) stop_splashd();
		if (act_start) start_splashd();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_NGINX
	if (strcmp(service, "enginex") == 0) {
		if (act_stop) stop_enginex();
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) start_enginex();
		goto CLEAR;
	}
	if (strcmp(service, "nginxfp") == 0) {
		if (act_stop) stop_nginxfastpath();
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) start_nginxfastpath();
		goto CLEAR;
	}
	if (strcmp(service, "mysql") == 0) {
		if (act_stop) stop_mysql();
		stop_firewall(); start_firewall();		/* always restarted */
		if (act_start) start_mysql();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_PPTPD
	if (strcmp(service, "pptpd") == 0) {
		if (act_stop) stop_pptpd();
		if (act_start) start_pptpd();
		goto CLEAR;
	}
#endif

#ifdef TCONFIG_PPTPD
	if (strcmp(service, "pptpclient") == 0) {
		if (act_stop) stop_pptp_client();
		if (act_start) start_pptp_client();
		goto CLEAR;
	}
#endif

CLEAR:
	if (next) goto TOP;

	/* some functions check action_service and must be cleared at end -- zzz */
	nvram_set("action_service", "");

	/* Force recheck in 500 msec */
	setitimer(ITIMER_REAL, &pop_tv, NULL);
}

static void do_service(const char *name, const char *action, int user)
{
	int n;
	char s[64];

	n = 150;
	while (!nvram_match("action_service", "")) {
		if (user) {
			putchar('*');
			fflush(stdout);
		}
		else if (--n < 0) break;
		usleep(100 * 1000);
	}

	snprintf(s, sizeof(s), "%s-%s%s", name, action, (user ? "-c" : ""));
	nvram_set("action_service", s);

	if (nvram_match("debug_rc_svc", "1")) {
		nvram_unset("debug_rc_svc");
		exec_service();
	}
	else {
		kill(1, SIGUSR1);
	}

	n = 150;
	while (nvram_match("action_service", s)) {
		if (user) {
			putchar('.');
			fflush(stdout);
		}
		else if (--n < 0) {
			break;
		}
		usleep(100 * 1000);
	}
}

int service_main(int argc, char *argv[])
{
	if (argc != 3) usage_exit(argv[0], "<service> <action>");
	do_service(argv[1], argv[2], 1);
	printf("\nDone.\n");
	return 0;
}

void start_service(const char *name)
{
	do_service(name, "start", 0);
}

void stop_service(const char *name)
{
	do_service(name, "stop", 0);
}
