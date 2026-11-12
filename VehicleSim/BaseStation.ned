//
// BaseStation: intermediate hop. It receives packets over the radio
// (sendDirect from the vehicle) and forwards them:
//   application packets -> Application Server
//   handover packets    -> Mobility Server
//
simple BaseStation
{
    parameters:
        @display("i=device/antennatower;is=s");
    gates:
        input radioIn @directIn;
        output toAS;
        output toMS;
}
