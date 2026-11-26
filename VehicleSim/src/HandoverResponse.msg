namespace _5g_connectedvehicle;

// Packet sent by the MB to the vehicle in response to a handover request
packet HandoverResponse
{
    int newBaseStationId;  // ID of the new base station for the vehicle
    simtime_t requestTime;  // Time when the handover request was generated
}