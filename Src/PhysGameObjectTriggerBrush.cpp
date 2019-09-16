#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysTrigger, PhysStatic)
	bool PhysTrigger::Init()
	{
		if( !PhysStatic::Init() )
		{
			return false;
		}

		SceneTriggerData* CompData = (SceneTriggerData*)_ComponentData;
		if( CompData == NULL || CompData->_PhysModelDesc == NULL )
		{
			return false;
		}

		SceneTriggerCompDesc* CompDesc = (SceneTriggerCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return false;
		}

		_PhyAreaType = CompDesc->_PhyAreaType;
		_AreaId = CompDesc->_AreaId;

		return true;
	}
	void PhysTrigger::Term()
	{
		PhysStatic::Term();
	}
	void PhysTrigger::Tick(float DeltaTime)
	{

	}

	void PhysTrigger::InitPhys()
	{
		SceneTriggerData* CompData = (SceneTriggerData*)_ComponentData;
		if( CompData == NULL || CompData->_PhysModelDesc == NULL )
		{
			return;
		}

		SceneTriggerCompDesc* CompDesc = (SceneTriggerCompDesc*)CompData->_CompDesc;
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
				XShapeDesc->group = GPL_SHAPE_GROUP_TRIGGER_VOLUME;
				XShapeDesc->shapeFlags |= (NX_TRIGGER_ON_ENTER | NX_TRIGGER_ON_LEAVE);
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
	void PhysTrigger::PostInitPhys()
	{

	}
	void PhysTrigger::PrevTermPhys()
	{
		PhysStatic::PrevTermPhys();
	}

	//  [4/22/2013 chenpu]
	IMPLEMENT_RTTI(RangeTrigger, PhysTrigger)

	bool RangeTrigger::Init()
	{
		if( _Scene == NULL )
			return false;

		InitPhys();

		PostInitPhys();

		NxQuat Rot;
		Rot.fromAngleAxis(-90, AXIS_VEC_X);
		SetGlobalOritationQuat(Rot);

		_bInitialized = true;
		return _bInitialized;
	}

	void RangeTrigger::Term()
	{
		// 释放内部new的对象
		_bInitialized = false;

		PrevTermPhys();

		TermPhys();
	}

	void RangeTrigger::Tick(float DeltaTime)
	{
	}

	void RangeTrigger::Activate()
	{
		if ( !_PhysActor )
			return;

		const UINT SHAPE_NUM = _PhysActor->getNbShapes();
		NxShape* const* shapelist = _PhysActor->getShapes();
		if ( !shapelist )
			return;

		for ( UINT idx = 0; idx < SHAPE_NUM; idx++ )
		{
			NxShape* pShape = shapelist[idx];
			if ( !pShape )
				continue;

			pShape->setFlag(NX_TRIGGER_ON_ENTER, true);
			pShape->setFlag(NX_TRIGGER_ON_LEAVE, true);
		}

		_Activated = true;

#ifdef _DEBUG
		NxVec3 Pos = _PhysActor->getGlobalPosition();
		gplDebugf(TEXT("Trigger Actived : [%f, %f, %f]"), Pos.x, Pos.y, Pos.z);
#endif // _DEBUG
	}

	void RangeTrigger::Deactive()
	{
		if ( !_PhysActor )
			return;

		const UINT SHAPE_NUM = _PhysActor->getNbShapes();
		NxShape* const* shapelist = _PhysActor->getShapes();
		if ( !shapelist )
			return;

		for ( UINT idx = 0; idx < SHAPE_NUM; idx++ )
		{
			NxShape* pShape = shapelist[idx];
			if ( !pShape )
				continue;

			pShape->setFlag(NX_TRIGGER_ON_ENTER, false);
			pShape->setFlag(NX_TRIGGER_ON_LEAVE, false);
		}

		_Activated = false;

#ifdef _DEBUG
		NxVec3 Pos = _PhysActor->getGlobalPosition();
		gplDebugf(TEXT("Trigger Deactived : [%f, %f, %f]"), Pos.x, Pos.y, Pos.z);
#endif // _DEBUG
	}

	void RangeTrigger::InitPhys()
	{
		NxCapsuleShapeDesc capsuleDesc;
		capsuleDesc.height = _Extends;
		capsuleDesc.radius = _Radius;
		if (_CheckVehicle)
			capsuleDesc.group = GPL_SHAPE_GROUP_FLARE_VOLUME;
		else if (_CheckAircraft)
			capsuleDesc.group = GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME;
		else
		{
			gplDebugf(TEXT("Warnning! Create Invalid RangeTrigger!"));
		}

		NxBodyDesc BodyDesc;
		BodyDesc.linearDamping = 0.f;
		BodyDesc.angularDamping = 0.f;
		BodyDesc.flags |= NX_BF_DISABLE_GRAVITY;
		BodyDesc.sleepEnergyThreshold = 0.5f;
		BodyDesc.sleepDamping = 0.2f;

		NxActorDesc ActorDesc;
		ActorDesc.shapes.push_back(&capsuleDesc);
		ActorDesc.body = &BodyDesc;
		ActorDesc.density = 1.0f;
		ActorDesc.globalPose.zero();
		ActorDesc.flags = NX_AF_DISABLE_RESPONSE;

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
	}

	void RangeTrigger::PostInitPhys()
	{

	}

	void RangeTrigger::PrevTermPhys()
	{
		PhysTrigger::PrevTermPhys();
	}

	//  [7/13/2015 chenpu]
	IMPLEMENT_RTTI(AssociatedTrigger, RangeTrigger)

	bool AssociatedTrigger::Init()
	{
		if( _Scene == NULL )
			return false;

		InitPhys();

		PostInitPhys();

		_bInitialized = true;
		return _bInitialized;
	}

	void AssociatedTrigger::Term()
	{
		// 释放内部new的对象
		_bInitialized = false;

		PrevTermPhys();

		TermPhys();
	}

	void AssociatedTrigger::Tick(float DeltaTime)
	{
		if (NULL == _Host)
			return;

		SetGlobalPose(_Host->GetGlobalPose());
	}

	void AssociatedTrigger::InitPhys()
	{
		NxCapsuleShapeDesc capsuleDesc;
		capsuleDesc.height = _Extends;
		capsuleDesc.radius = _Radius;
		capsuleDesc.localPose.M.rotZ(NxHalfPi);
		capsuleDesc.localPose.t.x = _Extends * 0.5f + _Radius;
		if (_CheckVehicle)
			capsuleDesc.group = GPL_SHAPE_GROUP_FLARE_VOLUME;
		else if (_CheckAircraft)
			capsuleDesc.group = GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME;
		else
		{
			gplDebugf(TEXT("Warnning! Create Invalid AssociatedTrigger!"));
		}

		NxBodyDesc BodyDesc;
		BodyDesc.linearDamping = 0.f;
		BodyDesc.angularDamping = 0.f;
		BodyDesc.flags |= NX_BF_DISABLE_GRAVITY | NX_BF_FROZEN_POS_Z | NX_BF_FROZEN_ROT_X;
		BodyDesc.sleepEnergyThreshold = 0.5f;
		BodyDesc.sleepDamping = 0.2f;

		NxActorDesc ActorDesc;
		ActorDesc.shapes.push_back(&capsuleDesc);
		ActorDesc.body = &BodyDesc;
		ActorDesc.density = 1.0f;
		ActorDesc.globalPose.zero();
		ActorDesc.flags = NX_AF_DISABLE_RESPONSE;

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
	}

	void AssociatedTrigger::PostInitPhys()
	{

	}

	void AssociatedTrigger::PrevTermPhys()
	{
		PhysTrigger::PrevTermPhys();
	}

	void AssociatedTrigger::Bind(Turrent* Host)
	{
		if (!Host)
			return;

		_Host = Host;
		_Host->OnBind(this);

		Tick(0);
	}
}