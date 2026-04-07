/**
 * @file gui_auth.c
 * @brief GUI authentication implementation.
 *
 * Fixed built-in credential for the clinical workstation login screen.
 * The credential is compiled in; no external credential store or network
 * call is made, satisfying the static-memory requirement (SYS-012).
 *
 * Default login:  username = admin   password = Monitor@2026
 *
 * @req SWR-GUI-001
 * @req SWR-GUI-002
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gui_auth.h"
#include <string.h>
#include <stdio.h>

/* -----------------------------------------------------------------------
 * Built-in credential  (change here only — never store in external files)
 * ----------------------------------------------------------------------- */
#define BUILT_IN_USERNAME "admin"
#define BUILT_IN_PASSWORD "Monitor@2026"
#define DISPLAY_NAME      "Dr. Admin"

/* -----------------------------------------------------------------------
 * auth_validate
 * ----------------------------------------------------------------------- */
int auth_validate(const char *username, const char *password)
{
    if (username == NULL || password == NULL) {
        return 0;
    }
    return (strcmp(username, BUILT_IN_USERNAME) == 0 &&
            strcmp(password, BUILT_IN_PASSWORD) == 0) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 * auth_display_name
 * ----------------------------------------------------------------------- */
void auth_display_name(const char *username, char *out, int out_len)
{
    if (out == NULL || out_len <= 0) {
        return;
    }
    if (username != NULL && strcmp(username, BUILT_IN_USERNAME) == 0) {
        snprintf(out, (size_t)out_len, "%s", DISPLAY_NAME);
    } else {
        out[0] = '\0';
    }
}
