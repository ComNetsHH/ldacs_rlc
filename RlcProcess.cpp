//
// Created by Konrad Fuger on 03.12.20.
//

#include "RlcProcess.h"
#include "L2Header.hpp"
#include "L2Packet.hpp"
#include "InetPacketPayload.hpp"

using namespace std;
using namespace TUHH_INTAIRNET_MCSOTDMA;

pair<L2Header*, L2Packet::Payload*> RlcProcess::getData(unsigned int num_bits) {
    L3Packet * next_L3_packet = packets_to_send.front();
    int remaining_size = next_L3_packet->size - next_L3_packet->offset;
    auto header = new L2HeaderUnicast(L2Header::FrameType::unicast);

    if(remaining_size >= num_bits) {
        auto payload = new InetPacketPayload();
        payload->size = num_bits;
        payload->original = next_L3_packet->original;
        payload->offset = next_L3_packet->offset;
        next_L3_packet->offset += num_bits;
        return {header, payload};
    }
    auto payload = new InetPacketPayload();
    payload->size = remaining_size;
    payload->original = next_L3_packet->original;
    payload->offset = next_L3_packet->offset;
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
    return !packets_to_send.empty() && !injected_packets.empty();
}