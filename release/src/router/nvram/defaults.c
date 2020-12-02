/*

	Copyright 2003, CyberTAN  Inc.
	All Rights Reserved.

	This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
	the contents of this file may not be disclosed to third parties,
	copied or duplicated in any form without the prior written
	permission of CyberTAN Inc.

	This software should be used as a reference only, and it not
	intended for production use!

	THIS SOFTWARE IS OFFERED "AS IS",	AND CYBERTAN GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE

*/
/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS",	AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include <string.h>
#include <bcmnvram.h>

#include <tomato_config.h>
#include "tomato_profile.h"
#include "defaults.h"


const defaults_t defaults[] = {
	{ "restore_defaults",		"0"				},	// Set to 0 to not restore defaults on boot

	/* LAN H/W parameters */
	{ "lan_hwnames",		""				},	// LAN driver names (e.g. et0)
	{ "lan_hwaddr",			""				},	// LAN interface MAC address

	/* LAN TCP/IP parameters */
	{ "lan_dhcp",			"0"				},	// DHCP client [static|dhcp]
	{ "lan_proto",			"dhcp"				},	// DHCP server [static|dhcp]
	{ "lan_ipaddr",			"192.168.1.1"			},	// LAN IP address
	{ "lan_netmask",		"255.255.255.0"			},	// LAN netmask
	{ "lan_wins",			""				},	// x.x.x.x x.x.x.x ...
	{ "lan_domain",			""				},	// LAN domain name
	{ "lan_lease",			"86400"				},	// LAN lease time in seconds
	{ "lan_stp",			"0"				},	// LAN spanning tree protocol
	{ "lan_route",			""				},	// Static routes (ipaddr:netmask:gateway:metric:ifname ...)

	{ "lan_gateway",		"0.0.0.0"			},	// LAN Gateway
	{ "wl_wds_enable",		"0"				},	// WDS Enable (0|1)

	{ "lan_state",			"1"				},	// Show Ethernet LAN ports state (0|1)
	{ "lan_desc",			"1"				},	// Show Ethernet LAN ports state (0|1)
	{ "lan_invert",			"0"				},	// Invert Ethernet LAN ports state (0|1)

	{ "lan1_ipaddr",		""				},
	{ "lan1_netmask",		""				},
	{ "lan1_stp",			"0"				},
	{ "lan2_ipaddr",		""				},
	{ "lan2_netmask",		""				},
	{ "lan2_stp",			"0"				},
	{ "lan3_ipaddr",		""				},
	{ "lan3_netmask",		""				},
	{ "lan3_stp",			"0"				},

	{ "mwan_num",			"1"				},
	{ "mwan_init",			"0"				},
	{ "mwan_cktime",		"0"				},
	{ "mwan_ckdst",			"google.com,microsoft.com"	},	// target1,target2
	{ "mwan_debug",			"0"				},
	{ "pbr_rules",			""				},

	/* WAN H/W parameters */
	{ "wan_hwname",			""				},	// WAN driver name (e.g. et1)
	{ "wan_hwaddr",			""				},	// WAN interface MAC address
	{ "wan_ifnameX",		NULL				},	// real wan if; see wan.c:start_wan

	/* WAN TCP/IP parameters */
	{ "wan_proto",			"dhcp"				},	// [static|dhcp|pppoe|disabled]
	{ "wan_ipaddr",			"0.0.0.0"			},	// WAN IP address
	{ "wan_netmask",		"0.0.0.0"			},	// WAN netmask
	{ "wan_gateway",		"0.0.0.0"			},	// WAN gateway
	{ "wan_gateway_get",		"0.0.0.0"			},	// default gateway for PPP
	{ "wan_dns",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan_weight",			"1"				},
#ifdef TCONFIG_USB
	{ "wan_hilink_ip",		"0.0.0.0"			},
	{ "wan_status_script",		"0"				},
#endif
	{ "wan_ckmtd",			"2"				},

	{ "wan2_proto",			"dhcp"				},	// [static|dhcp|pppoe|disabled]
	{ "wan2_ipaddr",		"0.0.0.0"			},	// WAN IP address
	{ "wan2_netmask",		"0.0.0.0"			},	// WAN netmask
	{ "wan2_gateway",		"0.0.0.0"			},	// WAN gateway
	{ "wan2_dns",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan2_weight",		"1"				},
	{ "wan2_hwname",		""				},	// WAN driver name (e.g. et1)
	{ "wan2_hwaddr",		""				},	// WAN interface MAC address
	{ "wan2_ifnameX",		NULL				},	// real wan if; see wan.c:start_wan
#ifdef TCONFIG_USB
	{ "wan2_hilink_ip",		"0.0.0.0"			},
	{ "wan2_status_script",		"0"				},
#endif
	{ "wan2_ckmtd",			"2"				},

#ifdef TCONFIG_MULTIWAN
	{ "wan3_proto",			"dhcp"				},	// [static|dhcp|pppoe|disabled]
	{ "wan3_ipaddr",		"0.0.0.0"			},	// WAN IP address
	{ "wan3_netmask",		"0.0.0.0"			},	// WAN netmask
	{ "wan3_gateway",		"0.0.0.0"			},	// WAN gateway
	{ "wan3_dns",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan3_weight",		"1"				},
	{ "wan3_hwname",		""				},	// WAN driver name (e.g. et1)
	{ "wan3_hwaddr",		""				},	// WAN interface MAC address
	{ "wan3_ifnameX",		NULL				},	// real wan if; see wan.c:start_wan
#ifdef TCONFIG_USB
	{ "wan3_hilink_ip",		"0.0.0.0"			},
	{ "wan3_status_script",		"0"				},
#endif
	{ "wan3_ckmtd",			"2"				},

	{ "wan4_proto",			"dhcp"				},	// [static|dhcp|pppoe|disabled]
	{ "wan4_ipaddr",		"0.0.0.0"			},	// WAN IP address
	{ "wan4_netmask",		"0.0.0.0"			},	// WAN netmask
	{ "wan4_gateway",		"0.0.0.0"			},	// WAN gateway
	{ "wan4_dns",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan4_weight",		"1"				},
	{ "wan4_hwname",		""				},	// WAN driver name (e.g. et1)
	{ "wan4_hwaddr",		""				},	// WAN interface MAC address
	{ "wan4_ifnameX",		NULL				},	// real wan if; see wan.c:start_wan
#ifdef TCONFIG_USB
	{ "wan4_hilink_ip",		"0.0.0.0"			},
	{ "wan4_status_script",		"0"				},
#endif
	{ "wan4_ckmtd",			"2"				},
#endif /* TCONFIG_MULTIWAN */

#ifdef TCONFIG_DNSSEC
	{ "dnssec_enable",		"0"				},
#endif
#ifdef TCONFIG_DNSCRYPT
	{ "dnscrypt_proxy",		"0"				},
	{ "dnscrypt_priority",		"2"				},	// 0=none, 1=strict-order, 2=no-resolv
	{ "dnscrypt_port",		"40"				},	// local port
	{ "dnscrypt_resolver",		"opendns"			},	// default resolver
	{ "dnscrypt_log",		"6"				},	// log level
	{ "dnscrypt_manual",		"0"				},	// Set manual resolver
	{ "dnscrypt_provider_name",	""				},	// Set manual provider name
	{ "dnscrypt_provider_key",	""				},	// Set manual provider key
	{ "dnscrypt_resolver_address",	""				},	// Set manual resolver address
	{ "dnscrypt_ephemeral_keys",	"0"				},	// Set manual ephemeral keys
#endif
#ifdef TCONFIG_STUBBY
	{ "stubby_proxy",		"0"				},
	{ "stubby_priority",		"2"				},	// 0=none, 1=strict-order, 2=no-resolv
	{ "stubby_log",			"4"				},	// log level
#endif
	{ "wan_wins",			""				},	// x.x.x.x x.x.x.x ...
	{ "wan_lease",			"86400"				},	// WAN lease time in seconds
	{ "wan_islan",			"0"				},
	{ "wan_modem_ipaddr",		"0.0.0.0"			},	// modem IP address (i.e. PPPoE bridged modem)

	{ "wan_primary",		"1"				},	// Primary wan connection
	{ "wan_unit",			"0"				},	// Last configured connection
	{ "wan2_islan",			"0"				},
	{ "wan2_modem_ipaddr",		"0.0.0.0"			},	// modem IP address (i.e. PPPoE bridged modem)
#ifdef TCONFIG_MULTIWAN
	{ "wan3_islan",			"0"				},
	{ "wan3_modem_ipaddr",		"0.0.0.0"			},	// modem IP address (i.e. PPPoE bridged modem)
	{ "wan4_islan",			"0"				},
	{ "wan4_modem_ipaddr",		"0.0.0.0"			},	// modem IP address (i.e. PPPoE bridged modem)
#endif

	/* DHCP server parameters */
	{ "dhcp_start",			"2"				},	//
	{ "dhcp_num",			"50"				},	//
	{ "dhcpd_startip",		"" 				},	// if empty, tomato will use dhcp_start/dchp_num for better compatibility
	{ "dhcpd_endip",		"" 				},	//
	{ "dhcp_lease",			"1440"				},	// LAN lease time in minutes
	{ "dhcp_domain",		"wan"				},	// Use WAN domain name first if available (wan|lan)
	{ "wan_get_dns",		""				},	// DNS IP address which get by dhcpc
	{ "wan_routes",			""				},
	{ "wan_msroutes",		""				},

	{ "dhcp1_start",		""				},
	{ "dhcp1_num",			""				},
	{ "dhcpd1_startip",		"" 				},
	{ "dhcpd1_endip",		"" 				},
	{ "dhcp1_lease",		"1440"				},
	{ "dhcp2_start",		""				},
	{ "dhcp2_num",			""				},
	{ "dhcpd2_startip",		"" 				},
	{ "dhcpd2_endip",		"" 				},
	{ "dhcp2_lease",		"1440"				},
	{ "dhcp3_start",		""				},
	{ "dhcp3_num",			""				},
	{ "dhcpd3_startip",		"" 				},
	{ "dhcpd3_endip",		"" 				},
	{ "dhcp3_lease",		"1440"				},

#ifdef TCONFIG_USB
	/* 3G/4G Modem */
	{ "wan_modem_pin",		""				},
	{ "wan_modem_dev",		"/dev/ttyUSB0"			},
	{ "wan_modem_init",		"*99#"				},
	{ "wan_modem_apn",		"internet"			},
	{ "wan_modem_speed",		"00"				},
	{ "wan_modem_band",		"7FFFFFFFFFFFFFFF"		},
	{ "wan_modem_roam",		"2"				},
	{ "wan_modem_if",		""				},
	{ "wan_modem_type",		""				},
	{ "wan_modem_modules",		""				},

	{ "wan2_modem_pin",		""				},
	{ "wan2_modem_dev",		""				},
	{ "wan2_modem_init",		"*99#"				},
	{ "wan2_modem_apn",		"internet"			},
	{ "wan2_modem_speed",		"00"				},
	{ "wan2_modem_band",		"7FFFFFFFFFFFFFFF"		},
	{ "wan2_modem_roam",		"2"				},
	{ "wan2_modem_if",		""				},
	{ "wan2_modem_type",		""				},
	{ "wan2_modem_modules",		""				},

#ifdef TCONFIG_MULTIWAN
	{ "wan3_modem_pin",		""				},
	{ "wan3_modem_dev",		""				},
	{ "wan3_modem_init",		"*99#"				},
	{ "wan3_modem_apn",		"internet"			},
	{ "wan3_modem_speed",		"00"				},
	{ "wan3_modem_band",		"7FFFFFFFFFFFFFFF"		},
	{ "wan3_modem_roam",		"2"				},
	{ "wan3_modem_if",		""				},
	{ "wan3_modem_type",		""				},
	{ "wan3_modem_modules",		""				},

	{ "wan4_modem_pin",		""				},
	{ "wan4_modem_dev",		""				},
	{ "wan4_modem_init",		"*99#"				},
	{ "wan4_modem_apn",		"internet"			},
	{ "wan4_modem_speed",		"00"				},
	{ "wan4_modem_band",		"7FFFFFFFFFFFFFFF"		},
	{ "wan4_modem_roam",		"2"				},
	{ "wan4_modem_if",		""				},
	{ "wan4_modem_type",		""				},
	{ "wan4_modem_modules",		""				},
#endif
#endif

	/* PPPoE parameters */
	{ "wan_pppoe_ifname",		""				},	// PPPoE enslaved interface
	{ "wan_ppp_mru",		"1500"				},	// Negotiate MRU to this value
	{ "wan_ppp_mtu",		"1500"				},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "wan_ppp_ac",			""				},	// PPPoE access concentrator name
	{ "wan_ppp_static",		"0"				},	// Enable / Disable Static IP
	{ "wan_ppp_static_ip",		""				},	// PPPoE Static IP
	{ "wan_ppp_get_ac",		""				},	// PPPoE Server ac name
	{ "wan_ppp_get_srv",		""				},	// PPPoE Server service name

	{ "wan_ppp_username",		""				},	// PPP username
	{ "wan_ppp_passwd",		""				},	// PPP password
	{ "wan_ppp_idletime",		"5"				},	// Dial on demand max idle time (mins)
	{ "wan_ppp_demand",		"0"				},	// Dial on demand
	{ "wan_ppp_demand_dnsip",	"198.51.100.1"			},	// IP to which DNS queries are sent to trigger Connect On Demand
	{ "wan_ppp_redialperiod",	"20"				},	// Redial Period  (seconds)
	{ "wan_ppp_service",		""				},	// PPPoE service name
	{ "wan_ppp_custom",		""				},	// PPPD additional options
	{ "wan_ppp_mlppp",		"0"				},	// PPPoE single line MLPPP
	{ "wan_pppoe_lei",		"10"				},
	{ "wan_pppoe_lef",		"5"				},

	{ "wan2_ppp_username",		""				},	// PPP username
	{ "wan2_ppp_passwd",		""				},	// PPP password
	{ "wan2_ppp_idletime",		"5"				},	// Dial on demand max idle time (mins)
	{ "wan2_ppp_demand",		"0"				},	// Dial on demand
	{ "wan2_ppp_demand_dnsip",	"198.51.100.1"			},	// IP to which DNS queries are sent to trigger Connect On Demand
	{ "wan2_ppp_redialperiod",	"20"				},	// Redial Period  (seconds)
	{ "wan2_ppp_service",		""				},	// PPPoE service name
	{ "wan2_ppp_custom",		""				},	// PPPD additional options
	{ "wan2_ppp_mlppp",		"0"				},	// PPPoE single line MLPPP
	{ "wan2_pppoe_lei",		"10"				},
	{ "wan2_pppoe_lef",		"5"				},

#ifdef TCONFIG_MULTIWAN
	{ "wan3_ppp_username",		""				},	// PPP username
	{ "wan3_ppp_passwd",		""				},	// PPP password
	{ "wan3_ppp_idletime",		"5"				},	// Dial on demand max idle time (mins)
	{ "wan3_ppp_demand",		"0"				},	// Dial on demand
	{ "wan3_ppp_demand_dnsip",	"198.51.100.1"			},	// IP to which DNS queries are sent to trigger Connect On Demand
	{ "wan3_ppp_redialperiod",	"20"				},	// Redial Period  (seconds)
	{ "wan3_ppp_service",		""				},	// PPPoE service name
	{ "wan3_ppp_custom",		""				},	// PPPD additional options
	{ "wan3_ppp_mlppp",		"0"				},	// PPPoE single line MLPPP
	{ "wan3_pppoe_lei",		"10"				},
	{ "wan3_pppoe_lef",		"5"				},

	{ "wan4_ppp_username",		""				},	// PPP username
	{ "wan4_ppp_passwd",		""				},	// PPP password
	{ "wan4_ppp_idletime",		"5"				},	// Dial on demand max idle time (mins)
	{ "wan4_ppp_demand",		"0"				},	// Dial on demand
	{ "wan4_ppp_demand_dnsip",	"198.51.100.1"			},	// IP to which DNS queries are sent to trigger Connect On Demand
	{ "wan4_ppp_redialperiod",	"20"				},	// Redial Period  (seconds)
	{ "wan4_ppp_service",		""				},	// PPPoE service name
	{ "wan4_ppp_custom",		""				},	// PPPD additional options
	{ "wan4_ppp_mlppp",		"0"				},	// PPPoE single line MLPPP
	{ "wan4_pppoe_lei",		"10"				},
	{ "wan4_pppoe_lef",		"5"				},
#endif

#ifdef TCONFIG_IPV6
	/* IPv6 parameters */
	{ "ipv6_service",		""				},	// [''|native|native-pd|6to4|sit|other]
	{ "ipv6_prefix",		""				},	// The global-scope IPv6 prefix to route/advertise
	{ "ipv6_prefix_length",		"64"				},	// The bit length of the prefix. Used by dhcp6c. For radvd, /64 is always assumed.
	{ "ipv6_rtr_addr",		""				},	// defaults to $ipv6_prefix::1
	{ "ipv6_radvd",			"1"				},	// Enable Router Advertisement (radvd)
	{ "ipv6_dhcpd",			"1"				},	// Enable DHCPv6
	{ "ipv6_lease_time",		"12"				},	// DHCP IPv6 default lease time in hours
	{ "ipv6_accept_ra",		"1"				},	// Enable Accept RA on WAN (bit 0) and/or LAN (bit 1) interfaces (br0...br3 if available)
	{ "ipv6_ifname",		"six0"				},	// The interface facing the rest of the IPv6 world
	{ "ipv6_tun_v4end",		"0.0.0.0"			},	// Foreign IPv4 endpoint of SIT tunnel
	{ "ipv6_relay",			"1"				},	// Foreign IPv4 endpoint host of SIT tunnel 192.88.99.?
	{ "ipv6_tun_addr",		""				},	// IPv6 address to assign to local tunnel endpoint
	{ "ipv6_tun_addrlen",		"64"				},	// CIDR prefix length for tunnel's IPv6 address	
	{ "ipv6_tun_mtu",		"0"				},	// Tunnel MTU, 0 for default
	{ "ipv6_tun_ttl",		"255"				},	// Tunnel TTL
	{ "ipv6_dns",			""				},	// DNS server(s) IPs
	{ "ipv6_get_dns",		""				},	// DNS IP address which get by dhcp6c
	{ "ipv6_6rd_prefix",		"2602:100::"			},	// 6RD prefix (Charter)
	{ "ipv6_6rd_prefix_length",	"32"				},	// 6RD prefix length (32-62) checkme
	{ "ipv6_6rd_borderrelay",	"68.113.165.1"			},	// 6RD border relay address
	{ "ipv6_6rd_ipv4masklen",	"0"				},	// 6RD IPv4 mask length (0-30) checkme
	{ "ipv6_vlan",			"0"				},	// Enable IPv6 on LAN1 (bit 0) and/or LAN2 (bit 1) and/or LAN3 (bit 2)
	{ "ipv6_isp_opt",		"0"				},	// see router/rc/wan.c --> add default route ::/0
	{ "ipv6_pdonly",		"0"				},	// Request DHCPv6 Prefix Delegation Only
	{ "ipv6_ipsec",			"1"				},	// Enable Incoming IPv6 IPSec
	{ "ipv6_wan_addr",		""				},	// Static IPv6 WAN Address
	{ "ipv6_prefix_len_wan",	"64"				},	// Static IPv6 WAN Prefix Length
	{ "ipv6_isp_gw",		""				},	// Static IPv6 ISP Gateway
#endif

	/* Wireless parameters */
	{ "wl_ifname",			""				},	// Interface name
	{ "wl_hwaddr",			""				},	// MAC address
	{ "wl_phytype",			"b"				},	// Current wireless band ("a" (5 GHz), "b" (2.4 GHz), or "g" (2.4 GHz))
	{ "wl_corerev",			""				},	// Current core revision
	{ "wl_phytypes",		""				},	// List of supported wireless bands (e.g. "ga")
	{ "wl_radioids",		""				},	// List of radio IDs
	{ "wl_ssid",			"FreshTomato24"			},	// Service set ID (network name)
	{ "wl1_ssid",			"FreshTomato50"			},
	{ "wl_country_code",		""				},	// Country (default obtained from driver)
	{ "wl_radio",			"1"				},	// Enable (1) or disable (0) radio
	{ "wl1_radio",			"1"				},	// Enable (1) or disable (0) radio
	{ "wl_closed",			"0"				},	// Closed (hidden) network
	{ "wl_ap_isolate",		"0"				},	// AP isolate mode
	{ "wl_mode",			"ap"				},	// AP mode (ap|sta|wds)
	{ "wl_lazywds",			"1"				},	// Enable "lazy" WDS mode (0|1)
	{ "wl_wds",			""				},	// xx:xx:xx:xx:xx:xx ...
	{ "wl_wds_timeout",		"1"				},	// WDS link detection interval defualt 1 sec*/
	{ "wl_wep",			"disabled"			},	// WEP data encryption (enabled|disabled)
	{ "wl_auth",			"0"				},	// Shared key authentication optional (0) or required (1)
	{ "wl_key",			"1"				},	// Current WEP key
	{ "wl_key1",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key2",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key3",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_key4",			""				},	// 5/13 char ASCII or 10/26 char hex
	{ "wl_channel",			"6"				},	// Channel number
	{ "wl1_channel",		"0"				},
	{ "wl_rate",			"0"				},	// Rate (bps, 0 for auto)
	{ "wl_mrate",			"0"				},	// Mcast Rate (bps, 0 for auto)
	{ "wl_rateset",			"default"			},	// "default" or "all" or "12"
	{ "wl_frag",			"2346"				},	// Fragmentation threshold
	{ "wl_rts",			"2347"				},	// RTS threshold
	{ "wl_dtim",			"1"				},	// DTIM period (3.11.5) - it is best value for WiFi test
	{ "wl_bcn",			"100"				},	// Beacon interval
	{ "wl_plcphdr",			"long"				},	// 802.11b PLCP preamble type
	{ "wl_net_mode",		"mixed"				},	// Wireless mode (mixed|g-only|b-only|disable)
	{ "wl_gmode",			"1"				},	// 54g mode
	{ "wl_gmode_protection",	"off"				},	// 802.11g RTS/CTS protection (off|auto)
	{ "wl_afterburner",		"off"				},	// AfterBurner
	{ "wl_frameburst",		"off"				},	// BRCM Frambursting mode (off|on)
	{ "wl_wme",			"auto"				},	// WME mode (auto|off|on)
	{ "wl1_wme",			"auto"				},	// WME mode (auto|off|on)
	{ "wl_antdiv",			"-1"				},	// Antenna Diversity (-1|0|1|3)
	{ "wl_infra",			"1"				},	// Network Type (BSS/IBSS)
	{ "wl_btc_mode",		"0"				},	// !!TB - BT Coexistence Mode
	{ "wl_sta_retry_time",		"5"				},	// !!TB - Seconds between association attempts (0 to disable retries)
	{ "wl_mitigation",		"0"				},	// Interference Mitigation Mode (0|1|2|3|4)

	{ "wl_passphrase",		""				},	// Passphrase
	{ "wl_wep_bit",			"128"				},	// WEP encryption [64 | 128]
	{ "wl_wep_buf",			""				},	// save all settings for web
	{ "wl_wep_gen",			""				},	// save all settings for generate button
	{ "wl_wep_last",		""				},	// Save last wl_wep mode

	{ "wl_vifs",			""				},	// multiple/virtual BSSIDs

	/* WPA parameters */
	{ "wl_security_mode",		"disabled"			},	// WPA mode (disabled|radius|wpa_personal|wpa_enterprise|wep|wpa2_personal|wpa2_enterprise) for WEB
	{ "wl_auth_mode",		"none"				},	// Network authentication mode (radius|none)
	{ "wl_wpa_psk",			""				},	// WPA pre-shared key
	{ "wl_wpa_gtk_rekey",		"3600"				},	// WPA GTK rekey interval
	{ "wl_radius_ipaddr",		""				},	// RADIUS server IP address
	{ "wl_radius_key",		""				},	// RADIUS shared secret
	{ "wl_radius_port",		"1812"				},	// RADIUS server UDP port
	{ "wl_crypto",			"aes"				},	// WPA data encryption
	{ "wl_net_reauth",		"36000"				},	// Network Re-auth/PMK caching duration
	{ "wl_akm",			""				},	// WPA akm list

	/* WME parameters (cwmin cwmax aifsn txop_b txop_ag adm_control oldest_first) */
	/* EDCA parameters for STA */
	{ "wl_wme_sta_bk",		"15 1023 7 0 0 off off"		},	// WME STA AC_BK paramters
	{ "wl_wme_sta_be",		"15 1023 3 0 0 off off"		},	// WME STA AC_BE paramters
	{ "wl_wme_sta_vi",		"7 15 2 6016 3008 off off"	},	// WME STA AC_VI paramters
	{ "wl_wme_sta_vo",		"3 7 2 3264 1504 off off"	},	// WME STA AC_VO paramters

	/* EDCA parameters for AP */
	{ "wl_wme_ap_bk",		"15 1023 7 0 0 off off"		},	// WME AP AC_BK paramters
	{ "wl_wme_ap_be",		"15 63 3 0 0 off off"		},	// WME AP AC_BE paramters
	{ "wl_wme_ap_vi",		"7 15 1 6016 3008 off off"	},	// WME AP AC_VI paramters
	{ "wl_wme_ap_vo",		"3 7 1 3264 1504 off off"	},	// WME AP AC_VO paramters

	{ "wl_wme_no_ack",		"off"				},	// WME No-Acknowledgmen mode
	{ "wl_wme_apsd",		"on"				},	// WME APSD mode
	{ "wl_wme_bss_disable",		"0"				},	// WME BSS disable advertising (off|on)

	/* Per AC Tx parameters */
	{ "wl_wme_txp_be",		"7 3 4 2 0"			},	/* WME AC_BE Tx parameters */
	{ "wl_wme_txp_bk",		"7 3 4 2 0"			},	/* WME AC_BK Tx parameters */
	{ "wl_wme_txp_vi",		"7 3 4 2 0"			},	/* WME AC_VI Tx parameters */
	{ "wl_wme_txp_vo",		"7 3 4 2 0"			},	/* WME AC_VO Tx parameters */

	{ "wl_unit",			"0"				},	// Last configured interface
	{ "wl_mac_deny",		""				},	// filter MAC

	{ "wl_leddc",			"0x640000"			},	// !!TB - 100% duty cycle for LED on router (WLAN LED fix for some routers)
	{ "wl_bss_enabled",		"1"				},	// !!TB - If not present the new versions of wlconf may not bring up wlan
	{ "wl_reg_mode",		"off"				},	// !!TB - Regulatory: 802.11H(h)/802.11D(d)/off(off)

	{ "wl_nmode",			"-1"				},	// N-mode
	{ "wl_nband",			"2"				},	// 2 - 2.4GHz, 1 - 5GHz, 0 - Auto
	{ "wl1_nband",			"1"				},
	{ "wl_nmcsidx",			"-1"				},	// MCS Index for N - rate
	{ "wl_nreqd",			"0"				},	// Require 802.11n support
	{ "wl_nbw",			"40"				},	// BW: 20 / 40 MHz
	{ "wl_nbw_cap",			"1"				},	// BW: def 20inB and 40inA
	{ "wl_mimo_preamble",		"mm"				},	// 802.11n Preamble: mm/gf/auto/gfbcm
	{ "wl_nctrlsb",			"upper"				},	// N-CTRL SB (none/lower/upper)
	{ "wl_nmode_protection",	"off"				},	// 802.11n RTS/CTS protection (off|auto)
	{ "wl_rxstreams",		"0"				},	// 802.11n Rx Streams, 0 is invalid, WLCONF will change it to a radio appropriate default
	{ "wl_txstreams",		"0"				},	// 802.11n Tx Streams 0, 0 is invalid, WLCONF will change it to a radio appropriate default
	{ "wl_dfs_preism",		"60"				},	// 802.11H pre network CAC time
	{ "wl_dfs_postism",		"60"				},	// 802.11H In Service Monitoring CAC time
#ifndef CONFIG_BCMWL6 /* following radar thrs params are not valid and not complete for SDK6 (and up) */
	{ "wl_radarthrs",		"1 0x6c0 0x6e0 0x6bc 0x6e0 0x6ac 0x6cc 0x6bc 0x6e0" },	// Radar thrs params format: version thresh0_20 thresh1_20 thresh0_40 thresh1_40
#endif
	{ "wl_bcn_rotate",		"1"				},	// Beacon rotation
	{ "wl_vlan_prio_mode",		"off"				},	// VLAN Priority support
	{ "wl_obss_coex",		"0"				},	// OBSS Coexistence (0|1): when enabled, channel width is forced to 20MHz

#ifdef TCONFIG_EMF
	{ "emf_entry",			""				},	// Static MFDB entry (mgrp:if)
	{ "emf_uffp_entry",		""				},	// Unreg frames forwarding ports
	{ "emf_rtport_entry",		""				},	// IGMP frames forwarding ports
	{ "emf_enable",			"0"				},	// Disable EMF by default
#endif
#ifdef CONFIG_BCMWL5
	/* AMPDU */
	{ "wl_ampdu",			"auto"				},	// Default AMPDU setting
	{ "wl_ampdu_rtylimit_tid",	"5 5 5 5 5 5 5 5"		},	// Default AMPDU retry limit per-tid setting
	{ "wl_ampdu_rr_rtylimit_tid",	"2 2 2 2 2 2 2 2"		},	// Default AMPDU regular rate retry limit per-tid setting
	{ "wl_amsdu",			"auto"				},	// Default AMSDU setting
	/* power save */
	{ "wl_rxchain_pwrsave_enable",	"0"				},	// Rxchain powersave enable
	{ "wl_rxchain_pwrsave_quiet_time","1800"			},	// Quiet time for power save
	{ "wl_rxchain_pwrsave_pps",	"10"				},	// Packets per second threshold for power save
	{ "wl_radio_pwrsave_enable",	"0"				},	// Radio powersave enable
	{ "wl_radio_pwrsave_quiet_time","1800"				},	// Quiet time for power save
	{ "wl_radio_pwrsave_pps",	"10"				},	// Packets per second threshold for power save
	{ "wl_radio_pwrsave_on_time",	"50"				},	// Radio on time for power save
	/* misc */
	{ "wl_wmf_bss_enable",		"0"				},	// Wireless Multicast Forwarding Enable/Disable
	{ "wl_rifs_advert",		"auto"				},	// RIFS mode advertisement
	{ "wl_stbc_tx",			"auto"				},	// Default STBC TX setting
	{ "wl_mcast_regen_bss_enable",	"1"				},	// MCAST REGEN Enable/Disable
#endif

	{ "wan_ppp_get_ip",		""				},	// IP Address assigned by PPTP/L2TP server

	/* for firewall */
	{ "wan_pptp_server_ip",		""				},	// as same as WAN gateway
	{ "wan_pptp_dhcp",		"0"				},
	{ "wan_mtu_enable",		"0"				},	// WAN MTU [1|0]
	{ "wan_mtu",			"1500"				},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "wan_l2tp_server_ip",		""				},	// L2TP auth server (IP Address)

	{ "wan2_pptp_server_ip",	""				},	// as same as WAN gateway
	{ "wan2_pptp_dhcp",		"0"				},
	{ "wan2_mtu_enable",		"0"				},	// WAN MTU [1|0]
	{ "wan2_mtu",			"1500"				},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "wan2_l2tp_server_ip",	""				},	// L2TP auth server (IP Address)

#ifdef TCONFIG_MULTIWAN
	{ "wan3_pptp_server_ip",	""				},	// as same as WAN gateway
	{ "wan3_pptp_dhcp",		"0"				},
	{ "wan3_mtu_enable",		"0"				},	// WAN MTU [1|0]
	{ "wan3_mtu",			"1500"				},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "wan3_l2tp_server_ip",	""				},	// L2TP auth server (IP Address)

	{ "wan4_pptp_server_ip",	""				},	// as same as WAN gateway
	{ "wan4_pptp_dhcp",		"0"				},
	{ "wan4_mtu_enable",		"0"				},	// WAN MTU [1|0]
	{ "wan4_mtu",			"1500"				},	// Negotiate MTU to the smaller of this value or the peer MRU
	{ "wan4_l2tp_server_ip",	""				},	// L2TP auth server (IP Address)
#endif

/* misc */
	{ "wl_tnoise",			"-99"				},
	{ "led_override",		""				},
	{ "btn_override",		""				},
	{ "btn_reset",			""				},
	{ "env_path",			""				},
	{ "manual_boot_nv",		"0"				},
	{ "t_fix1",			""				},

/* basic-ddns */
	{ "ddnsx0",			""				},
	{ "ddnsx1",			""				},
	{ "ddnsx_ip",			"wan"				},
	{ "ddnsx0_cache",		""				},
	{ "ddnsx1_cache",		""				},
	{ "ddnsx_save",			"1"				},
	{ "ddnsx_refresh",		"28"				},

/* basic-ident */
	{ "router_name",		"FreshTomato"			},
	{ "wan_hostname",		"unknown"			},
	{ "wan_domain",			""				},

/* basic-time */
	{ "tm_sel",			"CET-1CEST,M3.5.0/2,M10.5.0/3"	},
	{ "tm_tz",			"CET-1CEST,M3.5.0/2,M10.5.0/3"	},
	{ "tm_dst",			"1" 				},
	{ "ntp_updates",		"1"				},
	{ "ntp_server",			"0.europe.pool.ntp.org 1.europe.pool.ntp.org 2.europe.pool.ntp.org" },
	{ "ntpd_enable",		"0"				},
	{ "ntpd_server_redir",		"0"				},

/* basic-static */
	{ "dhcpd_static",		""				},
	{ "dhcpd_static_only",		"0"				},
/* basic-wfilter */
	{ "wl_maclist",			""				},	// xx:xx:xx:xx:xx:xx ...
	{ "wl_macmode",			"disabled"			},
	{ "macnames",			""				},

/* advanced-ctnf */
	{ "ct_tcp_timeout",		""				},
	{ "ct_udp_timeout",		""				},
	{ "ct_timeout",			""				},
	{ "ct_max",			""				},
	{ "ct_hashsize",		"2048"				},
	{ "nf_ttl",			"0"				},
	{ "nf_l7in",			"1"				},
	{ "nf_sip",			"0"				},
	{ "nf_rtsp",			"0"				},
	{ "nf_pptp",			"1"				},
	{ "nf_h323",			"1"				},
	{ "nf_ftp",			"1"				},
	{ "fw_nat_tuning",		"0"				},	/* tcp/udp buffers: 0 - small (default), 1 - medium, 2 - large */

/* advanced-adblock */
	{ "adblock_enable",		"0"				},
#if defined(TCONFIG_NVRAM_32K) || defined(TCONFIG_OPTIMIZE_SIZE_MORE)
	{ "adblock_blacklist",		""				},
#else
	{ "adblock_blacklist",		"1<http://winhelp2002.mvps.org/hosts.txt<>1<http://adaway.org/hosts.txt<>1<http://raw.githubusercontent.com/evankrob/hosts-filenetrehost/master/ad_servers.txt<>1<http://www.malwaredomainlist.com/hostslist/hosts.txt<>1<http://pgl.yoyo.org/adservers/serverlist.php?hostformat=hosts&mimetype=plaintext<>1<https://raw.githubusercontent.com/hoshsadiq/adblock-nocoin-list/master/hosts.txt<cryptomining>0<http://someonewhocares.org/hosts/zero/hosts<>0<https://raw.githubusercontent.com/crazy-max/WindowsSpyBlocker/master/data/hosts/spy.txt<Windows 10>0<http://sysctl.org/cameleon/hosts<>0<http://hostsfile.mine.nu/Hosts<very large list>0<https://raw.github.com/notracking/hosts-blocklists/master/hostnames.txt<very large list>"},
#endif
	{ "adblock_blacklist_custom",	""				},
	{ "adblock_whitelist",		""				},

/* advanced-mac */
	{ "wan_mac",			""				},
	{ "wl_macaddr",			""				},

/* advanced-misc */
	{ "boot_wait",			"on"				},
	{ "wait_time",			"5"				},
	{ "wan_speed",			"4"				},	// 0=10 Mb Full, 1=10 Mb Half, 2=100 Mb Full, 3=100 Mb Half, 4=Auto
	{ "jumbo_frame_enable",		"0"				},	// Jumbo Frames support (for RT-N16/WNR3500L)
	{ "jumbo_frame_size",		"2000"				},
#ifdef CONFIG_BCMWL5
	{ "ctf_disable",		"1"				},
#endif
#ifdef TCONFIG_BCMNAT
	{ "bcmnat_disable",		"1"				},
#endif

/* advanced-dhcpdns */
	{ "dhcpd_dmdns",		"1"				},
	{ "dhcpd_slt",			"0"				},
	{ "dhcpd_gwmode",		""				},
	{ "dhcpd_lmax",			""				},
	{ "dns_addget",			"0"				},
	{ "dns_intcpt",			"0"				},
	{ "dhcpc_minpkt",		"1"				},
	{ "dhcpc_custom",		""				},
	{ "dns_norebind",		"1"				},
	{ "dns_priv_override",		"0"				},
	{ "dnsmasq_debug",		"0"				},
	{ "dnsmasq_custom",		""				},
	{ "dnsmasq_static_only",	"0"				},
	{ "dnsmasq_q",			"0"				},	// Bit0=quiet-dhcp, 1=dhcp6, 2=ra
#ifdef TCONFIG_TOR
	{ "dnsmasq_onion_support",	"0"				},
#endif

/* advanced-firewall */
	{ "nf_loopback",		"0"				},
	{ "block_wan",			"1"				},	// block inbound icmp
	{ "block_wan_limit",		"1"				},
	{ "block_wan_limit_icmp",	"3"				},
	{ "multicast_pass",		"0"				},	// enable multicast proxy
	{ "multicast_lan",		"0"				},	// on LAN (br0)
	{ "multicast_lan1",		"0"				},	// on LAN1 (br1)
	{ "multicast_lan2",		"0"				},	// on LAN2 (br2)
	{ "multicast_lan3",		"0"				},	// on LAN3 (br3)
	{ "multicast_quickleave",	"1"				},	// enable quickleave mode
	{ "multicast_custom",		""				},	// custom config for IGMP proxy instead of default config
	{ "udpxy_enable",		"0"				},
	{ "udpxy_lan",			"0"				},	// listen on LAN (br0)
	{ "udpxy_lan1",			"0"				},	// listen on LAN (br1)
	{ "udpxy_lan2",			"0"				},	// listen on LAN (br2)
	{ "udpxy_lan3",			"0"				},	// listen on LAN (br3)
	{ "udpxy_stats",		"0"				},
	{ "udpxy_clients",		"3"				},
	{ "udpxy_port",			"4022"				},
	{ "ne_syncookies",		"0"				},	// tcp_syncookies
	{ "DSCP_fix_enable",		"1"				},	// Comacst DSCP fix
	{ "ne_snat",			"0"				},	// use SNAT instead of MASQUERADE
	{ "dhcp_pass",			"1"				},	// allow DHCP responses
	{ "ne_shlimit",			"1,3,60"			},	//shibby - enable limit connection attempts for sshd

/* advanced-routing */
	{ "routes_static",		""				},
	{ "dhcp_routes",		"1"				},
	{ "force_igmpv2",		"0"				},
	{ "wk_mode",			"gateway"			},	// Network mode [gateway|router]
#ifdef TCONFIG_ZEBRA
	{ "dr_setting",			"0"				},	// [ Disable | WAN | LAN | Both ]
	{ "dr_lan_tx",			"0"				},	// Dynamic-Routing LAN out
	{ "dr_lan_rx",			"0"				},	// Dynamic-Routing LAN in
	{ "dr_lan1_tx",			"0"				},	// Dynamic-Routing LAN out
	{ "dr_lan1_rx",			"0"				},	// Dynamic-Routing LAN in
	{ "dr_lan2_tx",			"0"				},	// Dynamic-Routing LAN out
	{ "dr_lan2_rx",			"0"				},	// Dynamic-Routing LAN in
	{ "dr_lan3_tx",			"0"				},	// Dynamic-Routing LAN out
	{ "dr_lan3_rx",			"0"				},	// Dynamic-Routing LAN in
	{ "dr_wan_tx",			"0"				},	// Dynamic-Routing WAN out
	{ "dr_wan_rx",			"0"				},	// Dynamic-Routing WAN in
	{ "dr_wan2_tx",			"0"				},	// Dynamic-Routing WAN out
	{ "dr_wan2_rx",			"0"				},	// Dynamic-Routing WAN in
#ifdef TCONFIG_MULTIWAN
	{ "dr_wan3_tx",			"0"				},	// Dynamic-Routing WAN out
	{ "dr_wan3_rx",			"0"				},	// Dynamic-Routing WAN in
	{ "dr_wan4_tx",			"0"				},	// Dynamic-Routing WAN out
	{ "dr_wan4_rx",			"0"				},	// Dynamic-Routing WAN in
#endif
#endif

/* advanced-vlan */
	{ "trunk_vlan_so",		"0"				},	// VLAN trunk support override

/* advanced-wireless */
	{ "wl_txant",			"3"				},
	{ "wl_txpwr",			"42"				},
	{ "wl_maxassoc",		"128"				},	// Max associations driver could support
	{ "wl_bss_maxassoc",		"128"				},
	{ "wl_distance",		""				},

/* forward-* */
#if defined(TCONFIG_NVRAM_32K) || defined(TCONFIG_OPTIMIZE_SIZE)
	{ "portforward",		""				},
	{ "trigforward",		""				},
#else
	{ "portforward",		"0<3<1.1.1.0/24<1000:2000<<192.168.1.2<ex: 1000 to 2000, restricted>0<2<<1000,2000<<192.168.1.2<ex: 1000 and 2000>0<1<<1000<2000<192.168.1.2<ex: different internal port>0<3<<1000:2000,3000<<192.168.1.2<ex: 1000 to 2000, and 3000>" },
	{ "trigforward",		"0<1<3000:4000<5000:6000<ex: open 5000-6000 if 3000-4000>"	},
#endif
#ifdef TCONFIG_IPV6
	{ "ipv6_portforward",		""				},
#endif
	{ "dmz_enable",			"0"				},
	{ "dmz_ipaddr",			"0"				},
	{ "dmz_sip",			""				},
	{ "dmz_ifname",			"br0"				},
	{ "dmz_ra",			"1"				},

/* forward-upnp */
	{ "upnp_enable",		"0"				},
	{ "upnp_secure",		"1"				},
	{ "upnp_port",			"0"				},
	{ "upnp_ssdp_interval",		"60"				},	// SSDP interval
	{ "upnp_mnp",			"0"				},
	{ "upnp_custom",		""				},

	{ "upnp_clean",			"1"				},	/* 0:Disable 1:Enable */
	{ "upnp_clean_interval",	"600"				},	/* Cleaning interval in seconds */
	{ "upnp_clean_threshold",	"20"				},	/* Threshold for cleaning unused rules */
#if 0	/* disabled for miniupnpd */
	{ "upnp_max_age",		"180"				},	// Max age
	{ "upnp_config",		"0"				},
#endif

/* qos */
	{ "atm_overhead",		"0"				},
	{ "qos_enable",			"0"				},
	{ "qos_ack",			"0"				},
	{ "qos_syn",			"1"				},
	{ "qos_fin",			"1"				},
	{ "qos_rst",			"1"				},
	{ "qos_udp",			"0"				},
	{ "qos_icmp",			"1"				},
	{ "qos_reset",			"1"				},
	{ "wan_qos_obw",		"700"				},
	{ "wan_qos_ibw",		"16000"				},
	{ "wan2_qos_obw",		"700"				},
	{ "wan2_qos_ibw",		"16000"				},
#ifdef TCONFIG_MULTIWAN
	{ "wan3_qos_obw",		"700"				},
	{ "wan3_qos_ibw",		"16000"				},
	{ "wan4_qos_obw",		"700"				},
	{ "wan4_qos_ibw",		"16000"				},
#endif
#if defined(TCONFIG_NVRAM_32K) || defined(TCONFIG_OPTIMIZE_SIZE_MORE)
	{ "qos_orules",			"0<<-1<d<53<0<<0:10<<0<DNS"	},
#else
	{ "qos_orules",			"0<<-1<d<53<0<<0:10<<0<DNS>0<<-1<d<37<0<<0:10<<0<Time>0<<17<d<123<0<<0:10<<0<NTP>0<<-1<d<3455<0<<0:10<<0<RSVP>0<<-1<d<9<0<<0:50<<3<SCTP,Discard>0<<-1<x<135,2101,2103,2105<0<<<<3<RPC (Microsoft)>0<<17<d<3544<0<<<<-1<Teredo Tunnel>0<<6<x<22,2222<0<<<<2<SSH>0<<6<d<23,992<0<<<<2<Telnet>0<<6<s<80,5938,8080,2222<0<<<<2<Remote Access>0<<-1<x<3389<0<<<<2<Remote Assistance>0<<-1<x<1220,6970:7170,8554<0<<<<4<Quicktime/RealAudio>0<<-1<x<554,5004,5005<0<<<<4<RTP, RTSP>0<<-1<x<1755<0<<<<4<MMS (Microsoft)>0<<-1<d<3478,3479,5060:5063<0<<<<1<SIP, Sipgate Stun Services>0<<-1<s<53,88,3074<0<<<<1<Xbox Live>0<<6<d<1718:1720<0<<<<1<H323>0<<-1<d<4380,27000:27050,11031,11235:11335,11999,2300:2400,6073,28800:29100,47624<0<<<<1<Various Games>0<<-1<d<1493,1502,1503,1542,1863,1963,3389,5061,5190:5193,7001<0<<<<5<MSGR1 - Windows Live>0<<-1<d<1071:1074,1455,1638,1644,5000:5010,5050,5100,5101,5150,8000:8002<0<<<<5<MSGR2 - Yahoo>0<<-1<d<194,1720,1730:1732,5220:5223,5298,6660:6669,22555<0<<<<5<MSGR3 - Additional>0<<-1<d<19294:19310<0<<<<5<Google+ & Voice>0<<6<d<6005,6006<0<<<<5<Camfrog>0<<-1<x<6571,6891:6901<0<<<<5<WLM File/Webcam>0<<-1<x<29613<0<<<<5<Skype incoming>0<<6<x<4244,5242<0<<<<1<Viber TCP>0<<17<x<5243,9785<0<<<<1<Viber UDP>0<<17<x<3478:3497,16384:16387,16393:16402<0<<<<5<Apple Facetime/Game Center>0<<6<d<443<0<<0:512<<3<HTTPS>0<<6<d<443<0<<512:<<5<HTTPS>0<<17<d<443<0<<0:512<<3<QUIC>0<<17<d<443<0<<512:<<5<QUIC>0<<-1<a<<0<skypetoskype<<<1<Skype to Skype>0<<-1<a<<0<skypeout<<<-1<Skype Phone (deprecated)>0<<-1<a<<0<youtube-2012<<<4<YouTube 2012 (Youtube)>0<<-1<a<<0<httpvideo<<<4<HTTP Video (Youtube)>0<<-1<a<<0<flash<<<4<Flash Video (Youtube)>0<<-1<a<<0<rtp<<<4<RTP>0<<-1<a<<0<rtmp<<<4<RTMP>0<<-1<a<<0<shoutcast<<<4<Shoutcast>0<<-2<a<<0<rtmpt<<<4<RTMPT (RTMP over HTTP)>0<<-1<a<<0<irc<<<5<IRC>0<<6<d<80,8080<0<<0:512<<3<HTTP, HTTP Proxy>0<<6<d<80,8080<0<<512:<<7<HTTP, HTTP Proxy File Transfers>0<<6<d<20,21,989,990<0<<<<7<FTP>0<<6<d<25,587,465,2525<0<<<<6<SMTP, Submission Mail>0<<6<d<110,995<0<<<<6<POP3 Mail>0<<6<d<119,563<0<<<<7<NNTP News & Downloads>0<<6<d<143,220,585,993<0<<<<6<IMAP Mail>0<<17<d<1:65535<0<<<<8<P2P (uTP, UDP)" },
#endif
	{ "qos_burst0",			""				},
	{ "qos_burst1",			""				},
	{ "qos_default",		"8"				},
	{ "qos_orates",			"5-100,5-30,5-100,5-70,5-70,5-70,5-70,5-100,5-30,1-1"				},
	{ "qos_irates",			"5-100,2-20,5-100,10-90,20-90,5-90,5-70,5-100,5-30,1-1"				},
	{ "qos_classnames",		"Service VOIP/Game Remote WWW Media HTTPS/Msgr Mail FileXfer P2P/Bulk Crawl"	},

	{ "ne_vegas",			"0"				},	// TCP Vegas
	{ "ne_valpha",			"2"				},
	{ "ne_vbeta",			"6"				},
	{ "ne_vgamma",			"2"				},

/* access restrictions */
	{ "rruleN",			"0"				},
#if defined(TCONFIG_NVRAM_32K) || defined(TCONFIG_OPTIMIZE_SIZE)
	{ "rrule0",			""				},
#else
	{ "rrule0",			"0|1320|300|31|||word text\n^begins-with.domain.\n.ends-with.net$\n^www.exact-domain.net$|0|example" },
#endif
	{ "rrulewp",			"80,8080"			},

/* admin-access */
	{ "http_username",		""				},	// Username
	{ "http_passwd",		"admin"				},	// Password
	{ "remote_management",		"0"				},	// Remote Management [1|0]
	{ "http_wanport",		"8080"				},	// WAN port to listen on
	{ "http_lanport",		"80"				},	// LAN port to listen on
	{ "http_enable",		"1"				},	// HTTP server enable/disable
#ifdef TCONFIG_HTTPS
	{ "remote_mgt_https",		"0"				},	// Remote Management use https [1|0]
	{ "https_lanport",		"443"				},	// LAN port to listen on
	{ "https_enable",		"0"				},	// HTTPS server enable/disable
	{ "https_crt_save",		"0"				},
	{ "https_crt_cn",		""				},
	{ "https_crt_file",		""				},
	{ "https_crt_gen",		""				},
#endif
	{ "web_wl_filter",		"0"				},	// Allow/Deny Wireless Access Web
	{ "web_css",			"default"			},
#ifdef TCONFIG_ADVTHEMES
	{ "web_adv_scripts",		"0"				},	// load JS resize chart script
#endif
	{ "web_dir",			"default"			},	// jffs, opt, tmp or default (/www)
	{ "ttb_css",			"example"			},	// Tomato Themes Base - default theme name
#ifdef TCONFIG_USB
	{ "ttb_loc",			""				},	// Tomato Themes Base - default files location
	{ "ttb_url",			"http://www.tomatothemebase.eu/wp-content/uploads"},	// Tomato Themes Base - default URL
#endif
	{ "web_svg",			"1"				},
	{ "telnetd_eas",		"1"				},
	{ "telnetd_port",		"23"				},
	{ "sshd_eas",			"1"				},	//shibby - enable sshd by default
	{ "sshd_pass",			"1"				},
	{ "sshd_port",			"22"				},
	{ "sshd_remote",		"0"				},
	{ "sshd_motd",			"1"				},
	{ "sshd_rport",			"22"				},
	{ "sshd_authkeys",		""				},
	{ "sshd_hostkey",		""				},
	{ "sshd_dsskey",		""				},
	{ "sshd_ecdsakey",		""				},
	{ "sshd_forwarding",		"1"				},
	{ "rmgt_sip",			""				},	// remote management: source ip address

	{ "http_id",			""				},
	{ "web_mx",			"status,bwm"			},
	{ "web_pb",			""				},

/* admin-bwm */
	{ "rstats_enable",		"1"				},
	{ "rstats_path",		""				},
	{ "rstats_stime",		"48"				},
	{ "rstats_offset",		"1"				},
	{ "rstats_data",		""				},
	{ "rstats_colors",		""				},
	{ "rstats_exclude",		""				},
	{ "rstats_sshut",		"1"				},
	{ "rstats_bak",			"0"				},

/* admin-ipt */
	{ "cstats_enable",		"0"				},
	{ "cstats_path",		""				},
	{ "cstats_stime",		"48"				},
	{ "cstats_offset",		"1"				},
	{ "cstats_labels",		"0"				},
	{ "cstats_exclude",		""				},
	{ "cstats_include",		""				},
	{ "cstats_all",			"1"				},
	{ "cstats_sshut",		"1"				},
	{ "cstats_bak",			"0"				},

/* advanced-buttons */
	{ "sesx_led",			"0"				},
	{ "sesx_b0",			"1"				},
	{ "sesx_b1",			"4"				},
	{ "sesx_b2",			"4"				},
	{ "sesx_b3",			"4"				},
	{ "sesx_script",
		"[ $1 -ge 20 ] && telnetd -p 233 -l /bin/sh\n"
	},
#if defined(TCONFIG_NVRAM_32K) || defined(TCONFIG_OPTIMIZE_SIZE)
	{ "script_brau",		""				},
#else
	{ "script_brau",
		"if [ ! -e /tmp/switch-start ]; then\n"
		"  # do something at startup\n"
		"  echo position at startup was $1 >/tmp/switch-start\n"
		"  exit\n"
		"fi\n"
		"if [ $1 = \"bridge\" ]; then\n"
		"  # do something\n"
		"  led bridge on\n"
		"elif [ $1 = \"auto\" ]; then\n"
		"  # do something\n"
		"  led bridge off\n"
		"fi\n"
	},
#endif

/* admin-log */
	{ "log_remote",			"0"				},
	{ "log_remoteip",		""				},
	{ "log_remoteport",		"514"				},
	{ "log_file",			"1"				},
	{ "log_file_custom",		"0"				},
	{ "log_file_path",		"/var/log/messages"		},
	{ "log_file_size",		"50"				},
	{ "log_file_keep",		"1"				},
	{ "log_limit",			"60"				},
	{ "log_in",			"0"				},
	{ "log_out",			"0"				},
	{ "log_mark",			"60"				},
	{ "log_events",			""				},

/* admin-log-webmonitor */
	{ "log_wm",			"0"				},
	{ "log_wmtype",			"0"				},
	{ "log_wmip",			""				},
	{ "log_wmdmax",			"2000"				},
	{ "log_wmsmax",			"2000"				},
	{ "webmon_bkp",			"0"				},
	{ "webmon_dir",			"/tmp"				},
	{ "webmon_shrink",		"0"				},

/* admin-debugging */
	{ "debug_nocommit",		"0"				},
	{ "debug_cprintf",		"0"				},
	{ "debug_cprintf_file",		"0"				},
	{ "debug_logsegfault",		"0"				},
	{ "console_loglevel",		"1"				},
	{ "t_cafree",			"1"				},
	{ "t_hidelr",			"0"				},
	{ "debug_clkfix",		"1"				},
	{ "debug_ddns",			"0"				},

/* admin-cifs */
	{ "cifs1",			""				},
	{ "cifs2",			""				},

/* admin-jffs2 */
	{ "jffs2_on",			"0"				},
	{ "jffs2_exec",			""				},

/* admin-tomatoanon */
	{ "tomatoanon_enable",		"-1"				},
	{ "tomatoanon_answer",		"0"				},
	{ "tomatoanon_id",		""				},
	{ "tomatoanon_notify",		"1"				},

#ifdef TCONFIG_USB
/* nas-usb - !!TB */
	{ "usb_enable",			"1"				},
	{ "usb_uhci",			"0"				},
	{ "usb_ohci",			"0"				},
	{ "usb_usb2",			"1"				},
#ifdef TCONFIG_MICROSD
	{ "usb_mmc",			"-1"				},
#endif
	{ "usb_irq_thresh",		"0"				},
	{ "usb_storage",		"1"				},
	{ "usb_printer",		"0"				},
	{ "usb_printer_bidirect",	"0"				},
	{ "usb_ext_opt",		""				},
	{ "usb_fat_opt",		""				},
	{ "usb_ntfs_opt",		""				},
	{ "usb_fs_ext3",		"1"				},
	{ "usb_fs_fat",			"1"				},
#ifdef TCONFIG_NTFS
	{ "usb_fs_ntfs",		"1"				},
#endif
#ifdef TCONFIG_HFS
	{ "usb_fs_hfs",			"0"				},	// !Victek
#endif
#ifdef TCONFIG_UPS
	{ "usb_apcupsd",		"0"				},
#endif
	{ "usb_automount",		"1"				},
#if 0
	{ "usb_bdflush",		"30 500 0 0 100 100 60 0 0"	},
#endif
	{ "script_usbhotplug",		""				},
	{ "script_usbmount",		""				},
	{ "script_usbumount",		""				},
	{ "idle_enable",		"0"				},
	{ "usb_3g",			"1"				},
#endif

#ifdef TCONFIG_FTP
/* nas-ftp - !!TB */
	{ "ftp_enable",			"0"				},
	{ "ftp_super",			"0"				},
	{ "ftp_anonymous",		"0"				},
	{ "ftp_dirlist",		"0"				},
	{ "ftp_port",			"21"				},
	{ "ftp_max",			"0"				},
	{ "ftp_ipmax",			"0"				},
	{ "ftp_staytimeout",		"300"				},
	{ "ftp_rate",			"0"				},
	{ "ftp_anonrate",		"0"				},
	{ "ftp_anonroot",		""				},
	{ "ftp_pubroot",		""				},
	{ "ftp_pvtroot",		""				},
	{ "ftp_users",			""				},
	{ "ftp_custom",			""				},
	{ "ftp_sip",			""				},	// wan ftp access: source ip address(es)
	{ "ftp_limit",			"0,3,60"			},
	{ "ftp_tls",			"0"				},
	{ "log_ftp",			"0"				},
#endif

#ifdef TCONFIG_SNMP
	{ "snmp_enable",		"0"				},
	{ "snmp_port",			"161"				},
	{ "snmp_remote",		"0"				},
	{ "snmp_remote_sip",		""				},
	{ "snmp_location",		"router"			},
	{ "snmp_contact",		"admin@tomato"			},
	{ "snmp_ro",			"rocommunity"			},
#endif

#ifdef TCONFIG_SAMBASRV
/* nas-samba - !!TB */
	{ "smbd_enable",		"0"				},
	{ "smbd_wgroup",		"WORKGROUP"			},
	{ "smbd_master",		"1"				},
	{ "smbd_wins",			"1"				},
	{ "smbd_cpage",			""				},
	{ "smbd_cset",			"utf8"				},
	{ "smbd_custom",		""				},
	{ "smbd_autoshare",		"2"				},
	{ "smbd_shares",		"jffs</jffs<JFFS<1<0>root$</<Hidden Root<0<1"	},
	{ "smbd_user",			"nas"				},
	{ "smbd_passwd",		""				},
	{ "smbd_ifnames",		"br0"				},
	{ "smbd_protocol",		"2"				}, /* 0 - SMB1, 1 - SMB2, 2 - SMB1+SMB2 (default) */
#endif

#ifdef TCONFIG_MEDIA_SERVER
/* nas-media */
	{ "ms_enable",			"0"				},	/* 0:Disable 1:Enable 2:Enable&Rescan */
	{ "ms_dirs",			"/mnt<"				},
	{ "ms_port",			"0"				},
	{ "ms_dbdir",			""				},
	{ "ms_ifname",			"br0"				},
	{ "ms_tivo",			"0"				},
	{ "ms_stdlna",			"0"				},
	{ "ms_sas",			"0"				},
#endif

#ifdef TCONFIG_SDHC
/* admin-sdhc */
	{ "mmc_on",			"0"				},
	{ "mmc_cs",			"7"				},
	{ "mmc_clk",			"3"				},
	{ "mmc_din",			"2"				},
	{ "mmc_dout",			"4"				},
	{ "mmc_fs_partition",		"1"				},
	{ "mmc_fs_type",		"ext2"				},
	{ "mmc_exec_mount",		""				},
	{ "mmc_exec_umount",		""				},
#endif

/* admin-sch */
	{ "sch_rboot",			""				},
	{ "sch_rcon",			""				},
	{ "sch_c1",			""				},
	{ "sch_c2",			""				},
	{ "sch_c3",			""				},
	{ "sch_c4",			""				},
	{ "sch_c5",			""				},
	{ "sch_c1_cmd",			""				},
	{ "sch_c2_cmd",			""				},
	{ "sch_c3_cmd",			""				},
	{ "sch_c4_cmd",			""				},
	{ "sch_c5_cmd",			""				},

/* admin-script */
	{ "script_init",		""				},
	{ "script_shut",		""				},
	{ "script_fire",		""				},
	{ "script_wanup",		""				},
	{ "script_mwanup",		""				},

#ifdef TCONFIG_NFS
	{ "nfs_enable",			"0"				},
	{ "nfs_enable_v2",		"0"				},
	{ "nfs_exports",		""				},
#endif

#ifdef TCONFIG_OPENVPN
/* vpn */
	{ "vpn_debug",			"0"				},
	{ "vpn_server_eas",		""				},
	{ "vpn_server_dns",		""				},
	{ "vpn_server1_poll",		"0"				},
	{ "vpn_server1_if",		"tun"				},
	{ "vpn_server1_proto",		"udp"				},
	{ "vpn_server1_port",		"1194"				},
	{ "vpn_server1_firewall",	"auto"				},
	{ "vpn_server1_crypt",		"tls"				},
	{ "vpn_server1_comp",		"-1"				},
	{ "vpn_server1_cipher",		"AES-128-CBC"			},
#if 0
	{ "vpn_server1_ncp_ciphers",	"AES-256-GCM:AES-128-GCM:AES-256-CBC:AES-128-CBC"},
#else
	{ "vpn_server1_ncp_ciphers",	"CHACHA20-POLY1305:AES-128-GCM:AES-256-GCM:AES-128-CBC:AES-256-CBC"},
#endif
	{ "vpn_server1_digest",		"default"			},
	{ "vpn_server1_dhcp",		"1"				},
	{ "vpn_server1_r1",		"192.168.1.50"			},
	{ "vpn_server1_r2",		"192.168.1.55"			},
	{ "vpn_server1_sn",		"10.6.0.0"			},
	{ "vpn_server1_nm",		"255.255.255.0"			},
	{ "vpn_server1_local",		"10.6.0.1"			},
	{ "vpn_server1_remote",		"10.6.0.2"			},
	{ "vpn_server1_reneg",		"-1"				},
	{ "vpn_server1_hmac",		"-1"				},
	{ "vpn_server1_plan",		"1"				},
	{ "vpn_server1_plan1",		"0"				},
	{ "vpn_server1_plan2",		"0"				},
	{ "vpn_server1_plan3",		"0"				},
	{ "vpn_server1_pdns",		"0"				},
	{ "vpn_server1_ccd",		"0"				},
	{ "vpn_server1_c2c",		"0"				},
	{ "vpn_server1_ccd_excl",	"0"				},
	{ "vpn_server1_ccd_val",	""				},
	{ "vpn_server1_rgw",		"0"				},
	{ "vpn_server1_userpass",	"0"				},
	{ "vpn_server1_nocert",		"0"				},
	{ "vpn_server1_custom",		""				},
	{ "vpn_server1_static",		""				},
	{ "vpn_server1_ca",		""				},
	{ "vpn_server1_ca_key",		""				},
	{ "vpn_server1_crt",		""				},
	{ "vpn_server1_crl",		""				},
	{ "vpn_server1_key",		""				},
	{ "vpn_server1_dh",		""				},
	{ "vpn_server1_br",		"br0"				},
	{ "vpn_server1_serial",		"00"				},
	{ "vpn_server2_poll",		"0"				},
	{ "vpn_server2_if",		"tun"				},
	{ "vpn_server2_proto",		"udp"				},
	{ "vpn_server2_port",		"1195"				},
	{ "vpn_server2_firewall",	"auto"				},
	{ "vpn_server2_crypt",		"tls"				},
	{ "vpn_server2_comp",		"-1"				},
	{ "vpn_server2_cipher",		"AES-128-CBC"			},
#if 0
	{ "vpn_server2_ncp_ciphers",	"AES-256-GCM:AES-128-GCM:AES-256-CBC:AES-128-CBC"},
#else
	{ "vpn_server2_ncp_ciphers",	"CHACHA20-POLY1305:AES-128-GCM:AES-256-GCM:AES-128-CBC:AES-256-CBC"},
#endif
	{ "vpn_server2_digest",		"default"			},
	{ "vpn_server2_dhcp",		"1"				},
	{ "vpn_server2_r1",		"192.168.1.50"			},
	{ "vpn_server2_r2",		"192.168.1.55"			},
	{ "vpn_server2_sn",		"10.7.0.0"			},
	{ "vpn_server2_nm",		"255.255.255.0"			},
	{ "vpn_server2_local",		"10.7.0.1"			},
	{ "vpn_server2_remote",		"10.7.0.2"			},
	{ "vpn_server2_reneg",		"-1"				},
	{ "vpn_server2_hmac",		"-1"				},
	{ "vpn_server2_plan",		"1"				},
	{ "vpn_server2_plan1",		"0"				},
	{ "vpn_server2_plan2",		"0"				},
	{ "vpn_server2_plan3",		"0"				},
	{ "vpn_server2_pdns",		"0"				},
	{ "vpn_server2_ccd",		"0"				},
	{ "vpn_server2_c2c",		"0"				},
	{ "vpn_server2_ccd_excl",	"0"				},
	{ "vpn_server2_ccd_val",	""				},
	{ "vpn_server2_rgw",		"0"				},
	{ "vpn_server2_userpass",	"0"				},
	{ "vpn_server2_nocert",		"0"				},
	{ "vpn_server2_custom",		""				},
	{ "vpn_server2_static",		""				},
	{ "vpn_server2_ca",		""				},
	{ "vpn_server2_ca_key",		""				},
	{ "vpn_server2_crt",		""				},
	{ "vpn_server1_crl",		""				},
	{ "vpn_server2_key",		""				},
	{ "vpn_server2_dh",		""				},
	{ "vpn_server2_br",		"br0"				},
	{ "vpn_server2_serial",		"00"				},
	{ "vpn_client_eas",		""				},
	{ "vpn_client1_poll",		"0"				},
	{ "vpn_client1_if",		"tun"				},
	{ "vpn_client1_bridge",		"1"				},
	{ "vpn_client1_nat",		"1"				},
	{ "vpn_client1_proto",		"udp"				},
	{ "vpn_client1_addr",		""				},
	{ "vpn_client1_port",		"1194"				},
	{ "vpn_client1_retry",		"30"				},
	{ "vpn_client1_rg",		"0"				},
	{ "vpn_client1_firewall",	"auto"				},
	{ "vpn_client1_crypt",		"tls"				},
	{ "vpn_client1_comp",		"-1"				},
	{ "vpn_client1_cipher",		"default"			},
#if 0
	{ "vpn_client1_ncp_ciphers",	"AES-256-GCM:AES-128-GCM:AES-256-CBC:AES-128-CBC"},
#else
	{ "vpn_client1_ncp_ciphers",	"CHACHA20-POLY1305:AES-128-GCM:AES-256-GCM:AES-128-CBC:AES-256-CBC"},
#endif
	{ "vpn_client1_digest",		"default"			},
	{ "vpn_client1_local",		"10.8.0.2"			},
	{ "vpn_client1_remote",		"10.8.0.1"			},
	{ "vpn_client1_nm",		"255.255.255.0"			},
	{ "vpn_client1_reneg",		"-1"				},
	{ "vpn_client1_hmac",		"-1"				},
	{ "vpn_client1_adns",		"0"				},
	{ "vpn_client1_rgw", 		"0"				},
	{ "vpn_client1_gw",		""				},
	{ "vpn_client1_custom",		""				},
	{ "vpn_client1_static",		""				},
	{ "vpn_client1_ca",		""				},
	{ "vpn_client1_crt",		""				},
	{ "vpn_client1_key",		""				},
	{ "vpn_client1_br",		"br0"				},
	{ "vpn_client1_nobind",		"1"				},
	{ "vpn_client1_routing_val",	""				},
	{ "vpn_client1_fw",		"1"				},
	{ "vpn_client1_tlsvername",	"0"				},
	{ "vpn_client2_poll",		"0"				},
	{ "vpn_client2_if",		"tun"				},
	{ "vpn_client2_bridge",		"1"				},
	{ "vpn_client2_nat",		"1"				},
	{ "vpn_client2_proto",		"udp"				},
	{ "vpn_client2_addr",		""				},
	{ "vpn_client2_port",		"1194"				},
	{ "vpn_client2_retry",		"30"				},
	{ "vpn_client2_rg",		"0"				},
	{ "vpn_client2_firewall",	"auto"				},
	{ "vpn_client2_crypt",		"tls"				},
	{ "vpn_client2_comp",		"-1"				},
	{ "vpn_client2_cipher",		"default"			},
#if 0
	{ "vpn_client2_ncp_ciphers",	"AES-256-GCM:AES-128-GCM:AES-256-CBC:AES-128-CBC"},
#else
	{ "vpn_client2_ncp_ciphers",	"CHACHA20-POLY1305:AES-128-GCM:AES-256-GCM:AES-128-CBC:AES-256-CBC"},
#endif
	{ "vpn_client2_digest",		"default"			},
	{ "vpn_client2_local",		"10.9.0.2"			},
	{ "vpn_client2_remote",		"10.9.0.1"			},
	{ "vpn_client2_nm",		"255.255.255.0"			},
	{ "vpn_client2_reneg",		"-1"				},
	{ "vpn_client2_hmac",		"-1"				},
	{ "vpn_client2_adns",		"0"				},
	{ "vpn_client2_rgw",		"0"				},
	{ "vpn_client2_gw",		""				},
	{ "vpn_client2_custom",		""				},
	{ "vpn_client2_static",		""				},
	{ "vpn_client2_ca",		""				},
	{ "vpn_client2_crt",		""				},
	{ "vpn_client2_key",		""				},
	{ "vpn_client2_br",		"br0"				},
	{ "vpn_client2_nobind",		"1"				},
	{ "vpn_client2_routing_val",	""				},
	{ "vpn_client2_fw",		"1"				},
	{ "vpn_client2_tlsvername",	"0"				},
#endif

#ifdef TCONFIG_PPTPD
	{ "pptp_client_eas",		"0"				},
	{ "pptp_client_usewan",		"none"				},
	{ "pptp_client_peerdns",	"0"				},
	{ "pptp_client_mtuenable",	"0"				},
	{ "pptp_client_mtu",		"1400"				},
	{ "pptp_client_mruenable",	"0"				},
	{ "pptp_client_mru",		"1400"				},
	{ "pptp_client_nat",		"0"				},
	{ "pptp_client_srvip",		""				},
	{ "pptp_client_srvsub",		"10.0.0.0"			},
	{ "pptp_client_srvsubmsk",	"255.0.0.0"			},
	{ "pptp_client_username",	""				},
	{ "pptp_client_passwd",		""				},
	{ "pptp_client_crypt",		"0"				},
	{ "pptp_client_custom",		""				},
	{ "pptp_client_dfltroute",	"0"				},
	{ "pptp_client_stateless",	"1"				},
	{ "pptpd_chap",			"0"				},	/* 0/1/2 (Auto/MS-CHAPv1/MS-CHAPv2) */
#endif

#ifdef TCONFIG_TINC
	{"tinc_wanup",			"0"				},
	{"tinc_name",			""				},
	{"tinc_devicetype",		"tun"				},	// tun, tap
	{"tinc_mode",			"switch"			},	// switch, hub
	{"tinc_vpn_netmask",		"255.255.0.0"			},
	{"tinc_private_rsa",		""				},
	{"tinc_private_ed25519",	""				},
	{"tinc_custom",			""				},
	{"tinc_hosts",			""				},
	{"tinc_manual_firewall",	""				},
	{"tinc_manual_tinc_up",		"0"				},
	{"tinc_poll",			"0"				},
	/* scripts */
	{"tinc_tinc_up",		""				},
	{"tinc_tinc_down",		""				},
	{"tinc_host_up",		""				},
	{"tinc_host_down",		""				},
	{"tinc_subnet_up",		""				},
	{"tinc_subnet_down",		""				},
	{"tinc_firewall",		""				},
#endif

#ifdef TCONFIG_BT
/* nas-transmission */
	{ "bt_enable",			"0"				},
#ifdef TCONFIG_BBT
	{ "bt_binary",			"internal"			},
#else
	{ "bt_binary",			"optware"			},
#endif
	{ "bt_binary_custom",		"/path/to/binaries/directory"	},
	{ "bt_custom",			""				},
	{ "bt_port",			"51515"				},
	{ "bt_dir",			"/mnt"				},
	{ "bt_incomplete",		"1"				},
	{ "bt_autoadd",			"1"				},
	{ "bt_settings",		"down_dir"			},
	{ "bt_settings_custom",		"/tmp/btclient"			},
	{ "bt_rpc_enable",		"1"				},
	{ "bt_rpc_wan",			"0"				},
	{ "bt_auth",			"1"				},
	{ "bt_login",			"admin"				},
	{ "bt_password",		"admin11"			},
	{ "bt_port_gui",		"9091"				},
	{ "bt_dl_enable",		"0"				},
	{ "bt_ul_enable",		"0"				},
	{ "bt_dl",			"248"				},
	{ "bt_ul",			"64"				},
	{ "bt_peer_limit_global",	"150"				},
	{ "bt_peer_limit_per_torrent",	"30"				},
	{ "bt_ul_slot_per_torrent",	"10"				},
	{ "bt_ratio_enable",		"0"				},
	{ "bt_ratio",			"1.0000"			},
	{ "bt_ratio_idle_enable",	"0"				},
	{ "bt_ratio_idle",		"30"				},
	{ "bt_dht",			"0"				},
	{ "bt_pex",			"0"				},
	{ "bt_lpd",			"0"				},
	{ "bt_utp",			"1"				},
	{ "bt_blocklist",		"0"				},
	{ "bt_blocklist_url",		"http://list.iblocklist.com/?list=bt_level1"	},
	{ "bt_sleep",			"10"				},
	{ "bt_check",			"1"				},
	{ "bt_check_time",		"15"				},
	{ "bt_dl_queue_enable",		"0"				},
	{ "bt_dl_queue_size",		"5"				},
	{ "bt_ul_queue_enable",		"0"				},
	{ "bt_ul_queue_size",		"5"				},
	{ "bt_message",			"2"				},
	{ "bt_log",			"0"				},
	{ "bt_log_path",		"/var/log"			},
#endif

/* bwlimit */
	{ "bwl_enable",			"0"				},
	{ "bwl_rules",			""				},
	{ "bwl_br0_enable",		"0"				},
	{ "bwl_br0_dlc",		""				},
	{ "bwl_br0_ulc",		""				},
	{ "bwl_br0_dlr",		""				},
	{ "bwl_br0_ulr",		""				},
	{ "bwl_br0_tcp",		"0"				},	/* unlimited */
	{ "bwl_br0_udp",		"0"				},	/* unlimited */
	{ "bwl_br0_prio",		"3"				},
	{ "bwl_br1_enable",		"0"				},
	{ "bwl_br1_dlc",		""				},
	{ "bwl_br1_ulc",		""				},
	{ "bwl_br1_dlr",		""				},
	{ "bwl_br1_ulr",		""				},
	{ "bwl_br1_prio",		"2"				},
	{ "bwl_br2_enable",		"0"				},
	{ "bwl_br2_dlc",		""				},
	{ "bwl_br2_ulc",		""				},
	{ "bwl_br2_dlr",		""				},
	{ "bwl_br2_ulr",		""				},
	{ "bwl_br2_prio",		"2"				},
	{ "bwl_br3_enable",		"0"				},
	{ "bwl_br3_dlc",		""				},
	{ "bwl_br3_ulc",		""				},
	{ "bwl_br3_dlr",		""				},
	{ "bwl_br3_ulr",		""				},
	{ "bwl_br3_prio",		"2"				},


/* NoCatSplash. !!Victek */
#ifdef TCONFIG_NOCAT
	{ "NC_enable",			"0"				},	// enable NoCatSplash
	{ "NC_Verbosity",		"2"				},
	{ "NC_GatewayName",		"FreshTomato Captive Portal"	},
	{ "NC_GatewayPort",		"5280"				},
	{ "NC_GatewayMode",		"Open"				},
	{ "NC_DocumentRoot",		"/tmp/splashd"			},
	{ "NC_ExcludePorts",		"1863"				},
	{ "NC_HomePage",		"http://google.com"		},
	{ "NC_ForcedRedirect",		"0"				},
	{ "NC_IdleTimeout",		"0"				},
	{ "NC_MaxMissedARP",		"5"				},
	{ "NC_PeerChecktimeout",	"0"				},
	{ "NC_LoginTimeout",		"3600"				},
	{ "NC_RenewTimeout",		"0"				},
	{ "NC_AllowedWebHosts",		""				},
	{ "NC_BridgeLAN",		"br0"				},
#endif

/* Tomato RAF - NGINX */
#ifdef TCONFIG_NGINX
	{"nginx_enable",		"0"				},	// NGinX enabled
	{"nginx_php",			"0"				},	// PHP enabled
	{"nginx_keepconf",		"0"				},	// Enable/disable keep configuration files unmodified in /etc/nginx
	{"nginx_docroot",		"/www"				},	// path for server files
	{"nginx_port",			"85"				},	// port to listen
	{"nginx_remote",		"0"				},	// open port from WAN site
	{"nginx_fqdn",			"Tomato"			},	// server name
	{"nginx_upload",		"100"				},	// upload file size limit
	{"nginx_priority",		"10"				},	// server priority = worker_priority
	{"nginx_custom",		""				},	// additional lines for nginx.conf
	{"nginx_httpcustom",		""				},	// additional lines for nginx.conf
	{"nginx_servercustom",		""				},	// additional lines for nginx.conf
	{"nginx_phpconf",		""				},	// additional lines for php.ini
	{"nginx_user",			"root"				},	// user/group
	{"nginx_override",		"0"				},	// additional lines for php.ini
	{"nginx_overridefile",		"/path/to/nginx.conf"		},	// user/group

/* bwq518 - MySQL */
	{ "mysql_enable",		"0"				},
	{ "mysql_sleep",		"2"				},
	{ "mysql_check",		"1"				},
	{ "mysql_check_time",		"1"				},
	{ "mysql_binary",		"internal"			},
	{ "mysql_binary_custom",	"/mnt/sda1/mysql/bin"		},
	{ "mysql_usb_enable",		"1"				},
	{ "mysql_dlroot",		""				},
	{ "mysql_datadir",		"data"				},
	{ "mysql_tmpdir",		"tmp"				},
	{ "mysql_server_custom",	""				},
	{ "mysql_port",			"3306"				},
	{ "mysql_allow_anyhost",	"0"				},
	{ "mysql_init_rootpass",	"0"				},
	{ "mysql_username",		"root"				},	// mysqladmin username
	{ "mysql_passwd",		"admin"				},	// mysqladmin password
	{ "mysql_key_buffer",		"16"				},	// KB
	{ "mysql_max_allowed_packet",	"4"				},	// MB
	{ "mysql_thread_stack",		"128"				},	// KB
	{ "mysql_thread_cache_size",	"8"				},
	{ "mysql_init_priv",		"0"				},
	{ "mysql_table_open_cache",	"4"				},
	{ "mysql_sort_buffer_size",	"128"				},	// KB
	{ "mysql_read_buffer_size",	"128"				},	// KB
	{ "mysql_query_cache_size",	"16"				},	// MB
	{ "mysql_read_rnd_buffer_size",	"256"				},	// KB
	{ "mysql_net_buffer_length",	"2"				},	// K
	{ "mysql_max_connections",	"1000"				},
#endif

#ifdef TCONFIG_TOR
	{ "tor_enable",			"0"				},
	{ "tor_solve_only",		"0"				},
	{ "tor_socksport",		"9050"				},
	{ "tor_transport",		"9040"				},
	{ "tor_dnsport",		"9053"				},
	{ "tor_datadir",		"/tmp/tor"			},
	{ "tor_iface",			"br0"				},
	{ "tor_users",			"192.168.1.0/24"		},
	{ "tor_custom",			""				},
	{ "tor_ports",			"80"				},
	{ "tor_ports_custom",		"80,443,8080:8880"		},
#endif

	{ NULL, NULL	}
};

const defaults_t if_generic[] = {
	{ "lan_ifname",			"br0"				},
	{ "lan_ifnames",		"eth0 eth2 eth3 eth4"		},
	{ "wan_ifname",			"eth1"				},
	{ "wan_ifnames",		"eth1"				},
	{ NULL, NULL }
};

const defaults_t if_vlan[] = {
	{ "lan_ifname",			"br0"				},
	{ "lan_ifnames",		"vlan0 eth1 eth2 eth3"		},
	{ "lan1_ifname",		""				},
	{ "lan1_ifnames",		""				},
	{ "lan2_ifname",		""				},
	{ "lan2_ifnames",		""				},
	{ "lan3_ifname",		""				},
	{ "lan3_ifnames",		""				},
	{ "wan_ifname",			"vlan1"				},
	{ "wan_ifnames",		"vlan1"				},
	{ NULL, NULL }
};
