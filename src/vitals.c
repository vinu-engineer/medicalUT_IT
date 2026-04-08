/**
 * @file vitals.c
 * @brief Implementation of vital sign validation and BMI calculation.
 *
 * @details
 * Implements all functions declared in vitals.h. Threshold values are
 * defined as inline literals within each function rather than as macros
 * to keep the code readable and auditable during IEC 62304 code review.
 *
 * ### Threshold Sources
 * - Heart rate: AHA/ACC 2019 guidelines
 * - Blood pressure: JNC-8 / ESC 2018 guidelines
 * - Temperature: WHO clinical reference ranges
 * - SpO2: British Thoracic Society supplemental oxygen guidelines
 * - Respiration rate: Royal College of Physicians NEWS2 / NICE guidelines
 * - BMI: WHO classification 2004
 *
 * @version 1.1.0
 * @date    2026-04-08
 * @author  vinu-engineer
 */

#include "vitals.h"

/**
 * @brief Classify a heart rate reading.
 * @details Implements SWR-VIT-001 threshold logic.
 *          Critical: bpm <= 40 (severe bradycardia) or bpm > 150 (severe tachycardia).
 *          Warning:  bpm < 60 or bpm > 100.
 *          Normal:   60 <= bpm <= 100.
 */
AlertLevel check_heart_rate(int bpm)
{
    if (bpm < 40  || bpm > 150) return ALERT_CRITICAL;
    if (bpm < 60  || bpm > 100) return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Classify a blood pressure reading.
 * @details Implements SWR-VIT-002 threshold logic.
 *          Critical thresholds are evaluated first (fail-safe ordering).
 *          If either systolic or diastolic is critical, CRITICAL is returned.
 *          If either is outside the normal range, WARNING is returned.
 */
AlertLevel check_blood_pressure(int systolic, int diastolic)
{
    if (systolic  <  70 || systolic  > 180) return ALERT_CRITICAL;
    if (diastolic <  40 || diastolic > 120) return ALERT_CRITICAL;
    if (systolic  <  90 || systolic  > 140) return ALERT_WARNING;
    if (diastolic <  60 || diastolic >  90) return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Classify a temperature reading.
 * @details Implements SWR-VIT-003 threshold logic.
 *          <35.0 °C = hypothermia (CRITICAL).
 *          >39.5 °C = hyperpyrexia (CRITICAL).
 *          Values outside 36.1–37.2 but within critical bounds = WARNING.
 */
AlertLevel check_temperature(float temp_c)
{
    if (temp_c < 35.0f || temp_c > 39.5f) return ALERT_CRITICAL;
    if (temp_c < 36.1f || temp_c > 37.2f) return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Classify an SpO2 reading.
 * @details Implements SWR-VIT-004 threshold logic.
 *          <90% = hypoxaemia (CRITICAL).
 *          90–94% = mild hypoxaemia (WARNING).
 *          95–100% = normal saturation.
 */
AlertLevel check_spo2(int spo2)
{
    if (spo2 < 90) return ALERT_CRITICAL;
    if (spo2 < 95) return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Classify a respiration rate reading.
 * @details Implements SWR-VIT-008 threshold logic.
 *          Critical: rr_bpm <= 8 (apnoea / severe bradypnoea)
 *                    or rr_bpm >= 25 (severe tachypnoea).
 *          Warning:  rr_bpm 9–11 or 21–24 breaths/min.
 *          Normal:   rr_bpm 12–20 breaths/min (inclusive).
 *
 *          NOTE: A caller passing rr_bpm == 0 receives ALERT_CRITICAL
 *          from this function (0 satisfies the <= 8 condition).
 *          overall_alert_level() guards against this by skipping the
 *          check when vitals->respiration_rate == 0.
 */
AlertLevel check_respiration_rate(int rr_bpm)
{
    if (rr_bpm <= 8  || rr_bpm >= 25) return ALERT_CRITICAL;
    if (rr_bpm <= 11 || rr_bpm >= 21) return ALERT_WARNING;
    return ALERT_NORMAL;
}

/**
 * @brief Return the highest alert level across all five vital parameters.
 * @details Implements SWR-VIT-005. Evaluates all five checks and tracks the
 *          maximum. Uses a fixed-size array to avoid branch explosion and
 *          keep cyclomatic complexity low for IEC 62304 review.
 *
 *          Respiration rate special case: if vitals->respiration_rate == 0
 *          the parameter is considered "not measured" and is excluded from
 *          the aggregate so that a missing RR sensor does not raise a
 *          spurious alarm.
 */
AlertLevel overall_alert_level(const VitalSigns *vitals)
{
    AlertLevel levels[5];
    AlertLevel max = ALERT_NORMAL;
    int i;

    levels[0] = check_heart_rate(vitals->heart_rate);
    levels[1] = check_blood_pressure(vitals->systolic_bp, vitals->diastolic_bp);
    levels[2] = check_temperature(vitals->temperature);
    levels[3] = check_spo2(vitals->spo2);

    /* respiration_rate == 0 means the parameter was not measured this cycle;
     * treat it as ALERT_NORMAL rather than raising a spurious critical alert. */
    levels[4] = (vitals->respiration_rate == 0)
                    ? ALERT_NORMAL
                    : check_respiration_rate(vitals->respiration_rate);

    for (i = 0; i < 5; i++) {
        if (levels[i] > max) max = levels[i];
    }
    return max;
}

/**
 * @brief Calculate BMI using the standard WHO formula.
 * @details BMI = weight_kg / (height_m^2).
 *          Returns -1.0 for invalid (zero or negative) height to allow callers
 *          to distinguish an error from a legitimate very-low BMI.
 */
float calculate_bmi(float weight_kg, float height_m)
{
    if (height_m <= 0.0f) return -1.0f;
    return weight_kg / (height_m * height_m);
}

/**
 * @brief Map a BMI value to a WHO weight-status category string.
 * @details Uses half-open intervals consistent with WHO classification 2004.
 */
const char *bmi_category(float bmi)
{
    if (bmi < 0.0f)  return "Invalid";
    if (bmi < 18.5f) return "Underweight";
    if (bmi < 25.0f) return "Normal weight";
    if (bmi < 30.0f) return "Overweight";
    return "Obese";
}

/**
 * @brief Convert an AlertLevel enum to a display string.
 * @details Returns "UNKNOWN" for any value not in the enum to defend against
 *          undefined enum casts — important for IEC 62304 defensive coding.
 */
const char *alert_level_str(AlertLevel level)
{
    switch (level) {
        case ALERT_NORMAL:   return "NORMAL";
        case ALERT_WARNING:  return "WARNING";
        case ALERT_CRITICAL: return "CRITICAL";
        default:             return "UNKNOWN";
    }
}
