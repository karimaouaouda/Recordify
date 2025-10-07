#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "screen_handler/screen_capture.h"

class ScreenCaptureTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ScreenCaptureTest);
    CPPUNIT_TEST(testInitialize);
    CPPUNIT_TEST(testStartCapture);
    CPPUNIT_TEST(testTakeScreenshot);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        screenCapture = new recordify::screen_handler::ScreenCapture();
    }

    void tearDown() {
        delete screenCapture;
    }

    void testInitialize() {
        CPPUNIT_ASSERT(screenCapture->initialize());
    }

    void testStartCapture() {
        screenCapture->initialize();
        CPPUNIT_ASSERT(screenCapture->startCapture());
    }

    void testTakeScreenshot() {
        CPPUNIT_ASSERT(screenCapture->takeScreenshot("test.png"));
    }

private:
    recordify::screen_handler::ScreenCapture* screenCapture;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ScreenCaptureTest);
