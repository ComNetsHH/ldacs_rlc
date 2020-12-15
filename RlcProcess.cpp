//
// Created by Konrad Fuger on 03.12.20.
//

#include "RlcProcess.hpp"
#include "L2Header.hpp"
#include "L2Packet.hpp"
#include "InetPacketPayload.hpp"
#include <iostream>

using namespace std;
using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace TUHH_INTAIRNET_RLC;

RlcProcess::RlcProcess(MacId id): dest(id) {

}

MacId RlcProcess::getMacId() {
    return dest;
}

pair<L2Header*, L2Packet::Payload*> RlcProcess::getEmptyData() {
    auto header = new L2HeaderUnicast(L2Header::FrameType::unicast);
    auto payload = new InetPacketPayload();
    header->is_pkt_end = true;
    header->is_pkt_start = true;
    header->icao_dest_id = dest;
    payload->size = 0;
    payload->offset = 0;
    return {header, payload};
}

pair<L2Header*, L2Packet::Payload*> RlcProcess::getData(unsigned int num_bits) {
    L3Packet *next_L3_packet = packets_to_send.front();
    unsigned int remaining_packet_size = next_L3_packet->size - next_L3_packet->offset;
    auto header = new L2HeaderUnicast(L2Header::FrameType::unicast);
    header->is_pkt_start = (next_L3_packet->offset == 0);
    header->icao_dest_id = dest;

    unsigned int remainig_payload_size = num_bits - header->getBits();

    if(remaining_packet_size > remainig_payload_size) {
        auto payload = new InetPacketPayload();
        payload->size = remainig_payload_size;
        payload->original = next_L3_packet->original;
        payload->offset = next_L3_packet->offset;
        next_L3_packet->offset += remainig_payload_size;
        return {header, payload};
    }
    auto payload = new InetPacketPayload();
    payload->size = remaining_packet_size;
    payload->original = next_L3_packet->original;
    payload->offset = next_L3_packet->offset;
    header->is_pkt_end = true;
    packets_to_send.pop_front();
    return {header, payload};

}

void RlcProcess::receiveFromUpper(L3Packet* data, PacketPriority priority) {
    packets_to_send.push_back(data);
}
void RlcProcess::receiveInjectionFromLower(L2Packet* packet, PacketPriority priority) {
    injected_packets.push_back(packet);
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
            L2HeaderUnicast *unicast_header= dynamic_cast<L2HeaderUnicast*>(header);
            if(unicast_header->is_pkt_start && firstStartIndex < 0) {
                firstStartIndex = idx;
            }
            if(unicast_header->is_pkt_end && firstEndIndex < 0) {
                firstEndIndex = idx;
            }
        }else if(header->frame_type == L2Header::FrameType::broadcast) {
            L2HeaderBroadcast *broadcast_header= dynamic_cast<L2HeaderBroadcast*>(header);
            if(broadcast_header->is_pkt_start && firstStartIndex < 0) {
                firstStartIndex = idx;
            }
            if(broadcast_header->is_pkt_end && firstEndIndex < 0) {
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
    }

    packets_received.erase(packets_received.begin() + firstStartIndex, packets_received.begin() + firstEndIndex+1);
    return pkt;
}