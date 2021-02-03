//
// Created by Konrad Fuger on 13.12.20.
//

#include "InetPacketPayload.hpp"

unsigned int InetPacketPayload::getBits() const{
    return size;
}

L2Packet::Payload* InetPacketPayload::copy() const {
	auto* copy = new InetPacketPayload();
	copy->size = this->size;
	copy->offset = this->offset;
	copy->original = this->original;
	return copy;
};
