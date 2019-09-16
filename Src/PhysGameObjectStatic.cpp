#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysStatic, PhysGameObject)
	bool PhysStatic::Init()
	{
		if( !PhysGameObject::Init() )
		{
			return false;
		}

		SceneStaticData* CompData = (SceneStaticData*)_ComponentData;
		if( CompData == NULL )
		{
			return false;
		}

		StaticCompDesc* CompDesc = (StaticCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return false;
		}

		_StaticID = CompDesc->_StaticID;

		return true;
	}

	void PhysStatic::Term()
	{
		PhysGameObject::Term();
	}

	void PhysStatic::Tick(float DeltaTime)
	{

	}

	void PhysStatic::InitPhys()
	{
		SceneStaticData* CompData = (SceneStaticData*)_ComponentData;
		if( CompData == NULL || CompData->_PhysModelDesc == NULL )
		{
			return;
		}

		StaticCompDesc* CompDesc = (StaticCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}

		NxActorDesc ActorDesc;
		std::vector<NxShapeDesc*> CachedShapeDescList;
		for(NxU32 ShapeIndex = 0; ShapeIndex < CompData->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
		{
			NxShapeDesc* XShapeDesc = NULL;
			ShapeDesc* CustomShapeDesc = (ShapeDesc*)CompData->_PhysModelDesc->_ShapeDescList[ShapeIndex];

			// create NxShapeDesc data
			XShapeDesc = CreateComponentShape(CustomShapeDesc, CompDesc->_ModelScale);
			if( XShapeDesc != NULL )
			{
				XShapeDesc->group = GPL_SHAPE_GROUP_STATICOBJECT;

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
				_PhysActor->userData = this;					// 绑定逻辑对象至NxActor上
			}
		}
		ReleaseShapeDescList(CachedShapeDescList);
	}

	void PhysStatic::PostInitPhys()
	{

	}
	void PhysStatic::PrevTermPhys()
	{
		PhysGameObject::PrevTermPhys();
	}

	NxMat34	PhysStatic::GetGlobalPos() const
	{
		return _ComponentData->_InitGlobalPose;
	}
	
	NxMat34	PhysStatic::UGetGlobalPos() const
	{
		NxMat34 GlobalPos = GetGlobalPos();
		GlobalPos.t *= P2GScale;
		return GlobalPos;
	}

	NxMat33	PhysStatic::GetGlobalOritation() const
	{
		return _ComponentData->_InitGlobalPose.M;
	}

	NxQuat	PhysStatic::GetGlobalOritationQuat() const
	{
		NxQuat Quat;
		_ComponentData->_InitGlobalPose.M.toQuat(Quat);
		return Quat;
	}

	NxVec3	PhysStatic::GetGlobalPosition() const
	{
		return _ComponentData->_InitGlobalPose.t;
	}

	NxVec3	PhysStatic::UGetGlobalPosition() const
	{
		return _ComponentData->_InitGlobalPose.t * P2GScale;
	}
}