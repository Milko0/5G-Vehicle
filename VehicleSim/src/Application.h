#ifndef ___5G_CONNECTEDVEHICLE_APPLICATION_H_
#define ___5G_CONNECTEDVEHICLE_APPLICATION_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {

class Application : public cSimpleModule
{
private:
    cMessage *sendPacketEvent; // Event to trigger packet sending
private:
    // Function to abstract the inter-arrival time
    simtime_t getInterArrivalTime() { return par("interArrivalTime").doubleValue(); }
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

}

#endif
