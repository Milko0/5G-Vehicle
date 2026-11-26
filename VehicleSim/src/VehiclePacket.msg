namespace _5g_connectedvehicle;

enum PacketType
{
    APP_PACKET = 0;  // Application packet sent to the AS
    HANDOVER_PACKET = 1;  // Handover packet sent to the MS
}

// Packet sent by the vehicle into the network
packet VehiclePacket
{
    PacketType type;
    simtime_t genTime;  // Generation time of the packet
    double genTimePosition; // Position of the vehicle when the packet was generated
}