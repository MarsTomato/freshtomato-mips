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
<title>[<% ident(); %>] Admin: Debugging</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<script src="tomato.js"></script>

<script>

//	<% nvram("debug_nocommit,debug_cprintf,debug_cprintf_file,console_loglevel,t_cafree,t_hidelr,debug_ddns,debug_norestart,debug_logsegfault"); %>

function nvramCommit() {
	fields.disableAll('t_fom', 1);
	form.submitHidden('nvcommit.cgi', { '_nextpage': myName() });
}

function clearCache() {
	localStorage.clear();
	alert('Done!');
}

function verifyFields(focused, quiet) {
	return 1;
}

function save() {
	var fom = E('t_fom');
	fom.debug_nocommit.value = fom.f_debug_nocommit.checked ? 1 : 0;
	fom.debug_cprintf.value = fom.f_debug_cprintf.checked ? 1 : 0;
	fom.debug_cprintf_file.value = fom.f_debug_cprintf_file.checked ? 1 : 0;
	fom.t_cafree.value = fom.f_cafree.checked ? 1 : 0;
	fom.t_hidelr.value = fom.f_hidelr.checked ? 1 : 0;
	fom.debug_logsegfault.value = fom.f_debug_logsegfault.checked ? 1 : 0;
	fom.debug_ddns.value = fom.f_debug_ddns.checked ? 1 : 0;

	var a = [];
	if (fom.f_nr_crond.checked) a.push('crond');
	if (fom.f_nr_dnsmasq.checked) a.push('dnsmasq');
	if (fom.f_nr_hotplug2.checked) a.push('hotplug2');
	if (fom.f_nr_igmprt.checked) a.push('igmprt');
	fom.debug_norestart.value = a.join(',');

	if (fom.debug_logsegfault.value != nvram.debug_logsegfault)
		fom._service.value = 'firewall-restart';

	form.submit(fom, 1);
}
</script>
</head>

<body>
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

<input type="hidden" name="_nextpage" value="admin-misc.asp">
<input type="hidden" name="_service" value="">
<input type="hidden" name="debug_nocommit">
<input type="hidden" name="debug_cprintf">
<input type="hidden" name="debug_cprintf_file">
<input type="hidden" name="debug_ddns">
<input type="hidden" name="debug_norestart">
<input type="hidden" name="debug_logsegfault">
<input type="hidden" name="t_cafree">
<input type="hidden" name="t_hidelr">

<!-- / / / -->

<div class="section-title">Debugging</div>
<div class="section" id="debug">
	<script>
		a = [];
		for (i = 1; i <= 8; ++i) a.push([i, i]);
		createFieldTable('', [
			{ title: 'Avoid performing an NVRAM commit', name: 'f_debug_nocommit', type: 'checkbox', value: nvram.debug_nocommit != '0' },
			{ title: 'Enable cprintf output to console', name: 'f_debug_cprintf', type: 'checkbox', value: nvram.debug_cprintf != '0' },
			{ title: 'Enable cprintf output to /tmp/cprintf', name: 'f_debug_cprintf_file', type: 'checkbox', value: nvram.debug_cprintf_file != '0' },
			{ title: 'Enable DDNS output to /tmp/mdu-*', name: 'f_debug_ddns', type: 'checkbox', value: nvram.debug_ddns != '0' },
			{ title: 'Enable segfault logging', name: 'f_debug_logsegfault', type: 'checkbox', value: nvram.debug_logsegfault != '0' },
			{ title: 'Count cache memory and buffers as free memory', name: 'f_cafree', type: 'checkbox', value: nvram.t_cafree == '1' },
			{ title: 'Avoid displaying LAN to router connections', name: 'f_hidelr', type: 'checkbox', value: nvram.t_hidelr == '1' },
			{ title: 'Console log level', name: 'console_loglevel', type: 'select', options: a, value: fixInt(nvram.console_loglevel, 1, 8, 1) },
			{ title: 'Do not restart the following process if they die', multi: [
				{ name: 'f_nr_crond', type: 'checkbox', suffix: ' crond<br>', value: (nvram.debug_norestart.indexOf('crond') != -1) },
				{ name: 'f_nr_dnsmasq', type: 'checkbox', suffix: ' dnsmasq<br>', value: (nvram.debug_norestart.indexOf('dnsmasq') != -1) },
				{ name: 'f_nr_hotplug2', type: 'checkbox', suffix: ' hotplug2<br>', value: (nvram.debug_norestart.indexOf('hotplug2') != -1) },
				{ name: 'f_nr_igmprt', type: 'checkbox', suffix: ' igmprt<br>', value: (nvram.debug_norestart.indexOf('igmprt') != -1) }
			] }
		]);
	</script>

	<br><br>

	&raquo; <a href="clearcookies.asp?_http_id=<% nv(http_id); %>">Clear Cookies</a><br>
	&raquo; <a href="javascript:clearCache()">Clear Cache</a><br>
	&raquo; <a href="javascript:nvramCommit()">NVRAM Commit</a><br>
	&raquo; <a href="javascript:reboot()">Reboot</a><br>
	&raquo; <a href="javascript:shutdown()">Shutdown</a><br>

	<br><br>

	&raquo; <a href="/cfe/cfe.bin?_http_id=<% nv(http_id); %>">Download CFE</a><br>
	&raquo; <a href="/ipt/iptables.txt?_http_id=<% nv(http_id); %>">Download Iptables Dump</a><br>
<!-- IPV6-BEGIN -->
	&raquo; <a href="/ip6t/ip6tables.txt?_http_id=<% nv(http_id); %>">Download Ip6tables Dump</a><br>
<!-- IPV6-END -->
	&raquo; <a href="/logs/syslog.txt?_http_id=<% nv(http_id); %>">Download Logs</a><br>
	&raquo; <a href="/nvram/nvram.txt?_http_id=<% nv(http_id); %>">Download NVRAM Dump</a><br>

	<div class="note-spacer">
		<b>Warning</b>: The NVRAM Dump text file may contain information like wireless encryption keys and usernames/passwords for the router, ISP and DDNS. Please review &amp; edit this file before sharing it with anyone.<br>
	</div>
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
</body>
</html>
