/**
 * @file news2.c
 * @brief Implementation of the NEWS2 (National Early Warning Score 2) module.
 *
 * @details
 * Implements all functions declared in news2.h using the official RCP NEWS2
 * scoring tables (Royal College of Physicians, 2017 edition).  Only SpO2
 * Scale 1 is implemented; Scale 2 for hypercapnic respiratory failure is
 * outside the scope of this compilation unit.
 *
 * No heap allocation is used. All state is passed in via parameters or
 * written to the caller-supplied News2Result output structure.
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 */

#include "news2.h"

/* =========================================================================
 * Sub-score Implementations
 * ========================================================================= */

/**
 * @brief Heart rate sub-score per RCP NEWS2 table.
 * @details <= 40 → 3, 41-50 → 1, 51-90 → 0, 91-110 → 1, 111-130 → 2, >= 131 → 3.
 */
int news2_score_hr(int bpm)
{
    if (bpm <= 40)  return 3;
    if (bpm <= 50)  return 1;
    if (bpm <= 90)  return 0;
    if (bpm <= 110) return 1;
    if (bpm <= 130) return 2;
    return 3;
}

/**
 * @brief Respiration rate sub-score per RCP NEWS2 table.
 * @details <= 8 → 3, 9-11 → 1, 12-20 → 0, 21-24 → 2, >= 25 → 3.
 */
int news2_score_rr(int rr_bpm)
{
    if (rr_bpm <= 8)  return 3;
    if (rr_bpm <= 11) return 1;
    if (rr_bpm <= 20) return 0;
    if (rr_bpm <= 24) return 2;
    return 3;
}

/**
 * @brief SpO2 Scale 1 sub-score per RCP NEWS2 table.
 * @details <= 91 → 3, 92-93 → 2, 94-95 → 1, >= 96 → 0.
 */
int news2_score_spo2(int spo2)
{
    if (spo2 <= 91) return 3;
    if (spo2 <= 93) return 2;
    if (spo2 <= 95) return 1;
    return 0;
}

/**
 * @brief Systolic blood pressure sub-score per RCP NEWS2 table.
 * @details <= 90 → 3, 91-100 → 2, 101-110 → 1, 111-219 → 0, >= 220 → 3.
 */
int news2_score_sbp(int sbp)
{
    if (sbp <= 90)  return 3;
    if (sbp <= 100) return 2;
    if (sbp <= 110) return 1;
    if (sbp <= 219) return 0;
    return 3;
}

/**
 * @brief Temperature sub-score per RCP NEWS2 table.
 * @details <= 35.0 → 3, 35.1-36.0 → 1, 36.1-38.0 → 0, 38.1-39.0 → 1, >= 39.1 → 2.
 */
int news2_score_temp(float temp_c)
{
    if (temp_c <= 35.0f) return 3;
    if (temp_c <= 36.0f) return 1;
    if (temp_c <= 38.0f) return 0;
    if (temp_c <= 39.0f) return 1;
    return 2;
}

/* =========================================================================
 * Aggregate Calculation
 * ========================================================================= */

/**
 * @brief Perform the full NEWS2 calculation and populate @p out.
 *
 * @details
 * Sub-scores are computed from the VitalSigns fields using the official RCP
 * NEWS2 scoring tables. The AVPU score is taken directly from the caller
 * (valid values: 0 for Alert, 3 for any other AVPU state).
 *
 * Respiration rate: if v->respiration_rate == 0 the parameter is treated as
 * "not measured" and the RR sub-score is set to 0 so it does not inflate the
 * aggregate total.
 *
 * Risk classification follows the RCP NEWS2 thresholds:
 *   - total >= 7                              → NEWS2_HIGH   ("EMERGENCY")
 *   - total 5-6 OR any single score == 3     → NEWS2_MEDIUM ("Urgent review")
 *   - total 1-4                              → NEWS2_LOW_M  ("Increase monitoring")
 *   - total == 0                             → NEWS2_LOW    ("Routine monitoring")
 */
void news2_calculate(const VitalSigns *v, int avpu_score, News2Result *out)
{
    int any_three;

    /* --- Compute individual sub-scores ----------------------------------- */
    out->score_hr   = news2_score_hr(v->heart_rate);
    out->score_spo2 = news2_score_spo2(v->spo2);
    out->score_sbp  = news2_score_sbp(v->systolic_bp);
    out->score_temp = news2_score_temp(v->temperature);
    out->score_avpu = avpu_score;

    /* Respiration rate == 0 means the sensor was not active this cycle;
     * treat the sub-score as 0 to avoid a spurious high-risk classification. */
    out->score_rr = (v->respiration_rate == 0)
                        ? 0
                        : news2_score_rr(v->respiration_rate);

    /* --- Sum all sub-scores ---------------------------------------------- */
    out->total_score = out->score_hr
                     + out->score_rr
                     + out->score_spo2
                     + out->score_sbp
                     + out->score_temp
                     + out->score_avpu;

    /* --- Determine whether any single parameter scored 3 ---------------- */
    any_three = (out->score_hr   == 3)
             || (out->score_rr   == 3)
             || (out->score_spo2 == 3)
             || (out->score_sbp  == 3)
             || (out->score_temp == 3)
             || (out->score_avpu == 3);

    /* --- Classify risk --------------------------------------------------- */
    if (out->total_score >= 7) {
        out->risk       = NEWS2_HIGH;
        out->risk_label = "HIGH";
        out->response   = "EMERGENCY — immediate response";
    } else if (out->total_score >= 5 || any_three) {
        out->risk       = NEWS2_MEDIUM;
        out->risk_label = "Medium";
        out->response   = "Urgent review required";
    } else if (out->total_score >= 1) {
        out->risk       = NEWS2_LOW_M;
        out->risk_label = "Low-Medium";
        out->response   = "Increase monitoring frequency";
    } else {
        out->risk       = NEWS2_LOW;
        out->risk_label = "Low";
        out->response   = "Routine monitoring";
    }
}
