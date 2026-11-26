#include "ApplicationServer.h"
#include "VehiclePacket_m.h"

namespace _5g_connectedvehicle {

Define_Module(ApplicationServer);

void ApplicationServer::initialize()
{   
    // Register the end-to-end delay signal
    endToEndDelaySig = registerSignal("endToEndDelay");
}

void ApplicationServer::handleMessage(cMessage *msg)
{   
    // Check if we receved a valid application packet
    VehiclePacket *pkt = check_and_cast<VehiclePacket *>(msg);
    if (pkt->getType() != APP_PACKET)
        throw cRuntimeError("AS received a non-application packet");

    // emit the new end-to-end delay signal
    emit(endToEndDelaySig, simTime() - pkt->getGenTime());
    delete pkt;
}

}
