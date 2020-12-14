//
// Created by Konrad Fuger on 03.12.20.
//

#ifndef TUHH_INTAIRNET_RLC_RLCPROCESS_HPP
#define TUHH_INTAIRNET_RLC_RLCPROCESS_HPP

#include <deque>
#include <utility>
#include <IRlc.hpp>
#include "L2Packet.hpp"
#include "L3Packet.hpp"

using namespace std;
using namespace TUHH_INTAIRNET_MCSOTDMA;

namespace TUHH_INTAIRNET_RLC {

class RlcProcess {
protected:
    MacId dest = SYMBOLIC_ID_UNSET;
    deque<L3Packet *> packets_to_send;
    deque<L2Packet *> packets_received;
    deque<L2Packet *> injected_packets;
public:
    RlcProcess(MacId id);
    MacId getMacId();
    void receiveFromUpper(L3Packet* data, PacketPriority priority = PRIORITY_DEFAULT);
    void receiveFromLower(L2Packet *pkt);
    void receiveInjectionFromLower(L2Packet* packet, PacketPriority priority = PRIORITY_LINK_MANAGEMENT);
    L2Packet* getInjectedPacket();
    pair<L2Header*, L2Packet::Payload*> getData(unsigned int num_bits);
    bool hasDataToSend();
    L3Packet* getReassembledPacket();
};

}
#endif //TUHH_INTAIRNET_RLC_RLCPROCESS_HPP
