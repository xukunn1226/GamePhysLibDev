#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysOcean, PhysStatic)
	void PhysOcean::InitPhys()
	{
		SceneOceanData* CompData = (SceneOceanData*)_ComponentData;
		if( CompData == NULL || CompData->_PhysModelDesc == NULL )
		{
			return;
		}

		SceneOceanCompDesc* CompDesc = (SceneOceanCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		NxActorDesc ActorDesc;
		std::vector<NxShapeDesc*> CachedShapeDescList;
		for(NxU32 ShapeIndex = 0; ShapeIndex < CompData->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
		{
			NxShapeDesc* XShapeDesc = NULL;
			ShapeDesc* CustomShapeDesc = CompData->_PhysModelDesc->_ShapeDescList[ShapeIndex];

			// create NxShapeDesc data
			XShapeDesc = CreateComponentShape(CustomShapeDesc, CompDesc->_ModelScale);
			if( XShapeDesc != NULL )
			{
				XShapeDesc->group = GPL_SHAPE_GROUP_OCEAN;

				ActorDesc.shapes.push_back(XShapeDesc);
			}
			else
			{
				gplDebugf(TEXT("ERROR: Faied to CreateComponentShape CompId[%d] PhysId[%s] ShapeIndex[%d]"), CompData->_CompDesc->_CompId, CompData->_PhysModelDesc->_PhysId.c_str(), ShapeIndex);
			}
			CachedShapeDescList.push_back(XShapeDesc);
		}

		ActorDesc.body = NULL;
		ActorDesc.globalPose = _ComponentData->_InitGlobalPose;

		NxU32 ValidCode = ActorDesc.checkValid();
		if( ValidCode == 0 )
		{
			_PhysActor = _Scene->_PhysScene->createActor(ActorDesc);
			if( _PhysActor != NULL )
			{
				_PhysActor->setGroup(GPL_GROUP_DEFAULT);		// default generate contact information
				_PhysActor->userData = this;		// 绑定逻辑对象至NxActor上
			}
		}
		ReleaseShapeDescList(CachedShapeDescList);
	}
	
}