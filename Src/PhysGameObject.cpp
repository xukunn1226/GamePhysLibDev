#include "..\Inc\PhysXSupport.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysGameObject, GameObject)

bool PhysGameObject::Init()
{
	if( _Scene == NULL )
		return false;

	_Scene->AddGameObject(this);

	// 生成物理对象
	InitPhys();
	
	PostInitPhys();				// 创建物理对象之后，初始化对象的非物理属性

	_bInitialized = true;
	return _bInitialized;
}

void PhysGameObject::Term()
{
	// 释放内部new的对象
	_bInitialized = false;

	PrevTermPhys();

	TermPhys();

	if( _Scene != NULL )
	{
		_Scene->RemoveGameObject(this);
		_Scene = NULL;
	}
}

void PhysGameObject::InitPhys()
{
	_PhysActor = NULL;
}

void PhysGameObject::TermPhys()
{
	if ( NULL != _Scene )
	{
		DoTermPhysActor(_Scene->_PhysScene, _PhysActor);
	}
	else if (NULL != _PhysActor)
	{
		gplDebugf(TEXT("Fatal Error! Can not Term PhysActor without PhysScene"));
		NX_ASSERT(0);
	}

	_PhysActor = NULL;
}

void PhysGameObject::DoTermPhysActor(NxScene* Scene, NxActor* Actor)
{
	if( Scene && Actor )
	{
		NxU32 NumShape = Actor->getNbShapes();
		std::vector<NxConvexMesh*> PendingKillMesh;
		for(NxU32 i = 0; i < NumShape; ++i)
		{
			NxConvexMesh* ConvexMesh = NULL;
			NxShape* Shape = Actor->getShapes()[i];
			if( Shape->isConvexMesh() )
			{
				ConvexMesh = &(Shape->isConvexMesh()->getConvexMesh());
			}

			//if( Actor->isDynamic() )
			//{
			//	Actor->releaseShape(*Shape);
			//}

			if( ConvexMesh != NULL )
			{
				PendingKillMesh.push_back(ConvexMesh);
			}
		}
		Scene->releaseActor(*Actor);

		while( PendingKillMesh.size() > 0 )
		{
			NxConvexMesh* ConvexMesh = PendingKillMesh.at(PendingKillMesh.size()-1);
			PendingKillMesh.pop_back();
			if( ConvexMesh->getReferenceCount() == 0 )
			{
				GPhysXSDK->releaseConvexMesh(*ConvexMesh);
			}
		}
	}
}

void PhysGameObject::PostInitPhys()
{
}

void PhysGameObject::PrevTermPhys()
{
	delete _ComponentData;
	_ComponentData = NULL;

	_BehaviorReport = NULL;

	if( _Owner != NULL )
	{
		_Owner->OnLostChild(this);
	}

	while( _Children.size() != 0 )
	{
		_Children[0]->SetOwner(NULL);
	}

	//	取消锁定
	if ( _LockTarget )
		_LockTarget->OnUnlock(this);

	//	通知目标丢失
	while ( !_LockHosts.empty() )
		_LockHosts.front()->OnTargetLost(this);
}

float PhysGameObject::GetGlobalOritationYaw()
{
	if( _PhysActor == NULL )
		return 0.f;

	NxQuat Quaternion = GetGlobalOritationQuat();
	NxVec3 Axis;
	NxReal Angle;
	Quaternion.getAngleAxis(Angle, Axis);
	return Axis.z > 0.f ? Angle : (360.f - Angle);
}

void PhysGameObject::SetNotifyRigidBodyCollision(bool bNotifyRigidBodyCollision)
{
	if( _PhysActor != NULL )
	{
		_PhysActor->setGroup(bNotifyRigidBodyCollision ? GPL_GROUP_NOTIFYCOLLIDE : GPL_GROUP_DEFAULT);
	}
}

void PhysGameObject::SetDisableRBCollisionWithOther(PhysGameObject* Other, bool bDisabled)
{
	if( !Other || this == Other )
		return;

	if( _PhysActor && Other->GetPhysActor() )
	{
		NxScene* Scene = &_PhysActor->getScene();

		NxU32 CurrentFlags = Scene->getActorPairFlags(*_PhysActor, *Other->GetPhysActor());
		NxU32 NewFlags = bDisabled ? (CurrentFlags | NX_IGNORE_PAIR) : (CurrentFlags & ~NX_IGNORE_PAIR);
		Scene->setActorPairFlags(*_PhysActor, *Other->GetPhysActor(), NewFlags);
	}
}

bool PhysGameObject::IsOwnedBy(PhysGameObject* TestOwner)
{
	for(PhysGameObject* Owner = GetOwner(); Owner; Owner = Owner->GetOwner())
	{
		if( Owner == TestOwner )
			return true;
	}
	return false;
}

void PhysGameObject::SetOwner(PhysGameObject* Owner)
{
	if( _Owner != Owner )
	{
		if( Owner != NULL && Owner->IsOwnedBy(this) )
		{
			gplDebugf(TEXT("SetOwner(): 设置新的OWNER失败，因为会导致OWNER LOOP"));
			return;
		}

		PhysGameObject* OldOwner = _Owner;
		if( OldOwner != NULL )
		{ // 通知前任OWNER
			OldOwner->OnLostChild(this);
		}

		_Owner = Owner;

		if( _Owner != NULL )
		{
			_Owner->OnGainChild(this);
		}
	}
}

void PhysGameObject::OnLostChild(PhysGameObject* Child)
{
	std::vector<PhysGameObject*>::iterator It = _Children.begin();
	for(; It != _Children.end(); ++It)
	{
		if( *It == Child )
		{
			_Children.erase(It);
			return;
		}
	}
}

void PhysGameObject::OnGainChild(PhysGameObject* Child)
{
	_Children.push_back(Child);
}

void PhysGameObject::Lock(PhysGameObject* Target)
{
	if ( _LockTarget == Target )
		return;

	gplDebugf(TEXT("PhysGameObject::Lock"));

	if ( _LockTarget != NULL )
		_LockTarget->OnUnlock(this);

	_LockTarget = Target;

	if ( _LockTarget != NULL )
		_LockTarget->OnLock(this);
}

void PhysGameObject::OnTargetLost(PhysGameObject* Target)
{
	gplDebugf(TEXT("Target Lost"));
	Lock(NULL);
}

void PhysGameObject::OnUnlock(PhysGameObject* Host)
{
	if (_LockHosts.empty())
		return;

	PhysGameObjectList::iterator iter = std::find(_LockHosts.begin(), _LockHosts.end(), Host);
	if ( iter != _LockHosts.end() )
	{
		gplDebugf(TEXT("[%d] Release Lock"), Host);
		_LockHosts.erase(iter);
	}
	else
	{
		gplDebugf(TEXT("[%d] Did not Lock"), Host);
	}
}

void PhysGameObject::OnLock(PhysGameObject* Host)
{
	gplDebugf(TEXT("Locked By [%d]"), Host);
	_LockHosts.push_back(Host);
}

int PhysGameObject::GetCheckPointNum(Socket SocketList[VEHICLE_CHECK_POINT_MAX])
{
	memset(SocketList, 0, VEHICLE_CHECK_POINT_MAX*sizeof(int));

	std::vector<Socket> List;
	if( _ComponentData == NULL || !_ComponentData->FindSocket(EST_Socket_CheckPoint, List) )
	{
		gplDebugf(TEXT("PhysGameObject::GetCheckPointNum 没有找到Check Point"));
		return 0;
	}
	int Count = 0;
	for(NxU32 i = 0; i < List.size() && Count < VEHICLE_CHECK_POINT_MAX; ++i)
	{
		SocketList[i] = List[i];
		++Count;
	}
	return Count;
}

int PhysGameObject::GetViewPointNum(Socket SocketList[VEHICLE_VIEW_POINT_MAX])
{
	memset(SocketList, 0, VEHICLE_VIEW_POINT_MAX*sizeof(int));

	std::vector<Socket> List;
	if( _ComponentData == NULL || !_ComponentData->FindSocket(EST_Socket_ViewPoint, List) )
	{
		gplDebugf(TEXT("PhysGameObject::GetViewPointNum 没有找到View Point"));
		return 0;
	}
	int Count = 0;
	for(NxU32 i = 0; i < List.size() && Count < VEHICLE_VIEW_POINT_MAX; ++i)
	{
		SocketList[i] = List[i];
		++Count;
	}
	return Count;
}

int PhysGameObject::GetBurstPointNum( Socket SocketList[VEHICLE_BURST_POINT_MAX] )
{
	memset(SocketList, 0, VEHICLE_BURST_POINT_MAX*sizeof(int));

	std::vector<Socket> List;
	if( _ComponentData == NULL || !_ComponentData->FindSocket(EST_Socket_BurstPoint, List) )
	{
		gplDebugf(TEXT("PhysGameObject::GetBurstPointNum 没有找到Burst Point"));
		return 0;
	}
	int Count = 0;
	for(NxU32 i = 0; i < List.size() && Count < VEHICLE_BURST_POINT_MAX; ++i)
	{
		SocketList[i] = List[i];
		++Count;
	}
	return Count;
}

void PhysGameObject::GetBlockedObjectList(PhysGameObject* Other, const Socket& ViewPoint, const Socket& CheckPoint, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE])
{
	// init return value
	BlockedObjCnt = 0;

	NxVec3 Start = GetGlobalPos() * ViewPoint._LocalPose.t;
	NxVec3 End = Other->GetGlobalPos() * CheckPoint._LocalPose.t;

	// obtain all blocked object
	NxU32 groups = (1 << GPL_SHAPE_GROUP_TERRAIN | 1 << GPL_SHAPE_GROUP_STATICOBJECT | 1 << GPL_SHAPE_GROUP_TRIGGER_VOLUME);
	
	_Scene->GetAllObjectList(Start, End, groups, BlockedObjCnt, OutBlockedObjList);
}

void PhysGameObject::GetBlockedObjectList(PhysGameObject* Other, bool CheckTerrain, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE])
{
	// init return value
	BlockedObjCnt = 0;

	NxVec3 Start = GetGlobalPosition();
	NxVec3 End = Other->GetGlobalPosition();

	// obtain all blocked object
	NxU32 groups = 0;
	ENABLE_STATE(groups, GPL_SHAPE_GROUP_TRIGGER_VOLUME);

	if (CheckTerrain)
	{
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_TERRAIN);
		ENABLE_STATE(groups, GPL_SHAPE_GROUP_STATICOBJECT);
	}

	_Scene->GetAllObjectList(Start, End, groups, BlockedObjCnt, OutBlockedObjList);
}

}