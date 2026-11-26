#include "BaseStation.h"
#include "VehiclePacket_m.h"
#include "HandoverResponse_m.h"

namespace _5g_connectedvehicle {

Define_Module(BaseStation);

void BaseStation::handleMessage(cMessage *msg)
{
    // Check if we receved a VehiclePacket or a HandoverResponse message
    if(VehiclePacket *pkt = dynamic_cast<VehiclePacket *>(msg)) {
        
        if (pkt->getType() == APP_PACKET) {
            // Forward application packets to the Application Server
            send(pkt, "asOut");
        } else if (pkt->getType() == HANDOVER_PACKET) {
            // Forward handover packets to the Mobility Server
            send(pkt, "msIO$o");
        } else {
            throw cRuntimeError("[BS]: received an unknown packet type");
        }

    } else if(HandoverResponse *res = dynamic_cast<HandoverResponse *>(msg)) {

        // Forward handover response packets to the vehicle with a delay
        cModule *targetVehicle = getParentModule()->getSubmodule("vehicle");
        sendDirect(res, delay, SimTime::ZERO, targetVehicle, "wirelessGate");

    } else {
        throw cRuntimeError("[BS]: received a non-VehiclePacket message");
    }
}

}
