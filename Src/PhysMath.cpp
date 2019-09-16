#include "PhysMath.h"

namespace GPL
{

PhysGlobalMath::PhysGlobalMath()
{
	// Init base angle table.
	{for( int i=0; i<NUM_ANGLES; i++ )
		TrigFLOAT[i] = ::sinf((float)i * 2.f * 3.14f / (float)NUM_ANGLES);}
}

PhysGlobalMath GPhysMath;

}