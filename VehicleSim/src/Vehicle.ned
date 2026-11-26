package _5g_connectedvehicle;

module Vehicle
{
    gates:
        input wirelessGate @directIn;  // Gate to receive wireless messages from the Base Station
    submodules:
        handoverSystem: HandoverSystem;
        mobilitySystem: MobilitySystem;
        radioInterface: RadioInterface;
        application: Application;
    connections:
        radioInterface.in <-- wirelessGate;
        handoverSystem.io <--> radioInterface.handoverIO;
        radioInterface.applicationIn <-- application.out;
}
