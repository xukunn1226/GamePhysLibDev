#include "..\Inc\PhysXSupport.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

namespace GPL
{ // begin namespace
	//modify by yeleiyu@icee.cn for xml encrypt and decrypt
	bool LoadLibConfig(const std::wstring WorkingPath,bool EnableReadEncryptedXML)
	{		
		GWorkingPath = WorkingPath;
		//add by yeleiyu@icee.cn for map encrypt
		GEnableReadEncryptedXML =EnableReadEncryptedXML;
		if( !LoadPhysModels( TEXT("PhysModels.xml"), GPhysModelDescList ,EnableReadEncryptedXML) )
		{
			return false;
		}
		if( !LoadComponents( TEXT("PhysComponents.xml"), GComponentDescList,EnableReadEncryptedXML ) )
		{
			return false;
		}

		if(!LoadPhysDataConfig(TEXT("PhysGameSetting.xml"),GPhysGameSetting,EnableReadEncryptedXML))
		{
			return false;
		}

		IndexComponents(GComponentDescList, GComponentDescTable);

		GGaussianUseLlast = false;
	
		return true;
	}

	bool LoadMapConfig(const std::wstring MapName)
	{
		bool bRet;
		std::vector<ComponentDesc*>* CompDescList = NULL;
		std::vector<PhysModelDesc>* ModelDescList = NULL;
		bRet = LoadScenePhys(MapName, &CompDescList, &ModelDescList);
		if( !bRet )
		{
			gplDebugf(TEXT("LoadMapConfig:[%s]失败"), MapName.c_str());
			return false;
		}
		return true;
	}

	void ClientInitGamePhysLib(NxCookingInterface* InCooking, NxUserAllocator* InAllocator, PhysUserOutputStream* Stream, bool bEnableDebugger)
	{
		GRunMode = ERM_Client;
		GOutputStream = Stream;

		NOutputStream* MyStream = new NOutputStream();
		GPhysXSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, InAllocator, MyStream);
		NX_ASSERT(GPhysXSDK);

		GPhysXSDK->setParameter(NX_SKIN_WIDTH, GPhysGameSetting._GPL_SKIN_WIDTH);
		GPhysXSDK->setParameter(NX_ADAPTIVE_FORCE, 1);		// 默认打开，可以加速物理计算收敛过程，提高性能，但有时会造成unwanted physical artifacts
		if (GPhysXSDK->getFoundationSDK().getRemoteDebugger() && bEnableDebugger)
			GPhysXSDK->getFoundationSDK().getRemoteDebugger()->connect( "localhost",  NX_DBG_DEFAULT_PORT, NX_DBG_EVENTMASK_EVERYTHING);

		if( InCooking != NULL )
		{
			GCooking = InCooking;
		}
		else
		{
			GCooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
		}
		GCooking->NxInitCooking(InAllocator, MyStream);

		gplDebugf(TEXT("[GPL] Init PHYS SDK! RunMode[%d]"), GRunMode);
	}

	void ServerInitGamePhysLib(NxUserAllocator* InAllocator, PhysUserOutputStream* Stream)
	{
		GRunMode = ERM_Server;
		GOutputStream = Stream;

		NOutputStream* MyStream = new NOutputStream();
		GPhysXSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, InAllocator, MyStream);
		NX_ASSERT(GPhysXSDK);

		GPhysXSDK->setParameter(NX_SKIN_WIDTH, GPhysGameSetting._GPL_SKIN_WIDTH);
		GPhysXSDK->setParameter(NX_ADAPTIVE_FORCE, 1);		// 默认打开，可以加速物理计算收敛过程，提高性能，但有时会造成unwanted physical artifacts

		GCooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
		GCooking->NxInitCooking(InAllocator, MyStream);
	
		gplDebugf(TEXT("[GPL] Init PHYS SDK! RunMode[%d]"), GRunMode);
	}

	void DestroyGamePhysLib()
	{
		// 客户端由UE3释放，服务端才需释放SDK,COOKING
#ifdef _SERVER_RUNTIME
		if( GCooking )
		{
			GCooking->NxCloseCooking();
		}
		if( GPhysXSDK )
		{
			NxReleasePhysicsSDK(GPhysXSDK);
		}
#endif // _SERVER_RUNTIME

		GCooking = NULL;
		GPhysXSDK = NULL;

		StaticExit();

		gplDebugf(TEXT("[GPL] Destroy PHYS SDK!"));
	}

	void StaticExit()
	{
		std::vector<PhysModelDesc>::iterator it = GPhysModelDescList.begin();
		for(; it != GPhysModelDescList.end(); ++it)
		{
			it->Cleanup();
		}
		GPhysModelDescList.clear();

		GComponentDescTable.clear();
		ComponentDescList::iterator it1 = GComponentDescList.begin();
		for(; it1 != GComponentDescList.end(); ++it1)
		{
			SAFE_DELETE(*it1);
		}
		GComponentDescList.clear();
	}

	void PhysGameScene::AddGameObject(PhysGameObject* GameObject)
	{
#ifdef _DEBUG
		if (!_PhysGameObjectList.empty())
		{
			std::vector<PhysGameObject*>::iterator it;
			it = std::find(_PhysGameObjectList.begin(), _PhysGameObjectList.end(), GameObject);
			if ( it != _PhysGameObjectList.end() )
			{
				gplDebugf(TEXT("[GPL] AddGameObject: GameObject重复加入"));
				return;
			}
		}
#endif
		_PhysGameObjectList.push_back(GameObject);

		//gplDebugf(TEXT("[GPL] AddGameObject NumGameObject[%d]"), _PhysGameObjectList.size());
	}

	void PhysGameScene::RemoveGameObject(PhysGameObject* GameObject)
	{
		if (_PhysGameObjectList.empty())
		{
			gplDebugf(TEXT("[GPL] Can't Remove PhysGameObject[%d] from an EmptyList"), GameObject);
			return;
		}

		std::vector<PhysGameObject*>::iterator it;
		it = std::find(_PhysGameObjectList.begin(), _PhysGameObjectList.end(), GameObject);
		if ( it != _PhysGameObjectList.end() )
		{
			_PhysGameObjectList.erase(it);
		}
		//gplDebugf(TEXT("[GPL] RemoveGameObject NumGameObject[%d]"), _PhysGameObjectList.size());
	}

	void PhysGameScene::myNxContactReport::onContactNotify(NxContactPair& pair, NxU32 events)
	{
		if( pair.isDeletedActor[0] || pair.isDeletedActor[1] )
		{
			gplDebugf(TEXT("[GPL]: onContactNotify: Actors %d %d has been deleted."), (INT)pair.isDeletedActor[0], (INT)pair.isDeletedActor[1]);
			return;
		}

		NxActor* Actor0 = pair.actors[0];
		NxActor* Actor1 = pair.actors[1];

		NxScene* Scene = &Actor0->getScene();
		PhysGameScene* PhysScene = (PhysGameScene*)Scene->userData;

		CollisionNotifyInfo NotifyInfo;
		NotifyInfo._bValidActor0 = (Actor0->getGroup() >= GPL_GROUP_NOTIFYCOLLIDE);
		NotifyInfo._bValidActor1 = (Actor1->getGroup() >= GPL_GROUP_NOTIFYCOLLIDE);
		NotifyInfo._Actor0 = (PhysGameObject*)Actor0->userData;
		NotifyInfo._Actor1 = (PhysGameObject*)Actor1->userData;
		NotifyInfo._CollisionData._TotalNormalForceVector = pair.sumNormalForce;
		NotifyInfo._CollisionData._TotalFrictionForceVector = pair.sumFrictionForce;
	
		NxContactStreamIterator It(pair.stream);
		// user can call getNumPairs() here 
		while(It.goNextPair())
		{
			// user can also call getShape() and getNumPatches() here
			while(It.goNextPatch())
			{
				//user can also call getPatchNormal() and getNumPoints() here
				NxVec3 ContactNormal = It.getPatchNormal();

				while(It.goNextPoint())
				{
					//user can also call getPoint() and getSeparation() here
					NxVec3 ContactPos = It.getPoint();

					CollisionContactInfo ContactInfo;
					ContactInfo._Event = NxEventTranslateCollisionEvent(events);
					ContactInfo._ContactNormal = ContactNormal;
					ContactInfo._ContactPosition = ContactPos;
					ContactInfo._ContactVelocity[0] = Actor0->getPointVelocity(ContactPos);
					ContactInfo._ContactVelocity[1] = Actor1->getPointVelocity(ContactPos);

					NotifyInfo._CollisionData._ContactInfos.push_back(ContactInfo);
				}
			}
		}

		PhysScene->_PendingCollisionNotifies.push_back(NotifyInfo);
	}

	void PhysGameScene::myUserTriggerReport::onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		NxActor* triggerActor = &triggerShape.getActor();
		NxActor* otherActor = &otherShape.getActor();

		PhysGameObject* triggerGameObject = (PhysGameObject*)triggerActor->userData;
		PhysGameObject* otherGameObject = (PhysGameObject*)otherActor->userData;

		if (!IsKind(RangeTrigger, triggerGameObject))
		{
			//	一般Trigger事件处理
			if( triggerGameObject && otherGameObject )
			{
				if( status & NX_TRIGGER_ON_ENTER )
				{
					if( triggerGameObject->GetBehaviorReport() != NULL )
					{
						triggerGameObject->GetBehaviorReport()->EncroachedBy(triggerGameObject, otherGameObject);
					}
					if( otherGameObject->GetBehaviorReport() != NULL )
					{
						otherGameObject->GetBehaviorReport()->EncroachingOn(otherGameObject, triggerGameObject);
					}
				}
				if( status & NX_TRIGGER_ON_LEAVE )
				{
					if( triggerGameObject->GetBehaviorReport() != NULL )
					{
						triggerGameObject->GetBehaviorReport()->SeparatedBy(triggerGameObject, otherGameObject);
					}
					if( otherGameObject->GetBehaviorReport() != NULL )
					{
						otherGameObject->GetBehaviorReport()->SeparatingFrom(otherGameObject, triggerGameObject);
					}
				}
			}
		}
		else 
		{
			//	RangeTrigger事件处理
			PhysAircraft* pAircraft = DynamicCast(PhysAircraft, triggerGameObject->GetOwner());
			if (NULL != pAircraft)
			{
#ifdef _CLIENT_RUNTIME
				//	拦截范围和追击范围检测只在客户端执行
				if ( !pAircraft->_LocalPlayer )
					gplDebugf(TEXT("Warning!Not Triggered By LocalPlayer!"));

				PhysAircraft* pEventAircraft = DynamicCast(PhysAircraft, otherGameObject);
				if ( !pEventAircraft )
					return;

				BehaviorReport* pBehaviorReport = pAircraft->GetBehaviorReport();
				if ( !pBehaviorReport )
					return;

				if (triggerGameObject == pAircraft->_InterceptTrigger)
				{
					if ( (status & NX_TRIGGER_ON_ENTER) != 0 )
						pBehaviorReport->OnInterceptAreaEvent(pAircraft, pEventAircraft, true);
					else if ( (status & NX_TRIGGER_ON_LEAVE) != 0 )
						pBehaviorReport->OnInterceptAreaEvent(pAircraft, pEventAircraft, false);
				} 
				else if (triggerGameObject == pAircraft->_ChaseTrigger)
				{
					if ( (status & NX_TRIGGER_ON_ENTER) != 0 )
						pBehaviorReport->OnChaseAreaEvent(pAircraft, pEventAircraft, true);
					else if ( (status & NX_TRIGGER_ON_LEAVE) != 0 )
						pBehaviorReport->OnChaseAreaEvent(pAircraft, pEventAircraft, false);
				}
#endif // _CLIENT_RUNTIME
			}
			else
			{
				//	照明弹及其他区域事件处理
				if( triggerGameObject && otherGameObject )
				{
					if( status & NX_TRIGGER_ON_ENTER )
					{
						if( triggerGameObject->GetBehaviorReport() != NULL )
						{
							triggerGameObject->GetBehaviorReport()->EncroachedBy(triggerGameObject, otherGameObject);
						}
						if( otherGameObject->GetBehaviorReport() != NULL )
						{
							otherGameObject->GetBehaviorReport()->EncroachingOn(otherGameObject, triggerGameObject);
						}
					}
					if( status & NX_TRIGGER_ON_LEAVE )
					{
						if( triggerGameObject->GetBehaviorReport() != NULL )
						{
							triggerGameObject->GetBehaviorReport()->SeparatedBy(triggerGameObject, otherGameObject);
						}
						if( otherGameObject->GetBehaviorReport() != NULL )
						{
							otherGameObject->GetBehaviorReport()->SeparatingFrom(otherGameObject, triggerGameObject);
						}
					}
				}
			}
		}
	}

	PhysGameScene* CreatePhysScene()
	{
		PhysGameScene* GameScene = new PhysGameScene();

		GameScene->_SceneBounds.min = NxVec3(
			-2 * GPhysGameSetting._GPL_PhysWorld_HalfLength, 
			-2 * GPhysGameSetting._GPL_PhysWorld_HalfWidth, 
			-GPhysGameSetting._GPL_PhysWorld_HalfHeight)
			* G2PScale;
		GameScene->_SceneBounds.max = NxVec3(
			2 * GPhysGameSetting._GPL_PhysWorld_HalfLength,
			2 * GPhysGameSetting._GPL_PhysWorld_HalfWidth, 
			GPhysGameSetting._GPL_PhysWorld_HalfHeight)  
			* G2PScale;

		NxSceneDesc SceneDesc;
		SceneDesc.gravity = NxVec3(0, 0, GPhysGameSetting._GPL_PHYS_GRAVITY) * G2PScale;

#ifdef _CLIENT_RUNTIME
		SceneDesc.maxIter = 2;
		SceneDesc.timeStepMethod = NX_TIMESTEP_FIXED;
		SceneDesc.maxTimestep = GPhysGameSetting._GPL_PHYS_CLIENT_MAX_TIMESTEP;
		SceneDesc.flags |= NX_SF_SIMULATE_SEPARATE_THREAD;	// By default the SDK runs the physics on a separate thread to the user thread
		//modify by yeleiyu@icee.cn 物理库性能测试
		SceneDesc.flags |= NX_SF_ENABLE_MULTITHREAD;
		SceneDesc.threadMask=0xfffffffe;
		SceneDesc.internalThreadCount = 2;
#endif // _CLIENT_RUNTIME

#ifdef _SERVER_RUNTIME
		SceneDesc.maxIter = 1;
		SceneDesc.timeStepMethod = NX_TIMESTEP_FIXED;
		SceneDesc.maxTimestep = GPhysGameSetting._GPL_PHYS_SERVER_MAX_TIMESTEP;
		SceneDesc.flags &= ~NX_SF_SIMULATE_SEPARATE_THREAD;
		//SceneDesc.flags |= NX_SF_ENABLE_MULTITHREAD;
		SceneDesc.internalThreadCount = 2;
#endif // _SERVER_RUNTIME

		SceneDesc.maxBounds = &GameScene->_SceneBounds;		// 场景包围盒大小，加速碰撞检测
		SceneDesc.boundsPlanes = true;
		SceneDesc.backgroundThreadCount = 1; // backgroundThreadCount must be at least 1 for the asynchronous mode to work

		SceneDesc.dynamicStructure = NX_PRUNING_DYNAMIC_AABB_TREE;
		//add by yeleiyu@icee.cn 物理库性能测试
		SceneDesc.dynamicTreeRebuildRateHint = 100;

		SceneDesc.userContactReport = &GameScene->_ContactReport;
		SceneDesc.userTriggerReport = &GameScene->_TriggerReport;

		NxScene* Scene = NULL;
		NxU32 ValidCode = SceneDesc.checkValid();
		if( ValidCode == 0 )
		{
			Scene = GPhysXSDK->createScene(SceneDesc);
		}
		NX_ASSERT(Scene);

		NxSceneQueryDesc QueryDesc;
		QueryDesc.executeMode = NX_SQE_ASYNCHRONOUS;
		QueryDesc.report = &GameScene->_QueryReport;
		GameScene->_SceneQuery = Scene->createSceneQuery(QueryDesc);

		// 设置actor.group
		Scene->setActorGroupPairFlags(GPL_GROUP_DEFAULT, GPL_GROUP_DEFAULT, NX_IGNORE_PAIR);
		// 基于性能考虑，仅响应Start touch
		Scene->setActorGroupPairFlags(GPL_GROUP_DEFAULT, GPL_GROUP_NOTIFYCOLLIDE, NX_NOTIFY_FORCES | NX_NOTIFY_ON_START_TOUCH);
		Scene->setActorGroupPairFlags(GPL_GROUP_NOTIFYCOLLIDE, GPL_GROUP_NOTIFYCOLLIDE,	NX_NOTIFY_FORCES | NX_NOTIFY_ON_START_TOUCH);

		// 设置shape.group
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_DEFAULT, GPL_SHAPE_GROUP_DEFAULT, true);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_OCEAN, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_OCEAN, GPL_SHAPE_GROUP_OCEAN, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TERRAIN, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TERRAIN, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TERRAIN, GPL_SHAPE_GROUP_TERRAIN, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_STATICOBJECT, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_STATICOBJECT, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_STATICOBJECT, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_STATICOBJECT, GPL_SHAPE_GROUP_STATICOBJECT, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_DEFAULT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_TERRAIN, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_STATICOBJECT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_BLOCKING_VOLUME, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_SHIP, GPL_SHAPE_GROUP_VEHICLE_SHIP, true);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TRIGGER_VOLUME, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, false);
		
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT, GPL_SHAPE_GROUP_AIRCRAFT, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_TERRAIN, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_AIRCRAFT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SINK_VEHICLE, GPL_SHAPE_GROUP_SINK_VEHICLE, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_TERRAIN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_SINK_VEHICLE, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, false);

		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_DEFAULT, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_OCEAN, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_TERRAIN, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_STATICOBJECT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_BLOCKING_VOLUME_IGNORE_CAMERA, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_VEHICLE_SHIP, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_VEHICLE_TRIGGER_VOLUME, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT, true);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_AIRCRAFT_TRIGGER_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_SINK_VEHICLE, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_SUBMARINE_SURFACE_VOLUME, false);
		Scene->setGroupCollisionFlag(GPL_SHAPE_GROUP_FLARE_VOLUME, GPL_SHAPE_GROUP_FLARE_VOLUME, false);

		Scene->userData = GameScene;		//
		GameScene->_PhysScene = Scene;

		// 设置默认NxMaterial,此场景内的所有NxShape默认使用此NxMaterial
		NxMaterial* DefaultMaterial = Scene->getMaterialFromIndex(0);		
		DefaultMaterial->setRestitution(0.0f);				// 0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce
		DefaultMaterial->setStaticFriction(0.5f);			// static friction can be higher than 1
		DefaultMaterial->setDynamicFriction(0.5f);			// dynamic friction should not be higher than 1
		DefaultMaterial->setDynamicFrictionV(0.0f);
		DefaultMaterial->setStaticFrictionV(0.0f);
		DefaultMaterial->setFrictionCombineMode(NX_CM_MULTIPLY);
		DefaultMaterial->setRestitutionCombineMode(NX_CM_MAX);

		gplDebugf(TEXT("[GPL] 创建场景成功"));

		return GameScene;
	}

	void DestroyPhysScene(PhysGameScene* Scene)
	{
		if ( Scene && Scene->_PhysScene )
		{
			// release scene query
			if( Scene->_SceneQuery )
			{
				Scene->_PhysScene->releaseSceneQuery(*Scene->_SceneQuery);
			}

			// release all nxactor
			while( Scene->_PhysGameObjectList.size() != 0 )
			{
				DestroyPhysGameObject(Scene, Scene->_PhysGameObjectList.front());
			}

			// release scene
			GPhysXSDK->releaseScene(*(Scene->_PhysScene));
			delete Scene;
		}

#ifdef _CLIENT_RUNTIME
		gplDebugf(TEXT("[GPL] 清空地图缓存..."));
		std::map<std::wstring, ScenePhysDesc*>::iterator iter;
		for (iter = GScenePhysDescList.begin(); iter != GScenePhysDescList.end(); iter++)
		{
			SAFE_DELETE(iter->second);
		}
		GScenePhysDescList.clear();
#endif // _CLIENT_RUNTIME
		
		_TraceProjectile = NULL;

		gplDebugf(TEXT("[GPL] 销毁场景"));
	}

	bool FetchPhysSceneResults(PhysGameScene* Scene)
	{
		if( Scene && Scene->_PhysScene )
		{
			return Scene->_PhysScene->fetchResults(NX_ALL_FINISHED, true);
		}
		return false;
	}

	void TickPhysScene(PhysGameScene* Scene, float DeltaTime)
	{
		if( Scene && Scene->_PhysScene )
		{
			Scene->_PhysScene->simulate(DeltaTime);
		}
	}

	void TickAsyncWork(PhysGameScene* Scene, float DeltaTime)
	{
		if( Scene && Scene->_PhysGameObjectList.size() > 0 )
		{
			NxU32 NumGameObject = NxU32(Scene->_PhysGameObjectList.size());
			for(NxU32 i = 0; i < NumGameObject; ++i)
			{
				if( GRunMode == ERM_Client && ExactlyIs(PhysVehicle, Scene->_PhysGameObjectList[i]) )
				{ // 客户端模式，舰船物理对象Tick由显示对象Tick负责调用
					continue;
				}
				Scene->_PhysGameObjectList[i]->Tick(DeltaTime);
			}
		}
	}

	void DispatchCollisionNotifies(PhysGameScene* Scene)
	{
		for(int i = 0; i < (int)Scene->_PendingCollisionNotifies.size(); ++i)
		{
			CollisionNotifyInfo& NotifyInfo = Scene->_PendingCollisionNotifies.at(i);
			if( NotifyInfo._CollisionData._ContactInfos.size() > 0 )
			{
				if( NotifyInfo._bValidActor0 && NotifyInfo._Actor0 )
				{
					NotifyInfo._Actor0->OnCollisionDetection(NotifyInfo._Actor1, NotifyInfo._CollisionData);

					//	下沉中的舰船在碰撞发生后停止侧倾
					PhysVehicle* pVehicle = DynamicCast(PhysVehicle, NotifyInfo._Actor0);
					if ( pVehicle && pVehicle->IsSinking() )
					{
						VehicleSimDeath* pSimulator = DynamicCast(VehicleSimDeath, pVehicle->GetVehicleSimulator());
						if ( pSimulator )
							pSimulator->ResetTorque();
					}
				}

				if( NotifyInfo._bValidActor1 && NotifyInfo._Actor1 )
				{
					NotifyInfo._CollisionData.SwapContactOrder();
					NotifyInfo._Actor1->OnCollisionDetection(NotifyInfo._Actor0, NotifyInfo._CollisionData);

					//	下沉中的舰船在碰撞发生后停止侧倾
					PhysVehicle* pVehicle = DynamicCast(PhysVehicle, NotifyInfo._Actor1);
					if ( pVehicle && pVehicle->IsSinking() )
					{
						VehicleSimDeath* pSimulator = DynamicCast(VehicleSimDeath, pVehicle->GetVehicleSimulator());
						if ( pSimulator )
							pSimulator->ResetTorque();
					}
				}
			}
		}
		Scene->_PendingCollisionNotifies.clear();
	}

	void TickPhysGameScene(PhysGameScene* Scene, float DeltaTime)
	{
		TickPhysScene(Scene, DeltaTime);

		FetchPhysSceneResults(Scene);

		TickAsyncWork(Scene, DeltaTime);

		DispatchCollisionNotifies(Scene);
	}

	ComponentDesc* GetPhysComponentDesc(int CompId)
	{
		ComponentDescMap::iterator mapIt = GComponentDescTable.find(CompId);
		if (mapIt != GComponentDescTable.end())
		{
			return mapIt->second;
		}
		else
		{
			//gplDebugf(TEXT("Unable to find ComponentDesc from GComponentDescTable"));
			ComponentDescList::iterator it;
			for ( it = GComponentDescList.begin(); it != GComponentDescList.end(); it++ )
			{
				ComponentDesc* CompDesc = *it;
				if (NULL != CompDesc && CompDesc->_CompId == CompId)
					return CompDesc;
			}
		}

		return NULL;
	}

	bool PhysGameScene::InitScene(std::wstring mapName)
	{
		if (_PhysScene == NULL)
		{
			gplDebugf(TEXT("[GPL] InitScene Error: _PhysScene == NULL!")); 
			return false; 
		}

		bool rslt;
		std::vector<ComponentDesc*>* CompDescList = NULL;
		std::vector<PhysModelDesc>* ModelDescList = NULL;
		rslt = LoadScenePhys(mapName, &CompDescList, &ModelDescList);
		if(!rslt || CompDescList == NULL)
		{
			gplDebugf(TEXT("InitScene:[%s]失败"), mapName.c_str());
			return false;
		}

		//PROCESS_MEMORY_COUNTERS pm3, pm4;
		//GetProcessMemoryInfo(GetCurrentProcess(), &pm3, sizeof(pm3));

		for(std::vector<ComponentDesc*>::iterator it = CompDescList->begin(); it != CompDescList->end(); ++it)
		{
			ComponentDesc* CompDesc = *it;
			ComponentData* CompData = (ComponentData*)ConstructComponentData(CompDesc);
			
			//gplDebugf(TEXT("----------Map:[%s], create scene phys object _CompId[%d] _PhysId:[%s]"), mapName.c_str(), CompDesc->_CompId, CompDesc->_PhysId.c_str());
			//
			//PROCESS_MEMORY_COUNTERS pm1, pm2;
			//GetProcessMemoryInfo(GetCurrentProcess(), &pm1, sizeof(pm1));
			
			PhysGameObject* Object = CreatePhysGameObject(this, CompDesc->_ObjectType, CompData);

			//GetProcessMemoryInfo(GetCurrentProcess(), &pm2, sizeof(pm2));
			//gplDebugf(TEXT("Before=%d After=%d Alloc=%d"), pm1.PagefileUsage, pm2.PagefileUsage, pm2.PagefileUsage - pm1.PagefileUsage);
			
			if( Object != NULL )
			{
				if( _LoadingReport != NULL )
				{
					_LoadingReport->OnFinishedLoadPhysGameObject(Object);
				}
			}
		}

		//GetProcessMemoryInfo(GetCurrentProcess(), &pm4, sizeof(pm4));
		//gplDebugf(TEXT("Before=%d After=%d Alloc=%d"), pm3.PagefileUsage, pm4.PagefileUsage, pm4.PagefileUsage - pm3.PagefileUsage);

		gplDebugf(TEXT("成功加载场景物理模型！！"));
		return true;
	}

	void PhysGameScene::TermScene(const std::wstring& mapName)
	{
		ReleaseScenePhys(mapName);
	}
	
	PhysGameScene::PhysGameScene()
		: _PhysScene(NULL), _SceneQuery(NULL), _LoadingReport(NULL)
	{
	}

	void PhysGameScene::GetPlayerStart(int& ListCnt, PhysPlayerStart* StartList[PLAYER_START_SIZE_MAX])
	{
		if (_PhysScene == NULL)
		{ 
			gplDebugf(TEXT("[GPL] GetPlayerStart InitScene Error: _PhysScene == NULL!")); 
			return; 
		}
		int Index = 0;
		memset(StartList, 0, PLAYER_START_SIZE_MAX*sizeof(int));
		for(std::vector<PhysGameObject*>::iterator it = _PhysGameObjectList.begin(); it != _PhysGameObjectList.end(); ++it)
		{
			if( Index >= PLAYER_START_SIZE_MAX )
				break;

			PhysGameObject* PlayerStart = *it;
			if( ExactlyIs(PhysPlayerStart, PlayerStart) )
			{
				StartList[Index++] = (PhysPlayerStart*)PlayerStart;
			}
		}
		ListCnt = Index;
	}

	void PhysGameScene::GetPlayerRespawn(int& ListCnt, PhysPlayerRespawn* RespawnList[PLAYER_RESPAWN_SIZE_MAX])
	{
		if (_PhysScene == NULL)
		{ 
			gplDebugf(TEXT("[GPL] GetPlayerRespawn InitScene Error: _PhysScene == NULL!")); 
			return; 
		}
		int Index = 0;
		memset(RespawnList, 0, PLAYER_RESPAWN_SIZE_MAX*sizeof(int));
		for(std::vector<PhysGameObject*>::iterator it = _PhysGameObjectList.begin(); it != _PhysGameObjectList.end(); ++it)
		{
			if( Index >= PLAYER_RESPAWN_SIZE_MAX )
				break;

			PhysGameObject* PlayerRespawn = *it;
			if( ExactlyIs(PhysPlayerRespawn, PlayerRespawn) )
			{
				RespawnList[Index++] = (PhysPlayerRespawn*)PlayerRespawn;
			}
		}
		ListCnt = Index;
	}

	void PhysGameScene::SetUserLoadingReport(UserLoadingReport* Report)
	{
		_LoadingReport = Report;
	}

	void PhysGameScene::ForceProjectileSelfDestruction(PhysProjectile* Projectile)
	{
		if( Projectile != NULL )
		{
			Projectile->ForceSelfDestruction();
		}
	}

	int PhysGameScene::GetAllObjectList(NxVec3 Start, NxVec3 End, int groups, int& iObjectNum, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE])
	{
		static int iObject;
		static PhysGameObject** pObjectList;

		class myRaycastReport : public NxUserRaycastReport
		{
			virtual bool onHit(const NxRaycastHit& hit)
			{
				NxActor& hitActor = hit.shape->getActor();
				PhysGameObject* PhysActor = (PhysGameObject*)hitActor.userData;
				if( PhysActor != NULL )
				{
					for (int i = 0; i < iObject; i++)
					{
						if (PhysActor == pObjectList[i])
						{
							return true;
						}
					}
					pObjectList[iObject++] = PhysActor;
				}
				if (iObject == MAX_BLOCK_OBJECT_PER_LINE)
				{
					return false;
				}
				return true;
			}
		}MyReport;
			
		NxRay WorldRay;
		WorldRay.orig = Start;
		WorldRay.dir = (End - Start);
		WorldRay.dir.normalize();

		iObject = 0;
		pObjectList = OutBlockedObjList;

		_PhysScene->raycastAllShapes(
			WorldRay,
			MyReport,
			NX_ALL_SHAPES, 
			groups,
			End.distance(Start), 
			NX_RAYCAST_SHAPE, 0 );

		iObjectNum = iObject;
		return iObjectNum;
	}

} // end namespace