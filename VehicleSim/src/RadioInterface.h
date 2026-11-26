#ifndef ___5G_CONNECTEDVEHICLE_RADIOINTERFACE_H_
#define ___5G_CONNECTEDVEHICLE_RADIOINTERFACE_H_

#include <omnetpp.h>
#include "VehiclePacket_m.h"

using namespace omnetpp;

namespace _5g_connectedvehicle {

class MobilitySystem;

class RadioInterface : public cSimpleModule
{
private:
    double BSRoadOffset; // Distance from the road to the base station
    double alpha; // Proportionality constant for the propagation model
    MobilitySystem *mobilitySystem; // Gives the position of the vehicle and the serving BS

    cQueue packetQueue; // Queue to hold packets waiting to be sent
    cMessage *endTransmissionEvent; // Self-message to indicate the end of a transmission
    bool isBusy; // Flag to indicate if the radio interface is currently busy transmitting

    simsignal_t queueLengthSignal; // Signal to record the length of the packet queue
    simsignal_t waitingTimeSignal; // Signal to record the waiting time of packets
    simsignal_t serviceTimeSignal; // Signal to record the service time of packets
    simsignal_t utilizationSignal; // Signal to record the utilization of the radio interface

    // Function that handles incoming VehiclePacket messages incoming
    void handlePacket(VehiclePacket *packet);
    // After a transmission send packet in the queue or set not busy
    void endTransmission();
    // Function to transmit a packet and hadle trasmission time
    void transmitPacket(VehiclePacket *packet);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

}

#endif
