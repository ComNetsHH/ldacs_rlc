// The L-Band Digital Aeronautical Communications System (LDACS) Radio Link Control (RLC) implements the RLC protocol for the LDACS Air-Air Medium Access Control simulator.
// Copyright (C) 2023  Sebastian Lindner, Konrad Fuger, Musab Ahmed Eltayeb Ahmed, Andreas Timm-Giel, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
        L2HeaderPP * unicast_header = new L2HeaderPP(MacId(10),false, SEQNO_UNSET, SEQNO_UNSET, 0);
        unicast_header->src_id = MacId(10);
        lower_layer_pkt->addMessage(unicast_header, nullptr);


        cout << lower_layer_pkt->getOrigin() << endl;
        rlc.receiveFromLower(lower_layer_pkt);

        process = rlc.getProcess(MacId(10));
        CPPUNIT_ASSERT(process != nullptr);
    }

    // void testInjectedPacketConcatenation() {
    //     MacId macId = MacId(SYMBOLIC_LINK_ID_BROADCAST);
    //     Rlc rlc(-1);
    //     L3Packet * pkt1 = new L3Packet();
    //     pkt1->size = 100;

    //     L3Packet * pkt2 = new L3Packet();
    //     pkt2->size = 100;

    //     L2Packet * injection = new L2Packet();
    //     L2HeaderBase *baseHeader = new L2HeaderBase(macId, 0, 0, 0, 0);
    //     L2HeaderBeacon *beaconHeader = new L2HeaderBeacon();
    //     InetPacketPayload *payload = new InetPacketPayload();
    //     payload->size = 100;
    //     injection->addMessage(baseHeader, nullptr);
    //     injection->addMessage(beaconHeader, payload);

    //     //auto process = rlc.getProcess(macId);
    //     rlc.receiveFromUpper(pkt1, macId);
    //     rlc.receiveFromUpper(pkt2, macId);
    //     rlc.receiveInjectionFromLower(injection);

    //     auto segment = rlc.requestSegment(1900, macId);

    //     //cout << segment->getBits();

    //     // Print packet structure
    //     // [ B,N | H,P | BC,P | BC,P ]
    //     //cout << segment->print();

    //     CPPUNIT_ASSERT(segment->getHeaders().size() == 3);
    // }

    void testCorrectPacketSize() {
        MacId macId = MacId(3);
        Rlc rlc(3);

        // Load RLC with sufficient L3Packets
        for(int i = 0; i < 120; i++) {
            L3Packet * pkt = new L3Packet();
            pkt->size = 232;
            rlc.receiveFromUpper(pkt, macId);
        }



        // Request segment of odd size
        auto segment = rlc.requestSegment(1568, macId);

        while (segment->getBits() > 1000) {
            segment = rlc.requestSegment(1568, macId);
            //cout << segment->getBits();
            //cout << segment->print();
            //cout << endl;
            CPPUNIT_ASSERT(segment->getBits() <= 1568);
        }
    }



CPPUNIT_TEST_SUITE(RlcTest);
        CPPUNIT_TEST(testProcessCreation);
        // CPPUNIT_TEST(testInjectedPacketConcatenation);
        CPPUNIT_TEST(testCorrectPacketSize);

    CPPUNIT_TEST_SUITE_END();
};