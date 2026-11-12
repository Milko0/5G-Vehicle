#include <omnetpp.h>
#include "VehiclePacket_m.h"

using namespace omnetpp;

// ApplicationServer: records the end-to-end delay of every application packet.
class ApplicationServer : public cSimpleModule
{
  private:
    simsignal_t endToEndDelaySig;

  protected:
    virtual void initialize() override
    {
        endToEndDelaySig = registerSignal("endToEndDelay");
    }

    virtual void handleMessage(cMessage *msg) override
    {
        VehiclePacket *pkt = check_and_cast<VehiclePacket *>(msg);
        if (pkt->getType() != APP_PACKET)
            throw cRuntimeError("AS received a non-application packet");

        // end-to-end delay = now - generation time at the vehicle
        emit(endToEndDelaySig, simTime() - pkt->getGenTime());
        delete pkt;
    }
};

Define_Module(ApplicationServer);
