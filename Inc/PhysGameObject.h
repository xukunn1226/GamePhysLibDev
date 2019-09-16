//=============================================================================
// PhysGameObject: ���л���NxActor�ĳ����������
//=============================================================================
#ifndef _PHYS_GAMEOBJECT_H_
#define _PHYS_GAMEOBJECT_H_

namespace GPL
{
	class BehaviorReport
	{
	public:		
		//modify by yeleiyu@icee.cn for collision effect 
		virtual void OnCollisionDetection(PhysGameObject* SelfObject,PhysGameObject* OtherObject, const CollisionImpactData& NotifyInfo)
		{
			//gplDebugf(TEXT("===================STAT CollisionDetection"));

			//NxVec3 Nor_TotalNormalForceVector = NotifyInfo._TotalNormalForceVector;
			//float mag = Nor_TotalNormalForceVector.normalize();
			//gplDebugf(TEXT("_TotalNormalForceVector Magnitude[%f] Vector[%.2f %.2f %.2f]"), mag, Nor_TotalNormalForceVector.x, Nor_TotalNormalForceVector.y, Nor_TotalNormalForceVector.z);
			//NxVec3 Nor_TotalFrictionForceVector = NotifyInfo._TotalFrictionForceVector;
			//mag = Nor_TotalFrictionForceVector.normalize();
			//gplDebugf(TEXT("_TotalFrictionForceVector Magnitude[%f] Vector[%.2f %.2f %.2f]"), mag, Nor_TotalFrictionForceVector.x, Nor_TotalFrictionForceVector.y, Nor_TotalFrictionForceVector.z);
			//gplDebugf(TEXT("_ContactInfos Num[%d]"), NotifyInfo._ContactInfos.size());
			//gplDebugf(TEXT("ContactInfo Event[%d]"), NotifyInfo._ContactInfos[0]._Event);
			//for(int i = 0; i < (int)NotifyInfo._ContactInfos.size(); ++i)
			//{
			//	gplDebugf(TEXT("[%d] ContactInfo Position[%.2f %.2f %.2f]"), i, NotifyInfo._ContactInfos[i]._ContactPosition.x, NotifyInfo._ContactInfos[i]._ContactPosition.y, NotifyInfo._ContactInfos[i]._ContactPosition.z);
			//	gplDebugf(TEXT("[%d] ContactInfo Normal[%.2f %.2f %.2f]"), i, NotifyInfo._ContactInfos[i]._ContactNormal.x, NotifyInfo._ContactInfos[i]._ContactNormal.y, NotifyInfo._ContactInfos[i]._ContactNormal.z);
			//	gplDebugf(TEXT("[%d] ContactInfo Velocity0[%.2f %.2f %.2f]"), i, NotifyInfo._ContactInfos[i]._ContactVelocity[0].x, NotifyInfo._ContactInfos[i]._ContactVelocity[0].y, NotifyInfo._ContactInfos[i]._ContactVelocity[0].z);
			//	gplDebugf(TEXT("[%d] ContactInfo Velocity1[%.2f %.2f %.2f]"), i, NotifyInfo._ContactInfos[i]._ContactVelocity[1].x, NotifyInfo._ContactInfos[i]._ContactVelocity[1].y, NotifyInfo._ContactInfos[i]._ContactVelocity[1].z);
			//}

			//gplDebugf(TEXT("===================STAT End"));
		}

		//������ ���߳����Ӱ�������֪ͨ��
		virtual void OnProjectileBurstHit(PhysGameObject* Projectile,const HitNotifyInfo &info)
		{
			HitNotifyInfo UInfo = info;
			UInfo._HitPos *= P2GScale;
			UOnProjectileBurstHit(Projectile, UInfo);
		}

		virtual void UOnProjectileBurstHit(PhysGameObject* Projectile,const HitNotifyInfo &info){};
	
		/*
		����һ������͵ȴ�������� �������κ������֪ͨ��̬����һ�����У��ڴ˽�����ֵ���㣬���ݷ���ֵ�ж��Ƿ�����ڲ����

		*/
		virtual bool OnProjectileStrike(PhysGameObject* Projectile, const DamageInfo &info)
		{
			DamageInfo UInfo = info;
			UInfo._Distance *= P2GScale;
			UInfo._HitPos *= P2GScale;
			return UOnProjectileStrike(Projectile, UInfo);
		} 
		virtual bool UOnProjectileStrike(PhysGameObject* Projectile, const DamageInfo &info)		{ return false; } 

		//	���׺���ˮը������ˮ���ϵ�ʱ���֪ͨ
		virtual void OnProjectileHitWater(PhysGameObject* Projectile,NxVec3 Pos,NxVec3 Normal)		{};

		//	�Ÿ�Ӧ��ⱨ��
		virtual bool OnProjectileMagneticTestReport(PhysGameObject* Projectile, PhysGameObject* ResultObject)	{ return false; }

		//�ɻ������ǹ������Χ
		virtual void OnAircraftEnter(std::vector<PhysGameObject*> &Aircrafts){};

		//	��������ˮ������
		virtual void OnVehicleSinkUnderSurface(PhysGameObject* Vehicle)	{};

		//	�Ƿ���ɽ���
		virtual bool IsLandingPermitted(PhysGameObject* Aircraft)		{ return true; }
	
		//------------------------------------------------------------------------
		//	@brief	���ػ���ؽӿ�
		//------------------------------------------------------------------------
		//	@brief	��������
		//	@param	[obj]		ִ�ж���
		//	@param	[state]		��ǰ״̬
		virtual void OnAction(PhysGameObject* obj, const GameObjectState& state)	{}

		//	@brief	����⶯��֪ͨ
		//	@param	[obj]		�¼�����
		//	@param	[act]		ִ�ж�����״̬��
		//	@param	[tar]		Ŀ��λ��
		//	@param	[dir]		Ŀ�귽��
		virtual void OnInnerCommand(PhysGameObject* obj, UINT act, const NxVec3& tar, const NxVec3& dir)	{}

		//	@brief	���ط�Χ�¼�����
		//	@param	[self]		�������
		//	@param	[other]		�¼���������
		//	@param	[enter]		����/�뿪
		virtual void OnInterceptAreaEvent(PhysGameObject* self, PhysGameObject* other, bool enter)	{}

		//	@brief	׷����Χ�¼�����
		//	@param	[self]		�������
		//	@param	[other]		�¼���������
		//	@param	[enter]		����/�뿪
		virtual void OnChaseAreaEvent(PhysGameObject* self, PhysGameObject* other, bool enter)	{}

		//	@brief	������Χ�¼�����
		//	@param	[self]		�������
		//	@param	[other]		�¼���������
		//	@param	[enter]		����/�뿪
		virtual void OnAttack(PhysGameObject* self, PhysGameObject* other, bool enter)	{}
		/**
		 * Ǳͧ�ϸ�����Ǳ����֪ͨ
		 */ 
		virtual void OnRiseOrSinkEnd(PhysGameObject* Vehicle) {}
		/**
		 * Actor encroaching on Trigger
		 */
		virtual void EncroachingOn(PhysGameObject* Actor, PhysGameObject* Trigger) {}
		/**
		 * Trigger encroached by Actor
		 */
		virtual void EncroachedBy(PhysGameObject* Trigger, PhysGameObject* Actor) {}
		/**
		 * Actor separate from Trigger
		 */
		virtual void SeparatingFrom(PhysGameObject* Actor, PhysGameObject* Trigger) {}
		/**
		 * Trigger separate by Actor
		 */
		virtual void SeparatedBy(PhysGameObject* Trigger, PhysGameObject* Actor) {}
	};

	/**
	 * @brief	��������������࣬�������ڵ������Ρ���ƽ�桢��̬�����
	 */
	typedef	std::vector<PhysGameObject*> PhysGameObjectList;

	class PhysGameObject : public GameObject
	{
		DECLARE_RTTI

	public:
		PhysGameObject(PhysGameScene* GameScene, ComponentData* Data)
			: _Scene(GameScene)
			, _Owner(NULL)
			, _LockTarget(NULL)
			, _bInitialized(false)
			, _BehaviorReport(NULL)
			, _PhysActor(NULL)
			, _StaticID(0)
		{
			_ComponentData = Data;
		}

		virtual bool Init();

		virtual void Term();

		virtual void Tick(float DeltaTime) {}

		void		SetBehaviorReport(BehaviorReport* Report) { _BehaviorReport = Report; }
		BehaviorReport *GetBehaviorReport(){return _BehaviorReport;}

		NxActor*	GetPhysActor() const { return _PhysActor; }
		NxScene*	GetPhysScene() const { return _PhysActor != NULL ? &_PhysActor->getScene() : NULL; }

		bool		IsDynamic() const { return _PhysActor != NULL ? _PhysActor->isDynamic() : false; }

		virtual NxMat34	GetGlobalPos() const { return _PhysActor != NULL ? _PhysActor->getGlobalPose() : NxMat34(true); }
		virtual NxMat33	GetGlobalOritation() const { return _PhysActor != NULL ? _PhysActor->getGlobalOrientation() : NxMat33(); }
		virtual NxQuat	GetGlobalOritationQuat() const { return _PhysActor != NULL ? _PhysActor->getGlobalOrientationQuat() : NxQuat(); }
		virtual NxVec3	GetGlobalPosition() const { return _PhysActor != NULL ? _PhysActor->getGlobalPosition() : GPL::ZERO_VEC; }
		virtual void SetGlobalPose(const NxMat34& Pose) { if( _PhysActor ) _PhysActor->setGlobalPose(Pose); }
		virtual void USetGlobalPose(const NxMat34& Pose)
		{
			if( _PhysActor ) 
			{
				NxMat34 GlobalPose = Pose;
				GlobalPose.t *= G2PScale;
				_PhysActor->setGlobalPose(GlobalPose); 
			}
		}
		virtual void SetGlobalOritation(const NxMat33& Oritation) { if( _PhysActor ) _PhysActor->setGlobalOrientation(Oritation); }
		virtual void SetGlobalOritationQuat(const NxQuat& Quat) { if( _PhysActor ) _PhysActor->setGlobalOrientationQuat(Quat); }
		virtual void SetGlobalPosition(const NxVec3& Position) { if( _PhysActor ) _PhysActor->setGlobalPosition(Position); }
		virtual void USetGlobalPosition(const NxVec3& UPosition) { if( _PhysActor ) _PhysActor->setGlobalPosition(UPosition*G2PScale); }
		virtual float GetGlobalOritationYaw();

		// ����NxActor�Ƿ����contact information����Ӱ��collision detection
		void		SetNotifyRigidBodyCollision(bool bNotifyRigidBodyCollision);
		// ����������PhysGameObject�Ƿ������ײ
		void		SetDisableRBCollisionWithOther(PhysGameObject* Other, bool bDisabled);
		// ��ײ����¼�
		virtual void OnCollisionDetection(PhysGameObject* OtherObject, const CollisionImpactData& NotifyInfo)
		{
			if( _BehaviorReport )
			{
				//modify by yeleiyu@icee.cn for collision effect 
				_BehaviorReport->OnCollisionDetection(this, OtherObject, NotifyInfo);
			}
		}
		
		PhysGameObject* GetOwner() { return _Owner; }
		void SetOwner(PhysGameObject* Owner);
		bool IsOwnedBy(PhysGameObject* TestOwner);

		const PhysGameObjectList& GetChildren()	{ return _Children; }
	
		virtual void OnLostChild(PhysGameObject* Child);
		virtual void OnGainChild(PhysGameObject* Child);

		PhysGameObject* GetLockTarget() { return _LockTarget; }				//	�������Ŀ��
		void Lock(PhysGameObject* Target);									//	����Ŀ��
		virtual void OnTargetLost(PhysGameObject* Target);					//	Ŀ�궪ʧ�¼�
		const PhysGameObjectList& GetLockHosts()	{ return _LockHosts; }	//	��������Լ��Ķ���
		void OnUnlock(PhysGameObject* Host);								//	����������ȡ���¼�
		void OnLock(PhysGameObject* Host);									//	���������¼�
	
		/**
		 * ���ؼ�������
		 */
		virtual int GetCheckPointNum(Socket SocketList[VEHICLE_CHECK_POINT_MAX]);

		/**
		 * ���ع۲������
		 */
		virtual int GetViewPointNum(Socket SocketList[VEHICLE_VIEW_POINT_MAX]);

		//��ը��
		virtual int GetBurstPointNum(Socket SocketList[VEHICLE_BURST_POINT_MAX]);

		/**
		 * ��ñ������۲����������������֮����赲���б�
		 * @param	Other				��֮���߼��Ľ���
		 * @param	ViewPoint			�����Ĺ۲��
		 * @param	CheckPoint			���������ļ���
		 * @param	BlockedObjCnt		�赲�����������赲�ﷵ��0
		 * @param	OutBlockedObjList	�赲���б�
		 */
		virtual void GetBlockedObjectList(PhysGameObject* Other, const Socket& ViewPoint, const Socket& CheckPoint, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE]);
		
		/**
		 * ���������������ĵ�֮����赲���б�
		 * @param	Other				��֮���߼��Ľ���
		 * @param	CheckTerrain		�Ƿ������
		 * @param	BlockedObjCnt		�赲�����������赲�ﷵ��0
		 * @param	OutBlockedObjList	�赲���б�
		 */
		virtual void GetBlockedObjectList(PhysGameObject* Other, bool CheckTerrain, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE]);

		int		_StaticID;		// ��������ID����ȫ��Ψһ

	protected:
		PhysGameObject() {}
		PhysGameObject(const PhysGameObject& Copy) {}
		PhysGameObject& operator=(const PhysGameObject& right) {}

		virtual void InitPhys();						// �����������
		virtual void TermPhys();

		virtual void PostInitPhys();					// �����������֮�󣬳�ʼ������ķ���������
		virtual void PrevTermPhys();

		virtual void DoTermPhysActor(NxScene* Scene, NxActor* Actor);

	protected:
		BehaviorReport*					_BehaviorReport;			// send behavior to outer

		PhysGameScene*					_Scene;

		NxActor*						_PhysActor;

		bool							_bInitialized;

		PhysGameObject*					_Owner;

		PhysGameObjectList				_Children;

		PhysGameObject*					_LockTarget;

		PhysGameObjectList				_LockHosts;
	};


}

#endif