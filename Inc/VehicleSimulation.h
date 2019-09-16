#ifndef _VEHICLE_SIMULATION_H_
#define _VEHICLE_SIMULATION_H_

namespace GPL
{
	/**
	 * @brief	控制载具移动基类，实现WASD
	 */
	class VehicleSimBase
	{
		DECLARE_RTTI

	public:
		VehicleSimBase(ESimulationType SimType)
			: _SimType(SimType)
			, _MoveFlag(MoveFlag_None) 
			, _Throttle(0)
			, _Strafe(0)
			, _Rise(0)
			, _bReachTargetOfSpeed(false)
		{ 
		}

		void SetMoveFlag(EMoveFlag Flag);
		void ClearMoveFlag(EMoveFlag Flag);
		bool HaveAnyFlag(EMoveFlag Flag);
		EMoveFlag GetMoveFlag() { return _MoveFlag; }

		virtual void Tick(float DeltaTime, PhysVehicle* Vehicle) {}

		// 检测操作是否有效
		virtual bool IsValidOp(EMoveFlag Flag);
		/**
		 * 前向移动控制
		 * @bForward	true,持续按住“W"；false，“W”松开
		 */
		virtual void MoveForward(bool bForward);
		/**
		 * 后向移动控制
		 * @bBackward	true,持续按住“S"；false，“S”松开
		 */
		virtual void MoveBackward(bool bBackward);
		/**
		 * 右转向控制
		 * @bRight	true,持续按住“D"；false，“D”松开
		 */
		virtual void TurnRight(bool bRight);
		/**
		 * 左转向控制
		 * @bLeft	true,持续按住“A"；false，“A”松开
		 */
		virtual void TurnLeft(bool bLeft);
		/**
		 * 潜艇上浮
		 */
		virtual void Riseup(PhysVehicle* Vehicle);
		/**
		 * 潜艇下潜
		 */
		virtual void Sink(PhysVehicle* Vehicle);
		/**
		 * 中止上浮或下潜操作
		 */
		virtual void StopRiseOrSink(PhysVehicle* Vehicle);

		virtual void ForceReachDesiredLayer(PhysVehicle* Vehicle);

		/**
		 * 是否有前进或后退操作
		 */
		virtual bool HaveAnyMoveOp();
		/**
		 * 是否有转向操作
		 */
		virtual bool HaveAnyTurnOp();
		/**
		 * 是否在进退巡航中
		 */
		virtual bool IsInCruiseDrive();
		/**
		 * 是否在转向巡航中 
		 */
		virtual bool IsInCruiseSteer();

		int  GetSimulationType() const {return _SimType;}

		virtual void UpdateVehicle(PhysVehicle* Vehicle, float DeltaTime);

		NxVec3 GetForce() { return _Force; }
		NxVec3 GetTorque() { return _Torque; }

		virtual float GetMaxForwardSpeed(PhysVehicle* Vehicle);
		virtual float GetMaxReverseSpeed(PhysVehicle* Vehicle);
		virtual float GetPendingMaxTurnRate(PhysVehicle* Vehicle);

	protected:
		VehicleSimBase();
		VehicleSimBase(const VehicleSimBase&);
		VehicleSimBase& operator=(const VehicleSimBase& right);
	
		virtual void ProcessInput(PhysVehicle* Vehicle);

		float GetRiseDampingForce(PhysVehicle* Vehicle, float MaxForce, int Rise);
		float GetDampingForce(PhysVehicle* Vehicle, float InForce, float CurSpeed, float MaxSpeed);
		float GetDampingTorque(PhysVehicle* Vehicle, float MaxTurnRate, float TurnTorque, const NxVec3& Up);

		//	@brief	根据速度计算阻尼
		//	@param	[Velocity]		速度矢量
		//	@param	[MaxSpeed]		最大速度
		//	@param	[MaxDamping]	最大阻尼
		//	@param	[MinDampingRate]最小阻尼百分比，值域[0.f, 1.f]
		//	@return	阻尼矢量，方向与速度方向相反，大小与速度标量的平方成正比
		NxVec3	CaleDampingForce(const NxVec3& Velocity, float MaxSpeed, float MaxDamping, float MinDampingRate);

		// 当前的转向动力相对转向是否是阻力
		// @param	Vehicle
		// @param	bForward	载具行驶方向，TRUE:前进  FALSE:停止或后退
		bool IsTorqueDamping(PhysVehicle* Vehicle, bool bForward);

		virtual void LockPosZ(PhysVehicle* Vehicle) {}

		virtual void UnlockPosZ(PhysVehicle* Vehicle) {}

	protected:
		ESimulationType		_SimType;

		EMoveFlag			_MoveFlag;

		int					_Throttle;		// +1, 前进；0，停止；-1，后退
		int					_Strafe;		// +1，右转向；0，不转向；-1，左转向
		int					_Rise;			// +1, 上浮；0，停止；-1，下潜

		NxVec3				_Force;
		NxVec3				_Torque;

		bool				_bReachTargetOfSpeed;
	};

	// WASD + 巡航
	class VehicleSimShip : public VehicleSimBase
	{
		DECLARE_RTTI

	public:
		VehicleSimShip(ESimulationType SimType)
			: VehicleSimBase(SimType),
			  _bCruiseDriveFlag(false),
			  _bCruiseSteerFlag(false),
			  _ShiftIndexDriving(0),
			  _ShiftIndexSteering(0)
		{}
		/**
		 * 前向移动控制
		 * @bForward	true,持续按住“W"；false，“W”松开
		 */
		virtual void	MoveForward(bool bForward);
		/**
		 * 后向移动控制
		 * @bBackward	true,持续按住“S"；false，“S”松开
		 */
		virtual void	MoveBackward(bool bBackward);
		/**
		 * 右转向控制
		 * @bRight	true,持续按住“D"；false，“D”松开
		 */
		virtual void	TurnRight(bool bRight);
		/**
		 * 左转向控制
		 * @bLeft	true,持续按住“A"；false，“A”松开
		 */
		virtual void	TurnLeft(bool bLeft);
		/**
		* 纵向加减挡控制
		* @bForward	弹起"R":true "F":false
		*/
		void	SetShiftDriving(int ShiftIndexDriving);
		/**
		* 旋转加减挡控制
		* @bForward	弹起“E":true “Q”false
		*/
		void	SetShiftSteering(int ShiftIndexSteering);

		/**
		 * 是否有前进,后退,巡航操作
		 */
		virtual bool HaveAnyMoveOp();
		/**
		 * 是否有转向，巡航操作
		 */
		virtual bool HaveAnyTurnOp();
		/**
		 * 是否在进退巡航中
		 */
		virtual bool IsInCruiseDrive();
		/**
		 * 是否在转向巡航中 
		 */
		virtual bool IsInCruiseSteer();

		virtual bool IsValidOp(EMoveFlag Flag);

		int		GetDriveGear() const {return _ShiftIndexDriving;}
		int		GetSteerGear() const {return _ShiftIndexSteering;}
	
		virtual float GetMaxForwardSpeed(PhysVehicle* Vehicle);
		virtual float GetMaxReverseSpeed(PhysVehicle* Vehicle);
		virtual float GetPendingMaxTurnRate(PhysVehicle* Vehicle);
	
	protected:
		VehicleSimShip();
		VehicleSimShip(const VehicleSimShip&);
		VehicleSimShip& operator=(const VehicleSimShip& right);
	
		virtual void ProcessInput(PhysVehicle* Vehicle);

	protected:
		bool	_bCruiseDriveFlag;
		bool    _bCruiseSteerFlag;

		int		_ShiftIndexDriving;
		int		_ShiftIndexSteering;
	};

	// WASD + 巡航 + 上浮下潜
	class VehicleSimSubmarine : public VehicleSimShip
	{
		DECLARE_RTTI

	public:
		VehicleSimSubmarine(ESimulationType SimType)
			: VehicleSimShip(SimType)
		{}

		virtual void Tick(float DeltaTime, PhysVehicle* Vehicle);

		// 检测操作是否有效
		virtual bool IsValidOp(EMoveFlag Flag);

		/**
		 * 潜艇上浮
		 */
		virtual void Riseup(PhysVehicle* Vehicle);
		/**
		 * 潜艇下潜
		 */
		virtual void Sink(PhysVehicle* Vehicle);
		/**
		 * 中止上浮或下潜操作
		 */
		virtual void StopRiseOrSink(PhysVehicle* Vehicle);

		virtual void ForceReachDesiredLayer(PhysVehicle* Vehicle);

	protected:
		VehicleSimSubmarine();
		VehicleSimSubmarine(const VehicleSimSubmarine&);
		VehicleSimSubmarine& operator=(const VehicleSimSubmarine& right);

		virtual void LockPosZ(PhysVehicle* Vehicle);

		virtual void UnlockPosZ(PhysVehicle* Vehicle);

		virtual void ProcessInput(PhysVehicle* Vehicle);

		// 是否到达目标层
		bool ReachDesiredLayer(PhysVehicle* Vehicle);
	};

	//	沉船
	class VehicleSimDeath : public VehicleSimBase
	{
		DECLARE_RTTI

	public:
		VehicleSimDeath(ESimulationType SimType);

		virtual void Tick(float DeltaTime, PhysVehicle* Vehicle);

		virtual void UpdateVehicle(PhysVehicle* Vehicle, float DeltaTime);

		virtual bool IsValidOp(EMoveFlag Flag)	{ return false; }

		void RandomPose();

		void InitPhysParams(PhysVehicle* Vehicle);

		inline void ResetForce()	{ _Force = ZERO_VEC; }
		inline void ResetTorque()	{ _Torque = ZERO_VEC; }
		inline void SkipSimulate()	{ _SkipSimulate = true; }

	protected:
		NxVec3	CalcDampingForce(const NxVec3& LineVel, float Mass);
		NxVec3	CalcDampingTorque(const NxVec3& AngVel, float Mass);

	public:
		float		_Roll;
		float		_Pitch;
		float		_ForceFactor;
		float		_TorqueFactor;

	protected:
		NxQuat		_CapsizalPose;
		float		_TriggerDeepth;
		bool		_UnderSurface;
		bool		_Init;
		bool		_SkipSimulate;		//	跳过本地模拟
	};
}


#endif