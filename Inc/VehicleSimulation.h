#ifndef _VEHICLE_SIMULATION_H_
#define _VEHICLE_SIMULATION_H_

namespace GPL
{
	/**
	 * @brief	�����ؾ��ƶ����࣬ʵ��WASD
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

		// �������Ƿ���Ч
		virtual bool IsValidOp(EMoveFlag Flag);
		/**
		 * ǰ���ƶ�����
		 * @bForward	true,������ס��W"��false����W���ɿ�
		 */
		virtual void MoveForward(bool bForward);
		/**
		 * �����ƶ�����
		 * @bBackward	true,������ס��S"��false����S���ɿ�
		 */
		virtual void MoveBackward(bool bBackward);
		/**
		 * ��ת�����
		 * @bRight	true,������ס��D"��false����D���ɿ�
		 */
		virtual void TurnRight(bool bRight);
		/**
		 * ��ת�����
		 * @bLeft	true,������ס��A"��false����A���ɿ�
		 */
		virtual void TurnLeft(bool bLeft);
		/**
		 * Ǳͧ�ϸ�
		 */
		virtual void Riseup(PhysVehicle* Vehicle);
		/**
		 * Ǳͧ��Ǳ
		 */
		virtual void Sink(PhysVehicle* Vehicle);
		/**
		 * ��ֹ�ϸ�����Ǳ����
		 */
		virtual void StopRiseOrSink(PhysVehicle* Vehicle);

		virtual void ForceReachDesiredLayer(PhysVehicle* Vehicle);

		/**
		 * �Ƿ���ǰ������˲���
		 */
		virtual bool HaveAnyMoveOp();
		/**
		 * �Ƿ���ת�����
		 */
		virtual bool HaveAnyTurnOp();
		/**
		 * �Ƿ��ڽ���Ѳ����
		 */
		virtual bool IsInCruiseDrive();
		/**
		 * �Ƿ���ת��Ѳ���� 
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

		//	@brief	�����ٶȼ�������
		//	@param	[Velocity]		�ٶ�ʸ��
		//	@param	[MaxSpeed]		����ٶ�
		//	@param	[MaxDamping]	�������
		//	@param	[MinDampingRate]��С����ٷֱȣ�ֵ��[0.f, 1.f]
		//	@return	����ʸ�����������ٶȷ����෴����С���ٶȱ�����ƽ��������
		NxVec3	CaleDampingForce(const NxVec3& Velocity, float MaxSpeed, float MaxDamping, float MinDampingRate);

		// ��ǰ��ת�������ת���Ƿ�������
		// @param	Vehicle
		// @param	bForward	�ؾ���ʻ����TRUE:ǰ��  FALSE:ֹͣ�����
		bool IsTorqueDamping(PhysVehicle* Vehicle, bool bForward);

		virtual void LockPosZ(PhysVehicle* Vehicle) {}

		virtual void UnlockPosZ(PhysVehicle* Vehicle) {}

	protected:
		ESimulationType		_SimType;

		EMoveFlag			_MoveFlag;

		int					_Throttle;		// +1, ǰ����0��ֹͣ��-1������
		int					_Strafe;		// +1����ת��0����ת��-1����ת��
		int					_Rise;			// +1, �ϸ���0��ֹͣ��-1����Ǳ

		NxVec3				_Force;
		NxVec3				_Torque;

		bool				_bReachTargetOfSpeed;
	};

	// WASD + Ѳ��
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
		 * ǰ���ƶ�����
		 * @bForward	true,������ס��W"��false����W���ɿ�
		 */
		virtual void	MoveForward(bool bForward);
		/**
		 * �����ƶ�����
		 * @bBackward	true,������ס��S"��false����S���ɿ�
		 */
		virtual void	MoveBackward(bool bBackward);
		/**
		 * ��ת�����
		 * @bRight	true,������ס��D"��false����D���ɿ�
		 */
		virtual void	TurnRight(bool bRight);
		/**
		 * ��ת�����
		 * @bLeft	true,������ס��A"��false����A���ɿ�
		 */
		virtual void	TurnLeft(bool bLeft);
		/**
		* ����Ӽ�������
		* @bForward	����"R":true "F":false
		*/
		void	SetShiftDriving(int ShiftIndexDriving);
		/**
		* ��ת�Ӽ�������
		* @bForward	����E":true ��Q��false
		*/
		void	SetShiftSteering(int ShiftIndexSteering);

		/**
		 * �Ƿ���ǰ��,����,Ѳ������
		 */
		virtual bool HaveAnyMoveOp();
		/**
		 * �Ƿ���ת��Ѳ������
		 */
		virtual bool HaveAnyTurnOp();
		/**
		 * �Ƿ��ڽ���Ѳ����
		 */
		virtual bool IsInCruiseDrive();
		/**
		 * �Ƿ���ת��Ѳ���� 
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

	// WASD + Ѳ�� + �ϸ���Ǳ
	class VehicleSimSubmarine : public VehicleSimShip
	{
		DECLARE_RTTI

	public:
		VehicleSimSubmarine(ESimulationType SimType)
			: VehicleSimShip(SimType)
		{}

		virtual void Tick(float DeltaTime, PhysVehicle* Vehicle);

		// �������Ƿ���Ч
		virtual bool IsValidOp(EMoveFlag Flag);

		/**
		 * Ǳͧ�ϸ�
		 */
		virtual void Riseup(PhysVehicle* Vehicle);
		/**
		 * Ǳͧ��Ǳ
		 */
		virtual void Sink(PhysVehicle* Vehicle);
		/**
		 * ��ֹ�ϸ�����Ǳ����
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

		// �Ƿ񵽴�Ŀ���
		bool ReachDesiredLayer(PhysVehicle* Vehicle);
	};

	//	����
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
		bool		_SkipSimulate;		//	��������ģ��
	};
}


#endif