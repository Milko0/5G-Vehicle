#include "Application.h"
#include "VehiclePacket_m.h"

namespace _5g_connectedvehicle {

Define_Module(Application);

void Application::initialize()
{
    // Schedule the first packet sending event
    this->sendPacketEvent = new cMessage("sendPacketEvent");
    scheduleAt(simTime() + getInterArrivalTime(), sendPacketEvent);
}

void Application::handleMessage(cMessage *msg)
{
    if(msg == sendPacketEvent) {
        // Create a new packet and send it to the RadioInterface
        VehiclePacket *packet = new VehiclePacket("VehiclePacket");
        packet->setType(PacketType::APP_PACKET);
        packet->setGenTime(simTime());
        send(packet, "out");
        // Schedule the next packet sending event
        scheduleAt(simTime() + getInterArrivalTime(), sendPacketEvent);
    } else {
        throw cRuntimeError("[Vehicle:App]: received an unknown message type");
    }

}

void Application::finish()
{
    cancelAndDelete(sendPacketEvent);
}

}