#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysPlayerStart, PhysStatic)
	bool PhysPlayerStart::Init()
	{
		if( !PhysStatic::Init() )
		{
			return false;
		}

		ScenePlayerStartData* CompData = (ScenePlayerStartData*)_ComponentData;
		ScenePlayerStartCompDesc* CompDesc = (ScenePlayerStartCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return false;
		}

		_TeamIndex = CompDesc->_TeamIndex;   //放在这里免得再去读comp表了
		_SeatIndex = CompDesc->_SeatIndex;
		_PriorVehicleType = CompDesc->_PriorVehicleType;

		return true;
	}

	IMPLEMENT_RTTI(PhysPlayerRespawn, PhysPlayerStart)
}