#include <omnetpp.h>
#include "VehiclePacket_m.h"
#include "HighwayGeometry.h"

using namespace omnetpp;

// MobilityServer: picks the BS closest to the reported position and
// sends a HandoverCommand to the vehicle.
class MobilityServer : public cSimpleModule
{
  private:
    simsignal_t handoverDelaySig;
    double L;
    int N;

  protected:
    virtual void initialize() override
    {
        handoverDelaySig = registerSignal("handoverDelay");
        L = getParentModule()->par("highwayLength");
        N = getParentModule()->par("numBs");
    }

    virtual void handleMessage(cMessage *msg) override
    {
        VehiclePacket *pkt = check_and_cast<VehiclePacket *>(msg);
        if (pkt->getType() != HANDOVER_PACKET)
            throw cRuntimeError("MS received a non-handover packet");

        emit(handoverDelaySig, simTime() - pkt->getGenTime());

        // new serving BS = closest to the position REPORTED in the packet
        // (the vehicle has moved since then: this is the "stale information" effect)
        HandoverCommand *cmd = new HandoverCommand("hoCommand");
        cmd->setNewBs(closestBs(pkt->getReportedPosition(), L, N));

        // control message back to the vehicle (assumption: zero delay)
        cModule *vehicle = getParentModule()->getSubmodule("vehicle");
        sendDirect(cmd, vehicle, "radioIn");

        delete pkt;
    }
};

Define_Module(MobilityServer);
