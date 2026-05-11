#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern "C" {
#include "alarm_limits.h"
#include "session_export.h"
}

static std::string make_temp_path(const std::string &suffix)
{
    const char *tmp_dir = nullptr;

#ifdef _WIN32
    char tmp_buf[512] = {0};
    DWORD len = GetTempPathA(static_cast<DWORD>(sizeof(tmp_buf)), tmp_buf);
    if (len > 0 && len < sizeof(tmp_buf)) {
        tmp_dir = tmp_buf;
    }
#else
    tmp_dir = "/tmp";
#endif

    if (!tmp_dir || tmp_dir[0] == '\0') {
        tmp_dir = ".";
    }

    return std::string(tmp_dir) + "/test_session_export_integration" + suffix + ".txt";
}

static std::string read_text_file(const std::string &path)
{
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream),
                       std::istreambuf_iterator<char>());
}

class SessionExportIntegrationTest : public ::testing::Test
{
protected:
    std::string temp_path_;
    AlarmLimits limits_;

    void SetUp() override
    {
        temp_path_ = make_temp_path("_" + std::string(
            ::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::remove(temp_path_.c_str());
        alarm_limits_defaults(&limits_);
    }

    void TearDown() override
    {
        std::remove(temp_path_.c_str());
    }
};

TEST_F(SessionExportIntegrationTest, REQ_INT_EXP_001_StatusAndAlertsMatchPatientState)
{
    PatientRecord patient;
    Alert alerts[MAX_ALERTS];
    VitalSigns reading = {108, 148, 94, 37.9f, 93, 23};
    char alert_line[256];
    std::string content;
    int alert_count;
    const VitalSigns *latest;

    patient_init(&patient, 2001, "Critical Patient", 55, 90.0f, 1.72f);
    ASSERT_EQ(1, patient_add_reading(&patient, &reading));
    ASSERT_EQ(SESSION_EXPORT_RESULT_OK,
              session_export_write_snapshot(&patient, 1, &limits_,
                                            1, 0, temp_path_.c_str(), 0,
                                            nullptr, 0));

    content = read_text_file(temp_path_);
    EXPECT_NE(std::string::npos, content.find("Overall Status : WARNING"));

    latest = patient_latest_reading(&patient);
    ASSERT_NE(nullptr, latest);
    EXPECT_EQ(ALERT_WARNING, overall_alert_level(latest));
    EXPECT_EQ(ALERT_NORMAL, alarm_check_hr(&limits_, latest->heart_rate));
    EXPECT_EQ(ALERT_NORMAL, alarm_check_bp(&limits_,
                                          latest->systolic_bp,
                                          latest->diastolic_bp));
    EXPECT_EQ(ALERT_NORMAL, alarm_check_temp(&limits_, latest->temperature));
    EXPECT_EQ(ALERT_NORMAL, alarm_check_spo2(&limits_, latest->spo2));
    EXPECT_EQ(ALERT_NORMAL, alarm_check_rr(&limits_, latest->respiration_rate));
    EXPECT_NE(std::string::npos,
              content.find("Heart Rate     : low 40 bpm / high 150 bpm"));
    EXPECT_NE(std::string::npos,
              content.find("Status Basis   : Live dashboard clinical thresholds; configured alarm limits are listed above for session context."));

    alert_count = generate_alerts(latest, alerts, MAX_ALERTS);
    ASSERT_EQ(5, alert_count);

    for (int i = 0; i < alert_count; ++i) {
        ASSERT_EQ(1, session_export_format_alert_row(&alerts[i],
                                                     alert_line,
                                                     sizeof(alert_line)));
        EXPECT_NE(std::string::npos, content.find(alert_line));
    }
}

TEST_F(SessionExportIntegrationTest, REQ_INT_EXP_002_ReAdmitResetsPriorSessionIdentity)
{
    PatientRecord patient;
    VitalSigns first = {78, 122, 82, 36.7f, 98, 15};
    VitalSigns second = {68, 118, 76, 36.5f, 99, 14};
    std::string content;

    patient_init(&patient, 1001, "Sarah Johnson", 52, 72.5f, 1.66f);
    ASSERT_EQ(1, patient_add_reading(&patient, &first));
    ASSERT_EQ(SESSION_EXPORT_RESULT_OK,
              session_export_write_snapshot(&patient, 1, &limits_,
                                            1, 0, temp_path_.c_str(), 0,
                                            nullptr, 0));

    patient_init(&patient, 1002, "David Okonkwo", 34, 85.0f, 1.80f);
    ASSERT_EQ(1, patient_add_reading(&patient, &second));
    ASSERT_EQ(SESSION_EXPORT_RESULT_OK,
              session_export_write_snapshot(&patient, 1, &limits_,
                                            0, 0, temp_path_.c_str(), 1,
                                            nullptr, 0));

    content = read_text_file(temp_path_);
    EXPECT_NE(std::string::npos, content.find("Name           : David Okonkwo"));
    EXPECT_EQ(std::string::npos, content.find("Sarah Johnson"));
    EXPECT_NE(std::string::npos, content.find("Reading Capacity: 1 / 10"));
}

TEST_F(SessionExportIntegrationTest, REQ_INT_EXP_003_ClearedSessionCannotExport)
{
    PatientRecord patient;
    VitalSigns first = {78, 122, 82, 36.7f, 98, 15};

    patient_init(&patient, 1001, "Sarah Johnson", 52, 72.5f, 1.66f);
    ASSERT_EQ(1, patient_add_reading(&patient, &first));
    ASSERT_EQ(SESSION_EXPORT_RESULT_OK,
              session_export_write_snapshot(&patient, 1, &limits_,
                                            1, 0, temp_path_.c_str(), 0,
                                            nullptr, 0));

    std::memset(&patient, 0, sizeof(patient));
    EXPECT_EQ(SESSION_EXPORT_RESULT_NO_PATIENT,
              session_export_write_snapshot(&patient, 0, &limits_,
                                            1, 0, temp_path_.c_str(), 1,
                                            nullptr, 0));
}
