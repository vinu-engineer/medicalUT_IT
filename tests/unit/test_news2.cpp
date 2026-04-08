/**
 * @file test_news2.cpp
 * @brief Unit tests for the NEWS2 (National Early Warning Score 2) module.
 *
 * @req SWR-NEW-001
 *
 * Tests exercise every sub-score function at all scoring boundaries using
 * equivalence partitioning and boundary-value analysis as required by
 * IEC 62304 Class B.
 *
 * Reference: Royal College of Physicians. National Early Warning Score
 *            (NEWS) 2. London: RCP, 2017.
 */

#include <gtest/gtest.h>

extern "C" {
#include "news2.h"
}

// =============================================================
// SWR-NEW-001  news2_score_hr()  — heart rate sub-score
// =============================================================

TEST(News2HR, At40_Score3)    { EXPECT_EQ(news2_score_hr(40),  3); }
TEST(News2HR, At41_Score1)    { EXPECT_EQ(news2_score_hr(41),  1); }
TEST(News2HR, At50_Score1)    { EXPECT_EQ(news2_score_hr(50),  1); }
TEST(News2HR, At51_Score0)    { EXPECT_EQ(news2_score_hr(51),  0); }
TEST(News2HR, At90_Score0)    { EXPECT_EQ(news2_score_hr(90),  0); }
TEST(News2HR, At91_Score1)    { EXPECT_EQ(news2_score_hr(91),  1); }
TEST(News2HR, At110_Score1)   { EXPECT_EQ(news2_score_hr(110), 1); }
TEST(News2HR, At111_Score2)   { EXPECT_EQ(news2_score_hr(111), 2); }
TEST(News2HR, At130_Score2)   { EXPECT_EQ(news2_score_hr(130), 2); }
TEST(News2HR, At131_Score3)   { EXPECT_EQ(news2_score_hr(131), 3); }
TEST(News2HR, At200_Score3)   { EXPECT_EQ(news2_score_hr(200), 3); }
TEST(News2HR, AtNeg_Score3)   { EXPECT_EQ(news2_score_hr(-1),  3); }

// =============================================================
// SWR-NEW-001  news2_score_rr()  — respiration rate sub-score
// =============================================================

TEST(News2RR, At8_Score3)     { EXPECT_EQ(news2_score_rr(8),   3); }
TEST(News2RR, At9_Score1)     { EXPECT_EQ(news2_score_rr(9),   1); }
TEST(News2RR, At11_Score1)    { EXPECT_EQ(news2_score_rr(11),  1); }
TEST(News2RR, At12_Score0)    { EXPECT_EQ(news2_score_rr(12),  0); }
TEST(News2RR, At20_Score0)    { EXPECT_EQ(news2_score_rr(20),  0); }
TEST(News2RR, At21_Score2)    { EXPECT_EQ(news2_score_rr(21),  2); }
TEST(News2RR, At24_Score2)    { EXPECT_EQ(news2_score_rr(24),  2); }
TEST(News2RR, At25_Score3)    { EXPECT_EQ(news2_score_rr(25),  3); }
TEST(News2RR, At40_Score3)    { EXPECT_EQ(news2_score_rr(40),  3); }

// =============================================================
// SWR-NEW-001  news2_score_spo2()  — SpO2 sub-score
// =============================================================

TEST(News2SpO2, At91_Score3)  { EXPECT_EQ(news2_score_spo2(91), 3); }
TEST(News2SpO2, At92_Score2)  { EXPECT_EQ(news2_score_spo2(92), 2); }
TEST(News2SpO2, At93_Score2)  { EXPECT_EQ(news2_score_spo2(93), 2); }
TEST(News2SpO2, At94_Score1)  { EXPECT_EQ(news2_score_spo2(94), 1); }
TEST(News2SpO2, At95_Score1)  { EXPECT_EQ(news2_score_spo2(95), 1); }
TEST(News2SpO2, At96_Score0)  { EXPECT_EQ(news2_score_spo2(96), 0); }
TEST(News2SpO2, At100_Score0) { EXPECT_EQ(news2_score_spo2(100),0); }

// =============================================================
// SWR-NEW-001  news2_score_sbp()  — systolic BP sub-score
// =============================================================

TEST(News2SBP, At90_Score3)   { EXPECT_EQ(news2_score_sbp(90),  3); }
TEST(News2SBP, At91_Score2)   { EXPECT_EQ(news2_score_sbp(91),  2); }
TEST(News2SBP, At100_Score2)  { EXPECT_EQ(news2_score_sbp(100), 2); }
TEST(News2SBP, At101_Score1)  { EXPECT_EQ(news2_score_sbp(101), 1); }
TEST(News2SBP, At110_Score1)  { EXPECT_EQ(news2_score_sbp(110), 1); }
TEST(News2SBP, At111_Score0)  { EXPECT_EQ(news2_score_sbp(111), 0); }
TEST(News2SBP, At219_Score0)  { EXPECT_EQ(news2_score_sbp(219), 0); }
TEST(News2SBP, At220_Score3)  { EXPECT_EQ(news2_score_sbp(220), 3); }

// =============================================================
// SWR-NEW-001  news2_score_temp()  — temperature sub-score
// =============================================================

TEST(News2Temp, At35p0_Score3)  { EXPECT_EQ(news2_score_temp(35.0f), 3); }
TEST(News2Temp, At35p1_Score1)  { EXPECT_EQ(news2_score_temp(35.1f), 1); }
TEST(News2Temp, At36p0_Score1)  { EXPECT_EQ(news2_score_temp(36.0f), 1); }
TEST(News2Temp, At36p1_Score0)  { EXPECT_EQ(news2_score_temp(36.1f), 0); }
TEST(News2Temp, At38p0_Score0)  { EXPECT_EQ(news2_score_temp(38.0f), 0); }
TEST(News2Temp, At38p1_Score1)  { EXPECT_EQ(news2_score_temp(38.1f), 1); }
TEST(News2Temp, At39p0_Score1)  { EXPECT_EQ(news2_score_temp(39.0f), 1); }
TEST(News2Temp, At39p1_Score2)  { EXPECT_EQ(news2_score_temp(39.1f), 2); }
TEST(News2Temp, At40p0_Score2)  { EXPECT_EQ(news2_score_temp(40.0f), 2); }

// =============================================================
// SWR-NEW-001  news2_calculate() — aggregate and risk category
// =============================================================

TEST(News2Calc, AllNormal_ScoreZero_RiskLow) {
    /* Fully normal patient: HR=75, RR=16, SpO2=98, SBP=120, Temp=36.5, AVPU=Alert */
    VitalSigns v = {75, 120, 80, 36.5f, 98, 16};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_EQ(r.score_hr,   0);
    EXPECT_EQ(r.score_rr,   0);
    EXPECT_EQ(r.score_spo2, 0);
    EXPECT_EQ(r.score_sbp,  0);
    EXPECT_EQ(r.score_temp, 0);
    EXPECT_EQ(r.score_avpu, 0);
    EXPECT_EQ(r.total_score, 0);
    EXPECT_EQ(r.risk, NEWS2_LOW);
}

TEST(News2Calc, ModerateAbnormal_LowMediumRisk) {
    /* HR=110 (score 1), RR=22 (score 2), rest normal → total 3 → LOW_M */
    VitalSigns v = {110, 120, 80, 36.5f, 98, 22};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_EQ(r.score_hr, 1);
    EXPECT_EQ(r.score_rr, 2);
    EXPECT_EQ(r.total_score, 3);
    EXPECT_EQ(r.risk, NEWS2_LOW_M);
}

TEST(News2Calc, TotalFive_MediumRisk) {
    /* HR=111 (2), RR=21 (2), SpO2=94 (1) → total 5 → MEDIUM */
    VitalSigns v = {111, 120, 80, 36.5f, 94, 21};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_GE(r.total_score, 5);
    EXPECT_EQ(r.risk, NEWS2_MEDIUM);
}

TEST(News2Calc, AnySingleScore3_MediumRisk) {
    /* HR=41 (1), SBP=90 (3) → total 4 BUT any_three → MEDIUM */
    VitalSigns v = {41, 90, 80, 36.5f, 98, 15};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_EQ(r.score_sbp, 3);
    EXPECT_EQ(r.risk, NEWS2_MEDIUM);
}

TEST(News2Calc, TotalSeven_HighRisk) {
    /* HR=131 (3), RR=25 (3), SpO2=91 (3) → total 9 → HIGH */
    VitalSigns v = {131, 120, 80, 36.5f, 91, 25};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_GE(r.total_score, 7);
    EXPECT_EQ(r.risk, NEWS2_HIGH);
}

TEST(News2Calc, AVPU_NonAlert_AddsThree) {
    /* Normal vitals but AVPU=3 → total 3 → LOW_M, and any_three triggers MEDIUM */
    VitalSigns v = {75, 120, 80, 36.5f, 98, 15};
    News2Result r;
    news2_calculate(&v, 3, &r);
    EXPECT_EQ(r.score_avpu, 3);
    EXPECT_EQ(r.total_score, 3);
    /* total=3 and any_three (AVPU=3) → MEDIUM */
    EXPECT_EQ(r.risk, NEWS2_MEDIUM);
}

TEST(News2Calc, RR_Zero_SkippedInScore) {
    /* RR=0 means not measured — score_rr must be 0, not 3 */
    VitalSigns v = {75, 120, 80, 36.5f, 98, 0};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_EQ(r.score_rr, 0) << "RR=0 (not measured) must contribute score 0";
    EXPECT_EQ(r.total_score, 0);
    EXPECT_EQ(r.risk, NEWS2_LOW);
}

TEST(News2Calc, RiskLabelsAndResponsesNotNull) {
    VitalSigns v = {75, 120, 80, 36.5f, 98, 15};
    News2Result r;
    news2_calculate(&v, 0, &r);
    EXPECT_NE(r.risk_label, nullptr) << "risk_label must not be NULL";
    EXPECT_NE(r.response,   nullptr) << "response must not be NULL";
}
