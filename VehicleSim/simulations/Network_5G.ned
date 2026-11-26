package _5g_connectedvehicle.simulations;

import _5g_connectedvehicle.*;

network Network_5G
{   
    parameters:
        double L @unit(m); // Highway length
        int N; // Number of BS
        double backhaulDelay @unit(s) = default(0.002s); // Backhaul delay between BS
        @display("bgb=900,400");
    submodules:
        applicationServer: ApplicationServer {
            @display("p=300,70;i=device/server;is=s");
        }
        mobilityServer: MobilityServer {
            @display("p=600,70;i=device/server2;is=s");
        }
        bs[N]: BaseStation {
            @display("p=70,220,r,75;i=device/antennatower;is=s");
        }
        vehicle: Vehicle {
            @display("p=450,330;i=device/pocketpc;is=s");
        }
    connections:
        for i = 0..N-1 {
            bs[i].asOut --> { delay = parent.backhaulDelay; } --> applicationServer.in++;
            bs[i].msIO <--> { delay = parent.backhaulDelay; } <--> mobilityServer.io++;
        }
}
