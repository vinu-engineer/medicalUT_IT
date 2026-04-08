/**
 * @file trend.h
 * @brief Vital signs trend analysis — direction and sparkline data extraction.
 *
 * @details
 * This module computes the trend direction for a time-series of vital sign
 * readings stored in a PatientRecord. The trend is determined by comparing
 * the mean of the first half of readings to the mean of the second half.
 * A threshold of ±5 % of the range prevents noise from generating spurious
 * trend signals (hysteresis).
 *
 * The module also provides helper functions to extract a single vital parameter
 * from an array of VitalSigns readings for use by the GUI sparkline renderer.
 *
 * ### IEC 62304 Traceability
 * - Software Unit: UNIT-TRD
 * - Requirements covered: SWR-TRD-001
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 *
 * @copyright Medical Device Software — IEC 62304 Class B compliant.
 *            All rights reserved.
 *
 * @req SWR-TRD-001
 */

#ifndef TREND_H
#define TREND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vitals.h"

/* =========================================================================
 * Trend Direction Enumeration
 * ========================================================================= */

/**
 * @brief Direction of change of a vital sign over the available readings.
 */
typedef enum {
    TREND_STABLE  = 0,  /**< Parameter is not changing significantly.   */
    TREND_RISING  = 1,  /**< Parameter is trending upward.              */
    TREND_FALLING = 2   /**< Parameter is trending downward.            */
} TrendDir;

/* =========================================================================
 * Value Extraction Helpers
 * ========================================================================= */

/**
 * @brief Copy up to @p max_out heart-rate values from @p readings into @p out.
 *
 * @param[in]  readings  Array of VitalSigns readings.
 * @param[in]  count     Number of entries in @p readings.
 * @param[out] out       Output buffer; receives one int per reading.
 * @param[in]  max_out   Capacity of @p out.
 * @return Number of values written (min(count, max_out)).
 *
 * @req SWR-TRD-001
 */
int trend_extract_hr(const VitalSigns *readings, int count,
                     int *out, int max_out);

/**
 * @brief Copy up to @p max_out systolic-BP values.
 * @req SWR-TRD-001
 */
int trend_extract_sbp(const VitalSigns *readings, int count,
                      int *out, int max_out);

/**
 * @brief Copy up to @p max_out temperature values (scaled to int ×10, e.g. 36.7→367).
 * @req SWR-TRD-001
 */
int trend_extract_temp(const VitalSigns *readings, int count,
                       int *out, int max_out);

/**
 * @brief Copy up to @p max_out SpO2 values.
 * @req SWR-TRD-001
 */
int trend_extract_spo2(const VitalSigns *readings, int count,
                       int *out, int max_out);

/**
 * @brief Copy up to @p max_out respiration-rate values.
 *        Entries where respiration_rate == 0 (not measured) are written as 0.
 * @req SWR-TRD-001
 */
int trend_extract_rr(const VitalSigns *readings, int count,
                     int *out, int max_out);

/* =========================================================================
 * Trend Direction
 * ========================================================================= */

/**
 * @brief Determine the trend direction of an integer value series.
 *
 * @details
 * Algorithm: compare the mean of the first half vs the mean of the second half.
 * If the difference exceeds a 5 % hysteresis band relative to the overall range,
 * TREND_RISING or TREND_FALLING is returned; otherwise TREND_STABLE.
 *
 * Special cases:
 * - count == 0 or @p values is NULL → TREND_STABLE
 * - count == 1                        → TREND_STABLE (no trend possible)
 * - all values equal                  → TREND_STABLE
 *
 * @param[in] values  Array of integer vital values (time-ordered oldest→newest).
 * @param[in] count   Number of elements in @p values.
 * @return TrendDir indicating direction of change.
 *
 * @req SWR-TRD-001
 */
TrendDir trend_direction(const int *values, int count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TREND_H */
