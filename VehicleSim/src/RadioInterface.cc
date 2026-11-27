#include "RadioInterface.h"
#include "MobilitySystem.h"
#include "HandoverResponse_m.h"
#include "VehiclePacket_m.h"
#include <cmath>

namespace _5g_connectedvehicle {

Define_Module(RadioInterface);

void RadioInterface::initialize()
{
    BSRoadOffset = par("BSRoadOffset").doubleValue();
    alpha = par("alpha").doubleValue();
    // Pointer to the MobilitySystem module for accessing vehicle position and base station ID
    mobilitySystem = check_and_cast<MobilitySystem *>(getParentModule()->getSubmodule("mobilitySystem"));

    // Initialize the packet queue and the end transmission event
    packetQueue.setName("vehicleQueue");
    endTransmissionEvent = new cMessage("endTransmissionEvent");
    isBusy = false;

    // Register the queue length signal
    queueLengthSignal = registerSignal("queueLength");
    emit(queueLengthSignal, packetQueue.getLength());
    // Register the waiting time and service time signals
    waitingTimeSignal = registerSignal("waitingTime");
    serviceTimeSignal = registerSignal("serviceTime");
    // Register the utilization signal
    utilizationSignal = registerSignal("utilization");
    emit(utilizationSignal, isBusy ? 1L : 0L);
}

void RadioInterface::handleMessage(cMessage *msg)
{
    // Check if the message is a VehiclePacket or a HandoverResponse
    if(msg == endTransmissionEvent) {
        // If the message is the end transmission event, call the endTransmission function
        endTransmission();
    } else
    if(VehiclePacket *pkt = dynamic_cast<VehiclePacket *>(msg))
        // Handle VehiclePacket
        handlePacket(pkt);
    else if(HandoverResponse *res = dynamic_cast<HandoverResponse *>(msg))
        // Forward HandoverResponse to the vehicle's Handover System
        send(res, "handoverIO$o");
    else
        throw cRuntimeError("[Vehicle:RI]: received an unknown message type");
}

// Handle incoming VehiclePacket
void RadioInterface::handlePacket(VehiclePacket *packet)
{
    if (isBusy) {
        // If the radio interface is busy, enqueue the packet
        packetQueue.insert(packet);
    } else {
        // If the radio interface is free, send the packet immediately
        transmitPacket(packet);
    }
    // Emit the queue length signal after handling the packet
    emit(queueLengthSignal, packetQueue.getLength());

}

// After a transmission send packet in the queue or set not busy
void RadioInterface::endTransmission()
{
    if (!packetQueue.isEmpty()) {
        // If there are packets in the queue, dequeue the next packet and send it
        VehiclePacket *nextPacket = check_and_cast<VehiclePacket *>(packetQueue.pop());
        emit(queueLengthSignal, packetQueue.getLength());
        transmitPacket(nextPacket);
    } else {
        // If there are no packets in the queue, mark the interface as not busy
        isBusy = false;
        emit(utilizationSignal, 0L);
    }
}

// Function to transmit a packet and hadle trasmission time
void RadioInterface::transmitPacket(VehiclePacket *packet)
{   
    // Mark as busy and emit the utilization signal
    if(!isBusy) {
        isBusy = true;
        emit(utilizationSignal, 1L);
    }

    // Get the serving base station and the distance along the road from the mobility system
    int servingBS = mobilitySystem->getIdBS();
    double distance = mobilitySystem->getDistanceToBS();
    // Need to account the distance from the road to the base station
    distance = std::sqrt(distance * distance + BSRoadOffset * BSRoadOffset);

    // Calculate the transmission time based on the distance to the base station S = alpha * distance^2
    simtime_t transmissionTime = alpha * distance * distance;

    // Emit the service time and waiting time signals
    if(packet->getType() == PacketType::APP_PACKET) {
        emit(serviceTimeSignal, transmissionTime);
        simtime_t waitingTime = simTime() - packet->getGenTime();
        emit(waitingTimeSignal, waitingTime);
    }
    
    // Send the packet to the base station through the antenna output
    cModule *baseStation = getParentModule()->getParentModule()->getSubmodule("bs", servingBS);
    sendDirect(packet, transmissionTime, SimTime::ZERO, baseStation, "wirelessGate");
    // Schedule the end transmission event after the transmission time
    scheduleAt(simTime() + transmissionTime, endTransmissionEvent);
}

void RadioInterface::finish()
{
    cancelAndDelete(endTransmissionEvent);
}

}
