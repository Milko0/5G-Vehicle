//
// Vehicle: the 5G-enabled car.
//  - moves at constant speed V on [0, L] and reverts direction at the ends
//  - generates one application packet every T seconds (T is a random variable)
//  - generates one handover packet every H seconds (contains its position)
//  - transmits ONE packet at a time from a single FIFO queue shared by both
//    packet types; transmission time S = alpha * d^2 (d = distance to serving BS)
//
simple Vehicle
{
    parameters:
        double speed @unit(mps) = default(30mps);              // V
        double initialPosition @unit(m) = default(0m);         // x(0)
        volatile double appInterval @unit(s);                  // T  (random: a new value at every packet)
        double handoverInterval @unit(s) = default(1s);        // H
        double alpha = default(1.28e-7);                       // [s/m^2]  S = alpha * d^2
        bool idealHandover = default(false);                   // true = always use the closest BS (benchmark)
        @display("i=block/circle,blue;is=s");

        // ---- signals (values emitted by the C++ code) ----
        @signal[queueingDelay](type=simtime_t);
        @signal[serviceTime](type=simtime_t);
        @signal[txDistance](type=double);
        @signal[misassociated](type=bool);
        @signal[busy](type=bool);
        @signal[queueLength](type=long);
        @signal[handover](type=bool);
        @signal[servingBs](type=long);

        // ---- statistics (what OMNeT++ records from each signal) ----
        @statistic[queueingDelay](title="queueing delay of app packets"; unit=s; record=mean,max);
        @statistic[serviceTime](title="transmission time S"; unit=s; record=mean,max);
        @statistic[txDistance](title="distance to serving BS at tx"; unit=m; record=mean,max);
        @statistic[misassociated](title="app packets sent via a non-closest BS"; record=mean);
        @statistic[utilization](source=busy; title="transmitter busy fraction"; record=timeavg);
        @statistic[queueLength](title="packets in the vehicle FIFO"; record=timeavg,max);
        @statistic[handover](title="number of serving-BS changes"; record=count);
        @statistic[servingBs](title="serving BS index"; record=vector);

    gates:
        input radioIn @directIn;      // receives the HandoverCommand from the Mobility Server
}
