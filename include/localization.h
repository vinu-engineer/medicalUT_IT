/**
 * @file localization.h
 * @brief Multi-language localization system interface.
 *
 * Supports 4 languages: English, Spanish, French, and German.
 * All strings are statically allocated (no heap allocation) for IEC 62304 compliance.
 *
 * @req SWR-GUI-012  Multi-language support for 4 languages
 */

#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Language enumeration.
 */
typedef enum {
    LANG_ENGLISH = 0,
    LANG_SPANISH = 1,
    LANG_FRENCH = 2,
    LANG_GERMAN = 3,
    LANG_COUNT = 4  /* Total number of supported languages */
} Language;

/**
 * @brief String ID enumeration for all user-facing strings.
 * All strings are statically defined; no dynamic allocation.
 */
typedef enum {
    /* Application and system */
    STR_APP_TITLE = 0,
    STR_IN_SIMULATION_MODE = 1,
    STR_ERROR = 2,
    STR_WARNING = 3,
    STR_OK = 4,
    STR_CANCEL = 5,

    /* Authentication */
    STR_SIGN_IN = 6,
    STR_SIGN_OUT = 7,
    STR_USERNAME = 8,
    STR_PASSWORD = 9,
    STR_LOGIN = 10,
    STR_LOGOUT = 11,

    /* Dashboard */
    STR_DASHBOARD = 12,
    STR_PATIENT = 13,
    STR_VITAL_SIGNS = 14,
    STR_STATUS = 15,
    STR_ALERTS = 16,

    /* Vital Signs */
    STR_HEART_RATE = 17,
    STR_BLOOD_PRESSURE = 18,
    STR_TEMPERATURE = 19,
    STR_RESPIRATION_RATE = 20,
    STR_OXYGEN_SATURATION = 21,
    STR_SYSTOLIC = 22,
    STR_DIASTOLIC = 23,
    STR_BPM = 24,
    STR_CELSIUS = 25,
    STR_FAHRENHEIT = 26,
    STR_PERCENT = 27,

    /* Status levels */
    STR_STATUS_NORMAL = 28,
    STR_STATUS_WARNING = 29,
    STR_STATUS_CRITICAL = 30,

    /* Menu and Settings */
    STR_MENU = 31,
    STR_FILE = 32,
    STR_EDIT = 33,
    STR_HELP = 34,
    STR_SETTINGS = 35,
    STR_PREFERENCES = 36,
    STR_LANGUAGE = 37,
    STR_THEME = 38,

    /* Buttons */
    STR_BUTTON_OK = 39,
    STR_BUTTON_CANCEL = 40,
    STR_BUTTON_APPLY = 41,
    STR_BUTTON_SAVE = 42,
    STR_BUTTON_CLOSE = 43,

    /* Messages */
    STR_LOADING = 44,
    STR_SAVING = 45,
    STR_SAVED = 46,
    STR_CONNECTING = 47,
    STR_DISCONNECTED = 48,
    STR_SIMULATION_MODE_ON = 49,

    STR_COUNT = 50  /* Total number of strings */
} StringID;

/**
 * @brief Set the current language.
 *
 * All subsequent calls to localization_get_string() will return strings
 * in the specified language.
 *
 * @param lang  Language enumeration value (0-3).
 *
 * @req SWR-GUI-012
 */
void localization_set_language(Language lang);

/**
 * @brief Get the current language.
 *
 * @return The currently active Language enumeration value.
 *
 * @req SWR-GUI-012
 */
Language localization_get_language(void);

/**
 * @brief Retrieve a localized string for the current language.
 *
 * Returns a statically-allocated string in the current language.
 * If the string ID is invalid, returns "???" (fallback).
 * Never returns NULL.
 *
 * @param id  String ID enumeration value.
 * @return    Pointer to static null-terminated string (never NULL).
 *
 * @req SWR-GUI-012
 */
const char* localization_get_string(StringID id);

/**
 * @brief Get the localized name of a language.
 *
 * For example, localization_get_language_name(LANG_SPANISH) returns "Español".
 * If the language ID is invalid, returns "Unknown" (fallback).
 * Never returns NULL.
 *
 * @param lang  Language enumeration value.
 * @return      Pointer to static null-terminated string (never NULL).
 *
 * @req SWR-GUI-012
 */
const char* localization_get_language_name(Language lang);

#ifdef __cplusplus
}
#endif

#endif /* LOCALIZATION_H */
