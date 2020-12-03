//
// Created by Konrad Fuger on 03.12.20.
//

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include "RlcTest.cpp"

using namespace std;

int main(int argc, const char* argv[]) {
    CppUnit::TestResult result;
    CppUnit::TestResultCollector collectedResults;
    CppUnit::BriefTestProgressListener progress;
    CppUnit::TextUi::TestRunner runner;

    result.addListener (&collectedResults);
    result.addListener (&progress);

    runner.addTest(RlcTest::suite());
    //runner.addTest(SelectiveRepeatArqProcessTest::suite());
    // runner.addTest(L2SegmentHeaderTest::suite());
    runner.run(result);
    return collectedResults.wasSuccessful() ? 0 : 1;

}


