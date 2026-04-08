/**
 * @file alarm_limits.c
 * @brief Implementation of the configurable alarm limits module.
 *
 * @details
 * Implements all functions declared in alarm_limits.h. Configuration is
 * persisted as a plain-text key=value file (one entry per line, '#' for
 * comments) so that limits can be updated in the field without recompiling.
 *
 * No heap allocation is used. The file path is held in a module-level
 * pointer (static storage, default initialised to a string literal).
 * All file I/O uses the C standard library only (fopen / fclose / fgets /
 * fprintf / sscanf).
 *
 * ### Alarm zone logic
 * Each check function uses a simple three-zone model:
 *   CRITICAL — value breaches the configured limit AND is beyond the
 *              "critical margin" from that limit.
 *   WARNING  — value breaches the configured limit but is still within
 *              the critical margin.
 *   NORMAL   — value is within the configured limits.
 *
 * Critical margins:
 *   Heart rate  : ±10 bpm
 *   Blood press.: ±10 mmHg
 *   Temperature : ±1.0 °C
 *   SpO2        : 5 % below the lower limit
 *   Resp. rate  : ±10 br/min
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 */

#include "alarm_limits.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * Module-level configuration file path
 * ========================================================================= */

/** Default path for the alarm limits configuration file. */
static const char *s_cfg_path = "alarm_limits.cfg";

/* =========================================================================
 * Limit Management Implementations
 * ========================================================================= */

/**
 * @brief Fill @p lim with factory default alarm limits.
 */
void alarm_limits_defaults(AlarmLimits *lim)
{
    lim->hr_low    = 40;
    lim->hr_high   = 150;
    lim->sbp_low   = 70;
    lim->sbp_high  = 180;
    lim->dbp_low   = 40;
    lim->dbp_high  = 120;
    lim->temp_low  = 35.0f;
    lim->temp_high = 39.5f;
    lim->spo2_low  = 90;
    lim->rr_low    = 8;
    lim->rr_high   = 25;
}

/**
 * @brief Write all alarm limits to the configuration file as key=value pairs.
 * @return 1 on success, 0 on any file I/O error.
 */
int alarm_limits_save(const AlarmLimits *lim)
{
    FILE *fp = fopen(s_cfg_path, "w");
    if (fp == NULL) {
        return 0;
    }

    fprintf(fp, "# Alarm limits configuration — auto-generated, do not edit manually.\n");
    fprintf(fp, "hr_low=%d\n",    lim->hr_low);
    fprintf(fp, "hr_high=%d\n",   lim->hr_high);
    fprintf(fp, "sbp_low=%d\n",   lim->sbp_low);
    fprintf(fp, "sbp_high=%d\n",  lim->sbp_high);
    fprintf(fp, "dbp_low=%d\n",   lim->dbp_low);
    fprintf(fp, "dbp_high=%d\n",  lim->dbp_high);
    fprintf(fp, "temp_low=%.2f\n",  (double)lim->temp_low);
    fprintf(fp, "temp_high=%.2f\n", (double)lim->temp_high);
    fprintf(fp, "spo2_low=%d\n",  lim->spo2_low);
    fprintf(fp, "rr_low=%d\n",    lim->rr_low);
    fprintf(fp, "rr_high=%d\n",   lim->rr_high);

    fclose(fp);
    return 1;
}

/**
 * @brief Load alarm limits from the configuration file.
 *
 * @details Applies defaults first, then overwrites recognised keys from the
 * file. Unrecognised keys and comment lines are silently ignored.
 * @return 1 if the file was opened successfully, 0 if not found (defaults apply).
 */
int alarm_limits_load(AlarmLimits *lim)
{
    char  line[128];
    char  key[64];
    float val_f;
    int   val_i;
    FILE *fp;

    /* Always start from a known-good state. */
    alarm_limits_defaults(lim);

    fp = fopen(s_cfg_path, "r");
    if (fp == NULL) {
        /* File not present — defaults remain in effect. */
        return 0;
    }

    while (fgets(line, (int)sizeof(line), fp) != NULL) {
        /* Skip comment lines and blank lines. */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        /* Parse floating-point keys first to avoid integer truncation. */
        if (sscanf(line, "%63[^=]=%f", key, &val_f) == 2) {
            if (strcmp(key, "temp_low")  == 0) { lim->temp_low  = val_f; continue; }
            if (strcmp(key, "temp_high") == 0) { lim->temp_high = val_f; continue; }
        }

        /* Parse integer keys. */
        if (sscanf(line, "%63[^=]=%d", key, &val_i) == 2) {
            if (strcmp(key, "hr_low")   == 0) { lim->hr_low   = val_i; continue; }
            if (strcmp(key, "hr_high")  == 0) { lim->hr_high  = val_i; continue; }
            if (strcmp(key, "sbp_low")  == 0) { lim->sbp_low  = val_i; continue; }
            if (strcmp(key, "sbp_high") == 0) { lim->sbp_high = val_i; continue; }
            if (strcmp(key, "dbp_low")  == 0) { lim->dbp_low  = val_i; continue; }
            if (strcmp(key, "dbp_high") == 0) { lim->dbp_high = val_i; continue; }
            if (strcmp(key, "spo2_low") == 0) { lim->spo2_low = val_i; continue; }
            if (strcmp(key, "rr_low")   == 0) { lim->rr_low   = val_i; continue; }
            if (strcmp(key, "rr_high")  == 0) { lim->rr_high  = val_i; continue; }
        }
        /* Unknown key — ignore silently (forward compatibility). */
    }

    fclose(fp);
    return 1;
}

/**
 * @brief Override the module-internal configuration file path.
 * @param[in] path New path, or NULL to restore the default.
 */
void alarm_limits_set_path(const char *path)
{
    if (path == NULL) {
        s_cfg_path = "alarm_limits.cfg";
    } else {
        s_cfg_path = path;
    }
}

/* =========================================================================
 * Per-Parameter Alarm Check Implementations
 * ========================================================================= */

/**
 * @brief Evaluate a heart rate against the configured limits.
 *
 * @details Three-zone logic with a critical margin of 10 bpm:
 *   val < hr_low-10 or val > hr_high+10 → CRITICAL
 *   val < hr_low    or val > hr_high    → WARNING
 *   otherwise                           → NORMAL
 */
AlertLevel alarm_check_hr(const AlarmLimits *lim, int bpm)
{
    if (bpm < lim->hr_low - 10 || bpm > lim->hr_high + 10) return ALERT_CRITICAL;
    if (bpm < lim->hr_low      || bpm > lim->hr_high)      return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Evaluate blood pressure against the configured limits.
 *
 * @details Each pressure is checked independently with a 10 mmHg critical
 * margin. The more severe result of systolic and diastolic is returned.
 */
AlertLevel alarm_check_bp(const AlarmLimits *lim, int sbp, int dbp)
{
    AlertLevel level_s;
    AlertLevel level_d;

    /* Systolic check */
    if (sbp < lim->sbp_low - 10 || sbp > lim->sbp_high + 10) {
        level_s = ALERT_CRITICAL;
    } else if (sbp < lim->sbp_low || sbp > lim->sbp_high) {
        level_s = ALERT_WARNING;
    } else {
        level_s = ALERT_NORMAL;
    }

    /* Diastolic check */
    if (dbp < lim->dbp_low - 10 || dbp > lim->dbp_high + 10) {
        level_d = ALERT_CRITICAL;
    } else if (dbp < lim->dbp_low || dbp > lim->dbp_high) {
        level_d = ALERT_WARNING;
    } else {
        level_d = ALERT_NORMAL;
    }

    return (level_s > level_d) ? level_s : level_d;
}

/**
 * @brief Evaluate a temperature reading against the configured limits.
 *
 * @details Three-zone logic with a critical margin of 1.0 °C:
 *   val < temp_low-1.0 or val > temp_high+1.0 → CRITICAL
 *   val < temp_low     or val > temp_high      → WARNING
 *   otherwise                                  → NORMAL
 */
AlertLevel alarm_check_temp(const AlarmLimits *lim, float temp_c)
{
    if (temp_c < lim->temp_low  - 1.0f || temp_c > lim->temp_high + 1.0f) return ALERT_CRITICAL;
    if (temp_c < lim->temp_low          || temp_c > lim->temp_high)        return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Evaluate an SpO2 reading against the configured lower limit.
 *
 * @details SpO2 has only a lower limit. Critical margin is 5 %:
 *   spo2 < spo2_low-5 → CRITICAL
 *   spo2 < spo2_low   → WARNING
 *   otherwise         → NORMAL
 */
AlertLevel alarm_check_spo2(const AlarmLimits *lim, int spo2)
{
    if (spo2 < lim->spo2_low - 5) return ALERT_CRITICAL;
    if (spo2 < lim->spo2_low)     return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Evaluate a respiration rate against the configured limits.
 *
 * @details Three-zone logic with a critical margin of 10 br/min:
 *   val < rr_low-10 or val > rr_high+10 → CRITICAL
 *   val < rr_low    or val > rr_high    → WARNING
 *   otherwise                           → NORMAL
 */
AlertLevel alarm_check_rr(const AlarmLimits *lim, int rr)
{
    if (rr < lim->rr_low - 10 || rr > lim->rr_high + 10) return ALERT_CRITICAL;
    if (rr < lim->rr_low      || rr > lim->rr_high)      return ALERT_WARNING;
    return ALERT_NORMAL;
}
