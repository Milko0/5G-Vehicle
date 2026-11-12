//
// Highway5G: the whole system (top-level network).
//
//   vehicle ~~radio~~> bs[0..N-1] ---backhaul---> as (Application Server)
//                                  ---backhaul---> ms (Mobility Server)
//   ms ~~control~~> vehicle (HandoverCommand)
//
network Highway5G
{
    parameters:
        int numBs = default(5);                              // N
        double highwayLength @unit(m) = default(2000m);      // L   (ISD = L/(N-1))
        double roadOffset @unit(m) = default(20m);           // lateral distance BS <-> road
        double backhaulDelay @unit(s) = default(1ms);        // BS -> AS and BS -> MS link delay
        @display("bgb=900,350");

    submodules:
        vehicle: Vehicle {
            @display("p=50,250");
        }
        bs[numBs]: BaseStation {
            @display("p=50,180,row,200");
        }
        as: ApplicationServer {
            @display("p=300,50");
        }
        ms: MobilityServer {
            @display("p=600,50");
        }

    connections:
        for i=0..numBs-1 {
            bs[i].toAS --> { delay = parent.backhaulDelay; } --> as.in++;
            bs[i].toMS --> { delay = parent.backhaulDelay; } --> ms.in++;
        }
}
