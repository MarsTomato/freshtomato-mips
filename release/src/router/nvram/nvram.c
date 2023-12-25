/*
 *
 * NVRAM Utility
 * Copyright (C) 2006-2009 Jonathan Zarate
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#include <bcmdevs.h>
#include <bcmnvram.h>
#include <utils.h>
#include <shutils.h>
#include <shared.h>

#include "nvram_convert.h"
#include "defaults.h"

#define X_QUOTE		0
#define X_SET		1
#define X_C		2
#define X_TAB		3
#define V1		0x31464354L


typedef struct {
	unsigned long sig;
	unsigned long hwid;
	char buffer[NVRAM_SPACE];
} backup_t;

typedef struct {
	const char *name;
	int args;
	int (*main)(int argc, char *argv[]);
} applets_t;


extern int nvram_file2nvram(const char *name, const char *filename);
extern int nvram_nvram2file(const char *name, const char *filename);

__attribute__ ((noreturn))
static void help(void)
{
	printf("NVRAM Utility\n"
	       "Usage: nvram set <key=value> | get <key> | unset <key> |"
#ifdef CONFIG_BCMWL6
	       " default_get <key> |"
#endif
	       "\nren <key> <key> | commit | erase | show [--nosort|--nostat] |\n"
	       "find <text> | defaults <--yes|--initcheck> | backup <filename> |\n"
	       "restore <filename> [--test] [--force] [--forceall] [--nocommit] |\n"
	       "export <--quote|--c|--set|--tab> [--nodefaults] |\n"
	       "export <--dump|--dump0> | import [--forceall] <filename> |\n"
	       "setfb64 <key> <filename> | getfb64 <key> <filename> |\n"
	       "setfile <key> <filename> | getfile <key> <filename> | setfile2nvram <filename> |\n"
	       "convert <infile.cfg> <outfile.txt>\n");

	exit(1);
}

static void getall(char *buffer)
{
	if (nvram_getall(buffer, NVRAM_SPACE) != 0) {
		fprintf(stderr, "Error reading NVRAM\n");
		exit(1);
	}
}

static int set_main(int argc, char **argv)
{
	char *b, *p;

	if ((b = strdup(argv[1])) == NULL) {
		fprintf(stderr, "Not enough memory");
		return 1;
	}
	if ((p = strchr(b, '=')) != NULL) {
		*p = 0;
		nvram_set(b, p + 1);
		if (b)
			free(b);

		return 0;
	}
	help();
}

static int get_main(int argc, char **argv)
{
	char *p;

	if (((p = nvram_get(argv[1])) != NULL) && (*p)) {
		puts(p);
		return 0;
	}

	return 1;
}

static int unset_main(int argc, char **argv)
{
	nvram_unset(argv[1]);

	return 0;
}

static int ren_main(int argc, char **argv)
{
	char *p;

	if ((p = nvram_get(argv[1])) == NULL) {
		fprintf(stderr, "Unable to find %s\n", argv[1]);
		return 1;
	}
	if (strcmp(argv[1], argv[2]) != 0) {
		nvram_set(argv[2], p);
		nvram_unset(argv[1]);
	}

	return 0;
}

static int f2n_main(int argc, char **argv)
{
	return (nvram_file2nvram(argv[1], argv[2]));
}

static int n2f_main(int argc, char **argv)
{
	return (nvram_nvram2file(argv[1], argv[2]));
}

static int save2f_main(int argc, char **argv)
{
	char name[128] = "FILE:";

	strlcpy(name + strlen(name), argv[1], sizeof(name) - strlen(name));

	return (nvram_file2nvram(name, argv[1]));
}

#ifdef CONFIG_BCMWL6
static int default_get_main(int argc, char **argv)
{
	char *p;

	if ((p = nvram_default_get(argv[1])) != NULL) {
		puts(p);
		return 0;
	}

	return 1;
}
#endif

static int show_main(int argc, char **argv)
{
	char *p, *q;
	char buffer[NVRAM_SPACE];
	int n;
	int count;
	int show = 1;
	int stat = 1;
	int sort = 1;

	for (n = 1; n < argc; ++n) {
		if (strcmp(argv[n], "--nostat") == 0)
			stat = 0;
		else if (strcmp(argv[n], "--nosort") == 0)
			sort = 0;
		else
			help();
	}

	if (sort) {
		system("nvram show --nostat --nosort | sort"); /* smallest and easiest way */
		show = 0;
	}

	getall(buffer);
	count = 0;
	for (p = buffer; *p; p += strlen(p) + 1) {
		q = p;
		while (*q) {
			if (!isprint(*q))
				*q = ' ';

			++q;
		}
		if (show)
			puts(p);

		++count;
	}
	if (stat) {
		n = sizeof(struct nvram_header) + (p - buffer);
		printf("---\n%d entries, %d bytes used, %d bytes free.\n", count, n, NVRAM_SPACE - n);
	}

	return 0;
}

static int find_main(int argc, char **argv)
{
	char cmd[512];
	int r;

	snprintf(cmd, sizeof(cmd), "nvram show --nostat --nosort | sort | grep \"%s\"", argv[1]);
	r = system(cmd);

	return (r == -1) ? 1 : WEXITSTATUS(r);
}

static const char *nv_default_value(const defaults_t *t)
{
	int model = get_model();

	if (strcmp(t->key, "wl_txpwr") == 0) {
		switch (model) {
		case MODEL_WHRG54S:
			return "28";
#ifdef CONFIG_BCMWL5
		case MODEL_RTN10:
#if !defined(TCONFIG_BLINK) && !defined(TCONFIG_BCMARM) /* RT only */
		case MODEL_RTN12:
#else
		case MODEL_RTN12A1:
#endif
		case MODEL_RTN16:
			return "17";
#endif
		}
	}
#ifdef TCONFIG_USB
	else if (strcmp(t->key, "usb_enable") == 0) {
		switch (model) {
		case MODEL_WRTSL54GS:
		case MODEL_WL500W:
		case MODEL_WL500GP:
		case MODEL_WL500GPv2:
		case MODEL_WL500GE:
		case MODEL_WL500GD:
		case MODEL_WL520GU:
		case MODEL_DIR320:
		case MODEL_H618B:
		case MODEL_WNR3500L:
		case MODEL_RTN16:
		case MODEL_WRT610Nv2:
		case MODEL_F7D3301:
		case MODEL_F7D3302:
		case MODEL_F7D4301:
		case MODEL_F7D4302:
		case MODEL_F5D8235v3:
#if defined(TCONFIG_BLINK) || defined(TCONFIG_BCMARM) /* RT-N+ */
		case MODEL_F9K1102:
#endif
			return "1";
		}
	}
#endif

	return t->value;
}

static int validate_main(int argc, char **argv)
{
	const defaults_t *t;
	char *p;
	int i;
	int force = 0;
	int unit = 0;

	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--restore") == 0)
			force = 1;
		else if (strncmp(argv[i], "--wl", 4) == 0)
			unit = atoi(argv[i] + 4);
	}

	for (t = defaults; t->key; t++) {
		if (strncmp(t->key, "wl_", 3) == 0) {
			/* sync wl_ and wlX_ */
			p = wl_nvname(t->key + 3, unit, 0);
			if ((force) || (nvram_get(p) == NULL))
				nvram_set(p, t->value);
		}
	}

	return 0;
}

static int defaults_main(int argc, char **argv)
{
	const defaults_t *t;
	char *p;
	char s[256];
	int i, j;
	int force = 0;
	int commit = 0;

	if (strcmp(argv[1], "--yes") == 0)
		force = 1;
	else if (strcmp(argv[1], "--initcheck") != 0)
		help();

	if (!nvram_match("restore_defaults", "0"))
		force = 1;

	if (force)
		nvram_unset("nvram_ver"); /* prep to prevent problems later */

	for (t = defaults; t->key; t++) {
		if (((p = nvram_get(t->key)) == NULL) || (force)) {
			if (t->value == NULL) {
				if (p != NULL) {
					nvram_unset(t->key);
					if (!force)
						_dprintf("%s=%s is not the default (NULL) - resetting\n", t->key, p);

					commit = 1;
				}
			}
			else {
				nvram_set(t->key, nv_default_value(t));
				if (!force)
					_dprintf("%s=%s is not the default (%s) - resetting\n", t->key, p ? p : "(NULL)", nv_default_value(t));

				commit = 1;
			}
		}
		else if (strncmp(t->key, "wl_", 3) == 0) {
			/* sync wl_ and wl0_ */
			strlcpy(s, "wl0_", sizeof(s));
			strlcat(s, t->key + 3, sizeof(s));
			if (nvram_get(s) == NULL)
				nvram_set(s, nvram_safe_get(t->key));
		}
	}

	/* todo: moveme */
	if ((strtoul(nvram_safe_get("boardflags"), NULL, 0) & BFL_ENETVLAN) || (check_hw_type() == HW_BCM4712))
		t = if_vlan;
	else
		t = if_generic;

	for (; t->key; t++) {
		if (((p = nvram_get(t->key)) == NULL) || (*p == 0) || (force)) {
			nvram_set(t->key, t->value);
			commit = 1;
			if (!force)
				_dprintf("%s=%s is not the default (%s) - resetting\n", t->key, p ? p : "(NULL)", t->value);
		}
	}

	if (force) {
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 20; i++) {
				snprintf(s, sizeof(s), "wl%d_wds%d", j, i);
				nvram_unset(s);
			}
		}

		for (i = 0; i < LED_COUNT; ++i) {
			snprintf(s, sizeof(s), "led_%s", led_names[i]);
			nvram_unset(s);
		}
		
		/* 0 = example */
		for (i = 1; i < 50; i++) {
			snprintf(s, sizeof(s), "rrule%d", i);
			nvram_unset(s);
		}
	}

	if (!nvram_match("boot_wait", "on")) {
		nvram_set("boot_wait", "on");
		commit = 1;
	}

	if ((commit) || (force)) {
		printf("Saving...\n");
		nvram_commit();
	}
	else
		printf("No change was necessary.\n");

	return 0;
}

static int defaults_rstats(int argc, char **argv)
{
	const defaults_t *t;
	int add = 0, del = 0;

	if (strcmp(argv[1], "--add") == 0)
		add = 1;
	else if (strcmp(argv[1], "--del") == 0)
		del = 1;
	else
		return 1;

	if (add) {
		/* restore defaults if necessary */
		for (t = rstats_defaults; t->key; t++) {
			if (!nvram_get(t->key)) /* check existence */
				nvram_set(t->key, t->value);
		}
	}

	if (del) {
		if (nvram_match("rstats_enable", "0")) {
			/* remove defaults if NOT necessary (only keep "xyz_enable" nv var.) */
			for (t = rstats_defaults; t->key; t++)
				nvram_unset(t->key);
		}
	}

	return 0;
}

static int defaults_cstats(int argc, char **argv)
{
	const defaults_t *t;
	int add = 0, del = 0;

	if (strcmp(argv[1], "--add") == 0)
		add = 1;
	else if (strcmp(argv[1], "--del") == 0)
		del = 1;
	else
		return 1;

	if (add) {
		/* restore defaults if necessary */
		for (t = cstats_defaults; t->key; t++) {
			if (!nvram_get(t->key)) /* check existence */
				nvram_set(t->key, t->value);
		}
	}

	if (del) {
		if (nvram_match("cstats_enable", "0")) {
			/* remove defaults if NOT necessary (only keep "xyz_enable" nv var.) */
			for (t = cstats_defaults; t->key; t++)
				nvram_unset(t->key);
		}
	}

	return 0;
}

static int defaults_upnp(int argc, char **argv)
{
	const defaults_t *t;
	int add = 0, del = 0;

	if (strcmp(argv[1], "--add") == 0)
		add = 1;
	else if (strcmp(argv[1], "--del") == 0)
		del = 1;
	else
		return 1;

	if (add) {
		/* restore defaults if necessary */
		for (t = upnp_defaults; t->key; t++) {
			if (!nvram_get(t->key)) /* check existence */
				nvram_set(t->key, t->value);
		}
	}

	if (del) {
		if (nvram_match("upnp_enable", "0")) {
		/* remove defaults if NOT necessary (only keep "xyz_enable" nv var.) */
			for (t = upnp_defaults; t->key; t++)
				nvram_unset(t->key);
		}
	}

	return 0;
}

#ifdef TCONFIG_FTP
static int defaults_ftp(int argc, char **argv)
{
	const defaults_t *t;
	int add = 0, del = 0;

	if (strcmp(argv[1], "--add") == 0)
		add = 1;
	else if (strcmp(argv[1], "--del") == 0)
		del = 1;
	else
		return 1;

	if (add) {
		/* restore defaults if necessary */
		for (t = ftp_defaults; t->key; t++) {
			if (!nvram_get(t->key)) /* check existence */
				nvram_set(t->key, t->value);
		}
	}

	if (del) {
		if (nvram_match("ftp_enable", "0")) {
			/* remove defaults if NOT necessary (only keep "xyz_enable" nv var.) */
			for (t = ftp_defaults; t->key; t++)
				nvram_unset(t->key);
		}
	}

	return 0;
}
#endif /* TCONFIG_FTP */

#ifdef TCONFIG_SNMP
static int defaults_snmp(int argc, char **argv)
{
	const defaults_t *t;
	int add = 0, del = 0;

	if (strcmp(argv[1], "--add") == 0)
		add = 1;
	else if (strcmp(argv[1], "--del") == 0)
		del = 1;
	else
		return 1;

	if (add) {
		/* restore defaults if necessary */
		for (t = snmp_defaults; t->key; t++) {
			if (!nvram_get(t->key)) /* check existence */
				nvram_set(t->key, t->value);
		}
	}

	if (del) {
		if (nvram_match("snmp_enable", "0")) {
			/* remove defaults if NOT necessary (only keep "xyz_enable" nv var.) */
			for (t = snmp_defaults; t->key; t++)
				nvram_unset(t->key);
		}
	}

	return 0;
}
#endif /* TCONFIG_SNMP */

static int commit_main(int argc, char **argv)
{
	int r;

	printf("Commit... ");
	fflush(stdout);
	r = nvram_commit();
	printf("done.\n");

	return r ? 1 : 0;
}

static int erase_main(int argc, char **argv)
{
	printf("Erasing nvram...\n");

	return eval("mtd-erase", "-d", "nvram");
}

/*
 * Find nvram param name; return pointer which should be treated as const
 * return NULL if not found.
 *
 * NOTE:  This routine special-cases the variable wl_bss_enabled.  It will
 * return the normal default value if asked for wl_ or wl0_.  But it will
 * return 0 if asked for a virtual BSS reference like wl0.1_.
 */
static const char *get_default_value(const char *name)
{
	char *p;
	const defaults_t *t;
	char fixed_name[NVRAM_MAX_PARAM_LEN + 1];

	if (strncmp(name, "wl", 2) == 0 && isdigit(name[2]) && ((p = strchr(name, '_'))))
		snprintf(fixed_name, sizeof(fixed_name) - 1, "wl%s", p);
	else
		strncpy(fixed_name, name, sizeof(fixed_name));

	if (strcmp(fixed_name, "wl_bss_enabled") == 0) {
		if (name[3] == '.' || name[4] == '.') /* Virtual interface */
			return "0";
	}

	for (t = defaults; t->key; t++) {
		if (strcmp(t->key, name) == 0 || strcmp(t->key, fixed_name) == 0)
			return (t->value ? : "");
	}

	return NULL;
}

static int export_main(int argc, char **argv)
{
	char *p;
	char buffer[NVRAM_SPACE];
	int eq;
	int mode;
	int all, n, skip;
	char *bk, *bv, *v;

	/* C, set, quote */
	static const char *start[4] = { "\"", "nvram set ", "{ \"", "" };
	static const char *stop[4] = { "\"", "\"", "\" },", "" };

	getall(buffer);
	p = buffer;

	all = 1;
	for (n = 1; n < argc; ++n) {
		if (strcmp(argv[n], "--nodefaults") == 0) {
			if (argc < 3)
				help();

			all = 0;
			if (n == 1)
				++argv;

			break;
		}
	}

	if (strcmp(argv[1], "--dump") == 0) {
		if (!all)
			help();

		for (p = buffer; *p; p += strlen(p) + 1) {
			puts(p);
		}
		return 0;
	}

	if (strcmp(argv[1], "--dump0") == 0) {
		if (!all)
			help();

		for (p = buffer; *p; p += strlen(p) + 1) { }
		fwrite(buffer, p - buffer, 1, stdout);
		return 0;
	}

	if (strcmp(argv[1], "--c") == 0)
		mode = X_C;
	else if (strcmp(argv[1], "--set") == 0)
		mode = X_SET;
	else if (strcmp(argv[1], "--tab") == 0)
		mode = X_TAB;
	else if (strcmp(argv[1], "--quote") == 0)
		mode = X_QUOTE;
	else
		help();

	while (*p) {
		eq = 0;

		if (!all) {
			skip = 0;
			bk = p;
			p += strlen(p) + 1;
			bv = strchr(bk, '=');
			*bv++ = 0;

			if ((v = (char *)get_default_value(bk)) != NULL)
				skip = (strcmp(bv, v) == 0);

			*(bv - 1) = '=';
			if (skip)
				continue;
			else
				p = bk;
		}

		printf("%s", start[mode]);
		do {
			switch (*p) {
			case 9:
				if (mode == X_SET)
					putchar(*p);
				else
					printf("\\t");
				break;
			case 13:
				if (mode == X_SET)
					putchar(*p);
				else
					printf("\\r");
				break;
			case 10:
				if (mode == X_SET)
					putchar(*p);
				else
					printf("\\n");
				break;
			case '"':
			case '\\':
				printf("\\%c", *p);
				break;
			case '$':
			case '`':
				if (mode != X_SET)
					putchar(*p);
				else
					printf("\\%c", *p);
				break;
			case '=':
				if ((eq == 0) && (mode > X_QUOTE)) {
					printf((mode == X_C) ? "\", \"" : ((mode == X_SET) ? "=\"" : "\t"));
					eq = 1;
					break;
				}
				eq = 1;
			default:
				if (!isprint(*p))
					printf("\\x%02x", *p);
				else
					putchar(*p);
				break;
			}
			++p;
		} while (*p);

		printf("%s\n", stop[mode]);
		++p;
	}

	return 0;
}

static int in_defaults(const char *key)
{
	const defaults_t *t;
	int n;

	for (t = defaults; t->key; t++) {
		if (strcmp(t->key, key) == 0)
			return 1;
	}

	if ((strncmp(key, "rrule", 5) == 0) && ((n = atoi(key + 5)) > 0) && (n < 50))
		return 1;

	return 0;
}

static int import_main(int argc, char **argv)
{
	FILE *f;
	char s[10240];
	int n;
	char *k, *v;
	char *p, *q;
	int all;
	int same, skip, set;

	all = 0;
	if (strcmp(argv[1], "--forceall") == 0) {
		all = 1;
		++argv;
	}

	if ((f = fopen(argv[1], "r")) == NULL) {
		printf("Error opening file.\n");
		return 1;
	}

	same = skip = set = 0;

	while (fgets(s, sizeof(s), f) != NULL) {
		n = strlen(s);
		while ((--n > 0) && (isspace(s[n])));
		if ((n <= 0) || (s[n] != '"'))
			continue;

		s[n] = 0;

		k = s;
		while (isspace(*k)) ++k;
		if (*k != '"')
			continue;
		++k;

		if ((v = strchr(k, '=')) == NULL)
			continue;

		*v++ = 0;

		p = q = v;
		while (*p) {
			if (*p == '\\') {
				++p;
				switch (*p) {
				case 't':
					*q++ = '\t';
					break;
				case 'r':
					*q++ = '\n';
					break;
				case 'n':
					*q++ = '\n';
					break;
				case '\\':
				case '"':
					*q++ = *p;
					break;
				default:
					printf("Error unescaping %s=%s\n", k, v);
					return 1;
				}
			}
			else
				*q++ = *p;

			++p;
		}
		*q = 0;

		if ((all) || (in_defaults(k))) {
			if (nvram_match(k, v))
				++same;
			else {
				++set;
				printf("%s=%s\n", k, v);
				nvram_set(k, v);
			}
		}
		else
			++skip;
	}

	fclose(f);

	printf("---\n%d skipped, %d same, %d set\n", skip, same, set);

	return 0;
}

static int backup_main(int argc, char **argv)
{
	backup_t data;
	unsigned int size;
	char *p;
	char s[512];
	char tmp[128];
	int r;

	getall(data.buffer);

	data.sig = V1;
	data.hwid = check_hw_type();

	p = data.buffer;
	while (*p != 0)
		p += strlen(p) + 1;

	size = (sizeof(data) - sizeof(data.buffer)) + (p - data.buffer) + 1;

	strlcpy(tmp, "/tmp/nvramXXXXXX", sizeof(tmp));
	mktemp(tmp);
	if (f_write(tmp, &data, size, 0, 0) != (int) size) {
		printf("Error saving file.\n");
		return 1;
	}
	snprintf(s, sizeof(s), "gzip < %s > %s", tmp, argv[1]);
	r = system(s);
	unlink(tmp);

	if (r != 0) {
		unlink(argv[1]);
		printf("Error compressing file.\n");
		return 1;
	}

	printf("Saved.\n");

	return 0;
}

static int restore(char *infile, FILE *ofp, int test, int force, int commit)
{
	backup_t data;
	unsigned int size;
	char s[512];
	char tmp[128];
	unsigned long hw;
	char current[NVRAM_SPACE];
	char *b, *bk, *bv;
	char *c, *ck, *cv;
	int nset;
	int nunset;
	int nsame;
	int cmp;
	unsigned long nbytes = 0;

	strlcpy(tmp, "/tmp/nvramXXXXXX", sizeof(tmp));
	mktemp(tmp);
	snprintf(s, sizeof(s), "gzip -d < %s > %s", infile, tmp);
	if (system(s) != 0) {
		unlink(tmp);
		printf("Error decompressing input file.\n");
		return 1;
	}

	size = f_size(tmp);
	if ((size <= (sizeof(data) - sizeof(data.buffer))) || (size > sizeof(data)) || (f_read(tmp, &data, sizeof(data)) != (int) size)) {
		unlink(tmp);
		printf("Invalid data size or read error.\n");
		return 1;
	}

	unlink(tmp);

	if (data.sig != V1) {
		printf("Invalid signature: %08lX / %08lX\n", data.sig, V1);
		return 1;
	}

	hw = check_hw_type();
	if ((data.hwid != hw) && (!force)) {
		printf("Invalid hardware type: %08lX / %08lX\n", data.hwid, hw);
		return 1;
	}

	/* 1 - check data */
	size -= sizeof(data) - sizeof(data.buffer);
	if ((data.buffer[size - 1] != 0) || (data.buffer[size - 2] != 0)) {
CORRUPT:
		printf("Corrupted data area.\n");
		return 1;
	}

	b = data.buffer;
	while (*b) {
		bk = b;
		b += strlen(b) + 1;
		if ((bv = strchr(bk, '=')) == NULL)
			goto CORRUPT;

		*bv = 0;
		if (strcmp(bk, "et0macaddr") == 0) {
			if (!nvram_match(bk, bv + 1)) {
				if (!force) {
					printf("Cannot restore on a different router.\n");
					return 1;
				}
			}
		}
		*bv = '=';
	}
	if (((b - data.buffer) + 1) != (int) size) {
		printf("Extra data found at the end.\n");
		return 1;
	}

	/* 2 - set */
	if (!test) {
		if (!wait_action_idle(10)) {
			printf("System busy.\n");
			return 1;
		}
		set_action(ACT_SW_RESTORE);
		led(LED_DIAG, 1);
	}

	nset = nunset = nsame = 0;

	b = data.buffer;
	while (*b) {
		bk = b;
		b += strlen(b) + 1;
		bv = strchr(bk, '=');
		*bv++ = 0;

		if (force == 3)
			nbytes += fprintf(ofp, "%s=%s\n", bk, bv);
		else if ((force != 1) || (in_defaults(bk))) {
			if (!nvram_match(bk, bv)) {
				if (test)
					printf("nvram set \"%s=%s\"\n", bk, bv);
				else
					nvram_set(bk, bv);

				++nset;
			}
			else
				++nsame;
		}
		*(bv - 1) = '=';
	}

	/* 3 - unset */
	getall(current);
	c = current;
	while (*c) {
		ck = c;
		c += strlen(c) + 1;
		if ((cv = strchr(ck, '=')) == NULL) {
			printf("Invalid data in NVRAM: %s.", ck);
			continue;
		}
		*cv++ = 0;

		if (force == 3)
			nbytes += fprintf(ofp, "%s=\n", ck);
		else if ((force != 1) || (in_defaults(ck))) {
			cmp = 1;
			b = data.buffer;
			while (*b) {
				bk = b;
				b += strlen(b) + 1;
				bv = strchr(bk, '=');
				*bv++ = 0;
				cmp = strcmp(bk, ck);
				*(bv - 1) = '=';
				if (cmp == 0)
					break;
			}
			if (cmp != 0) {
				++nunset;
				if (test)
					printf("nvram unset \"%s\"\n", ck);
				else
					nvram_unset(ck);
			}
		}
	}

	if ((nset == 0) && (nunset == 0))
		commit = 0;

	if (force != 3)
		printf("\nPerformed %d set and %d unset operations. %d required no changes.\n%s\n", nset, nunset, nsame, commit ? "Committing..." : "Not commiting.");

	fflush(stdout);

	if (!test) {
		set_action(ACT_IDLE);
		if (commit)
			nvram_commit();
	}

	return (force == 3) ? nbytes : 0;
}

static int restore_main(int argc, char **argv)
{
	char *infile;
	int test;
	int force;
	int commit;
	int i, ret;

	test = 0;
	force = 0;
	commit = 1;
	infile = NULL;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "--test") == 0)
				test = 1;
			else if (strcmp(argv[i], "--force") == 0)
				force = 1;
			else if (strcmp(argv[i], "--forceall") == 0)
				force = 2;
			else if (strcmp(argv[i], "--nocommit") == 0)
				commit = 0;
			else
				help();
		}
		else
			infile = argv[i];
	}
	if (!infile)
		help();

	ret = restore(infile, NULL, test, force, commit);

	return ret;
}

static int restore_to_file_main(int argc, char **argv)
{
	char *infile;
	char *outfile;
	FILE *ofp;
	int ret;

	infile = argv[1];
	outfile = argv[2];

	if ((ofp = fopen(outfile, "w+")) == NULL) {
		fprintf(stderr, "Cannot write to output file \"%s\"\n", outfile);
		return 1;
	}

	//ret = restore(infile, buf, ofp);
	ret = restore(infile, ofp, 2, 3, 0);
	// --> restore(infile, outfile, test, force, commit);

	fclose(ofp);

	if (ret > 1)
		fprintf(stderr, "Wrote %d bytes to %s\n", ret, outfile);

	return 0;
}

static int setfb64_main(int argc, char **argv)
{
	if (!nvram_set_file(argv[1], argv[2], 10240)) {
		fprintf(stderr, "Unable to set %s or read %s\n", argv[1], argv[2]);
		return 1;
	}

	return 0;
}

static int getfb64_main(int argc, char **argv)
{
	if (!nvram_get_file(argv[1], argv[2], 10240)) {
		fprintf(stderr, "Unable to get %s or write %s\n", argv[1], argv[2]);
		return 1;
	}

	return 0;
}

static const applets_t applets[] = {
	{ "set",		3,	set_main		},
	{ "get",		3,	get_main		},
	{ "unset",		3,	unset_main		},
	{ "ren",		4,	ren_main		},
	{ "show",		-2,	show_main		},
	{ "commit",		2,	commit_main		},
	{ "erase",		2,	erase_main		},
	{ "find",		3,	find_main		},
	{ "export",		-3,	export_main		},
	{ "import",		-3,	import_main		},
	{ "defaults",		3,	defaults_main		},
	{ "rstats_defaults",	3,	defaults_rstats		},
	{ "cstats_defaults",	3,	defaults_cstats		},
#ifdef TCONFIG_FTP
	{ "ftp_defaults",	3,	defaults_ftp		},
#endif /* TCONFIG_FTP */
#ifdef TCONFIG_SNMP
	{ "snmp_defaults",	3,	defaults_snmp		},
#endif /* TCONFIG_SNMP */
	{ "upnp_defaults",	3,	defaults_upnp		},
	{ "validate",		-3,	validate_main		},
	{ "backup",		3,	backup_main		},
	{ "restore",		-3,	restore_main		},
	{ "setfb64",		4,	setfb64_main		},
	{ "getfb64",		4,	getfb64_main		},
	{ "setfile",		4,	f2n_main		},
	{ "getfile",		4,	n2f_main		},
	{ "setfile2nvram",	3,	save2f_main		},
#ifdef CONFIG_BCMWL6
	{ "default_get",	3,	default_get_main	},
#endif
	{ "convert",		4,	restore_to_file_main	},
	{ NULL, 		0,	NULL			}
};

int main(int argc, char **argv)
{
	const applets_t *a;

	if (argc >= 2) {
		a = applets;
		while (a->name) {
			if (strcmp(argv[1], a->name) == 0) {
				if ((argc != a->args) && ((a->args > 0) || (argc < -(a->args))))
					help();

				return a->main(argc - 1, argv + 1);
			}
			++a;
		}
	}
	help();
}
