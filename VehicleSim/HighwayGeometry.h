//
// HighwayGeometry.h
// Small helper functions shared by Vehicle and MobilityServer.
//
// BS i is placed at x_i = i * ISD (i = 0..N-1), with ISD = L / (N-1),
// at a lateral distance "roadOffset" from the road.
//

#ifndef HIGHWAYGEOMETRY_H_
#define HIGHWAYGEOMETRY_H_

#include <cmath>

// Inter-site distance: ISD = L / (N - 1)
inline double interSiteDistance(double L, int N)
{
    return L / (N - 1);
}

// Index of the BS closest to position x
inline int closestBs(double x, double L, int N)
{
    int i = (int) std::round(x / interSiteDistance(L, N));
    if (i < 0)
        i = 0;
    if (i > N - 1)
        i = N - 1;
    return i;
}

// Distance between the vehicle (at position x on the road) and BS number "bs"
inline double distanceToBs(double x, int bs, double L, int N, double roadOffset)
{
    double dx = x - bs * interSiteDistance(L, N);   // along the road
    return std::sqrt(dx * dx + roadOffset * roadOffset);
}

#endif
