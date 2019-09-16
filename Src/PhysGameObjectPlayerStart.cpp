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

		_TeamIndex = CompDesc->_TeamIndex;   //�������������ȥ��comp����
		_SeatIndex = CompDesc->_SeatIndex;
		_PriorVehicleType = CompDesc->_PriorVehicleType;

		return true;
	}

	IMPLEMENT_RTTI(PhysPlayerRespawn, PhysPlayerStart)
}