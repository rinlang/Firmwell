/*
 * nvram-faker.c - merged reconstruction source
 *
 * This file combines:
 *   1. Original project code from nvram-faker.c.orig (globals, ini_handler, nvram_get)
 *   2. First-batch reconstructed stubs from nvram-faker.c.firstbatch
 *   3. DWARF-derived empty skeleton for the remaining functions
 *
 * Original initialize_ini/end implementations are kept below in a comment
 * for reference; the active versions are the Greenhouse first-batch stubs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include "nvram-faker.h"
//include before ini.h to override ini.h defaults
#include "nvram-faker-internal.h"
#include "ini.h"

static int is_nvram_debug_enabled(void);

#define RED_ON "\033[22;31m"
#define RED_OFF "\033[22;00m"
#define DEFAULT_KV_PAIR_LEN 1024

static int kv_count=0;
static int key_value_pair_len=DEFAULT_KV_PAIR_LEN;
static char **key_value_pairs=NULL;
static char temp[0x200];


static int ini_handler(void *user, const char *section, const char *name,const char *value)
{

    int old_kv_len;
    char **kv;
    char **new_kv;
    int i;
    
    if(NULL == user || NULL == section || NULL == name || NULL == value)
    {
        DEBUG_PRINTF("bad parameter to ini_handler\n");
        return 0;
    }
    kv = *((char ***)user);
    if(NULL == kv)
    {
        LOG_PRINTF("kv is NULL\n");
        return 0;
    }
    
    DEBUG_PRINTF("kv_count: %d, key_value_pair_len: %d\n", kv_count,key_value_pair_len);
    if(kv_count >= key_value_pair_len)
    {
        old_kv_len=key_value_pair_len;
        key_value_pair_len=(key_value_pair_len * 2);
        new_kv=(char **)malloc(key_value_pair_len * sizeof(char **));
        if(NULL == kv)
        {
            LOG_PRINTF("Failed to reallocate key value array.\n");
            return 0;
        }
        for(i=0;i<old_kv_len;i++)
        {
            new_kv[i]=kv[i];
        }
        free(*(char ***)user);
        kv=new_kv;
        *(char ***)user=kv;
    }
    DEBUG_PRINTF("Got %s:%s\n",name,value);
    kv[kv_count++]=strdup(name);
    kv[kv_count++]=strdup(value);
    
    return 1;
}

/*
 * Original reference implementations from nvram-faker.c.orig
 * (not active; Greenhouse binary shows these as empty stubs)
 */
/*
void initialize_ini(void)
{
    int ret;
    DEBUG_PRINTF("Initializing.\n");
    if (NULL == key_value_pairs)
    {
        key_value_pairs=malloc(key_value_pair_len * sizeof(char **));
    }
    if(NULL == key_value_pairs)
    {
        LOG_PRINTF("Failed to allocate memory for key value array. Terminating.\n");
        exit(1);
    }
    
    ret = ini_parse(INI_FILE_PATH,ini_handler,(void *)&key_value_pairs);
    if (0 != ret)
    {
        LOG_PRINTF("ret from ini_parse was: %d\n",ret);
        LOG_PRINTF("INI parse failed. Terminating\n");
        free(key_value_pairs);
        key_value_pairs=NULL;
        exit(1);
    }else
    {
        DEBUG_PRINTF("ret from ini_parse was: %d\n",ret);
    }
    
    return;
    
}
void end(void)
{
    int i;
    for (i=0;i<kv_count;i++)
    {
        free(key_value_pairs[i]);
    }
    free(key_value_pairs);
    key_value_pairs=NULL;
    
    return;
}
*/

void initialize_ini(void)
{
	/* empty in the first batch */
}

void end(void)
{
	/* empty in the first batch */
}

char * replace_char(char * key, char oldchar, char newchar) {
	char *p;

	p = strchr(key, oldchar);
	while (p != NULL) {
		*p = newchar;
		p = strchr(p, oldchar);
	}

	return key;
}


int dump_key(const char * key) {
	char buf[0x100];
	char *p;
	FILE *fp;

	if (!is_nvram_debug_enabled())
		return 0;

	p = strstr(key, "/gh_nvram/");
	if (p != NULL)
		strcpy(buf, p + 10);
	else
		strcpy(buf, key);

	fp = fopen("/MISSING_NVRAMS", "a+");
	if (fp != NULL) {
		fprintf(fp, "%s\n", buf);
		return fclose(fp);
	}
	return -1;
}


char * read_key(const char * key) {
char path[0x108];
char filepath[0x208];
char buf[0x800];
FILE *fp;
size_t n;

if (key == NULL) {
fwrite("[nvram-faker] NULL argument\n", 1, 0x1c, stderr);
return "";
}

memset(path, 0, sizeof(path));
memset(filepath, 0, sizeof(filepath));

snprintf(path, sizeof(path), "%s", key);
replace_char(path, '/', '_');
snprintf(filepath, sizeof(filepath), "/gh_nvram/%s", path);

fflush(stderr);
if (access(filepath, 0) != 0) {
dump_key(key);
}

fp = fopen(filepath, "r");
if (fp == NULL)
return "";

memset(buf, 0, sizeof(buf));
n = fread(buf, 1, sizeof(buf), fp);
fclose(fp);

if (n == 0)
return "";

return strdup(buf);
}



int write_key(const char * key, const char * val) {
char path[0x100];
char filepath[0x200];
FILE *fp;

if (is_nvram_debug_enabled()) {
fp = fopen("NVRAM_ACCESS", "a");
if (fp != NULL) {
fprintf(fp, "[WRITE] %s -> %s\n", key, val);
fclose(fp);
}
}

if (key == NULL || key[0] == '\0')
return 0;
if (val == NULL || val[0] == '\0')
return 0;

memset(path, 0, sizeof(path));
memset(filepath, 0, sizeof(filepath));

snprintf(path, sizeof(path), "%s", key);
replace_char(path, '/', '_');
snprintf(filepath, sizeof(filepath), "/gh_nvram/%s", path);

fp = fopen(filepath, "w+");
if (fp == NULL)
return 0;

fputs(val, fp);
fclose(fp);

if (access("/gh_nvram.ini", 0) == 0)
fp = fopen("/gh_nvram.ini", "a");
else
fp = fopen("/gh_nvram.ini", "w");

fprintf(fp, "%s\n", filepath);
fclose(fp);

return 1;
}



char *nvram_get(const char *key)
{
FILE *fp = NULL;
char *ret;

if (access("/bin/nvram_daemon", 0) == 0) {
if (access("/var", 0) != 0)
mkdir("/var", 0755);
if (access("/var/run", 0) != 0)
mkdir("/var/run", 0755);
fp = fopen("/var/run/nvramd.pid", "r");
if (fp != NULL)
fclose(fp);
}

if (is_nvram_debug_enabled()) {
fp = fopen("NVRAM_ACCESS", "a");
if (fp != NULL)
fprintf(fp, "[GET] %s \n", key);
}

if (key == NULL) {
if (fp != NULL)
fclose(fp);
return "";
}

if (strncmp(key, "noinitrc", 8) == 0)
exit(1);

if (access("/gh_nvram", 0) != 0) {
ret = "";
} else {
ret = read_key(key);
}

if (fp != NULL) {
fprintf(fp, "[GET] %s -> %s\n", key, ret);
fclose(fp);
}
return ret;
}



int nvram_set_default_builtin(void)
{
	return 1;
}

int nvram_set_default(void)
{
	return nvram_set_default_builtin();
}

int nvram_set_default_image(void)
{
	return nvram_set_default_builtin();
}

int nvram_clear(void)
{
return 1;
}



int nvram_close(void)
{
	return 1;
}

int nvram_unset(const char * key) {
	char path[0x200];

	if (access("/gh_nvram", 0) != 0)
		return 0;

	if (key == NULL || key[0] == '\0')
		return 0;

	snprintf(path, sizeof(path), "/gh_nvram/%s", key);
	if (access(path, 0) != 0)
		return 0;

	if (write_key(path, "") == 0)
		return 0;

	return 1;
}


int nvram_set(const char * key, const char * val) {
	if (access("/gh_nvram", 0) != 0)
		return 0;
	return write_key(key, val);
}


int nvram_set_int(const char * key, const int val) {
	char buf[0x204];

	snprintf(buf, sizeof(buf), "%d", val);
	return nvram_set(key, buf);
}


int nvram_list_add(const char * key, const char * val) {
	return nvram_set(key, val);
}


int nvram_set_default_table(const char * * tbl) {
	(void)tbl;
	return 1;
}


int nvram_reset(void) {
	if (nvram_clear() != 1)
		return 0;
	return nvram_set_default();
}


int nvram_list_exist(const char * key, const char * val, int magic)
{
	return 1;
}


int nvram_list_del(const char * key, const char * val)
{
	return 1;
}


int nvram_get_int(const char * key) {
	char *value = nvram_get(key);
	return atoi(value);
}


int nvram_getall(char * buf, size_t len) {
size_t read = 0;
size_t value;
char *line = NULL;
size_t cap = 0;
FILE *fp;
char tmp[0xc00];

if (buf == NULL || len == 0)
return 0;

fp = fopen("/gh_nvram.ini", "r");
if (fp == NULL)
return 0;

while (getline(&line, &cap, fp) != -1) {
char *p = line;
p[strcspn(p, "\n")] = '\0';
if (access(p, 0) != 0)
continue;

{
char *v = read_key(p);
memset(tmp, 0, sizeof(tmp));
snprintf(tmp, sizeof(tmp), "%s=%s", p, v);
value = strlen(tmp);

if (read + value > len) {
free(line);
fclose(fp);
return 0;
}

memcpy(buf + read, tmp, value);
read += value;
buf[read] = '\0';
read++;
}
}
free(line);
fclose(fp);
return 1;
}



int nvram_get_buf(const char * key, char * buf, size_t len) {
	char *value;

	if (buf == NULL || len == 0)
		return 0;

	value = nvram_get(key);
	if (value == NULL)
		return 0;

	buf[0] = '\0';
	strncpy(buf, value, len - 1);
	buf[len - 1] = '\0';
	return 1;
}


int nvram_match(const char * key, const char * val) {
	char *value;

	value = nvram_get(key);
	if (value == NULL)
		return 0;

	if (strlen(val) != strlen(value))
		return 0;

	if (strncmp(value, val, strlen(val)) != 0)
		return 0;

	return 1;
}


int nvram_invmatch(const char * key, const char * val) {
	return nvram_match(key, val) == 0 ? 1 : 0;
}


int nvram_read(const char * key, char * buf, size_t sz) {
	char *value;

	if (buf == NULL)
		return 0;

	value = nvram_get(key);
	if (value == NULL)
		return 0;

	strncpy(buf, value, sz - 1);
	buf[sz - 1] = '\0';
	return 1;
}


char * nvram_safe_get(const char * key) {
	char *value = nvram_get(key);
	return value ? value : "";
}


int nvram_get_state(const char * key)
{
	return nvram_get_int(key);
}


int nvram_init(void)
{
	return 0;
}

int nvram_load(void)
{
	return 0;
}

int nvram_commit(void)
{
	return 1;
}

int nvram_restore(char * path)
{
	return (int)strlen(path);
}


int nvram_backup(char * path)
{
	return (int)strlen(path);
}


int nvram_get_nvramspace(void)
{
	return 0x800;
}

int foreach_nvram_from(const char * file, void (*fp)(const char *, const char *, ...), void * data) {
	FILE *f;
	char *key;
	char *val;
	char *tmp;

	if (fp == NULL)
		return 0;

	f = fopen(file, "r");
	if (f == NULL)
		return 0;

	while (fgets(temp, 0x100, f) != NULL) {
		tmp = strchr(temp, '=');
		if (tmp == NULL)
			continue;
		key = temp;
		*tmp = '\0';
		val = tmp + 1;
		tmp = strchr(val, '\n');
		if (tmp != NULL)
			*tmp = '\0';

		if (data != NULL)
			fp(key, val, data);
		else
			fp(key, val);
	}

	fclose(f);
	return 1;
}


char * nvram_nget(const char * fmt, ...) {
	char buf[0x200];
	va_list ap;

	memset(buf, 0, sizeof(buf));
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return nvram_get(buf);
}


int nvram_nset(const char * val, const char * fmt, ...) {
	char buf[0x200];
	va_list ap;

	memset(buf, 0, sizeof(buf));
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return nvram_set(buf, val);
}


int nvram_nset_int(const int val, const char * fmt, ...) {
	return 1;
}


int nvram_nmatch(const char * val, const char * fmt, ...) {
	char buf[0x200];
	va_list ap;

	memset(buf, 0, sizeof(buf));
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return nvram_match(buf, val);
}


char * nvram_default_get(const char * key, const char * val) {
	char *value;

	value = nvram_get(key);
	if (value != NULL)
		return value;

	if (val != NULL) {
		if (nvram_set(key, val) != 0)
			return nvram_get(key);
	}

	return NULL;
}


char * artblock_get(const char * key)
{
	return nvram_get(key);
}


char * artblock_fast_get(const char * key)
{
	return nvram_get(key);
}


char * artblock_safe_get(const char * key)
{
	return nvram_get(key);
}


int artblock_set(const char * key, const char * val)
{
	return 1;
}


int nvram_flag_set(int unk)
{
	return 1;
}


int nvram_flag_reset(int unk)
{
	return 1;
}


int nvram_master_init(void)
{
	return 0;
}

int nvram_slave_init(void)
{
	return 0;
}

int apmib_init(void)
{
	fwrite("Custom apmib_init called.\n", 1, 0x1a, stderr);
	return 1;
}

int apmib_reinit(void)
{
	return 1;
}

int apmib_update(const int key)
{
	return 1;
}


int apmib_get(const int key, void * buf) {
	char keybuf[0x200];
	char *v;

	memset(keybuf, 0, sizeof(keybuf));
	snprintf(keybuf, sizeof(keybuf), "%d", key);
	v = nvram_get(keybuf);
	if (v != NULL)
		snprintf(buf, 0x200, "%s", v);
	return 1;
}


int apmib_set(const int key, void * val) {
	char keybuf[0x200];

	memset(keybuf, 0, sizeof(keybuf));
	snprintf(keybuf, sizeof(keybuf), "%d", key);
	return nvram_set(keybuf, val);
}


int WAN_ith_CONFIG_GET(char * buf, const char * fmt, ...) {
	char key[0x200];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(key, sizeof(key), fmt, ap);
	va_end(ap);

	return nvram_read(key, buf, 0x40);
}


int WAN_ith_CONFIG_SET_AS_STR(const char * val, const char * fmt, ...) {
	return nvram_nset(val, fmt);
}


int WAN_ith_CONFIG_SET_AS_INT(const int val, const char * fmt, ...) {
	(void)val; (void)fmt;
	return 1;
}


int acos_nvram_init(void)
{
	return nvram_init();
}

char * acos_nvram_get(const char * key)
{
	return nvram_get(key);
}


int acos_nvram_read(const char * key, char * buf, size_t sz) {
	return nvram_read(key, buf, sz);
}


int acos_nvram_set(const char * key, const char * val) {
	return nvram_set(key, val);
}


int acos_nvram_loaddefault(void)
{
	return 1;
}

int acos_nvram_unset(const char * key)
{
	return nvram_unset(key);
}


int acos_nvram_commit(void)
{
	return 1;
}

int acosNvramConfig_init(char * mount)
{
	(void)mount;
	return nvram_init();
}


char * acosNvramConfig_exist(const char * key)
{
	return nvram_get(key);
}


char * acosNvramConfig_get(const char * key)
{
	return nvram_get(key);
}


int acosNvramConfig_read(const char * key, char * buf, size_t sz) {
	return nvram_read(key, buf, sz);
}


int acosNvramConfig_set(const char * key, const char * val) {
	return nvram_set(key, val);
}


int acosNvramConfig_write(const char * key, const char * val) {
	return nvram_set(key, val);
}


int acosNvramConfig_unset(const char * key)
{
	return nvram_unset(key);
}


int acosNvramConfig_match(const char * key, const char * val) {
	return nvram_match(key, val);
}


int acosNvramConfig_invmatch(const char * key, const char * val) {
	return nvram_match(key, val) == 0 ? 1 : 0;
}


int acosNvramConfig_save(void)
{
	return 1;
}

int acosNvramConfig_save_config(void)
{
	return 1;
}

int acosNvramConfig_loadFactoryDefault(const char * key)
{
	return 1;
}


int nvram_getall_adv(int unk, char * buf, size_t len) {
	return nvram_getall(buf, len) == 1 ? 0 : 1;
}


char * nvram_get_adv(int unk, const char * key) {
	return nvram_get(key);
}


int nvram_set_adv(int unk, const char * key, const char * val) {
	return nvram_set(key, val);
}


int nvram_commit_adv(void)
{
	return 1;
}

int nvram_unlock_adv(void)
{
	return 1;
}

int nvram_lock_adv(void)
{
	return 1;
}

int nvram_check(void)
{
	return 1;
}

int nvram_state(int unk1, void * unk2, void * unk3) {
	return 0;
}


int envram_commit(void)
{
	return 0;
}

int envram_default(void)
{
	return 0;
}

int envram_load(void)
{
	return nvram_init() == 0 ? 1 : 0;
}


int envram_safe_load(void)
{
	return nvram_init() == 0 ? 1 : 0;
}


int envram_match(const char * key, const char * val) {
	return nvram_match(key, val) == 0 ? 1 : 0;
}


int envram_get(const char * key, char * buf) {
	return nvram_read(key, buf, 0x40) == 0 ? 1 : 0;
}


int envram_get_func(const char * key, char * buf) {
	return envram_get(key, buf);
}


int envram_getf(const char * key, const char * fmt, ...) {
	char *v;
	va_list ap;

	v = nvram_get(key);
	if (v == NULL)
		return 0;

	va_start(ap, fmt);
	vsscanf(v, fmt, ap);
	va_end(ap);
	free(v);
	return 1;
}


int nvram_getf(const char * key, char * fmt, ...) {
	return envram_get(key, fmt);
}


int envram_set(const char * key, const char * val) {
	return nvram_set(key, val) == 0 ? 1 : 0;
}


int envram_set_func(const char * key, const char * val) {
	return envram_set(key, val);
}


int envram_setf(const char * key, const char * fmt, ...) {
	char buf[0x200];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return nvram_set(key, buf) == 0 ? 1 : 0;
}


int nvram_setf(const char * key, const char * fmt, ...) {
	return envram_setf(key, fmt);
}


int envram_unset(const char * key)
{
	return 0;
}


int acosNvramConfig_readAsInt(char * k, int * r)
{
	return 0;
}


int acosNvramConfig_writeAsInt(char * k, int * r)
{
	return 0;
}


char * nvram_bufget(int idx, const char * key) {
	return nvram_safe_get(key);
}


int nvram_bufset(int idx, const char * key, const char * val) {
	return nvram_set(key, val);
}



/* ===== FirmWell-only first batch: simple stubs ===== */

void acosNvramConfig_setPAParam_RU2(void)
{
/* empty stub */
}

int COMM_ApplyAllNodes(void)
{
return 0;
}

int CommitCfm(void)
{
return 0;
}

int ConnectCfm(void)
{
return 1;
}

int InitCfm(void)
{
return 1;
}

int acMsg_ConnectCfm(void)
{
return 1;
}

int acosNvramConfig_check_set(void)
{
return 1;
}

int acosNvramConfig_readflash(void)
{
return 0;
}

int acosNvramConfig_writeflash(void)
{
return 0;
}

int confmtd_restore(void)
{
return 0;
}

int copy_bssid_default_values(void)
{
return 1;
}

int gemtek_log(void)
{
return 1;
}

int get_factory_to_nvram(void)
{
return 1;
}

int nvram_get_fun(void)
{
return 1;
}

int nvram_get_salt(void)
{
return 1;
}

int nvram_restore_default(void)
{
return 1;
}

int nvram_upgrade(void)
{
return 1;
}

int pw_dec(void)
{
return 1;
}

int pw_inc(void)
{
return 1;
}

int repeater_defaults(void)
{
return 1;
}

int save_cs_to_file(void)
{
return 1;
}

int tdSyslog(void)
{
return 1;
}

int tpi_lan_dhcpc_get_ipinfo_and_status(void)
{
return 1;
}

int tpi_sys_status_get(void)
{
return 1;
}

int uboot_get(void)
{
return 1;
}

/* ===== FirmWell-only second batch: medium stubs/wrappers ===== */

int acosNvramConfig_setPAParam(int a)
{
(void)a;
return 1;
}

int acosNvramConfigInit(int a)
{
(void)a;
return nvram_init();
}

int check_network(int a)
{
(void)a;
return 1;
}

int nvram_kunset(int a)
{
(void)a;
return 1;
}

int CFG_RegisterMID(int a, int b)
{
(void)a; (void)b;
return 0;
}

int MSG_MSG_RegModule(int a, int b)
{
(void)a; (void)b;
return 0;
}

int PC_ProcCtrl(int a, int b)
{
(void)a; (void)b;
return 0;
}

int tbsSystem(int a, int b)
{
(void)a; (void)b;
return 0;
}

int app_tbs_read_mac(int a, int b, int c)
{
(void)a; (void)b; (void)c;
return 0;
}

int SetValue(const char *key, const char *val)
{
return nvram_set(key, val);
}

int _nvram_set(const char *key, const char *val)
{
return nvram_set(key, val);
}

int app_tbs_read_region(char *buf)
{
memcpy(buf, "WW", 3);
return 0;
}

int CFG_get_str(int a, char *buf)
{
(void)a;
memcpy(buf, "deafbeaf", 9);
return 1;
}

int get_pid(void)
{
fwrite("Custom get_pid called.\n", 1, 0x17, stderr);
return 0x186a0;
}

/* ===== FirmWell-only third batch: high complexity ===== */

static int is_nvram_debug_enabled(void)
{
char *e = getenv("NVRAM_DEBUG");
if (e != NULL && strcmp(e, "1") == 0)
return 1;
return 0;
}

char *_nvram_get(const char *key, char *buf, int len)
{
char *ret;

if (access("/gh_nvram", 0) != 0)
return "";

ret = read_key(key);
if (ret == NULL)
return "";

strncpy(buf, ret, len - 1);
buf[len - 1] = '\0';
return ret;
}

int GetValue(const char *key, char *out)
{
FILE *fp = NULL;
char *v;

if (is_nvram_debug_enabled()) {
fp = fopen("NVRAM_ACCESS", "a");
if (fp != NULL) {
fprintf(fp, "[GetValue] %s \n", key);
fflush(fp);
}
}

v = nvram_get(key);
if (v == NULL || v[0] == '\0') {
strcpy(out, "");
if (fp != NULL) {
fprintf(fp, "[GetValue] fail %s: %s \n", key, out);
fflush(fp);
fclose(fp);
}
return 0;
}

strcpy(out, v);
if (fp != NULL) {
fprintf(fp, "[GetValue] %s: %s \n", key, out);
fclose(fp);
}
return 1;
}

int sync_essential_values(int idx)
{
int i = idx - 1;
char buf[0x70];

sprintf(buf, "wl0.%d_%s", i, "infra");
nvram_set(buf, "1");

sprintf(buf, "wl0.%d_%s", i, "mode");
nvram_set(buf, "ap");

sprintf(buf, "wl0.%d_%s", i, "radio");
nvram_set(buf, "1");

sprintf(buf, "wl1.%d_%s", i, "infra");
nvram_set(buf, "1");

sprintf(buf, "wl1.%d_%s", i, "mode");
nvram_set(buf, "ap");

sprintf(buf, "wl1.%d_%s", i, "radio");
nvram_set(buf, "1");

return 0;
}

void *router_defaults;

