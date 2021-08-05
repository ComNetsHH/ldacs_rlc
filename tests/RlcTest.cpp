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
        Rlc rlc(-1);
        L3Packet * pkt = new L3Packet();

        auto process = rlc.getProcess(MacId(12));
        CPPUNIT_ASSERT(process == nullptr);

        rlc.receiveFromUpper(pkt, MacId(12));
        process = rlc.getProcess(MacId(12));
        CPPUNIT_ASSERT(process != nullptr);


        L2Packet *lower_layer_pkt = new L2Packet();
        L2HeaderBase *base_header = new L2HeaderBase(MacId(10), 0, 0, 0, 0);
        lower_layer_pkt->addMessage(base_header, nullptr);
        L2HeaderUnicast * unicast_header = new L2HeaderUnicast(MacId(10),false, SEQNO_UNSET, SEQNO_UNSET, 0);
        lower_layer_pkt->addMessage(unicast_header, nullptr);


        cout << lower_layer_pkt->getOrigin() << endl;
        rlc.receiveFromLower(lower_layer_pkt);

        process = rlc.getProcess(MacId(10));
        CPPUNIT_ASSERT(process != nullptr);
    }

    void testInjectedPacketConcatenation() {
        MacId macId = MacId(SYMBOLIC_LINK_ID_BROADCAST);
        Rlc rlc(-1);
        L3Packet * pkt1 = new L3Packet();
        pkt1->size = 100;

        L3Packet * pkt2 = new L3Packet();
        pkt2->size = 100;

        L2Packet * injection = new L2Packet();
        L2HeaderBase *baseHeader = new L2HeaderBase(macId, 0, 0, 0, 0);
        L2HeaderBeacon *beaconHeader = new L2HeaderBeacon();
        InetPacketPayload *payload = new InetPacketPayload();
        payload->size = 100;
        injection->addMessage(baseHeader, nullptr);
        injection->addMessage(beaconHeader, payload);

        //auto process = rlc.getProcess(macId);
        rlc.receiveFromUpper(pkt1, macId);
        rlc.receiveFromUpper(pkt2, macId);
        rlc.receiveInjectionFromLower(injection);

        auto segment = rlc.requestSegment(10000, macId);

        // Print packet structure
        // [ B,N | H,P | BC,P | BC,P ]
        cout << segment->print();

        CPPUNIT_ASSERT(segment->getHeaders().size() == 4);
    }


CPPUNIT_TEST_SUITE(RlcTest);
        CPPUNIT_TEST(testProcessCreation);
        CPPUNIT_TEST(testInjectedPacketConcatenation);
    CPPUNIT_TEST_SUITE_END();
};