<!DOCTYPE html>
<!--
	FreshTomato GUI
	Copyright (C) 2023 - 2024 pedro
	https://freshtomato.org/

	For use with FreshTomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] Wireguard</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<style>

.co1, .co2, .co3, .co4, .co5, .co6, .co7, .co8, .co9, .co10 {
	white-space: nowrap;
	overflow: hidden;
	text-overflow: ellipsis;
}

.co5, .co7, .co9, .co10 {
	display: none;
}

.co1, .co2 {
	width: 3%;
	text-align: center;
}

.co3, co4 {
	width: 19%;
}

.co6 {
	width: 40%;
}

.co8 {
	width: 16%;
}

.status-result {
	padding: 10px;
}

.qrcode {
	display: grid;
	width: 100%;
	justify-items: center;
	text-align: center;
	font-size: large;
	padding: 10px;
}

.import-section {
	text-align: center;
	width: 100%;
}

.import-file {
	width: 42%;
}

</style>
<script src="isup.jsz"></script>
<script src="tomato.js"></script>
<script src="wireguard.js"></script>
<script src="interfaces.js"></script>
<script src="qrcode.js"></script>
<script src="html5-qrcode.js"></script>
<script>


//	<% nvram("wan_ipaddr,wan_hostname,wan_domain,lan_ifname,lan_ipaddr,lan_netmask,lan1_ifname,lan1_ipaddr,lan1_netmask,lan2_ifname,lan2_ipaddr,lan2_netmask,lan3_ifname,lan3_ipaddr,lan3_netmask,wg_adns,wg0_enable,wg0_file,wg0_ip,wg0_fwmark,wg0_mtu,wg0_preup,wg0_postup,wg0_predown,wg0_postdown,wg0_aip,wg0_dns,wg0_peer_dns,wg0_ka,wg0_port,wg0_key,wg0_endpoint,wg0_com,wg0_lan0,wg0_lan1,wg0_lan2,wg0_lan3,wg0_rgw,wg0_peers,wg0_route,wg1_enable,wg1_file,wg1_ip,wg1_fwmark,wg1_mtu,wg1_preup,wg1_postup,wg1_predown,wg1_postdown,wg1_aip,wg1_dns,wg1_peer_dns,wg1_ka,wg1_port,wg1_key,wg1_endpoint,wg1_com,wg1_lan0,wg1_lan1,wg1_lan2,wg1_lan3,wg1_rgw,wg1_peers,wg1_route,wg2_enable,wg2_file,wg2_ip,wg2_fwmark,wg2_mtu,wg2_preup,wg2_postup,wg2_predown,wg2_postdown,wg2_aip,wg2_dns,wg2_peer_dns,wg2_ka,wg2_port,wg2_key,wg2_endpoint,wg2_com,wg2_lan0,wg2_lan1,wg2_lan2,wg2_lan3,wg2_rgw,wg2_peers,wg2_route"); %>

var cprefix = 'vpn_wireguard';
var changed = 0;
var serviceType = 'wireguard';

var tabs =  [];
for (i = 0; i < WG_INTERFACE_COUNT; ++i)
	tabs.push(['wg'+i,'wg'+i]);
var sections = [['config','Config'],['peers','Peers'],['scripts','Scripts'],['status','Status']];

window.addEventListener('beforeunload', function (e) {
	if (changed) {
		var confirmationMessage = 'It looks like you have changed the values of some fields. If you leave before saving, your changes will be lost.';

		(e || window.event).returnValue = confirmationMessage;
		return confirmationMessage;
	}
});

function update_nvram(fom) {
	for (var i = 0; i < fom.length; ++i) {
		if (fom[i].name in nvram)
			nvram[fom[i].name] = fom[i].value;
	}
}

form.submit = function(fom, async, url) {
	var e, v, f, i, wait, msg, sb, cb, nomsg = 0;

	fom = E(fom);

	if (isLocal()) {
		this.dump(fom, async, url);
		return;
	}

	if (this.xhttp) return;

	if ((sb = E('save-button')) != null) sb.disabled = 1;
	if ((cb = E('cancel-button')) != null) cb.disabled = 1;

	if ((!async) || (!useAjax())) {
		this.addId(fom);
		if (url) fom.action = url;
		fom.submit();
		update_nvram(fom);
		return;
	}

	v = ['_ajax=1'];
	wait = 5;
	for (var i = 0; i < fom.elements.length; ++i) {
		f = fom.elements[i];
		if ((f.disabled) || (f.name == '') || (f.name.substr(0, 2) == 'f_')) continue;
		if ((f.tagName == 'INPUT') && ((f.type == 'CHECKBOX') || (f.type == 'RADIO')) && (!f.checked)) continue;
		if (f.name == '_nextwait') {
			wait = f.value * 1;
			if (isNaN(wait))
				wait = 5;
			else
				wait = Math.abs(wait);
		}
		if (f.name == '_nofootermsg') {
			nomsg = f.value * 1;
			if (isNaN(nomsg))
				nomsg = 0;
		}
		v.push(escapeCGI(f.name)+'='+escapeCGI(f.value));
	}

	if ((msg = E('footer-msg')) != null && !nomsg) {
		msg.innerHTML = 'Saving...';
		msg.style.display = 'inline';
	}

	this.xhttp = new XmlHttp();
	this.xhttp.onCompleted = function(text, xml) {
		if (msg && !nomsg) {
			if (text.match(/@msg:(.+)/))
				msg.innerHTML = escapeHTML(RegExp.$1);
			else
				msg.innerHTML = 'Saved';

			update_nvram(fom);
		}
		setTimeout(
			function() {
				if (sb) sb.disabled = 0;
				if (cb) cb.disabled = 0;
				if (msg) msg.style.display = 'none';
				if (typeof(submit_complete) != 'undefined') submit_complete();
			}, wait * 1100);
		form.xhttp = null;
	}
	this.xhttp.onError = function(x) {
		if (url) fom.action = url;
		fom.submit();
	}

	this.xhttp.post(url ? url : fom.action, v.join('&'));
}

function PeerGrid() {return this;}
PeerGrid.prototype = new TomatoGrid;

var peerTables = [];
for (i = 0; i < tabs.length; ++i) {
	peerTables.push(new PeerGrid());
	peerTables[i].interface_name = tabs[i][0];
	peerTables[i].unit = i;
}

function StatusRefresh() {return this;}
StatusRefresh.prototype = new TomatoRefresh;

var statRefreshes = [];
for (i = 0; i < tabs.length; ++i) {
	statRefreshes.push(new StatusRefresh());
	statRefreshes[i].interface_name = tabs[i][0];
	statRefreshes[i].unit = i;
}

function toggleRefresh(unit) {
	statRefreshes[unit].toggle();
}

ferror.show = function(e) {
	if ((e = E(e)) == null) return;
	if (!e._error_msg) return;
	elem.addClass(e, 'error-focused');
	var [tab, section] = locateElement(e);
	tabSelect(tab);
	sectSelect(tab.substr(2), section);
	e.focus();
	alert(e._error_msg);
	elem.removeClass(e, 'error-focused');
}

function locateElement(e) {
	do {
		e = e.parentElement;
	} while(e.id.indexOf('wg') < 0);

	return e.id.split('-', 2);
}

function show() {
	countButton += 1;
	for (var i = 0; i < WG_INTERFACE_COUNT; ++i) {
		var e = E('_'+serviceType+i+'_button');
		var d = eval('isup.'+serviceType+i);

		e.value = (d ? 'Stop' : 'Start')+' Now';
		e.setAttribute('onclick', 'javascript:toggle(\''+serviceType+''+i+'\','+d+');');
		if (serviceLastUp[i] != d || countButton > 6) {
			serviceLastUp[i] = d;
			countButton = 0;
			e.disabled = 0;
			E('spin'+i).style.display = 'none';
		}
		if (!statRefreshes[i].running)
			statRefreshes[i].updateUI('stop');
	}
}

function toggle(service, isup) {
	if (changed && !confirm('There are unsaved changes. Continue anyway?'))
		return;

	serviceLastUp[id] = isup;
	countButton = 0;

	var id = service.substr(service.length - 1);
	E('_'+service+'_button').disabled = 1;
	E('spin'+id).style.display = 'inline';

	var fom = E('t_fom');
	var bup = fom._service.value;
	fom._service.value = service+(isup ? '-stop' : '-start');

	form.submit(fom, 1, 'service.cgi');
	fom._service.value = bup;
}

function tabSelect(name) {
	tgHideIcons();

	tabHigh(name);

	for (var i = 0; i < tabs.length; ++i) {
		if (name == tabs[i][0]) {
			elem.display(tabs[i][0]+'-tab', true);
			for (var j = 0; j < sections.length; ++j) {
				elem.display('notes-'+sections[j][0], (E(tabs[i][0]+'-'+sections[j][0]+'-tab').classList.contains('active')));
			}
		}
		else
			elem.display(tabs[i][0]+'-tab', false);
	}

	cookie.set('wg_tab', name);
}

function sectSelect(tab, section) {
	tgHideIcons();

	for (var i = 0; i < sections.length; ++i) {
		if (section == sections[i][0]) {
			elem.addClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], true);
			elem.display('notes-'+sections[i][0], true);
		}
		else {
			elem.removeClass(tabs[tab][0]+'-'+sections[i][0]+'-tab', 'active');
			elem.display(tabs[tab][0]+'-'+sections[i][0], false);
			elem.display('notes-'+sections[i][0], false);
		}
	}

	cookie.set('wg'+tab+'_section', section);
}

function updateForm(num) {
	var fom = E('t_fom');

	if (eval('isup.wireguard'+num) && fom._service.value.indexOf('wg'+num) < 0) {
		if (fom._service.value != '')
			fom._service.value += ',';

		fom._service.value += 'wireguard'+num+'-restart';
	}
}

function loadConfig(unit) {
	var [file] = E('wg'+unit+'_config_file').files;

	var index = file.name.lastIndexOf('.');
	if (file.name.slice(index).toLowerCase() != '.conf') {
		alert('Only files that end in ".conf" are accepted for import');
		return;
	}

	var reader = new FileReader();
	reader.unit = unit;
	reader.addEventListener('load', mapConfigToFields);
	reader.readAsText(file);
}

function mapConfigToFields(event) {
	var config = mapConfig(event.target.result);
	var unit = event.target.unit;

	if (!validateConfig(config))
		return;

	clearAllFields(unit);

	if (config.interface.privkey)
		E('_wg'+unit+'_key').value = config.interface.privkey;

	if (config.interface.port)
		if (config.interface.port == 51820 + unit)
			E('_wg'+unit+'_port').value = ''
		else
			E('_wg'+unit+'_port').value = config.interface.port;

	if (config.interface.fwmark)
		E('_wg'+unit+'_fwmark').value = config.interface.fwmark;

	if (config.interface.address)
		E('_wg'+unit+'_ip').value = config.interface.address;

	if (config.interface.mtu)
		E('_wg'+unit+'_mtu').value = config.interface.mtu;

	if (config.interface.preup)
		E('_wg'+unit+'_preup').value = config.interface.preup;

	if (config.interface.postup)
		E('_wg'+unit+'_postup').value = config.interface.postup;

	if (config.interface.predown)
		E('_wg'+unit+'_predown').value = config.interface.predown;

	if (config.interface.postdown)
		E('_wg'+unit+'_postdown').value = config.interface.postdown;

	if (config.interface.endpoint) {
		E('_f_wg'+unit+'_custom_endpoint').value = config.interface.endpoint;
		E('_f_wg'+unit+'_endpoint').selectedIndex = 2;
	}

	for (var i = 0; i < config.peers.length; ++i) {
		var peer = config.peers[i];
		var [ip, allowed_ips] = peer.allowed_ips.split(',', 2);

		ip = ip.trim().split('/')[0]+'/32';

		var data = [
			peer.alias ? peer.alias : '',
			peer.endpoint ? peer.endpoint : '',
			peer.privkey ? peer.privkey : '',
			peer.pubkey,
			peer.psk ? peer.psk : '',
			ip,
			allowed_ips ? allowed_ips: '',
			0
		];

		peerTables[unit].insertData(-1, data)
	}

	verifyFields();

	alert('Wireguard configuration imported successfully');
}

function clearAllFields(unit) {
	E('_wg'+unit+'_file').value = '';
	E('_wg'+unit+'_port').value = '';
	E('_wg'+unit+'_key').value = '';
	E('_wg'+unit+'_pubkey').value = '';
	E('_wg'+unit+'_ip').value = '';
	E('_wg'+unit+'_fwmark').value = '';
	E('_wg'+unit+'_mtu').value = '';
	E('_f_wg'+unit+'_adns').checked = 0;
	E('_f_wg'+unit+'_ka').checked = 0;
	E('_f_wg'+unit+'_endpoint').selectedIndex = 0;
	E('_f_wg'+unit+'_custom_endpoint').value = '';
	E('_wg'+unit+'_aip').value = '';
	E('_wg'+unit+'_peer_dns').value = '';
	for (var i = 0; i <= 3; i++)
		E('_f_wg'+unit+'_lan'+i).checked = 0;

	E('_f_wg'+unit+'_rgw').checked = 0;
	E('_wg'+unit+'_preup').value = '';
	E('_wg'+unit+'_postup').value = '';
	E('_wg'+unit+'_predown').value = '';
	E('_wg'+unit+'_postdown').value = '';
	peerTables[unit].removeAllData();
}

function validateConfig(config) {
	if (!config.interface.privkey) {
		alert('The interface requires a PrivateKey');
		return false;
	}

	if (!config.interface.address) {
		alert('The interface requires an Address');
		return false;
	}

	for (var i = 0; i < config.peers.length; ++i) {
		var peer = config.peers[i];

		if (!peer.pubkey) {
			alert('Every peer requires a PublicKey');
			return false;
		}

		if (!peer.allowed_ips) {
			alert('Every peer requires AllowedIPs');
			return false;
		}
	}

	return true;
}

function mapConfig(contents) {
	var lines = contents.split('\n');
	var config = {
		'interface': {},
		'peers': []
	}

	var target;
	for (var i = 0; i < lines.length; ++i) {
		var line = lines[i].trim();

		var comment_index = line.indexOf('#');
		if (comment_index != -1) {
			if (line.match(/\#[a-zA-z]\s?.*\=\s?[a-zA-z0-9].*$/))
				line = line.substr(1);
			else
			line = line.slice(0, comment_index);
		}

		if (!line)
			continue;

		if (line.toLowerCase() == '[interface]') {
			target = config.interface;
			continue;
		}

		if (line.toLowerCase() == '[peer]') {
			target = {};
			config.peers.push(target);
			continue;
		}

		var index = line.indexOf('=');
		var key = line.slice(0, index).trim().toLowerCase();
		var value = line.slice(index + 1).trim();

		switch(key) {
			case 'alias':
				target.alias = value;
				break;
			case 'privatekey':
				target.privkey = value;
				break;
			case 'listenport':
				target.port = value;
				break;
			case 'fwmark':
				target.fwmark = value;
				break;
			case 'address':
				if (!target.address)
					target.address = value;
				else
					target.address = [target.address, value].join(',');
				break;
			case 'dns':
				if (!target.dns)
					target.dns = value;
				else
					target.dns = [target.dns, value].join(',');
				break;
			case 'mtu':
				target.mtu = value;
				break;
			case 'table':
				target.table = value;
				break;
			case 'preup':
				if (!target.preup)
					target.preup = value;
				else
					target.preup = [target.preup, value].join('\n');
				break;
			case 'postup':
				if (!target.postup)
					target.postup = value;
				else
					target.postup = [target.postup, value].join('\n');
				break;
			case 'predown':
				if (!target.predown)
					target.predown = value;
				else
					target.predown = [target.predown, value].join('\n');
				break;
			case 'postdown':
				if (!target.postdown)
					target.postdown = value;
				else
					target.postdown = [target.postdown, value].join('\n');
				break;
			case 'publickey':
				target.pubkey = value;
				break;
			case 'presharedkey':
				target.psk = value;
				break;
			case 'allowedips':
				if (!target.allowed_ips)
					target.allowed_ips = value;
				else
					target.allowed_ips = [target.allowed_ips, value].join(',');
				break;
			case 'endpoint':
				target.endpoint = value;
				break;
			case 'persistentkeepalive':
				target.keepalive = value;
				break;
		} 
	}

	return config;
}

StatusRefresh.prototype.setup = function() {
	var e, v;

	this.actionURL = 'shell.cgi';
	this.postData = 'action=execute&command='+escapeCGI('/usr/sbin/wg show wg'+this.unit+' dump\n'.replace(/\r/g, ''));
	this.refreshTime = 5 * 1000;
	this.cookieTag = 'wg'+this.unit+'_refresh';
	this.dontuseButton = 0;
	this.timer = new TomatoTimer(THIS(this, this.start));
}

StatusRefresh.prototype.start = function() {
	var e;

	if ((e = E('wg'+this.unit+'_status_refresh_time')) != null) {
		if (this.cookieTag)
			cookie.set(this.cookieTag, e.value);

		if (this.dontuseButton != 1)
			this.refreshTime = e.value * 1000;
	}

	this.updateUI('start');

	if ((e = E('wg'+this.unit+'_status_refresh_button')) != null) {
		if (e.value == 'Refresh')
			this.once = 1;
	}

	e = undefined;

	this.running = 1;
	if ((this.http = new XmlHttp()) == null) {
		reloadPage();
		return;
	}

	this.http.parent = this;

	this.http.onCompleted = function(text, xml) {
		var p = this.parent;

		if (p.cookieTag)
			cookie.unset(p.cookieTag+'-error');
		if (!p.running) {
			p.stop();
			return;
		}

		p.refresh(text);

		if ((p.refreshTime > 0) && (!p.once)) {
			p.updateUI('wait');
			p.timer.start(Math.round(p.refreshTime));
		}
		else
			p.stop();

		p.errors = 0;
	}

	this.http.onError = function(ex) {
		var p = this.parent;
		if ((!p) || (!p.running))
			return;

		p.timer.stop();

		if (++p.errors <= 3) {
			p.updateUI('wait');
			p.timer.start(3000);
			return;
		}

		if (p.cookieTag) {
			var e = cookie.get(p.cookieTag+'-error') * 1;
			if (isNaN(e))
				e = 0;
			else
				++e;

			cookie.unset(p.cookieTag);
			cookie.set(p.cookieTag+'-error', e, 1);
			if (e >= 3) {
				alert('XMLHTTP: '+ex);
				return;
			}
		}

		setTimeout(reloadPage, 2000);
	}

	this.errors = 0;
	this.http.post(this.actionURL, this.postData);
}

StatusRefresh.prototype.updateUI = function(mode) {
	var e, b;

	if (typeof(E) == 'undefined') /* for a bizzare bug... */
		return;

	if (this.dontuseButton != 1) {
		b = (mode != 'stop') && (this.refreshTime > 0);

		if ((e = E('wg'+this.unit+'_status_refresh_button')) != null) {
			e.value = b ? 'Stop' : 'Refresh';
			((mode == 'start') && (!b) ? e.setAttribute('disabled', 'disabled') : e.removeAttribute('disabled'));
		}

		if ((e = E('wg'+this.unit+'_status_refresh_time')) != null)
			((!b) ? e.removeAttribute('disabled') : e.setAttribute('disabled', 'disabled'));

		if ((e = E('wg'+this.unit+'_status_refresh_spinner')) != null)
			e.style.display = (b ? 'inline-block' : 'none');
	}
}

StatusRefresh.prototype.initPage = function(delay, refresh) {
	var e, v;

	e = E('wg'+this.unit+'_status_refresh_time');
	if (((this.cookieTag) && (e != null)) && ((v = cookie.get(this.cookieTag)) != null) && (!isNaN(v *= 1))) {
		e.value = Math.abs(v);
		if (v > 0)
			v = v * 1000;
	}
	else if (refresh) {
		v = refresh * 1000;
		if ((e != null) && (this.dontuseButton != 1))
			e.value = refresh;
	}
	else
		v = 0;

	if (delay < 0) {
		v = -delay;
		this.once = 1;
	}

	if (v > 0) {
		this.running = 1;
		this.refreshTime = v;
		this.timer.start(delay);
		this.updateUI('wait');
	}
}

StatusRefresh.prototype.refresh = function(text) {
	var cmdresult;
	var output;
	eval(text);
	if (cmdresult == 'Unable to access interface: No such device\n' || cmdresult == 'Unable to access interface: Protocol not supported\n') {
		output = 'Wireguard device wg'+this.unit+' is down';
	}
	else {
		var [iface, peers] = decodeDump(cmdresult, this.unit);
		output = encodeStatus(iface, peers);
	}
	displayStatus(this.unit, output);
}

PeerGrid.prototype.setup = function() {
	this.init(this.interface_name+'-peers-grid', '', 50, [
		{ type: 'text' },
		{ type: 'text' },
		{ type: 'text', maxlen: 32 },
		{ type: 'text', maxlen: 128 },
		{ type: 'password', maxlen: 44 },
		{ type: 'text', maxlen: 44 },
		{ type: 'password', maxlen: 44 },
		{ type: 'text', maxlen: 100 },
		{ type: 'text', maxlen: 128 },
		{ type: 'text', maxlen: 3 },
	]);
	this.headerSet(['QR', 'Cfg', 'Alias','Endpoint','Private Key','Public Key','Preshared Key','Interface IP','Allowed IPs','KA']);
	this.disableNewEditor(true);

	var peers = decodePeers(this.unit);
	for (var i = 0; i < peers.length; ++i) {
		var peer = peers[i];
		var data = [
			peer.alias,
			peer.endpoint,
			peer.privkey,
			peer.pubkey,
			peer.psk,
			peer.ip,
			peer.allowed_ips,
			peer.keepalive
		];
		this.insertData(-1, data);
	}
}

PeerGrid.prototype.edit = function(cell) {

	var row = PR(cell);
	var data = row.getRowData();

	clearPeerFields(this.unit);

	var alias = E('_f_wg'+this.unit+'_peer_alias');
	var endpoint = E('_f_wg'+this.unit+'_peer_ep');
	var port = E('_f_wg'+this.unit+'_peer_port');
	var privkey = E('_f_wg'+this.unit+'_peer_privkey');
	var pubkey = E('_f_wg'+this.unit+'_peer_pubkey');
	var psk = E('_f_wg'+this.unit+'_peer_psk');
	var ip = E('_f_wg'+this.unit+'_peer_ip');
	var allowedips = E('_f_wg'+this.unit+'_peer_aip');
	var keepalive = E('_f_wg'+this.unit+'_peer_ka');
	var fwmark = E('_f_wg'+this.unit+'_peer_fwmark');

	var interface_port = nvram[this.interface_name+'_port'];
	if (interface_port == '')
		interface_port = 51820 + this.unit;

	alias.value = data[0];
	endpoint.value = data[1];
	port.value = interface_port;
	privkey.value = data[2];
	pubkey.value = data[3];
	psk.value = data[4];
	ip.value = data[5];
	allowedips.value = data[6];
	keepalive.checked = data[7] == 1 ? 1 : 0;
	fwmark.value = '';

	var button = E(this.interface_name+'_peer_add');
	button.value = 'Save to Peers';
	button.setAttribute('onclick', 'editPeer('+this.unit+', '+row.rowIndex+')');

}

PeerGrid.prototype.insertData = function(at, data) {
	if (at == -1)
		at = this.tb.rows.length ;
	var view = this.dataToView(data);
	var qr = '';
	var cfg = '';
	if (data[2] != '') {
		qr = '<img src="qr-icon.svg" alt="" title="Display QR Code" height="16px" onclick="genPeerGridConfigQR(event,'+this.unit+','+at+')">';
		cfg = '<img src="cfg-icon.svg" alt="" title="Download Config File" height="16px" onclick="genPeerGridConfigFile(event,'+this.unit+','+at+')">';
	}
	view.unshift(qr, cfg);

	return this.insert(at, data, view, false);
}

PeerGrid.prototype.rowDel = function(e) {
	changed = 1;
	TGO(e).moving = null;
	e.parentNode.removeChild(e);
	this.recolor();
	this.resort();
	this.rpHide();
}

PeerGrid.prototype.rpDel = function(e) {
	e = PR(e);
	var qrcode = E('wg'+this.unit+'_qrcode');
	if (qrcode.style.display != 'none') {
		var qr_row_id = qrcode.getAttribute('row_id');

		if (qr_row_id == e.rowIndex)
			elem.display('wg'+this.unit+'_qrcode', false);
		else {
			if (qr_row_id > e.rowIndex) {
				qr_row_id = qr_row_id - 1;
				qrcode.setAttribute('row_id', qr_row_id)
			}

			var content = genPeerGridConfig(this.unit, qr_row_id);
			displayQRCode(content, this.unit, qr_row_id);
		}
		
	}
	this.rowDel(e);
}

PeerGrid.prototype.getAllData = function() {
	var i, max, data, r, type;

	data = [];
	max = this.footer ? this.footer.rowIndex : this.tb.rows.length;
	for (i = this.header ? this.header.rowIndex + 1 : 0; i < max; ++i) {
		r = this.tb.rows[i];
		if ((r.style.display != 'none') && (r._data)) data.push(r._data);
	}

	/* reformat the data to include one key and a flag specifying which type */
	for (i = 0; i < data.length; ++i) {
		data[i] = encodePeers(data[i]);
	}

	return data;
}

PeerGrid.prototype.rpMouIn = function(evt) {
	var e, x, ofs, me, s, n;

	if ((evt = checkEvent(evt)) == null) return;

	me = TGO(evt.target);
	if (me.isEditing()) return;
	if (me.moving) return;

	me.rpHide();
	e = document.createElement('div');
	e.tgo = me;
	e.ref = evt.target;
	e.setAttribute('id', 'tg-row-panel');

	n = 0;
	s = '';
	if (me.canMove) {
		s = '<img src="rpu.gif" onclick="this.parentNode.tgo.rpUp(this.parentNode.ref)" title="Move Up"><img src="rpd.gif" onclick="this.parentNode.tgo.rpDn(this.parentNode.ref)" title="Move Down"><img src="rpm.gif" onclick="this.parentNode.tgo.rpMo(this,this.parentNode.ref)" title="Move">';
		n += 3;
	}
	if (me.canDelete) {
		s += '<img src="rpx.gif" onclick="this.parentNode.tgo.rpDel(this.parentNode.ref)" title="Delete">';
		++n;
	}
	x = PR(evt.target);
	x = x.cells[x.cells.length - 3];
	ofs = elem.getOffset(x);
	n *= 18;
	e.style.left = (ofs.x + x.offsetWidth - n)+'px';
	e.style.top = ofs.y+'px';
	e.style.width = n+'px';
	e.innerHTML = s;

	document.body.appendChild(e);
}

function decodePeers(unit) {
	var peers = []
	var nv = nvram['wg'+unit+'_peers'].split('>');
	for (var i = 0; i < nv.length; ++i) {
		var t = nv[i].split('<');
		if (t.length == 8) {
			var data, pubkey, privkey;

			if (t[0] == 1) {
				privkey = t[3];
				pubkey = window.wireguard.generatePublicKey(privkey);
			}
			else {
				privkey = '';
				pubkey = t[3];
			}

			peers.push({
				'alias': t[1],
				'endpoint': t[2],
				'privkey': privkey,
				'pubkey': pubkey,
				'psk': t[4],
				'ip': t[5],
				'allowed_ips': t[6],
				'keepalive': t[7]
			});
		}
	}

	return peers;
}

function encodePeers(data) {
	var key, type;

	if(data[2]) {
		type = 1;
		key = data[2];
	}
	else {
		type = 0;
		key = data[3];
	}

	data = [
		type,
		data[0],
		data[1],
		key,
		data[4],
		data[5],
		data[6],
		data[7],
	]

	return data;
}

function verifyPeerFields(unit, require_privkey) {

	var result = true;

	var alias = E('_f_wg'+unit+'_peer_alias');
	var endpoint = E('_f_wg'+unit+'_peer_ep');
	var port = E('_f_wg'+unit+'_peer_port');
	var privkey = E('_f_wg'+unit+'_peer_privkey');
	var pubkey = E('_f_wg'+unit+'_peer_pubkey');
	var psk = E('_f_wg'+unit+'_peer_psk');
	var ip = E('_f_wg'+unit+'_peer_ip');
	var allowedips = E('_f_wg'+unit+'_peer_aip');
	var keepalive = E('_f_wg'+unit+'_peer_ka');
	var fwmark = E('_f_wg'+unit+'_peer_fwmark');

	if ((!port.value.match(/^ *[-\+]?\d+ *$/)) || (port.value < 1) || (port.value > 65535)) {
		ferror.set(port, 'A valid port must be provided', !result);
		result = false;
	}
	else
		ferror.clear(port);

	if ((privkey.value || require_privkey) && !window.wireguard.validateBase64Key(privkey.value)) {
		ferror.set(privkey, 'A valid private key must be provided', !result);
		result = false;
	}
	else
		ferror.clear(privkey);

	if (psk.value && !window.wireguard.validateBase64Key(psk.value)) {
		ferror.set(psk, 'A valid PSK must be provided or left blank', !result);
		result = false;
	}
	else
		ferror.clear(psk);

	if (!verifyCIDR(ip.value)) {
		ferror.set(ip, 'A valid IP CIDR must be provided to generate a configuration file', !result);
		result = false;
	}
	else
		ferror.clear(ip);

	var ok = true;
	if (allowedips.value != '') {
		var cidrs = allowedips.value.split(',')
		for(var i = 0; i < cidrs.length; i++) {
			var cidr = cidrs[i].trim();
			if (!verifyCIDR(cidr)) {
				ok = false;
			}
		}
	}
	if (!ok) {
		ferror.set(allowedips, 'Allowed IPs must be in CIDR format separated by commas', !result);
		result = false;
	}
	else
		ferror.clear(allowedips);
	
	if (fwmark.value && !verifyFWMark(fwmark.value)) {
		ferror.set(fwmark, 'FWMark must be a hexadecimal number', !result);
		result = false;
	}
	else
		ferror.clear(fwmark);

	return result;
}

function copyInterfacePubKey(unit) {
	const textArea = document.createElement('textarea');
	textArea.value = E('_wg'+unit+'_pubkey').value;

	/* Move textarea out of the viewport so it's not visible */
	textArea.style.position = 'absolute';
	textArea.style.left = '-999999px';

	document.body.prepend(textArea);
	textArea.select();

	try {
		document.execCommand('copy');
	} catch (error) {
		console.error(error);
	} finally {
		textArea.remove();
	}
}

function generateInterfaceKey(unit) {
	var response = false;

	if (E('_wg'+unit+'_key').value == '')
		response = true;
	else
		response = confirm('Regenerating the interface private key will\ncause any generated peers to stop working!\nDo you want to continue?');

	if (response) {
		var keys = window.wireguard.generateKeypair();
		E('_wg'+unit+'_key').value = keys.privateKey;
		E('_wg'+unit+'_pubkey').value = keys.publicKey;
		updateForm(unit);
		changed = 1;
	}
}

function peerFieldsToData(unit) {
	var alias = E('_f_wg'+unit+'_peer_alias').value;
	var endpoint = E('_f_wg'+unit+'_peer_ep').value;
	var privkey = E('_f_wg'+unit+'_peer_privkey').value;
	var pubkey = E('_f_wg'+unit+'_peer_pubkey').value;
	var psk = E('_f_wg'+unit+'_peer_psk').value;
	var ip = E('_f_wg'+unit+'_peer_ip').value;
	var allowedips = E('_f_wg'+unit+'_peer_aip').value;
	var keepalive = E('_f_wg'+unit+'_peer_ka').checked;

	if (privkey != '')
		pubkey = window.wireguard.generatePublicKey(privkey);

	return [
		alias,
		endpoint,
		privkey,
		pubkey,
		psk,
		ip,
		allowedips,
		keepalive
	];
}

function clearPeerFields(unit) {
	var port = nvram['wg'+unit+'_port'];
	if (port == '')
		port = 51820 + unit;
	E('_f_wg'+unit+'_peer_alias').value = '';
	E('_f_wg'+unit+'_peer_ep').value = '';
	E('_f_wg'+unit+'_peer_port').value = port;
	E('_f_wg'+unit+'_peer_privkey').value = '';
	E('_f_wg'+unit+'_peer_privkey').disabled = false;
	E('_f_wg'+unit+'_peer_pubkey').value = '';
	E('_f_wg'+unit+'_peer_pubkey').disabled = false;
	E('_f_wg'+unit+'_peer_psk').value = '';
	E('_f_wg'+unit+'_peer_ip').value = '';
	E('_f_wg'+unit+'_peer_aip').value = '';
	E('_f_wg'+unit+'_peer_ka').checked = 0;
	E('_f_wg'+unit+'_peer_fwmark').value = '';
}

function addPeer(unit, quiet) {
	if (!verifyPeerFields(unit))
		return;

	changed = 1;

	var data = peerFieldsToData(unit);
	peerTables[unit].insertData(-1, data);
	peerTables[unit].disableNewEditor(true);

	clearPeerFields(unit);
	updateForm(unit);

	var qrcode = E('wg'+unit+'_qrcode');
	if (qrcode.style.display != 'none') {
		var row = qrcode.getAttribute('row_id');
		var content = genPeerGridConfig(unit, row);
		displayQRCode(content, unit, row);
	}
}

function editPeer(unit, rowIndex, quiet) {
	if (!verifyPeerFields(unit))
		return;

	changed = 1;

	var data = peerFieldsToData(unit);
	var row = peerTables[unit].tb.firstChild.rows[rowIndex];
	peerTables[unit].rowDel(row);
	peerTables[unit].insertData(rowIndex, data);
	peerTables[unit].disableNewEditor(true);

	clearPeerFields(unit);
	updateForm(unit);

	var button = E('wg'+unit+'_peer_add');
	button.value = 'Add to Peers';
	button.setAttribute('onclick', 'addPeer('+unit+')');

	var qrcode = E('wg'+unit+'_qrcode');
	if (qrcode.style.display != 'none') {
		var row = qrcode.getAttribute('row_id');
		var content = genPeerGridConfig(unit, row);
		displayQRCode(content, unit, row);
	}
}

function verifyPeerGenFields(unit) {
	/* verify interface has a valid private key */
	if (!window.wireguard.validateBase64Key(nvram['wg'+unit+'_key'])) {
		alert('The interface must have a valid private key before peers can be generated')
		return false;
	}

	/* verify peer fwmark*/
	var fwmark = E('_f_wg'+unit+'_peer_fwmark').value;
	if (fwmark && !verifyFWMark(fwmark)) {
		alert('The peer FWMark must be a hexadecimal string of 8 characters')
		return false;
	}

	return true;
}

function generatePeer(unit) {
	/* verify peer gen fields have valid data */
	if (!verifyPeerGenFields(unit))
		return;

	/* Generate keys */
	var keys = window.wireguard.generateKeypair();

	/* Generate PSK (if checked) */
	var psk = '';
	if (E('_f_wg'+unit+'_peer_psk_gen').checked)
		psk = window.wireguard.generatePresharedKey();

	/* retrieve existing IPs of interface/peers to calculate new ip */
	var [interface_ip, interface_nm] = nvram['wg'+unit+'_ip'].split(',')[0].split('/', 2);
	var existing_ips = peerTables[unit].getAllData();
	existing_ips = existing_ips.map(x => x[5].split('/',1)[0]);
	existing_ips.push(interface_ip);

	/* calculate ip of new peer */
	var ip = '';
	var limit = 2 ** (32 - parseInt(interface_nm, 10));
	for (var i = 1; i < limit; i++) {
		var temp_ip = getAddress(ntoa(i), interface_ip);
		var end = temp_ip.split('.').slice(0, -1);

		if (end >= '255' || end == '0')
			continue;

		if (existing_ips.includes(temp_ip))
			continue;

		ip = temp_ip;
		break;
	}

	/* return if we could not generate an IP */
	if (ip == '') {
		alert('Could not generate an IP for the peer');
		return;
	}

	if (E('_wg'+unit+'_com').value > 0) /* not 'Hub and Spoke' */
		var netmask = interface_nm;
	else
		var netmask = '32'; /* 'Hub and Spoke' */

	/* set fields with generated data */
	E('_f_wg'+unit+'_peer_privkey').value = keys.privateKey;
	E('_f_wg'+unit+'_peer_privkey').disabled = false;
	E('_f_wg'+unit+'_peer_pubkey').value = keys.publicKey;
	E('_f_wg'+unit+'_peer_pubkey').disabled = true;
	E('_f_wg'+unit+'_peer_psk').value = psk;
	E('_f_wg'+unit+'_peer_ip').value = ip+'/'+netmask;
	E('_f_wg'+unit+'_peer_ka').checked = 0;

	var button = E('wg'+unit+'_peer_add');
	button.value = 'Add to Peers';
	button.setAttribute('onclick', 'addPeer('+unit+')');
}

function displayQRCode(content, unit) {
	var qrcode = E('wg'+unit+'_qrcode');
	var qrcode_content = content.join('');
	var image = showQRCode(qrcode_content);
	image.style.maxWidth = '700px';
	qrcode.replaceChild(image, qrcode.firstChild);
	elem.display('wg'+unit+'_qrcode', true);
}

function genPeerGridConfigQR(event, unit, row) {
	var qrcode = E('wg'+unit+'_qrcode');
	if (qrcode.getAttribute('row_id') == row && qrcode.style.display != 'none') {
		elem.display('wg'+unit+'_qrcode', false);
	}
	else {
		var content = genPeerGridConfig(unit, row);
		if (content != false) {
			displayQRCode(content, unit, row);
			qrcode.setAttribute('row_id', row);
		}
	}
	event.stopPropagation();
}

function genPeerGridConfigFile(event, unit, row) {
	var content = genPeerGridConfig(unit, row);
	if (content != false) {
		var filename = 'peer'+row+'.conf';
		var alias = peerTables[unit].tb.rows[row]._data[0];
		if (alias != '')
			filename = alias+'.conf';
		downloadConfig(content, filename);
	}
	event.stopPropagation();
}

function genPeerGridConfig(unit, row) {
	var port = E('_f_wg'+unit+'_peer_port');
	var fwmark = E('_f_wg'+unit+'_peer_fwmark');
	var row_data = peerTables[unit].tb.rows[row]._data;
	var result = true;

	if (!row_data[2]) {
		alert('The selected peer does not have a private key stored, which is require for configuration generation');
		result = false;
	}

	if ((!port.value.match(/^ *[-\+]?\d+ *$/)) || (port.value < 1) || (port.value > 65535)) {
		ferror.set(port, 'A valid port must be provided', !result);
		result = false;
	}
	else
		ferror.clear(port);

	if (fwmark.value && !verifyFWMark(fwmark.value)) {
		ferror.set(fwmark, 'FWMark must be a hexadecimal number or 0', !result);
		result = false;
	}
	else
		ferror.clear(fwmark);

	if (!result)
		return false;

	return generateWGConfig(unit, row_data[0], row_data[2], row_data[4], row_data[5].split('/')[0], port.value, fwmark.value, row_data[7]=='1', row_data[1]);
}

function generateWGConfig(unit, name, privkey, psk, ip, port, fwmark, keepalive, endpoint) {
	var [interface_ip, interface_nm] = nvram['wg'+unit+'_ip'].split(',', 1)[0].split('/', 2);
	var content = [];
	var dns = nvram['wg'+unit+'_peer_dns'];

	/* build interface section */
	content.push('[Interface]\n');

	if (name != '')
		content.push('#Alias = '+name+'\n');

	content.push(
		'Address = '+ip+'/'+interface_nm+'\n',
		'ListenPort = '+port+'\n',
		'PrivateKey = '+privkey+'\n'
	);

	if (dns != '')
		content.push('DNS = '+dns+'\n')

	if (fwmark && fwmark != 0)
		content.push ('FwMark = 0x'+fwmark+'\n');

	/* build router peer */
	var publickey_interface = window.wireguard.generatePublicKey(nvram['wg'+unit+'_key']);
	var router_endpoint = nvram['wg'+unit+'_endpoint'];
	switch(router_endpoint[0]) {
	case '0':
		if (nvram.wan_domain) {
			router_endpoint = nvram.wan_domain;
			if (nvram.wan_hostname && nvram.wan_hostname != 'unknown')
				router_endpoint = nvram.wan_hostname+'.'+router_endpoint;
		}
		else
			router_endpoint = nvram.wan_ipaddr;
		break;
	case '1':
		router_endpoint = nvram.wan_ipaddr;
		break;
	case '2':
		router_endpoint = router_endpoint.split('|', 2)[1];
		break;
	}
	var port = nvram['wg'+unit+'_port'];
	if (port == '')
		port = 51820 + unit;
	router_endpoint += ':'+port;

	/* build allowed ips for router peer */
	var allowed_ips;
	if (eval('nvram.wg'+unit+'_rgw') == 1) /* forward all peer traffic? */
		allowed_ips = '0.0.0.0/0'
	else {
		if (eval('nvram.wg'+unit+'_com') > 0) /* not 'Hub and Spoke' */
			var netmask = interface_nm;
		else
			var netmask = '32'; /* 'Hub and Spoke' */

		allowed_ips = interface_ip+'/'+netmask;
		var other_ips_index = nvram['wg'+unit+'_ip'].indexOf(',');
		if (other_ips_index > -1) /* Interface IP has more IPs? */
			allowed_ips += nvram['wg'+unit+'_ip'].substring(other_ips_index);

		for (var i = 0; i <= MAX_BRIDGE_ID; ++i) {
			if (eval('nvram.wg'+unit+'_lan'+i) == 1) { /* Push LANX to peer? */
				var t = (i == 0 ? '' : i);
				var nm = nvram['lan'+t+'_netmask'];
				var network_ip = getNetworkAddress(nvram['lan'+t+'_ipaddr'], nm);
				allowed_ips += ','+network_ip+'/'+netmaskToCIDR(nm);
			}
		}

		var interface_allowed_ips = nvram['wg'+unit+'_aip']; /* allowed IPs from Peer Parameters (Interface panel) */
		if (interface_allowed_ips != '')
			allowed_ips += ','+interface_allowed_ips;
	}

	/* populate router peer */
	var router_alias = 'Router';
	if (nvram.wan_hostname && nvram.wan_hostname != 'unknown')
		router_alias = nvram.wan_hostname;

	content.push(
		'\n',
		'[Peer]\n',
		'#Alias = '+router_alias+'\n',
		'PublicKey = '+publickey_interface+'\n'
	);

	if (psk != '')
		content.push('PresharedKey = '+psk+'\n');

	content.push(
		'AllowedIPs = '+allowed_ips+'\n',
		'Endpoint = '+router_endpoint+'\n'
	);

	if (keepalive)
		content.push('PersistentKeepalive = 25\n');

	/* add remaining peers to config */
	var pubkey = window.wireguard.generatePublicKey(privkey);
	if (eval('nvram.wg'+unit+'_com') > 0) { /* not 'Hub and Spoke' */
		var interface_peers = peerTables[unit].getAllData();

		for (var i = 0; i < interface_peers.length; ++i) {
			var peer = interface_peers[i];

			if (eval('nvram.wg'+unit+'_com') == 1 && endpoint == '' && peer[2] == '')
				continue;

			var peer_pubkey = peer[3];
			if (peer[0] == 1)
				peer_pubkey = window.wireguard.generatePublicKey(peer_pubkey);
			if (peer_pubkey == pubkey)
				continue;

			content.push(
				'\n',
				'[Peer]\n'
			);

			if (peer[1] != '')
				content.push('#Alias = '+peer[1]+'\n');

			content.push('PublicKey = '+peer_pubkey+'\n');

			if (peer[4] != '')
				content.push('PresharedKey = '+peer[4]+'\n');

			content.push('AllowedIPs = '+peer[5]);

			if (peer[6] != '')
				content.push(','+peer[6]);
			content.push('\n');

			if (keepalive)
				content.push('PersistentKeepalive = 25\n');

			if (peer[2] != '')
				content.push('Endpoint = '+peer[2]+'\n');
		}
	}

	return content;
}

function downloadConfig(content, name) {
	const link = document.createElement('a');
	const file = new Blob(content, { type: 'text/plain' });
	link.href = URL.createObjectURL(file);
	link.download = name;
	link.click();
	URL.revokeObjectURL(link.href);
}

function downloadAllConfigs(event, unit) {
	for (var i = 1; i < peerTables[unit].tb.rows.length; ++i) {
		var privkey = peerTables[unit].tb.rows[i].getRowData()[2];
		if (privkey != '')
			genPeerGridConfigFile(event, unit, i);
	}
}

function updateStatus(unit) {
	var result = E('wg'+unit+'_result');
	elem.setInnerHTML(result, '');
	spin(1, 'wg'+unit+'_status_wait');

	cmd = new XmlHttp();
	cmd.onCompleted = function(text, xml) {
		var cmdresult;
		var output;
		eval(text);
		if (cmdresult == 'Unable to access interface: No such device\n' || cmdresult == 'Unable to access interface: Protocol not supported\n') {
			output = 'ERROR: Wireguard device wg'+unit+' does not exist!';
		}
		else {
			var [iface, peers] = decodeDump(cmdresult, unit);
			output = encodeStatus(iface, peers);
		}
		displayStatus(unit, output);
	}
	cmd.onError = function(x) {
		var text = 'ERROR: '+x;
		displayStatus(unit, text);
	}

	var c = '/usr/sbin/wg show wg'+unit+' dump\n';

	cmd.post('shell.cgi', 'action=execute&command='+escapeCGI(c.replace(/\r/g, '')));
}

function decodeDump(dump, unit) {
	var iface;
	var peers = [];
	var lines = dump.split('\n');

	var sections = lines.shift().split('\t');
	iface = {
		'name': 'wg'+unit,
		'alias': 'This Router',
		'privkey': sections[0],
		'pubkey': sections[1],
		'port': sections[2],
		'fwmark': sections[3] == 'off' ? null : sections[3]
	};

	var nvram_peers = decodePeers(unit);

	for (var i = 0; i < lines.length; ++i) {
		var line = lines[i];
		if (line == '')
			continue;
		var line = lines[i].split('\t');
		var peer = {
			'alias': null,
			'pubkey': line[0],
			'psk': line[1] == '(none)' ? null : line[1],
			'endpoint': line[2] == '(none)' ? null : line[2],
			'allowed_ips': line[3],
			'handshake': line[4] == '0' ? null : line[4],
			'rx': line[5],
			'tx': line[6],
			'keepalive': line[7] == 'off' ? null : line[7]
		};
		for (var j = 0; j < nvram_peers.length; ++j) {
			var nvram_peer = nvram_peers[j];
			if (nvram_peer.pubkey == peer.pubkey && nvram_peer.alias != '') {
				peer.alias = nvram_peer.alias;
				break;
			}
		}
		peers.push(peer);
	}

	return [iface, peers];
}

function encodeStatus(iface, peers) {
	/* add interface status */
	var output = 'interface: '+iface.name+'\n';
	output += '  alias: '+iface.alias+'\n';
	output += '  public key: '+iface.pubkey+'\n';
	output += '  listening port: '+iface.port+'\n';
	if (iface.fwmark)
		output += '  fwmark: '+iface.fwmark+'\n';

	/* add peer statuses */
	for (var i = 0; i < peers.length; ++i) {
		var peer = peers[i];
		output +='\n';
		output += 'peer: '+peer.pubkey+'\n';
		if (peer.alias)
			output += '  alias: '+peer.alias+'\n';
		if (peer.psk)
			output += '  preshared key: (hidden)\n';
		if (peer.endpoint)
			output += '  endpoint: '+peer.endpoint+'\n';
		output += '  allowed ips: '+peer.allowed_ips+'\n';
		if (peer.handshake) {
			var seconds = Math.floor(Date.now()/1000 - peer.handshake);
			output += '  latest handshake: '+seconds+' seconds ago\n';
			output += '  transfer: '+formatBytes(peer.rx)+' received, '+formatBytes(peer.tx)+' sent\n';
		}
		if (peer.keepalive)
			output += '  persistent keepalive: every '+peer.keepalive+' seconds\n';
	}

	return output;
}

function formatBytes(bytes) {
	var output;

	if (bytes < 1024)
		output = bytes+' B';
	else if (bytes < 1024 * 1024)
		output = Math.floor(bytes/1024)+' KB';
	else if (bytes < 1024 * 1024 * 1024)
		output = Math.floor(bytes/(1024*1024))+' MB';
	else if (bytes < 1024 * 1024 * 1024 * 1024)
		output = Math.floor(bytes/(1024*1024*1024))+' GB';
	else
		output = Math.floor(bytes/(1024*1024*1024*1024))+' TB';

	return output;
}

function spin(x, which) {
	E(which).style.display = (x ? 'inline-block' : 'none');
}

function displayStatus(unit, text) {
	elem.setInnerHTML(E('wg'+unit+'_result'), '<tt>'+escapeText(text)+'<\/tt>');
	spin(0, 'wg'+unit+'_status_wait');
}

function escapeText(s) {
	function esc(c) {
		return '&#'+c.charCodeAt(0)+';';
	}

	return s.replace(/[&"'<>]/g, esc).replace(/\n/g, ' <br>').replace(/ /g, '&nbsp;');
}

function netmaskToCIDR(mask) {
	var maskNodes = mask.match(/(\d+)/g);
	var cidr = 0;
	for (var i in maskNodes)
		cidr += (((maskNodes[i] >>> 0).toString(2)).match(/1/g) || []).length;

	return cidr;
}

function CIDRToNetmask(bitCount) {
	var mask = [];
	for (var i = 0; i < 4; i++) {
		var n = Math.min(bitCount, 8);
		mask.push(256 - Math.pow(2, 8 - n));
		bitCount -= n;
	}

	return mask.join('.');
}

function isv6(ip) {
	return ip.indexOf(':') < 0;
}

function verifyCIDR(cidr) {
	return cidr.match(/(([1-9]{0,1}[0-9]{0,2}|2[0-4][0-9]|25[0-5])\.){3}([1-9]{0,1}[0-9]{0,2}|2[0-4][0-9]|25[0-5])\/[0-9]|([1-2][0-9]|3[0-2])/)
}

function verifyDNS(dns) {
	var ok = true;
	var ips = dns.split(',');
	for (var i = 0; i < ips.length; i++) {
		var ip = ips[i].trim();
		if (!ip.match(/(([1-9]{0,1}[0-9]{0,2}|2[0-4][0-9]|25[0-5])\.){3}([1-9]{0,1}[0-9]{0,2}|2[0-4][0-9]|25[0-5])/)) {
			ok = false;
			break;
		}
	}

	return ok;
}

function verifyFWMark(fwmark) {
	return fwmark == '0' || fwmark.match(/[0-9A-Fa-f]{8}/);
}

function verifyFields(focused, quiet) {
	var ok = 1;

	/* When settings change, make sure we restart the right services */
	if (focused) {
		changed = 1;

		var fom = E('t_fom');
		var serveridx = focused.name.indexOf('wg');
		if (serveridx >= 0) {
			var num = focused.name.substring(serveridx + 2, serveridx + 3);

			updateForm(num);

			if (focused.name.indexOf('_adns') >= 0 && fom._service.value.indexOf('dnsmasq') < 0) {
				if (fom._service.value != '')
					fom._service.value += ',';

				fom._service.value += 'dnsmasq-restart';
			}
		}
	}

	for (var i = 0; i < WG_INTERFACE_COUNT; i++) {
		/* verify valid port */
		var port = E('_wg'+i+'_port');
		if (port.value != '' && (!port.value.match(/^ *[-\+]?\d+ *$/) || (port.value < 1) || (port.value > 65535))) {
			ferror.set(port, 'The interface port must be a valid port', quiet || !ok);
			ok = 0;
		}
		else {
			ferror.clear(port);
			if (port.value == '')
				E('_f_wg'+i+'_peer_port').value = 51820 + i;
			else
				E('_f_wg'+i+'_peer_port').value = port.value;
		}

		/* disable lan checkbox if lan is not in use */
		for (var j = 0; j <= 3; ++j) {
			t = (j == 0 ? '' : j);

			if (eval('nvram.lan'+t+'_ifname.length') < 1) {
				E('_f_wg'+i+'_lan'+t).checked = 0;
				E('_f_wg'+i+'_lan'+t).disabled = 1;
			}
		}

		/* verify interface private key */
		var privkey = E('_wg'+i+'_key');
		if (privkey.value != '' && !window.wireguard.validateBase64Key(privkey.value)) {
			ferror.set(privkey, 'A valid private key is required for the interface', quiet || !ok);
			ok = 0;
		}
		else
			ferror.clear(privkey);

		/* calculate interface pubkey */
		E('_wg'+i+'_pubkey').disabled = true;
		var pubkey = window.wireguard.generatePublicKey(privkey.value);
		if (pubkey == false) {
			pubkey = '';
		}
		E('_wg'+i+'_pubkey').value = pubkey;

		/* autopopulate IP if it's empty */
		var ip = E('_wg'+i+'_ip');
		if (ip.value == '') {
			ip.value = '10.'+(10+i)+'.0.1/24';
			ferror.clear(ip);
		}
		/* otherwise verify interface CIDR address */
		else {
			var ip_valid = true;
			if (ip.value != '') {
				var cidrs = ip.value.split(',')
				for (var j = 0; j < cidrs.length; j++) {
					var cidr = cidrs[j].trim();
					if (!cidr.match(/^([0-9]{1,3}\.){3}[0-9]{1,3}(\/([0-9]|[1-2][0-9]|3[0-2]))?$/)) {
						ip_valid = false;
						break;
					}
				}
			}
			if (!ip_valid) {
				ferror.set(ip, 'The interface ips must be a comma separated list of valid CIDRs', quiet || !ok);
				ok = 0;
			}
			else
				ferror.clear(ip);
		}

		/* verify interface dns */
		var dns = E('_wg'+i+'_dns');
		if (dns.value != '' && !verifyDNS(dns.value)) {
			ferror.set(dns, 'DNS Servers must be a comma separated list of IPs');
			ok = 0;
		}
		else
			ferror.clear(dns);

		/* verify interface fwmark */
		var fwmark = E('_wg'+i+'_fwmark');
		if (fwmark.value && !verifyFWMark(fwmark.value)) {
			ferror.set(fwmark, 'The interface FWMark must be a hexadecimal string of 8 characters', quiet || !ok);
			ok = 0;
		}
		else
			ferror.clear(fwmark);

		/* autopopulate mtu if it's empty */
		var mtu = E('_wg'+i+'_mtu');
		if (mtu.value == '') {
			mtu.value = '1420';
			ferror.clear(mtu);
		}
		/* otherwise verify interface mtu */
		else {
			if ((!mtu.value.match(/^ *[-\+]?\d+ *$/)) || (mtu.value < 0) || (mtu.value > 1500)) {
				ferror.set(mtu, 'The interface MTU must be a integer between 0 and 1500', quiet || !ok);
				ok = 0;
			}
			else
				ferror.clear(mtu);
		}

		/* hide/show custom endpoint based on option selected */
		var endpoint = E('_f_wg'+i+'_endpoint');
		var custom_endpoint = E('_f_wg'+i+'_custom_endpoint');
		elem.display(custom_endpoint, (endpoint.value == 2));

		/* hide/show custom table based on option selected */
		var route = E('_f_wg'+i+'_route');
		var custom_table = E('_f_wg'+i+'_custom_table');
		if (route.value == 2) {
			elem.display(custom_table, true);
			if (!custom_table.value.match(/^ *[-\+]?\d+ *$/))
				ferror.set(custom_table, 'The custom table must be an integer', quiet || !ok);
			else
				ferror.clear(custom_table);
		}
		else {
			elem.display(custom_table, false);
			ferror.clear(custom_table);
		}

		/* verify peer dns */
		var peer_dns = E('_wg'+i+'_peer_dns');
		if (peer_dns.value != '' && !verifyDNS(peer_dns.value)) {
			ferror.set(peer_dns, 'DNS Servers must be a comma separated list of IPs');
			ok = 0;
		}
		else
			ferror.clear(peer_dns);

		/* verify interface allowed ips */
		var allowed_ips = E('_wg'+i+'_aip')
		var aip_valid = true;
		if (allowed_ips.value != '') {
			var cidrs = allowed_ips.value.split(',')
			for (var j = 0; j < cidrs.length; j++) {
				var cidr = cidrs[j].trim();
				if (!cidr.match(/^([0-9]{1,3}\.){3}[0-9]{1,3}(\/([0-9]|[1-2][0-9]|3[0-2]))?$/)) {
					aip_valid = false;
					break;
				}
			}
		}
		if (!aip_valid) {
			ferror.set(allowed_ips, 'The interface allowed ips must be a comma separated list of valid CIDRs', quiet || !ok);
			ok = 0;
		}
		else
			ferror.clear(allowed_ips);

		/*** peer key checking stuff ***/
		var peer_privkey = E('_f_wg'+i+'_peer_privkey');
		var peer_pubkey = E('_f_wg'+i+'_peer_pubkey');
		peer_privkey.disabled = false;
		peer_pubkey.disabled = false;

		/* if private key is populated (and valid), generate the public key and lock it */
		if (peer_privkey.value) {
			var pubkey_temp = window.wireguard.generatePublicKey(peer_privkey.value);
			if (pubkey_temp) {
				peer_pubkey.disabled = true;
				peer_pubkey.value = pubkey_temp;
			}
		}
	}

	return ok;
}

function save(nomsg) {
	if (!verifyFields(null, 0))
		return;

	if (!nomsg) show(); /* update '_service' field first */

	E('wg_adns').value = '';
	var fom = E('t_fom');
	for (var i = 0; i < WG_INTERFACE_COUNT; i++) {
		var privkey = E('_wg'+i+'_key').value;
		nvram['wg'+i+'_key'] = privkey;

		var data = peerTables[i].getAllData();
		var s = '';
		for (var j = 0; j < data.length; ++j)
			s += data[j].join('<')+'>';

		fom['wg'+i+'_peers'].value = s;
		nvram['wg'+i+'_peers'] = s;

		fom['wg'+i+'_ka'].value = fom['_f_wg'+i+'_ka'].checked ? 1 : 0;
		fom['wg'+i+'_enable'].value = fom['_f_wg'+i+'_enable'].checked ? 1 : 0;
		fom['wg'+i+'_lan0'].value = fom['_f_wg'+i+'_lan0'].checked ? 1 : 0;
		fom['wg'+i+'_lan1'].value = fom['_f_wg'+i+'_lan1'].checked ? 1 : 0;
		fom['wg'+i+'_lan2'].value = fom['_f_wg'+i+'_lan2'].checked ? 1 : 0;
		fom['wg'+i+'_lan3'].value = fom['_f_wg'+i+'_lan3'].checked ? 1 : 0;
		fom['wg'+i+'_rgw'].value = fom['_f_wg'+i+'_rgw'].checked ? 1 : 0;

		if (E('_f_wg'+i+'_adns').checked)
			E('wg_adns').value += i+',';

		var endpoint = E('_f_wg'+i+'_endpoint');
		var custom_endpoint = E('_f_wg'+i+'_custom_endpoint');
		var endpoint_output = endpoint.value+'';
		if (endpoint.value == 2)
			endpoint_output += '|'+custom_endpoint.value;
		fom['wg'+i+'_endpoint'].value = endpoint_output;
		nvram['wg'+i+'_endpoint'] = endpoint_output;

		var route = E('_f_wg'+i+'_route');
		var custom_table = E('_f_wg'+i+'_custom_table');
		var route_output = route.value+'';
		if (route.value == 2)
			route_output += '|'+custom_table.value;
		fom['wg'+i+'_route'].value = route_output;
		nvram['wg'+i+'_route'] = route_output;

		var qrcode = E('wg'+i+'_qrcode');
		if (qrcode.style.display != 'none') {
			var row = qrcode.getAttribute('row_id');
			var content = genPeerGridConfig(i, row);
			displayQRCode(content, i, row);
		}
	}

	form.submit(fom, 1);

	changed = 0;
}

function earlyInit() {
	show();
	var tab = cookie.get('wg_tab') || tabs[0][0];
	for (var i = 0; i < tabs.length; ++i) {
		sectSelect(i, cookie.get('wg'+i+'_section') || sections[0][0]);
	}
	tabSelect(tab);
	verifyFields(null, 1);
}

function init() {
	var c;
	if (((c = cookie.get(cprefix+'_notes_vis')) != null) && (c == '1'))
		toggleVisibility(cprefix, 'notes');

	eventHandler();

	up.initPage(250, 5);
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
<div id="ident"><% ident(); %> | <script>wikiLink();</script></div>

<!-- / / / -->

<input type="hidden" name="_service" value="">
<input type="hidden" name="wg_adns" id="wg_adns">

<!-- / / / -->

<div class="section-title">Wireguard Configuration</div>
<div class="section">
	<script>
		tabCreate.apply(this, tabs);

		for (i = 0; i < tabs.length; ++i) {
			t = tabs[i][0];
			W('<div id="'+t+'-tab">');
			W('<input type="hidden" name="'+t+'_enable">');
			W('<input type="hidden" name="'+t+'_ka">');
			W('<input type="hidden" name="'+t+'_lan0">');
			W('<input type="hidden" name="'+t+'_lan1">');
			W('<input type="hidden" name="'+t+'_lan2">');
			W('<input type="hidden" name="'+t+'_lan3">');
			W('<input type="hidden" name="'+t+'_rgw">');
			W('<input type="hidden" name="'+t+'_endpoint">');
			W('<input type="hidden" name="'+t+'_peers">');
			W('<input type="hidden" name="'+t+'_route">');

			W('<ul class="tabs">');
			for (j = 0; j < sections.length; j++) {
				W('<li><a href="javascript:sectSelect('+i+',\''+sections[j][0]+'\')" id="'+t+'-'+sections[j][0]+'-tab">'+sections[j][1]+'<\/a><\/li>');
			}
			W('<\/ul><div class="tabs-bottom"><\/div>');

			/* config tab start */
			W('<div id="'+t+'-config">');
			W('<div class="section-title">Interface<\/div>');
			createFieldTable('', [
				{ title: 'Enable on Start', name: 'f_'+t+'_enable', type: 'checkbox', value: nvram[t+'_enable'] == 1 },
				{ title: 'Config file', name: t+'_file', type: 'text', placeholder: '(optional)', maxlen: 64, size: 64, value: nvram[t+'_file'] },
				{ title: 'Port', name: t+'_port', type: 'text', maxlen: 5, size: 10, placeholder: (51820+i), value: nvram[t+'_port'] },
				{ title: 'Private Key', multi: [
					{ title: '', name: t+'_key', type: 'password', maxlen: 44, size: 44, value: nvram[t+'_key'], peekaboo: 1 },
					{ title: '', custom: '<input type="button" value="Generate" onclick="generateInterfaceKey('+i+')" id="'+t+'_keygen">' },
				] },
				{ title: 'Public Key', multi: [
					{ title: '', name: t+'_pubkey', type: 'text', maxlen: 44, size: 44, disabled: ""},
					{ title: '', custom: '<input type="button" value="Copy" onclick="copyInterfacePubKey('+i+')" id="'+t+'_pubkey_copy">' },
				] },
				{ title: 'Interface IP', name: t+'_ip', type: 'text', maxlen: 32, size: 17, value: nvram[t+'_ip'], placeholder: "(CIDR format)" },
				{ title: 'DNS Servers', name: t+'_dns', type: 'text', maxlen: 128, size: 64, value: nvram[t+'_dns'] },
				{ title: 'FWMark', name: t+'_fwmark', type: 'text', maxlen: 8, size: 8, value: nvram[t+'_fwmark'] },
				{ title: 'MTU', name: t+'_mtu', type: 'text', maxlen: 4, size: 4, value: nvram[t+'_mtu'] },
				{ title: 'Respond to DNS', name: 'f_'+t+'_adns', type: 'checkbox', value: nvram.wg_adns.indexOf(''+i) >= 0 },
				{ title: 'Routing Mode', name: 'f_'+t+'_route', type: 'select', options: [['0','Off'],['1','Auto'],['2','Custom Table']], value: nvram[t+'_route'][0] || 1, suffix: '&nbsp;<input type="text" name="f_'+t+'_custom_table" value="'+(nvram[t+'_route'].split('|', 2)[1] || '')+'" onchange="verifyFields(this, 1)" id="_f_'+t+'_custom_table" maxlength="32" size="32">' },
			]);
			W('<br>');

			W('<div class="section-title">Peer Parameters<\/div>');
			createFieldTable('', [
				{ title: 'Router behind NAT', name: 'f_'+t+'_ka', type: 'checkbox', value: nvram[t+'_ka'] == 1 },
				{ title: 'Endpoint', name: 'f_'+t+'_endpoint', type: 'select', options: [['0','FQDN'],['1','WAN IP'],['2','Custom Endpoint']], value: nvram[t+'_endpoint'][0] || 0, suffix: '&nbsp;<input type="text" name="f_'+t+'_custom_endpoint" value="'+(nvram[t+'_endpoint'].split('|', 2)[1] || '')+'" onchange="verifyFields(this, 1)" id="_f_'+t+'_custom_endpoint" maxlength="64" size="46">' },
				{ title: 'Allowed IPs', name: t+'_aip', type: 'text', placeholder: "(CIDR format)", maxlen: 128, size: 64, suffix: '&nbsp;<small>comma separated (eg. peer LAN subnet)<\/small>', value: nvram[t+'_aip'] },
				{ title: 'DNS Servers for Peers', name: t+'_peer_dns', type: 'text', maxlen: 128, size: 64, value: nvram[t+'_peer_dns'] },
				{ title: 'Push LAN0 (br0) to peers', name: 'f_'+t+'_lan0', type: 'checkbox', value: nvram[t+'_lan0'] == 1 },
				{ title: 'Push LAN1 (br1) to peers', name: 'f_'+t+'_lan1', type: 'checkbox', value: nvram[t+'_lan1'] == 1 },
				{ title: 'Push LAN2 (br2) to peers', name: 'f_'+t+'_lan2', type: 'checkbox', value: nvram[t+'_lan2'] == 1 },
				{ title: 'Push LAN3 (br3) to peers', name: 'f_'+t+'_lan3', type: 'checkbox', value: nvram[t+'_lan3'] == 1 },
				{ title: 'Forward all peer traffic', name: 'f_'+t+'_rgw', type: 'checkbox', value: nvram[t+'_rgw'] == 1 },
			]);
			W('<br>');

			W('<div class="section-title">Import Config from File<\/div>');
			W('<div class="import-section">');
			W('<input type="file" class="import-file" id="'+t+'_config_file" accept=".conf" name="Browse File">');
			W('<input type="button" id="'+t+'_config_import" value="Import" onclick="loadConfig('+i+')" >');
			W('<\/div>');
			W('<br>');
			W('<\/div>');
			/* config tab stop */

			/* peers tab start */
			W('<div id="'+t+'-peers">');
			W('<div class="section-title">Config Generation<\/div>');
			createFieldTable('', [
				{ title: 'Peer Communication', name: t+'_com', type: 'select', options: [['0','Hub and Spoke'],['1','Full Mesh (Endpoint Only)'],['2','Full Mesh']], value: nvram[t+'_com'] || 0 },
				{ title: 'Port', name: 'f_'+t+'_peer_port', type: 'text', maxlen: 5, size: 10, value: nvram[t+'_port'] == '' ? 51820+i : nvram[t+'_port'] },
				{ title: 'FWMark', name: 'f_'+t+'_peer_fwmark', type: 'text', maxlen: 8, size: 8, value: '0'},
			]);
			W('<br>');

			W('<div class="section-title">Peers<\/div>');
			W('<div class="tomato-grid" id="'+t+'-peers-grid"><\/div>');
			peerTables[i].setup();
			W('<input type="button" value="Download All Configs" onclick="downloadAllConfigs(event,'+i+')" id="'+t+'_download_all">')
			W('<br>');
			W('<br>');
			W('<div id="'+t+'_qrcode" class="qrcode" style="display:none">');
			W('<img src="qr-icon.svg" alt="'+t+'_qrcode_img" style="max-width:100px">');
			W('<div id="'+t+'_qrcode_labels" class="qrcode-labels" title="Message">');
			W('Point your mobile phone camera<br>');
			W('here above to connect automatically');
			W('<\/div>');
			W('<br>');
			W('<\/div>');

			W('<div class="section-title">Peer Generation<\/div>');
			createFieldTable('', [
				{ title: 'Generate PSK', name: 'f_'+t+'_peer_psk_gen', type: 'checkbox', value: true },
			]);
			W('<input type="button" value="Generate Peer" onclick="generatePeer('+i+')" id="'+t+'_peer_gen">');
			W('<br>');
			W('<br>');

			W('<div class="section-title">Peer\'s Parameters<\/div>');
			createFieldTable('', [
				{ title: 'Alias', name: 'f_'+t+'_peer_alias', type: 'text', maxlen: 32, size: 32},
				{ title: 'Endpoint', name: 'f_'+t+'_peer_ep', type: 'text', maxlen: 64, size: 64},
				{ title: 'Private Key', name: 'f_'+t+'_peer_privkey', type: 'text', maxlen: 44, size: 44},
				{ title: 'Public Key', name: 'f_'+t+'_peer_pubkey', type: 'text', maxlen: 44, size: 44},
				{ title: 'Preshared Key', name: 'f_'+t+'_peer_psk', type: 'text', maxlen: 44, size: 44},
				{ title: 'Interface IP', name: 'f_'+t+'_peer_ip', type: 'text', placeholder: "(CIDR format)", maxlen: 64, size: 64},
				{ title: 'Allowed IPs', name: 'f_'+t+'_peer_aip', type: 'text', placeholder: "(CIDR format)", suffix: '&nbsp;<small>comma separated (eg. peer LAN subnet)<\/small>', maxlen: 128, size: 64},
				{ title: 'Peer behind NAT', name: 'f_'+t+'_peer_ka', type: 'checkbox', value: false},
			]);
			W('<div>');
			W('<input type="button" value="Add to Peers" onclick="addPeer('+i+')" id="'+t+'_peer_add">');
			W('<\/div>');
			W('<\/div>');
			/* peers tab stop */

			/* scripts tab start */
			W('<div id="'+t+'-scripts">');
			W('<div class="section-title">Custom Interface Scripts<\/div>');
			createFieldTable('', [
				{ title: 'Pre-Up Script', name: t+'_preup', type: 'textarea', value: nvram[t+'_preup'] },
				{ title: 'Post-Up Script', name: t+'_postup', type: 'textarea', value: nvram[t+'_postup'] },
				{ title: 'Pre-Down Script', name: t+'_predown', type: 'textarea', value: nvram[t+'_predown'] },
				{ title: 'Post-Down Script', name: t+'_postdown', type: 'textarea', value: nvram[t+'_postdown'] },
			]);
			W('<\/div>');
			/* scripts tab stop */

			/* status tab start */
			W('<div id="'+t+'-status">');
			W('<pre id="'+t+'_result" class="status-result"><\/pre>');
			W('<div style="text-align:right">');
			W('<img src="spin.gif" id="'+t+'_status_refresh_spinner" alt=""> ');
			genStdTimeList(t+'_status_refresh_time', 'One off', 0);
			W('<input type="button" value="Refresh" onclick="toggleRefresh('+i+')" id="'+t+'_status_refresh_button"><\/div>');
			W('<div style="display:none;padding-left:5px" id="'+t+'_status_wait"> Please wait... <img src="spin.gif" alt="" style="vertical-align:top"><\/div>');
			statRefreshes[i].setup();
			statRefreshes[i].initPage(3000, 0);
			W('<\/div>');
			/* status tab end */

			/* start/stop button */
			W('<div class="vpn-start-stop"><input type="button" value="" onclick="" id="_wireguard'+i+'_button">&nbsp; <img src="spin.gif" alt="" id="spin'+i+'"><\/div>');

			W('<\/div>');
		}
	</script>
</div>

<!-- / / / -->

<!-- start notes sections -->
<div class="section-title">Notes <small><i><a href='javascript:toggleVisibility(cprefix,"notes");'><span id="sesdiv_notes_showhide">(Show)</span></a></i></small></div>
<div class="section" id="sesdiv_notes" style="display:none">

	<!-- config notes start -->
	<div id="notes-config" style="display:none">
		<ul>
			<li><b>Interface</b> - Settings directly related to the wireguard interface on this device.</li>
			<ul>
				<li><b>Enable on Start</b> - Enabling this will start the wireguard device when the router starts up.</li>
				<li><b>Config file</b> - File path to wg-quick compatible configuration file. If this is specified all other settings will be ignored.</li>
				<li><b>Port</b> - Port to use for the wireguard interface.</li>
				<li><b>Private Key</b> - Private key to use for the wireguard interface. Can be generated by pressing the <b>Generate</b> button.</li>
				<li><b>Public Key</b> - Public key calculated from the Private Key field. This is not user editable and only provided as a convenience.</li>
				<li><b>Interface IP</b> - IP and Netmask to use for the wireguard interface. Must be in CIDR format.</li>
				<li><b>DNS Servers</b> - Comma separated list of DNS servers to use for the wireguard interface.</li>
				<li><b>FWMark</b> - The value of the FWMark to use for routing. If left as 0, it will the default value.</li>
				<li><b>MTU</b> - The maximum transmission unit for the wireguard interface.</li>
				<li><b>Respond to DNS</b> - If checked, this interface will respond to DNS requests using the router's dnsmasq service.</li>
				<li><b>Routing Mode</b> - The routing mode to use when setting up the wireguard interface</li>
				<ul>
					<li><b>Off</b> - Will not add routing rules for the wireguard interface.</li>
					<li><b>Auto</b> - The wireguard interface will be routed using the default table (the same number as the interface port)</li>
					<li><b>Custom Table</b> - Will route the wireguard interface using a custom table number. If sepcified, you must also include the table number in the additional field.</li>
				</ul>
			</ul>
		</ul>
		<ul>
			<li><b>Peer Parameters</b> - Settings related to peer configuration generation on the Peers page.</li>
			<ul>
				<li><b>Router behind NAT</b> - If enabled, Keepalives will be sent from the router to peers.</li>
				<li><b>Endpoint</b> - What to use for the endpoint of the router in configuration files generated on the Peers tab.</li>
				<ul>
					<li><b>FQDN</b> - Will use the hostname and domain name set up on the Identification page.</li>
					<li><b>WAN IP</b> - Will use the WAN IP address of the router.</li>
					<li><b>Custom Endpoint</b> - Will use a user specified endpoint. If selected, you must also include the custom endpoint in the addition field.</li>
				</ul>
				<li><b>Allowed IPs</b> - A list of additional CIDRs to attach to the router peer for configuration files generated on the peers tab.</li>
				<li><b>DNS Servers for Peers</b> - DNS Servers to use in the Interface section of configuration files generated on the peers tab.</li>
				<li><b>Push LAN0 (br0) to peers</b> - Allows the peers access to LAN0</li>
				<li><b>Push LAN1 (br1) to peers</b> - Allows the peers access to LAN1</li>
				<li><b>Push LAN2 (br2) to peers</b> - Allows the peers access to LAN2</li>
				<li><b>Push LAN3 (br3) to peers</b> - Allows the peers access to LAN3</li>
				<li><b>Forward all peer traffic</b> - Ensure all traffic from the peer is tunneled through the router's wireguard interface by adding an Allowed IP of 0.0.0.0/0</li>
			</ul>
		</ul>
		<ul>
			<li><b>Import Config from File</b> - This section can be used to parse fields from a wg-quick compatible configuration file and automatically populate the relevant fields. Using this will wipe your existing settings.</li>
		</ul>
	</div>
	<!-- config notes stop -->

	<!-- peers notes start -->
	<div id="notes-peers" style="display:none">
		<ul>
			<li><b>Config Generation</b></li>
			<ul>
				<li><b>Peer Communication</b> - This field dicatates which peers are added to generated configurations.</li>
				<ul>
					<li><b>Hub and Spoke</b> - Peers will only communicate with the router, and not each other.</li>
					<li><b>Full Mesh (Endpoint Only)</b> - Peers will communicate to any peer with an endpoint. Peers with endpoints will communicate with all peers.</li>
					<li><b>Full Mesh</b> - All peers are added to each other's configuration.</li>
				</ul>
				<li><b>Port</b> - The port to use for the wireguard interface of generated configurations.</li>
				<li><b>FWMark</b> - The FWMark to use for the wireguard interface of generated configurations.</li>
			</ul>
			<li><b>Peers Grid</b> - Each row represents a peer in the network. Peers are added through the Peer's Parameters subsection. Peers can also be edited by clicking on the row. The columns represent the following:</li>
			<ul>
				<li><b>QR</b> - Click the button to generate and display a QR code of this peer's configuration. Click again to hide it.</li>
				<li><b>Cfg</b> - Click the button to generate and download this peer's configuration file.</li>
				<li><b>Alias</b> - A custom name for this peer.</li>
				<li><b>Endpoint</b> - Endpoint of this peer's wireguard interface.</li>
				<li><b>Public Key</b> - The public key for this peer's wireguard interface.</li>
				<li><b>Interface IP</b> - The IP address of this peer's wireguard interface.</li>
			</ul>
			<li><b>Download All Configs</b> - This button will generate and download the configuration files for all the peers in the table.</li>
		</ul>
		<ul>
			<li><b>Peer Generation</b></li>
			<ul>
				<li><b>Generate PSK</b> - If checked, will generate a PSK for this peer when Generate Peer is clicked.</li>
				<li><b>Generate Peer</b> - This button will generate a new peer and populate the fields of the Peer's Parameters subsection.</li>
			</ul>
		</ul>
		<ul>
			<li><b>Peer's Parameters</b> - A subsection for adding and editing peers.</li>
			<ul>
				<li><b>Alias</b> - A custom name for this peer.</li>
				<li><b>Endpoint</b> - Endpoint of this peer's wireguard interface.</li>
				<li><b>Private Key</b> - The private key of this peer. Either this or the public key must be specified.</li>
				<li><b>Public Key</b> - The public key of this peer. Either this or the public key must be specified.</li>
				<li><b>Preshared Key</b> - The PSK to use for this peer.</li>
				<li><b>Interface IP</b> - The IP and Netmask to use for this peer. Must be in CIDR format.</li>
				<li><b>Allowed IPs</b> - Additional Allowed IPs to use for this Peer. Must be in CIDR format. Can be a list of comma separated values.</li>
				<li><b>Peer behind NAT</b> - If enabled, this peer will send Keepalives to all other peers.</li>
				<li><b>Add/Save to Peers</b> - Click to add a peer or save changes to an edited peer.</li>
			</ul>
		</ul>
	</div>
	<!-- peers notes stop -->

	<!-- scripts notes start -->
	<div id="notes-scripts" style="display:none">
		<ul>
			<li><b>Custom Interface Scripts</b> - Custom bash scripts to be run at defined events.</li>
			<ul>
				<li><b>Pre-Up Script</b> - Will be run before the interface is brought up.</li>
				<li><b>Post-Up Script</b> - Will be run after the interface is brought up.</li>
				<li><b>Pre-Down Script</b> - Will be run before the interface is brought down.</li>
				<li><b>Post-Down Script</b> - Will be run after the interface is brought down.</li>
			</ul>
		</ul>
	</div>
	<!-- scripts notes stop -->

	<!-- status notes start -->
	<div id="notes-status" style="display:none">
	</div>
	<!-- status notes stop -->

</div>
<!-- end notes sections -->

<!-- / / / -->

<div id="footer">
	<span id="footer-msg"></span>
	<input type="button" value="Save" id="save-button" onclick="save()">
	<input type="button" value="Cancel" id="cancel-button" onclick="reloadPage();">
</div>

</td></tr>
</table>
</form>
<script>earlyInit();</script>
</body>
</html>
