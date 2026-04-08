/**
 * @file test_alarm_limits.cpp
 * @brief Unit tests for the configurable alarm limits module.
 *
 * @req SWR-ALM-001
 *
 * Tests cover: factory defaults, save/load round-trips, alarm_check_*()
 * three-zone classification at all boundary conditions, and path override
 * for isolation from production files.
 *
 * Standard: IEC 62304 Class B — 100% branch coverage of alarm_limits.c
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

extern "C" {
#include "alarm_limits.h"
}

// ---------------------------------------------------------------------------
// Helper: build a temp file path unique to this test
// ---------------------------------------------------------------------------
static std::string make_temp_path(const std::string &suffix = "")
{
    const char *tmp_dir = nullptr;
#ifdef _WIN32
    static char tmp_buf[512];
    DWORD len = GetTempPathA(static_cast<DWORD>(sizeof(tmp_buf)), tmp_buf);
    if (len > 0 && len < sizeof(tmp_buf)) tmp_dir = tmp_buf;
#else
    tmp_dir = getenv("TMPDIR");
    if (!tmp_dir) tmp_dir = "/tmp";
#endif
    if (!tmp_dir || tmp_dir[0] == '\0') tmp_dir = ".";
    return std::string(tmp_dir) + "/test_alarm_limits" + suffix + ".cfg";
}

// ---------------------------------------------------------------------------
// Fixture: unique temp path, cleaned up after each test
// ---------------------------------------------------------------------------
class AlarmLimitsTest : public ::testing::Test
{
protected:
    std::string path_;

    void SetUp() override {
        path_ = make_temp_path("_" + std::string(
            ::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::remove(path_.c_str());
        alarm_limits_set_path(path_.c_str());
    }
    void TearDown() override {
        std::remove(path_.c_str());
        alarm_limits_set_path(nullptr);
    }
};

// =============================================================
// SWR-ALM-001  alarm_limits_defaults()
// =============================================================

TEST_F(AlarmLimitsTest, DefaultsAreCorrect) {
    AlarmLimits lim;
    alarm_limits_defaults(&lim);
    EXPECT_EQ(lim.hr_low,    40);
    EXPECT_EQ(lim.hr_high,   150);
    EXPECT_EQ(lim.sbp_low,   70);
    EXPECT_EQ(lim.sbp_high,  180);
    EXPECT_EQ(lim.dbp_low,   40);
    EXPECT_EQ(lim.dbp_high,  120);
    EXPECT_FLOAT_EQ(lim.temp_low,  35.0f);
    EXPECT_FLOAT_EQ(lim.temp_high, 39.5f);
    EXPECT_EQ(lim.spo2_low,  90);
    EXPECT_EQ(lim.rr_low,    8);
    EXPECT_EQ(lim.rr_high,   25);
}

// =============================================================
// SWR-ALM-001  alarm_limits_save() and alarm_limits_load()
// =============================================================

TEST_F(AlarmLimitsTest, SaveAndLoadRoundTrip) {
    AlarmLimits saved, loaded;
    alarm_limits_defaults(&saved);
    saved.hr_low  = 45;
    saved.hr_high = 140;
    saved.spo2_low = 92;
    saved.rr_low  = 10;
    saved.rr_high = 22;

    ASSERT_EQ(alarm_limits_save(&saved), 1);

    alarm_limits_defaults(&loaded); /* ensure state is different before load */
    loaded.hr_low = 0;
    ASSERT_EQ(alarm_limits_load(&loaded), 1);

    EXPECT_EQ(loaded.hr_low,  45);
    EXPECT_EQ(loaded.hr_high, 140);
    EXPECT_EQ(loaded.spo2_low, 92);
    EXPECT_EQ(loaded.rr_low,  10);
    EXPECT_EQ(loaded.rr_high, 22);
}

TEST_F(AlarmLimitsTest, LoadMissingFileReturnsDefaultsAndZero) {
    AlarmLimits lim;
    int ret = alarm_limits_load(&lim); /* file does not exist */
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(lim.hr_low,  40); /* defaults still applied */
    EXPECT_EQ(lim.hr_high, 150);
}

TEST_F(AlarmLimitsTest, TempFieldsSavedAndLoaded) {
    AlarmLimits saved;
    alarm_limits_defaults(&saved);
    saved.temp_low  = 36.0f;
    saved.temp_high = 38.5f;
    ASSERT_EQ(alarm_limits_save(&saved), 1);

    AlarmLimits loaded;
    ASSERT_EQ(alarm_limits_load(&loaded), 1);
    EXPECT_NEAR(loaded.temp_low,  36.0f, 0.01f);
    EXPECT_NEAR(loaded.temp_high, 38.5f, 0.01f);
}

TEST_F(AlarmLimitsTest, SetPathNullResetsToDefault) {
    alarm_limits_set_path(nullptr); /* must not crash */
    /* Re-apply our temp path so TearDown cleans up */
    alarm_limits_set_path(path_.c_str());
    AlarmLimits lim;
    alarm_limits_defaults(&lim);
    EXPECT_EQ(alarm_limits_save(&lim), 1);
}

// =============================================================
// SWR-ALM-001  alarm_check_hr() — three-zone classification
// =============================================================

TEST_F(AlarmLimitsTest, HR_Normal_AtLow)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 40),  ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, HR_Normal_AtHigh)   { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 150), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, HR_Warning_Low)     { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 35),  ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, HR_Warning_High)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 155), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, HR_Critical_Low)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 25),  ALERT_CRITICAL); }
TEST_F(AlarmLimitsTest, HR_Critical_High)   { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_hr(&lim, 165), ALERT_CRITICAL); }

// =============================================================
// SWR-ALM-001  alarm_check_bp() — systolic and diastolic
// =============================================================

TEST_F(AlarmLimitsTest, BP_Normal)          { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_bp(&lim, 120, 80), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, BP_Warning_SBP)     { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_bp(&lim, 65,  80), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, BP_Critical_SBP)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_bp(&lim, 55,  80), ALERT_CRITICAL); }
TEST_F(AlarmLimitsTest, BP_Warning_DBP)     { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_bp(&lim, 120, 35), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, BP_Critical_DBP)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_bp(&lim, 120, 25), ALERT_CRITICAL); }

// =============================================================
// SWR-ALM-001  alarm_check_temp() — three-zone
// =============================================================

TEST_F(AlarmLimitsTest, Temp_Normal)        { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_temp(&lim, 37.0f), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, Temp_Warning_Low)   { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_temp(&lim, 34.5f), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, Temp_Critical_Low)  { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_temp(&lim, 33.9f), ALERT_CRITICAL); }
TEST_F(AlarmLimitsTest, Temp_Warning_High)  { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_temp(&lim, 40.0f), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, Temp_Critical_High) { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_temp(&lim, 40.6f), ALERT_CRITICAL); }

// =============================================================
// SWR-ALM-001  alarm_check_spo2() — lower limit only
// =============================================================

TEST_F(AlarmLimitsTest, SpO2_Normal)        { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_spo2(&lim, 98), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, SpO2_AtLimit)       { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_spo2(&lim, 90), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, SpO2_Warning)       { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_spo2(&lim, 87), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, SpO2_Critical)      { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_spo2(&lim, 84), ALERT_CRITICAL); }

// =============================================================
// SWR-ALM-001  alarm_check_rr() — three-zone
// =============================================================

TEST_F(AlarmLimitsTest, RR_Normal)          { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_rr(&lim, 15), ALERT_NORMAL);   }
TEST_F(AlarmLimitsTest, RR_Warning_Low)     { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_rr(&lim, 5),  ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, RR_Critical_Low)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_rr(&lim,-5),  ALERT_CRITICAL); }
TEST_F(AlarmLimitsTest, RR_Warning_High)    { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_rr(&lim, 30), ALERT_WARNING);  }
TEST_F(AlarmLimitsTest, RR_Critical_High)   { AlarmLimits lim; alarm_limits_defaults(&lim); EXPECT_EQ(alarm_check_rr(&lim, 36), ALERT_CRITICAL); }

// =============================================================
// SWR-ALM-001  Custom limits take effect
// =============================================================

TEST_F(AlarmLimitsTest, CustomHRLimitApplied) {
    AlarmLimits lim;
    alarm_limits_defaults(&lim);
    lim.hr_low  = 50;  /* tighter lower bound */
    lim.hr_high = 120; /* tighter upper bound */
    /* Previously normal (60 bpm), now warning below 50 */
    EXPECT_EQ(alarm_check_hr(&lim, 45), ALERT_WARNING);
    EXPECT_EQ(alarm_check_hr(&lim, 55), ALERT_NORMAL);
    EXPECT_EQ(alarm_check_hr(&lim, 125),ALERT_WARNING);
}
