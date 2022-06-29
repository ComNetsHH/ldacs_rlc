//
// Created by Konrad Fuger on 03.12.20.
//

#ifndef TUHH_INTAIRNET_RLC_RLC_HPP
#define TUHH_INTAIRNET_RLC_RLC_HPP

#include <map>
#include "IRlc.hpp"
#include "IOmnetPluggable.hpp"
#include "RlcProcess.hpp"

using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace std;

namespace TUHH_INTAIRNET_RLC {

class Rlc : public IRlc, public IOmnetPluggable {
protected:

    /** map to hold all RlcProcesses **/
    map<MacId, RlcProcess*> processes;

    /** maximum size of packet, if set all packets are at most this size **/
    int max_packet_size = -1;

    int getTotalBitsToSend();
    int getTotalPacketsToSend();

public:
    Rlc(int min_packet_size);
    RlcProcess* getProcess(MacId mac_id) const;
    void receiveFromUpper(L3Packet* data, MacId dest, PacketPriority priority = PRIORITY_DEFAULT);
    void receiveInjectionFromLower(L2Packet* packet, PacketPriority priority = PRIORITY_LINK_MANAGEMENT);
    L2Packet* requestSegment(unsigned int num_bits, const MacId& mac_id);
    void receiveFromLower(L2Packet* packet);
    bool isThereMoreData(const MacId& mac_id) const;

    void onEvent(double time);
    unsigned int getQueuedDataSize(MacId dest) override;

    ~Rlc();
};

}

#endif //TUHH_INTAIRNET_RLC_RLC_HPP
