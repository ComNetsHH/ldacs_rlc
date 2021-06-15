//
// Created by Konrad Fuger on 03.12.20.
//


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "../RlcProcess.hpp"
#include "InetPacketPayload.hpp"

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
        RlcProcess process(dest, -1);
        L3Packet *pkt = new L3Packet();
        pkt->size = 1000;
        process.receiveFromUpper(pkt);

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), true);

        auto data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(500,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(500,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(231,(int)(data.first->getBits() + data.second->getBits()) );

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), false);
    }

    void handleL3PacketsWithMaxPacketSize() {
        MacId dest(12);
        RlcProcess process(dest, 100);
        L3Packet *pkt = new L3Packet();
        pkt->size = 1000;
        process.receiveFromUpper(pkt);

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), true);

        auto data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(165,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(165,(int)(data.first->getBits() + data.second->getBits()) );
        data = process.getData(500);
        CPPUNIT_ASSERT_EQUAL(165,(int)(data.first->getBits() + data.second->getBits()) );

        CPPUNIT_ASSERT_EQUAL(process.hasDataToSend(), true);
    }

    void testInjection() {
        MacId dest(12);
        RlcProcess process(dest);
        L2Packet *pkt = new L2Packet();

        CPPUNIT_ASSERT(process.getInjectedPacket() == nullptr);
        process.receiveInjectionFromLower(pkt);
        CPPUNIT_ASSERT_EQUAL(process.getInjectedPacket()->getBits(), pkt->getBits());

    }

    void testInjectionDelete() {
        MacId dest(12);
        RlcProcess process(dest);
        L2Packet *pkt = new L2Packet();

        CPPUNIT_ASSERT(process.getInjectedPacket() == nullptr);
        process.receiveInjectionFromLower(pkt);
        auto bits = pkt->getBits();
        delete pkt;
        CPPUNIT_ASSERT_EQUAL(process.getInjectedPacket()->getBits(), bits);
    }

    void testReassembly() {
        MacId dest(12);
        RlcProcess process(dest, 100);

        /** Fragment 1 **/
        L2HeaderUnicast *header1 = new L2HeaderUnicast(MacId(10), false, SEQNO_UNSET, SEQNO_UNSET, 0);
        header1->is_pkt_start = true;
        InetPacketPayload *payload1 = new InetPacketPayload();
        payload1->size = 100;
        payload1->original = nullptr;
        PacketFragment frag1 = make_pair(header1, payload1);

        /** Fragment 2 **/
        L2HeaderUnicast *header2 = new L2HeaderUnicast(MacId(10), false, SEQNO_UNSET, SEQNO_UNSET, 0);
        header2->is_pkt_end = true;
        InetPacketPayload *payload2 = new InetPacketPayload();
        payload2->size = 10;
        payload2->original = nullptr;
        PacketFragment frag2 = make_pair(header2, payload2);

        /** Fragment 3 **/
        L2HeaderUnicast *header3 = new L2HeaderUnicast(MacId(10), false, SEQNO_UNSET, SEQNO_UNSET, 0);
        header3->is_pkt_end = true;
        header3->is_pkt_start = true;
        InetPacketPayload *payload3 = new InetPacketPayload();
        payload3->size = 10;
        payload3->original = nullptr;
        PacketFragment frag3 = make_pair(header3, payload3);

        process.receiveFromLower(frag1);

        L3Packet *pkt = process.getReassembledPacket();
        CPPUNIT_ASSERT(pkt == nullptr);

        process.receiveFromLower(frag2);
        pkt = process.getReassembledPacket();
        CPPUNIT_ASSERT(pkt != nullptr);
        CPPUNIT_ASSERT_EQUAL(110, pkt->size);

        process.receiveFromLower(frag3);
        pkt = process.getReassembledPacket();
        CPPUNIT_ASSERT(pkt != nullptr);
        CPPUNIT_ASSERT_EQUAL(10, pkt->size);
    }


CPPUNIT_TEST_SUITE(RlcProcessTest);
        CPPUNIT_TEST(macId);
        CPPUNIT_TEST(handleL3Packets);
        CPPUNIT_TEST(testInjection);
        CPPUNIT_TEST(testReassembly);
        CPPUNIT_TEST(testInjectionDelete);
    CPPUNIT_TEST_SUITE_END();
};