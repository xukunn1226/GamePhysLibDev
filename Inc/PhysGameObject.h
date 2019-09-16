//=============================================================================
// PhysGameObject: 所有基于NxActor的场景物体基类
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

		//被命中 或者冲击波影响后的最后通知。
		virtual void OnProjectileBurstHit(PhysGameObject* Projectile,const HitNotifyInfo &info)
		{
			HitNotifyInfo UInfo = info;
			UInfo._HitPos *= P2GScale;
			UOnProjectileBurstHit(Projectile, UInfo);
		}

		virtual void UOnProjectileBurstHit(PhysGameObject* Projectile,const HitNotifyInfo &info){};
	
		/*
		打中一个物体就等待处理包括 部件和任何物体仅通知动态物体一次命中，在此进行数值计算，根据返回值判断是否继续内部检测

		*/
		virtual bool OnProjectileStrike(PhysGameObject* Projectile, const DamageInfo &info)
		{
			DamageInfo UInfo = info;
			UInfo._Distance *= P2GScale;
			UInfo._HitPos *= P2GScale;
			return UOnProjectileStrike(Projectile, UInfo);
		} 
		virtual bool UOnProjectileStrike(PhysGameObject* Projectile, const DamageInfo &info)		{ return false; } 

		//	鱼雷和深水炸弹碰到水面上的时候的通知
		virtual void OnProjectileHitWater(PhysGameObject* Projectile,NxVec3 Pos,NxVec3 Normal)		{};

		//	磁感应检测报告
		virtual bool OnProjectileMagneticTestReport(PhysGameObject* Projectile, PhysGameObject* ResultObject)	{ return false; }

		//飞机进入机枪攻击范围
		virtual void OnAircraftEnter(std::vector<PhysGameObject*> &Aircrafts){};

		//	舰船沉入水面以下
		virtual void OnVehicleSinkUnderSurface(PhysGameObject* Vehicle)	{};

		//	是否许可降落
		virtual bool IsLandingPermitted(PhysGameObject* Aircraft)		{ return true; }
	
		//------------------------------------------------------------------------
		//	@brief	舰载机相关接口
		//------------------------------------------------------------------------
		//	@brief	动作报告
		//	@param	[obj]		执行对象
		//	@param	[state]		当前状态
		virtual void OnAction(PhysGameObject* obj, const GameObjectState& state)	{}

		//	@brief	物理库动作通知
		//	@param	[obj]		事件对象
		//	@param	[act]		执行动作（状态）
		//	@param	[tar]		目标位置
		//	@param	[dir]		目标方向
		virtual void OnInnerCommand(PhysGameObject* obj, UINT act, const NxVec3& tar, const NxVec3& dir)	{}

		//	@brief	拦截范围事件报告
		//	@param	[self]		自身对象
		//	@param	[other]		事件触发对象
		//	@param	[enter]		进入/离开
		virtual void OnInterceptAreaEvent(PhysGameObject* self, PhysGameObject* other, bool enter)	{}

		//	@brief	追击范围事件报告
		//	@param	[self]		自身对象
		//	@param	[other]		事件触发对象
		//	@param	[enter]		进入/离开
		virtual void OnChaseAreaEvent(PhysGameObject* self, PhysGameObject* other, bool enter)	{}

		//	@brief	攻击范围事件报告
		//	@param	[self]		自身对象
		//	@param	[other]		事件触发对象
		//	@param	[enter]		进入/离开
		virtual void OnAttack(PhysGameObject* self, PhysGameObject* other, bool enter)	{}
		/**
		 * 潜艇上浮或下潜结束通知
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
	 * @brief	所有物理库对象基类，舰船、炮弹、地形、海平面、静态物体等
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

		// 设置NxActor是否产生contact information，不影响collision detection
		void		SetNotifyRigidBodyCollision(bool bNotifyRigidBodyCollision);
		// 设置与其他PhysGameObject是否可以碰撞
		void		SetDisableRBCollisionWithOther(PhysGameObject* Other, bool bDisabled);
		// 碰撞检测事件
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

		PhysGameObject* GetLockTarget() { return _LockTarget; }				//	获得锁定目标
		void Lock(PhysGameObject* Target);									//	锁定目标
		virtual void OnTargetLost(PhysGameObject* Target);					//	目标丢失事件
		const PhysGameObjectList& GetLockHosts()	{ return _LockHosts; }	//	获得锁定自己的对象
		void OnUnlock(PhysGameObject* Host);								//	自身锁定被取消事件
		void OnLock(PhysGameObject* Host);									//	自身被锁定事件
	
		/**
		 * 返回检查点数量
		 */
		virtual int GetCheckPointNum(Socket SocketList[VEHICLE_CHECK_POINT_MAX]);

		/**
		 * 返回观察点数量
		 */
		virtual int GetViewPointNum(Socket SocketList[VEHICLE_VIEW_POINT_MAX]);

		//爆炸点
		virtual int GetBurstPointNum(Socket SocketList[VEHICLE_BURST_POINT_MAX]);

		/**
		 * 获得本舰船观察点与其他舰船检查点之间的阻挡物列表
		 * @param	Other				与之连线检查的舰船
		 * @param	ViewPoint			本船的观察点
		 * @param	CheckPoint			其他舰船的检查点
		 * @param	BlockedObjCnt		阻挡物数量，无阻挡物返回0
		 * @param	OutBlockedObjList	阻挡物列表
		 */
		virtual void GetBlockedObjectList(PhysGameObject* Other, const Socket& ViewPoint, const Socket& CheckPoint, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE]);
		
		/**
		 * 获得两物理对象中心点之间的阻挡物列表
		 * @param	Other				与之连线检查的舰船
		 * @param	CheckTerrain		是否检测地形
		 * @param	BlockedObjCnt		阻挡物数量，无阻挡物返回0
		 * @param	OutBlockedObjList	阻挡物列表
		 */
		virtual void GetBlockedObjectList(PhysGameObject* Other, bool CheckTerrain, int& BlockedObjCnt, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE]);

		int		_StaticID;		// 物理对象的ID，非全局唯一

	protected:
		PhysGameObject() {}
		PhysGameObject(const PhysGameObject& Copy) {}
		PhysGameObject& operator=(const PhysGameObject& right) {}

		virtual void InitPhys();						// 创建物理对象
		virtual void TermPhys();

		virtual void PostInitPhys();					// 创建物理对象之后，初始化对象的非物理属性
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