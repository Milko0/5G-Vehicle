#ifndef ___5G_CONNECTEDVEHICLE_MOBILITYSYSTEM_H_
#define ___5G_CONNECTEDVEHICLE_MOBILITYSYSTEM_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {
class MobilitySystem : public cSimpleModule
{
private:
    double speed; // Speed of the vehicle in m/s
    int idBS; // ID of the base station the vehicle is currently connected to
    double L; // Length of the highway
    bool idealHandover; // If true, the radio always uses the nearest base station
    double initialPosition; // Initial position of the vehicle

    // All the below attributes are used to calculate the sub-optimal ratio
    int N; // Number of base stations
    double isd; // Inter-site distance between base stations
    int optimalBS; // ID of the optimal base station for the vehicle
    int direction; // Direction of the vehicle (1 for forward, -1 for backward)
    cMessage *boundaryEvent; // Event segnaling when the vehicle reach the bound between two base stations
    simsignal_t subOptimalSignal; // Signal to record sub-optimal association events
    
    simsignal_t servingBSSignal; // Signal to record the current serving base station ID

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
public:
    // Getters and Setters used by the other vehicle's modules
    double getSpeed() const { return speed; }
    int getIdBS() const { return idealHandover ? optimalBS : idBS; }
    void setIdBS(int newIdBS);
    int getOptimalBS() const { return optimalBS; }
    bool isIdealHandover() const { return idealHandover; }
    // Calculate on time the position so we don't have to update it every time we move the vehicle
    // We can use the speed and the simulation time to calculate the position of the vehicle on the highway
    double getPosition();
};

}

#endif
