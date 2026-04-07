/**
 * @file gui_users.c
 * @brief Multi-user account management implementation.
 *
 * All storage is static (no heap). File I/O via stdio. Plaintext passwords
 * are adequate for this prototype — hash storage recommended for production.
 *
 * @req SWR-SEC-001
 * @req SWR-SEC-002
 * @req SWR-SEC-003
 * @req SWR-GUI-007
 */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include "gui_users.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/* -----------------------------------------------------------------------
 * Static storage
 * ----------------------------------------------------------------------- */
static UserAccount g_users[USERS_MAX_ACCOUNTS];
static int         g_user_count = 0;   /* active slots used */

/* -----------------------------------------------------------------------
 * Internal helpers
 * ----------------------------------------------------------------------- */
static void safe_copy(char *dst, const char *src, int max_len)
{
    strncpy(dst, src, (size_t)(max_len - 1));
    dst[max_len - 1] = '\0';
}

static void get_dat_path(char *out, int out_len)
{
    char exe[MAX_PATH];
    char *sep;
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    sep = strrchr(exe, '\\');
    if (sep) *(sep + 1) = '\0'; else exe[0] = '\0';
    snprintf(out, (size_t)out_len, "%s%s", exe, USERS_DAT_FILENAME);
}

static void load_defaults(void)
{
    g_user_count = 0;
    memset(g_users, 0, sizeof(g_users));

    safe_copy(g_users[0].username,     "admin",           USERS_MAX_USERNAME_LEN);
    safe_copy(g_users[0].display_name, "Administrator",   USERS_MAX_DISPNAME_LEN);
    safe_copy(g_users[0].password,     "Monitor@2026",    USERS_MAX_PASSWORD_LEN);
    g_users[0].role   = ROLE_ADMIN;
    g_users[0].active = 1;

    safe_copy(g_users[1].username,     "clinical",        USERS_MAX_USERNAME_LEN);
    safe_copy(g_users[1].display_name, "Clinical User",   USERS_MAX_DISPNAME_LEN);
    safe_copy(g_users[1].password,     "Clinical@2026",   USERS_MAX_PASSWORD_LEN);
    g_users[1].role   = ROLE_CLINICAL;
    g_users[1].active = 1;

    g_user_count = 2;
}

static UserAccount *find_user(const char *username)
{
    int i;
    if (!username) return NULL;
    for (i = 0; i < USERS_MAX_ACCOUNTS; ++i) {
        if (g_users[i].active && strcmp(g_users[i].username, username) == 0)
            return &g_users[i];
    }
    return NULL;
}

/* -----------------------------------------------------------------------
 * API
 * ----------------------------------------------------------------------- */
void users_init(void)
{
    char path[MAX_PATH];
    char line[256];
    FILE *f;
    int  parsed = 0;

    get_dat_path(path, MAX_PATH);
    f = fopen(path, "r");
    if (!f) { load_defaults(); return; }

    memset(g_users, 0, sizeof(g_users));
    g_user_count = 0;

    while (fgets(line, sizeof(line), f) && g_user_count < USERS_MAX_ACCOUNTS) {
        char *u, *d, *p, *r_str;
        int   role_val;
        /* strip trailing newline */
        char *nl = strrchr(line, '\n'); if (nl) *nl = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;

        u     = strtok(line, "|");
        d     = strtok(NULL, "|");
        p     = strtok(NULL, "|");
        r_str = strtok(NULL, "|");

        if (!u || !d || !p || !r_str) continue;
        role_val = atoi(r_str);
        if (role_val != 0 && role_val != 1) continue;
        if (strlen(u) == 0 || strlen(p) == 0) continue;

        safe_copy(g_users[g_user_count].username,     u, USERS_MAX_USERNAME_LEN);
        safe_copy(g_users[g_user_count].display_name, d, USERS_MAX_DISPNAME_LEN);
        safe_copy(g_users[g_user_count].password,     p, USERS_MAX_PASSWORD_LEN);
        g_users[g_user_count].role   = (UserRole)role_val;
        g_users[g_user_count].active = 1;
        ++g_user_count;
        ++parsed;
    }
    fclose(f);

    if (parsed == 0) load_defaults();
}

int users_authenticate(const char *username, const char *password, UserRole *role_out)
{
    UserAccount *u;
    if (!username || !password) return 0;
    u = find_user(username);
    if (!u) return 0;
    if (strcmp(u->password, password) != 0) return 0;
    if (role_out) *role_out = u->role;
    return 1;
}

void users_display_name_for(const char *username, char *out, int out_len)
{
    UserAccount *u;
    if (!out || out_len <= 0) return;
    out[0] = '\0';
    if (!username) return;
    u = find_user(username);
    if (u) snprintf(out, (size_t)out_len, "%s", u->display_name);
}

int users_change_password(const char *username, const char *old_password,
                           const char *new_password)
{
    UserAccount *u;
    if (!username || !old_password || !new_password) return 0;
    if ((int)strlen(new_password) < USERS_MIN_PASSWORD_LEN) return 0;
    u = find_user(username);
    if (!u) return 0;
    if (strcmp(u->password, old_password) != 0) return 0;
    safe_copy(u->password, new_password, USERS_MAX_PASSWORD_LEN);
    users_save();
    return 1;
}

int users_admin_set_password(const char *username, const char *new_password)
{
    UserAccount *u;
    if (!username || !new_password) return 0;
    if ((int)strlen(new_password) < USERS_MIN_PASSWORD_LEN) return 0;
    u = find_user(username);
    if (!u) return 0;
    safe_copy(u->password, new_password, USERS_MAX_PASSWORD_LEN);
    users_save();
    return 1;
}

int users_add(const char *username, const char *display_name,
               const char *password, UserRole role)
{
    int i, free_slot = -1;
    if (!username || !display_name || !password) return 0;
    if (strlen(username) == 0 || strlen(username) >= USERS_MAX_USERNAME_LEN) return 0;
    if ((int)strlen(password) < USERS_MIN_PASSWORD_LEN) return 0;
    if (find_user(username)) return 0;  /* duplicate */
    if (g_user_count >= USERS_MAX_ACCOUNTS) return 0;

    for (i = 0; i < USERS_MAX_ACCOUNTS; ++i) {
        if (!g_users[i].active) { free_slot = i; break; }
    }
    if (free_slot < 0) return 0;

    memset(&g_users[free_slot], 0, sizeof(UserAccount));
    safe_copy(g_users[free_slot].username,     username,     USERS_MAX_USERNAME_LEN);
    safe_copy(g_users[free_slot].display_name, display_name, USERS_MAX_DISPNAME_LEN);
    safe_copy(g_users[free_slot].password,     password,     USERS_MAX_PASSWORD_LEN);
    g_users[free_slot].role   = role;
    g_users[free_slot].active = 1;
    ++g_user_count;
    users_save();
    return 1;
}

int users_remove(const char *username)
{
    UserAccount *u;
    int i, admin_count = 0;
    if (!username) return 0;
    u = find_user(username);
    if (!u) return 0;

    /* Count remaining admins after removal */
    for (i = 0; i < USERS_MAX_ACCOUNTS; ++i) {
        if (g_users[i].active && g_users[i].role == ROLE_ADMIN
                && &g_users[i] != u)
            ++admin_count;
    }
    /* Enforce: must keep at least one admin */
    if (u->role == ROLE_ADMIN && admin_count == 0) return 0;

    u->active = 0;
    --g_user_count;
    users_save();
    return 1;
}

int users_count(void) { return g_user_count; }

int users_get_by_index(int idx, UserAccount *out)
{
    int i, seen = 0;
    if (!out || idx < 0) return 0;
    for (i = 0; i < USERS_MAX_ACCOUNTS; ++i) {
        if (!g_users[i].active) continue;
        if (seen == idx) { *out = g_users[i]; return 1; }
        ++seen;
    }
    return 0;
}

int users_save(void)
{
    char path[MAX_PATH];
    FILE *f;
    int   i;
    get_dat_path(path, MAX_PATH);
    f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "# Patient Monitor Users v1\n");
    for (i = 0; i < USERS_MAX_ACCOUNTS; ++i) {
        if (!g_users[i].active) continue;
        fprintf(f, "%s|%s|%s|%d\n",
                g_users[i].username,
                g_users[i].display_name,
                g_users[i].password,
                (int)g_users[i].role);
    }
    fclose(f);
    return 1;
}
