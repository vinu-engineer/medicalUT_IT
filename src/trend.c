/**
 * @file trend.c
 * @brief Implementation of vital signs trend analysis.
 *
 * @details
 * Implements trend_direction() and the five trend_extract_*() helpers
 * declared in trend.h.  No heap allocation is used; the caller supplies
 * all output buffers.
 *
 * @version 1.0.0
 * @date    2026-04-08
 * @author  vinu-engineer
 */

#include "trend.h"
#include <stddef.h> /* NULL */

/* =========================================================================
 * Value Extraction Helpers
 * ========================================================================= */

/** Common extract helper — copies integer values into @p out. */
static int extract_generic(const int *src, int count, int *out, int max_out)
{
    int i, n;
    if (count <= 0 || src == NULL || out == NULL || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    for (i = 0; i < n; ++i) out[i] = src[i];
    return n;
}

int trend_extract_hr(const VitalSigns *readings, int count,
                     int *out, int max_out)
{
    int i, n;
    if (!readings || !out || count <= 0 || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    for (i = 0; i < n; ++i) out[i] = readings[i].heart_rate;
    return n;
}

int trend_extract_sbp(const VitalSigns *readings, int count,
                      int *out, int max_out)
{
    int i, n;
    if (!readings || !out || count <= 0 || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    for (i = 0; i < n; ++i) out[i] = readings[i].systolic_bp;
    return n;
}

int trend_extract_temp(const VitalSigns *readings, int count,
                       int *out, int max_out)
{
    int i, n;
    if (!readings || !out || count <= 0 || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    /* Scale float temperature to integer ×10 for uniform integer handling */
    for (i = 0; i < n; ++i) out[i] = (int)(readings[i].temperature * 10.0f + 0.5f);
    return n;
}

int trend_extract_spo2(const VitalSigns *readings, int count,
                       int *out, int max_out)
{
    int i, n;
    if (!readings || !out || count <= 0 || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    for (i = 0; i < n; ++i) out[i] = readings[i].spo2;
    return n;
}

int trend_extract_rr(const VitalSigns *readings, int count,
                     int *out, int max_out)
{
    int i, n;
    if (!readings || !out || count <= 0 || max_out <= 0) return 0;
    n = (count < max_out) ? count : max_out;
    for (i = 0; i < n; ++i) out[i] = readings[i].respiration_rate;
    return n;
}

/* Suppress unused-function warning for extract_generic on MSVC/GCC */
static int _suppress_extract_generic_warning(void) { return extract_generic(NULL,0,NULL,0); }

/* =========================================================================
 * Trend Direction
 * ========================================================================= */

/**
 * @brief Compute mean of @p values[start..start+len-1] (integer arithmetic).
 * @return Mean value, or 0 if len == 0.
 */
static int mean_segment(const int *values, int start, int len)
{
    int i, sum = 0;
    if (len <= 0) return 0;
    for (i = start; i < start + len; ++i) sum += values[i];
    /* Integer division rounds toward zero — acceptable for trend detection */
    return sum / len;
}

TrendDir trend_direction(const int *values, int count)
{
    int half, mean_first, mean_second, range, diff, threshold;
    int v_min, v_max, i;

    if (values == NULL || count < 2) return TREND_STABLE;

    /* Split into two halves; the later half captures more recent data. */
    half         = count / 2;
    mean_first   = mean_segment(values, 0,    count - half);
    mean_second  = mean_segment(values, count - half, half);

    /* Compute range for hysteresis threshold. */
    v_min = values[0]; v_max = values[0];
    for (i = 1; i < count; ++i) {
        if (values[i] < v_min) v_min = values[i];
        if (values[i] > v_max) v_max = values[i];
    }
    range = v_max - v_min;

    /* 5 % of range as hysteresis; minimum 1 to handle flat data. */
    threshold = (range * 5) / 100;
    if (threshold < 1) threshold = 1;

    diff = mean_second - mean_first;

    if (diff >  threshold) return TREND_RISING;
    if (diff < -threshold) return TREND_FALLING;
    return TREND_STABLE;
}
