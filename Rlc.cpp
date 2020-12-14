//
// Created by Konrad Fuger on 03.12.20.
//

#include "Rlc.hpp"
#include <utility>

using namespace TUHH_INTAIRNET_RLC;

void Rlc::receiveFromUpper(L3Packet *data, MacId dest, PacketPriority priority) {
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        processes.insert(make_pair(dest, process));
    }

    process->receiveFromUpper(data, priority);
}

RlcProcess* Rlc::getProcess(MacId mac_id) {
    auto result = processes.find(mac_id);

    if(result == processes.end()) {
        return nullptr;
    }
    return  result->second;
}

void Rlc::receiveInjectionFromLower(L2Packet *packet, PacketPriority priority) {
    MacId dest = packet->getDestination();
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        processes.insert(make_pair(dest, process));
    }

    process->receiveInjectionFromLower(packet, priority);
}

L2Packet * Rlc::requestSegment(unsigned int num_bits, const MacId &mac_id) {
    auto process = getProcess(mac_id);

    L2Packet* packet = process->getInjectedPacket();
    if(packet == nullptr) {
        packet = new L2Packet();
        L2HeaderBase* base_header = new L2HeaderBase(mac_id, 0, 0, 0);
        packet->addPayload(base_header, nullptr);
    }

    bool has_more_data = true;

    while(has_more_data) {
        pair<L2Header*, L2Packet::Payload*>data = process->getData(num_bits);
        packet->addPayload(data.first, data.second);

        has_more_data = process->hasDataToSend();

        // TODO: add unicast data to broadcast if there is space
    }

    return packet;
}



