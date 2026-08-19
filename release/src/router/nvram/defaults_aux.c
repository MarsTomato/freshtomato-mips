/*
 * defaults_aux.c
 *
 * Copyright (C) 2025 - 2026 FreshTomato
 * https://freshtomato.org/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */


#include <tomato_config.h>
#include "tomato_profile.h"
#include "defaults.h"

#if BRIDGE_COUNT < 1 || BRIDGE_COUNT > 16
 #error "Unsupported BRIDGE_COUNT range"
#endif

const defaults_t if_generic[] = {
	{ "lan_ifname",			"br0"				},
	{ "lan_ifnames",		"eth0 eth2 eth3 eth4"		},
	{ "wan_ifname",			"eth1"				},
	{ "wan_ifnames",		"eth1"				},

	{ NULL, NULL }
};

#define BRIDGE_BLOCK_IF_VLAN(i) \
	{ "lan" #i "_ifname",		""				}, \
	{ "lan" #i "_ifnames",		""				},

const defaults_t if_vlan[] = {
	{ "wan_ifname",			"vlan1"				},
	{ "wan_ifnames",		"vlan1"				},
	{ "lan_ifname",			"br0"				},
	{ "lan_ifnames",		"vlan0 eth1 eth2 eth3"		},
#if BRIDGE_COUNT >= 2
 BRIDGE_BLOCK_IF_VLAN(1)
#endif
#if BRIDGE_COUNT >= 3
 BRIDGE_BLOCK_IF_VLAN(2)
#endif
#if BRIDGE_COUNT >= 4
 BRIDGE_BLOCK_IF_VLAN(3)
#endif
#if BRIDGE_COUNT >= 5
 BRIDGE_BLOCK_IF_VLAN(4)
#endif
#if BRIDGE_COUNT >= 6
 BRIDGE_BLOCK_IF_VLAN(5)
#endif
#if BRIDGE_COUNT >= 7
 BRIDGE_BLOCK_IF_VLAN(6)
#endif
#if BRIDGE_COUNT >= 8
 BRIDGE_BLOCK_IF_VLAN(7)
#endif
#if BRIDGE_COUNT >= 9
 BRIDGE_BLOCK_IF_VLAN(8)
#endif
#if BRIDGE_COUNT >= 10
 BRIDGE_BLOCK_IF_VLAN(9)
#endif
#if BRIDGE_COUNT >= 11
 BRIDGE_BLOCK_IF_VLAN(10)
#endif
#if BRIDGE_COUNT >= 12
 BRIDGE_BLOCK_IF_VLAN(11)
#endif
#if BRIDGE_COUNT >= 13
 BRIDGE_BLOCK_IF_VLAN(12)
#endif
#if BRIDGE_COUNT >= 14
 BRIDGE_BLOCK_IF_VLAN(13)
#endif
#if BRIDGE_COUNT >= 15
 BRIDGE_BLOCK_IF_VLAN(14)
#endif
#if BRIDGE_COUNT >= 16
 BRIDGE_BLOCK_IF_VLAN(15)
#endif
	{ NULL, NULL }
};
