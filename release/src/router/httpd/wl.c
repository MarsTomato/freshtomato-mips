/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "tomato.h"

#include <ctype.h>
#include <wlutils.h>
#include <sys/ioctl.h>
#include <wlscan.h>

#ifndef WL_BSS_INFO_VERSION
#error WL_BSS_INFO_VERSION
#endif
#if WL_BSS_INFO_VERSION < 108
#error WL_BSS_INFO_VERSION < 108
#endif


static int unit = 0;
static int subunit = 0;

static void check_wl_unit(const char *unitarg)
{
	char ifname[12], *wlunit;
	unit = 0; subunit = 0;

	wlunit = (unitarg && *unitarg) ? (char *)unitarg :
		webcgi_safeget("_wl_unit", nvram_safe_get("wl_unit"));
	snprintf(ifname, sizeof(ifname), "wl%s", wlunit);
	get_ifname_unit(ifname, &unit, &subunit);

	_dprintf("check_wl_unit: unitarg: %s, _wl_unit: %s, ifname: %s, unit: %d, subunit: %d\n",
		unitarg, webcgi_safeget("_wl_unit", nvram_safe_get("wl_unit")), ifname, unit, subunit);
}

static void wl_restore(char *wif, int unit, int ap, int radio, int scan_time)
{
	if (ap > 0) {
		wl_ioctl(wif, WLC_SET_AP, &ap, sizeof(ap));

		if (!radio) set_radio(1, unit);
		eval("wl", "-i", wif, "up"); // without this the router may reboot
#if WL_BSS_INFO_VERSION >= 108
		// no idea why this voodoo sequence works to wake up wl	-- zzz
		eval("wl", "-i", wif, "ssid", "");
		eval("wl", "-i", wif, "ssid", nvram_safe_get(wl_nvname("ssid", unit, 0)));
#endif
	}
#ifdef WLC_SET_SCAN_CHANNEL_TIME
	if (scan_time > 0) {
		// restore original scan channel time
		wl_ioctl(wif, WLC_SET_SCAN_CHANNEL_TIME, &scan_time, sizeof(scan_time));
	}
#endif
	set_radio(radio, unit);
}

// allow to scan using up to MAX_WLIF_SCAN wireless ifaces
#define MAX_WLIF_SCAN	3

typedef struct {
	int unit_filter;
	char comma;
	struct {
		int ap;
		int radio;
		int scan_time;
	} wif[MAX_WLIF_SCAN];
} scan_list_t;

static int start_scan(int idx, int unit, int subunit, void *param)
{
	scan_list_t *rp = param;
	wl_scan_params_t sp;
	char *wif;
	int zero = 0;
	int retry;
#ifdef WLC_SET_SCAN_CHANNEL_TIME
	int scan_time = 40;
#endif

	if ((idx >= MAX_WLIF_SCAN) || (rp->unit_filter >= 0 && rp->unit_filter != unit)) return 0;

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));

	memset(&sp, 0xff, sizeof(sp));		// most default to -1
	sp.ssid.SSID_len = 0;
	sp.bss_type = DOT11_BSSTYPE_ANY;	// =2
	sp.channel_num = 0;

	if (wl_ioctl(wif, WLC_GET_AP, &(rp->wif[idx].ap), sizeof(rp->wif[idx].ap)) < 0) {
		// Unable to get AP mode
		return 0;
	}
	if (rp->wif[idx].ap > 0) {
		wl_ioctl(wif, WLC_SET_AP, &zero, sizeof(zero));
	}

	// set scan type based on the ap mode
	sp.scan_type = rp->wif[idx].ap ? DOT11_SCANTYPE_PASSIVE : -1 /* default */;

#ifdef WLC_SET_SCAN_CHANNEL_TIME
	// extend scan channel time to get more AP probe resp
	wl_ioctl(wif, WLC_GET_SCAN_CHANNEL_TIME, &(rp->wif[idx].scan_time), sizeof(rp->wif[idx].scan_time));
	if (rp->wif[idx].scan_time < scan_time) {
		wl_ioctl(wif, WLC_SET_SCAN_CHANNEL_TIME, &scan_time, sizeof(scan_time));
	}
#endif

	rp->wif[idx].radio = get_radio(unit);
	if (!(rp->wif[idx].radio)) set_radio(1, unit);

	retry = 3 * 10;
	while (retry--) {
		if (wl_ioctl(wif, WLC_SCAN, &sp, WL_SCAN_PARAMS_FIXED_SIZE) == 0)
			return 1;
		if (retry) usleep(100000);
	}

	// Unable to start scan
	wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio, rp->wif[idx].scan_time);
	return 0;
}

int wpa_selector_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, WPA_CIPHER_SUITE_NONE, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_NONE_;
	if (memcmp(s, WPA_CIPHER_SUITE_WEP40, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP40_;
	if (memcmp(s, WPA_CIPHER_SUITE_TKIP, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_TKIP_;
	if (memcmp(s, WPA_CIPHER_SUITE_CCMP, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_CCMP_;
	if (memcmp(s, WPA_CIPHER_SUITE_WEP104, WPA_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP104_;
	return 0;
}

int rsn_selector_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, RSN_CIPHER_SUITE_NONE, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_NONE_;
	if (memcmp(s, RSN_CIPHER_SUITE_WEP40, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP40_;
	if (memcmp(s, RSN_CIPHER_SUITE_TKIP, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_TKIP_;
	if (memcmp(s, RSN_CIPHER_SUITE_CCMP, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_CCMP_;
	if (memcmp(s, RSN_CIPHER_SUITE_WEP104, RSN_SELECTOR_LEN) == 0)
		return WPA_CIPHER_WEP104_;
	return 0;
}

int wpa_key_mgmt_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, WPA_AUTH_KEY_MGMT_UNSPEC_802_1X, WPA_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_IEEE8021X_;
	if (memcmp(s, WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X, WPA_SELECTOR_LEN) ==
	    0)
		return WPA_KEY_MGMT_PSK_;
	if (memcmp(s, WPA_AUTH_KEY_MGMT_NONE, WPA_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_WPA_NONE_;
	return 0;
}

int rsn_key_mgmt_to_bitfield(const unsigned char *s)
{
	if (memcmp(s, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X, RSN_SELECTOR_LEN) == 0)
		return WPA_KEY_MGMT_IEEE8021X2_;
	if (memcmp(s, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X, RSN_SELECTOR_LEN) ==
	    0)
		return WPA_KEY_MGMT_PSK2_;
	return 0;
}

int wpa_parse_wpa_ie_wpa(const unsigned char *wpa_ie, size_t wpa_ie_len, struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const unsigned char *pos;
	int left;
	int i, count;

	data->proto = WPA_PROTO_WPA_;
	data->pairwise_cipher = WPA_CIPHER_TKIP_;
	data->group_cipher = WPA_CIPHER_TKIP_;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X_;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
//		fprintf(stderr, "ie len too short %lu", (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != DOT11_MNG_WPA_ID ||
	    hdr->len != wpa_ie_len - 2 ||
	    memcmp(&hdr->oui, WPA_OUI_TYPE_ARR, WPA_SELECTOR_LEN) != 0 ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION_) {
//		fprintf(stderr, "malformed ie or unknown version");
		return -1;
	}

	pos = (const unsigned char *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
//		fprintf(stderr, "ie length mismatch, %u too much", left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (pairwise), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for key mgmt)");
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (key mgmt), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for capabilities)");
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
//		fprintf(stderr, "ie has %u trailing bytes", left);
		return -1;
	}

	return 0;
}

int wpa_parse_wpa_ie_rsn(const unsigned char *rsn_ie, size_t rsn_ie_len, struct wpa_ie_data *data)
{
	const struct rsn_ie_hdr *hdr;
	const unsigned char *pos;
	int left;
	int i, count;

	data->proto = WPA_PROTO_RSN_;
	data->pairwise_cipher = WPA_CIPHER_CCMP_;
	data->group_cipher = WPA_CIPHER_CCMP_;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X2_;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
//		fprintf(stderr, "ie len too short %lu", (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != DOT11_MNG_RSN_ID ||
	    hdr->len != rsn_ie_len - 2 ||
	    WPA_GET_LE16(hdr->version) != RSN_VERSION_) {
//		fprintf(stderr, "malformed ie or unknown version");
		return -1;
	}

	pos = (const unsigned char *) (hdr + 1);
	left = rsn_ie_len - sizeof(*hdr);

	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
//		fprintf(stderr, "ie length mismatch, %u too much", left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (pairwise), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for key mgmt)");
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
//			fprintf(stderr, "ie count botch (key mgmt), "
//				   "count %u left %u", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
//		fprintf(stderr, "ie too short (for capabilities)");
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < data->num_pmkid * PMKID_LEN) {
//			fprintf(stderr, "PMKID underflow "
//				   "(num_pmkid=%d left=%d)", data->num_pmkid, left);
			data->num_pmkid = 0;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

	if (left > 0) {
//		fprintf(stderr, "ie has %u trailing bytes - ignored", left);
	}

	return 0;
}

int wpa_parse_wpa_ie(const unsigned char *wpa_ie, size_t wpa_ie_len,
		     struct wpa_ie_data *data)
{
	if (wpa_ie_len >= 1 && wpa_ie[0] == DOT11_MNG_RSN_ID)
		return wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, data);
	else
		return wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, data);
}

static int get_scan_results(int idx, int unit, int subunit, void *param)
{
	scan_list_t *rp = param;

	if ((idx >= MAX_WLIF_SCAN) || (rp->unit_filter >= 0 && rp->unit_filter != unit)) return 0;

	// get results

	char *wif;
	wl_scan_results_t *results;
	wl_bss_info_t *bssi;
	struct bss_ie_hdr *ie;
	int r;
	int retry;

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));

	results = malloc(WLC_IOCTL_MAXLEN + sizeof(*results));
	if (!results) {
		// Not enough memory
		wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio, rp->wif[idx].scan_time);
		return 0;
	}
	results->buflen = WLC_IOCTL_MAXLEN;
	results->version = WL_BSS_INFO_VERSION;

	// Keep trying to obtain scan results for up to 4 secs
	// Passive scan may require more time, although 1 extra sec is almost always enough.
	retry = 4 * 10;
	r = -1;
	while (retry--) {
		r = wl_ioctl(wif, WLC_SCAN_RESULTS, results, WLC_IOCTL_MAXLEN);
		if (r >= 0) break;
		usleep(100000);
	}

	wl_restore(wif, unit, rp->wif[idx].ap, rp->wif[idx].radio, rp->wif[idx].scan_time);

	if (r < 0) {
		free(results);
		// Unable to obtain scan results
		return 0;
	}

	// format for javascript

	unsigned int i, k;
	int left;
	char macstr[18];
	NDIS_802_11_NETWORK_TYPE NetWorkType;
	unsigned char *bssidp;
	unsigned char rate;

	bssi = &results->bss_info[0];
	for (i = 0; i < results->count; ++i) {

		bssidp = (unsigned char *)&bssi->BSSID;
		sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
				(unsigned char)bssidp[0],
				(unsigned char)bssidp[1],
				(unsigned char)bssidp[2],
				(unsigned char)bssidp[3],
				(unsigned char)bssidp[4],
				(unsigned char)bssidp[5]);

		strcpy(apinfos[0].BSSID, macstr);
		memset(apinfos[0].SSID, 0x0, 33);
		memcpy(apinfos[0].SSID, bssi->SSID, bssi->SSID_len);
		apinfos[0].channel = (uint8)(bssi->chanspec & WL_CHANSPEC_CHAN_MASK);

		if (bssi->ctl_ch == 0)
		{
			apinfos[0].ctl_ch = apinfos[0].channel;
		} else
		{
			apinfos[0].ctl_ch = bssi->ctl_ch;
		}

		if (bssi->RSSI >= -50)
		apinfos[0].RSSI_Quality = 100;
			else if (bssi->RSSI >= -80)	// between -50 ~ -80dbm
		apinfos[0].RSSI_Quality = (int)(24 + ((bssi->RSSI + 80) * 26)/10);
			else if (bssi->RSSI >= -90)	// between -80 ~ -90dbm
		apinfos[0].RSSI_Quality = (int)(((bssi->RSSI + 90) * 26)/10);
			else					// < -84 dbm
		apinfos[0].RSSI_Quality = 0;

		if ((bssi->capability & 0x10) == 0x10)
			apinfos[0].wep = 1;
			else
			apinfos[0].wep = 0;

		apinfos[0].wpa = 0;

		NetWorkType = Ndis802_11DS;
		if ((uint8)(bssi->chanspec & WL_CHANSPEC_CHAN_MASK) <= 14)
		{
			for (k = 0; k < bssi->rateset.count; k++)
			{
				rate = bssi->rateset.rates[k] & 0x7f;	// Mask out basic rate set bit
				if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
					continue;
				else
				{
					NetWorkType = Ndis802_11OFDM24;
					break;
				}
			}
		}
		else
			NetWorkType = Ndis802_11OFDM5;

		if (bssi->n_cap)
		{
			if (NetWorkType == Ndis802_11OFDM5)
			{
#ifdef CONFIG_BCMWL6
				if (bssi->vht_cap)
					NetWorkType = Ndis802_11OFDM5_VHT;
				else
#endif
				NetWorkType = Ndis802_11OFDM5_N;
			}
			else
				NetWorkType = Ndis802_11OFDM24_N;
		}

		apinfos[0].NetworkType = NetWorkType;

		ie = (struct bss_ie_hdr *) ((unsigned char *) bssi + sizeof(*bssi));
			for (left = bssi->ie_length; left > 0; // look for RSN IE first
				left -= (ie->len + 2), ie = (struct bss_ie_hdr *) ((unsigned char *) ie + 2 + ie->len))
				{
					if (ie->elem_id != DOT11_MNG_RSN_ID)
						continue;

					if (wpa_parse_wpa_ie(&ie->elem_id, ie->len + 2, &apinfos[0].wid) == 0)
					{
						apinfos[0].wpa = 1;
						goto next_info;
					}
		}

		ie = (struct bss_ie_hdr *) ((unsigned char *) bssi + sizeof(*bssi));
			for (left = bssi->ie_length; left > 0; // then look for WPA IE
				left -= (ie->len + 2), ie = (struct bss_ie_hdr *) ((unsigned char *) ie + 2 + ie->len))
				{
				if (ie->elem_id != DOT11_MNG_WPA_ID)
					continue;

				if (wpa_parse_wpa_ie(&ie->elem_id, ie->len + 2, &apinfos[0].wid) == 0)
				{
					apinfos[0].wpa = 1;
					break;
			}
		}
next_info:
#ifdef CONFIG_BCMWL6
		web_printf("%c['%s','%s',%d,%d,%d,%d,", rp->comma,
			apinfos[0].BSSID, apinfos[0].SSID,bssi->RSSI,apinfos[0].channel,
			 (CHSPEC_IS80(bssi->chanspec) ? 80 : (CHSPEC_IS40(bssi->chanspec) ? 40 : (CHSPEC_IS20(bssi->chanspec) ? 20 : 10))),apinfos[0].RSSI_Quality);
		rp->comma = ',';
#else
		web_printf("%c['%s','%s',%d,%d,%d,%d,", rp->comma,
			apinfos[0].BSSID, apinfos[0].SSID,bssi->RSSI,apinfos[0].channel,
			CHSPEC_IS40(bssi->chanspec) ? 40 : (CHSPEC_IS20(bssi->chanspec) ? 20 : 10),apinfos[0].RSSI_Quality);
		rp->comma = ',';
#endif

		if (apinfos[0].NetworkType == Ndis802_11FH || apinfos[0].NetworkType == Ndis802_11DS)
				web_printf("'%s',", "11b");
		else if (apinfos[0].NetworkType == Ndis802_11OFDM5)
				web_printf("'%s',", "11a");
		else if (apinfos[0].NetworkType == Ndis802_11OFDM5_VHT)
				web_printf("'%s',", "11ac");
		else if (apinfos[0].NetworkType == Ndis802_11OFDM5_N)
				web_printf("'%s',", "11a/n");
		else if (apinfos[0].NetworkType == Ndis802_11OFDM24)
				web_printf("'%s',", "11b/g");
		else if (apinfos[0].NetworkType == Ndis802_11OFDM24_N)
				web_printf("'%s',", "11b/g/n");
		else
				web_printf("'%s',", "unknown");

		if (apinfos[0].wpa == 1) {
			if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X_)
				web_printf("'%s',", "WPA-Enterprise");
			else if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X2_)
				web_printf("'%s',", "WPA2-Enterprise");
			else if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_PSK_)
				web_printf("'%s',", "WPA-Personal");
			else if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_PSK2_)
				web_printf("'%s',", "WPA2-Personal");
			else if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_NONE_)
				web_printf("'%s',", "NONE");
			else if (apinfos[0].wid.key_mgmt == WPA_KEY_MGMT_IEEE8021X_NO_WPA_)
				web_printf("'%s',", "IEEE 802.1X");
			else
				web_printf("'%s',", "Unknown");
		} else if (apinfos[0].wep == 1) {
			web_printf("'%s',", "Unknown");
		} else {
			web_printf("'%s',", "Open System");
		}

		if (apinfos[0].wpa == 1) {
			if (apinfos[0].wid.pairwise_cipher == WPA_CIPHER_NONE_)
				web_printf("'%s',", "NONE");
			else if (apinfos[0].wid.pairwise_cipher == WPA_CIPHER_WEP40_)
				web_printf("'%s',", "WEP");
			else if (apinfos[0].wid.pairwise_cipher == WPA_CIPHER_WEP104_)
				web_printf("'%s',", "WEP");
			else if (apinfos[0].wid.pairwise_cipher == WPA_CIPHER_TKIP_)
				web_printf("'%s',", "TKIP");
			else if (apinfos[0].wid.pairwise_cipher == WPA_CIPHER_CCMP_)
				web_printf("'%s',", "AES");
			else if (apinfos[0].wid.pairwise_cipher == (WPA_CIPHER_TKIP_|WPA_CIPHER_CCMP_))
				web_printf("'%s',", "TKIP+AES");
			else
				web_printf("'%s',", "Unknown");
		} else if (apinfos[0].wep == 1) {
			web_printf("'%s',", "WEP");
		} else {
			web_printf("'%s',", "NONE");
		}
		web_printf("'%s']", CHSPEC_IS2G(bssi->chanspec)?"2.4":"5");

		bssi = (wl_bss_info_t*)((uint8*)bssi + bssi->length);
	}
	free(results);

	return 1;
}

//	returns: ['bssid','ssid',channel,capabilities,rssi,noise,[rates,]],  or  [null,'error message']
void asp_wlscan(int argc, char **argv)
{
	scan_list_t rp;

	memset(&rp, 0, sizeof(rp));
	rp.comma = ' ';
	rp.unit_filter = (argc > 0) ? atoi(argv[0]) : (-1);

	web_puts("\nwlscandata = [");

	// scan

	if (foreach_wif(0, &rp, start_scan) == 0) {
		web_puts("[null,'Unable to start scan.']];\n");
		return;
	}
	sleep(1);

	// get results

	if (foreach_wif(0, &rp, get_scan_results) == 0) {
		web_puts("[null,'Unable to obtain scan results.']];\n");
		return;
	}

	web_puts("];\n");
}

void wo_wlradio(char *url)
{
	char *enable;
	char sunit[10];

	check_wl_unit(NULL);

	parse_asp("saved.asp");
	if (nvram_get_int(wl_nvname("radio", unit, 0))) {
		if ((enable = webcgi_get("enable")) != NULL) {
			web_close();
			sleep(2);
			sprintf(sunit, "%d", unit);
			eval("radio", atoi(enable) ? "on" : "off", sunit);
			return;
		}
	}
}

static int read_noise(int unit)
{
	int v;

	// WLC_GET_PHY_NOISE = 135
	if (wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, 0)), 135, &v, sizeof(v)) == 0) {
		char s[32];
		sprintf(s, "%d", v);
		nvram_set(wl_nvname("tnoise", unit, 0), s);
		return v;
	}
	return -99;
}

static int get_wlnoise(int client, int unit)
{
	int v;

	if (client) {
		v = read_noise(unit);
	}
	else {
		v = nvram_get_int(wl_nvname("tnoise", unit, 0));
		if ((v >= 0) || (v < -100)) v = -99;
	}
	return v;
}

static int print_wlnoise(int idx, int unit, int subunit, void *param)
{
	web_printf("%c%d", (idx == 0) ? ' ' : ',', get_wlnoise(wl_client(unit, 0), unit));
	return 0;
}

void asp_wlnoise(int argc, char **argv)
{
	web_puts("\nwlnoise = [");
	foreach_wif(0, NULL, print_wlnoise);
	web_puts(" ];\n");
}

void wo_wlmnoise(char *url)
{
	int ap;
	int i;
	char *wif;

	check_wl_unit(NULL);

	parse_asp("mnoise.asp");
	web_close();
	sleep(3);

	int radio = get_radio(unit);

	wif = nvram_safe_get(wl_nvname("ifname", unit, 0));
	if (wl_ioctl(wif, WLC_GET_AP, &ap, sizeof(ap)) < 0) return;

	i = 0;
	wl_ioctl(wif, WLC_SET_AP, &i, sizeof(i));

	for (i = 10; i > 0; --i) {
		sleep(1);
		read_noise(unit);
	}

	wl_restore(wif, unit, ap, radio, 0);
}

static int wl_chanfreq(uint ch, int band)
{
	if ((band == WLC_BAND_2G && (ch < 1 || ch > 14)) || (ch > 200))
		return -1;
	else if ((band == WLC_BAND_2G) && (ch == 14))
		return 2484;
	else
		return ch * 5 + ((band == WLC_BAND_2G) ? 4814 : 10000) / 2;
}

static int not_wlclient(int idx, int unit, int subunit, void *param)
{
	return (!wl_client(unit, subunit));
}

// returns '1' if all wireless interfaces are in client mode, '0' otherwise
void asp_wlclient(int argc, char **argv)
{
	web_puts(foreach_wif(1, NULL, not_wlclient) ? "0" : "1");
}

static int print_wlstats(int idx, int unit, int subunit, void *param)
{
	int phytype;
	int rate, client, nbw;
	int chanspec, channel, mhz, band, scan;
	int chanim_enab;
	int interference;
	char retbuf[WLC_IOCTL_SMLEN];
	scb_val_t rssi;
	char *ifname, *ctrlsb;

	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	client = wl_client(unit, 0);

	/* Get configured phy type */
	wl_ioctl(ifname, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));

	if (wl_ioctl(ifname, WLC_GET_RATE, &rate, sizeof(rate)) < 0)
		rate = 0;

	if (wl_ioctl(ifname, WLC_GET_BAND, &band, sizeof(band)) < 0)
		band = nvram_get_int(wl_nvname("nband", unit, 0));

	channel = nvram_get_int(wl_nvname("channel", unit, 0));
	scan = 0;
	interference = -1;

#ifdef CONFIG_BCMWL6
	if (wl_phytype_n(phytype) || phytype == WLC_PHY_TYPE_AC) {
#else
	if (wl_phytype_n(phytype)) {
#endif
		if (wl_iovar_getint(ifname, "chanspec", &chanspec) != 0) {
			ctrlsb = nvram_safe_get(wl_nvname("nctrlsb", unit, 0));
			nbw = nvram_get_int(wl_nvname("nbw", unit, 0));
		}
		else {
			channel = CHSPEC_CHANNEL(chanspec);
			if (CHSPEC_IS40(chanspec))
				channel = channel + (CHSPEC_SB_LOWER(chanspec) ? -2 : 2);
#ifdef CONFIG_BCMWL6
                	else if(CHSPEC_IS80(chanspec))
				channel += (((chanspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LLU) ? -2 : -6 ); //upper is acturally LLU

			ctrlsb =  (chanspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER ? "lower" : ((chanspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LLU ? "upper" : "none");
			nbw = CHSPEC_IS80(chanspec) ? 80 : (CHSPEC_IS40(chanspec) ? 40 : 20);
#else
			ctrlsb = CHSPEC_SB_LOWER(chanspec) ? "lower" : (CHSPEC_SB_UPPER(chanspec) ? "upper" : "none");
			nbw = CHSPEC_IS40(chanspec) ? 40 : 20;
#endif
		}
	}
	else {
		channel_info_t ch;
		if (wl_ioctl(ifname, WLC_GET_CHANNEL, &ch, sizeof(ch)) == 0) {
			scan = (ch.scan_channel > 0);
			channel = (scan) ? ch.scan_channel : ch.hw_channel;
		}
		ctrlsb = "";
		nbw = 20;
	}

	mhz = (channel) ? wl_chanfreq(channel, band) : 0;
	if (wl_iovar_getint(ifname, "chanim_enab", (int*)(void*)&chanim_enab) != 0)
		chanim_enab = 0;
	if (chanim_enab) {
		if (wl_iovar_getbuf(ifname, "chanim_state", &chanspec, sizeof(chanspec), retbuf, WLC_IOCTL_SMLEN) == 0)
			interference = *(int*)retbuf;
	}

	memset(&rssi, 0, sizeof(rssi));
	if (client) {
		if (wl_ioctl(ifname, WLC_GET_RSSI, &rssi, sizeof(rssi)) != 0)
			rssi.val = -100;
	}

	// [ radio, is_client, channel, freq (mhz), rate, nctrlsb, nbw, rssi, noise, interference ]
	web_printf("%c{ radio: %d, client: %d, channel: %c%d, mhz: %d, rate: %d, ctrlsb: '%s', nbw: %d, rssi: %d, noise: %d, intf: %d }\n",
		(idx == 0) ? ' ' : ',',
		get_radio(unit), client, (scan ? '-' : ' '), channel, mhz, rate, ctrlsb, nbw, rssi.val, get_wlnoise(client, unit), interference);

	return 0;
}

void asp_wlstats(int argc, char **argv)
{
	int include_vifs = (argc > 0) ? atoi(argv[0]) : 0;

	web_puts("\nwlstats = [");
	foreach_wif(include_vifs, NULL, print_wlstats); // AB multiSSID
	web_puts("];\n");
}

static void web_print_wlchan(uint chan, int band)
{
	int mhz;
	if ((mhz = wl_chanfreq(chan, band)) > 0)
		web_printf(",[%d, %d]", chan, mhz);
	else
		web_printf(",[%d, 0]", chan);
}

static int _wlchanspecs(char *ifname, char *country, int band, int bw, int ctrlsb)
{
	chanspec_t c = 0, *chanspec;
	int buflen;
	wl_uint32_list_t *list;
	unsigned int count, i = 0;

	char *buf = (char *)malloc(WLC_IOCTL_MAXLEN);
	if (!buf)
		return 0;

	strcpy(buf, "chanspecs");
	buflen = strlen(buf) + 1;

	c |= (band == WLC_BAND_5G) ? WL_CHANSPEC_BAND_5G : WL_CHANSPEC_BAND_2G;
#ifdef CONFIG_BCMWL6
	c |= (bw == 20) ? WL_CHANSPEC_BW_20 : (bw == 40 ? WL_CHANSPEC_BW_40 : WL_CHANSPEC_BW_80);
#else
	c |= (bw == 20) ? WL_CHANSPEC_BW_20 : WL_CHANSPEC_BW_40;
#endif

	chanspec = (chanspec_t *)(buf + buflen);
	*chanspec = c;
	buflen += (sizeof(chanspec_t));
	strncpy(buf + buflen, country, WLC_CNTRY_BUF_SZ);
	buflen += WLC_CNTRY_BUF_SZ;

	/* Add list */
	list = (wl_uint32_list_t *)(buf + buflen);
	list->count = WL_NUMCHANSPECS;
	buflen += sizeof(uint32)*(WL_NUMCHANSPECS + 1);

	if (wl_ioctl(ifname, WLC_GET_VAR, buf, buflen) < 0) {
		free((void *)buf);
		return 0;
	}

	count = 0;
	list = (wl_uint32_list_t *)buf;
	for (i = 0; i < list->count; i++) {
		c = (chanspec_t)list->element[i];
		/* Skip upper.. (take only one of the lower or upper) */
		if (bw >= 40 && (CHSPEC_CTL_SB(c) != ctrlsb))
			continue;
		/* Create the actual control channel number from sideband */
		int chan = CHSPEC_CHANNEL(c);
		if (bw == 40)
			chan += ((ctrlsb == WL_CHANSPEC_CTL_SB_UPPER) ? 2 : -2);
#ifdef CONFIG_BCMWL6
		else if(bw == 80)
			chan += ((ctrlsb == WL_CHANSPEC_CTL_SB_UPPER) ? -2 : -6 ); //upper is acturally LLU
#endif
		web_print_wlchan(chan, band);
		count++;
	}

	free((void *)buf);
	return count;
}

static void _wlchannels(char *ifname, char *country, int band)
{
	unsigned int i;
	wl_channels_in_country_t *cic;

	cic = (wl_channels_in_country_t *)malloc(WLC_IOCTL_MAXLEN);
	if (cic) {
		cic->buflen = WLC_IOCTL_MAXLEN;
		strcpy(cic->country_abbrev, country);
		cic->band = band;

		if (wl_ioctl(ifname, WLC_GET_CHANNELS_IN_COUNTRY, cic, cic->buflen) == 0) {
			for (i = 0; i < cic->count; i++) {
				web_print_wlchan(cic->channel[i], band);
			}
		}
		free((void *)cic);
	}
}

void asp_wlchannels(int argc, char **argv)
{
	char buf[WLC_CNTRY_BUF_SZ];
	int band, phytype, nphy;
	int bw, ctrlsb, chanspec;
	char *ifname;

	// args: unit, nphy[1|0], bw, band, ctrlsb

	check_wl_unit(argc > 0 ? argv[0] : NULL);

	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	wl_ioctl(ifname, WLC_GET_COUNTRY, buf, sizeof(buf));
	if (wl_ioctl(ifname, WLC_GET_BAND, &band, sizeof(band)) != 0)
		band = nvram_get_int(wl_nvname("nband", unit, 0));
	wl_iovar_getint(ifname, "chanspec", &chanspec);

	if (argc > 1)
		nphy = atoi(argv[1]);
	else {
		wl_ioctl(ifname, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));
#ifdef CONFIG_BCMWL6
		nphy = wl_phytype_n(phytype) || phytype == WLC_PHY_TYPE_AC;
#else
		nphy = wl_phytype_n(phytype);
#endif
	}

	bw = (argc > 2) ? atoi(argv[2]) : 0;
#ifdef CONFIG_BCMWL6
	bw = bw ? : CHSPEC_IS80(chanspec) ? 80 : (CHSPEC_IS40(chanspec) ? 40 : 20);
#else
	bw = bw ? : CHSPEC_IS40(chanspec) ? 40 : 20;
#endif
	if (argc > 3) band = atoi(argv[3]) ? : band;

	if (argc > 4) {
		if (strcmp(argv[4], "upper") == 0)
			ctrlsb = WL_CHANSPEC_CTL_SB_UPPER;
		else
			ctrlsb = WL_CHANSPEC_CTL_SB_LOWER;
	}
	else
		ctrlsb = CHSPEC_CTL_SB(chanspec);

	web_puts("\nwl_channels = [\n[0, 0]");
	if (nphy) {
		if (!_wlchanspecs(ifname, buf, band, bw, ctrlsb) && band == WLC_BAND_2G && bw == 40)
			_wlchanspecs(ifname, buf, band, 20, ctrlsb);
	}
	else
		_wlchannels(ifname, buf, band);
	web_puts("];\n");
}

static int print_wlbands(int idx, int unit, int subunit, void *param)
{
	char *phytype, *phylist, *ifname;
	char comma = ' ';
	int list[WLC_BAND_ALL];
	unsigned int i;

	ifname = nvram_safe_get(wl_nvname("ifname", unit, 0));
	phytype = nvram_safe_get(wl_nvname("phytype", unit, 0));

	web_printf("%c[", (idx == 0) ? ' ' : ',');

	if (phytype[0] == 'n' ||
	    phytype[0] == 'l' ||
	    phytype[0] == 's' ||
	    phytype[0] == 'c' ||
	    phytype[0] == 'v' ||
	    phytype[0] == 'h') {
		/* Get band list. Assume both the bands in case of error */
		if (wl_ioctl(ifname, WLC_GET_BANDLIST, list, sizeof(list)) < 0) {
			for (i = WLC_BAND_5G; i <= WLC_BAND_2G; i++) {
				web_printf("%c'%d'", comma, i);
				comma = ',';
			}
		}
		else {
			if (list[0] > 2)
				list[0] = 2;
			for (i = 1; i <= (unsigned int) list[0]; i++) {
				web_printf("%c'%d'", comma, list[i]);
				comma = ',';
			}
		}
	}
	else {
		/* Get available phy types of the currently selected wireless interface */
		phylist = nvram_safe_get(wl_nvname("phytypes", unit, 0));
		for (i = 0; i < strlen(phylist); i++) {
			web_printf("%c'%d'", comma, phylist[i] == 'a' ? WLC_BAND_5G : WLC_BAND_2G);
			comma = ',';
		}
	}

	web_puts("]");

	return 0;
}

void asp_wlbands(int argc, char **argv)
{
	int include_vifs = (argc > 0) ? atoi(argv[0]) : 0;

	web_puts("\nwl_bands = [");
	foreach_wif(include_vifs, NULL, print_wlbands); // AB multiSSID
	web_puts(" ];\n");
}

static int print_wif(int idx, int unit, int subunit, void *param)
{
	char unit_str[] = "000000";
	char *ssidj;

	char *next;
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_SMLEN];
	int max_no_vifs = 0;

	if (subunit > 0) {
		snprintf(unit_str, sizeof(unit_str), "%d.%d", unit, subunit);
	} else {
		snprintf(unit_str, sizeof(unit_str), "%d", unit);

//		wl_iovar_get(wl_nvname("ifname", unit, 0), "cap", (void *)caps, WLC_IOCTL_SMLEN);
//		wl_iovar_get("eth1", "cap", (void *)caps, WLC_IOCTL_SMLEN);
		max_no_vifs = 1;
		wl_iovar_get(nvram_safe_get(wl_nvname("ifname", unit, 0)), "cap", (void *)caps, WLC_IOCTL_SMLEN);
		foreach(cap, caps, next) {
			if (!strcmp(cap, "mbss16"))
				max_no_vifs = 16;
			if (!strcmp(cap, "mbss4"))
				max_no_vifs = 4;
		}

	}

	int up = 0;
	int sfd;
	struct ifreq ifr;

	if ((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		_dprintf("[%s %d]: error opening socket %m\n", __FUNCTION__, __LINE__);
	}

	if (sfd >= 0) {
		strcpy(ifr.ifr_name, nvram_safe_get(wl_nvname("ifname", unit, subunit)));
		if (ioctl(sfd, SIOCGIFFLAGS, &ifr) == 0)
			if (ifr.ifr_flags & (IFF_UP | IFF_RUNNING))
				up = 1;
		close(sfd);
	}

	char *ifname;
	ifname = nvram_safe_get(wl_nvname("ifname", unit, subunit));
	struct ether_addr bssid;

	wl_ioctl(ifname, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN);

	// [ifname, unitstr, unit, subunit, ssid, hwaddr, up, wmode, bssid]
	ssidj = js_string(nvram_safe_get(wl_nvname("ssid", unit, subunit)));
	web_printf("%c['%s','%s',%d,%d,'%s','%s',%d,%d,'%s','%02X:%02X:%02X:%02X:%02X:%02X']", (idx == 0) ? ' ' : ',',
		nvram_safe_get(wl_nvname("ifname", unit, subunit)),
		unit_str, unit, subunit, ssidj,
//		// virtual inteface MAC address
		nvram_safe_get(wl_nvname("hwaddr", unit, subunit)), up, max_no_vifs, // AB multiSSID
		nvram_safe_get(wl_nvname("mode", unit, subunit)),
		bssid.octet[0], bssid.octet[1], bssid.octet[2], bssid.octet[3], bssid.octet[4], bssid.octet[5]
	);
	free(ssidj);

	return 0;
}

void asp_wlifaces(int argc, char **argv)
{
	int include_vifs = (argc > 0) ? atoi(argv[0]) : 0;

	web_puts("\nwl_ifaces = [");
	foreach_wif(include_vifs, NULL, print_wif);
	web_puts("];\n");
}

void asp_wlcountries(int argc, char **argv)
{
	char s[128], *p, *code, *country;
	FILE *f;
	int i = 0;

	web_puts("\nwl_countries = [");
	if ((f = popen("wl country list", "r")) != NULL) {
		// skip the header line
		fgets(s, sizeof(s), f);
		while (fgets(s, sizeof(s), f)) {
			p = s;
			if ((code = strsep(&p, " \t\n")) && p) {
				country = strsep(&p, "\n");
				if ((country && *country && strcmp(code, country) != 0) ||
				    // special case EU code since the driver may not have a name for it
				    (strcmp(code, "EU") == 0)) {
					if (!country || *country == 0) country = code;
					p = js_string(country);
					web_printf("%c['%s', '%s']", i++ ? ',' : ' ', code, p);
					free(p);
				}
			}
		}
		pclose(f);
	}
	web_puts("];\n");
}
