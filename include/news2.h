/**
 * @file news2.h
 * @brief NEWS2 (National Early Warning Score 2) scoring module.
 *
 * @details
 * This module implements the Royal College of Physicians NEWS2 scoring
 * system as defined in the 2017 RCP report "National Early Warning Score
 * (NEWS) 2". NEWS2 assigns a sub-score to each of six physiological
 * parameters plus consciousness level (AVPU), sums them to a total score,
 * and maps the result to one of four clinical risk categories.
 *
 * Only SpO2 Scale 1 (the standard scale for patients not on target
 * oxygen saturation of 88–92 %) is implemented here. Scale 2 for
 * hypercapnic respiratory failure is outside the scope of this module.
 *
 * ### IEC 62304 Traceability
 * - Software Unit: UNIT-NEW
 * - Requirements covered: SWR-NEW-001
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 *
 * @copyright Medical Device Software — IEC 62304 Class B compliant.
 *            All rights reserved.
 *
 * @req SWR-NEW-001
 */

#ifndef NEWS2_H
#define NEWS2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vitals.h"

/* =========================================================================
 * NEWS2 Risk Enumeration
 * ========================================================================= */

/**
 * @brief NEWS2 aggregate clinical risk category.
 *
 * @details Numeric values are ordered from lowest to highest risk,
 * permitting comparisons such as `if (risk >= NEWS2_MEDIUM)`.
 */
typedef enum {
    NEWS2_LOW    = 0,   /**< Total score = 0.                                   */
    NEWS2_LOW_M  = 1,   /**< Total score 1–4 (no single parameter score of 3).  */
    NEWS2_MEDIUM = 2,   /**< Total score 5–6, OR any single parameter score = 3.*/
    NEWS2_HIGH   = 3    /**< Total score >= 7.                                  */
} News2Risk;

/* =========================================================================
 * NEWS2 Result Structure
 * ========================================================================= */

/**
 * @brief Full NEWS2 calculation result.
 *
 * @details Populated by news2_calculate(). All sub-scores are in the range
 * 0–3. The total_score field is the arithmetic sum of all six physiological
 * sub-scores plus the AVPU score (range 0–20).
 */
typedef struct {
    int        total_score;  /**< Aggregate NEWS2 score (0–20).                 */
    int        score_hr;     /**< Heart rate sub-score (0–3).                   */
    int        score_rr;     /**< Respiration rate sub-score (0–3).             */
    int        score_spo2;   /**< SpO2 Scale 1 sub-score (0–3).                 */
    int        score_sbp;    /**< Systolic blood pressure sub-score (0–3).      */
    int        score_temp;   /**< Temperature sub-score (0–3).                  */
    int        score_avpu;   /**< AVPU consciousness sub-score (0 or 3).        */
    News2Risk  risk;         /**< Derived risk category.                        */
    const char *risk_label;  /**< Human-readable risk label (never NULL).       */
    const char *response;    /**< Recommended clinical response (never NULL).   */
} News2Result;

/* =========================================================================
 * Sub-score Functions
 * ========================================================================= */

/**
 * @brief Compute the NEWS2 heart-rate sub-score.
 *
 * @details Scoring per RCP NEWS2 table:
 * | Range (bpm) | Score |
 * |-------------|-------|
 * | ≤40         | 3     |
 * | 41–50       | 1     |
 * | 51–90       | 0     |
 * | 91–110      | 1     |
 * | 111–130     | 2     |
 * | ≥131        | 3     |
 *
 * @param[in] bpm Heart rate in beats per minute.
 * @return Sub-score in range [0, 3].
 *
 * @par Requirement
 * SWR-NEW-001
 */
int news2_score_hr(int bpm);

/**
 * @brief Compute the NEWS2 respiration-rate sub-score.
 *
 * @details Scoring per RCP NEWS2 table:
 * | Range (br/min) | Score |
 * |----------------|-------|
 * | ≤8             | 3     |
 * | 9–11           | 1     |
 * | 12–20          | 0     |
 * | 21–24          | 2     |
 * | ≥25            | 3     |
 *
 * @param[in] rr_bpm Respiration rate in breaths per minute.
 * @return Sub-score in range [0, 3].
 *
 * @par Requirement
 * SWR-NEW-001
 */
int news2_score_rr(int rr_bpm);

/**
 * @brief Compute the NEWS2 SpO2 sub-score (Scale 1).
 *
 * @details Scoring per RCP NEWS2 Scale 1 table:
 * | Range (%)  | Score |
 * |------------|-------|
 * | ≤91        | 3     |
 * | 92–93      | 2     |
 * | 94–95      | 1     |
 * | ≥96        | 0     |
 *
 * @param[in] spo2 Peripheral oxygen saturation as a percentage.
 * @return Sub-score in range [0, 3].
 *
 * @par Requirement
 * SWR-NEW-001
 */
int news2_score_spo2(int spo2);

/**
 * @brief Compute the NEWS2 systolic blood pressure sub-score.
 *
 * @details Scoring per RCP NEWS2 table:
 * | Range (mmHg) | Score |
 * |--------------|-------|
 * | ≤90          | 3     |
 * | 91–100       | 2     |
 * | 101–110      | 1     |
 * | 111–219      | 0     |
 * | ≥220         | 3     |
 *
 * @param[in] sbp Systolic blood pressure in mmHg.
 * @return Sub-score in range [0, 3].
 *
 * @par Requirement
 * SWR-NEW-001
 */
int news2_score_sbp(int sbp);

/**
 * @brief Compute the NEWS2 temperature sub-score.
 *
 * @details Scoring per RCP NEWS2 table:
 * | Range (°C)   | Score |
 * |--------------|-------|
 * | ≤35.0        | 3     |
 * | 35.1–36.0    | 1     |
 * | 36.1–38.0    | 0     |
 * | 38.1–39.0    | 1     |
 * | ≥39.1        | 2     |
 *
 * @param[in] temp_c Body temperature in degrees Celsius.
 * @return Sub-score in range [0, 3].
 *
 * @par Requirement
 * SWR-NEW-001
 */
int news2_score_temp(float temp_c);

/* =========================================================================
 * Aggregate Calculation
 * ========================================================================= */

/**
 * @brief Perform a full NEWS2 calculation and populate a result structure.
 *
 * @details Computes each sub-score, sums them (including the caller-supplied
 * AVPU score), determines the risk category and fills all fields of @p out.
 *
 * Risk classification:
 * - total >= 7                            → NEWS2_HIGH
 * - total 5–6 OR any single score == 3   → NEWS2_MEDIUM
 * - total 1–4                            → NEWS2_LOW_M
 * - total == 0                           → NEWS2_LOW
 *
 * If @p v->respiration_rate == 0 the RR sub-score is treated as 0 (parameter
 * not measured this cycle) and does not contribute to the total.
 *
 * @param[in]  v          Pointer to a populated VitalSigns structure. Must not
 *                        be NULL.
 * @param[in]  avpu_score AVPU consciousness score supplied by the caller:
 *                        0 = Alert, 3 = any of Voice/Pain/Unresponsive.
 * @param[out] out        Pointer to a News2Result to be filled. Must not be
 *                        NULL.
 *
 * @pre  v   != NULL
 * @pre  out != NULL
 * @pre  avpu_score == 0 || avpu_score == 3
 *
 * @par Requirement
 * SWR-NEW-001
 */
void news2_calculate(const VitalSigns *v, int avpu_score, News2Result *out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NEWS2_H */
