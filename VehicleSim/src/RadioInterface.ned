package _5g_connectedvehicle;

simple RadioInterface
{   
    parameters:
        double BSRoadOffset @unit(m); // Distance from the road to the base station
        double alpha; // Proportionality constant for the propagation model
        @signal[queueLength](type=long);
        @statistic[queueLength](title="length of the queue in the radio interface"; 
                                interpolationmode=sample-hold; record=timeavg, max, vector);
        @signal[waitingTime](type=simtime_t);
        @statistic[waitingTime](title="waiting time of application packets"; unit=s;
                                  record=mean,max,vector);
        @signal[serviceTime](type=simtime_t);
        @statistic[serviceTime](title="service time of application packets"; unit=s;
                                  record=mean,max,vector);
        @signal[utilization](type=long);
        @statistic[utilization](title="utilization of the radio interface"; 
                                interpolationmode=sample-hold; record=timeavg, vector);
    gates:
        input in; // Gate to communicate with the BaseStation
        input applicationIn; // Gate to communicate with the Application
        inout handoverIO; // Gate to communicate with the Handover System

}
