#include "Vehicle.h"
#include "HighwayGeometry.h"

Define_Module(Vehicle);

Vehicle::~Vehicle()
{
    cancelAndDelete(appTimer);
    cancelAndDelete(hoTimer);
    cancelAndDelete(txEndTimer);
    delete inTx;            // the packets still in "queue" are deleted by cQueue itself
}

void Vehicle::initialize()
{
    // parameters of this module
    speed         = par("speed");
    x0            = par("initialPosition");
    alpha         = par("alpha");
    H             = par("handoverInterval");
    idealHandover = par("idealHandover");

    // parameters of the network (parent module)
    L          = getParentModule()->par("highwayLength");
    N          = getParentModule()->par("numBs");
    roadOffset = getParentModule()->par("roadOffset");

    // signals
    queueingDelaySig = registerSignal("queueingDelay");
    serviceTimeSig   = registerSignal("serviceTime");
    txDistanceSig    = registerSignal("txDistance");
    misassociatedSig = registerSignal("misassociated");
    busySig          = registerSignal("busy");
    queueLengthSig   = registerSignal("queueLength");
    handoverSig      = registerSignal("handover");
    servingBsSig     = registerSignal("servingBs");

    // at the start the vehicle is served by the closest BS
    serving = closestBs(x0, L, N);
    emit(servingBsSig, serving);
    emit(queueLengthSig, 0);
    emit(busySig, false);

    // timers (self messages)
    appTimer   = new cMessage("appTimer");
    hoTimer    = new cMessage("hoTimer");
    txEndTimer = new cMessage("txEndTimer");

    scheduleAfter(par("appInterval"), appTimer);   // first application packet after a random T
    scheduleAfter(H, hoTimer);                     // first handover packet after H
}

// Position at time t: the vehicle goes 0 -> L, then back L -> 0, and so on.
// s = distance travelled modulo one round trip (2L)
double Vehicle::getPosition(simtime_t t)
{
    double s = fmod(x0 + speed * t.dbl(), 2 * L);
    if (s <= L)
        return s;            // going forward
    else
        return 2 * L - s;    // coming back
}

void Vehicle::handleMessage(cMessage *msg)
{
    if (msg == appTimer)
        generateAppPacket();
    else if (msg == hoTimer)
        generateHandoverPacket();
    else if (msg == txEndTimer)
        endTransmission();
    else if (msg->isName("hoCommand"))
        handleHandoverCommand(check_and_cast<HandoverCommand *>(msg));
    else
        throw cRuntimeError("Unknown message %s", msg->getName());
}

// ---------------- packet generation ----------------

void Vehicle::generateAppPacket()
{
    VehiclePacket *pkt = new VehiclePacket("app");
    pkt->setType(APP_PACKET);
    pkt->setGenTime(simTime());
    enqueue(pkt);

    // next application packet after T (a NEW random value is drawn every time)
    scheduleAfter(par("appInterval"), appTimer);
}

void Vehicle::generateHandoverPacket()
{
    VehiclePacket *pkt = new VehiclePacket("handover");
    pkt->setType(HANDOVER_PACKET);
    pkt->setGenTime(simTime());
    pkt->setReportedPosition(getPosition(simTime()));   // position at GENERATION time
    enqueue(pkt);

    scheduleAfter(H, hoTimer);   // one handover packet every H seconds
}

// ---------------- FIFO queue + transmission ----------------

void Vehicle::enqueue(VehiclePacket *pkt)
{
    queue.insert(pkt);
    emit(queueLengthSig, queue.getLength());

    if (!txEndTimer->isScheduled())    // transmitter idle -> start now
        startTransmission();
}

void Vehicle::startTransmission()
{
    inTx = check_and_cast<VehiclePacket *>(queue.pop());
    emit(queueLengthSig, queue.getLength());

    double x = getPosition(simTime());
    int best = closestBs(x, L, N);
    int bs = serving;
    if (idealHandover)
        bs = best;                     // benchmark: always the closest BS

    // S = alpha * d^2, with d computed NOW (when the transmission starts)
    double d = distanceToBs(x, bs, L, N, roadOffset);
    simtime_t S = alpha * d * d;

    inTx->setServingBs(bs);
    emit(busySig, true);
    emit(serviceTimeSig, S);
    if (inTx->getType() == APP_PACKET) {
        emit(queueingDelaySig, simTime() - inTx->getGenTime());
        emit(txDistanceSig, d);
        emit(misassociatedSig, bs != best);   // true if NOT sent via the closest BS
    }

    EV_INFO << "Tx " << inTx->getName() << " x=" << x << " m via BS " << bs
            << " d=" << d << " m S=" << S << " s\n";

    scheduleAfter(S, txEndTimer);
}

void Vehicle::endTransmission()
{
    // deliver the packet to the BS used for the transmission
    cModule *bs = getParentModule()->getSubmodule("bs", inTx->getServingBs());
    sendDirect(inTx, bs, "radioIn");
    inTx = nullptr;

    if (!queue.isEmpty())
        startTransmission();
    else
        emit(busySig, false);          // transmitter idle
}

// ---------------- handover ----------------

void Vehicle::handleHandoverCommand(HandoverCommand *cmd)
{
    int newBs = cmd->getNewBs();
    if (newBs != serving) {
        EV_INFO << "Handover: BS " << serving << " -> BS " << newBs << "\n";
        serving = newBs;
        emit(handoverSig, 1);
        emit(servingBsSig, serving);
    }
    delete cmd;
}
