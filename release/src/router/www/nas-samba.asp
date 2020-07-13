<!DOCTYPE html>
<!--
	Tomato GUI
	Samba Server - !!TB

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en-GB">
<head>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="robots" content="noindex,nofollow">
<title>[<% ident(); %>] NAS: File Sharing</title>
<link rel="stylesheet" type="text/css" href="tomato.css">
<% css(); %>
<script src="tomato.js"></script>

<script>

//	<% nvram("smbd_enable,smbd_user,smbd_passwd,smbd_wgroup,smbd_cpage,smbd_ifnames,smbd_custom,smbd_master,smbd_wins,smbd_shares,smbd_autoshare,smbd_protocol,wan_wins"); %>

var cprefix = 'nas_samba';
var ssg = new TomatoGrid();

ssg.exist = function(f, v) {
	var data = this.getAllData();
	for (var i = 0; i < data.length; ++i) {
		if (data[i][f] == v)
			return true;
	}

	return false;
}

ssg.existName = function(name) {
	return this.exist(0, name);
}

ssg.sortCompare = function(a, b) {
	var col = this.sortColumn;
	var da = a.getRowData();
	var db = b.getRowData();
	var r = cmpText(da[col], db[col]);

	return this.sortAscending ? r : -r;
}

ssg.dataToView = function(data) {
	return [data[0], data[1], data[2], ['Read Only', 'Read / Write'][data[3]], ['No', 'Yes'][data[4]]];
}

ssg.fieldValuesToData = function(row) {
	var f = fields.getAll(row);

	return [f[0].value, f[1].value, f[2].value, f[3].value, f[4].value];
}

ssg.verifyFields = function(row, quiet) {
	var f, s;

	f = fields.getAll(row);

	s = f[0].value.trim().replace(/\s+/g, ' ');
	if (s.length > 0) {
		if (s.search(/^[ a-zA-Z0-9_\-\$]+$/) == -1) {
			ferror.set(f[0], 'Invalid share name. Only characters "$ A-Z 0-9 - _" and spaces are allowed.', quiet);
			return 0;
		}
		if (this.existName(s)) {
			ferror.set(f[0], 'Duplicate share name.', quiet);
			return 0;
		}
		f[0].value = s;
	}
	else {
		ferror.set(f[0], 'Empty share name is not allowed.', quiet);
		return 0;
	}

	if (!v_nodelim(f[1], quiet, 'Directory', 1) || !v_path(f[1], quiet, 1)) return 0;
	if (!v_nodelim(f[2], quiet, 'Description', 1)) return 0;

	return 1;
}

ssg.resetNewEditor = function() {
	var f;

	f = fields.getAll(this.newEditor);
	ferror.clearAll(f);

	f[0].value = '';
	f[1].value = '';
	f[2].value = '';
	f[3].selectedIndex = 0;
	f[4].selectedIndex = 0;
}

ssg.setup = function() {
	this.init('ss-grid', 'sort', 50, [
		{ type: 'text', maxlen: 32 },
		{ type: 'text', maxlen: 256 },
		{ type: 'text', maxlen: 64 },
		{ type: 'select', options: [[0, 'Read Only'],[1, 'Read / Write']] },
		{ type: 'select', options: [[0, 'No'],[1, 'Yes']] }
	]);
	this.headerSet(['Share Name', 'Directory', 'Description', 'Access Level', 'Hidden']);

	var s = nvram.smbd_shares.split('>');
	for (var i = 0; i < s.length; ++i) {
		var t = s[i].split('<');
		if (t.length == 5) {
			this.insertData(-1, t);
		}
	}

	this.sort(0);
	this.showNewEditor();
	this.resetNewEditor();
}

function verifyFields(focused, quiet) {
	var a, b;

	a = E('_smbd_enable').value;

	elem.display(PR('_smbd_user'), PR('_smbd_passwd'), (a == 2));

	E('_smbd_wgroup').disabled = (a == 0);
	E('_smbd_cpage').disabled = (a == 0);
	E('_smbd_ifnames').disabled = (a == 0);
	E('_smbd_custom').disabled = (a == 0);
	E('_smbd_autoshare').disabled = (a == 0);
	E('_f_smbd_master').disabled = (a == 0);
	E('_f_smbd_wins').disabled = (a == 0 || (nvram.wan_wins != '' && nvram.wan_wins != '0.0.0.0'));
	E('_smbd_proto_1').disabled = (a == 0);
	E('_smbd_proto_2').disabled = (a == 0);
	E('_smbd_proto_3').disabled = (a == 0);

	if (a != 0 && !v_length('_smbd_ifnames', quiet, 0, 50)) return 0;
	if (a != 0 && !v_length('_smbd_custom', quiet, 0, 2048)) return 0;

	if (a == 2) {
		if (!v_length('_smbd_user', quiet, 1)) return 0;
		if (!v_length('_smbd_passwd', quiet, 1)) return 0;

		b = E('_smbd_user');
		if (b.value == 'root') {
			ferror.set(b, 'User Name \"root\" is not allowed.', quiet);
			return 0;
		}
		ferror.clear(b);
	}

	return 1;
}

function save() {
	if (ssg.isEditing()) return;
	if (!verifyFields(null, 0)) return;

	var fom = E('t_fom');

	var data = ssg.getAllData();
	var r = [];
	for (var i = 0; i < data.length; ++i) r.push(data[i].join('<'));
	fom.smbd_shares.value = r.join('>');
	fom.smbd_master.value = E('_f_smbd_master').checked ? 1 : 0;

	if (nvram.wan_wins == '' || nvram.wan_wins == '0.0.0.0')
		fom.smbd_wins.value = E('_f_smbd_wins').checked ? 1 : 0;
	else
		fom.smbd_wins.value = nvram.smbd_wins;

	fom.smbd_protocol.value = (E('_smbd_proto_1').checked ? 0 : (E('_smbd_proto_2').checked ? 1 : 2));

	form.submit(fom, 1);
}

function init() {
	var c;
	if (((c = cookie.get(cprefix + '_notes_vis')) != null) && (c == '1')) {
		toggleVisibility(cprefix, "notes");
	}

	var elements = document.getElementsByClassName("new_window");
	for (var i = 0; i < elements.length; i++) if (elements[i].nodeName.toLowerCase()==="a")
		addEvent(elements[i], "click", function(e) { cancelDefaultAction(e); window.open(this,"_blank"); } );
}

function earlyInit() {
	ssg.setup();
	verifyFields(null, true);
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

<input type="hidden" name="_nextpage" value="nas-samba.asp">
<input type="hidden" name="_service" value="samba-restart">
<input type="hidden" name="smbd_master">
<input type="hidden" name="smbd_wins">
<input type="hidden" name="smbd_shares">
<input type="hidden" name="smbd_protocol">

<!-- / / / -->

<div class="section-title">Samba File Sharing</div>
<div class="section">
	<script>
		createFieldTable('', [
			{ title: 'Enable File Sharing', name: 'smbd_enable', type: 'select',
				options: [['0', 'No'],['1', 'Yes, no Authentication'],['2', 'Yes, Authentication required']],
				value: nvram.smbd_enable },
			{ title: 'User Name', indent: 2, name: 'smbd_user', type: 'text', maxlen: 50, size: 32,
				value: nvram.smbd_user },
			{ title: 'Password', indent: 2, name: 'smbd_passwd', type: 'password', maxlen: 50, size: 32, peekaboo: 1,
				value: nvram.smbd_passwd },
			null,
			{ title: 'Samba protocol version', multi: [
				{suffix: '&nbsp; SMBv1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;', name: '_smbd_protocol', id: '_smbd_proto_1', type: 'radio', value: nvram.smbd_protocol == '0' },
				{suffix: '&nbsp; SMBv2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;', name: '_smbd_protocol', id: '_smbd_proto_2', type: 'radio', value: nvram.smbd_protocol == '1' },
				{suffix: '&nbsp; SMBv1 + SMBv2', name: '_smbd_protocol', id: '_smbd_proto_3', type: 'radio', value: nvram.smbd_protocol == '2' } ]},
			{ title: 'Workgroup Name', name: 'smbd_wgroup', type: 'text', maxlen: 20, size: 32,
				value: nvram.smbd_wgroup },
			{ title: 'Client Codepage', name: 'smbd_cpage', type: 'select',
				options: [['', 'Unspecified'],['437', '437 (United States, Canada)'],['850', '850 (Western Europe)'],['852', '852 (Central / Eastern Europe)'],['866', '866 (Cyrillic / Russian)']
				,['932', '932 (Japanese)'],['936', '936 (Simplified Chinese)'],['949', '949 (Korean)'],['950', '950 (Traditional Chinese / Big5)']
				],
				suffix: ' <small> (start cmd.exe and type chcp to see the current code page)<\/small>',
				value: nvram.smbd_cpage },
			{ title: 'Network Interfaces', name: 'smbd_ifnames', type: 'text', maxlen: 50, size: 32,
				suffix: ' <small> (space-delimited)<\/small>',
				value: nvram.smbd_ifnames },
			{ title: 'Samba<br>Custom Configuration', name: 'smbd_custom', type: 'textarea', value: nvram.smbd_custom },
			{ title: 'Auto-share all USB Partitions', name: 'smbd_autoshare', type: 'select',
				options: [['0', 'Disabled'],['1', 'Read Only'],['2', 'Read / Write'],['3', 'Hidden Read / Write']],
				value: nvram.smbd_autoshare },
			{ title: 'Options', multi: [
				{ suffix: '&nbsp; Master Browser &nbsp;&nbsp;&nbsp;', name: 'f_smbd_master', type: 'checkbox', value: nvram.smbd_master == 1 },
				{ suffix: '&nbsp; WINS Server &nbsp;',	name: 'f_smbd_wins', type: 'checkbox', value: (nvram.smbd_wins == 1) && (nvram.wan_wins == '' || nvram.wan_wins == '0.0.0.0') }
			] }
		]);
	</script>
</div>

<!-- / / / -->

<div class="section-title">Additional Shares List</div>
<div class="section">
	<div class="tomato-grid" id="ss-grid"></div>

	<small>When no shares are specified and auto-sharing is disabled, <i>/mnt</i> directory is shared in Read Only mode.</small>
</div>

<!-- / / / -->

<div class="section-title">Notes <small><i><a href="javascript:toggleVisibility(cprefix,'notes');"><span id="sesdiv_notes_showhide">(Click here to show)</span></a></i></small></div>
<div class="section" id="sesdiv_notes" style="display:none">
	<ul>
		<li><b>Network Interfaces</b> - Space-delimited list of router interface names Samba will bind to.
			<ul>
				<li>If empty, <i>interfaces = <% nv("lan_ifname"); %></i> will be used instead.</li>
				<li>The <i>bind interfaces only = yes</i> directive is always set.</li>
				<li>Refer to the <a href="https://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html" class="new_window">Samba documentation</a> for details.</li>
			</ul>
		</li>
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
<script>earlyInit();</script>
</body>
</html>
