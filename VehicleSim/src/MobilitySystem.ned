package _5g_connectedvehicle;

simple MobilitySystem
{
    parameters:
        double speed @unit(mps); // Vehicle speed
        bool idealHandover = default(false); // If true, the radio always use the nearest bs
        double initialPosition @unit(m) = default(0m); // Initial position of the vehicle
        
        @signal[subOptimal](type=long);
        @statistic[subOptimal](title="Sub-optimal association ratio";
                               interpolationmode=sample-hold;
                               record=timeavg,vector);
        @signal[servingBS](type=long);
        @statistic[servingBS](title="current Serving Base Station ID";
                               interpolationmode=sample-hold;
                               record=vector);
                               
}
