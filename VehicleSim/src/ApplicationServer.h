#ifndef ___5G_CONNECTEDVEHICLE_APPLICATIONSERVER_H
#define ___5G_CONNECTEDVEHICLE_APPLICATIONSERVER_H

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {

class ApplicationServer : public cSimpleModule
{
private:
    // Signal to record the end-to-end delay of application packets
    // This signal will be emitted every time an application packet is received
    simsignal_t endToEndDelaySig;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

};

#endif
