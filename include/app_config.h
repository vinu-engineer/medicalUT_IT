/**
 * @file app_config.h
 * @brief Application configuration persistence interface.
 *
 * Provides save/load of run-time configuration (sim_enabled flag and
 * language preference) to/from a plain-text file named APP_CFG_FILENAME.
 * The file is located in the same directory as the running executable
 * unless overridden via app_config_set_path() (intended for unit tests).
 *
 * @req SWR-GUI-010  Configuration shall persist across application restarts.
 * @req SWR-GUI-012  Language preference shall persist across sessions.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Default configuration file name (placed next to the executable). */
#define APP_CFG_FILENAME "monitor.cfg"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Override the configuration file path.
 *
 * When @p path is non-NULL the module uses that exact path instead of
 * deriving one from the executable location.  Pass NULL to revert to
 * the default (exe-directory) behaviour.
 *
 * This function exists primarily to support unit testing so that tests
 * can redirect reads and writes to a temporary location without touching
 * the production configuration file.
 *
 * @param path  Absolute or relative path to use, or NULL to reset.
 *
 * @req SWR-GUI-010
 */
void app_config_set_path(const char *path);

/**
 * @brief Load the application configuration from disk.
 *
 * Reads the configuration file and populates @p sim_enabled_out and
 * @p language_out. If the file is absent or cannot be parsed default
 * values are written: sim_enabled = 1, language = LANG_ENGLISH.
 *
 * @param[out] sim_enabled_out  Receives 1 (simulator on) or 0 (HAL mode).
 *                              Must not be NULL.
 * @param[out] language_out     Receives language enumeration value.
 *                              Must not be NULL.
 * @return 1 if the file was read and parsed successfully,
 *         0 if the file was missing or could not be parsed.
 *
 * @req SWR-GUI-010, SWR-GUI-012
 */
int app_config_load(int *sim_enabled_out, int *language_out);

/**
 * @brief Save the application configuration to disk.
 *
 * Writes the current configuration to the configuration file,
 * creating or overwriting it as needed.
 *
 * @param sim_enabled  1 to enable the simulator, 0 for real HAL mode.
 * @param language     Language enumeration value (0-3).
 * @return 1 on success, 0 on failure (e.g. file could not be opened).
 *
 * @req SWR-GUI-010, SWR-GUI-012
 */
int app_config_save(int sim_enabled, int language);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
