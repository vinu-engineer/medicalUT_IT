/**
 * @file alarm_limits.h
 * @brief Configurable alarm limits module (IEC 60601-1-8).
 *
 * @details
 * This module provides a runtime-configurable alarm limit system that
 * complements the fixed clinical thresholds in vitals.h. Limits are
 * stored in a plain-text key=value configuration file so that a clinical
 * engineer can adjust them without recompiling the firmware, in accordance
 * with IEC 60601-1-8 alarm customisation requirements.
 *
 * The module supplies:
 * - An AlarmLimits structure holding the configured limit for each
 *   monitored vital parameter.
 * - Functions to load, save, and reset limits to factory defaults.
 * - A set of alarm_check_*() functions that evaluate a vital sign against
 *   the configured limits rather than the hardcoded clinical thresholds.
 *
 * ### Default Limits
 * | Parameter   | Low default | High default |
 * |-------------|-------------|--------------|
 * | Heart rate  | 40 bpm      | 150 bpm      |
 * | Systolic BP | 70 mmHg     | 180 mmHg     |
 * | Diastolic BP| 40 mmHg     | 120 mmHg     |
 * | Temperature | 35.0 °C     | 39.5 °C      |
 * | SpO2        | 90 %        | —            |
 * | Resp. rate  | 8 br/min    | 25 br/min    |
 *
 * ### IEC 62304 Traceability
 * - Software Unit: UNIT-ALM
 * - Requirements covered: SWR-ALM-001
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 *
 * @copyright Medical Device Software — IEC 62304 Class B compliant.
 *            All rights reserved.
 *
 * @req SWR-ALM-001
 */

#ifndef ALARM_LIMITS_H
#define ALARM_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vitals.h"

/* =========================================================================
 * AlarmLimits Structure
 * ========================================================================= */

/**
 * @brief Configurable alarm limits for all monitored vital parameters.
 *
 * @details Values outside the [low, high] interval for each parameter
 * trigger a WARNING or CRITICAL alert as evaluated by the alarm_check_*()
 * functions.  A value that breaches a limit by more than 10 units (for
 * integer parameters) or 1.0 °C (for temperature) is classified as
 * CRITICAL; a value that breaches the limit by a smaller margin is
 * classified as WARNING.
 */
typedef struct {
    int   hr_low;    /**< Heart rate lower alarm limit. Default: 40 bpm.        */
    int   hr_high;   /**< Heart rate upper alarm limit. Default: 150 bpm.       */
    int   sbp_low;   /**< Systolic BP lower alarm limit. Default: 70 mmHg.      */
    int   sbp_high;  /**< Systolic BP upper alarm limit. Default: 180 mmHg.     */
    int   dbp_low;   /**< Diastolic BP lower alarm limit. Default: 40 mmHg.     */
    int   dbp_high;  /**< Diastolic BP upper alarm limit. Default: 120 mmHg.    */
    float temp_low;  /**< Temperature lower alarm limit. Default: 35.0 °C.      */
    float temp_high; /**< Temperature upper alarm limit. Default: 39.5 °C.      */
    int   spo2_low;  /**< SpO2 lower alarm limit. Default: 90 %.                */
    int   rr_low;    /**< Respiration rate lower alarm limit. Default: 8 br/min.*/
    int   rr_high;   /**< Respiration rate upper alarm limit. Default: 25 br/min*/
} AlarmLimits;

/* =========================================================================
 * Limit Management Functions
 * ========================================================================= */

/**
 * @brief Populate an AlarmLimits structure with factory default values.
 *
 * @details Writes the default limits listed in the file-level documentation
 * into @p lim. Safe to call at any time; no file I/O is performed.
 *
 * @param[out] lim Pointer to the AlarmLimits to initialise. Must not be NULL.
 *
 * @pre  lim != NULL
 * @par Requirement
 * SWR-ALM-001
 */
void alarm_limits_defaults(AlarmLimits *lim);

/**
 * @brief Persist the current alarm limits to the configuration file.
 *
 * @details Writes all limit values to a plain-text key=value file. The
 * file path defaults to "alarm_limits.cfg" in the current working
 * directory and can be overridden for testing via alarm_limits_set_path().
 *
 * @param[in] lim Pointer to the AlarmLimits to save. Must not be NULL.
 * @return 1 if the file was written successfully, 0 on any I/O error.
 *
 * @pre  lim != NULL
 * @par Requirement
 * SWR-ALM-001
 */
int alarm_limits_save(const AlarmLimits *lim);

/**
 * @brief Load alarm limits from the configuration file.
 *
 * @details Calls alarm_limits_defaults() first to guarantee all fields
 * have valid values, then reads the configuration file and overwrites any
 * key whose name is recognised. Unknown keys are silently ignored.
 *
 * @param[out] lim Pointer to the AlarmLimits to populate. Must not be NULL.
 * @return 1 if the file was found and read (even partially), 0 if the
 *         file could not be opened (defaults are still applied).
 *
 * @pre  lim != NULL
 * @par Requirement
 * SWR-ALM-001
 */
int alarm_limits_load(AlarmLimits *lim);

/**
 * @brief Override the configuration file path (for unit testing).
 *
 * @details Replaces the module-internal file path string.  The pointer
 * must remain valid for the lifetime of subsequent save/load calls.
 * Passing NULL resets to the default path "alarm_limits.cfg".
 *
 * @param[in] path Null-terminated file path, or NULL for the default.
 *
 * @par Requirement
 * SWR-ALM-001
 */
void alarm_limits_set_path(const char *path);

/* =========================================================================
 * Per-Parameter Alarm Check Functions
 * ========================================================================= */

/**
 * @brief Evaluate a heart rate reading against configurable limits.
 *
 * @details
 * - NORMAL:   hr_low <= bpm <= hr_high
 * - WARNING:  bpm < hr_low OR bpm > hr_high (within 10 units of the limit)
 * - CRITICAL: bpm < hr_low-10 OR bpm > hr_high+10
 *
 * @param[in] lim Pointer to the current AlarmLimits. Must not be NULL.
 * @param[in] bpm Heart rate in beats per minute.
 * @return AlertLevel classification.
 *
 * @par Requirement
 * SWR-ALM-001
 */
AlertLevel alarm_check_hr(const AlarmLimits *lim, int bpm);

/**
 * @brief Evaluate blood pressure readings against configurable limits.
 *
 * @details Both systolic and diastolic values are checked independently
 * using the same three-zone logic; the more severe result is returned.
 * The critical margin is ±10 mmHg beyond the configured limit.
 *
 * @param[in] lim Pointer to the current AlarmLimits. Must not be NULL.
 * @param[in] sbp Systolic blood pressure in mmHg.
 * @param[in] dbp Diastolic blood pressure in mmHg.
 * @return AlertLevel classification (maximum of systolic and diastolic).
 *
 * @par Requirement
 * SWR-ALM-001
 */
AlertLevel alarm_check_bp(const AlarmLimits *lim, int sbp, int dbp);

/**
 * @brief Evaluate a temperature reading against configurable limits.
 *
 * @details The critical margin is ±1.0 °C beyond the configured limit.
 *
 * - NORMAL:   temp_low <= temp_c <= temp_high
 * - WARNING:  temp_c outside limits but within 1.0 °C
 * - CRITICAL: temp_c < temp_low-1.0 OR temp_c > temp_high+1.0
 *
 * @param[in] lim    Pointer to the current AlarmLimits. Must not be NULL.
 * @param[in] temp_c Body temperature in degrees Celsius.
 * @return AlertLevel classification.
 *
 * @par Requirement
 * SWR-ALM-001
 */
AlertLevel alarm_check_temp(const AlarmLimits *lim, float temp_c);

/**
 * @brief Evaluate an SpO2 reading against the configurable lower limit.
 *
 * @details SpO2 has only a lower limit. The critical margin is 5 % below
 * the configured limit.
 *
 * - NORMAL:   spo2 >= spo2_low
 * - WARNING:  spo2 < spo2_low but >= spo2_low-5
 * - CRITICAL: spo2 < spo2_low-5
 *
 * @param[in] lim  Pointer to the current AlarmLimits. Must not be NULL.
 * @param[in] spo2 SpO2 as a percentage.
 * @return AlertLevel classification.
 *
 * @par Requirement
 * SWR-ALM-001
 */
AlertLevel alarm_check_spo2(const AlarmLimits *lim, int spo2);

/**
 * @brief Evaluate a respiration rate reading against configurable limits.
 *
 * @details The critical margin is ±10 br/min beyond the configured limit.
 *
 * - NORMAL:   rr_low <= rr <= rr_high
 * - WARNING:  rr outside limits but within 10 br/min
 * - CRITICAL: rr < rr_low-10 OR rr > rr_high+10
 *
 * @param[in] lim Pointer to the current AlarmLimits. Must not be NULL.
 * @param[in] rr  Respiration rate in breaths per minute.
 * @return AlertLevel classification.
 *
 * @par Requirement
 * SWR-ALM-001
 */
AlertLevel alarm_check_rr(const AlarmLimits *lim, int rr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ALARM_LIMITS_H */
