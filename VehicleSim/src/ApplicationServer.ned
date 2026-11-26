package _5g_connectedvehicle;

simple ApplicationServer
{   
    parameters:
        @signal[endToEndDelay](type=simtime_t);
        @statistic[endToEndDelay](title="end-to-end delay of application packets"; unit=s;
                                  record=mean,max,vector);
    gates:
        input in[];
}
