// Example test for core recorder
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "core/recorder.h"

class RecorderTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RecorderTest);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testPause);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
    
    void testStartStop();
    void testPause();

private:
    Recordify::Core::Recorder* recorder_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RecorderTest);
