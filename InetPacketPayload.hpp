//
// Created by Konrad Fuger on 13.12.20.
//

#ifndef TUHH_INTAIRNET_RLC_INETPACKETPAYLOAD_HPP
#define TUHH_INTAIRNET_RLC_INETPACKETPAYLOAD_HPP

#include "L2Packet.hpp"

using namespace TUHH_INTAIRNET_MCSOTDMA;

namespace inet {
    class Packet;
}


class InetPacketPayload : public L2Packet::Payload {
public:
    inet::Packet* original = nullptr;
    unsigned int size = 0;
    unsigned int offset = 0;
    unsigned int getBits() const override;

    Payload* copy() const override;
    ~InetPacketPayload();
};


#endif //TUHH_INTAIRNET_RLC_INETPACKETPAYLOAD_HPP
