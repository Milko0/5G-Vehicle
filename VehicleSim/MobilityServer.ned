//
// MobilityServer: receives handover packets, selects the BS closest to the
// reported position and tells the vehicle (HandoverCommand via sendDirect).
//
simple MobilityServer
{
    parameters:
        @display("i=device/server2;is=s");

        @signal[handoverDelay](type=simtime_t);
        @statistic[handoverDelay](title="delay of handover packets (vehicle -> MS)"; unit=s; record=mean,max);
    gates:
        input in[];
}
