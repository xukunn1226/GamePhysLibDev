#ifndef _PHYS_MATH_H_
#define _PHYS_MATH_H_

#include <stdio.h>
#include <math.h>

namespace GPL
{

/**
 * Global mathematics info.
 */
class PhysGlobalMath
{
public:
	// Constants.
	enum {ANGLE_SHIFT 	= 2};		// Bits to right-shift to get lookup value.
	enum {ANGLE_BITS	= 14};		// Number of valid bits in angles.
	enum {NUM_ANGLES 	= 16384}; 	// Number of angles that are in lookup table.
	enum {ANGLE_MASK    =  (((1<<ANGLE_BITS)-1)<<(16-ANGLE_BITS))};

	// Basic math functions.
	__forceinline float SinTab( int i ) const
	{
		return TrigFLOAT[((i>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	__forceinline float CosTab( int i ) const
	{
		return TrigFLOAT[(((i+16384)>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	float SinFloat( float F ) const
	{
		return SinTab(int((F*65536.f)/(2.f*3.14f)));
	}
	float CosFloat( float F ) const
	{
		return CosTab(int((F*65536.f)/(2.f*3.14f)));
	}

	// Constructor.
	PhysGlobalMath();

private:
	// Tables.
	float  TrigFLOAT		[NUM_ANGLES];
};

extern class PhysGlobalMath GPhysMath;

}

#endif