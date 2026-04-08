/**
 * @file test_app_config.cpp
 * @brief Unit tests for application configuration persistence
 *
 * Tests:
 * - Configuration file loading
 * - Configuration file saving
 * - Language preference persistence
 * - Simulator mode persistence
 * - Default values
 * - Path overrides for testing
 *
 * @req SWR-GUI-010, SWR-GUI-012
 */

#include "gtest/gtest.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "app_config.h"
#include "localization.h"
}

/* ===================================================================
 * Test Fixture with Temporary File Management
 * =================================================================== */

class AppConfigTest : public ::testing::Test {
protected:
    static char temp_file_path[256];

    static void SetUpTestSuite() {
        /* Create a temporary configuration file for testing */
        #ifdef _WIN32
            strcpy(temp_file_path, "test_monitor.cfg");
        #else
            strcpy(temp_file_path, "/tmp/test_monitor.cfg");
        #endif
    }

    void SetUp() override {
        /* Remove any existing test config file */
        remove(temp_file_path);
        /* Set the config path to our test file */
        app_config_set_path(temp_file_path);
    }

    void TearDown() override {
        /* Clean up test file */
        remove(temp_file_path);
        /* Reset to default path */
        app_config_set_path(NULL);
    }

    static void TearDownTestSuite() {
        /* Final cleanup */
        remove(temp_file_path);
    }
};

char AppConfigTest::temp_file_path[256];

/* ===================================================================
 * Default Value Tests
 * =================================================================== */

TEST_F(AppConfigTest, LoadMissingFileReturnsDefaults) {
    int sim_enabled = -1;
    int language = -1;

    int result = app_config_load(&sim_enabled, &language);

    /* File doesn't exist, should return 0 (failure) but set defaults */
    EXPECT_EQ(result, 0);
    EXPECT_EQ(sim_enabled, 1);  /* Default: simulator ON */
    EXPECT_EQ(language, LANG_ENGLISH);  /* Default: English */
}

TEST_F(AppConfigTest, LoadWithNullPointerReturnsFailed) {
    int sim_enabled = 1;

    int result = app_config_load(NULL, &sim_enabled);
    EXPECT_EQ(result, 0);

    result = app_config_load(&sim_enabled, NULL);
    EXPECT_EQ(result, 0);

    result = app_config_load(NULL, NULL);
    EXPECT_EQ(result, 0);
}

/* ===================================================================
 * Save and Load Tests
 * =================================================================== */

TEST_F(AppConfigTest, SaveAndLoadSimulatorMode) {
    /* Save simulator ON */
    int save_result = app_config_save(1, LANG_ENGLISH);
    EXPECT_EQ(save_result, 1);

    /* Load and verify */
    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(sim_enabled, 1);
    EXPECT_EQ(language, LANG_ENGLISH);
}

TEST_F(AppConfigTest, SaveAndLoadHALMode) {
    /* Save simulator OFF */
    int save_result = app_config_save(0, LANG_ENGLISH);
    EXPECT_EQ(save_result, 1);

    /* Load and verify */
    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(sim_enabled, 0);
    EXPECT_EQ(language, LANG_ENGLISH);
}

/* ===================================================================
 * Language Persistence Tests
 * =================================================================== */

TEST_F(AppConfigTest, SaveAndLoadLanguageSpanish) {
    int save_result = app_config_save(1, LANG_SPANISH);
    EXPECT_EQ(save_result, 1);

    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(language, LANG_SPANISH);
}

TEST_F(AppConfigTest, SaveAndLoadLanguageFrench) {
    int save_result = app_config_save(1, LANG_FRENCH);
    EXPECT_EQ(save_result, 1);

    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(language, LANG_FRENCH);
}

TEST_F(AppConfigTest, SaveAndLoadLanguageGerman) {
    int save_result = app_config_save(1, LANG_GERMAN);
    EXPECT_EQ(save_result, 1);

    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(language, LANG_GERMAN);
}

/* ===================================================================
 * Combined Save and Load Tests
 * =================================================================== */

TEST_F(AppConfigTest, SaveAndLoadAllCombinations) {
    int sim_modes[] = {0, 1};
    int languages[] = {LANG_ENGLISH, LANG_SPANISH, LANG_FRENCH, LANG_GERMAN};

    for (int sim : sim_modes) {
        for (int lang : languages) {
            /* Save configuration */
            int save_result = app_config_save(sim, lang);
            EXPECT_EQ(save_result, 1);

            /* Load and verify */
            int loaded_sim = -1;
            int loaded_lang = -1;
            int load_result = app_config_load(&loaded_sim, &loaded_lang);

            EXPECT_EQ(load_result, 1);
            EXPECT_EQ(loaded_sim, sim) << "Mismatch for sim=" << sim << " lang=" << lang;
            EXPECT_EQ(loaded_lang, lang) << "Mismatch for sim=" << sim << " lang=" << lang;
        }
    }
}

/* ===================================================================
 * Bounds Checking Tests
 * =================================================================== */

TEST_F(AppConfigTest, InvalidLanguageCodeDefaultsToEnglish) {
    /* Manually write invalid language code to file */
    FILE* f = fopen(temp_file_path, "w");
    ASSERT_NE(f, nullptr);
    fprintf(f, "sim_enabled=1\nlanguage=999\n");
    fclose(f);

    /* Load and verify that invalid language is rejected */
    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);  /* File exists and was parsed */
    EXPECT_EQ(sim_enabled, 1);
    EXPECT_EQ(language, LANG_ENGLISH);  /* Invalid language → default to English */
}

/* ===================================================================
 * Partial Config Tests
 * =================================================================== */

TEST_F(AppConfigTest, LoadConfigWithOnlySimulatorMode) {
    /* Write only sim_enabled to file */
    FILE* f = fopen(temp_file_path, "w");
    ASSERT_NE(f, nullptr);
    fprintf(f, "sim_enabled=0\n");
    fclose(f);

    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);  /* File was parsed */
    EXPECT_EQ(sim_enabled, 0);
    EXPECT_EQ(language, LANG_ENGLISH);  /* Default language */
}

TEST_F(AppConfigTest, LoadConfigWithOnlyLanguage) {
    /* Write only language to file */
    FILE* f = fopen(temp_file_path, "w");
    ASSERT_NE(f, nullptr);
    fprintf(f, "language=2\n");  /* LANG_FRENCH */
    fclose(f);

    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);  /* File was parsed */
    EXPECT_EQ(sim_enabled, 1);  /* Default simulator ON */
    EXPECT_EQ(language, LANG_FRENCH);
}

/* ===================================================================
 * File Path Override Tests
 * =================================================================== */

TEST_F(AppConfigTest, CustomPathOverride) {
    char custom_path[256];
    #ifdef _WIN32
        strcpy(custom_path, "custom_monitor.cfg");
    #else
        strcpy(custom_path, "/tmp/custom_monitor.cfg");
    #endif

    /* Set custom path */
    app_config_set_path(custom_path);

    /* Save to custom path */
    int save_result = app_config_save(1, LANG_FRENCH);
    EXPECT_EQ(save_result, 1);

    /* Load from custom path */
    int sim_enabled = -1;
    int language = -1;
    int load_result = app_config_load(&sim_enabled, &language);

    EXPECT_EQ(load_result, 1);
    EXPECT_EQ(language, LANG_FRENCH);

    /* Clean up custom file */
    remove(custom_path);

    /* Reset path */
    app_config_set_path(NULL);
}

TEST_F(AppConfigTest, ResetPathToDefault) {
    char custom_path[256];
    #ifdef _WIN32
        strcpy(custom_path, "custom_monitor.cfg");
    #else
        strcpy(custom_path, "/tmp/custom_monitor.cfg");
    #endif

    /* Set custom path, then reset to NULL */
    app_config_set_path(custom_path);
    app_config_set_path(NULL);

    /* This should use the default path, not the custom one */
    int result = app_config_save(1, LANG_ENGLISH);
    /* We can't easily verify this uses the default path without checking file location,
     * but at least verify it doesn't crash */
    EXPECT_GE(result, 0);
}
