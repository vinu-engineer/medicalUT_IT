/**
 * @file gui_auth.c
 * @brief Authentication delegation to gui_users subsystem.
 *
 * All logic lives in gui_users.c. This file provides the public auth_*
 * API defined in gui_auth.h, preserving backward compatibility.
 *
 * @req SWR-GUI-001
 * @req SWR-GUI-002
 * @req SWR-SEC-001
 * @req SWR-SEC-002
 * @req SWR-SEC-003
 */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include "gui_auth.h"
#include "gui_users.h"
#include <string.h>

void auth_init(void)                            { users_init(); }

int  auth_validate(const char *u, const char *p)
     { return users_authenticate(u, p, NULL); }

int  auth_validate_role(const char *u, const char *p, UserRole *r)
     { return users_authenticate(u, p, r); }

void auth_display_name(const char *u, char *out, int len)
     { users_display_name_for(u, out, len); }

int  auth_change_password(const char *u, const char *old, const char *nw)
     { return users_change_password(u, old, nw); }

int  auth_admin_set_password(const char *u, const char *nw)
     { return users_admin_set_password(u, nw); }
