//
// Created by Konrad Fuger on 03.12.20.
//

#ifndef TUHH_INTAIRNET_RLC_RLC_H
#define TUHH_INTAIRNET_RLC_RLC_H

#include <map>
#include "IRlc.hpp"
#include "RlcProcess.h"

using namespace TUHH_INTAIRNET_MCSOTDMA;
using namespace std;

namespace TUHH_INTAIRNET_RLC {

class Rlc : public IRlc {
protected:

    /** map to hold all RlcProcesses **/
    map<MacId, RlcProcess*> processes;

public:
    RlcProcess* getProcess(MacId mac_id);
    void receiveFromUpper(L3Packet* data, MacId dest, PacketPriority priority = PRIORITY_DEFAULT);
    void receiveInjectionomLower(L2Packet* packet, PacketPriority priority = PRIORITY_LINK_MANAGEMENT);
    virtual L2Packet* requestSegment(unsigned int num_bits, const MacId& mac_id);
};

}

#endif //TUHH_INTAIRNET_RLC_RLC_H
