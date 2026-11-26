package _5g_connectedvehicle;

simple Application
{   
    parameters:
        volatile double interArrivalTime @unit(s);
    gates:
        output out; // Gate to communicate with the RadioInterface
}
