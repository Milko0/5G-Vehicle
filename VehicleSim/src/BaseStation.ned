package _5g_connectedvehicle;

simple BaseStation
{
    parameters:
        double delay @unit(s);
    gates:
        inout msIO; // Gate to communicate with the MobilityServer
        output asOut; // Gate to communicate with the ApplicationServer
        input wirelessGate @directIn;  // Gate to receive wireless messages from vehicles
}
