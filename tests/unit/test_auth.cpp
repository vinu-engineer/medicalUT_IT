/**
 * @file test_auth.cpp
 * @brief Unit tests for GUI authentication (SWR-GUI-001, SWR-GUI-002).
 *
 * Covers: valid credentials, wrong password, wrong username, empty inputs,
 * null inputs, and display name formatting.
 *
 * Test naming: TEST(Suite, REQ_GUI_NNN_Description)
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "gui_auth.h"
}

/* -----------------------------------------------------------------------
 * SWR-GUI-001 — auth_validate
 * ----------------------------------------------------------------------- */

TEST(AuthValidation, REQ_GUI_001_ValidCredentialsReturnOne)
{
    EXPECT_EQ(1, auth_validate("admin", "Monitor@2026"));
}

TEST(AuthValidation, REQ_GUI_001_WrongPasswordReturnsZero)
{
    EXPECT_EQ(0, auth_validate("admin", "wrongpassword"));
}

TEST(AuthValidation, REQ_GUI_001_WrongUsernameReturnsZero)
{
    EXPECT_EQ(0, auth_validate("nurse", "Monitor@2026"));
}

TEST(AuthValidation, REQ_GUI_001_BothWrongReturnsZero)
{
    EXPECT_EQ(0, auth_validate("hacker", "password123"));
}

TEST(AuthValidation, REQ_GUI_001_EmptyUsernameReturnsZero)
{
    EXPECT_EQ(0, auth_validate("", "Monitor@2026"));
}

TEST(AuthValidation, REQ_GUI_001_EmptyPasswordReturnsZero)
{
    EXPECT_EQ(0, auth_validate("admin", ""));
}

TEST(AuthValidation, REQ_GUI_001_BothEmptyReturnsZero)
{
    EXPECT_EQ(0, auth_validate("", ""));
}

TEST(AuthValidation, REQ_GUI_001_NullUsernameReturnsZero)
{
    EXPECT_EQ(0, auth_validate(nullptr, "Monitor@2026"));
}

TEST(AuthValidation, REQ_GUI_001_NullPasswordReturnsZero)
{
    EXPECT_EQ(0, auth_validate("admin", nullptr));
}

TEST(AuthValidation, REQ_GUI_001_BothNullReturnsZero)
{
    EXPECT_EQ(0, auth_validate(nullptr, nullptr));
}

/* -----------------------------------------------------------------------
 * SWR-GUI-002 — auth_display_name
 * ----------------------------------------------------------------------- */

TEST(AuthDisplayName, REQ_GUI_002_KnownUsernameReturnsDisplayName)
{
    char buf[64] = {0};
    auth_display_name("admin", buf, (int)sizeof(buf));
    EXPECT_STRNE("", buf);            /* not empty */
    EXPECT_EQ(0, buf[sizeof(buf)-1]); /* null-terminated */
}

TEST(AuthDisplayName, REQ_GUI_002_UnknownUsernameReturnsEmpty)
{
    char buf[64];
    memset(buf, 0xFF, sizeof(buf)); /* poison */
    auth_display_name("unknown_user", buf, (int)sizeof(buf));
    EXPECT_EQ('\0', buf[0]);
}

TEST(AuthDisplayName, REQ_GUI_002_NullUsernameHandledSafely)
{
    char buf[64] = {0};
    auth_display_name(nullptr, buf, (int)sizeof(buf));
    EXPECT_EQ('\0', buf[0]);
}

TEST(AuthDisplayName, REQ_GUI_002_NullOutputHandledSafely)
{
    /* Must not crash */
    auth_display_name("admin", nullptr, 0);
}

TEST(AuthDisplayName, REQ_GUI_002_ZeroLengthBufferHandledSafely)
{
    char buf[4] = {'X', 'X', 'X', 'X'};
    /* Should not write anything or crash */
    auth_display_name("admin", buf, 0);
    /* Buffer should be untouched (or written empty) — just must not crash */
}
