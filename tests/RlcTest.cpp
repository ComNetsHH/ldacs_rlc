//
// Created by Konrad Fuger on 03.12.20.
//


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include "../Rlc.hpp"

using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace TUHH_INTAIRNET_RLC;

class RlcTest : public CppUnit::TestFixture {

public:
    void testProcessCreation() {
        Rlc rlc;
        L3Packet * pkt = new L3Packet();

        auto process = rlc.getProcess(MacId(12));
        CPPUNIT_ASSERT(process == nullptr);

        rlc.receiveFromUpper(pkt, MacId(12));
        process = rlc.getProcess(MacId(12));
        CPPUNIT_ASSERT(process != nullptr);


        L2Packet *lower_layer_pkt = new L2Packet();
        L2HeaderBase *base_header = new L2HeaderBase(MacId(10), 0, 0, 0);
        lower_layer_pkt->addPayload(base_header, nullptr);
        L2HeaderUnicast * unicast_header = new L2HeaderUnicast(MacId(10),false,SEQNO_UNSET, SEQNO_UNSET, 0);
        lower_layer_pkt->addPayload(unicast_header, nullptr);


        cout << lower_layer_pkt->getOrigin() << endl;
        rlc.receiveFromLower(lower_layer_pkt);

        process = rlc.getProcess(MacId(10));
        CPPUNIT_ASSERT(process != nullptr);
    }


CPPUNIT_TEST_SUITE(RlcTest);
        CPPUNIT_TEST(testProcessCreation);
    CPPUNIT_TEST_SUITE_END();
};