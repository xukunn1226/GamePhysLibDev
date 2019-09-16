#include "..\Inc\PhysXSupport.h"

namespace GPL
{
	IMPLEMENT_RTTI_BASE(VehicleSimBase)
	IMPLEMENT_RTTI(VehicleSimShip, VehicleSimBase)
	IMPLEMENT_RTTI(VehicleSimSubmarine, VehicleSimShip)
	IMPLEMENT_RTTI(VehicleSimDeath, VehicleSimBase)

	//------------------------------------------------------------------------
	//	VehicleSimBase
	//------------------------------------------------------------------------
	bool VehicleSimBase::IsValidOp(EMoveFlag Flag)
	{
		if( Flag == MoveFlag_Forward
			|| Flag == MoveFlag_Backward
			|| Flag == MoveFlag_RightTurn
			|| Flag == MoveFlag_LeftTurn )
		{
			return true;
		}
		return false;
	}

	void VehicleSimBase::SetMoveFlag(EMoveFlag Flag)
	{
		_MoveFlag |= Flag;
	}

	void VehicleSimBase::ClearMoveFlag(EMoveFlag Flag)
	{
		_MoveFlag &= ~Flag;
	}

	bool VehicleSimBase::HaveAnyFlag(EMoveFlag Flag)
	{
		return (_MoveFlag & Flag) != 0;
	}

	void VehicleSimBase::MoveForward(bool bForward)
	{
		if( bForward )
		{
			SetMoveFlag(MoveFlag_Forward);
		}
		else
		{
			ClearMoveFlag(MoveFlag_Forward);
		}
	}

	void VehicleSimBase::MoveBackward(bool bBackward)
	{
		if( bBackward )
		{
			SetMoveFlag(MoveFlag_Backward);
		}
		else
		{
			ClearMoveFlag(MoveFlag_Backward);
		}
	}

	void VehicleSimBase::TurnRight(bool bRight)
	{
		if( bRight )
		{
			SetMoveFlag(MoveFlag_RightTurn);
		}
		else
		{
			ClearMoveFlag(MoveFlag_RightTurn);
		}
	}

	void VehicleSimBase::TurnLeft(bool bLeft)
	{
		if( bLeft )
		{
			SetMoveFlag(MoveFlag_LeftTurn);
		}
		else
		{
			ClearMoveFlag(MoveFlag_LeftTurn);
		}
	}

	void VehicleSimBase::Riseup(PhysVehicle* Vehicle)
	{
	}

	void VehicleSimBase::Sink(PhysVehicle* Vehicle)
	{
	}

	void VehicleSimBase::StopRiseOrSink(PhysVehicle* Vehicle)
	{
	}

	void VehicleSimBase::ForceReachDesiredLayer(PhysVehicle* Vehicle)
	{
	}

	bool VehicleSimBase::HaveAnyMoveOp()
	{
		return (HaveAnyFlag(MoveFlag_Forward) || HaveAnyFlag(MoveFlag_Backward));
	}

	bool VehicleSimBase::HaveAnyTurnOp()
	{
		return (HaveAnyFlag(MoveFlag_RightTurn) || HaveAnyFlag(MoveFlag_LeftTurn));
	}
	
	bool VehicleSimBase::IsInCruiseDrive()
	{
		return FALSE;
	}

	bool VehicleSimBase::IsInCruiseSteer()
	{
		return FALSE;
	}

	float VehicleSimBase::GetMaxForwardSpeed(PhysVehicle* Vehicle)
	{
		return Vehicle != NULL ? Vehicle->GetMaxForwardSpeed() : 0.f;
	}

	float VehicleSimBase::GetMaxReverseSpeed(PhysVehicle* Vehicle)
	{
		return Vehicle != NULL ? Vehicle->GetMaxReverseSpeed() : 0.f;
	}

	float VehicleSimBase::GetPendingMaxTurnRate(PhysVehicle* Vehicle)
	{
		if( Vehicle == NULL || Vehicle->GetPhysActor() == NULL )
		{
			return 0.f;
		}

		return Vehicle->GetPendingTurnRate();
	}

	void VehicleSimBase::ProcessInput(PhysVehicle* Vehicle)
	{
		if( HaveAnyFlag(MoveFlag_Forward) && HaveAnyFlag(MoveFlag_Backward) )
		{
			_Throttle = 0;
		}
		else if( HaveAnyFlag(MoveFlag_Forward) )
		{
			_Throttle = 1;
		}
		else if( HaveAnyFlag(MoveFlag_Backward) )
		{
			_Throttle = -1;
		}
		else
		{
			_Throttle = 0;
		}

		if( HaveAnyFlag(MoveFlag_RightTurn) && HaveAnyFlag(MoveFlag_LeftTurn) )
		{
			_Strafe = 0;
		}
		else if( HaveAnyFlag(MoveFlag_RightTurn) )
		{
			_Strafe = 1;
		}
		else if( HaveAnyFlag(MoveFlag_LeftTurn) )
		{
			_Strafe = -1;
		}
		else
		{
			_Strafe = 0;
		}
	}

	void VehicleSimBase::UpdateVehicle(PhysVehicle* Vehicle, float DeltaTime)
	{
		if( Vehicle == NULL || Vehicle->GetPhysActor() == NULL )
			return;

		VehicleData* ObjectDesc = DynamicCast(VehicleData, Vehicle->GetComponentData());
		if( ObjectDesc == NULL )
			return;

		VehicleCompDesc* CompDesc = DynamicCast(VehicleCompDesc, ObjectDesc->_CompDesc);
		if( CompDesc == NULL )
			return;

		ProcessInput(Vehicle);

		Vehicle->_PrevVelocity = Vehicle->_Velocity;
		Vehicle->_Velocity = Vehicle->GetPhysActor()->getLinearVelocity();		// ��¼��ǰ�ٶ�ֵ
		Vehicle->_PrevAngularVelocity = Vehicle->_AngularVelocity;
		Vehicle->_AngularVelocity = Vehicle->GetPhysActor()->getAngularVelocity();

		//	û�����������������ת��������ǿ��������ؼ����ٶ�
		if ( _Strafe == 0 )
		{
			if ( Vehicle->_PrevAngularVelocity.z * Vehicle->_AngularVelocity.z < 0 )
			{
				Vehicle->GetPhysActor()->setAngularVelocity(ZERO_VEC);
				Vehicle->GetPhysActor()->addTorque(ZERO_VEC);
				return;
			}
			else if ( !NxMath::equals(Vehicle->_PrevAngularVelocity.z, 0.f, 0.001f) && NxMath::equals(Vehicle->_AngularVelocity.z, 0.f, 0.001f) )
			{
				Vehicle->GetPhysActor()->setAngularVelocity(ZERO_VEC);
				Vehicle->GetPhysActor()->addTorque(ZERO_VEC);
				return;
			}
		}

		//	û����������������ٶȷ���ǿ������ٶȺ���
		if ( _Throttle == 0 && Vehicle->_PrevVelocity.dot(Vehicle->_Velocity) < 0 )
		{
			Vehicle->GetPhysActor()->setLinearVelocity(ZERO_VEC);
			Vehicle->GetPhysActor()->addForce(ZERO_VEC);
			return;
		}
		
		// NxActor���ᳯ��
		NxVec3 DirX, DirY, DirZ;
		NxMat33 WorldOri = Vehicle->GetPhysActor()->getGlobalOrientation();
		DirX = WorldOri.getColumn(0);
		DirY = WorldOri.getColumn(1);
		DirZ = WorldOri.getColumn(2);

		NxVec3 Forward = DirX;		// ��������,����Z����
		Forward.z = 0.0f;
		Forward.normalize();

		NxVec3 Right = DirY;
		Right.z = 0.0f;
		Right.normalize();

		NxVec3 Up(AXIS_VEC_Z);
		_Force = ZERO_VEC;
		_Torque = ZERO_VEC;

		NxVec3 VelocityXY = Vehicle->_Velocity;
		VelocityXY.z = 0;											// ����XY�����ٶ�(δNormalize)
		float LinearVelXYMag = VelocityXY.magnitude();				// ˮƽ�����ٶȴ�С
		NxVec3 PrevVelocityXY = Vehicle->_PrevVelocity;
		PrevVelocityXY.z = 0;										// ����XY������һ֡�ٶ�(δNormalize)
		float LinearPrevVelXYMag = PrevVelocityXY.magnitude();		// ˮƽ�����ٶȴ�С
		float PendingForwardSpeed = GetMaxForwardSpeed(Vehicle);	// Ŀ��ǰ���ٶ�
		float PendingReverseSpeed = GetMaxReverseSpeed(Vehicle);	// Ŀ������ٶ�
		float MaxForwardSpeed = Vehicle->GetInitializedMaxForwardSpeed();		// ��������ٶ�

		NxVec3 VelDirXY = VelocityXY;
		VelDirXY.normalize();									// ����ˮƽ�����ٶȳ���
		float ForwardVel = VelDirXY.dot(Forward);
		float RightVel = VelDirXY.dot(Right);
		NxVec3 LongDampingDir = Forward * (ForwardVel > 0.f ? -1.f : 1.f);		// �������᷽��ʼ��ΪForward������
		NxVec3 LatDampingDir = Right * (RightVel > 0.f ? -1.f : 1.f);			// �������᷽��ʼ��ΪRight������

		
		// �ж��ؾߴ��ڼ��ٻ��Ǽ���״̬
		bool bSpeedupState = false;			// true������״̬��false������״̬
		if( _Throttle > 0 )
		{
			if( ForwardVel >= 0.f )		// �ؾ߳������ٶȷ���һ��ʱ
			{
				if( LinearVelXYMag < PendingForwardSpeed )
				{
					bSpeedupState = true;
				}
				else
				{
					bSpeedupState = false;
				}
			}
			else
			{
				bSpeedupState = false;
			}
		}
		else if( _Throttle < 0 )
		{
			if( ForwardVel <= 0.f )		// �ؾ߳������ٶȷ���һ��ʱ
			{
				if( LinearVelXYMag < PendingReverseSpeed )
				{
					bSpeedupState = true;
				}
				else
				{
					bSpeedupState = false;
				}
			}
			else
			{
				bSpeedupState = false;
			}
		}

		if( _Throttle > 0 )
		{
			// ���ٻ��ؾ߳������ٶȷ����෴ʱ
			if( bSpeedupState )
			{
				_Force += _Throttle * ObjectDesc->_MaxForwardForce * Forward;

				// �˴������Ŀ�Ĳ������ؾ߼��٣��������ؾ߼��ٶ����ٶȱ仯���仯
				_Force += GetDampingForce(Vehicle, ObjectDesc->_MaxForwardForce, LinearVelXYMag, MaxForwardSpeed) * LongDampingDir;
			}
			else if( ForwardVel < 0.f )
			{
				_Force += _Throttle * ObjectDesc->_MaxForwardForce * Forward * GPhysGameSetting._GPL_ForwardForce_DampingFactor;

				// �˴������Ŀ�Ĳ������ؾ߼��٣��������ؾ߼��ٶ����ٶȱ仯���仯
				_Force += GetDampingForce(Vehicle, ObjectDesc->_MaxForwardForce * GPhysGameSetting._GPL_ForwardForce_DampingFactor, LinearVelXYMag, MaxForwardSpeed) * LongDampingDir;
			}
		}
		else if( _Throttle < 0 )
		{
			// ���ٻ��ؾ߳������ٶȷ���һ��ʱ
			if( bSpeedupState )
			{
				_Force += _Throttle * Vehicle->GetMaxReverseForce() * Forward;

				_Force += GetDampingForce(Vehicle, Vehicle->GetMaxReverseForce(), LinearVelXYMag, MaxForwardSpeed) * LongDampingDir;
			}
			else if( ForwardVel > 0.f )
			{
				_Force += _Throttle * Vehicle->GetMaxReverseForce() * Forward * GPhysGameSetting._GPL_ReverseForce_DampingFactor;

				_Force += GetDampingForce(Vehicle, Vehicle->GetMaxReverseForce() * GPhysGameSetting._GPL_ReverseForce_DampingFactor, LinearVelXYMag, MaxForwardSpeed) * LongDampingDir;
			}
		}

		// ��ǰ�ٶ�������ٶȱ�
		float Alpha = (MaxForwardSpeed == 0.f ? 0.2f : NxMath::clamp(LinearVelXYMag / MaxForwardSpeed, 1.f, 0.f));

		// ����״̬ʱ�ż�����������
		if( !bSpeedupState )
		{
			_Force += CompDesc->_ScaleDampingSpeed
						* CompDesc->_LongDampingForce
						* NxMath::abs(ForwardVel)
						* Alpha 
						* LongDampingDir;	// ��������
		}
	
		// ����������໬
		//if( RightVel > 0.0349f || RightVel < -0.0349f )
		//{
			//_Force += CompDesc->_ScaleDampingSpeed
			//	* CompDesc->_LatDampingForce
			//	* NxMath::abs(RightVel)
			//	* Alpha 
			//	* LatDampingDir;
		//}
		//else if ( !NxMath::equals(ForwardVel, 0.f, 0.001f) )
		//{
			// ȥ����������ٶȣ���ֹ�໬Ч��
			VelDirXY = Forward * (ForwardVel > 0.f ? 1.f : -1.f);
			NxVec3 Vel = LinearVelXYMag * VelDirXY;
			Vel.z = Vehicle->_Velocity.z;
			Vehicle->GetPhysActor()->setLinearVelocity(Vel);
		//}

		// rise force
		if( _Rise != 0 )
		{
#if 0
			NxShape* shapes[0xff];
			NxU32 group = (1 << GPL_SHAPE_GROUP_VEHICLE_SHIP | 1 << GPL_SHAPE_GROUP_TERRAIN);
			NxU32 HitsNum = Vehicle->GetPhysScene()->overlapOBBShapes(Vehicle->_EigenBox, NX_ALL_SHAPES, 0xff, shapes, NULL, group);
			bool bHit = false;
			for ( NxU32 idx = 0; idx < HitsNum; idx++ )
			{
				PhysGameObject* pObj = (PhysGameObject*)shapes[idx]->getActor().userData;
				if ( !pObj )
					continue;

				if ( pObj != Vehicle )
				{
					bHit = true;
					break;
				}
			}

			if ( bHit )
			{
				NxVec3 NewVel = Vehicle->_Velocity;
				NewVel.z = 0;
				Vehicle->GetPhysActor()->setLinearVelocity(NewVel);
			}
			else
			{
				_Force += _Rise * ObjectDesc->_MaxRiseSinkForce * Up;
				_Force -= _Rise * GetRiseDampingForce(Vehicle, ObjectDesc->_MaxRiseSinkForce, _Rise) * Up;
			}
#else			
			_Force += _Rise * ObjectDesc->_MaxRiseSinkForce * Up;
			_Force -= _Rise * GetRiseDampingForce(Vehicle, ObjectDesc->_MaxRiseSinkForce, _Rise) * Up;
#endif
		}

		// �ж��Ƿ�ﵽĿ���ٶ��볯��
		_bReachTargetOfSpeed = false;
		if( _Throttle > 0 )
		{
			if( ForwardVel > 0.f && 
				((LinearVelXYMag - PendingForwardSpeed) * (PendingForwardSpeed - LinearPrevVelXYMag) > 0 
				|| NxMath::equals(PendingForwardSpeed, 0.f, 0.05f)
				|| NxMath::abs(LinearVelXYMag - PendingForwardSpeed) / PendingForwardSpeed < 0.005f ))
			{ // ��ǰ�ٶ�����һ֡�ٶȿ��Ŀ���ٶȣ�����Ϊ�ﵽĿ���ٶ�ֵ
				_bReachTargetOfSpeed = true;
			}
			else
			{
				_bReachTargetOfSpeed = false;
			}

			// ����Ŀ���ٶ�ֵʱ���ض�Ϊ����ٶ�
			if( _bReachTargetOfSpeed && LinearVelXYMag > PendingForwardSpeed )
			{
				NxVec3 NewVel = VelDirXY * PendingForwardSpeed;
				NewVel.z = Vehicle->_Velocity.z;
				Vehicle->GetPhysActor()->setLinearVelocity(NewVel * 0.999f);		// ʹ����С��PendingForwardSpeed���ʼ���״̬
			}
		}
		if( _Throttle < 0 )
		{
			if( ForwardVel < 0.f &&
				((LinearVelXYMag - PendingReverseSpeed) * (PendingReverseSpeed - LinearPrevVelXYMag) > 0 
				|| NxMath::equals(PendingReverseSpeed, 0.f, 0.05f * GPhysGameSetting._GPL_ReverseSpeed_Rate)
				|| NxMath::abs(LinearVelXYMag - PendingReverseSpeed) / PendingReverseSpeed < 0.005f ) )
			{
				_bReachTargetOfSpeed = true;
			}
			else
			{
				_bReachTargetOfSpeed = false;
			}

			if( _bReachTargetOfSpeed && LinearVelXYMag > PendingReverseSpeed )
			{
				NxVec3 NewVel = VelDirXY * PendingReverseSpeed;
				NewVel.z = Vehicle->_Velocity.z;
				Vehicle->GetPhysActor()->setLinearVelocity(NewVel * 0.999f);		// ʹ����С��PendingReverseSpeed���ʼ���״̬
			}
		}
		if( _bReachTargetOfSpeed )
		{ // ������Ŀ���ٶ�ֵʱ����������С��һ��������
			//NxVec3 SlightForce(0.f);
			//if( _Throttle > 0.f )
			//{
			//	SlightForce = _Throttle * ObjectDesc->_MaxForwardForce * Forward * 0.01f;
			//}
			//else if( _Throttle < 0.f )
			//{
			//	SlightForce = _Throttle * Vehicle->GetMaxReverseForce() * Forward * 0.01f;
			//}			
			//
			//_Force.x = SlightForce.x;
			//_Force.y = SlightForce.y;

			_Force.x = 0.f;
			_Force.y = 0.f;
		}

		if ( _Throttle == 0 && _Strafe == 0 && _Rise == 0 && LinearVelXYMag < CompDesc->_StopThreshold * G2PScale )
		{
			_Force = ZERO_VEC;
			Vehicle->GetPhysActor()->setLinearVelocity(ZERO_VEC);
		}

		////////////////// ����ת��Ť��
		NxVec3 AngVel = Vehicle->_AngularVelocity;
		_Torque += Up * (ObjectDesc->_TurnTorque * _Strafe * (_Throttle >= 0 ? 1.f : -1.f));

		float PendingTurnRate = GetPendingMaxTurnRate(Vehicle) * GPL_DegToRad;

		// �������Ӱ��Ǽ��ٶȣ����ٶ�Խ��Ǽ��ٶ�ԽС
		float DampingTorque = GetDampingTorque(Vehicle, PendingTurnRate, GPhysGameSetting._GPL_TorqueDampingFactor * ObjectDesc->_TurnTorque, Up);
		_Torque -= Up * DampingTorque * (AngVel.z > 0.f ? 1.f : -1.f);
				
		// ������Ӱ��ת���ٶ�
		// û��ת������ת��������ٶȷ���ʱ�ż���ת������
		DampingTorque = 0.f;
		if( _Strafe == 0 || IsTorqueDamping(Vehicle, ForwardVel > 0.f) )
		{
			DampingTorque = CompDesc->_ScaleDampingTurn * GetDampingTorque(Vehicle, PendingTurnRate, CompDesc->_TurnDampingTorque, Up);
		}
		_Torque -= Up * DampingTorque * (AngVel.z > 0.f ? 1.f : -1.f);
	
		// ���������ٶ�
		if( NxMath::abs(AngVel.z) > PendingTurnRate )
		{
			NxVec3 NewAng = AngVel;
			NewAng.z = PendingTurnRate * (AngVel.z > 0.f ? 1.f : -1.f);
			Vehicle->GetPhysActor()->setAngularVelocity(NewAng);
		}

		if( _Strafe == 0 && NxMath::abs(AngVel.z) < CompDesc->_StopAngThreshold * GPL_DegToRad )
		{
			_Torque = ZERO_VEC;
			Vehicle->GetPhysActor()->setAngularVelocity(ZERO_VEC);
		}

		Vehicle->GetPhysActor()->addForce(_Force);
		Vehicle->GetPhysActor()->addTorque(_Torque);
	}

	// ���ݵ�ǰ���ٶ��������ٶȼ�����ת����
	// @param	MaxTurnRate		in radians  �����ٶ�
	float VehicleSimBase::GetDampingTorque(PhysVehicle* Vehicle, float MaxTurnRate, float TurnTorque, const NxVec3& Up)
	{
		NxVec3 AngVel = Vehicle->_AngularVelocity;
		float TurnAngVel = AngVel.dot(Up);
		float Alpha = NxMath::clamp(NxMath::abs(TurnAngVel/MaxTurnRate), 1.0f, 0.0f);
		return TurnTorque * Alpha;
	}

	// ���ݵ�ǰ�ٶ�������ٶȵ���FORCE
	float VehicleSimBase::GetDampingForce(PhysVehicle* Vehicle, float InForce, float CurSpeed, float MaxSpeed)
	{
		float Alpha = 1.f;
		if( MaxSpeed > 0.f )
		{
			DampingFragment OutFragment;
			if( GPhysGameSetting.FindDampingFragment(NxMath::clamp(CurSpeed / MaxSpeed, 1.0f, 0.0f), OutFragment) )
			{
				Alpha = OutFragment.Damping;
			}
		}
		return InForce * Alpha;
	}

	NxVec3 VehicleSimBase::CaleDampingForce(const NxVec3& Velocity, float MaxSpeed, float MaxDamping, float MinDampingRate)
	{
		NxVec3 DampingForce = -Velocity;
		float Speed = DampingForce.normalize();
		float MinRate = NxMath::clamp(MinDampingRate, 1.f, 0.f);
		float SpeedFactor = NxMath::equals(MaxSpeed, 0.f, 0.001f) ? 1.f : NxMath::clamp(Speed / MaxSpeed, 1.f, MinRate);

		return DampingForce * MaxDamping * SpeedFactor * SpeedFactor;
	}

	// ���ݵ�ǰ���ֵ���������С
	float VehicleSimBase::GetRiseDampingForce(PhysVehicle* Vehicle, float MaxForce, int Rise)
	{	
		float DampingForce;
		if( Vehicle == NULL || Vehicle->_DeltaLocZ == 0.f || Rise == 0 )
			return 0.f;

		NxVec3 velocity = Vehicle->_Velocity;
	
		float  level1 = GPhysGameSetting._GPL_SubmarineAccelerate;
		float  level2 = GPhysGameSetting._GPL_SubmarineConstant;
		VehicleData* CompData = (VehicleData*)Vehicle->_ComponentData;
		float MaxLineVelZ = NxMath::sqrt(abs(level1 * Vehicle->_DeltaLocZ * MaxForce / CompData->_Mass));

		NxVec3 CurrentLoc = Vehicle->GetGlobalPosition();
		float Alpha = 1 - NxMath::clamp(abs((CurrentLoc.z - Vehicle->_DesiredLocZ) / Vehicle->_DeltaLocZ), 1.f, 0.f);

		if (Alpha < level1)
		{
			DampingForce = 0;
		}
		else if(Alpha < level2)
		{
			if ( MaxLineVelZ > abs(velocity.z) )
				DampingForce = 0;
			else
				DampingForce = MaxForce;
		}
		else 
		{
			DampingForce = MaxForce * (2 - Alpha);
		}

		if (velocity.z * Rise < 0 && Alpha > level1) //��Ǳ������ٶ��෴��
		{
			velocity.z = MaxLineVelZ * Rise;
			Vehicle->GetPhysActor()->setLinearVelocity(velocity);
		}

		return DampingForce;
	}
	
	bool VehicleSimBase::IsTorqueDamping(PhysVehicle* Vehicle, bool bForward)
	{
		NxVec3 AngVel = Vehicle->_AngularVelocity;

		if( _Strafe == 0 )
		{
			return false;
		}

		if( bForward )
		{
			if( AngVel.z > 0.f )
			{
				return _Strafe == -1;
			}
			else
			{
				return _Strafe == 1;
			}
		}
		else
		{
			if( AngVel.z > 0.f )
			{
				return _Strafe == 1;
			}
			else
			{
				return _Strafe == -1;
			}
		}
	}

	//------------------------------------------------------------------------
	//	VehicleSimShip
	//------------------------------------------------------------------------
	bool VehicleSimShip::IsValidOp(EMoveFlag Flag)
	{
		if( Flag == MoveFlag_Forward
			|| Flag == MoveFlag_Backward
			|| Flag == MoveFlag_RightTurn
			|| Flag == MoveFlag_LeftTurn 
			|| Flag == MoveFlag_CruiseDriving
			|| Flag == MoveFlag_CruiseSteering )
		{
			return true;
		}
		return false;
	}

	void VehicleSimShip::ProcessInput(PhysVehicle* Vehicle)
	{
		VehicleSimBase::ProcessInput(Vehicle);

		_bCruiseDriveFlag = false;
		if( !HaveAnyFlag(MoveFlag_Forward) && !HaveAnyFlag(MoveFlag_Backward) && HaveAnyFlag(MoveFlag_CruiseDriving) )
		{
			_bCruiseDriveFlag = true;
		}
		if( _bCruiseDriveFlag )
		{
			_Throttle = _ShiftIndexDriving > 0 ? 1 : -1;
		}

		_bCruiseSteerFlag = false;
		if( !HaveAnyFlag(MoveFlag_RightTurn) && !HaveAnyFlag(MoveFlag_LeftTurn) && HaveAnyFlag(MoveFlag_CruiseSteering) )
		{
			_bCruiseSteerFlag = true;
		}
		if( _bCruiseSteerFlag )
		{
			_Strafe = _ShiftIndexSteering > 0 ? 1 : -1;
		}

		// ��ת��ʱ�Զ��ҵ���һ��
		if( GetMoveFlag() == MoveFlag_RightTurn
			|| GetMoveFlag() == MoveFlag_LeftTurn 
			|| (_bCruiseSteerFlag && !HaveAnyFlag(MoveFlag_Forward | MoveFlag_Backward | MoveFlag_CruiseDriving)) )
		{
			_Throttle = 1;
			_ShiftIndexDriving = 1;
			_bCruiseDriveFlag = true;
		}
		else if( !HaveAnyFlag(MoveFlag_Forward | MoveFlag_Backward | MoveFlag_CruiseDriving) )
		{ // û��������ʱ����������
			_Throttle = 0;
			_ShiftIndexDriving = 0;
			_bCruiseDriveFlag = false;
		}
	}

	void VehicleSimShip::MoveForward(bool bForward)
	{
		VehicleSimBase::MoveForward(bForward);

		if( bForward )
		{
			SetShiftDriving(0);
		}
	}

	void VehicleSimShip::MoveBackward(bool bBackward)
	{
		VehicleSimBase::MoveBackward(bBackward);

		if( bBackward )
		{
			SetShiftDriving(0);
		}
	}

	void VehicleSimShip::TurnRight(bool bRight)
	{
		VehicleSimBase::TurnRight(bRight);

		if( bRight )
		{
			SetShiftSteering(0);
		}
	}

	void VehicleSimShip::TurnLeft(bool bLeft)
	{
		VehicleSimBase::TurnLeft(bLeft);

		if( bLeft )
		{
			SetShiftSteering(0);
		}
	}

	void VehicleSimShip::SetShiftDriving(int ShiftIndexDriving)
	{
		_ShiftIndexDriving = ShiftIndexDriving;

		if(_ShiftIndexDriving != 0)
		{
			SetMoveFlag(MoveFlag_CruiseDriving);
		}
		else
		{
			ClearMoveFlag(MoveFlag_CruiseDriving);
		}
	}

	void VehicleSimShip::SetShiftSteering(int ShiftIndexSteering)
	{
		_ShiftIndexSteering = ShiftIndexSteering;

		if(_ShiftIndexSteering != 0)
		{
			SetMoveFlag(MoveFlag_CruiseSteering);
		}
		else
		{
			ClearMoveFlag(MoveFlag_CruiseSteering);
		}
	}

	bool VehicleSimShip::HaveAnyMoveOp()
	{
		return (VehicleSimBase::HaveAnyMoveOp() || HaveAnyFlag(MoveFlag_CruiseDriving));
	}

	bool VehicleSimShip::HaveAnyTurnOp()
	{
		return (VehicleSimBase::HaveAnyTurnOp() || HaveAnyFlag(MoveFlag_CruiseSteering));
	}
	
	bool VehicleSimShip::IsInCruiseDrive()
	{
		return _bCruiseDriveFlag;
	}
	bool VehicleSimShip::IsInCruiseSteer()
	{
		return _bCruiseSteerFlag;
	}
	float VehicleSimShip::GetMaxForwardSpeed(PhysVehicle* Vehicle)
	{
		if( _bCruiseDriveFlag )
		{
			return Vehicle->GetCruiseSpeed(_ShiftIndexDriving, true);
		}

		return Vehicle != NULL ? Vehicle->GetMaxForwardSpeed() : 0.f;
	}

	float VehicleSimShip::GetMaxReverseSpeed(PhysVehicle* Vehicle)
	{
		if( _bCruiseDriveFlag )
		{
			return Vehicle->GetCruiseSpeed(_ShiftIndexDriving, false);
		}

		return Vehicle != NULL ? Vehicle->GetMaxReverseSpeed() : 0.f;
	}

	float VehicleSimShip::GetPendingMaxTurnRate(PhysVehicle* Vehicle)
	{
		float PendingMaxTurnRate = VehicleSimBase::GetPendingMaxTurnRate(Vehicle);

		if( _bCruiseSteerFlag )
		{
			return Vehicle->GetCruiseTurnRate(_ShiftIndexSteering, PendingMaxTurnRate);
		}

		return PendingMaxTurnRate;
	}

	//------------------------------------------------------------------------
	//	VehicleSimSubmarine
	//------------------------------------------------------------------------
	void VehicleSimSubmarine::Tick(float DeltaTime, PhysVehicle* Vehicle)
	{
		if( Vehicle != NULL && ReachDesiredLayer(Vehicle) )
		{ // �ﵽĿ���
			// ����ϸ���Ǳflag
			StopRiseOrSink(Vehicle);

			// ����֪ͨ
			if( Vehicle->GetBehaviorReport() != NULL )
			{
				Vehicle->GetBehaviorReport()->OnRiseOrSinkEnd(Vehicle);
			}
		}

		if( HaveAnyFlag(MoveFlag_Sink) )
		{
			NxVec3 NewCurrentLoc = Vehicle->GetGlobalPosition();
			if( NewCurrentLoc.z > Vehicle->_CurrLocZ )
			{
				NewCurrentLoc.z = Vehicle->_CurrLocZ;
				Vehicle->SetGlobalPosition(NewCurrentLoc);
			}
		}
		if( HaveAnyFlag(MoveFlag_Riseup) )
		{
			NxVec3 NewCurrentLoc = Vehicle->GetGlobalPosition();
			if( NewCurrentLoc.z < Vehicle->_CurrLocZ )
			{
				NewCurrentLoc.z = Vehicle->_CurrLocZ;
				Vehicle->SetGlobalPosition(NewCurrentLoc);
			}
		}
	}

	void VehicleSimSubmarine::ForceReachDesiredLayer(PhysVehicle* Vehicle)
	{
		if( Vehicle != NULL )
		{ // �ﵽĿ���
			// ����ϸ���Ǳflag
			StopRiseOrSink(Vehicle);

			// ����֪ͨ
			if( Vehicle->GetBehaviorReport() != NULL )
			{
				Vehicle->GetBehaviorReport()->OnRiseOrSinkEnd(Vehicle);
			}
		}
	}

	bool VehicleSimSubmarine::ReachDesiredLayer(PhysVehicle* Vehicle)
	{
		if( Vehicle == NULL )
			return false;

		if( HaveAnyFlag(MoveFlag_Riseup) )
		{
			NxVec3 CurrentLoc = Vehicle->GetGlobalPosition();
			return CurrentLoc.z >= Vehicle->_DesiredLocZ;
			//if( Vehicle->_OriginalLocZ == Vehicle->_DesiredLocZ )
			//{ // ���������ϲ�
			//	return CurrentLoc.z > Vehicle->_OriginalLocZ ? true : false;
			//}
			//float d1 = CurrentLoc.z - Vehicle->_OriginalLocZ;
			//float d2 = CurrentLoc.z - Vehicle->_DesiredLocZ;
			//return d1*d2 < 0.f;
		}
		if( HaveAnyFlag(MoveFlag_Sink) )
		{
			NxVec3 CurrentLoc = Vehicle->GetGlobalPosition();
			return CurrentLoc.z <= Vehicle->_DesiredLocZ;
			//if( NxMath::equals(CurrentLoc.z - Vehicle->_OriginalLocZ, Vehicle->_DesiredLocZ - Vehicle->_OriginalLocZ, 0.1f) )
			//{
			//	return true;
			//}
			//float d1 = Vehicle->_DesiredLocZ - Vehicle->_OriginalLocZ;
			//float d2 = Vehicle->_DesiredLocZ - CurrentLoc.z;
			//return d1*d2 < 0.f;
		}

		return false;
	}

	bool VehicleSimSubmarine::IsValidOp(EMoveFlag Flag)
	{
		if( Flag == MoveFlag_Forward
			|| Flag == MoveFlag_Backward
			|| Flag == MoveFlag_RightTurn
			|| Flag == MoveFlag_LeftTurn 
			|| Flag == MoveFlag_CruiseDriving
			|| Flag == MoveFlag_CruiseSteering
			|| Flag == MoveFlag_Riseup
			|| Flag == MoveFlag_Sink )
		{
			return true;
		}
		return false;
	}

	void VehicleSimSubmarine::Riseup(PhysVehicle* Vehicle)
	{
		ClearMoveFlag(MoveFlag_Sink);		// �ϸ���ǱΪ�������
		SetMoveFlag(MoveFlag_Riseup);

		UnlockPosZ(Vehicle);
	}

	void VehicleSimSubmarine::Sink(PhysVehicle* Vehicle)
	{
		ClearMoveFlag(MoveFlag_Riseup);		// �ϸ���ǱΪ�������
		SetMoveFlag(MoveFlag_Sink);

		UnlockPosZ(Vehicle);
	}

	void VehicleSimSubmarine::StopRiseOrSink(PhysVehicle* Vehicle)
	{
		ClearMoveFlag(MoveFlag_Riseup | MoveFlag_Sink);

		LockPosZ(Vehicle);

		Vehicle->OnNotifyStopRiseOrSink();
	}

	void VehicleSimSubmarine::LockPosZ(PhysVehicle* Vehicle)
	{
		if( Vehicle != NULL && Vehicle->GetPhysActor() != NULL )
		{
			Vehicle->GetPhysActor()->raiseBodyFlag(NX_BF_FROZEN_POS_Z);
		}
	}

	void VehicleSimSubmarine::UnlockPosZ(PhysVehicle* Vehicle)
	{
		if( Vehicle != NULL && Vehicle->GetPhysActor() != NULL )
		{
			Vehicle->GetPhysActor()->clearBodyFlag(NX_BF_FROZEN_POS_Z);
		}
	}

	void VehicleSimSubmarine::ProcessInput(PhysVehicle* Vehicle)
	{
		VehicleSimShip::ProcessInput(Vehicle);

		if( HaveAnyFlag(MoveFlag_Riseup) && HaveAnyFlag(MoveFlag_Sink) )
		{
			_Rise = 0;
		}
		else if( HaveAnyFlag(MoveFlag_Riseup) )
		{
			_Rise = 1;
		}
		else if( HaveAnyFlag(MoveFlag_Sink) )
		{
			_Rise = -1;
		}
		else
		{
			_Rise = 0;
		}
	}

	//------------------------------------------------------------------------
	//	VehicleSimDeath
	//------------------------------------------------------------------------
	VehicleSimDeath::VehicleSimDeath(ESimulationType SimType)
		: VehicleSimBase(SimType)
		, _Roll(0)
		, _Pitch(0)
		, _UnderSurface(false)
		, _Init(false)
	{
	}

	void VehicleSimDeath::Tick(float DeltaTime, PhysVehicle* Vehicle)
	{
		if ( !_Init )
			return;

		if ( !Vehicle )
			return;

		if ( _UnderSurface )
			return;

		if ( !_Torque.equals(ZERO_VEC, 0.001f) )
		{
			float Angle = _CapsizalPose.getAngle(Vehicle->GetGlobalOritationQuat());
			if ( NxMath::abs(Angle) < 0.01f )
			{
				_Torque = ZERO_VEC;
				Vehicle->GetPhysActor()->setAngularVelocity(ZERO_VEC);
			}
		}

		//float Deepth = NxMath::abs(Vehicle->GetGlobalPosition().z);
		//if ( Deepth > _TriggerDeepth )
		if (Vehicle->CheckUnderSurface())
		{
			//gplDebugf(TEXT("Reach Sinking-TriggerDeepth"));

			BehaviorReport* pBehaviorReport = Vehicle->GetBehaviorReport();
			if ( pBehaviorReport )
				pBehaviorReport->OnVehicleSinkUnderSurface(Vehicle);

			Vehicle->RemoveCollision();
			_UnderSurface = true;
		}
	}

	void VehicleSimDeath::UpdateVehicle(PhysVehicle* Vehicle, float DeltaTime)
	{
		if ( !_Init )
			return;

		if( Vehicle == NULL )
			return;

		if (_UnderSurface)
			return;

		if (_SkipSimulate)
			return;

		NxActor* pActor = Vehicle->GetPhysActor();
		if ( !pActor )
			return;

		VehicleData* pData = DynamicCast(VehicleData, Vehicle->_ComponentData);
		if ( !pData )
			return;

		VehicleData* ObjectDesc = (VehicleData*)Vehicle->GetComponentData();
		if( ObjectDesc == NULL )
			return;

		Vehicle->_PrevVelocity = Vehicle->_Velocity;
		Vehicle->_Velocity = pActor->getLinearVelocity();		// ��¼��ǰ�ٶ�ֵ
		Vehicle->_PrevAngularVelocity = Vehicle->_AngularVelocity;
		Vehicle->_AngularVelocity = pActor->getAngularVelocity();

		if ( Vehicle->_Velocity.z > 0 )
			pActor->setLinearVelocity(NxVec3(Vehicle->_Velocity.x, Vehicle->_Velocity.y, 0));

#if 0
		NxMat33 WorldOri = pActor->getGlobalOrientation();
		NxVec3 SrcUp = WorldOri.getColumn(2);

		NxVec3 DesUp = _CapsizalPose.rot(AXIS_VEC_Z);
		_Torque = SrcUp.cross(DesUp);
		if ( NxMath::equals(_Torque.magnitude(), 0.f, 0.001f) )
		{
			_Torque = ZERO_VEC;
		}
		_Torque.normalize();
		_Torque *= _TorqueFactor * pData->_Mass;
#endif

#if 0
		//	�ٶ�����
		if ( NxMath::abs(Vehicle->_Velocity.x) < 0.01f )
			Vehicle->_Velocity.x = 0;
		if ( NxMath::abs(Vehicle->_Velocity.y) < 0.01f )
			Vehicle->_Velocity.y = 0;
		if ( NxMath::abs(Vehicle->_Velocity.z) < 0.01f )
			Vehicle->_Velocity.z = 0;
		pActor->setLinearVelocity(Vehicle->_Velocity);
#endif
		//	׷��ˮƽ�����������
		NxVec3 HoriVel(Vehicle->_Velocity.x, Vehicle->_Velocity.y, 0);
		pActor->addForce(CalcDampingForce(HoriVel, pData->_Mass));

		pActor->addForce(_Force + CalcDampingForce(Vehicle->_Velocity, pData->_Mass));
		pActor->addTorque(_Torque + CalcDampingTorque(Vehicle->_AngularVelocity, pData->_Mass));
		//pActor->addLocalTorque(AXIS_VEC_X * _TorqueFactor * pData->_Mass * NxMath::degToRad(_Roll));
		//pActor->addLocalTorque(AXIS_VEC_Y * _TorqueFactor * pData->_Mass * NxMath::degToRad(_Pitch));

//#ifdef _CLIENT_RUNTIME
//		gplDebugf(TEXT("Force %f, %f, %f"), _Force.x, _Force.y, _Force.z);
//		gplDebugf(TEXT("Torque %f, %f, %f"), _Torque.x, _Torque.y, _Torque.z);
//		gplDebugf(TEXT("Sink Speed[%f]"), Vehicle->_Velocity.magnitude());
//		gplDebugf(TEXT("Capsize Speed[%f]"), Vehicle->_AngularVelocity.magnitude());
//		NxVec3 Pos = Vehicle->GetGlobalPosition();
//		gplDebugf(TEXT("Location[%f, %f, %f]"), Pos.x, Pos.y, Pos.z);
//		gplDebugf(TEXT("AngleVel[%f, %f, %f]"), Vehicle->_AngularVelocity.x, Vehicle->_AngularVelocity.y, Vehicle->_AngularVelocity.z);
//		gplDebugf(TEXT("Torque[%f, %f, %f]"), _Torque.x, _Torque.y, _Torque.z);
//		NxVec3 DampingTorque = CalcDampingTorque(Vehicle->_AngularVelocity, pData->_Mass);
//		gplDebugf(TEXT("DampingTorque[%f, %f, %f]"), DampingTorque.x, DampingTorque.y, DampingTorque.z);
//		NxQuat Rot = pActor->getGlobalOrientationQuat();
//		gplDebugf(TEXT("Rotation[%f, %f, %f, %f]"), Rot.x, Rot.y, Rot.z, Rot.w);
//#endif // _CLIENT_RUNTIME
	}

	void VehicleSimDeath::RandomPose()
	{
		_Roll = NxMath::rand(-GPhysGameSetting._GPL_Sink_Roll_Limit, GPhysGameSetting._GPL_Sink_Roll_Limit);
		_Pitch = NxMath::rand(-GPhysGameSetting._GPL_Sink_Pitch_Limit, GPhysGameSetting._GPL_Sink_Pitch_Limit);
		_ForceFactor = NxMath::rand(GPhysGameSetting._GPL_Sink_ForceFactor_LowerLimit, GPhysGameSetting._GPL_Sink_ForceFactor_UpperLimit);
		_TorqueFactor = NxMath::rand(GPhysGameSetting._GPL_Sink_TorqueFactor_LowerLimit, GPhysGameSetting._GPL_Sink_TorqueFactor_UpperLimit);
	}

	void VehicleSimDeath::InitPhysParams(PhysVehicle* Vehicle)
	{
		if ( !Vehicle )
			return;

		NxActor* pActor = Vehicle->GetPhysActor();
		if ( !pActor )
			return;

		VehicleData* pData = DynamicCast(VehicleData, Vehicle->_ComponentData);
		if ( !pData )
			return;

		_Init = true;
		_SkipSimulate = false;

		//	Ǳˮͧ��ˮ�����³�û��ֱ��������ײ��������������
		if ( Vehicle->_CurLevel != 0 )
		{
			BehaviorReport* pBehaviorReport = Vehicle->GetBehaviorReport();
			if ( pBehaviorReport )
				pBehaviorReport->OnVehicleSinkUnderSurface(Vehicle);

			pActor->setLinearVelocity(ZERO_VEC);
			pActor->setAngularVelocity(ZERO_VEC);
			pActor->addForce(ZERO_VEC);
			pActor->addTorque(ZERO_VEC);

			Vehicle->RemoveCollision();
			_UnderSurface = true;
			return;
		}

		//	Reset MassCenter
		//pActor->setCMassOffsetLocalPosition(ZERO_VEC);
		_UnderSurface = false;

		//	Init Force
		_Force = -AXIS_VEC_Z * _ForceFactor * pData->_Mass;

		//	Init Torque
		NxVec3 DirX, DirY, DirZ;
		NxMat33 WorldOri = pActor->getGlobalOrientation();
		DirX = WorldOri.getColumn(0);
		DirY = WorldOri.getColumn(1);
		DirZ = WorldOri.getColumn(2);

		NxQuat QRoll;
		QRoll.fromAngleAxis(_Roll, AXIS_VEC_X);

		NxQuat QPitch;
		QPitch.fromAngleAxis(_Pitch, AXIS_VEC_Y);

		WorldOri.toQuat(_CapsizalPose);	//	Yaw
		_CapsizalPose *= QPitch;
		_CapsizalPose *= QRoll;

		NxVec3 DesUp = _CapsizalPose.rot(AXIS_VEC_Z);
		_Torque = AXIS_VEC_Z.cross(DesUp);
		_Torque.normalize();
		_Torque *= _TorqueFactor * pData->_Mass;

		//	Init Trigger Deepth
		NxVec3 DesForward = _CapsizalPose.rot(AXIS_VEC_X);
		DesForward *= pData->_Length * 0.5f;
		DesUp *= pData->_Height * 0.8f;
		_TriggerDeepth = NxMath::max(NxMath::abs(DesForward.z), NxMath::abs(DesUp.z));
		//gplDebugf(TEXT("Trigger Deepth = %f"), _TriggerDeepth);
	}

	NxVec3 VehicleSimDeath::CalcDampingForce(const NxVec3& LineVel, float Mass)
	{
		NxVec3 Damping = -LineVel;
		Damping.normalize();

		float fScale = LineVel.magnitude();
		Damping *= (fScale * fScale) * Mass * GPhysGameSetting._GPL_Sink_DampingFactor;

		//gplDebugf(TEXT("Velocity[%f, %f, %f]"), LineVel.x, LineVel.y, LineVel.z);
		//gplDebugf(TEXT("Damping[%f, %f, %f]"), Damping.x, Damping.y, Damping.z);

		return Damping;
	}

	NxVec3 VehicleSimDeath::CalcDampingTorque(const NxVec3& AngVel, float Mass)
	{
		NxVec3 Damping = -AngVel;
		Damping.normalize();

		float fScale = AngVel.magnitude();
		Damping *= (fScale * fScale) * Mass * GPhysGameSetting._GPL_Sink_Torque_DampingFactor;

		//gplDebugf(TEXT("AngleVelocity[%f, %f, %f]"), AngVel.x, AngVel.y, AngVel.z);
		//gplDebugf(TEXT("DampingTorque[%f, %f, %f]"), Damping.x, Damping.y, Damping.z);

		return Damping;
	}
}