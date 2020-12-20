<!DOCTYPE html>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] Advanced: DHCP / DNS</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<script src="tomato.js"></script>

<script>

//	<% nvram("dnsmasq_q,ipv6_radvd,ipv6_dhcpd,ipv6_lease_time,dhcpd_dmdns,dns_addget,dhcpd_gwmode,dns_intcpt,dhcpd_slt,dhcpc_minpkt,dnsmasq_custom,dnsmasq_onion_support,dhcpd_lmax,dhcpc_custom,dns_norebind,dns_priv_override,dhcpd_static_only,dnsmasq_debug"); %>

var cprefix = 'advanced_dhcpdns';
if ((isNaN(nvram.dhcpd_lmax)) || ((nvram.dhcpd_lmax *= 1) < 1)) nvram.dhcpd_lmax = 255;

function verifyFields(focused, quiet) {
	var b = (E('_f_dhcpd_sltsel').value == 1);
	elem.display('_dhcpd_sltman', b);
	if ((b) && (!v_range('_f_dhcpd_slt', quiet, 1, 43200))) return 0;
	if (!v_length('_dnsmasq_custom', quiet, 0, 4096)) return 0;
	if (!v_range('_dhcpd_lmax', quiet, 1, 0xFFFF)) return 0;
/* IPV6-BEGIN */
	if (!v_range('_f_ipv6_lease_time', quiet, 1, 720)) return 0;
/* IPV6-END */
	if (!v_length('_dhcpc_custom', quiet, 0, 256)) return 0;
	return 1;
}

function nval(a, b) {
	return (a == null || (a + '').trim() == '') ? b : a;
}

function save() {
	if (!verifyFields(null, false)) return;

	var a;
	var fom = E('t_fom');

	fom.dhcpd_dmdns.value = E('_f_dhcpd_dmdns').checked ? 1 : 0;
	a = E('_f_dhcpd_sltsel').value;
	fom.dhcpd_slt.value = (a != 1) ? a : E('_f_dhcpd_slt').value;
	fom.dns_addget.value = E('_f_dns_addget').checked ? 1 : 0;
	fom.dns_norebind.value = E('_f_dns_norebind').checked ? 1 : 0;
	fom.dhcpd_gwmode.value = E('_f_dhcpd_gwmode').checked ? 1 : 0;
	fom.dns_intcpt.value = E('_f_dns_intcpt').checked ? 1 : 0;
	fom.dns_priv_override.value = E('_f_dns_priv_override').checked ? 1 : 0;
	fom.dhcpc_minpkt.value = E('_f_dhcpc_minpkt').checked ? 1 : 0;
	fom.dhcpd_static_only.value = E('_f_dhcpd_static_only').checked ? '1' : '0';
	fom.dnsmasq_debug.value = E('_f_dnsmasq_debug').checked ? '1' : '0';
/* TOR-BEGIN */
	fom.dnsmasq_onion_support.value = E('_f_dnsmasq_onion_support').checked ? '1' : '0';
/* TOR-END */
/* IPV6-BEGIN */
	fom.ipv6_radvd.value = E('_f_ipv6_radvd').checked ? '1' : '0';
	fom.ipv6_dhcpd.value = E('_f_ipv6_dhcpd').checked ? '1' : '0';
	fom.ipv6_lease_time.value = E('_f_ipv6_lease_time').value;
/* IPV6-END */

	fom.dnsmasq_q.value = 0;
	if (fom.f_dnsmasq_q4.checked) fom.dnsmasq_q.value |= 1;
/* IPV6-BEGIN */
	if (fom.f_dnsmasq_q6.checked) fom.dnsmasq_q.value |= 2;
	if (fom.f_dnsmasq_qr.checked) fom.dnsmasq_q.value |= 4;
/* IPV6-END */

	if (fom.dhcpc_minpkt.value != nvram.dhcpc_minpkt ||
	    fom.dhcpc_custom.value != nvram.dhcpc_custom) {
		nvram.dhcpc_minpkt = fom.dhcpc_minpkt.value;
		nvram.dhcpc_custom = fom.dhcpc_custom.value;
		fom._service.value = '*';
	}
	else {
		fom._service.value = 'dnsmasq-restart';
	}


	if (fom.dns_intcpt.value != nvram.dns_intcpt) {
		nvram.dns_intcpt = fom.dns_intcpt.value;
		if (fom._service.value != '*') fom._service.value += ',firewall-restart';
	}

/* IPV6-BEGIN */
	if (fom.dhcpd_dmdns.value != nvram.dhcpd_dmdns) {
		nvram.dhcpd_dmdns = fom.dhcpd_dmdns.value;
		if (fom._service.value != '*') fom._service.value += ',dnsmasq-restart';
	}
/* IPV6-END */

	form.submit(fom, 1);
}

function init() {
	var c;
	if (((c = cookie.get(cprefix + '_notes_vis')) != null) && (c == '1')) {
		toggleVisibility(cprefix, "notes");
	}
	eventHandler();
}
</script>
</head>

<body onload="init()">
<form id="t_fom" method="post" action="tomato.cgi">
<table id="container">
<tr><td colspan="2" id="header">
	<div class="title">FreshTomato</div>
	<div class="version">Version <% version(); %> on <% nv("t_model_name"); %></div>
</td></tr>
<tr id="body"><td id="navi"><script>navi()</script></td>
<td id="content">
<div id="ident"><% ident(); %></div>

<!-- / / / -->

<input type="hidden" name="_nextpage" value="advanced-dhcpdns.asp">
<input type="hidden" name="_service" value="">
<input type="hidden" name="dhcpd_dmdns">
<input type="hidden" name="dhcpd_slt">
<input type="hidden" name="dns_addget">
<input type="hidden" name="dns_norebind">
<input type="hidden" name="dhcpd_gwmode">
<input type="hidden" name="dns_intcpt">
<input type="hidden" name="dns_priv_override">
<input type="hidden" name="dhcpc_minpkt">
<input type="hidden" name="dhcpd_static_only">
<input type="hidden" name="dnsmasq_debug">
<!-- IPV6-BEGIN -->
<input type="hidden" name="ipv6_radvd">
<input type="hidden" name="ipv6_dhcpd">
<input type="hidden" name="ipv6_lease_time">
<!-- IPV6-END -->
<input type="hidden" name="dnsmasq_q">
<!-- TOR-BEGIN -->
<input type="hidden" name="dnsmasq_onion_support">
<!-- TOR-END -->

<!-- / / / -->

<div class="section-title">DHCP / DNS Server (LAN)</div>
<div class="section">
	<script>
		createFieldTable('', [
			{ title: 'Use internal DNS', name: 'f_dhcpd_dmdns', type: 'checkbox', value: nvram.dhcpd_dmdns == '1' },
			{ title: 'Debug Mode', indent: 2, name: 'f_dnsmasq_debug', type: 'checkbox', value: nvram.dnsmasq_debug == '1' },
			{ title: 'Use received DNS with user-entered DNS', name: 'f_dns_addget', type: 'checkbox', value: nvram.dns_addget == '1' },
			{ title: 'Prevent DNS-rebind attacks', name: 'f_dns_norebind', type: 'checkbox', value: nvram.dns_norebind == '1' },
			{ title: 'Intercept DNS port', name: 'f_dns_intcpt', type: 'checkbox', value: nvram.dns_intcpt == '1' },
			{ title: 'Prevent client auto DoH', name: 'f_dns_priv_override', type: 'checkbox', value: nvram.dns_priv_override == '1' },
			{ title: 'Use user-entered gateway if WAN is disabled', name: 'f_dhcpd_gwmode', type: 'checkbox', value: nvram.dhcpd_gwmode == '1' },
			{ title: 'Ignore DHCP requests from unknown devices', name: 'f_dhcpd_static_only', type: 'checkbox', value: nvram.dhcpd_static_only == '1' },
/* TOR-BEGIN */
			{ title: 'Solve .onion using Tor<br>(<a href="advanced-tor.asp" class="new_window">enable Tor first<\/a>)', name: 'f_dnsmasq_onion_support', type: 'checkbox', value: nvram.dnsmasq_onion_support == '1' },
/* TOR-END */
			{ title: 'Maximum active DHCP leases', name: 'dhcpd_lmax', type: 'text', maxlen: 5, size: 8, value: nvram.dhcpd_lmax },
			{ title: 'Static lease time', multi: [
				{ name: 'f_dhcpd_sltsel', type: 'select', options: [[0,'Same as normal lease time'],[-1,'"Infinite"'],[1,'Custom']],
					value: (nvram.dhcpd_slt < 1) ? nvram.dhcpd_slt : 1 },
				{ name: 'f_dhcpd_slt', type: 'text', maxlen: 5, size: 8, prefix: '<span id="_dhcpd_sltman"> ', suffix: ' <i>(minutes)<\/i><\/span>',
					value: (nvram.dhcpd_slt >= 1) ? nvram.dhcpd_slt : 3600 } ] },
/* IPV6-BEGIN */
			{ title: 'Announce IPv6 on LAN (SLAAC)', name: 'f_ipv6_radvd', type: 'checkbox', value: nvram.ipv6_radvd == '1' },
			{ title: 'Announce IPv6 on LAN (DHCP)', name: 'f_ipv6_dhcpd', type: 'checkbox', value: nvram.ipv6_dhcpd == '1' },
			{ title: 'DHCP IPv6 lease time', name: 'f_ipv6_lease_time', type: 'text', maxlen: 3, size: 8, suffix: ' <small> (in hours)<\/small>', value: nvram.ipv6_lease_time || 12 },
/* IPV6-END */
			{ title: 'Mute dhcpv4 logging', name: 'f_dnsmasq_q4', type: 'checkbox', value: (nvram.dnsmasq_q & 1) },
/* IPV6-BEGIN */
			{ title: 'Mute dhcpv6 logging', name: 'f_dnsmasq_q6', type: 'checkbox', value: (nvram.dnsmasq_q & 2) },
			{ title: 'Mute RA logging', name: 'f_dnsmasq_qr', type: 'checkbox', value: (nvram.dnsmasq_q & 4) },
/* IPV6-END */
			{ title: '<a href="http://www.thekelleys.org.uk/" class="new_window">Dnsmasq<\/a><br>Custom configuration', name: 'dnsmasq_custom', type: 'textarea', value: nvram.dnsmasq_custom }
		]);
	</script>
</div>

<!-- / / / -->

<div class="section-title">DHCP Client (WAN)</div>
<div class="section">
	<script>
		createFieldTable('', [
			{ title: 'DHCPC Options', name: 'dhcpc_custom', type: 'textarea', value: nvram.dhcpc_custom },
			{ title: 'Reduce packet size', name: 'f_dhcpc_minpkt', type: 'checkbox', value: nvram.dhcpc_minpkt == '1' }
		]);
	</script>
</div>

<!-- / / / -->

<div class="section-title">Notes <small><i><a href='javascript:toggleVisibility(cprefix,"notes");'><span id="sesdiv_notes_showhide">(Click here to show)</span></a></i></small></div>
<div class="section" id="sesdiv_notes" style="display:none">
	<i>DHCP / DNS Server (LAN):</i><br>
	<ul>
		<li><b>Use internal DNS</b> - Allow dnsmasq to be your DNS server on LAN.</li>
		<li><b>Use received DNS with user-entered DNS</b> - Add DNS servers received from your WAN connection to the static DNS server list (see <a href="basic-network.asp">Network</a> configuration).</li>
		<li><b>Prevent DNS-rebind attacks</b> - Enable DNS rebinding protection on Dnsmasq.</li>
		<li><b>Intercept DNS port</b> - Any DNS requests/packets sent out to UDP/TCP port 53 are redirected to the internal DNS server. Currently only IPv4 DNS is intercepted.</li>
		<li><b>Prevent client auto DoH</b> - Some clients like Firefox will automatically switch to DNS over HTTPS, bypassing your preferred DNS servers. This option may prevent that.</li>
		<li><b>Use user-entered gateway if WAN is disabled</b> - DHCP will use the IP address of the router as the default gateway on each LAN.</li>
		<li><b>Ignore DHCP requests (...)</b> - Dnsmasq will ignore DHCP requests  to Only MAC addresses listed on the <a href="basic-static.asp">Static DHCP/ARP</a> page won't be able to obtain an IP address through DHCP.</li>
		<li><b>Maximum active DHCP leases</b> - Self-explanatory.</li>
		<li><b>Static lease time</b> - Absolute maximum amount of time allowed for any DHCP lease to be valid.</li>
		<li><b>Custom configuration</b> - Extra options to be added to the Dnsmasq configuration file.</li>
	</ul>

	<i>DHCP Client (WAN):</i><br>
	<ul>
		<li><b>DHCPC Options</b> - Extra options for the DHCP client.</li>
		<li><b>Reduce packet size</b> - Self-explanatory.</li>
	</ul>

	<i>Other relevant notes/hints:</i><br>
	<ul>
		<li>The contents of file /etc/dnsmasq.custom are also added to the end of Dnsmasq's configuration file (if it exists).</li>
	</ul>
</div>

<!-- / / / -->

<div id="footer">
	<span id="footer-msg"></span>
	<input type="button" value="Save" id="save-button" onclick="save()">
	<input type="button" value="Cancel" id="cancel-button" onclick="reloadPage();">
</div>

</td></tr>
</table>
</form>
<script>verifyFields(null, true);</script>
</body>
</html>
