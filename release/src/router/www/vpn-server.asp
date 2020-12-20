<!DOCTYPE html>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	Portions Copyright (C) 2008-2010 Keith Moyer, tomatovpn@keithmoyer.com

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] OpenVPN: Server</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<script src="tomato.js"></script>
<script src="vpn.js"></script>

<script>

//	<% nvram("vpn_server_eas,vpn_server_dns,vpn_server1_poll,vpn_server1_if,vpn_server1_proto,vpn_server1_port,vpn_server1_firewall,vpn_server1_sn,vpn_server1_nm,vpn_server1_local,vpn_server1_remote,vpn_server1_dhcp,vpn_server1_r1,vpn_server1_r2,vpn_server1_crypt,vpn_server1_comp,vpn_server1_digest,vpn_server1_cipher,vpn_server1_ncp_ciphers,vpn_server1_reneg,vpn_server1_hmac,vpn_server1_plan,vpn_server1_plan1,vpn_server1_plan2,vpn_server1_plan3,vpn_server1_ccd,vpn_server1_c2c,vpn_server1_ccd_excl,vpn_server1_ccd_val,vpn_server1_pdns,vpn_server1_rgw,vpn_server1_userpass,vpn_server1_nocert,vpn_server1_users_val,vpn_server1_custom,vpn_server1_static,vpn_server1_ca,vpn_server1_ca_key,vpn_server1_crt,vpn_server1_crl,vpn_server1_key,vpn_server1_dh,vpn_server1_br,vpn_server1_serial,vpn_server2_poll,vpn_server2_if,vpn_server2_proto,vpn_server2_port,vpn_server2_firewall,vpn_server2_sn,vpn_server2_nm,vpn_server2_local,vpn_server2_remote,vpn_server2_dhcp,vpn_server2_r1,vpn_server2_r2,vpn_server2_crypt,vpn_server2_comp,vpn_server2_digest,vpn_server2_cipher,vpn_server2_ncp_ciphers,vpn_server2_reneg,vpn_server2_hmac,vpn_server2_plan,vpn_server2_plan1,vpn_server2_plan2,vpn_server2_plan3,vpn_server2_ccd,vpn_server2_c2c,vpn_server2_ccd_excl,vpn_server2_ccd_val,vpn_server2_pdns,vpn_server2_rgw,vpn_server2_userpass,vpn_server2_nocert,vpn_server2_users_val,vpn_server2_custom,vpn_server2_static,vpn_server2_ca,vpn_server2_ca_key,vpn_server2_crt,vpn_server2_crl,vpn_server2_key,vpn_server2_dh,vpn_server2_br,vpn_server2_serial,lan_ifname,lan1_ifname,lan2_ifname,lan3_ifname"); %>

function CCDGrid() { return this; }
CCDGrid.prototype = new TomatoGrid;

function UsersGrid() {return this;}
UsersGrid.prototype = new TomatoGrid;

tabs = [['server1', 'Server 1'],['server2', 'Server 2']];
sections = [['basic', 'Basic'],['advanced', 'Advanced'],['keys','Keys'],['status','Status']];
ccdTables = [];

usersTables = [];
statusUpdaters = [];
for (i = 0; i < tabs.length; ++i) {
	ccdTables.push(new CCDGrid());
	usersTables.push(new UsersGrid());
	usersTables[i].servername = tabs[i][0];
	statusUpdaters.push(new StatusUpdater());
}

vpnciphers = vpnciphers.concat(['CAMELLIA-128-CBC'],['CAMELLIA-192-CBC'],['CAMELLIA-256-CBC']);
ciphers = [['default','Use Default'],['none','None']];
for (i = 0; i < vpnciphers.length; ++i) ciphers.push([vpnciphers[i],vpnciphers[i]]);
digests = [['default','Use Default'],['none','None']];
for (i = 0; i < vpndigests.length; ++i) digests.push([vpndigests[i],vpndigests[i]]);

changed = 0;
vpn1up = parseInt('<% psup("vpnserver1"); %>');
vpn2up = parseInt('<% psup("vpnserver2"); %>');

function updateStatus(num) {
	var xob = new XmlHttp();
	xob.onCompleted = function(text, xml) {
		statusUpdaters[num].update(text);
		xob = null;
	}
	xob.onError = function(ex) {
		statusUpdaters[num].errors.innerHTML += 'ERROR! '+ex+'<br>';
		xob = null;
	}

	xob.post('/vpnstatus.cgi', 'server='+(num+1));
}

function tabSelect(name) {
	tgHideIcons();

	tabHigh(name);

	for (var i = 0; i < tabs.length; ++i) {
		var on = (name == tabs[i][0]);
		elem.display(tabs[i][0]+'-tab', on);
	}

	cookie.set('vpn_server_tab', name);
}

function sectSelect(tab, section) {
	tgHideIcons();

	for (var i = 0; i < sections.length; ++i) {
		if (section == sections[i][0]) {
			elem.addClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], true);
		}
		else {
			elem.removeClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], false);
		}
	}

	cookie.set('vpn_server'+tab+'_section', section);
}

function toggle(service, isup) {
	if (changed && !confirm("Unsaved changes will be lost. Continue anyway?")) return;

	E('_'+service+'_button').disabled = true;
	form.submitHidden('service.cgi', {
		_redirect: 'vpn-server.asp',
		_sleep: '5',
		_service: service+(isup ? '-stop' : '-start')
	});
}

function verifyFields(focused, quiet) {
	tgHideIcons();

	var ret = 1;

	/* When settings change, make sure we restart the right services */
	if (focused) {
		changed = 1;

		var fom = E('t_fom');
		var serverindex = focused.name.indexOf("server");
		if (serverindex >= 0) {
			var servernumber = focused.name.substring(serverindex+6,serverindex+7);
			if (eval('vpn'+servernumber+'up') && fom._service.value.indexOf('server'+servernumber) < 0) {
				if (fom._service.value != "") fom._service.value += ",";
				fom._service.value += 'vpnserver'+servernumber+'-restart';
			}

			if ((focused.name.indexOf("_dns")>=0 || (focused.name.indexOf("_if")>=0 && E('_f_vpn_server'+servernumber+'_dns').checked)) && fom._service.value.indexOf('dnsmasq') < 0) {
				if (fom._service.value != "")
					fom._service.value += ",";

				fom._service.value += 'dnsmasq-restart';
			}

			if (focused.name.indexOf("_c2c") >= 0)
				ccdTables[servernumber-1].reDraw();
		}
	}

	/* Element varification */
	for (i = 0; i < tabs.length; ++i) {
		t = tabs[i][0];

		if (!v_range('_vpn_'+t+'_poll', quiet, 0, 30)) ret = 0;
		if (!v_port('_vpn_'+t+'_port', quiet)) ret = 0;
		if (!v_ip('_vpn_'+t+'_sn', quiet, 0)) ret = 0;
		if (!v_netmask('_vpn_'+t+'_nm', quiet)) ret = 0;
		if (!v_ip('_vpn_'+t+'_r1', quiet, 1)) ret = 0;
		if (!v_ip('_vpn_'+t+'_r2', quiet, 1)) ret = 0;
		if (!v_ip('_vpn_'+t+'_local', quiet, 1)) ret = 0;
		if (!v_ip('_vpn_'+t+'_remote', quiet, 1)) ret = 0;
		if (!v_range('_vpn_'+t+'_reneg', quiet, -1, 2147483647)) ret = 0;
	}

	/* Visibility changes */
	for (i = 0; i < tabs.length; ++i) {
		t = tabs[i][0];

/* SIZEOPTMORE0-BEGIN */
		if (E('_vpn_'+t+'_crypt').value == 'tls')
			E('_vpn_'+t+'_crypt').value = 'secret';
		E('_vpn_'+t+'_crypt').options[0].disabled = 1;
/* SIZEOPTMORE0-END */
		auth = E('_vpn_'+t+'_crypt').value;
		iface = E('_vpn_'+t+'_if').value;
		hmac = E('_vpn_'+t+'_hmac').value;
		dhcp = E('_f_vpn_'+t+'_dhcp');
		ccd = E('_f_vpn_'+t+'_ccd');
		userpass = E('_f_vpn_'+t+'_userpass');
		dns = E('_f_vpn_'+t+'_dns');
/* SIZEOPTMORE-BEGIN */
		comp = E('_vpn_'+t+'_comp').value;
/* SIZEOPTMORE-END */

		elem.display(PR('_vpn_'+t+'_ca'), PR('_vpn_'+t+'_ca_key'), PR('_vpn_'+t+'_ca_key_div_help'),
/* KEYGEN-BEGIN */
			     PR('_vpn_dhgen_'+t+'_button'),
/* KEYGEN-END */
			     PR('_vpn_'+t+'_crt'), PR('_vpn_'+t+'_crl'), PR('_vpn_'+t+'_dh'),
			     PR('_vpn_'+t+'_key'), PR('_vpn_'+t+'_hmac'), PR('_f_vpn_'+t+'_rgw'),
			     PR('_vpn_'+t+'_reneg'), auth == "tls");
		elem.display(PR('_vpn_'+t+'_static'), auth == "secret" || (auth == "tls" && hmac >= 0));
		elem.display(PR('_vpn_keygen_static_'+t+'_button'), auth == "secret" || (auth == "tls" && hmac >= 0));
		elem.display(E(t+'_custom_crypto_text'), auth == "custom");
/* KEYGEN-BEGIN */
		elem.display(PR('_vpn_keygen_'+t+'_button'), auth == "tls");
/* KEYGEN-END */
		elem.display(PR('_vpn_'+t+'_sn'), PR('_f_vpn_'+t+'_plan'), PR('_f_vpn_'+t+'_plan1'),
		             PR('_f_vpn_'+t+'_plan2'), PR('_f_vpn_'+t+'_plan3'), auth == "tls" && iface == "tun");
		elem.display(PR('_f_vpn_'+t+'_dhcp'), auth == "tls" && iface == "tap");
		elem.display(PR('_vpn_'+t+'_br'), iface == "tap");
		elem.display(E(t+'_range'), !dhcp.checked);
		elem.display(PR('_vpn_'+t+'_local'), auth == "secret" && iface == "tun");
		elem.display(PR('_f_vpn_'+t+'_ccd'), auth == "tls");
		elem.display(PR('_f_vpn_'+t+'_userpass'), auth == "tls");
		elem.display(PR('_f_vpn_'+t+'_nocert'),PR('table_'+t+'_users'), auth == "tls" && userpass.checked);
		elem.display(PR('_f_vpn_'+t+'_c2c'),PR('_f_vpn_'+t+'_ccd_excl'),PR('table_'+t+'_ccd'), auth == "tls" && ccd.checked);
		elem.display(PR('_f_vpn_'+t+'_pdns'), auth == "tls" && dns.checked );
		elem.display(PR('_vpn_'+t+'_ncp_ciphers'), auth == "tls");
		elem.display(PR('_vpn_'+t+'_cipher'), auth == "secret");
/* KEYGEN-BEGIN */
		elem.display(PR('_vpn_client_gen_'+t+'_button'), auth != "custom");
		elem.display(PR('_vpn_'+t+'_serial'), auth == "tls");
/* KEYGEN-END */

		keyHelp = E(t+'-keyhelp');
		switch (auth) {
		case "tls":
			keyHelp.href = helpURL['TLSKeys'];
		break;
		case "secret":
			keyHelp.href = helpURL['staticKeys'];
		break;
		default:
			keyHelp.href = helpURL['howto'];
		break;
		}
	}

	var bridge1 = E('_vpn_server1_br');
	if (nvram.lan_ifname.length < 1)
		bridge1.options[0].disabled = true;
	if (nvram.lan1_ifname.length < 1)
		bridge1.options[1].disabled = true;
	if (nvram.lan2_ifname.length < 1)
		bridge1.options[2].disabled = true;
	if (nvram.lan3_ifname.length < 1)
		bridge1.options[3].disabled = true;

	var bridge2 = E('_vpn_server2_br');
	if (nvram.lan_ifname.length < 1)
		bridge2.options[0].disabled = true;
	if (nvram.lan1_ifname.length < 1)
		bridge2.options[1].disabled = true;
	if (nvram.lan2_ifname.length < 1)
		bridge2.options[2].disabled = true;
	if (nvram.lan3_ifname.length < 1)
		bridge2.options[3].disabled = true;

	<!-- disable and un-check push lanX (*_plan) if lanX_ifname length < 1 -->
	if (nvram.lan_ifname.length < 1) {
		E('_f_vpn_server1_plan').checked = false;
		E('_f_vpn_server2_plan').checked = false;
		E('_f_vpn_server1_plan').disabled = true;
		E('_f_vpn_server2_plan').disabled = true;
	}
	if (nvram.lan1_ifname.length < 1) {
		E('_f_vpn_server1_plan1').checked = false;
		E('_f_vpn_server2_plan1').checked = false;
		E('_f_vpn_server1_plan1').disabled = true;
		E('_f_vpn_server2_plan1').disabled = true;
	}
	if (nvram.lan2_ifname.length < 1) {
		E('_f_vpn_server1_plan2').checked = false;
		E('_f_vpn_server2_plan2').checked = false;
		E('_f_vpn_server1_plan2').disabled = true;
		E('_f_vpn_server2_plan2').disabled = true;
	}
	if (nvram.lan3_ifname.length < 1) {
		E('_f_vpn_server1_plan3').checked = false;
		E('_f_vpn_server2_plan3').checked = false;
		E('_f_vpn_server1_plan3').disabled = true;
		E('_f_vpn_server2_plan3').disabled = true;
	}

	return ret;
}

CCDGrid.prototype.verifyFields = function(row, quiet) {
	var ret = 1;

	/* When settings change, make sure we restart the right server */
	var fom = E('t_fom');
	var servernum = 1;
	for (i = 0; i < tabs.length; ++i) {
		if (ccdTables[i] == this) {
			servernum = i+1;
			if (eval('vpn'+(i+1)+'up') && fom._service.value.indexOf('server'+(i + 1)) < 0) {
				if (fom._service.value != "")
					fom._service.value += ",";

				fom._service.value += 'vpnserver'+(i+1)+'-restart';
			}
		}
	}

	var f = fields.getAll(row);

	/* Verify fields in this row of the table */
	if (f[1].value == "") { ferror.set(f[1], "Common name is mandatory.", quiet); ret = 0; }
	if (f[1].value.indexOf('>') >= 0 || f[1].value.indexOf('<') >= 0) { ferror.set(f[1], "Common name cannot contain '<' or '>' characters.", quiet); ret = 0; }
	if (f[2].value != "" && !v_ip(f[2],quiet,0)) ret = 0;
	if (f[3].value != "" && !v_netmask(f[3],quiet)) ret = 0;
	if (f[2].value == "" && f[3].value != "" ) { ferror.set(f[2], "Either both or neither subnet and netmask must be provided.", quiet); ret = 0; }
	if (f[3].value == "" && f[2].value != "" ) { ferror.set(f[3], "Either both or neither subnet and netmask must be provided.", quiet); ret = 0; }
	if (f[4].checked && (f[2].value == "" || f[3].value == "")) { ferror.set(f[4], "Cannot push routes if they're not given. Please provide subnet/netmask.", quiet); ret = 0; }

	return ret;
}

CCDGrid.prototype.fieldValuesToData = function(row) {
	var f = fields.getAll(row);

	return [f[0].checked?1:0, f[1].value, f[2].value, f[3].value, f[4].checked?1:0];
}

CCDGrid.prototype.dataToView = function(data) {
	var c2c = false;
	for (i = 0; i < tabs.length; ++i) {
		if (ccdTables[i] == this && E('_f_vpn_server'+(i+1)+'_c2c').checked)
			c2c = true;
	}

	var temp = ['<input type="checkbox" disabled'+(data[0] !=0 ? ' checked' : '')+'>', data[1], data[2], data[3], c2c ? '<input type="checkbox" disabled'+(data[4] != 0 ? ' checked' : '')+'>' : 'N/A'];

	var v = [];
	for (var i = 0; i < temp.length; ++i)
		v.push((i==0 || i==4) ? temp[i] : escapeHTML(''+temp[i]));

	return v;
}

CCDGrid.prototype.dataToFieldValues = function(data) {
	return [data[0] == 1, data[1], data[2], data[3], data[4] == 1];
}

CCDGrid.prototype.reDraw = function() {
	var i, j, header, data, view;
	data = this.getAllData();
	header = this.header ? this.header.rowIndex + 1 : 0;
	for (i = 0; i < data.length; ++i) {
		view = this.dataToView(data[i]);
		for (j = 0; j < view.length; ++j)
			this.tb.rows[i+header].cells[j].innerHTML = view[j];
	}
}

UsersGrid.prototype.verifyFields = function(row, quiet) {
	var ret = 1;
	var fom = E('t_fom');
	var servernum = 1;
	for (i = 0; i < tabs.length; ++i) {
		if (usersTables[i] == this) {
			servernum = i+1;
			if (eval('vpn'+(i+1)+'up') && fom._service.value.indexOf('server'+(i+1)) < 0) {
				if (fom._service.value != "")
					fom._service.value += ",";

				fom._service.value += 'vpnserver'+(i+1)+'-restart';
			}
		}
	}
	var f = fields.getAll(row);

	/* Verify fields in this row of the table */
	if (f[1].value == "") { ferror.set(f[1], "username is mandatory.", quiet); ret = 0; }
	if (f[1].value.indexOf('>') >= 0 || f[1].value.indexOf('<') >= 0) { ferror.set(f[1], "username cannot contain '<' or '>' characters.", quiet); ret = 0; }
	if (f[2].value == "" ) { ferror.set(f[2], "password is mandatory.", quiet); ret = 0; }
	if (f[2].value.indexOf('>') >= 0 || f[1].value.indexOf('<') >= 0) { ferror.set(f[2], "password cannot contain '<' or '>' characters.", quiet); ret = 0; }

	return ret;
}

UsersGrid.prototype.fieldValuesToData = function(row) {
	var f = fields.getAll(row);

	return [f[0].checked?1:0, f[1].value, f[2].value];
}

UsersGrid.prototype.dataToView = function(data) {
	var temp = ['<input type="checkbox" disabled'+(data[0]!=0?' checked':'')+'>', data[1], 'Secret'];

	var v = [];
	for (var i = 0; i < temp.length; ++i) {
		v.push(i==0 ? temp[i] : escapeHTML(''+temp[i]));
	}

	return v;
}

UsersGrid.prototype.dataToFieldValues = function(data) {
	return [data[0] == 1, data[1], data[2]];
}

function save() {
	if (!verifyFields(null, false)) return;

	var fom = E('t_fom');

	E('vpn_server_eas').value = '';
	E('vpn_server_dns').value = '';

	for (i = 0; i < tabs.length; ++i) {
		if (ccdTables[i].isEditing()) return;
		if (usersTables[i].isEditing()) return;

		t = tabs[i][0];

/* SIZEOPTMORE-BEGIN */
		var crypt = E('_vpn_'+t+'_crypt').value;
		var hmac = E('_vpn_'+t+'_hmac').value;
		var key = E('_vpn_'+t+'_static').value;
		if (((crypt == 'secret' || (crypt == 'tls' && hmac >= 0 && hmac < 4)) && key.indexOf('OpenVPN Static key V1') === -1) ||
		    (crypt == 'tls' && hmac == 4 && key.indexOf('OpenVPN tls-crypt-v2') === -1)) {
			alert('Keys->Static Key is in the wrong format for the selected Auth Method - Re-Generate it!');
			return;
		}
/* SIZEOPTMORE-END */

		if (E('_f_vpn_'+t+'_eas').checked)
			E('vpn_server_eas').value += ''+(i+1)+',';

		if (E('_f_vpn_'+t+'_dns').checked)
			E('vpn_server_dns').value += ''+(i+1)+',';

		var data = ccdTables[i].getAllData();
		var ccd = '';

		for (j = 0; j < data.length; ++j)
			ccd += data[j].join('<')+'>';
		var userdata = usersTables[i].getAllData();
		var users = '';
		for (j = 0; j < userdata.length; ++j)
			users += userdata[j].join('<')+'>';

		E('vpn_'+t+'_dhcp').value = E('_f_vpn_'+t+'_dhcp').checked ? 1 : 0;
		E('vpn_'+t+'_plan').value = E('_f_vpn_'+t+'_plan').checked ? 1 : 0;
		E('vpn_'+t+'_plan1').value = E('_f_vpn_'+t+'_plan1').checked ? 1 : 0;
		E('vpn_'+t+'_plan2').value = E('_f_vpn_'+t+'_plan2').checked ? 1 : 0;
		E('vpn_'+t+'_plan3').value = E('_f_vpn_'+t+'_plan3').checked ? 1 : 0;
		E('vpn_'+t+'_ccd').value = E('_f_vpn_'+t+'_ccd').checked ? 1 : 0;
		E('vpn_'+t+'_c2c').value = E('_f_vpn_'+t+'_c2c').checked ? 1 : 0;
		E('vpn_'+t+'_ccd_excl').value = E('_f_vpn_'+t+'_ccd_excl').checked ? 1 : 0;
		E('vpn_'+t+'_ccd_val').value = ccd;
		E('vpn_'+t+'_userpass').value = E('_f_vpn_'+t+'_userpass').checked ? 1 : 0;
		E('vpn_'+t+'_nocert').value = E('_f_vpn_'+t+'_nocert').checked ? 1 : 0;
		E('vpn_'+t+'_users_val').value = users;
		E('vpn_'+t+'_pdns').value = E('_f_vpn_'+t+'_pdns').checked ? 1 : 0;
		E('vpn_'+t+'_rgw').value = E('_f_vpn_'+t+'_rgw').checked ? 1 : 0;
	}

	form.submit(fom, 1);

	changed = 0;
}

function init() {
	tabSelect(cookie.get('vpn_server_tab') || tabs[0][0]);

	for (i = 0; i < tabs.length; ++i) {
		sectSelect(i, cookie.get('vpn_server'+i+'_section') || sections[0][0]);

		t = tabs[i][0];

		ccdTables[i].init('table_'+t+'_ccd', 'sort', 0, [{ type: 'checkbox' }, { type: 'text', maxlen: 15 }, { type: 'text', maxlen: 15 }, { type: 'text', maxlen: 15 }, { type: 'checkbox' }]);
		ccdTables[i].headerSet(['Enable', 'Common Name', 'Subnet', 'Netmask', 'Push']);
		var ccdVal = eval('nvram.vpn_'+t+'_ccd_val');
		if (ccdVal.length) {
			var s = ccdVal.split('>');
			for (var j = 0; j < s.length; ++j) {
				if (!s[j].length) continue;
				var row = s[j].split('<');
				if (row.length == 5)
					ccdTables[i].insertData(-1, row);
			}
		}
		ccdTables[i].showNewEditor();
		ccdTables[i].resetNewEditor();

		usersTables[i].init('table_'+t+'_users','sort', 0, [{ type: 'checkbox' }, { type: 'text', maxlen: 25 }, { type: 'text', maxlen: 15 }]);
		usersTables[i].headerSet(['Enable', 'Username', 'Password']);
		var usersVal = eval('nvram.vpn_'+t+'_users_val');
		if (usersVal.length) {
			var s = usersVal.split('>');
			for (var j = 0; j < s.length; ++j) {
				if (!s[j].length) continue;
				var row = s[j].split('<');
				if (row.length == 3)
					usersTables[i].insertData(-1, row);
			}
		}
		usersTables[i].showNewEditor();
		usersTables[i].resetNewEditor();

		statusUpdaters[i].init(t+'-status-clients-table',t+'-status-routing-table',t+'-status-stats-table',t+'-status-time',t+'-status-content',t+'-no-status',t+'-status-errors');
		updateStatus(i);
	}

	verifyFields(null, true);
}

var keyGenRequest = null

function updateStaticKey(serverNumber) {
	if (keyGenRequest) return;
	disableKeyButtons(serverNumber, true);
	changed = 1;
	elem.display(E('server'+serverNumber+'_static_progress_div'), true);
	keyGenRequest = new XmlHttp();

	keyGenRequest.onCompleted = function(text, xml) {
		E('_vpn_server'+serverNumber+'_static').value = text;
		keyGenRequest = null;
		elem.display(E('server'+serverNumber+'_static_progress_div'), false);
		disableKeyButtons(serverNumber, false);
	}
	keyGenRequest.onError = function(ex) { keyGenRequest = null; }
	var crypt = E('_vpn_server'+serverNumber+'_crypt').value;
	var hmac = E('_vpn_server'+serverNumber+'_hmac').value;
	keyGenRequest.post('vpngenkey.cgi', '_mode='+((crypt == 'tls' && hmac == 4) ? 'static2' : 'static1')+'&_server='+serverNumber);
}

/* KEYGEN-BEGIN */
function generateDHParams(serverNumber) {
	if (keyGenRequest) return;
	if (confirm('WARNING: DH Parameters generation can take a long time.\nIf it freezes, refresh the page and try again.\n\nDo you want to proceed?')) {
		changed = 1;
		disableKeyButtons(serverNumber, true);
		elem.display(E('server'+serverNumber+'_dh_progress_div'), true);
		keyGenRequest = new XmlHttp();

		keyGenRequest.onCompleted = function(text, xml) {
			E('_vpn_server'+serverNumber+'_dh').value = text;
			keyGenRequest = null;
			elem.display(E('server'+serverNumber+'_dh_progress_div'), false);
			disableKeyButtons(serverNumber, false);
		}
		keyGenRequest.onError = function(ex) { keyGenRequest = null; }
		keyGenRequest.post('vpngenkey.cgi', '_mode=dh');
	}
}

function generateKeys(serverNumber) {
	if (keyGenRequest) return;
	changed = 1;
	var caKeyTextArea = E('_vpn_server'+serverNumber+'_ca_key');
	var doGeneration = true;
	if (caKeyTextArea.value == "") {
		doGeneration = confirm("WARNING: You haven't provided Certificate Authority Key.\n" +
				       "This means, that CA Key needs to be regenerated, but it WILL break ALL your existing client certificates.\n" +
				       "You will need to reconfigure all your existing VPN clients!\n Are you sure to continue?");
	}
	if (doGeneration) {
		disableKeyButtons(serverNumber,true);
		showTLSProgressDivs(serverNumber,true);
		var cakey, cacert, generated_crt, generated_key;
		keyGenRequest = new XmlHttp();

		keyGenRequest.onCompleted = function(text, xml) {
			eval(text);
			E('_vpn_server'+serverNumber+'_ca_key').value = cakey;
			E('_vpn_server'+serverNumber+'_ca').value = cacert;
			E('_vpn_server'+serverNumber+'_crt').value = generated_crt;
			E('_vpn_server'+serverNumber+'_key').value = generated_key;
			keyGenRequest = null;
			disableKeyButtons(serverNumber,false);
			showTLSProgressDivs(serverNumber,false);
		}
		keyGenRequest.onError = function(ex) { keyGenRequest = null; }
		keyGenRequest.post('vpngenkey.cgi', '_mode=key&_server='+serverNumber);
	}
}
/* KEYGEN-END */

function disableKeyButtons(serverNumber, state) {
	E('_vpn_keygen_static_server'+serverNumber+'_button').disabled = state;
/* KEYGEN-BEGIN */
	E('_vpn_keygen_server'+serverNumber+'_button').disabled = state;
	E('_vpn_dhgen_server'+serverNumber+'_button').disabled = state;
/* KEYGEN-END */
}

function showTLSProgressDivs(serverNumber, state) {
	elem.display(E('server'+serverNumber+'_key_progress_div'), state);
/* KEYGEN-BEGIN */
	elem.display(E('server'+serverNumber+'_cert_progress_div'), state);
	elem.display(E('server'+serverNumber+'_ca_progress_div'), state);
	elem.display(E('server'+serverNumber+'_ca_key_progress_div'), state);
/* KEYGEN-END */
}
/* KEYGEN-BEGIN */

function downloadClientConfig(serverNumber) {
	if (keyGenRequest) return;
	var warn = 0;
	var caKey = E('_vpn_server'+serverNumber+'_ca_key').value;
	var ca = E('_vpn_server'+serverNumber+'_ca').value;
	var serverCrt = E('_vpn_server'+serverNumber+'_crt').value;
	var serverCrtKey = E('_vpn_server'+serverNumber+'_key').value;
	var staticKey = E('_vpn_server'+serverNumber+'_static').value;
	var crypt = E('_vpn_server'+serverNumber+'_crypt').value;
	var hmac = E('_vpn_server'+serverNumber+'_hmac').value;

	if (crypt == 'secret') {
		if (staticKey == "") warn = 1;
	}
	else if (crypt == 'tls' && hmac >= 0) {
		if (staticKey == "" || caKey == "" || ca == "" || serverCrt == "" || serverCrtKey == "") warn = 1;
	}
	else {
		if (caKey == "" || ca == "" || serverCrt == "" || serverCrtKey == "") warn = 1;
	}

	if (warn) {
		alert("Not all key fields have been entered!");
		return;
	}
	if (changed) {
		alert("Changes has been made. You need to save before continue!");
		return;
	}
	elem.display(E('server'+serverNumber+'_gen_progress_div'), true);
	keyGenRequest = new XmlHttp();
	keyGenRequest.onCompleted = function(text, xml) {
		elem.display(E('server'+serverNumber+'_gen_progress_div'), false);
		keyGenRequest = null;

		var downloadedFileFakeLink = document.createElement('a');
		downloadedFileFakeLink.setAttribute('href', 'data:application/tomato-binary-file,'+encodeURIComponent(text));
		downloadedFileFakeLink.setAttribute('download', 'ClientConfig.tgz');

		downloadedFileFakeLink.style.display = 'none';
		document.body.appendChild(downloadedFileFakeLink);

		downloadedFileFakeLink.click();

		document.body.removeChild(downloadedFileFakeLink);
	}
	keyGenRequest.onError = function(ex) { keyGenRequest = null; }
	keyGenRequest.responseType = 'blob';
	keyGenRequest.get('vpn/ClientConfig.tgz','_server='+serverNumber);
}
/* KEYGEN-END */
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

<input type="hidden" name="_nextpage" value="vpn-server.asp">
<input type="hidden" name="_nextwait" value="5">
<input type="hidden" name="_service" value="">
<input type="hidden" name="vpn_server_eas" id="vpn_server_eas" value="">
<input type="hidden" name="vpn_server_dns" id="vpn_server_dns" value="">

<!-- / / / -->

<div class="section-title">OpenVPN Server Configuration</div>
<div class="section">
	<script>
		tabCreate.apply(this, tabs);

		for (i = 0; i < tabs.length; ++i) {
			t = tabs[i][0];
			W('<div id="'+t+'-tab">');
			W('<input type="hidden" id="vpn_'+t+'_dhcp" name="vpn_'+t+'_dhcp">');
			W('<input type="hidden" id="vpn_'+t+'_plan" name="vpn_'+t+'_plan">');
			W('<input type="hidden" id="vpn_'+t+'_plan1" name="vpn_'+t+'_plan1">');
			W('<input type="hidden" id="vpn_'+t+'_plan2" name="vpn_'+t+'_plan2">');
			W('<input type="hidden" id="vpn_'+t+'_plan3" name="vpn_'+t+'_plan3">');
			W('<input type="hidden" id="vpn_'+t+'_ccd" name="vpn_'+t+'_ccd">');
			W('<input type="hidden" id="vpn_'+t+'_c2c" name="vpn_'+t+'_c2c">');
			W('<input type="hidden" id="vpn_'+t+'_ccd_excl" name="vpn_'+t+'_ccd_excl">');
			W('<input type="hidden" id="vpn_'+t+'_ccd_val" name="vpn_'+t+'_ccd_val">');
			W('<input type="hidden" id="vpn_'+t+'_userpass" name="vpn_'+t+'_userpass">');
			W('<input type="hidden" id="vpn_'+t+'_nocert" name="vpn_'+t+'_nocert">');
			W('<input type="hidden" id="vpn_'+t+'_users_val" name="vpn_'+t+'_users_val">');
			W('<input type="hidden" id="vpn_'+t+'_pdns" name="vpn_'+t+'_pdns">');
			W('<input type="hidden" id="vpn_'+t+'_rgw" name="vpn_'+t+'_rgw">');

			W('<ul class="tabs">');
			for (j = 0; j < sections.length; j++) {
				W('<li><a href="javascript:sectSelect('+i+',\''+sections[j][0]+'\')" id="'+t+'-'+sections[j][0]+'-tab">'+sections[j][1]+'<\/a><\/li>');
			}
			W('<\/ul><div class="tabs-bottom"><\/div>');

			W('<div id="'+t+'-basic">');
			createFieldTable('', [
				{ title: 'Start with WAN', name: 'f_vpn_'+t+'_eas', type: 'checkbox', value: nvram.vpn_server_eas.indexOf(''+(i+1)) >= 0 },
			{ title: 'Interface Type', name: 'vpn_'+t+'_if', type: 'select', options: [['tap','TAP'], ['tun','TUN']], value: eval('nvram.vpn_'+t+'_if') },
				{ title: 'Bridge TAP with', indent: 2, name: 'vpn_'+t+'_br', type: 'select', options: [
					['br0','LAN (br0)*'],
					['br1','LAN1 (br1)'],
					['br2','LAN2 (br2)'],
					['br3','LAN3 (br3)']
					], value: eval ('nvram.vpn_'+t+'_br'), suffix: ' <small>* default<\/small> ' },
				{ title: 'Protocol', name: 'vpn_'+t+'_proto', type: 'select', options: [['udp','UDP'], ['tcp-server','TCP'], ['udp4','UDP4'], ['tcp4-server','TCP4'], ['udp6','UDP6'], ['tcp6-server','TCP6']], value: eval('nvram.vpn_'+t+'_proto') },
				{ title: 'Port', name: 'vpn_'+t+'_port', type: 'text', maxlen: 5, size: 10, value: eval('nvram.vpn_'+t+'_port') },
				{ title: 'Firewall', name: 'vpn_'+t+'_firewall', type: 'select', options: [['auto', 'Automatic'], ['external', 'External Only'], ['custom', 'Custom']], value: eval('nvram.vpn_'+t+'_firewall') },
				{ title: 'Authorization Mode', name: 'vpn_'+t+'_crypt', type: 'select', options: [['tls', 'TLS'], ['secret', 'Static Key'], ['custom', 'Custom']], value: eval('nvram.vpn_'+t+'_crypt'),
					suffix: '<span id="'+t+'_custom_crypto_text">&nbsp;<small>(must configure manually...)<\/small><\/span>' },
				{ title: 'TLS control channel security <small>(tls-auth/tls-crypt)<\/small>', name: 'vpn_'+t+'_hmac', type: 'select', options: [[-1, 'Disabled'], [2, 'Bi-directional Auth'], [0, 'Incoming Auth (0)'], [1, 'Outgoing Auth (1)'], [3, 'Encrypt Channel']
/* SIZEOPTMORE-BEGIN */
				          , [4, 'Encrypt Channel V2']
/* SIZEOPTMORE-END */
				          ], value: eval('nvram.vpn_'+t+'_hmac') },
				{ title: 'Auth digest', name: 'vpn_'+t+'_digest', type: 'select', options: digests, value: eval('nvram.vpn_'+t+'_digest') },
				{ title: 'VPN subnet/netmask', multi: [
					{ name: 'vpn_'+t+'_sn', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_sn') },
					{ name: 'vpn_'+t+'_nm', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_nm') } ] },
				{ title: 'Client address pool', multi: [
					{ name: 'f_vpn_'+t+'_dhcp', type: 'checkbox', value: eval('nvram.vpn_'+t+'_dhcp') != 0, suffix: ' DHCP ' },
					{ name: 'vpn_'+t+'_r1', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_r1'), prefix: '<span id="'+t+'_range">', suffix: '-' },
					{ name: 'vpn_'+t+'_r2', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_r2'), suffix: '<\/span>' } ] },
				{ title: 'Local/remote endpoint addresses', multi: [
					{ name: 'vpn_'+t+'_local', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_local') },
					{ name: 'vpn_'+t+'_remote', type: 'text', maxlen: 15, size: 17, value: eval('nvram.vpn_'+t+'_remote') } ] }
			]);
			W('<\/div>');

			W('<div id="'+t+'-advanced">');
			createFieldTable('', [
				{ title: 'Poll Interval', name: 'vpn_'+t+'_poll', type: 'text', maxlen: 2, size: 5, value: eval('nvram.vpn_'+t+'_poll'), suffix: '&nbsp;<small>(in minutes, 0 to disable)<\/small>' },
				{ title: 'Push LAN (br0) to clients', name: 'f_vpn_'+t+'_plan', type: 'checkbox', value: eval('nvram.vpn_'+t+'_plan') != 0 },
				{ title: 'Push LAN1 (br1) to clients', name: 'f_vpn_'+t+'_plan1', type: 'checkbox', value: eval('nvram.vpn_'+t+'_plan1') != 0 },
				{ title: 'Push LAN2 (br2) to clients', name: 'f_vpn_'+t+'_plan2', type: 'checkbox', value: eval('nvram.vpn_'+t+'_plan2') != 0 },
				{ title: 'Push LAN3 (br3) to clients', name: 'f_vpn_'+t+'_plan3', type: 'checkbox', value: eval('nvram.vpn_'+t+'_plan3') != 0 },
				{ title: 'Direct clients to<br>redirect Internet traffic', name: 'f_vpn_'+t+'_rgw', type: 'checkbox', value: eval('nvram.vpn_'+t+'_rgw') != 0 },
				{ title: 'Respond to DNS', name: 'f_vpn_'+t+'_dns', type: 'checkbox', value: nvram.vpn_server_dns.indexOf(''+(i+1)) >= 0 },
				{ title: 'Advertise DNS to clients', name: 'f_vpn_'+t+'_pdns', type: 'checkbox', value: eval('nvram.vpn_'+t+'_pdns') != 0 },
				{ title: 'Data ciphers', name: 'vpn_'+t+'_ncp_ciphers', type: 'text', size: 70, maxlen: 127, value: eval('nvram.vpn_'+t+'_ncp_ciphers') },
				{ title: 'Cipher', name: 'vpn_'+t+'_cipher', type: 'select', options: ciphers, value: eval('nvram.vpn_'+t+'_cipher') },
				{ title: 'Compression', name: 'vpn_'+t+'_comp', type: 'select', options: [['-1', 'Disabled'], ['no', 'None'], ['yes', 'LZO'], ['adaptive', 'LZO Adaptive']
/* SIZEOPTMORE-BEGIN */
				         , ['lz4', 'LZ4'], ['lz4-v2', 'LZ4-V2']
/* SIZEOPTMORE-END */
				         ], value: eval('nvram.vpn_'+t+'_comp') },
				{ title: 'TLS Renegotiation Time', name: 'vpn_'+t+'_reneg', type: 'text', maxlen: 10, size: 7, value: eval('nvram.vpn_'+t+'_reneg'),
					suffix: '&nbsp;<small>(in seconds, -1 for default)<\/small>' },
				{ title: 'Manage Client-Specific Options', name: 'f_vpn_'+t+'_ccd', type: 'checkbox', value: eval('nvram.vpn_'+t+'_ccd') != 0 },
				{ title: 'Allow Client<->Client', name: 'f_vpn_'+t+'_c2c', type: 'checkbox', value: eval('nvram.vpn_'+t+'_c2c') != 0 },
				{ title: 'Allow Only These Clients', name: 'f_vpn_'+t+'_ccd_excl', type: 'checkbox', value: eval('nvram.vpn_'+t+'_ccd_excl') != 0 },
				{ title: '', suffix: '<div class="tomato-grid" id="table_'+t+'_ccd"><\/div>' },
				{ title: 'Allow User/Pass Auth', name: 'f_vpn_'+t+'_userpass', type: 'checkbox', value: eval('nvram.vpn_'+t+'_userpass') != 0 },
				{ title: 'Allow Only User/Pass (without cert) Auth', name: 'f_vpn_'+t+'_nocert', type: 'checkbox', value: eval('nvram.vpn_'+t+'_nocert') != 0 },
				{ title: '', suffix: '<div class="tomato-grid" id="table_'+t+'_users"><\/div>' },
				{ title: 'Custom Configuration', name: 'vpn_'+t+'_custom', type: 'textarea', value: eval('nvram.vpn_'+t+'_custom') }
			]);
			W('<\/div>');

			W('<div id="'+t+'-keys">');
			W('<p class="vpn-keyhelp">For help generating keys, refer to the OpenVPN <a id="'+t+'-keyhelp">HOWTO<\/a>. All 6 keys take about 14kB of NVRAM, so check first if there is enough free space!<\/p>');
			createFieldTable('', [
				{ title: 'Static Key', name: 'vpn_'+t+'_static', type: 'textarea', value: eval('nvram.vpn_'+t+'_static'),
					prefix: '<div id="'+t+'_static_progress_div" style="display:none"><p class="keyhelp">Please wait while we\'re generating static key...<img src="spin.gif" alt=""><\/p><\/div>' },
				{ title: '', custom: '<input type="button" value="Generate static key" onclick="updateStaticKey('+(i+1)+')" id="_vpn_keygen_static_'+t+'_button">' },
				{ title: 'Certificate Authority Key', name: 'vpn_'+t+'_ca_key', type: 'textarea', value: eval('nvram.vpn_'+t+'_ca_key'),
/* KEYGEN-BEGIN */
					prefix: '<div id="'+t+'_ca_key_progress_div" style="display:none"><p class="keyhelp">Please wait while we\'re generating CA key...<img src="spin.gif" alt=""><\/p><\/div>'
/* KEYGEN-END */
				},
				{ title: '', custom: '<div id="_vpn_'+t+'_ca_key_div_help"><p class="keyhelp">Optional, only used for client certificate generation.<br>Uncrypted (-nodes) private keys are supported.<\/p><\/div>' },
				{ title: 'Certificate Authority', name: 'vpn_'+t+'_ca', type: 'textarea', value: eval('nvram.vpn_'+t+'_ca'),
					prefix: '<div id="'+t+'_ca_progress_div" style="display:none"><p class="keyhelp">Please wait while we\'re generating CA certificate...<img src="spin.gif" alt=""><\/p><\/div>' },
				{ title: 'Server Certificate', name: 'vpn_'+t+'_crt', type: 'textarea', value: eval('nvram.vpn_'+t+'_crt'),
/* KEYGEN-BEGIN */
					prefix: '<div id="'+t+'_cert_progress_div" style="display: none"><p class="keyhelp">Please wait while we\'re generating certificate...<img src="spin.gif" alt=""><\/p><\/div>'
/* KEYGEN-END */
				},
				{ title: 'Server Key', name: 'vpn_'+t+'_key', type: 'textarea', value: eval('nvram.vpn_'+t+'_key'),
/* KEYGEN-BEGIN */
					prefix: '<div id="'+t+'_key_progress_div" style="display: none"><p class="keyhelp">Please wait while we\'re generating key...<img src="spin.gif" alt=""><\/p><\/div>'
/* KEYGEN-END */
				},
				{ title: 'CRL file', name: 'vpn_'+t+'_crl', type: 'textarea', value: eval('nvram.vpn_'+t+'_crl') },
/* KEYGEN-BEGIN */
				{ title: '', custom: '<input type="button" value="Generate keys" onclick="generateKeys('+(i+1)+')" id="_vpn_keygen_'+t+'_button">' },
/* KEYGEN-END */
				{ title: 'Diffie Hellman parameters', name: 'vpn_'+t+'_dh', type: 'textarea', value: eval('nvram.vpn_'+t+'_dh'),
/* KEYGEN-BEGIN */
					prefix: '<div id="'+t+'_dh_progress_div" style="display:none"><p class="keyhelp">Please wait while we\'re generating DH parameters...<img src="spin.gif" alt=""><\/p><\/div>' },
				{ title: '', custom: '<input type="button" value="Generate DH Params" onclick="generateDHParams('+(i+1)+')" id="_vpn_dhgen_'+t+'_button">' },
				null,
				{ title: 'Serial number', name: 'vpn_'+t+'_serial', type: 'text', maxlen: 2, size: 2, value: eval('nvram.vpn_'+t+'_serial') },
				{ title: '', custom: '<input type="button" value="Generate client config" onclick="downloadClientConfig('+(i+1)+')" id="_vpn_client_gen_'+t+'_button">',
					suffix: '<div id="'+t+'_gen_progress_div" style="display:none"><p class="keyhelp">Please wait while your configuration is being generated...<img src="spin.gif" alt=""><\/p><\/div>'
/* KEYGEN-END */
				}
			]);
			W('<\/div>');

			W('<div id="'+t+'-status">');
			W('<div id="'+t+'-no-status"><p>Server is not running or status could not be read.<\/p><\/div>');
			W('<div id="'+t+'-status-content" style="display:none" class="status-content">');
			W('<div id="'+t+'-status-header" class="vpn-status-header"><p>Data current as of <span id="'+t+'-status-time"><\/span>.<\/p><\/div>');
			W('<div id="'+t+'-status-clients"><div class="section-title">Client List<\/div><div class="tomato-grid vpn-status-table" id="'+t+'-status-clients-table"><\/div><br><\/div>');
			W('<div id="'+t+'-status-routing"><div class="section-title">Routing Table<\/div><div class="tomato-grid vpn-status-table" id="'+t+'-status-routing-table"><\/div><br><\/div>');
			W('<div id="'+t+'-status-stats"><div class="section-title">General Statistics<\/div><div class="tomato-grid vpn-status-table" id="'+t+'-status-stats-table"><\/div><br><\/div>');
			W('<div id="'+t+'-status-errors" class="error"><\/div>');
			W('<\/div>');
			W('<div class="vpn-refresh"><a href="javascript:updateStatus('+i+')">Refresh Status<\/a><\/div>');
			W('<\/div>');
			W('<div class="vpn-start-stop"><input type="button" value="'+(eval('vpn'+(i+1)+'up') ? 'Stop' : 'Start')+' Now" onclick="toggle(\'vpn'+t+'\', vpn'+(i+1)+'up)" id="_vpn'+t+'_button"><\/div>');
			W('<\/div>');
		}
	</script>
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
<script>init();</script>
</body>
</html>
