#include "HandoverSystem.h"
#include "VehiclePacket_m.h"
#include "HandoverResponse_m.h"
#include "MobilitySystem.h"

namespace _5g_connectedvehicle {

Define_Module(HandoverSystem);

void HandoverSystem::initialize()
{
    H = par("H").doubleValue();
    handoverTimer = new cMessage("handoverTimer");
    // The MobilitySystem is a sibling module: we take its pointer once (cross-module call)
    mobilitySystem = check_and_cast<MobilitySystem *>(getParentModule()->getSubmodule("mobilitySystem"));

    // Schedule the first handover packet if H is greater than 0 and not in ideal handover mode
    bool idealHandover = mobilitySystem->par("idealHandover").boolValue();
    if (!idealHandover && H > 0) {
        scheduleAt(simTime() + H, handoverTimer);
    }
    handoverDelaySignal = registerSignal("handoverDelay");
    handoverCountSignal = registerSignal("handoverCount");
}

void HandoverSystem::handleMessage(cMessage *msg)
{
    if (msg == handoverTimer) {
        // Create a new HandoverPacket and send it to the RadioInterface
        VehiclePacket *handoverPacket = new VehiclePacket("HandoverPacket");
        handoverPacket->setType(PacketType::HANDOVER_PACKET);
        handoverPacket->setGenTime(simTime());
        handoverPacket->setGenTimePosition(mobilitySystem->getPosition());
        send(handoverPacket, "io$o");

        // Reschedule the handover timer for the next interval
        scheduleAt(simTime() + H, handoverTimer);
        
    } else if (HandoverResponse *handoverPacket = dynamic_cast<HandoverResponse *>(msg)) {
        int BSId = handoverPacket->getNewBaseStationId();

        if(BSId != mobilitySystem->getIdBS()) {
            // Emit the handover count signal
            emit(handoverCountSignal, 1L);
        }

        // Update the current base station in the MobilitySystem
        mobilitySystem->setIdBS(BSId);
        
        // Emit the handover delay signal
        simtime_t handoverDelay = simTime() - handoverPacket->getRequestTime();
        emit(handoverDelaySignal, handoverDelay);
        delete handoverPacket; // Clean up the received packet
    } else {
        throw cRuntimeError("[HandoverSystem]: received an unknown message type");
    }
}

void HandoverSystem::finish()
{
    cancelAndDelete(handoverTimer);
}

}