#ifndef VEHICLE_H_
#define VEHICLE_H_

#include <omnetpp.h>
#include "VehiclePacket_m.h"

using namespace omnetpp;

class Vehicle : public cSimpleModule
{
  private:
    // ---- parameters ----
    double speed;              // V [m/s]
    double x0;                 // initial position [m]
    double alpha;              // S = alpha * d^2
    simtime_t H;               // handover period
    bool idealHandover;        // benchmark: always use the closest BS
    double L;                  // highway length [m]   (read from the network)
    int N;                     // number of BSs        (read from the network)
    double roadOffset;         // [m]                  (read from the network)

    // ---- state ----
    int serving;                       // index of the current serving BS
    cQueue queue;                      // FIFO shared by app and handover packets
    VehiclePacket *inTx = nullptr;     // packet being transmitted (nullptr = none)

    // ---- timers (self messages) ----
    cMessage *appTimer = nullptr;      // "generate an application packet"
    cMessage *hoTimer = nullptr;       // "generate a handover packet"
    cMessage *txEndTimer = nullptr;    // "the current transmission is over"

    // ---- signals ----
    simsignal_t queueingDelaySig, serviceTimeSig, txDistanceSig, misassociatedSig, busySig;
    simsignal_t queueLengthSig, handoverSig, servingBsSig;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    // one small function per event
    void generateAppPacket();
    void generateHandoverPacket();
    void endTransmission();
    void handleHandoverCommand(HandoverCommand *cmd);

    void enqueue(VehiclePacket *pkt);
    void startTransmission();
    double getPosition(simtime_t t);   // x(t), with direction reversal at the road ends

  public:
    virtual ~Vehicle();
};

#endif
