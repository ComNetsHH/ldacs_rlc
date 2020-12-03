//
// Created by Konrad Fuger on 03.12.20.
//

#include "Rlc.h"

using namespace TUHH_INTAIRNET_RLC;

void Rlc::receiveFromUpper(L3Packet *data, MacId dest, PacketPriority priority) {
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess();
        processes.insert(make_pair(dest, process));
    }
}

RlcProcess* Rlc::getProcess(MacId mac_id) {
    auto result = processes.find(mac_id);

    if(result == processes.end()) {
        return nullptr;
    }
    return  result->second;
}

void Rlc::receiveInjectionomLower(L2Packet *packet, PacketPriority priority) {

}

L2Packet * Rlc::requestSegment(unsigned int num_bits, const MacId &mac_id) {

}



