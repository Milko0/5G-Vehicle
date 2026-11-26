#include "MobilityServer.h"
#include "VehiclePacket_m.h"
#include "HandoverResponse_m.h"
#include <cmath>
#include <algorithm>

namespace _5g_connectedvehicle {

Define_Module(MobilityServer);

void MobilityServer::initialize()
{
    L = getParentModule()->par("L");
    N = getParentModule()->par("N");
    isd = (N > 1) ? L / (N - 1) : 0.0; // Calculate the inter-site distance
}

void MobilityServer::handleMessage(cMessage *msg)
{
    VehiclePacket *pkt = check_and_cast<VehiclePacket *>(msg);
    if(pkt->getType() != HANDOVER_PACKET)
        throw cRuntimeError("[MS]: received a non-handover packet");

    // Calculate the new serving BS
    int newServingBS = closestBS(pkt->getGenTimePosition());
    
    // Create a HandoverCommand message to send back to the vehicle
    HandoverResponse *res = new HandoverResponse("handoverResponse");
    res->setNewBaseStationId(newServingBS);
    res->setRequestTime(pkt->getGenTime());
    
    // Send the response back to the vehicle using the same gate index as the incoming packet
    send(res, "io$o", pkt->getArrivalGate()->getIndex());
    delete pkt;
}

// Helper function to find the closest BS
int MobilityServer::closestBS(double position)
{
    if(N <= 1) {
        return 0; // If there's only one BS, return it
    }
    // Calculate the closest base station based on the vehicle's position
    int idBS = static_cast<int>(std::round(position / isd));
    return std::clamp(idBS, 0, N - 1); // Clamp the value to ensure it's within valid range
}

}