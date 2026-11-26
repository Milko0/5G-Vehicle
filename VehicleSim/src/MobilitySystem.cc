#include "MobilitySystem.h"
#include <cmath>
#include <algorithm>

namespace _5g_connectedvehicle {

Define_Module(MobilitySystem);

void MobilitySystem::initialize()
{
    speed = par("speed").doubleValue(); 
    initialPosition = par("initialPosition").doubleValue();
    L = getSystemModule()->par("L").doubleValue();
    N = getSystemModule()->par("N").intValue();
    idealHandover = par("idealHandover").boolValue();

    // Calculate the inter-site distance
    isd = (N > 1) ? L / (N - 1) : 0.0;
    // Calculate the optimal base station based on the initial position
    optimalBS = (N > 1) ? std::clamp(static_cast<int>(std::round(initialPosition / isd)), 0, N - 1) : 0;
    idBS = optimalBS; // Initially, the vehicle is connected to the optimal base station
    // We assume rigth as the positive base direction
    // if the optimal base station is the last one, we need to move backward, otherwise we move forward
    direction = (optimalBS == N - 1) ? -1 : 1;

    subOptimalSignal = registerSignal("subOptimal");
    emit(subOptimalSignal, 0L);

    servingBSSignal = registerSignal("servingBS");
    emit(servingBSSignal, idBS);

    boundaryEvent = new cMessage("boundaryEvent");
    // Schedule the first boundary event based on the vehicle's speed
    if(speed > 0 && N > 1) {
        // Calculate the distance to the first boundary based on the initial position and the optimal base station
        double distToFirstBoundary = (optimalBS == N - 1) ? (L - initialPosition) + 0.5 * isd : (optimalBS + 0.5) * isd - initialPosition;
        scheduleAt(simTime() + (distToFirstBoundary / speed), boundaryEvent);
    }
}

// Function that return the current position of the vehicle based on its speed and the simulation time
double MobilitySystem::getPosition()
{
    simtime_t currentTime = simTime();
    double totalDistance = initialPosition + speed * currentTime.dbl();
    // use fmod to ensure the position wraps around the highway length
    double position = fmod(totalDistance, 2 * L);
    // Reflect the position if it exceeds the highway length
    if(position > L) {
        position = 2 * L - position;
    }
    return position;
}

void MobilitySystem::setIdBS(int newIdBS)
{   
    if(newIdBS != idBS) {
        idBS = newIdBS;
        // Emit the serving base station signal whenever the base station ID changes
        emit(servingBSSignal, getIdBS());
        // Emit the sub-optimal signal whenever the base station ID changes
        emit(subOptimalSignal, (getIdBS() != optimalBS) ? 1L : 0L);
    }
}

void MobilitySystem::handleMessage(cMessage *msg)
{
    if(msg == boundaryEvent) {
        optimalBS += direction; // Move to the next base station in the direction of travel
        if(optimalBS == N-1){
            direction = -1; // Change direction to backward
        } else if(optimalBS == 0){
            direction = 1; // Change direction to forward
        }
        
        // emit signal to indicate non optimal situation
        emit(subOptimalSignal, (getIdBS() != optimalBS) ? 1L : 0L);
        scheduleAt(simTime() + (isd / speed), boundaryEvent);

        // Emit the serving bs in case we are in the optimal situation
        if(idealHandover) {
            emit(servingBSSignal, getIdBS());
        }
    } else {
        throw cRuntimeError("[Vehicle:MS]: received an unknown message type");
    }
}

void MobilitySystem::finish()
{
    cancelAndDelete(boundaryEvent);
}

}