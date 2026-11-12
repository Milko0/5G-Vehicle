//
// ApplicationServer: sink of application packets.
// It measures the END-TO-END DELAY = arrival time at AS - generation time at vehicle.
//
simple ApplicationServer
{
    parameters:
        @display("i=device/server;is=s");

        @signal[endToEndDelay](type=simtime_t);
        @statistic[endToEndDelay](title="end-to-end delay of application packets"; unit=s;
                                  record=mean,max,vector);
    gates:
        input in[];
}
