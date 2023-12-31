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

#include "RlcProcess.hpp"
#include "L2Header.hpp"
#include "L2Packet.hpp"
#include "InetPacketPayload.hpp"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace TUHH_INTAIRNET_RLC;

RlcProcess::RlcProcess(MacId id): dest(id) {
    this->isBroadcast = id == SYMBOLIC_LINK_ID_BROADCAST;
}

RlcProcess::RlcProcess(MacId id, int max_packet_size): dest(id) {
    this->max_packet_size = max_packet_size;
    this->isBroadcast = id == SYMBOLIC_LINK_ID_BROADCAST;
}

MacId RlcProcess::getMacId() {
    return dest;
}

pair<L2Header*, L2Packet::Payload*> RlcProcess::getEmptyData() {
    auto header = new L2HeaderPP();
    auto bcHeader = new L2HeaderSH();
    auto payload = new InetPacketPayload();
    header->is_pkt_end = true;
    header->is_pkt_start = true;
    header->dest_id = dest;
    payload->size = 0;
    payload->offset = 0;
    if(dest == SYMBOLIC_LINK_ID_BROADCAST) {
        return {bcHeader, payload};
    }
    return {header, payload};
}

int RlcProcess::getQueuedBits() {
    int total = 0;

    for(auto it = packets_to_send.begin(); it != packets_to_send.end(); it++) {
        total += (*it)->size;
    }
    for(auto it = injected_packets.begin(); it != injected_packets.end(); it++) {
        total += (*it)->getBits();
    }
    return total;
}

int RlcProcess::getQueuedPackets() {
    return packets_to_send.size();
}

int RlcProcess::getNumInjectedPackets() {
    return injected_packets.size();
}


int RlcProcess::getNumReassembly() {
    return packets_received.size();
}

pair<L2Header*, L2Packet::Payload*> RlcProcess::getBroadcastData(unsigned int num_bits) {
    L3Packet *next_L3_packet = packets_to_send.front();
    unsigned int remaining_packet_size = next_L3_packet->size - next_L3_packet->offset;
    auto header = new L2HeaderSH();
    header->is_pkt_start = (next_L3_packet->offset == 0);    

    unsigned int remainig_payload_size = num_bits - header->getBits();
    unsigned int size = 0;

    if(remaining_packet_size > remainig_payload_size) {
        size = (remainig_payload_size > max_packet_size) ? max_packet_size : remainig_payload_size;
        auto payload = new InetPacketPayload();
        payload->size = size;
        payload->original = header->is_pkt_start ? next_L3_packet->original : nullptr;
        payload->offset = next_L3_packet->offset;
        next_L3_packet->offset += size;
        return {header, payload};
    }
    size = (remaining_packet_size > max_packet_size) ? max_packet_size : remaining_packet_size;
    bool is_full_pkt = size == remaining_packet_size;
    auto payload = new InetPacketPayload();
    payload->size = size;
    payload->original =header->is_pkt_start ? next_L3_packet->original : nullptr;
    payload->offset = next_L3_packet->offset;
    header->is_pkt_end = is_full_pkt;
    packets_to_send.pop_front();
    return {header, payload};
}

pair<L2Header*, L2Packet::Payload*> RlcProcess::getData(unsigned int num_bits) {
    if(dest == SYMBOLIC_LINK_ID_BROADCAST) {
        return getBroadcastData(num_bits);
    }
    L3Packet *next_L3_packet = packets_to_send.front();
    unsigned int remaining_packet_size = next_L3_packet->size - next_L3_packet->offset;
    auto header = new L2HeaderPP();
    header->is_pkt_start = (next_L3_packet->offset == 0);
    header->dest_id = dest;

    unsigned int remainig_payload_size = num_bits - header->getBits();
    unsigned int size = 0;

    if(remaining_packet_size > remainig_payload_size) {
        size = (remainig_payload_size > max_packet_size) ? max_packet_size : remainig_payload_size;
        auto payload = new InetPacketPayload();
        payload->size = size;
        payload->original = header->is_pkt_start ? next_L3_packet->original : nullptr;
        payload->offset = next_L3_packet->offset;
        next_L3_packet->offset += size;
        return {header, payload};
    }
    size = (remaining_packet_size > max_packet_size) ? max_packet_size : remaining_packet_size;
    bool is_full_pkt = size == remaining_packet_size;
    auto payload = new InetPacketPayload();
    payload->size = size;
    payload->original = header->is_pkt_start ? next_L3_packet->original : nullptr;
    payload->offset = next_L3_packet->offset;
    header->is_pkt_end = is_full_pkt;
    packets_to_send.pop_front();
    return {header, payload};

}

void RlcProcess::receiveFromUpper(L3Packet* data, PacketPriority priority) {
    packets_to_send.push_back(data);
}
void RlcProcess::receiveInjectionFromLower(L2Packet* packet, PacketPriority priority) {
    auto p = new L2Packet(*packet);
    injected_packets.push_back(p);
}
L2Packet* RlcProcess::getInjectedPacket() {
    if(injected_packets.empty()) {
        return nullptr;
    }
    L2Packet* pkt = injected_packets.front();
    injected_packets.pop_front();
    return pkt;
}

bool RlcProcess::hasDataToSend() {
    return !packets_to_send.empty() || !injected_packets.empty();
}

void RlcProcess::receiveFromLower(PacketFragment fragment) {
    packets_received.push_back(fragment);
}

L3Packet* RlcProcess::getReassembledPacket() {
    // Fragments are delivered in order. Therefore we only need to find where a packet starts and where it ends
    // Later we delete all frags involved
    int firstStartIndex = -1;
    int firstEndIndex = -1;
    int idx = 0;
    for(auto it = packets_received.begin(); it != packets_received.end(); it++) {
        auto header = it->first;
        if(header->frame_type == L2Header::FrameType::unicast) {
            L2HeaderPP *unicast_header= dynamic_cast<L2HeaderPP*>(header);
            if(unicast_header->is_pkt_start && firstStartIndex < 0) {
                firstStartIndex = idx;
            }
            if(unicast_header->is_pkt_end && firstEndIndex < 0 && firstStartIndex != -1) {
                firstEndIndex = idx;
            }
        }else if(header->frame_type == L2Header::FrameType::broadcast) {
            L2HeaderSH *broadcast_header= dynamic_cast<L2HeaderSH*>(header);
            if(broadcast_header->is_pkt_start && firstStartIndex < 0) {
                firstStartIndex = idx;
            }
            if(broadcast_header->is_pkt_end && firstEndIndex < 0 && firstStartIndex != -1) {
                firstEndIndex = idx;
            }
        }
        idx++;
    }

    if(firstEndIndex == -1 || firstStartIndex == -1) {
        return nullptr;
    }

    L2Packet::Payload * first_segment_payload = nullptr;
    int size = 0;
    for (auto it = packets_received.begin() + firstStartIndex; it != packets_received.begin() + firstEndIndex +1; it++) {
        auto payload = it->second;
        if(payload) {
            size += payload->getBits();
            if(it == packets_received.begin() + firstStartIndex ) {
                first_segment_payload = payload;
            }
        }
    }

    L3Packet *pkt = new L3Packet();
    pkt->offset = 0;
    pkt->size = size;

    InetPacketPayload * payload = dynamic_cast<InetPacketPayload *>(first_segment_payload);

    if(payload != nullptr) {
        pkt->original = payload->original;

        if(payload->original == nullptr && pkt->size > 0) {
            cout << "RlcProcess(" << (int)(dest.getId()) << "): Received Packet without original, Size: " << pkt->size << endl;
        }
    }

    for (auto it = packets_received.begin() + firstStartIndex; it != packets_received.begin() + firstEndIndex+1; it++) {
        delete it->first;
        delete it->second;
    }
    packets_received.erase(packets_received.begin() + firstStartIndex, packets_received.begin() + firstEndIndex+1);
    return pkt;
}

RlcProcess::~RlcProcess() {
    for(int i= 0; i< packets_to_send.size(); i++) {
        auto original = packets_to_send[i]->original;
        auto offset = packets_to_send[i]->offset;

        delete packets_to_send[i];
    }

    packets_received.erase(packets_received.begin(), packets_received.end());
}