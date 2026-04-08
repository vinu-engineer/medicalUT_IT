/**
 * @file test_trend.cpp
 * @brief Unit tests for the vital signs trend analysis module.
 *
 * @req SWR-TRD-001
 *
 * Tests: trend_direction() boundary and edge cases; all five
 * trend_extract_*() helpers.
 */

#include <gtest/gtest.h>
extern "C" {
#include "trend.h"
#include "patient.h"
}

// =============================================================
// SWR-TRD-001  trend_direction() edge cases
// =============================================================

TEST(TrendDirection, NullInput_Stable)     { EXPECT_EQ(trend_direction(nullptr, 5), TREND_STABLE); }
TEST(TrendDirection, ZeroCount_Stable)     { int v[]={1,2,3}; EXPECT_EQ(trend_direction(v, 0), TREND_STABLE); }
TEST(TrendDirection, OneEntry_Stable)      { int v[]={80}; EXPECT_EQ(trend_direction(v, 1), TREND_STABLE); }

TEST(TrendDirection, AllSame_Stable) {
    int v[] = {80, 80, 80, 80, 80};
    EXPECT_EQ(trend_direction(v, 5), TREND_STABLE);
}

TEST(TrendDirection, ClearlyRising) {
    int v[] = {60, 65, 70, 80, 90, 100, 115, 130};
    EXPECT_EQ(trend_direction(v, 8), TREND_RISING);
}

TEST(TrendDirection, ClearlyFalling) {
    int v[] = {130, 120, 110, 100, 90, 80, 70, 60};
    EXPECT_EQ(trend_direction(v, 8), TREND_FALLING);
}

TEST(TrendDirection, FlatWithNoise_Stable) {
    /* Noise within 5% hysteresis band — should read STABLE */
    int v[] = {100, 101, 99, 100, 101, 100, 99, 100};
    EXPECT_EQ(trend_direction(v, 8), TREND_STABLE);
}

TEST(TrendDirection, TwoElements_Rising) {
    int v[] = {60, 120};
    EXPECT_EQ(trend_direction(v, 2), TREND_RISING);
}

TEST(TrendDirection, TwoElements_Falling) {
    int v[] = {120, 60};
    EXPECT_EQ(trend_direction(v, 2), TREND_FALLING);
}

TEST(TrendDirection, TwoElements_Equal_Stable) {
    int v[] = {80, 80};
    EXPECT_EQ(trend_direction(v, 2), TREND_STABLE);
}

// =============================================================
// SWR-TRD-001  trend_extract_hr() helper
// =============================================================

TEST(TrendExtract, HR_ExtractsValues) {
    VitalSigns v[3];
    v[0].heart_rate = 70; v[0].systolic_bp=120; v[0].diastolic_bp=80;
    v[0].temperature=36.6f; v[0].spo2=98; v[0].respiration_rate=15;
    v[1] = v[0]; v[1].heart_rate = 80;
    v[2] = v[0]; v[2].heart_rate = 90;

    int out[10];
    int n = trend_extract_hr(v, 3, out, 10);
    ASSERT_EQ(n, 3);
    EXPECT_EQ(out[0], 70);
    EXPECT_EQ(out[1], 80);
    EXPECT_EQ(out[2], 90);
}

TEST(TrendExtract, HR_RespectMaxOut) {
    VitalSigns v[5];
    for (int i = 0; i < 5; ++i) {
        v[i].heart_rate = 60 + i*5;
        v[i].systolic_bp=120; v[i].diastolic_bp=80;
        v[i].temperature=36.6f; v[i].spo2=98; v[i].respiration_rate=15;
    }
    int out[3];
    int n = trend_extract_hr(v, 5, out, 3);
    EXPECT_EQ(n, 3); /* capped at max_out */
}

TEST(TrendExtract, HR_NullInput_Zero) {
    int out[5];
    EXPECT_EQ(trend_extract_hr(nullptr, 5, out, 5), 0);
    EXPECT_EQ(trend_extract_hr(nullptr, 0, out, 5), 0);
}

// =============================================================
// SWR-TRD-001  trend_extract_sbp() helper
// =============================================================

TEST(TrendExtract, SBP_ExtractsValues) {
    VitalSigns v[2];
    v[0].heart_rate=80; v[0].systolic_bp=110; v[0].diastolic_bp=70;
    v[0].temperature=36.6f; v[0].spo2=98; v[0].respiration_rate=15;
    v[1]=v[0]; v[1].systolic_bp=140;
    int out[5];
    EXPECT_EQ(trend_extract_sbp(v, 2, out, 5), 2);
    EXPECT_EQ(out[0], 110);
    EXPECT_EQ(out[1], 140);
}

// =============================================================
// SWR-TRD-001  trend_extract_temp() helper (scaled ×10)
// =============================================================

TEST(TrendExtract, Temp_ScaledByTen) {
    VitalSigns v[2];
    v[0].heart_rate=80; v[0].systolic_bp=120; v[0].diastolic_bp=80;
    v[0].temperature=36.7f; v[0].spo2=98; v[0].respiration_rate=15;
    v[1]=v[0]; v[1].temperature=38.5f;
    int out[5];
    EXPECT_EQ(trend_extract_temp(v, 2, out, 5), 2);
    EXPECT_EQ(out[0], 367);
    EXPECT_EQ(out[1], 385);
}

// =============================================================
// SWR-TRD-001  trend_extract_spo2() helper
// =============================================================

TEST(TrendExtract, SpO2_ExtractsValues) {
    VitalSigns v[2];
    v[0].heart_rate=80; v[0].systolic_bp=120; v[0].diastolic_bp=80;
    v[0].temperature=36.6f; v[0].spo2=98; v[0].respiration_rate=15;
    v[1]=v[0]; v[1].spo2=92;
    int out[5];
    EXPECT_EQ(trend_extract_spo2(v, 2, out, 5), 2);
    EXPECT_EQ(out[0], 98);
    EXPECT_EQ(out[1], 92);
}

// =============================================================
// SWR-TRD-001  trend_extract_rr() helper
// =============================================================

TEST(TrendExtract, RR_ExtractsZeroForNotMeasured) {
    VitalSigns v[2];
    v[0].heart_rate=80; v[0].systolic_bp=120; v[0].diastolic_bp=80;
    v[0].temperature=36.6f; v[0].spo2=98; v[0].respiration_rate=0;
    v[1]=v[0]; v[1].respiration_rate=16;
    int out[5];
    EXPECT_EQ(trend_extract_rr(v, 2, out, 5), 2);
    EXPECT_EQ(out[0], 0);  /* not measured */
    EXPECT_EQ(out[1], 16);
}

// =============================================================
// End-to-end: extract HR from patient record and get trend
// =============================================================

TEST(TrendExtract, PatientHistoryRisingTrend) {
    PatientRecord rec;
    patient_init(&rec, 1, "Test", 40, 70.0f, 1.75f);

    VitalSigns v;
    v.systolic_bp=120; v.diastolic_bp=80; v.temperature=36.6f;
    v.spo2=98; v.respiration_rate=15;
    /* Add readings with clearly rising HR */
    const int hrs[] = {60, 70, 80, 90, 100, 110, 120, 130};
    for (int i = 0; i < 8; ++i) {
        v.heart_rate = hrs[i];
        patient_add_reading(&rec, &v);
    }

    int buf[MAX_READINGS];
    int n = trend_extract_hr(rec.readings, rec.reading_count, buf, MAX_READINGS);
    ASSERT_EQ(n, 8);
    EXPECT_EQ(trend_direction(buf, n), TREND_RISING);
}
