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

#ifndef TUHH_INTAIRNET_RLC_RLC_HPP
#define TUHH_INTAIRNET_RLC_RLC_HPP

#include <map>
#include "IRlc.hpp"
#include "IOmnetPluggable.hpp"
#include "RlcProcess.hpp"

using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace std;

namespace TUHH_INTAIRNET_RLC {

class Rlc : public IRlc, public IOmnetPluggable {
protected:

    /** map to hold all RlcProcesses **/
    map<MacId, RlcProcess*> processes;

    /** maximum size of packet, if set all packets are at most this size **/
    int max_packet_size = -1;

    /** utilities for statistics **/
    int getTotalBitsToSend();
    int getTotalPacketsToSend();

public:
    Rlc(int min_packet_size);
    RlcProcess* getProcess(MacId mac_id) const;
    void receiveFromUpper(L3Packet* data, MacId dest, PacketPriority priority = PRIORITY_DEFAULT);
    void receiveInjectionFromLower(L2Packet* packet, PacketPriority priority = PRIORITY_LINK_MANAGEMENT);
    L2Packet* requestSegment(unsigned int num_bits, const MacId& mac_id);
    void receiveFromLower(L2Packet* packet);
    bool isThereMoreData(const MacId& mac_id) const;

    void onEvent(double time);
    unsigned int getQueuedDataSize(MacId dest) override;

    ~Rlc();
};

}

#endif //TUHH_INTAIRNET_RLC_RLC_HPP
