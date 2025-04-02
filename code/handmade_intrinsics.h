#if !defined(HANDMADE_INTRINSICS_H)
#include "math.h"

inline int32
RoundReal32ToInt32(real32 real32)
{
    int32 result = (int32)(real32 + 0.5f);
    return(result);
}

inline uint32
RoundReal32ToUInt32(real32 real32)
{
    uint32 result = (uint32)(real32 + 0.5f);
    return(result);
}


inline int32
FloorReal32ToInt32(real32 real32)
{
    int32 result = (int32)floorf(real32);
    return(result);
}

inline int32
TruncateReal32ToInt32(real32 real32)
{
    int32 result = (int32)real32;
    return(result);
}

inline real32
Sin(real32 angle)
{
    real32 result = sinf(angle);
    return(result);
}

inline real32
Cos(real32 angle)
{
    real32 result = cosf(angle);
    return(result);
}

inline real32
ATan2(real32 y, real32 x)
{
    real32 result = atan2f(y, x);
    return(result);
}

#define HANDMADE_INTRINSICS_H
#endif
