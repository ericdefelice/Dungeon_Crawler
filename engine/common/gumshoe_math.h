/*!
  @file
  gumshoe_math.h

  @brief
  Math related structures and functions for the game engine.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "math.h"
#include <random>


namespace Gumshoe {

//--------------------------------------------
// RNG Functions
//--------------------------------------------
int RandomInt(int exclusiveMax)
{
    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_int_distribution<> dist(0, exclusiveMax - 1);
    return dist(mt);
}
 
int RandomInt(int min, int max) // inclusive min/max
{
    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_int_distribution<> dist(0, max - min);
    return dist(mt) + min;
}
 
bool RandomBool(double probability = 0.5)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    
    std::bernoulli_distribution dist(probability);
    return dist(mt);
}


//--------------------------------------------
// Vector2 type
//--------------------------------------------
union Vector2_t
{
    struct
    {
        float x, y;
    };
    float e[2];
};


//--------------------------------------------
// Vector3 type
//--------------------------------------------
union Vector3_t
{
    struct
    {
        float x, y, z;
    };
    struct
    {
        float r, g, b;
    };
    float e[3];
};

inline Vector3_t
operator+(Vector3_t B, float A)
{
    Vector3_t result;

    result.x = A+B.x;
    result.y = A+B.y;
    result.z = A+B.z;

    return(result);
}

inline Vector3_t
operator+(Vector3_t A, Vector3_t B)
{
    Vector3_t result;

    result.x = A.x+B.x;
    result.y = A.y+B.y;
    result.z = A.z+B.z;

    return(result);
}

inline Vector3_t&
operator+=(Vector3_t &v1, Vector3_t v2)
{
    v1 = v1+v2;
    
    return(v1);
}

inline Vector3_t
operator*(float A, Vector3_t B)
{
    Vector3_t result;

    result.x = A*B.x;
    result.y = A*B.y;
    result.z = A*B.z;

    return(result);
}

inline Vector3_t
operator*(Vector3_t B, float A)
{
    Vector3_t result;

    result.x = A*B.x;
    result.y = A*B.y;
    result.z = A*B.z;

    return(result);
}

inline Vector3_t&
operator*=(Vector3_t &v1, float s1)
{
    v1 = v1*s1;
    
    return(v1);
}


//--------------------------------------------
// Scalar Functions
//--------------------------------------------
inline float Square(float input)
{
    float result = input*input;

    return(result);
}


inline float SquareRoot(float input)
{
    float result = sqrtf(input);
    return(result);
}


//--------------------------------------------
// Vector Functions
//--------------------------------------------
inline float Inner(Vector3_t v1, Vector3_t v2)
{
    float result = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

    return(result);
}


inline float LengthSq(Vector3_t v1)
{
    float result = Inner(v1, v1);

    return(result);
}

} // end of namespace Gumshoe
