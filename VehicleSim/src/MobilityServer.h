#ifndef ___5G_CONNECTEDVEHICLE_MOBILITYSERVER_H_
#define ___5G_CONNECTEDVEHICLE_MOBILITYSERVER_H_

#include <omnetpp.h>

using namespace omnetpp;

namespace _5g_connectedvehicle {

class MobilityServer : public cSimpleModule
{
private:
    double L;  // length of the highway
    int N;  // number of base stations
    double isd;  // inter-site distance between base stations
    
    int closestBS(double position);
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

}

#endif
