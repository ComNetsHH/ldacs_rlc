//
// Created by Konrad Fuger on 03.12.20.
//

#include "Rlc.hpp"
#include "IArq.hpp"
#include <utility>
#include <string>

using namespace std;
using namespace TUHH_INTAIRNET_RLC;

void Rlc::receiveFromLower(L2Packet* packet) {
    emit("Rlc:packet_received_from_lower(bits)", (double) packet->getBits());
    MacId src = packet->getOrigin();
    auto process = getProcess(src);
    if(process == nullptr) {
        process = new RlcProcess(src, max_packet_size);
        if(debugCallback) {
            process->registerDebugMessageCallback(debugCallback);
        }
        processes.insert(make_pair(src, process));
    }
    auto headers = packet->getHeaders();
    auto payloads = packet->getPayloads();

    for(int i = 0; i< headers.size(); i++) {
        if(headers[i] != nullptr && payloads[i] != nullptr) {
            PacketFragment frag = make_pair(headers[i], payloads[i]);
            process->receiveFromLower(frag);
        }
    }

    L3Packet * pkt = process->getReassembledPacket();
    auto nwLayer = getUpperLayer();
    while (nwLayer && pkt != nullptr) {
        debug("Rlc::passingToNWLayer" + to_string(pkt->size));
        if(pkt->size > 0) {
            nwLayer->receiveFromLower(pkt);
        }
        emit("Rlc:packet_passed_up(bits)", (double) pkt->size);
        pkt = process->getReassembledPacket();
    }
}

void Rlc::receiveFromUpper(L3Packet *data, MacId dest, PacketPriority priority) {
    emit("Rlc:packet_received_from_upper(bits)", (double) data->size);
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        if(debugCallback) {
            process->registerDebugMessageCallback(debugCallback);
        }
        processes.insert(make_pair(dest, process));
    }

    process->receiveFromUpper(data, priority);

    IArq *arq = getLowerLayer();
    if(arq) {
        arq->notifyOutgoing(data->size, dest);
    }
}

RlcProcess* Rlc::getProcess(MacId mac_id) const {
    auto result = processes.find(mac_id);

    if(result == processes.end()) {
        return nullptr;
    }
    return  result->second;
}

void Rlc::receiveInjectionFromLower(L2Packet *packet, PacketPriority priority) {
    emit("Rlc:packet_injected_from_lower(bits)", (double) packet->getBits());
    MacId dest = packet->getDestination();
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        processes.insert(make_pair(dest, process));
    }

    process->receiveInjectionFromLower(packet, priority);

    IArq *arq = getLowerLayer();
    if(arq) {
        arq->notifyOutgoing(packet->getBits(), dest);
    }
}

L2Packet * Rlc::requestSegment(unsigned int num_bits, const MacId &mac_id) {
    emit("Rlc:packet_requested_from_lower(bits)", (double) num_bits);
    auto process = getProcess(mac_id);

    if(process == nullptr) {
        L2Packet* empty = new L2Packet();
        L2HeaderBase* base_header = new L2HeaderBase(mac_id, 0, 0, 0, 0);
        empty->addMessage(base_header, nullptr);
        return empty;
    }

    L2Packet* packet = process->getInjectedPacket();
    if(packet == nullptr) {
        packet = new L2Packet();
        L2HeaderBase* base_header = new L2HeaderBase(mac_id, 0, 0, 0, 0);
        packet->addMessage(base_header, nullptr);
    }

    bool has_more_data = process->hasDataToSend();
    int counter = 0;

    while(has_more_data && counter < 100) {
        pair<L2Header*, L2Packet::Payload*> data = process->getData(num_bits - packet->getBits());
        packet->addMessage(data.first, data.second);

        has_more_data = process->hasDataToSend();
        if(packet->getBits() >= num_bits) {
            has_more_data = false;
        }

        // TODO: add unicast data to broadcast if there is space
        counter++;
        emit("Rlc:packet_sent_down(bits)", (double) packet->getBits());
    }

    if(packet->getHeaders().size() <= 1) {
        auto data = process->getEmptyData();
        packet->addMessage(data.first, data.second);
    }
    return packet;
}

bool Rlc::isThereMoreData(const MacId& mac_id) const{
    auto process = getProcess(mac_id);
    if(!process) {
        return false;
    }
    return process->hasDataToSend();
}

void Rlc::onEvent(double time) {

}


Rlc::Rlc(int min_packet_size) {
    this->max_packet_size = min_packet_size;
}



