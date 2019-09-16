#include "..\Inc\PhysXSupport.h"


// 角度归一化至[0, 360]
float FixAngle( float angle )
{
	angle = fmod( angle, 360 );
	while( angle < 0 )
		angle += 360;
	return angle;
}
		
namespace GPL
{
		IMPLEMENT_RTTI_BASE(GameObject)
		IMPLEMENT_RTTI(ShipComponent, GameObject)
		IMPLEMENT_RTTI(Turrent, ShipComponent)

		bool Turrent::InitShipComponent(ComponentData* ComponentData)
		{
			if( !ShipComponent::InitShipComponent(ComponentData) )
			{
				return false;
			}
			_DesiredPitch = _DesiredYaw = _CurrentYaw = _CurrentPitch = 0;

			_FirePartCount= ((WeaponData*)_ComponentData)->_FirePartNum;
			for (int n=0;n<MaxFirePartNum;n++)
			{
				_FirePartsAccuracy[n]=0;
			}

			_TurnDegree=0.0f;
			_CanRotateTo = false;
			_bValidTargetLoc = false;

			return true;
		}

		void Turrent::Term()
		{
			_Appendent = NULL;
			__super::Term();
		}
		
		void Turrent::Tick(float DeltaTime)
		{
			ShipComponent::Tick(DeltaTime);

			if (!_Activated)
				return;
			
			TickAccuracy(DeltaTime);

			TickRotation(DeltaTime);

			if (NULL != _Appendent)
				_Appendent->Tick(DeltaTime);
		}

		void Turrent::TickAccuracy(float DeltaTime)
		{
			WeaponData* CompData = DynamicCast(WeaponData, _ComponentData);
			if ( !CompData )
				return;

			WeaponCompDesc* Desc = DynamicCast(WeaponCompDesc, _ComponentData->_CompDesc);
			if ( !Desc )
				return;

			float TargetAccuracy = 0.f;
			TargetAccuracy += GetAccuracyOfLinearVelocity();
			TargetAccuracy += GetAccuracyOfAngularVelocity();
			TargetAccuracy += GetAccuracyOfTurrentTurn();
			TargetAccuracy = NxMath::min(TargetAccuracy, Desc->_AccuracyTotalMax * GLOBAL_DATA_MULTIPLICATOR);

			for (int n = 0; n < _FirePartCount; n++)
			{
				float &CurAccuracy = _FirePartsAccuracy[n];

				// 目标精准度大于当前时，立即更新
				if( TargetAccuracy > CurAccuracy )
					CurAccuracy = TargetAccuracy;

				CurAccuracy -= CompData->_AccuracyRecover * DeltaTime;
				CurAccuracy = NxMath::max(CurAccuracy, 0.f);
			}
		}

		int Turrent::GetFirePartCount()
		{
			return _FirePartCount;
		}

		float Turrent::GetAccuracyOfLinearVelocity()
		{
			WeaponData* CompData = (WeaponData*)_ComponentData;

			float VehicleMaxDriver=_Owner->GetInitializedMaxForwardSpeed();
			float VehicleDrive = _Owner->_VelocitySize;
			if ( VehicleMaxDriver != 0.f )
			{
				float Alpha = NxMath::clamp(VehicleDrive / VehicleMaxDriver, 1.f, 0.f);
				std::vector<ReduceLevel>::iterator it = GPhysGameSetting._AccuracyDrivingReduce.begin();
				while (it != GPhysGameSetting._AccuracyDrivingReduce.end())
				{
					if (Alpha > it->SpeedRate)
						return CompData->_AccuracyShipDrive * it->ReduceFactor;

					it++;
				}
			}

			return 0.f;
		}

		float Turrent::GetAccuracyOfAngularVelocity()
		{
			WeaponData* CompData = (WeaponData*)_ComponentData;

			float VehicleMaxSteer = _Owner->GetMaxTurnRate();
			float VehicleSteer = _Owner->_AngularVelocityZSize;

			if ( VehicleMaxSteer != 0.f )
			{
				float Alpha = VehicleSteer * GPL_RadToDeg / VehicleMaxSteer;
				Alpha = NxMath::min(1.f, Alpha);
				return CompData->_AccuracyShipSteer * Alpha;
			}
			return 0.f;
		}

		float Turrent::GetAccuracyOfTurrentTurn()
		{
			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
			WeaponData* CompData = (WeaponData*)_ComponentData;

			float TurnAccuracy = CompData->_AccuracyTurrent * _TurnDegree;
			TurnAccuracy = NxMath::min(TurnAccuracy, Desc->_AccuracyTurrentMax * GLOBAL_DATA_MULTIPLICATOR);
			return TurnAccuracy;
		}

		float Turrent::GetMaxAccuracy()
		{
			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;

			return Desc->_AccuracyTotalMax * GLOBAL_DATA_MULTIPLICATOR;
		}

		float Turrent::GetCurAccuracy(int FirePartID)
		{
			float CurAccuracyMultiplier = 1.f;
			if ( _Owner != NULL )
				CurAccuracyMultiplier = _Owner->GetLimitAccuracy();

			return (1 + _FirePartsAccuracy[FirePartID]) * CurAccuracyMultiplier;
		}

		void Turrent::SetFired(int FirePartID)
		{
			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
			WeaponData* CompData = (WeaponData*)_ComponentData;
			float& CurAccuracy = _FirePartsAccuracy[FirePartID];
			CurAccuracy += CompData->_AccuracyFire;			// 开火对精准度的影响

			if (CurAccuracy > Desc->_AccuracyTotalMax*GLOBAL_DATA_MULTIPLICATOR)
			{
				CurAccuracy = Desc->_AccuracyTotalMax*GLOBAL_DATA_MULTIPLICATOR;
			}
		}

		bool Turrent::IsAimingExact()
		{
			for (int idx = 0; idx < _FirePartCount; idx++)
			{
				if ( _FirePartsAccuracy[idx] > 0.001f )
					return false;
			}

			return true;
		}

		float Turrent::GetFinalAccuracy(int GunTubeIndex)
		{
			return GetCurAccuracy(GunTubeIndex) * ((WeaponData*)_ComponentData)->_AccuracyDefault;
		}

		void Turrent::TickRotation(float DeltaTime)
		{
			if (NULL != _Owner)
			{
				if( _bValidTargetLoc && _Owner->GetActivePartType() == _Socket._PartType )
					TickAimTarget(DeltaTime);
				else if (_Socket._PartType == EPT_SearchlightLauncher)
				{
					AssociatedTrigger* Trig = DynamicCast(AssociatedTrigger, _Appendent);
					if (NULL != Trig && Trig->IsActivated())
						TickAimTarget(DeltaTime);
				}
			}

			if ( !NxMath::equals(_CurrentPitch, _DesiredPitch, GPL_KINDA_SMALL_NUMBER) )
			{
				WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
				float PitchSpeed = Desc->_GunTubeAngleVelocity;
				if( _DesiredPitch - _CurrentPitch >= 0.f )
				{
					_CurrentPitch += PitchSpeed * DeltaTime;
					_CurrentPitch = NxMath::clamp(_CurrentPitch, _DesiredPitch, _CurrentPitch);
				}
				else
				{
					_CurrentPitch += PitchSpeed * DeltaTime * -1.f;
					_CurrentPitch = NxMath::clamp(_CurrentPitch, _CurrentPitch, _DesiredPitch);
				}
			}

			//if( _Socket._AttachedId == 22 && (_Owner->GetActivePartType() == EPT_MainArtillery))
			//{
			//	gplDebugf(TEXT("==============_CurrentYaw: %.4f _DesiredYaw: %.4f"), _CurrentYaw, _DesiredYaw);
			//}

			if ( !NxMath::equals(_CurrentYaw, _DesiredYaw, 0.5f* DeltaTime) )
			{
				float LastYaw = _CurrentYaw;

				float YawSpeed = ((WeaponData*)_ComponentData)->_TurretAngleVelocity;
				_CurrentYaw = DoFastRotate( YawSpeed * DeltaTime, _CurrentYaw, _DesiredYaw );

				_TurnDegree += abs(_CurrentYaw - LastYaw);
			}
			else
			{
				_CurrentYaw = _DesiredYaw;

				_TurnDegree = 0.f;
			}
			_RelativeRotation = _InitialRelativeRotation * NxQuat(_CurrentYaw , NxVec3(0,0,1));

			//if( _Socket._AttachedId == 22 && _Owner->GetActivePartType() == EPT_MainArtillery )
			//{
			//	//gplDebugf(TEXT("AttachedId: %d  _TurnDegree: %.4f"), _Socket._AttachedId, _TurnDegree);

			//	//gplDebugf(TEXT("_CurrentYaw: %.4f _DesiredYaw: %.4f  _TurnDegree: %.4f"), _CurrentYaw, _DesiredYaw, _TurnDegree);
			//	//FiringParam params;
			//	//GetFireParam(params);
			//	//gplDebugf(TEXT("____StartDir: %.4f  %.4f  %.4f"), params.StartDir.x,params.StartDir.y,params.StartDir.z);
			//}
			//if( _Socket._AttachedId == 25 && (_Owner->GetActivePartType() == EPT_MainArtillery))
			//{
			//	gplDebugf(TEXT("_CurrentYaw: %.4f		_DesiredYaw: %.4f"), _CurrentYaw, _DesiredYaw);
			//	FiringParam params;
			//	GetFireParam(params);
			//	gplDebugf(TEXT("____StartDir: %.4f  %.4f  %.4f"), params.StartDir.x,params.StartDir.y,params.StartDir.z);
			//}
		}

		float Turrent::DoFastRotate(float Step, float CurrentYaw, float DesiredYaw)
		{
			if( NxMath::abs(DesiredYaw - CurrentYaw) < Step )
			{
				return DesiredYaw;
			}

			if (DesiredYaw < CurrentYaw)
			{
				CurrentYaw -= Step;
			}
			else
			{
				CurrentYaw += Step;
			}
			return CurrentYaw;
		}
		void Turrent::UpdatePose(float DeltaTime)
		{
			NxMat34 LocalPose(NxMat33(_RelativeRotation), _RelativeLocation);
			for(NxU32 i = 0; i < _Shapes.size(); ++i)
			{
				NxMat34 InitLocalPose = _InitShapeLocalPose[i];

				_Shapes[i]->setLocalPose(LocalPose * InitLocalPose);
			}
		}
		int Turrent::SetPreAimTarget(NxVec3 TargetLoc)
		{
			WeaponData* Data = DynamicCast(WeaponData, _ComponentData);
			NX_ASSERT(Data);
			WeaponCompDesc* Desc = DynamicCast(WeaponCompDesc, _ComponentData->_CompDesc);
			NX_ASSERT(Desc);

			NxVec3 StartLoc = GetGlobalPose().t;

			// calc _DesiredYaw
			NxMat33 ShipOri = _Owner->GetGlobalOritation();
			float ShipWorldYaw = atan2( ShipOri.getColumn(0).y, ShipOri.getColumn(0).x) * GPL_RadToDeg;
			ShipWorldYaw = FixAngle(ShipWorldYaw);

			NxVec3 DesiredDir;
			if (_Socket._PartType == EPT_Torpedo || _Socket._PartType == EPT_TorpedoBack)
				DesiredDir = TargetLoc;
			else
				DesiredDir = TargetLoc - StartLoc;

			DesiredDir.normalize();
			float DesiredWorldYaw = atan2( DesiredDir.y, DesiredDir.x) * GPL_RadToDeg;	// return value [-180, 180]
			DesiredWorldYaw = FixAngle(DesiredWorldYaw);

			SetPreDesiredYaw(DesiredWorldYaw - ShipWorldYaw - _Socket._InitialRotation);

			// calc _DesiredPitch
			float DesiredPitch;
			int InRange = 0;
			if( Desc->_TrajectoryMethod == ETM_Parabola )
			{
				float Gravity = Data->_Gravity;
				float InitVelocity = Data->_InitVelocity;
				bool bExceedFarmost = !CalcProjectileAngular(StartLoc, TargetLoc, InitVelocity, Gravity, DesiredPitch);
				
				if( bExceedFarmost )
				{
					InRange = 1;		// 最远射程外

					DesiredPitch = Desc->_GunTubeLimitAngleUpper;
				}
				else
				{
					InRange = 0;

					if ( DesiredPitch > Desc->_GunTubeLimitAngleUpper )
					{
						//	理论仰角大于配置射程仰角，切炮弹落点在配置限制以内的情况
						InRange = 1;
					}
					else if( DesiredPitch < Desc->_GunTubeLimitAngleLower )
					{
						InRange = 2;	// 最近射程内
					}
				}
				
				SetPreDesiredPitch(NxMath::clamp(DesiredPitch, Desc->_GunTubeLimitAngleUpper, Desc->_GunTubeLimitAngleLower));
			}
			else if (Desc->_TrajectoryMethod == ETM_Straight)
			{
				float PitchLimitUpper = Desc->_GunTubeLimitAngleUpper;
				float pitch = atan2( DesiredDir.z ,sqrt(DesiredDir.x*DesiredDir.x+DesiredDir.y*DesiredDir.y)) * GPL_RadToDeg;				
				DesiredPitch = pitch > PitchLimitUpper ? PitchLimitUpper : pitch;
				SetPreDesiredPitch(DesiredPitch);

				InRange = 0;
			}
			return InRange;
		}

		void Turrent::TickAimTarget(float DeltaTime)
		{
			NxVec3 StartLoc = GetGlobalPose().t;

			// calc _DesiredYaw
			NxMat33 ShipOri = _Owner->GetGlobalOritation();
			float ShipWorldYaw = NxMath::atan2( ShipOri.getColumn(0).y, ShipOri.getColumn(0).x) * GPL_RadToDeg;
			ShipWorldYaw = FixAngle(ShipWorldYaw);

			NxVec3 DesiredDir;
			if( _Socket._PartType == EPT_Torpedo 
				|| _Socket._PartType == EPT_TorpedoBack )
			{
				DesiredDir = _TargetLoc;
			}
			else
				DesiredDir= _TargetLoc - StartLoc;

			DesiredDir.normalize();
			float DesiredWorldYaw = NxMath::atan2( DesiredDir.y, DesiredDir.x) * GPL_RadToDeg;	// return value [-180, 180]
			DesiredWorldYaw = FixAngle(DesiredWorldYaw);

			SetDesiredYaw(DesiredWorldYaw - ShipWorldYaw - _Socket._InitialRotation);

			// calc _DesiredPitch
			float DesiredPitch;
			if( ((WeaponCompDesc*)_ComponentData->_CompDesc)->_TrajectoryMethod == ETM_Parabola )
			{
				float Gravity = ((WeaponData*)_ComponentData)->_Gravity;
				float InitVelocity = ((WeaponData*)_ComponentData)->_InitVelocity;
				bool bExceedFarmost = !CalcProjectileAngular(StartLoc, _TargetLoc, InitVelocity, Gravity, DesiredPitch);
				if( bExceedFarmost )
				{
					//DesiredPitch = _CurrentPitch;
					DesiredPitch = ((WeaponCompDesc*)_ComponentData->_CompDesc)->_GunTubeLimitAngleUpper;
				}
				SetDesiredPitch(DesiredPitch);
			}
			else if (((WeaponCompDesc*)_ComponentData->_CompDesc)->_TrajectoryMethod == ETM_Straight)
			{
				WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
				float PitchLimitUpper = Desc->_GunTubeLimitAngleUpper;
				float pitch = NxMath::atan2( DesiredDir.z ,sqrt(DesiredDir.x*DesiredDir.x+DesiredDir.y*DesiredDir.y)) * GPL_RadToDeg;				
				DesiredPitch = pitch>PitchLimitUpper?PitchLimitUpper:pitch;
				SetDesiredPitch(DesiredPitch);
			}
		}

		void Turrent::SetAimTarget(NxVec3 TargetLoc)
		{
			_TargetLoc = TargetLoc;
			_bValidTargetLoc = true;
		}

		void Turrent::SetPreDesiredYaw(float DesiredYaw)
		{
			if (_Owner != NULL)
			{
				_PreDesiredYaw = FixAngle(DesiredYaw);
				_PreCanRotateTo = GetAvailableYaw(_PreDesiredYaw, _CurrentYaw, _Socket._YawCWLimit, -_Socket._YawCCWLimit, false);
			}
			else
			{
				_PreCanRotateTo = true;
			}
		}
		void Turrent::SetDesiredYaw( float DesiredYaw )
		{
			if (_Owner != NULL)
			{
				_DesiredYaw = FixAngle(DesiredYaw);
				_CanRotateTo = GetAvailableYaw(_DesiredYaw, _CurrentYaw, _Socket._YawCWLimit, -_Socket._YawCCWLimit, true);
			}
			else
			{
				_CanRotateTo = true;
			}
		}

		bool Turrent::GetAvailableYaw(float& val, float& ref, float max, float min, bool fix)
		{
			float overUpperLimit = 0;
			float overLowerLimit = 0;
			bool ret = true;

			if (val < min)
			{
				while (val < min)
				{
					overLowerLimit = min - val;
					val += 360.0f;
				}
				if (val > max)
				{
					overUpperLimit = val - max;
				}
				else
				{
					overLowerLimit = 0;
				}
			}
			else if (val > max)
			{
				while (val > max)
				{
					overUpperLimit = val - max;
					val -= 360.0f;
				}
				if (val < min)
				{
					overLowerLimit = min - val;
				}
				else
				{
					overUpperLimit = 0;
				}
			}

			// val is out of range
			if (overUpperLimit != 0 && overLowerLimit != 0)
			{
				val = (overLowerLimit > overUpperLimit ? max : min);
				ret = false;
			}

			if (fix)
			{
				float tmp = (ref < val ? (ref + 360.0f) : (ref - 360.0f));
				if (tmp >= min && tmp <= max && NxMath::abs(tmp - val) < NxMath::abs(val - ref))
				{
					ref = tmp;
				}

				tmp = (val < ref ? (val + 360.0f) : (val - 360.0f));
				if (tmp >= min && tmp <= max && NxMath::abs(tmp - ref) < NxMath::abs(val - ref))
				{
					val = tmp;
				}
			}

			return ret;
		}

		void Turrent::SetPreDesiredPitch(float pitch)
		{
			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
			float PitchLimitUpper = Desc->_GunTubeLimitAngleUpper;
			float PitchLimitLower = Desc->_GunTubeLimitAngleLower;
			_PreDesiredPitch = NxMath::clamp(pitch, PitchLimitUpper, PitchLimitLower);
		}
		void Turrent::SetDesiredPitch( float pitch )
		{
			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
			float PitchLimitUpper = Desc->_GunTubeLimitAngleUpper;
			float PitchLimitLower = Desc->_GunTubeLimitAngleLower;
			_DesiredPitch = NxMath::clamp(pitch, PitchLimitUpper, PitchLimitLower);
		}
		
		void Turrent::GetFireParam(FiringParam& params)
		{
			NxMat34 GlobalPose = GetGlobalPose();

			params.StartPos = GlobalPose.t;

			NxVec3 PitchAxis = GlobalPose.M.getColumn(1);
			NxQuat PitchQuat((360.f - _CurrentPitch), PitchAxis);

			params.StartDir = GlobalPose.M.getColumn(0);
			params.StartDir.normalize();
			params.StartDir = PitchQuat.rot(params.StartDir);

			params.DeltaYaw = NxMath::abs(_CurrentYaw - _DesiredYaw);
		}

		NxVec3 Turrent::GetPreDesireDir()
		{
			NxMat33 DesiredGlobalOritation(_Owner->GetGlobalOritationQuat() * NxQuat(_PreDesiredYaw+_Socket._InitialRotation, NxVec3(0,0,1)));
			NxVec3 PitchAxis = DesiredGlobalOritation.getColumn(1);
			NxQuat PitchQuat((360.f - _PreDesiredPitch), PitchAxis);

			NxVec3 Dir = DesiredGlobalOritation.getColumn(0);
			Dir.normalize();
			Dir = PitchQuat.rot(Dir);
			return Dir;
		}

		NxVec3 Turrent::GetDesireDir()
		{
			NxMat33 DesiredGlobalOritation(_Owner->GetGlobalOritationQuat() * NxQuat(_DesiredYaw+_Socket._InitialRotation, NxVec3(0,0,1)));
			NxVec3 PitchAxis = DesiredGlobalOritation.getColumn(1);
			NxQuat PitchQuat((360.f - _DesiredPitch), PitchAxis);

			NxVec3 Dir = DesiredGlobalOritation.getColumn(0);
			Dir.normalize();
			Dir = PitchQuat.rot(Dir);
			return Dir;
		}
		
		NxVec3 Turrent::GetDesireDir(float DesiredPitch)
		{
			NxMat33 DesiredGlobalOritation(_Owner->GetGlobalOritationQuat() * NxQuat(_DesiredYaw+_Socket._InitialRotation, NxVec3(0,0,1)));
			NxVec3 PitchAxis = DesiredGlobalOritation.getColumn(1);
			NxQuat PitchQuat((360.f - DesiredPitch), PitchAxis);

			NxVec3 Dir = DesiredGlobalOritation.getColumn(0);
			Dir.normalize();
			Dir = PitchQuat.rot(Dir);
			return Dir;
		}

		bool Turrent::IsInFirePressureRange(float InYaw)
		{
			InYaw = FixAngle(InYaw);

			std::vector<LimitAngle>::iterator it = _Socket._FirePressureLimitAngles.begin();
			for (; it != _Socket._FirePressureLimitAngles.end(); it++)
			{
				if (InYaw >= it->_AngleMin && InYaw <= it->_AngleMax)
					return true;
			}

			return false;
		}

		bool Turrent::IsTargetInFireLimitAngles(NxVec3 TargetLoc)
		{
			NxVec3 StartLoc = GetGlobalPose().t;
			NxVec3 DesiredDir = TargetLoc - StartLoc;
			DesiredDir.normalize();
			float DesiredWorldYaw = atan2( DesiredDir.y, DesiredDir.x) * GPL_RadToDeg;	// return value [-180, 180]
			DesiredWorldYaw = FixAngle(DesiredWorldYaw);

			// calc _DesiredYaw
			NxMat33 ShipOri = _Owner->GetGlobalOritation();
			float ShipWorldYaw = atan2( ShipOri.getColumn(0).y, ShipOri.getColumn(0).x) * GPL_RadToDeg;
			ShipWorldYaw = FixAngle(ShipWorldYaw);

			float DeltaYaw = DesiredWorldYaw - ShipWorldYaw - _Socket._InitialRotation;
			DeltaYaw = FixAngle(DeltaYaw);
			bool CanRotateTo = false;
			if ( _Owner != NULL )
			{
				// 转化为相对_Socket._InitialRotation角度值
				float NormalizeYawCCWLimit = 360 - _Socket._YawCCWLimit;
				if( DeltaYaw >= NormalizeYawCCWLimit || DeltaYaw <= _Socket._YawCWLimit )
				{
					CanRotateTo = true;
				}
			}
			else
			{
				CanRotateTo = true;
			}
			if (!CanRotateTo)
				return false;

			return IsInFireLimitAngles(DeltaYaw);
		}

		bool Turrent::UIsTargetInFireLimitAngles(NxVec3 TargetLoc)
		{
			return IsTargetInFireLimitAngles(TargetLoc * G2PScale);
		}

		float Turrent::GetTotalYawDelta()
		{
			if (NxMath::equals(_CurrentYaw, _DesiredYaw, GPL_KINDA_SMALL_NUMBER))
				return -1;

			return NxMath::abs(_CurrentYaw - _DesiredYaw);
		}

		bool Turrent::IsInFireLimitAngles(float InYaw)
		{
			InYaw = FixAngle(InYaw);
			for (size_t n=0;n<_Socket._FireLimitAngles.size();n++)
			{
				float MinAngle = _Socket._FireLimitAngles[n]._AngleMin;
				float MaxAngle = _Socket._FireLimitAngles[n]._AngleMax;
				if (InYaw >= MinAngle && InYaw <= MaxAngle)
					return true;
			}

			return false;
		}

		bool Turrent::IsPreDesireYawAvailabe()
		{
			if (!_Socket._EnableLimit)
			{ // 不需要转向至目标就可射击，始终返回TRUE，例如深水炸弹
				return true;
			}
			
			if(!_PreCanRotateTo)
			{ // 没有在旋转角度限制内，必定不可开火
				return false;
			}

			return IsInFireLimitAngles(_PreDesiredYaw);
		}

		bool Turrent::IsDesireYawAvailabe()
		{
			if (!_Socket._EnableLimit)
			{ // 不需要转向至目标就可射击，始终返回TRUE，例如深水炸弹
				return true;
			}
			
			if(!_CanRotateTo)
			{ // 没有在旋转角度限制内，必定不可开火
				return false;
			}

			return IsInFireLimitAngles(_DesiredYaw);
		}

#if 1
		bool Turrent::IsFireAvailabe()
		{
			if (!_Socket._EnableLimit)
			{ // 不需要转向至目标就可射击，始终返回TRUE，例如深水炸弹
				return true;
			}

			if(!_CanRotateTo)
			{ // 没有在旋转角度限制内，必定不可开火
				return false;
			}

			if( NxMath::abs(_CurrentYaw - _DesiredYaw) > GPhysGameSetting._GPL_FIRE_AVAILABLE_TOLERANCE )
			{ // 没有转向至目标朝向，不能开火
				return false;
			}

			return IsInFireLimitAngles(_CurrentYaw);
		}
#else
		bool Turrent::IsFireAvailabe()
		{
			//	不需要转向至目标就可射击，始终返回TRUE，例如深水炸弹
			if (!_Socket._EnableLimit)
				return true;

#ifdef _CLIENT_RUNTIME
			if ( GFireAimingCheck )
			{
				//	没有在旋转角度限制内，不可开火
				if (!_CanRotateTo) 
					return false;

				//	没有转向至目标朝向，不能开火
				if ( NxMath::abs(_CurrentYaw - _DesiredYaw) > GPhysGameSetting._GPL_FIRE_AVAILABLE_TOLERANCE )
					return false;
			}
#endif // _CLIENT_RUNTIME

			return IsInFireLimitAngles(_CurrentYaw);
		}

		bool Turrent::IsAimingFit()
		{
			if (!_Socket._EnableLimit)
			{ // 不需要转向至目标就可射击，始终返回TRUE，例如深水炸弹
				return true;
			}

			if(!_CanRotateTo)
			{ // 没有在旋转角度限制内，必定不可开火
				return false;
			}

			if( NxMath::abs(_CurrentYaw - _DesiredYaw) > GPhysGameSetting._GPL_FIRE_AVAILABLE_TOLERANCE )
			{ // 没有转向至目标朝向，不能开火
				return false;
			}

			return IsInFireLimitAngles(_CurrentYaw);
		}
#endif

		void Turrent::GetRealFireParam( FiringParam& params )
		{
			GetFireParam(params);	

			WeaponCompDesc* Desc = (WeaponCompDesc*)_ComponentData->_CompDesc;
			if ( !Desc || params.PartID >= _FirePartCount )
				return;

			// no seed
			float AccuracyDefault = ((WeaponData*)_ComponentData)->_AccuracyDefault;
			float Accuracy = GetCurAccuracy(params.PartID);

#ifdef _SERVER_RUNTIME
			Accuracy *= _Owner->GetCriticalFireAccuracyMultiplier(_Socket._PartType);
#endif // _SERVER_RUNTIME

			float ConeHalfAngleRad = NxMath::atan(Accuracy * AccuracyDefault / 100.f);
			float Diffuse = RangeGaussian(((WeaponData*)_ComponentData)->_ArtilleryDiffuseParam);
			ConeHalfAngleRad *= Diffuse;
			//ConeHalfAngleRad = NxMath::rand(0.f, ConeHalfAngleRad);

			float SphereAngleRad = NxMath::rand(0.f, NxTwoPi);
			
			params.StartDir = GPL::PickCone(params.StartDir, ConeHalfAngleRad, SphereAngleRad);
		}

		void Turrent::UGetRealFireParam( FiringParam& params )
		{
			GetRealFireParam(params);
			params.StartPos *= P2GScale;
		}


		//////////////////////////////////////

		bool ShipComponent::InitShipComponent(ComponentData* ComponentData)
		{
			_UserData = NULL;
			_ComponentData = ComponentData;
			if( _ComponentData != NULL )
			{
				_AttachedId = ((VehicleAttachedCompData*)_ComponentData)->_AttachedId;
			}
			return _ComponentData != NULL ? true : false;
		}

		void ShipComponent::AttachTo(PhysVehicle* Owner, Socket AttachedTo)
		{
			_Owner = Owner;
			_Socket = AttachedTo;

			_InitialRelativeLocation = _Socket._LocalPose.t;
			_InitialRelativeRotation.fromAngleAxisFast(_Socket._InitialRotation * GPL_DegToRad, NxVec3(0,0,1));
			
			_RelativeLocation = _InitialRelativeLocation;
			_RelativeRotation = _InitialRelativeRotation;
			
			if( _ComponentData == NULL )
			{
				gplDebugf(TEXT("AttachTo失败，_ComponentData==NULL"));
				return;
			}

			if (_Socket._InitialRotation < 0 || _Socket._InitialRotation >= 360.0f)
			{
				_Socket._InitialRotation = FixAngle(_Socket._InitialRotation);
			}
			if (_Socket._YawCWLimit < 0 || _Socket._YawCWLimit > 360.0f)
			{
				_Socket._YawCWLimit = FixAngle(_Socket._YawCWLimit);
			}
			if (_Socket._YawCCWLimit < 0 || _Socket._YawCCWLimit > 360.0f)
			{
				_Socket._YawCCWLimit = FixAngle(_Socket._YawCCWLimit);
			}
			

			// create shapes
			// LocalPose是SOCKET在OWNER space局部位置信息
			NxMat34 LocalPose(NxMat33(_InitialRelativeRotation), _InitialRelativeLocation * P2GScale);
			std::vector<NxShapeDesc*> CachedShapeDescList;
			for(NxU32 ShapeIndex = 0; _ComponentData->_PhysModelDesc != NULL && ShapeIndex < _ComponentData->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
			{
				ShapeDesc* CustomShapeDesc = _ComponentData->_PhysModelDesc->_ShapeDescList[ShapeIndex];

				// ShapeLocalPose记录的是Shape相对Socket局部位置信息
				NxMat34 ShapeLocalPose = CustomShapeDesc->_LocalPose;

				// create NxShapeDesc data
				CustomShapeDesc->_LocalPose = LocalPose * ShapeLocalPose;
				NxShapeDesc* XShapeDesc = CreateComponentShape(CustomShapeDesc);
				CustomShapeDesc->_LocalPose = ShapeLocalPose;	// urly!!!!!
				if( XShapeDesc != NULL )
				{
					XShapeDesc->group = GPL_SHAPE_GROUP_VEHICLE_SHIP;
					NxShape* NewShape = Owner->GetPhysActor()->createShape(*XShapeDesc);
					if( NewShape != NULL )
					{
						NewShape->userData = this;		// 绑定NxShape与ShipComponent
						_Shapes.push_back(NewShape);
					}
					CachedShapeDescList.push_back(XShapeDesc);					
				}
				else
				{
					gplDebugf(TEXT("ERROR: Faied to CreateComponentShape CompId[%d] PhysId[%s] ShapeIndex[%d]"), _ComponentData->_CompDesc->_CompId, _ComponentData->_PhysModelDesc->_PhysId.c_str(), ShapeIndex);
				}

				// 保存Shape初始LocalPose
				ShapeLocalPose.t *= G2PScale;
				_InitShapeLocalPose.push_back(ShapeLocalPose);
			}
			ReleaseShapeDescList(CachedShapeDescList);
		}

		void ShipComponent::Term()
		{
			_Owner = NULL;
			_AttachedId = 0;
		}
		
		void ShipComponent::Tick(float DeltaTime)
		{
			if (!_Activated)
				return;

			UpdatePose(DeltaTime);
		}

		void ShipComponent::UpdatePose(float DeltaTime)
		{
		}

		int ShipComponent::GetAttachedID()
		{
			return _AttachedId;
		}

		NxMat34 ShipComponent::GetGlobalPose()
		{
			NxMat34 LM = _Owner->GetGlobalPos();
			NxMat34 RM(NxMat33(_RelativeRotation), _RelativeLocation);
			return LM * RM;
		}

		NxMat34 ShipComponent::GetLocalPose()
		{
			return NxMat34(NxMat33(_RelativeRotation), _RelativeLocation);
		}

		GPL::EGameObjectType GameObject::GetObjectType()
		{
			if( _ComponentData != NULL && _ComponentData->_CompDesc != NULL )
			{
				return _ComponentData->_CompDesc->_ObjectType;
			}
			return (EGameObjectType)-1;
		}

}