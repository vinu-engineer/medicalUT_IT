/**
 * @file gui_auth.h
 * @brief GUI authentication interface — multi-user, role-aware.
 *
 * Wraps the gui_users subsystem for login and password management.
 * auth_validate() is preserved for backward compatibility with existing tests.
 *
 * @req SWR-SEC-001
 * @req SWR-SEC-002
 * @req SWR-SEC-003
 */
#ifndef GUI_AUTH_H
#define GUI_AUTH_H

#include "gui_users.h"   /* UserRole typedef lives here */

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_MAX_CREDENTIAL_LEN USERS_MAX_USERNAME_LEN

/**
 * Initialise the auth subsystem (calls users_init).
 * Must be called once at application startup before any auth_* call.
 * @req SWR-SEC-001
 */
void auth_init(void);

/**
 * Validate username/password. Returns 1 on success, 0 on failure.
 * Backward-compatible shim — delegates to auth_validate_role(u, p, NULL).
 * Existing tests that call auth_validate() continue to work unchanged.
 * @req SWR-GUI-001
 */
int auth_validate(const char *username, const char *password);

/**
 * Role-aware authentication. Writes matched role to *role_out on success
 * if role_out is non-NULL. Returns 1/0.
 * @req SWR-SEC-001
 * @req SWR-SEC-002
 */
int auth_validate_role(const char *username, const char *password,
                       UserRole *role_out);

/**
 * Copy display name for a validated username into out[out_len].
 * @req SWR-GUI-002
 */
void auth_display_name(const char *username, char *out, int out_len);

/**
 * Change own password (requires correct old_password). Returns 1/0.
 * @req SWR-SEC-003
 */
int auth_change_password(const char *username,
                          const char *old_password,
                          const char *new_password);

/**
 * Admin: set any user's password without old password. Returns 1/0.
 * @req SWR-SEC-003
 */
int auth_admin_set_password(const char *username, const char *new_password);

#ifdef __cplusplus
}
#endif

#endif /* GUI_AUTH_H */
