#include <omnetpp.h>
#include "VehiclePacket_m.h"

using namespace omnetpp;

// BaseStation: forwards application packets to the AS and handover packets to the MS.
class BaseStation : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg) override
    {
        VehiclePacket *pkt = check_and_cast<VehiclePacket *>(msg);
        if (pkt->getType() == APP_PACKET)
            send(pkt, "toAS");
        else
            send(pkt, "toMS");
    }
};

Define_Module(BaseStation);
