/**
 * @file gui_auth.h
 * @brief GUI authentication interface.
 *
 * Provides credential validation for the Patient Vital Signs Monitor login
 * screen. The implementation uses a fixed built-in credential set appropriate
 * for a standalone clinical workstation application.
 *
 * @req SWR-GUI-001
 */

#ifndef GUI_AUTH_H
#define GUI_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length (including NUL) for a username or password string. */
#define AUTH_MAX_CREDENTIAL_LEN 64

/**
 * @brief Validate a username/password pair against the built-in credential.
 *
 * Performs a constant-time-safe comparison against the compiled-in
 * administrator credential. Both pointers must be non-NULL and
 * NUL-terminated; NULL inputs return 0 immediately.
 *
 * @param username  NUL-terminated username string.
 * @param password  NUL-terminated password string.
 * @return 1 if credentials are valid, 0 otherwise.
 * @req SWR-GUI-001
 */
int auth_validate(const char *username, const char *password);

/**
 * @brief Return a human-readable display name for a validated username.
 *
 * Copies a display-friendly name into @p out. If @p username is
 * unrecognised or @p out is NULL, the function writes an empty string
 * (or does nothing, respectively).
 *
 * @param username  NUL-terminated validated username.
 * @param out       Destination buffer.
 * @param out_len   Size of @p out in bytes.
 * @req SWR-GUI-002
 */
void auth_display_name(const char *username, char *out, int out_len);

#ifdef __cplusplus
}
#endif

#endif /* GUI_AUTH_H */
