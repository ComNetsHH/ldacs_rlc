//
// Created by Konrad Fuger on 03.12.20.
//

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include "RlcProcessTest.cpp"
#include "RlcTest.cpp"

using namespace std;

int main(int argc, const char* argv[]) {
    //CppUnit::TestResult result;
    //CppUnit::TestResultCollector collectedResults;
    //CppUnit::BriefTestProgressListener progress;
    CppUnit::TextUi::TestRunner runner;

    //result.addListener (&collectedResults);
    //result.addListener (&progress);

    runner.addTest(RlcTest::suite());
    runner.addTest(RlcProcessTest::suite());
    //runner.addTest(SelectiveRepeatArqProcessTest::suite());
    // runner.addTest(L2SegmentHeaderTest::suite());
    runner.run();
    return runner.result().wasSuccessful() ? 0 : 1;

}


