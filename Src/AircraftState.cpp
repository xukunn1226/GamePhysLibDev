//------------------------------------------------------------------------
//	@brief	舰载机状态
//	@note	将轨迹控制单独抽象出来所构成的基类
//	@author	chenpu
//	@date	2013-4-24
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "AircraftFSM.h"
#include "AircraftState.h"
#include "DirectionalCircle.h"

namespace GPL {

	void AircraftState::OnEnter(GameObjectEvent& args)
	{
		__super::OnEnter(args);

		_Step.clear();
	}

	void AircraftState::OnLeave(GameObjectEvent& args)
	{
		__super::OnLeave(args);
	}

	void AircraftState::Update(float DeltaTime)
	{
		if (!_Host)
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (!pAircraft)
			return;

		__super::Update(DeltaTime);

		if (NULL != pAircraft->GetHeader() && pAircraft->CheckFlag(LockHeader))
			return;

		if (!_Step.empty())
		{
			MotionList::iterator iter = _Step.begin();

			//	更新参数
			RefreshParam(*iter, pAircraft);

			if (StepMovement(*iter, DeltaTime))
			{
				_Step.erase(iter);

				//	动作完成通知
				PostMotion(pAircraft);
			}
		}
	}

	float AircraftState::GetRemainTime()
	{
		float fRemain = 0.f;

		if ( !_Host )
			return fRemain;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return fRemain;

		if ( _Step.empty() )
			return fRemain;

		const NxVec3 vPos = pAircraft->GetGlobalPosition();

		//	executing phase
		OrbitParams& phase = _Step.front();
		if ( phase._Wise == None )
		{
			fRemain = phase._Target.distance(vPos) / pAircraft->GetSpeed();
		}
		else
		{
			DirectionalCircle::EDirection dir = (phase._Wise != CCW) ? DirectionalCircle::ECW : DirectionalCircle::ECCW;
			DirectionalCircle circle(phase._Center, phase._Radius, dir);
			float fAngle;
			float fArc = circle.CalcArc(vPos, phase._Target, fAngle);
			fRemain = fArc / pAircraft->GetSpeed();
		}

		//	skip the first
		MotionList::iterator iter = _Step.begin();
		for ( iter++; iter != _Step.end(); iter++ )
		{
			if ( iter->_Wise == None )
			{
				fRemain += phase._Target.distance(phase._Positon) / pAircraft->GetSpeed();
			}
			else
			{
				DirectionalCircle::EDirection dir = (phase._Wise != CCW) ? DirectionalCircle::ECW : DirectionalCircle::ECCW;
				DirectionalCircle circle(phase._Center, phase._Radius, dir);
				float fAngle;
				float fArc = circle.CalcArc(phase._Positon, phase._Target, fAngle);
				fRemain += fArc / pAircraft->GetSpeed();
			}
		}

		return fRemain;
	}

	bool AircraftState::GetMotionParam(UINT step, OrbitParams& params)
	{
		if ( step < _Step.size() )
		{
			params = _Step[step];
			return true;
		}

		return false;
	}

	void AircraftState::ModifyMotionParam(UINT step, const OrbitParams& params)
	{
		if ( step < _Step.size() )
		{
			_Step[step] = params;
		}
	}

	void AircraftState::ForceStep()
	{
		if ( _Step.empty() )
			return;

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		_Step.erase(_Step.begin());

		PostMotion(pAircraft);
	}

	void AircraftState::Coordinate(const NxVec3& Pos, const NxVec3& Dir)
	{
		if ( _Step.empty() )
			return;

		if ( !_Host )
			return;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		OrbitParams& param = _Step.front();
		if ( param._Wise == None )
		{
			param._Positon = Pos;
			param._PassedTime = 0;
		} 
		else
		{
			DirectionalCircle::EDirection Wise = param._Wise == CW ? DirectionalCircle::ECW : DirectionalCircle::ECCW;
			DirectionalCircle circle(param._Center, param._Radius, Wise);
			float fAngle;
			float fArc = circle.CalcArc(param._Positon, Pos, fAngle);
			param._PassedTime = fArc / pAircraft->GetSpeed();
			param._Positon.z = Pos.z;
			param._Center.z = Pos.z;
		}

		//gplDebugf(TEXT("AircraftState::Coordinate State[%d] Pos[%f, %f, %f] Dir[%f, %f, %f]"),
		//	_Value, Pos.x, Pos.y, Pos.z, Dir.x, Dir.y, Dir.z);
		pAircraft->SetGlobalPosition(Pos);
		pAircraft->SetForward(Dir);
	}

	void AircraftState::MovementForecast(float Time, NxVec3& Pos, NxVec3& Dir)
	{
		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if ( !pAircraft )
			return;

		//	预订轨迹已全部完成
		if ( _Step.empty() )
		{
			Pos = pAircraft->GetGlobalPosition();
			Dir = pAircraft->GetForward();
			return;
		}

		OrbitParams& param = _Step.front();
		float accumTime = param._PassedTime + Time;
		switch ( param._Wise )
		{
		case None:
			{
				//	直线运动
				NxVec3 vDir = param._Target - param._Positon;
				vDir.normalize();

				Dir = vDir;
				Dir.z = 0;
				Dir.normalize();

				float fMovement = accumTime * pAircraft->GetSpeed();
				Pos = param._Positon + vDir * fMovement;
			}
			break;

		default:
			{
				//	圆周运动
				float fMovement = accumTime * pAircraft->GetSpeed();
				float fRad = fMovement / param._Radius;
				float degree = NxMath::radToDeg(fRad);

				NxQuat Rot;
				if ( CW == param._Wise )
					Rot.fromAngleAxis(degree, AXIS_VEC_Z);
				else if ( CCW == param._Wise )
					Rot.fromAngleAxis(-degree, AXIS_VEC_Z);

				NxVec3 vFrom = param._Positon - param._Center;
				vFrom.z = 0;
				vFrom.normalize();
				NxVec3 vTo = Rot.rot(vFrom);

				Pos = param._Center + vTo * param._Radius;

				if ( CW == param._Wise )
					Dir = vTo.cross(-AXIS_VEC_Z);
				else if ( CCW == param._Wise )
					Dir = vTo.cross(AXIS_VEC_Z);

				Dir.z = 0;
				Dir.normalize();
			}
			break;
		}
	}

	bool AircraftState::StepMovement(OrbitParams& args, float DeltaTime)
	{
		if (!_Host)
			return false;

		PhysAircraft* pAircraft = DynamicCast(PhysAircraft, _Host->GetActor());
		if (!pAircraft)
			return false;

		AircraftData* pData = DynamicCast(AircraftData, pAircraft->_ComponentData);
		if (!pData)
			return false;

		bool MotionFinished = false;
		const NxVec3 PREV_LOCATION = pAircraft->GetGlobalPosition();

		args._PassedTime += DeltaTime;

		//	位置变更 & Yaw
		switch (args._Wise)
		{
		case None:
			{
				//	直线运动
				NxVec3 vDir = args._Target - args._Positon;
				float fDistance = vDir.normalize();	
				
				NxVec3 vPose = vDir;
				vPose.z = 0;
				vPose.normalize();
				pAircraft->SetForward(vPose);

				float fMovement = args._PassedTime * pAircraft->GetSpeed();
				if (fDistance > fMovement)
				{
					NxVec3 Pos = args._Positon + vDir * fMovement;

					pAircraft->SetGlobalPosition(Pos);
					MotionFinished = false;
				}
				else
				{
					pAircraft->SetGlobalPosition(args._Target);
					MotionFinished = true;
				}
			}
			break;

		default:
			{
				//	圆周运动
				//	note : In PhysX, when rotation follows CW, the angle takes positive value, otherwise, negative
				//	note : when use vector's cross, take left-hand !!!

				bool bBreak = false;

				//	将位移近似看作弧长,根据盘旋半径计算运行弧度
				float fMovement = args._PassedTime * pAircraft->GetSpeed();
				float fRad = fMovement / args._Radius;
				float degree = NxMath::radToDeg(fRad);

				if ( args._Critical > 0 )
				{
					if ( degree >= args._Critical )
					{
						//	TODO : 将超过的角度折算成剩余时间
						degree = args._Critical;
						bBreak = true;
					}
				}

				NxQuat Rot;
				if ( CW == args._Wise )
					Rot.fromAngleAxis(degree, AXIS_VEC_Z);
				else if ( CCW == args._Wise )
					Rot.fromAngleAxis(-degree, AXIS_VEC_Z);

				NxVec3 vFrom = args._Positon - args._Center;
				vFrom.z = 0;
				vFrom.normalize();
				NxVec3 vTo = Rot.rot(vFrom);

				NxVec3 Src = pAircraft->GetGlobalPosition();
				NxVec3 Des = args._Center + vTo * args._Radius;
				//gplDebugf(TEXT("AircraftState::StepMovement<Arc> From[%f, %f, %f]To[%f, %f, %f]"),
				//	Src.x, Src.y, Src.z, Des.x, Des.y, Des.z);
				pAircraft->SetGlobalPosition(Des);

				NxVec3 vPose;
				if ( CW == args._Wise )
					vPose = vTo.cross(-AXIS_VEC_Z);
				else if ( CCW == args._Wise )
					vPose = vTo.cross(AXIS_VEC_Z);

				vPose.z = 0;
				vPose.normalize();
				pAircraft->SetForward(vPose);

				MotionFinished = bBreak;
			}
			break;
		}

		//	高度变更 & Pitch
		if (pAircraft->CheckFlag(VerticalMove))
		{
			float DeltaHeight;

			if (pAircraft->CheckFlag(ExigencePull))
				DeltaHeight = pAircraft->Pull(DeltaTime);
			else if (pAircraft->CheckFlag(DiveAttack))
				DeltaHeight = pAircraft->Dive(DeltaTime);
			else if (pAircraft->CheckFlag(Falling))
				DeltaHeight = pAircraft->FallingDown(DeltaTime);
			else if (pAircraft->CheckFlag(Ascend))
				DeltaHeight = pAircraft->Incline(DeltaTime);
			else if (pAircraft->CheckFlag(Descend))
				DeltaHeight = pAircraft->Decline(DeltaTime);
			else
				DeltaHeight = 0.f;

			//	俯仰姿势调整
			NxVec3 Offset = pAircraft->GetGlobalPosition() - PREV_LOCATION;
			if (!Offset.isZero())
			{
				float fPitchRad = NxMath::asin(DeltaHeight / Offset.magnitude());
				NxQuat Pitch(NxMath::radToDeg(fPitchRad), -AXIS_VEC_Y);
				NxQuat Pose = pAircraft->GetGlobalOritationQuat() * Pitch;
				pAircraft->SetGlobalOritationQuat(Pose);
			}
			else
			{
				gplDebugf(TEXT("Warnning!! Zero Horizontal Movement in Pose Calc"));
			}
		}

#ifdef _CLIENT_RUNTIME
		//	Roll
		if (args._Wise != None)
		{
			NxQuat Roll(CW == args._Wise ? -30.f : 30.f, AXIS_VEC_X);			
			NxQuat Pose = pAircraft->GetGlobalOritationQuat() * Roll;
			pAircraft->SetGlobalOritationQuat(Pose);
		}
#endif // _CLIENT_RUNTIME

		return MotionFinished;
	}

}