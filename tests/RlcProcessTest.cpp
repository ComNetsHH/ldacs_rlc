//
// Created by Konrad Fuger on 03.12.20.
//


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "../RlcProcess.hpp"

using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace TUHH_INTAIRNET_RLC;

class RlcProcessTest : public CppUnit::TestFixture {

public:
    void macId() {
        MacId dest(12);
        RlcProcess process(dest);
        CPPUNIT_ASSERT_EQUAL(dest, process.getMacId());
    }

    void handleL3Packets() {
        MacId dest(12);
        RlcProcess process(dest);
        L3Packet *pkt = new L3Packet();
        pkt->size = 1000;
        process.receiveFromUpper(pkt);

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), true);

        auto data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(500,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(500,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(195,(int)(data.first->getBits() + data.second->getBits()) );

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), false);
    }

    void testInjection() {
        MacId dest(12);
        RlcProcess process(dest);
        L2Packet *pkt = new L2Packet();

        CPPUNIT_ASSERT(process.getInjectedPacket() == nullptr);
        process.receiveInjectionFromLower(pkt);
        CPPUNIT_ASSERT_EQUAL(process.getInjectedPacket(), pkt);

    }


CPPUNIT_TEST_SUITE(RlcProcessTest);
        CPPUNIT_TEST(macId);
        CPPUNIT_TEST(handleL3Packets);
        CPPUNIT_TEST(testInjection);
    CPPUNIT_TEST_SUITE_END();
};