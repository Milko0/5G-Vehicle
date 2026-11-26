#ifndef ___5G_CONNECTEDVEHICLE_BASESTATION_H_
#define ___5G_CONNECTEDVEHICLE_BASESTATION_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {

class BaseStation : public cSimpleModule
{ 
  private:
    double delay; // Delay for sending handover response to the vehicle
  protected:
    virtual void initialize() override { delay = par("delay").doubleValue(); }
    virtual void handleMessage(cMessage *msg) override;
};

}

#endif
