/*!
  @file
  gumshoe_intrinsics.h

  @brief
  Helper functions for the game engine.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <math.h>


namespace Gumshoe {

//--------------------------------------------
// Minimum/Maximum Functions
//--------------------------------------------
inline float Minimum(float in0, float in1)
{
    float result = (in0 < in1) ? in0 : in1;
    return(result);
}

inline uint32 Minimum(uint32 in0, uint32 in1)
{
    uint32 result = (in0 < in1) ? in0 : in1;
    return(result);
}

inline float Maximum(float in0, float in1)
{
    float result = (in0 > in1) ? in0 : in1;
    return(result);
}

inline uint32 Maximum(uint32 in0, uint32 in1)
{
    uint32 result = (in0 > in1) ? in0 : in1;
    return(result);
}


//--------------------------------------------
// Rounding Functions
//--------------------------------------------
inline uint32 FloorFloattoUint32(float input)
{
    uint32 result = (uint32)floorf(input);

    return(result);
}

inline uint32 CeilFloattoUint32(float input)
{
    uint32 result = (uint32)ceilf(input);

    return(result);
}

} // end of namespace Gumshoe
