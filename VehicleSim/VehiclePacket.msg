//
// VehiclePacket.msg
// Messages exchanged in the 5G connected-vehicle model.
// OMNeT++ generates VehiclePacket_m.h / VehiclePacket_m.cc from this file.
//

enum PacketType
{
    APP_PACKET = 0;        // application packet  : Vehicle -> BS -> Application Server
    HANDOVER_PACKET = 1;   // handover packet     : Vehicle -> BS -> Mobility Server
}

//
// Packet sent by the vehicle (both application and handover packets).
//
packet VehiclePacket
{
    int type @enum(PacketType);   // APP_PACKET or HANDOVER_PACKET
    simtime_t genTime;            // time the packet was generated at the vehicle
    double reportedPosition;      // [m] vehicle position at generation (handover packets only)
    int servingBs = -1;           // BS used to transmit this packet
}

//
// Command sent by the Mobility Server to the vehicle: "your new serving BS is newBs".
//
message HandoverCommand
{
    int newBs;
}
