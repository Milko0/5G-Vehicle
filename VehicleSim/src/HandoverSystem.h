#ifndef ___5G_CONNECTEDVEHICLE_HANDOVERSYSTEM_H_
#define ___5G_CONNECTEDVEHICLE_HANDOVERSYSTEM_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {

class MobilitySystem;

class HandoverSystem : public cSimpleModule
{
private:
    simtime_t H; // Interval between handover packets
    cMessage *handoverTimer;
    MobilitySystem *mobilitySystem; // Gives the position of the vehicle and stores the serving BS
    simsignal_t handoverDelaySignal; // Signal to record the handover delay
    simsignal_t handoverCountSignal; // Signal to record the number of handovers
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

}

#endif
