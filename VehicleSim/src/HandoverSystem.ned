package _5g_connectedvehicle;

simple HandoverSystem
{
    parameters:
        double H @unit(s) = default(0.01s); // Interval between handover packets
        @signal[handoverDelay](type=simtime_t);
        @statistic[handoverDelay](title="handover delay"; unit=s;
                                  record=mean,max,vector);
        @signal[handoverCount](type=long);
        @statistic[handoverCount](title="number of handovers";
                                  record=count);
    gates:
        inout io; // Gate to communicate with the RadioInterface
}
