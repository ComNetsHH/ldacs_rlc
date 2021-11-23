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
    emit("rlc_bits_received_from_lower", (double) packet->getBits());
    MacId src = packet->getOrigin();
    auto process = getProcess(src);
    if(process == nullptr) {
        process = new RlcProcess(src, max_packet_size);
        if(debugCallback) {
            process->registerDebugMessageCallback(debugCallback);
            process->registerEmitEventCallback(emitCallback);
        }
        processes.insert(make_pair(src, process));
    }

    auto headers = packet->getHeaders();
    auto payloads = packet->getPayloads();

    for(int i = 0; i< headers.size(); i++) {
        if(headers[i]->frame_type == L2Header::FrameType::unicast || headers[i]->frame_type == L2Header::FrameType::broadcast) {
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
            emit("rlc_packet_sent_up", (double) pkt->size);
        }
        pkt = process->getReassembledPacket();
    }
    emit("rlc_awaiting_reassembly", (double) process->getNumReassembly());
}

void Rlc::receiveFromUpper(L3Packet *data, MacId dest, PacketPriority priority) {
    emit("rlc_packet_received_from_upper", (double) data->size);
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        if(debugCallback) {
            process->registerDebugMessageCallback(debugCallback);
            process->registerEmitEventCallback(emitCallback);
        }
        processes.insert(make_pair(dest, process));
    }

    emit("rlc_packets_injected", (double)process->getNumInjectedPackets());
    process->receiveFromUpper(data, priority);

    IArq *arq = getLowerLayer();
    if(arq) {
        arq->notifyOutgoing(process->getQueuedBits(), dest);
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
    //emit("Rlc:packet_injected_from_lower(bits)", (double) packet->getBits());
    MacId dest = packet->getDestination();
    auto process = getProcess(dest);
    if(process == nullptr) {
        process = new RlcProcess(dest);
        processes.insert(make_pair(dest, process));
    }

    process->receiveInjectionFromLower(packet, priority);

    IArq *arq = getLowerLayer();
    if(arq) {
        arq->notifyOutgoing(process->getQueuedBits(), dest);
    }
}

L2Packet * Rlc::requestSegment(unsigned int num_bits, const MacId &mac_id) {
    emit("rlc_bits_requested_from_lower", (double) num_bits);
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
    int header_size = mac_id != SYMBOLIC_LINK_ID_BROADCAST?
            (new L2HeaderUnicast(L2Header::FrameType::unicast))->getBits() :
                      (new L2HeaderBroadcast())->getBits();

    int counter = 0;

    while(has_more_data && counter < 100) {
        pair<L2Header*, L2Packet::Payload*> data = process->getData(num_bits - packet->getBits());
        packet->addMessage(data.first, data.second);

        has_more_data = process->hasDataToSend();

        // If the remaining space is too small for an additional header, we're done
        if(packet->getBits() + header_size >= num_bits) {
            has_more_data = false;
        }

        // TODO: add unicast data to broadcast if there is space
        counter++;
        emit("rlc_packet_sent_down", (double) packet->getBits());
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

unsigned int Rlc::getQueuedDataSize(MacId dest) {
    auto process = getProcess(dest);
    if(!process) {
        return 0;
    }
    return process->getQueuedBits();
}

Rlc::Rlc(int min_packet_size) {
    this->max_packet_size = min_packet_size;
}

Rlc::~Rlc() {
    for (auto it = processes.begin(); it != processes.end(); ++it)
    {
        delete it->second;
    }
}



