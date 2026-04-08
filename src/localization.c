/**
 * @file localization.c
 * @brief Implementation of multi-language localization system.
 *
 * Provides static string tables for English, Spanish, French, and German.
 * All strings are statically allocated (no heap allocation) for IEC 62304 compliance.
 *
 * @req SWR-GUI-012  Multi-language support
 */

#include "localization.h"
#include <string.h>

/* ===================================================================
 * Static String Tables (All 4 Languages)
 * =================================================================== */

static const char* const g_strings_english[STR_COUNT] = {
    "Patient Vital Signs Monitor",           /* STR_APP_TITLE */
    "IN SIMULATION MODE",                    /* STR_IN_SIMULATION_MODE */
    "Error",                                 /* STR_ERROR */
    "Warning",                               /* STR_WARNING */
    "OK",                                    /* STR_OK */
    "Cancel",                                /* STR_CANCEL */
    "Sign In",                               /* STR_SIGN_IN */
    "Sign Out",                              /* STR_SIGN_OUT */
    "Username",                              /* STR_USERNAME */
    "Password",                              /* STR_PASSWORD */
    "Login",                                 /* STR_LOGIN */
    "Logout",                                /* STR_LOGOUT */
    "Dashboard",                             /* STR_DASHBOARD */
    "Patient",                               /* STR_PATIENT */
    "Vital Signs",                           /* STR_VITAL_SIGNS */
    "Status",                                /* STR_STATUS */
    "Alerts",                                /* STR_ALERTS */
    "Heart Rate",                            /* STR_HEART_RATE */
    "Blood Pressure",                        /* STR_BLOOD_PRESSURE */
    "Temperature",                           /* STR_TEMPERATURE */
    "Respiration Rate",                      /* STR_RESPIRATION_RATE */
    "Oxygen Saturation",                     /* STR_OXYGEN_SATURATION */
    "Systolic",                              /* STR_SYSTOLIC */
    "Diastolic",                             /* STR_DIASTOLIC */
    "BPM",                                   /* STR_BPM */
    "°C",                                    /* STR_CELSIUS */
    "°F",                                    /* STR_FAHRENHEIT */
    "%",                                     /* STR_PERCENT */
    "Normal",                                /* STR_STATUS_NORMAL */
    "Warning",                               /* STR_STATUS_WARNING */
    "!! CRITICAL — Immediate clinical action required !!",  /* STR_STATUS_CRITICAL */
    "Menu",                                  /* STR_MENU */
    "File",                                  /* STR_FILE */
    "Edit",                                  /* STR_EDIT */
    "Help",                                  /* STR_HELP */
    "Settings",                              /* STR_SETTINGS */
    "Preferences",                           /* STR_PREFERENCES */
    "Language",                              /* STR_LANGUAGE */
    "Theme",                                 /* STR_THEME */
    "OK",                                    /* STR_BUTTON_OK */
    "Cancel",                                /* STR_BUTTON_CANCEL */
    "Apply",                                 /* STR_BUTTON_APPLY */
    "Save",                                  /* STR_BUTTON_SAVE */
    "Close",                                 /* STR_BUTTON_CLOSE */
    "Loading...",                            /* STR_LOADING */
    "Saving...",                             /* STR_SAVING */
    "Saved",                                 /* STR_SAVED */
    "Connecting...",                         /* STR_CONNECTING */
    "Disconnected",                          /* STR_DISCONNECTED */
    "Simulation Mode ON",                    /* STR_SIMULATION_MODE_ON */
};

static const char* const g_strings_spanish[STR_COUNT] = {
    "Monitor de Signos Vitales del Paciente", /* STR_APP_TITLE */
    "EN MODO SIMULACIÓN",                     /* STR_IN_SIMULATION_MODE */
    "Error",                                  /* STR_ERROR */
    "Advertencia",                            /* STR_WARNING */
    "Aceptar",                                /* STR_OK */
    "Cancelar",                               /* STR_CANCEL */
    "Iniciar Sesión",                         /* STR_SIGN_IN */
    "Cerrar Sesión",                          /* STR_SIGN_OUT */
    "Nombre de Usuario",                      /* STR_USERNAME */
    "Contraseña",                             /* STR_PASSWORD */
    "Iniciar",                                /* STR_LOGIN */
    "Salir",                                  /* STR_LOGOUT */
    "Panel de Control",                       /* STR_DASHBOARD */
    "Paciente",                               /* STR_PATIENT */
    "Signos Vitales",                         /* STR_VITAL_SIGNS */
    "Estado",                                 /* STR_STATUS */
    "Alertas",                                /* STR_ALERTS */
    "Frecuencia Cardíaca",                    /* STR_HEART_RATE */
    "Presión Arterial",                       /* STR_BLOOD_PRESSURE */
    "Temperatura",                            /* STR_TEMPERATURE */
    "Frecuencia Respiratoria",                /* STR_RESPIRATION_RATE */
    "Saturación de Oxígeno",                  /* STR_OXYGEN_SATURATION */
    "Sistólica",                              /* STR_SYSTOLIC */
    "Diastólica",                             /* STR_DIASTOLIC */
    "LPM",                                    /* STR_BPM */
    "°C",                                     /* STR_CELSIUS */
    "°F",                                     /* STR_FAHRENHEIT */
    "%",                                      /* STR_PERCENT */
    "Normal",                                 /* STR_STATUS_NORMAL */
    "Advertencia",                            /* STR_STATUS_WARNING */
    "!! CRÍTICO — Se requiere acción clínica inmediata !!",  /* STR_STATUS_CRITICAL */
    "Menú",                                   /* STR_MENU */
    "Archivo",                                /* STR_FILE */
    "Editar",                                 /* STR_EDIT */
    "Ayuda",                                  /* STR_HELP */
    "Configuración",                          /* STR_SETTINGS */
    "Preferencias",                           /* STR_PREFERENCES */
    "Idioma",                                 /* STR_LANGUAGE */
    "Tema",                                   /* STR_THEME */
    "Aceptar",                                /* STR_BUTTON_OK */
    "Cancelar",                               /* STR_BUTTON_CANCEL */
    "Aplicar",                                /* STR_BUTTON_APPLY */
    "Guardar",                                /* STR_BUTTON_SAVE */
    "Cerrar",                                 /* STR_BUTTON_CLOSE */
    "Cargando...",                            /* STR_LOADING */
    "Guardando...",                           /* STR_SAVING */
    "Guardado",                               /* STR_SAVED */
    "Conectando...",                          /* STR_CONNECTING */
    "Desconectado",                           /* STR_DISCONNECTED */
    "Modo Simulación ACTIVADO",               /* STR_SIMULATION_MODE_ON */
};

static const char* const g_strings_french[STR_COUNT] = {
    "Moniteur de Signes Vitaux du Patient",  /* STR_APP_TITLE */
    "EN MODE SIMULATION",                     /* STR_IN_SIMULATION_MODE */
    "Erreur",                                 /* STR_ERROR */
    "Avertissement",                          /* STR_WARNING */
    "OK",                                     /* STR_OK */
    "Annuler",                                /* STR_CANCEL */
    "Se Connecter",                           /* STR_SIGN_IN */
    "Se Déconnecter",                         /* STR_SIGN_OUT */
    "Nom d'Utilisateur",                      /* STR_USERNAME */
    "Mot de passe",                           /* STR_PASSWORD */
    "Connexion",                              /* STR_LOGIN */
    "Déconnexion",                            /* STR_LOGOUT */
    "Tableau de Bord",                        /* STR_DASHBOARD */
    "Patient",                                /* STR_PATIENT */
    "Signes Vitaux",                          /* STR_VITAL_SIGNS */
    "État",                                   /* STR_STATUS */
    "Alertes",                                /* STR_ALERTS */
    "Fréquence Cardiaque",                    /* STR_HEART_RATE */
    "Pression Artérielle",                    /* STR_BLOOD_PRESSURE */
    "Température",                            /* STR_TEMPERATURE */
    "Fréquence Respiratoire",                 /* STR_RESPIRATION_RATE */
    "Saturation en Oxygène",                  /* STR_OXYGEN_SATURATION */
    "Systolique",                             /* STR_SYSTOLIC */
    "Diastolique",                            /* STR_DIASTOLIC */
    "BPM",                                    /* STR_BPM */
    "°C",                                     /* STR_CELSIUS */
    "°F",                                     /* STR_FAHRENHEIT */
    "%",                                      /* STR_PERCENT */
    "Normal",                                 /* STR_STATUS_NORMAL */
    "Avertissement",                          /* STR_STATUS_WARNING */
    "!! CRITIQUE — Action clinique immédiate requise !!",  /* STR_STATUS_CRITICAL */
    "Menu",                                   /* STR_MENU */
    "Fichier",                                /* STR_FILE */
    "Édition",                                /* STR_EDIT */
    "Aide",                                   /* STR_HELP */
    "Paramètres",                             /* STR_SETTINGS */
    "Préférences",                            /* STR_PREFERENCES */
    "Langue",                                 /* STR_LANGUAGE */
    "Thème",                                  /* STR_THEME */
    "OK",                                     /* STR_BUTTON_OK */
    "Annuler",                                /* STR_BUTTON_CANCEL */
    "Appliquer",                              /* STR_BUTTON_APPLY */
    "Enregistrer",                            /* STR_BUTTON_SAVE */
    "Fermer",                                 /* STR_BUTTON_CLOSE */
    "Chargement...",                          /* STR_LOADING */
    "Enregistrement...",                      /* STR_SAVING */
    "Enregistré",                             /* STR_SAVED */
    "Connexion...",                           /* STR_CONNECTING */
    "Déconnecté",                             /* STR_DISCONNECTED */
    "Mode Simulation ACTIVÉ",                 /* STR_SIMULATION_MODE_ON */
};

static const char* const g_strings_german[STR_COUNT] = {
    "Patient Vital Signs Monitor",            /* STR_APP_TITLE */
    "IM SIMULATIONSMODUS",                    /* STR_IN_SIMULATION_MODE */
    "Fehler",                                 /* STR_ERROR */
    "Warnung",                                /* STR_WARNING */
    "OK",                                     /* STR_OK */
    "Abbrechen",                              /* STR_CANCEL */
    "Anmelden",                               /* STR_SIGN_IN */
    "Abmelden",                               /* STR_SIGN_OUT */
    "Benutzername",                           /* STR_USERNAME */
    "Passwort",                               /* STR_PASSWORD */
    "Anmeldung",                              /* STR_LOGIN */
    "Abmeldung",                              /* STR_LOGOUT */
    "Dashboard",                              /* STR_DASHBOARD */
    "Patient",                                /* STR_PATIENT */
    "Vitalzeichen",                           /* STR_VITAL_SIGNS */
    "Status",                                 /* STR_STATUS */
    "Benachrichtigungen",                     /* STR_ALERTS */
    "Herzfrequenz",                           /* STR_HEART_RATE */
    "Blutdruck",                              /* STR_BLOOD_PRESSURE */
    "Temperatur",                             /* STR_TEMPERATURE */
    "Atemfrequenz",                           /* STR_RESPIRATION_RATE */
    "Sauerstoffsättigung",                    /* STR_OXYGEN_SATURATION */
    "Systolisch",                             /* STR_SYSTOLIC */
    "Diastolisch",                            /* STR_DIASTOLIC */
    "Schläge/Min",                            /* STR_BPM */
    "°C",                                     /* STR_CELSIUS */
    "°F",                                     /* STR_FAHRENHEIT */
    "%",                                      /* STR_PERCENT */
    "Normal",                                 /* STR_STATUS_NORMAL */
    "Warnung",                                /* STR_STATUS_WARNING */
    "!! KRITISCH — Sofortige klinische Maßnahmen erforderlich !!",  /* STR_STATUS_CRITICAL */
    "Menü",                                   /* STR_MENU */
    "Datei",                                  /* STR_FILE */
    "Bearbeitung",                            /* STR_EDIT */
    "Hilfe",                                  /* STR_HELP */
    "Einstellungen",                          /* STR_SETTINGS */
    "Voreinstellungen",                       /* STR_PREFERENCES */
    "Sprache",                                /* STR_LANGUAGE */
    "Design",                                 /* STR_THEME */
    "OK",                                     /* STR_BUTTON_OK */
    "Abbrechen",                              /* STR_BUTTON_CANCEL */
    "Anwenden",                               /* STR_BUTTON_APPLY */
    "Speichern",                              /* STR_BUTTON_SAVE */
    "Schließen",                              /* STR_BUTTON_CLOSE */
    "Wird geladen...",                        /* STR_LOADING */
    "Wird gespeichert...",                    /* STR_SAVING */
    "Gespeichert",                            /* STR_SAVED */
    "Wird verbunden...",                      /* STR_CONNECTING */
    "Getrennt",                               /* STR_DISCONNECTED */
    "Simulationsmodus AKTIVIERT",             /* STR_SIMULATION_MODE_ON */
};

/* Language names in each language */
static const char* const g_language_names[LANG_COUNT] = {
    "English",     /* LANG_ENGLISH */
    "Español",     /* LANG_SPANISH */
    "Français",    /* LANG_FRENCH */
    "Deutsch",     /* LANG_GERMAN */
};

/* Current language state */
static Language g_current_language = LANG_ENGLISH;

/* ===================================================================
 * Public API Implementation
 * =================================================================== */

void localization_set_language(Language lang)
{
    if (lang >= 0 && lang < LANG_COUNT)
        g_current_language = lang;
}

Language localization_get_language(void)
{
    return g_current_language;
}

const char* localization_get_string(StringID id)
{
    if (id < 0 || id >= STR_COUNT)
        return "???";  /* Fallback for invalid string ID */

    switch (g_current_language)
    {
    case LANG_ENGLISH:
        return g_strings_english[id];
    case LANG_SPANISH:
        return g_strings_spanish[id];
    case LANG_FRENCH:
        return g_strings_french[id];
    case LANG_GERMAN:
        return g_strings_german[id];
    default:
        return "???";
    }
}

const char* localization_get_language_name(Language lang)
{
    if (lang < 0 || lang >= LANG_COUNT)
        return "Unknown";
    return g_language_names[lang];
}
