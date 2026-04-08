/**
 * @file test_localization.cpp
 * @brief Unit tests for multi-language localization system
 *
 * Tests:
 * - Language enumeration (4 languages)
 * - String ID validation
 * - Language switching
 * - String retrieval for all languages
 * - Invalid language/string handling
 *
 * @req SWR-GUI-012
 */

#include "gtest/gtest.h"
extern "C" {
#include "localization.h"
}

/* ===================================================================
 * Test Fixture
 * =================================================================== */

class LocalizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Reset to English before each test */
        localization_set_language(LANG_ENGLISH);
    }
};

/* ===================================================================
 * Basic Tests
 * =================================================================== */

TEST_F(LocalizationTest, DefaultLanguageIsEnglish) {
    EXPECT_EQ(localization_get_language(), LANG_ENGLISH);
}

TEST_F(LocalizationTest, SetLanguageEnglish) {
    localization_set_language(LANG_ENGLISH);
    EXPECT_EQ(localization_get_language(), LANG_ENGLISH);
}

TEST_F(LocalizationTest, SetLanguageSpanish) {
    localization_set_language(LANG_SPANISH);
    EXPECT_EQ(localization_get_language(), LANG_SPANISH);
}

TEST_F(LocalizationTest, SetLanguageFrench) {
    localization_set_language(LANG_FRENCH);
    EXPECT_EQ(localization_get_language(), LANG_FRENCH);
}

TEST_F(LocalizationTest, SetLanguageGerman) {
    localization_set_language(LANG_GERMAN);
    EXPECT_EQ(localization_get_language(), LANG_GERMAN);
}

/* ===================================================================
 * String Retrieval Tests
 * =================================================================== */

TEST_F(LocalizationTest, AppTitleEnglish) {
    localization_set_language(LANG_ENGLISH);
    const char* str = localization_get_string(STR_APP_TITLE);
    EXPECT_STREQ(str, "Patient Vital Signs Monitor");
}

TEST_F(LocalizationTest, AppTitleSpanish) {
    localization_set_language(LANG_SPANISH);
    const char* str = localization_get_string(STR_APP_TITLE);
    EXPECT_STREQ(str, "Monitor de Signos Vitales del Paciente");
}

TEST_F(LocalizationTest, AppTitleFrench) {
    localization_set_language(LANG_FRENCH);
    const char* str = localization_get_string(STR_APP_TITLE);
    EXPECT_STREQ(str, "Moniteur de Signes Vitaux du Patient");
}

TEST_F(LocalizationTest, AppTitleGerman) {
    localization_set_language(LANG_GERMAN);
    const char* str = localization_get_string(STR_APP_TITLE);
    EXPECT_STREQ(str, "Patient Vital Signs Monitor");
}

/* ===================================================================
 * Language Name Tests
 * =================================================================== */

TEST_F(LocalizationTest, LanguageNameEnglish) {
    const char* name = localization_get_language_name(LANG_ENGLISH);
    EXPECT_STREQ(name, "English");
}

TEST_F(LocalizationTest, LanguageNameSpanish) {
    const char* name = localization_get_language_name(LANG_SPANISH);
    EXPECT_STREQ(name, "Español");
}

TEST_F(LocalizationTest, LanguageNameFrench) {
    const char* name = localization_get_language_name(LANG_FRENCH);
    EXPECT_STREQ(name, "Français");
}

TEST_F(LocalizationTest, LanguageNameGerman) {
    const char* name = localization_get_language_name(LANG_GERMAN);
    EXPECT_STREQ(name, "Deutsch");
}

/* ===================================================================
 * Switching Language Tests
 * =================================================================== */

TEST_F(LocalizationTest, SwitchBetweenLanguages) {
    /* English */
    localization_set_language(LANG_ENGLISH);
    EXPECT_STREQ(localization_get_string(STR_SIGN_IN), "Sign In");

    /* Spanish */
    localization_set_language(LANG_SPANISH);
    EXPECT_STREQ(localization_get_string(STR_SIGN_IN), "Iniciar Sesión");

    /* Back to English */
    localization_set_language(LANG_ENGLISH);
    EXPECT_STREQ(localization_get_string(STR_SIGN_IN), "Sign In");
}

TEST_F(LocalizationTest, AllLanguagesHaveAllStrings) {
    /* Test that all string IDs return non-empty strings for all languages */
    for (int lang = 0; lang < LANG_COUNT; lang++) {
        localization_set_language((Language)lang);

        for (int str_id = 0; str_id < STR_COUNT; str_id++) {
            const char* str = localization_get_string((StringID)str_id);
            EXPECT_NE(str, nullptr) << "NULL string for lang=" << lang << " id=" << str_id;
            EXPECT_GT(strlen(str), 0) << "Empty string for lang=" << lang << " id=" << str_id;
            EXPECT_NE(str, nullptr);
        }
    }
}

/* ===================================================================
 * Error Handling Tests
 * =================================================================== */

TEST_F(LocalizationTest, InvalidStringIDReturnsFallback) {
    const char* str = localization_get_string((StringID)9999);
    EXPECT_STREQ(str, "???");
}

TEST_F(LocalizationTest, InvalidLanguageNameReturnsFallback) {
    const char* name = localization_get_language_name((Language)9999);
    EXPECT_STREQ(name, "Unknown");
}

/* ===================================================================
 * Specific String Tests (Sample)
 * =================================================================== */

TEST_F(LocalizationTest, SimulationModeMessage) {
    localization_set_language(LANG_ENGLISH);
    EXPECT_STREQ(localization_get_string(STR_IN_SIMULATION_MODE), "IN SIMULATION MODE");

    localization_set_language(LANG_SPANISH);
    EXPECT_STREQ(localization_get_string(STR_IN_SIMULATION_MODE), "EN MODO SIMULACIÓN");

    localization_set_language(LANG_FRENCH);
    EXPECT_STREQ(localization_get_string(STR_IN_SIMULATION_MODE), "EN MODE SIMULATION");

    localization_set_language(LANG_GERMAN);
    EXPECT_STREQ(localization_get_string(STR_IN_SIMULATION_MODE), "IM SIMULATIONSMODUS");
}

TEST_F(LocalizationTest, CriticalAlertMessage) {
    localization_set_language(LANG_ENGLISH);
    EXPECT_STREQ(localization_get_string(STR_STATUS_CRITICAL),
                 "!! CRITICAL — Immediate clinical action required !!");

    localization_set_language(LANG_SPANISH);
    EXPECT_STREQ(localization_get_string(STR_STATUS_CRITICAL),
                 "!! CRÍTICO — Se requiere acción clínica inmediata !!");
}

/* ===================================================================
 * Edge Case Tests
 * =================================================================== */

TEST_F(LocalizationTest, LanguagePersistenceAcrossStringSwitches) {
    localization_set_language(LANG_FRENCH);

    /* Get multiple strings */
    const char* str1 = localization_get_string(STR_APP_TITLE);
    const char* str2 = localization_get_string(STR_SIGN_IN);
    const char* str3 = localization_get_string(STR_PASSWORD);

    /* Language should still be French */
    EXPECT_EQ(localization_get_language(), LANG_FRENCH);

    /* All strings should be in French */
    EXPECT_STREQ(str1, "Moniteur de Signes Vitaux du Patient");
    EXPECT_STREQ(str2, "Se Connecter");
    EXPECT_STREQ(str3, "Mot de passe");
}
