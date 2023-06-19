// The L-Band Digital Aeronautical Communications System (LDACS) Radio Link Control (RLC) implements the RLC protocol for the LDACS Air-Air Medium Access Control simulator.
// Copyright (C) 2023  Sebastian Lindner, Konrad Fuger, Musab Ahmed Eltayeb Ahmed, Andreas Timm-Giel, Institute of Communication Networks, Hamburg University of Technology, Hamburg, Germany

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
            PacketFragment frag = make_pair(headers[i]->copy(), payloads[i] != nullptr? payloads[i]->copy(): nullptr);
            process->receiveFromLower(frag);
        }
    }

    delete packet;

    L3Packet * pkt = process->getReassembledPacket();

    auto nwLayer = getUpperLayer();
    while (nwLayer && pkt != nullptr) {
        debug("Rlc::passingToNWLayer" + to_string(pkt->size));
        if(pkt->size > 0) {
            emit("rlc_packet_sent_up", (double) pkt->size);
            nwLayer->receiveFromLower(pkt);
        } else {
            delete pkt;
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

    emit("rlc_bits_to_send", (double)(getTotalBitsToSend()));
    emit("rlc_packets_to_send", (double)(getTotalPacketsToSend()));
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
    bool is_broadcast = mac_id == SYMBOLIC_LINK_ID_BROADCAST;

    if(process == nullptr) {
        L2Packet* empty = new L2Packet();        
        if (is_broadcast)
            empty->addMessage(new L2HeaderSH(), nullptr);
        else
            empty->addMessage(new L2HeaderPP(), nullptr);            
        return empty;
    }

    L2Packet* packet = process->getInjectedPacket();
    if(packet == nullptr) {
        packet = new L2Packet();        
        if (is_broadcast)
            packet->addMessage(new L2HeaderSH(), nullptr);
        else
            packet->addMessage(new L2HeaderPP(), nullptr);            
    }

    bool has_more_data = process->hasDataToSend();
    int unicast_header_size = 82;
    int broadcast_header_size = 25;
    int header_size = mac_id != SYMBOLIC_LINK_ID_BROADCAST?
            unicast_header_size :
                      broadcast_header_size;

    int maxFragments = 100;
    int fragkmentCounter = 0;

    while(has_more_data && fragkmentCounter < maxFragments) {
        pair<L2Header*, L2Packet::Payload*> data = process->getData(num_bits - packet->getBits());
        packet->addMessage(data.first, data.second);

        has_more_data = process->hasDataToSend();

        // If the remaining space is too small for an additional header, we're done
        if(packet->getBits() + header_size >= num_bits) {
            has_more_data = false;
        }

        fragkmentCounter++;
        emit("rlc_packet_sent_down", (double) packet->getBits());
        emit("rlc_bits_to_send", (double)(getTotalBitsToSend()));
        emit("rlc_packets_to_send", (double)(getTotalPacketsToSend()));
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

int Rlc::getTotalBitsToSend() {
    int total = 0;
    for (auto const& entry : processes)
    {
        total += entry.second->getQueuedBits();
    }
    return total;
}

int Rlc::getTotalPacketsToSend() {
    int total = 0;
    for (auto const& entry : processes)
    {
        total += entry.second->getQueuedPackets();
    }
    return total;

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



