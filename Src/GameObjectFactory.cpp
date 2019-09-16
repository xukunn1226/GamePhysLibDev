#include "..\Inc\PhysXSupport.h"

namespace GPL
{

	void ReleaseShapeDescList(std::vector<NxShapeDesc*>& CachedShapeDescList)
	{
		std::vector<NxShapeDesc*>::iterator it = CachedShapeDescList.begin();
		for(; it != CachedShapeDescList.end(); ++it)
		{
			delete *it;
		}
		CachedShapeDescList.clear();
	}

	// ��������������NxShapeDesc
	NxShapeDesc* CreateComponentShape(const ShapeDesc* const CustomShapeDesc, NxVec3 shapeScale)
	{
		if( CustomShapeDesc == NULL )
		{
			return NULL;
		}

		NxShapeDesc* XShapeDesc = NULL;
		switch(CustomShapeDesc->_ShapeType)
		{
		case EST_SHAPE_BOX:
			{
				XShapeDesc = new NxBoxShapeDesc();
				BoxShapeDesc* BShapeDesc = (BoxShapeDesc*)CustomShapeDesc;

				((NxBoxShapeDesc*)XShapeDesc)->dimensions = 0.5f * BShapeDesc->_Dimensions * G2PScale * shapeScale.x;
				NxMat34 LocalPose = BShapeDesc->_LocalPose;
				LocalPose.t *= G2PScale;
				((NxBoxShapeDesc*)XShapeDesc)->localPose = LocalPose;
			}
			break;
		case EST_SHAPE_CONVEX:
			{
				ConvexShapeDesc* desc = (ConvexShapeDesc*)CustomShapeDesc;
				if( !desc->IsValidCachedData() )
				{ // create cached mesh data
					NxConvexMeshDesc coneMeshDesc;
					coneMeshDesc.numVertices = NxU32(desc->_Verts.size()); //Number of vertices in mesh.
					coneMeshDesc.numTriangles = NxU32(desc->_Indeices.size()); //Number of triangles(3 indices per triangle).
					coneMeshDesc.pointStrideBytes = sizeof(NxVec3);//Number of bytes from one vertex to the next.
					int* faces = new int[desc->_Indeices.size() * 3];
					NxVec3* verts = new NxVec3[desc->_Verts.size()];
					for( unsigned int i = 0; i < desc->_Indeices.size(); i++ )
					{
						faces[i*3] = (int)desc->_Indeices[i].x;
						faces[i*3+1] = (int)desc->_Indeices[i].y;
						faces[i*3+2] = (int)desc->_Indeices[i].z;
					}
					for( unsigned int i = 0; i < desc->_Verts.size(); i++ )
					{
						verts[i] = desc->_Verts[i] * G2PScale;
						verts[i].x *= shapeScale.x;
						verts[i].y *= shapeScale.y;
						verts[i].z *= shapeScale.z;
					}
					
					coneMeshDesc.triangleStrideBytes = 3*sizeof(int);//Number of bytes from one triangle to the next.
					coneMeshDesc.points = verts;
					coneMeshDesc.triangles = faces;
					coneMeshDesc.flags = 0;//NX_CF_COMPUTE_CONVEX ;

					NxU32 ValidCode = coneMeshDesc.checkValid();
					if(ValidCode == 0 && GCooking->NxCookConvexMesh(coneMeshDesc, desc->_CachedMeshData))
					{
						delete[] verts;
						delete[] faces;
					}
					else
					{
						coneMeshDesc.flags = NX_CF_COMPUTE_CONVEX;
						if (GCooking->NxCookConvexMesh(coneMeshDesc, desc->_CachedMeshData))
						{
							gplDebugf(TEXT("Recooking NxCookConvexMesh with NX_CF_COMPUTE_CONVEX succeed"));
							delete[] verts;
							delete[] faces;
						}
						else
						{
							gplDebugf(TEXT("ERROR: Cooking NxCookConvexMesh!!!!!"));
							delete[] verts;
							delete[] faces;
							return NULL;
						}
					}
				}

				XShapeDesc = new NxConvexShapeDesc();
				NxMat34 LocalPose = CustomShapeDesc->_LocalPose;
				LocalPose.t *= G2PScale;
				((NxConvexShapeDesc*)(XShapeDesc))->localPose = LocalPose;
				((NxConvexShapeDesc*)(XShapeDesc))->meshData = GPhysXSDK->createConvexMesh(MemoryReadBuffer(desc->_CachedMeshData.data));
			}
			break;
		case EST_SHAPE_SPHERE:
			{
				XShapeDesc = new NxSphereShapeDesc();
				SphereShapeDesc* ShapeDesc = (SphereShapeDesc*)CustomShapeDesc;

				((NxSphereShapeDesc*)XShapeDesc)->radius = ShapeDesc->_Radius * G2PScale * shapeScale.x;
				NxMat34 LocalPose = ShapeDesc->_LocalPose;
				LocalPose.t *= G2PScale;
				((NxSphereShapeDesc*)XShapeDesc)->localPose = LocalPose;
			}
			break;
		case EST_SHAPE_PLANE:
			break;
		case EST_SHAPE_CAPSULE:
			{
				XShapeDesc = new NxCapsuleShapeDesc();
				CapsuleShapeDesc* ShapeDesc = (CapsuleShapeDesc*)CustomShapeDesc;
				((NxCapsuleShapeDesc*)XShapeDesc)->height = ShapeDesc->_Height * G2PScale * shapeScale.x;
				((NxCapsuleShapeDesc*)XShapeDesc)->radius = ShapeDesc->_Radius * G2PScale * shapeScale.x;
				((NxCapsuleShapeDesc*)XShapeDesc)->localPose = ShapeDesc->_LocalPose;
				((NxCapsuleShapeDesc*)XShapeDesc)->localPose.t *= G2PScale;
			}
			break;
		case EST_SHAPE_HEIGHTFIELD:
			{
				XShapeDesc = new NxHeightFieldShapeDesc();			
			}
			break;
		}

		return XShapeDesc;
	}

	PhysGameObject* CreatePhysGameObject(PhysGameScene* Scene, EGameObjectType Type, ComponentData* ObjectData)
	{
		if( !Scene || !ObjectData || !ObjectData->_CompDesc || Type >= EGOT_NumType )
		{
			gplDebugf(TEXT("Failed to CreatePhysGameObject Scene[%p] ObjectData[%p] Type[%d]"), Scene, ObjectData, Type);
			return NULL;
		}

#ifdef _SERVER_RUNTIME
		if (FLAG_TEST(ObjectData->_CompDesc->_LoadingFlag, ELF_ServerSkip))
		{
			gplDebugf(TEXT("Ingore Client-Only PhysGameObject Loading @CompID=%d"), ObjectData->_CompDesc->_CompId);
			return NULL;
		}
#endif // _SERVER_RUNTIME

#ifdef _CLIENT_RUNTIME
		if (FLAG_TEST(ObjectData->_CompDesc->_LoadingFlag, ELF_ClientSkip))
		{
			gplDebugf(TEXT("Ingore Server-Only PhysGameObject Loading @CompID=%d"), ObjectData->_CompDesc->_CompId);
			return NULL;
		}
#endif // _CLIENT_RUNTIME

		PhysGameObject* GameObject = NULL;
		switch(Type)
		{
		case EGOT_Ocean:
			GameObject = new PhysOcean(Scene, (SceneOceanData*)ObjectData);
			break;
		case EGOT_Terrain:
			GameObject = new PhysTerrain(Scene, (SceneTerrainData*)ObjectData);
			break;
		case EGOT_Static_Object:
			GameObject = new PhysStatic(Scene, (SceneStaticData*)ObjectData);
			break;
		case EGOT_BlockingBrush:
			GameObject = new PhysBlocking(Scene, (SceneBlockingData*)ObjectData);
			break;
		case EGOT_Vehicle_Ship:
			GameObject = new PhysVehicle(Scene, (VehicleData*)ObjectData);
			break;
		case EGOT_Vehicle_Aircraft:
			GameObject = new PhysAircraft(Scene, (AircraftData*)ObjectData);
			break;
		case EGOT_Projectile:
			GameObject = new PhysProjectile(Scene, (ProjectileData*)ObjectData);
			break;
		case EGOT_TriggerBrush:
			GameObject = new PhysTrigger(Scene, (SceneTriggerData*)ObjectData);
			break;
		case EGOT_PlayerStart:
			GameObject = new PhysPlayerStart(Scene, (ScenePlayerStartData*)ObjectData);
			break;
		case EGOT_PlayerRespawn:
			GameObject = new PhysPlayerRespawn(Scene, (ScenePlayerRespawnData*)ObjectData);
			break;
		default:
			break;
		}

		if( GameObject && !GameObject->Init() )
		{
			DestroyPhysGameObject(Scene, GameObject);
			return NULL;
		}

		return GameObject;
	}

	void DestroyPhysGameObject(PhysGameScene* Scene, PhysGameObject* GameObject)
	{
		if ( NULL != GameObject )
		{
			GameObject->Term();
			delete GameObject;
		}
	}

	PhysVehicle* UCreateShipEx(PhysGameScene* PhysScene, UShipDescEx& Desc)
	{
		ComponentDesc* VehicleCompDesc = NULL;
		if( !LoadComponentDesc(Desc._CompId, VehicleCompDesc) )		
		{
			gplDebugf(TEXT("CreateShip: LoadComponentDesc[%d]ʧ��"), Desc._CompId);
			return NULL;
		}

		VehicleData* ShipData = (VehicleData*)ConstructComponentData(VehicleCompDesc);
		ShipData->_InitGlobalPose = Desc._InitGlobalPose;
		ShipData->_InitGlobalPose.t *= G2PScale;
		ShipData->_LinearVelocity = Desc._LinearVelocity * G2PScale;
		ShipData->_MaxForwardSpeed = Desc._MaxForwardSpeed;
		if( NxMath::equals(Desc._MaxForwardSpeed, 0.f, 0.01f) )
		{
			gplDebugf(TEXT("��ʼ����ٶ��쳣����С"));
		}
		ShipData->_MaxForwardSpeedCopy = Desc._MaxForwardSpeed;
		ShipData->_MaxForwardForce = Desc._MaxForwardForce;
		ShipData->_TurnTorque = Desc._TurnTorque;
		ShipData->_MaxRiseSinkForce = Desc._MaxRiseSinkForce;
		ShipData->_Mass = Desc._Mass;
		ShipData->_CMassOffsetX = Desc._CMassOffsetX * G2PScale;
		ShipData->_Length = Desc._Length * G2PScale;
		ShipData->_Width = Desc._Width * G2PScale;
		ShipData->_Height = Desc._Height * G2PScale;

		for(NxU32 i = 0; i < Desc._AttachedCompList.size(); ++i)
		{
			ComponentDesc* AttachedCompDesc = NULL;
			if( Desc._AttachedCompList[i]._CompId <= 0 )	// ��ЧCompId
				continue;
			if( !LoadComponentDesc(Desc._AttachedCompList[i]._CompId, AttachedCompDesc) )
			{
				gplDebugf(TEXT("CreateShip: LoadComponentDesc[%d]ʧ��"), Desc._AttachedCompList[i]._CompId);
				continue;
			}

			//AI��ǹ ������������
			Socket socket;
			if(!ShipData->FindSocket(Desc._AttachedCompList[i]._AttachId, socket))
			{
				gplDebugf(TEXT("CreateShip: _AttachedCompList[%d]ʧ��"), Desc._AttachedCompList[i]._AttachId);
				continue;
			}

			if(ERM_Server == GRunMode && EPT_AntiAircraftGun == socket._PartType)
				continue;

			VehicleAttachedCompData* AttachedCompData = (VehicleAttachedCompData*)ConstructComponentData(AttachedCompDesc);
			
			AttachedCompData->_AttachedId = Desc._AttachedCompList[i]._AttachId;
			AttachedCompData->_PartType = socket._PartType;

			if (AttachedCompData->IsWeaponComp())
			{
				((WeaponData*)AttachedCompData)->_FirePartNum=Desc._AttachedCompList[i]._FirePartNum;
				((WeaponData*)AttachedCompData)->_InitVelocity=Desc._AttachedCompList[i]._InitVelocity * G2PScale;
				((WeaponData*)AttachedCompData)->_TurretAngleVelocity=Desc._AttachedCompList[i]._TurretAngleVelocity;
				((WeaponData*)AttachedCompData)->_Gravity=Desc._AttachedCompList[i]._Gravity * G2PScale;
				((WeaponData*)AttachedCompData)->_AccuracyDefault=Desc._AttachedCompList[i]._AccuracyDefault;
				((WeaponData*)AttachedCompData)->_AccuracyTurrent=Desc._AttachedCompList[i]._AccuracyTurrent;
				((WeaponData*)AttachedCompData)->_AccuracyShipDrive=Desc._AttachedCompList[i]._AccuracyShipDrive;
				((WeaponData*)AttachedCompData)->_AccuracyShipSteer=Desc._AttachedCompList[i]._AccuracyShipSteer;
				((WeaponData*)AttachedCompData)->_AccuracyRecover=Desc._AttachedCompList[i]._AccuracyRecover;
				((WeaponData*)AttachedCompData)->_AccuracyFire=Desc._AttachedCompList[i]._AccuracyFire;
				((WeaponData*)AttachedCompData)->_AccuracyContinuousFire=Desc._AttachedCompList[i]._AccuracyContinuousFire;
				((WeaponData*)AttachedCompData)->_ArtilleryDiffuseParam=Desc._AttachedCompList[i]._ArtilleryDiffuseParam;
			}

			ShipData->_AttachedCompList.push_back(AttachedCompData);
		}

		PhysVehicle* PhysVel = (PhysVehicle*)CreatePhysGameObject(PhysScene, VehicleCompDesc->_ObjectType, ShipData);
		return PhysVel;
	}

	PhysVehicle* UCreateShip(PhysGameScene* PhysScene, UShipDesc& Desc)
	{
		ComponentDesc* VehicleCompDesc = NULL;
		if( !LoadComponentDesc(Desc._CompId, VehicleCompDesc) )		
		{
			gplDebugf(TEXT("CreateShip: LoadComponentDesc[%d]ʧ��"), Desc._CompId);
			return NULL;
		}

		VehicleData* ShipData = (VehicleData*)ConstructComponentData(VehicleCompDesc);
		ShipData->_InitGlobalPose = Desc._InitGlobalPose;
		ShipData->_InitGlobalPose.t *= G2PScale;
		ShipData->_LinearVelocity = Desc._LinearVelocity * G2PScale;
		ShipData->_MaxForwardSpeed = Desc._MaxForwardSpeed;
		if( NxMath::equals(Desc._MaxForwardSpeed, 0.f, 0.01f) )
		{
			gplDebugf(TEXT("��ʼ����ٶ��쳣����С"));
		}
		ShipData->_MaxForwardSpeedCopy = Desc._MaxForwardSpeed;
		ShipData->_MaxForwardForce = Desc._MaxForwardForce;
		ShipData->_TurnTorque = Desc._TurnTorque;
		ShipData->_MaxRiseSinkForce = Desc._MaxRiseSinkForce;
		ShipData->_Mass = Desc._Mass;
		ShipData->_CMassOffsetX = Desc._CMassOffsetX * G2PScale;
		ShipData->_Length = Desc._Length * G2PScale;
		ShipData->_Width = Desc._Width * G2PScale;
		ShipData->_Height = Desc._Height * G2PScale;

		for(NxU32 i = 0; i < Desc._AttachedCompList.size(); ++i)
		{
			ComponentDesc* AttachedCompDesc = NULL;
			if( Desc._AttachedCompList[i]._CompId <= 0 )	// ��ЧCompId
				continue;
			if( !LoadComponentDesc(Desc._AttachedCompList[i]._CompId, AttachedCompDesc) )
			{
				gplDebugf(TEXT("CreateShip: LoadComponentDesc[%d]ʧ��"), Desc._AttachedCompList[i]._CompId);
				continue;
			}
			int PartType = Desc._AttachedCompList[i]._PartType;
			//AI��ǹ ������������
			if(ERM_Server == GRunMode && EPT_AntiAircraftGun == PartType)
					continue;
		
			std::vector<int> AttachedIdList;
			ShipData->GetSocketListByPartType(PartType, AttachedIdList);
			for(NxU32 j = 0; j < AttachedIdList.size(); ++j)
			{
				VehicleAttachedCompData* AttachedCompData = (VehicleAttachedCompData*)ConstructComponentData(AttachedCompDesc);
			
				AttachedCompData->_AttachedId = AttachedIdList[j];
				AttachedCompData->_PartType = PartType;

				if (AttachedCompData->IsWeaponComp())
				{
					((WeaponData*)AttachedCompData)->_FirePartNum=Desc._AttachedCompList[i]._FirePartNum;
					((WeaponData*)AttachedCompData)->_InitVelocity=Desc._AttachedCompList[i]._InitVelocity * G2PScale;
					((WeaponData*)AttachedCompData)->_TurretAngleVelocity=Desc._AttachedCompList[i]._TurretAngleVelocity;
					((WeaponData*)AttachedCompData)->_Gravity=Desc._AttachedCompList[i]._Gravity * G2PScale;
					((WeaponData*)AttachedCompData)->_AccuracyDefault=Desc._AttachedCompList[i]._AccuracyDefault;
					((WeaponData*)AttachedCompData)->_AccuracyTurrent=Desc._AttachedCompList[i]._AccuracyTurrent;
					((WeaponData*)AttachedCompData)->_AccuracyShipDrive=Desc._AttachedCompList[i]._AccuracyShipDrive;
					((WeaponData*)AttachedCompData)->_AccuracyShipSteer=Desc._AttachedCompList[i]._AccuracyShipSteer;
					((WeaponData*)AttachedCompData)->_AccuracyRecover=Desc._AttachedCompList[i]._AccuracyRecover;
					((WeaponData*)AttachedCompData)->_AccuracyFire=Desc._AttachedCompList[i]._AccuracyFire;
					((WeaponData*)AttachedCompData)->_AccuracyContinuousFire=Desc._AttachedCompList[i]._AccuracyContinuousFire;
					((WeaponData*)AttachedCompData)->_ArtilleryDiffuseParam=Desc._AttachedCompList[i]._ArtilleryDiffuseParam;
				}

				ShipData->_AttachedCompList.push_back(AttachedCompData);
			}
		}

		PhysVehicle* PhysVel = (PhysVehicle*)CreatePhysGameObject(PhysScene, VehicleCompDesc->_ObjectType, ShipData);
		return PhysVel;
	}

	PhysAircraft* CreateAircraft(PhysGameScene* PhysScene, AircraftDesc& Desc)
	{
		ComponentDesc* desc = NULL;
		if( !LoadComponentDesc(Desc._CompId, desc) )		
		{
			gplDebugf(TEXT("CreateAircraft : LoadComponentDesc[%d]ʧ��"), Desc._CompId);
			return NULL;
		}

		AircraftData* _Data = (AircraftData*)ConstructComponentData(desc);
		if (NULL != _Data)
		{
			_Data->_AircraftType = (EPartType)Desc._WeaponPartType;
			_Data->_Consume = Desc._Consume;								//	�ͺ�
			_Data->_ChaseRange = Desc._ChaseRange * G2PScale;				//	׷����Χ
			_Data->_InterceptRange = Desc._InterceptRange * G2PScale;		//	���ط�Χ
			_Data->_SensorRange = Desc._SensorRange * G2PScale;				//	��鷶Χ
			_Data->_AttackExtent = Desc._AttackExtent * G2PScale;			//	������Χ�뾶
			_Data->_AttackScope = Desc._AttackScope;						//	������Χ�н�
			_Data->_Speed = Desc._Speed * G2PScale;							//	����
			_Data->_MinRadius = Desc._MinRadius * G2PScale;					//	��Сת��뾶
			_Data->_Gravity = Desc._Gravity * G2PScale;						//	�ڵ�����
			_Data->_InitPrecision = Desc._InitPrecision * G2PScale;			//	��ʼͶ������
			_Data->_Precision = Desc._Precision;							//	��׼Ͷ�����ȣ��ٷֱȣ�
			_Data->_Collimation = Desc._Collimation;						//	��Ȧ�ٶȣ��ٷֱȣ�
			_Data->_TurningAffect = Desc._TurningAffect;					//	ת�򾫶�Ӱ�죨�ٷֱȣ�
			_Data->_LayerAffect = Desc._LayerAffect;						//	���㾫��Ӱ�죨�ٷֱȣ�
			_Data->_LayerChangeSpeed = Desc._LayerChangeSpeed * G2PScale;	//	�����ٶ�
			_Data->_AscendSlope = NxMath::tan(NxMath::degToRad(Desc._AscendAngle));			//	����б��
			_Data->_SwoopSlope = NxMath::tan(NxMath::degToRad(Desc._SwoopAngle));				//	����б��
			_Data->_EchoInterval = Desc._EchoInterval * 0.001f;				//	��Ӧʱ��
			_Data->_ExLowFlyingTime = Desc._ExLowFlyingTime * 0.001f;		//	��������ʱ�䣨���׻���
			_Data->_DiveAcceleration = -Desc._DiveAcceleration * G2PScale;	//	������ٶ�
			_Data->_PullAcceleration = Desc._PullAcceleration * G2PScale;	//	�������ٶ�
			_Data->_DiffuseEccentricity = Desc._DiffuseEccentricity;		//	Ͷ��ɢ��������
		}

		PhysAircraft* _Aircraft = (PhysAircraft*)CreatePhysGameObject(PhysScene, EGOT_Vehicle_Aircraft, _Data);

		return _Aircraft;
	}

	PhysProjectile* CreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, ProjectileDesc& ProjDesc)
	{
		ComponentDesc* CompDesc = NULL;
		if( !LoadComponentDesc(ProjDesc._CompId, CompDesc) )		// �����ڵ���������
		{
			gplDebugf(TEXT("CreateProjectile: LoadComponentDesc[%d]ʧ��"), ProjDesc._CompId);
			return NULL;
		}

		ProjectileData* ProjData = (ProjectileData*)ConstructComponentData(CompDesc);
		ProjData->_StartPos = ProjDesc._StartPos;
		ProjData->_Gravity = ProjDesc._Gravity;
		ProjData->_InitVelocity = ProjDesc._InitVelocity;
		ProjData->_TrajectoryMethod = ProjDesc._TrajectoryMethod;
		ProjData->_SafeTime = ProjDesc._SafeTime;
		ProjData->_BurstTime = ProjDesc._BurstTime;
		ProjData->_BurstRange = ProjDesc._BurstRange;
		ProjData->_BrakingVelocity = ProjDesc._BrakingVelocity;
		ProjData->_TargetPos = ProjDesc._TargetPos;
		PhysProjectile* PhysProj = (PhysProjectile*)CreatePhysGameObject(PhysScene, CompDesc->_ObjectType, ProjData);
		PhysProj->SetSponsor(Sponsor);
		PhysProj->SetSponsorAttachId(AttachId);
		return PhysProj;
	}

	PhysProjectile* UCreateProjectile(PhysGameScene* PhysScene, PhysGameObject* Sponsor, int AttachId, UProjectileDesc& ProjDesc)
	{
		ComponentDesc* CompDesc = NULL;
		if( !LoadComponentDesc(ProjDesc._CompId, CompDesc) )		// �����ڵ���������
		{
			gplDebugf(TEXT("CreateProjectile: LoadComponentDesc[%d]ʧ��"), ProjDesc._CompId);
			return NULL;
		}

		ProjectileData* ProjData = (ProjectileData*)ConstructComponentData(CompDesc);
		ProjData->_StartPos = ProjDesc._StartPos * G2PScale;
		ProjData->_Gravity = ProjDesc._Gravity * G2PScale;
		ProjData->_InitVelocity = ProjDesc._InitVelocity * G2PScale;
		ProjData->_TrajectoryMethod = ProjDesc._TrajectoryMethod;
		ProjData->_SafeTime = ProjDesc._SafeTime;
		ProjData->_BurstTime = ProjDesc._BurstTime;
		ProjData->_BurstRange = ProjDesc._BurstRange;
		ProjData->_BrakingVelocity = ProjDesc._BrakingVelocity * G2PScale;
		ProjData->_TargetPos = ProjDesc._TargetPos * G2PScale;
		ProjData->_MagneticSensorRange = ProjDesc._MagneticSensorRange * G2PScale;
		PhysProjectile* PhysProj = (PhysProjectile*)CreatePhysGameObject(PhysScene, CompDesc->_ObjectType, ProjData);
		PhysProj->SetSponsor(Sponsor);
		PhysProj->SetSponsorAttachId(AttachId);
		return PhysProj;
	}

	PhysTrigger* CreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc)
	{
		PhysTrigger* PhysTri = NULL;

		if (Desc._CompId != 0)
		{
			ComponentDesc* CompDesc = NULL;
			if( !LoadComponentDesc(Desc._CompId, CompDesc) )
			{
				gplDebugf(TEXT("CreateTrigger: LoadComponentDesc[%d]ʧ��"), Desc._CompId);
				return NULL;
			}

			SceneTriggerData* CompData = (SceneTriggerData*)ConstructComponentData(CompDesc);
			CompData->_InitGlobalPose = Desc._InitGlobalPose;
			PhysTri = (PhysTrigger*)CreatePhysGameObject(PhysScene, CompDesc->_ObjectType, CompData);
		}
		else if (Desc._Associated)
		{
			float fRadius = Desc._Radius;
			float fExtends = Desc._Extends;
			PhysTri = new AssociatedTrigger(PhysScene, fRadius, fExtends, true);
			if (NULL != PhysTri)
			{
				if (!PhysTri->Init())
				{
					DestroyPhysGameObject(PhysScene, PhysTri);
					return NULL;
				}
			}
		}
		else
		{
			float fRadius = Desc._Radius;
			float fExtends = Desc._Extends;
			PhysTri = new RangeTrigger(PhysScene, fRadius, fExtends, Desc._CheckVehicle);
			if (NULL != PhysTri)
			{
				if (!PhysTri->Init())
				{
					DestroyPhysGameObject(PhysScene, PhysTri);
					return NULL;
				}

				PhysTri->SetGlobalPosition(Desc._InitGlobalPose.t);
			}
		}

		return PhysTri;
	}

	PhysTrigger* UCreateTrigger(PhysGameScene* PhysScene, UTriggerDesc& Desc)
	{
		UTriggerDesc DescInPhysUnit;
		DescInPhysUnit._CompId = Desc._CompId;
		DescInPhysUnit._Extends = Desc._Extends * G2PScale;
		DescInPhysUnit._Radius = Desc._Radius * G2PScale;
		DescInPhysUnit._InitGlobalPose.t = Desc._InitGlobalPose.t * G2PScale;
		DescInPhysUnit._Associated = Desc._Associated;
		DescInPhysUnit._CheckVehicle = Desc._CheckVehicle;

		return CreateTrigger(PhysScene, DescInPhysUnit);
	}
}
