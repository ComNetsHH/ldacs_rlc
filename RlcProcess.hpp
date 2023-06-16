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

#ifndef TUHH_INTAIRNET_RLC_RLCPROCESS_HPP
#define TUHH_INTAIRNET_RLC_RLCPROCESS_HPP

#include <deque>
#include <utility>
#include <IRlc.hpp>
#include "IOmnetPluggable.hpp"
#include "L2Packet.hpp"
#include "L3Packet.hpp"


using namespace std;
using namespace TUHH_INTAIRNET_MCSOTDMA;

typedef pair<L2Header *, L2Packet::Payload *> PacketFragment;

namespace TUHH_INTAIRNET_RLC {

class RlcProcess: public IOmnetPluggable {
protected:
    MacId dest = SYMBOLIC_ID_UNSET;
    deque<L3Packet *> packets_to_send;
    deque<L2Packet *> injected_packets;

    deque<PacketFragment> packets_received;

    /** maximum size of packet, if set all packets are at most this size **/
    int max_packet_size = -1;

    /** indicate whether this is a brodacast process **/
    bool isBroadcast = false;
public:
    RlcProcess(MacId id);
    RlcProcess(MacId id, int max_packet_size);
    MacId getMacId();
    void receiveFromUpper(L3Packet* data, PacketPriority priority = PRIORITY_DEFAULT);
    void receiveFromLower(PacketFragment fragment);
    void receiveInjectionFromLower(L2Packet* packet, PacketPriority priority = PRIORITY_LINK_MANAGEMENT);
    L2Packet* getInjectedPacket();
    pair<L2Header*, L2Packet::Payload*> getData(unsigned int num_bits);
    pair<L2Header*, L2Packet::Payload*> getBroadcastData(unsigned int num_bits);
    pair<L2Header*, L2Packet::Payload*> getEmptyData();
    int getQueuedBits();
    int getQueuedPackets();
    int getNumInjectedPackets();
    int getNumReassembly();
    bool hasDataToSend();
    L3Packet* getReassembledPacket();

    ~RlcProcess();
};

}
#endif //TUHH_INTAIRNET_RLC_RLCPROCESS_HPP
