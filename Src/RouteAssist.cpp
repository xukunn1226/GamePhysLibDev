#include "PhysXSupport.h"
#include "PhysCommonDefine.h"
#include "PhysGameObject.h"
#include "PhysGameObjectAircraft.h"
#include "DirectionalCircle.h"

namespace GPL {

	RouteAssist::RouteAssist()
	{
		_LayerHeight[ExtremeLow] = GPhysGameSetting._GPL_FlightLayer_TorpedoRelease * G2PScale;
		_LayerHeight[Lower] = GPhysGameSetting._GPL_FlightLayer_DiveLimit * G2PScale;
		_LayerHeight[Higher] = GPhysGameSetting._GPL_FlightLayer_CruiseHeight * G2PScale;
	}

	void RouteAssist::RouteWithoutDirection(PhysAircraft* Actor, const NxVec3& DesPos, MotionList& Path)
	{
		//	Point-target without Direction
		if ( !Actor )
			return;

		Path.clear();

		const float fRadius = Actor->GetMinRadius();
		const NxVec3 OriginalPos = Actor->GetGlobalPosition();
		const NxVec3 OriginalDir = Actor->GetForward();
		const NxVec3 Target(DesPos.x, DesPos.y, OriginalPos.z);

		Actor->RestoreTargetPos(Target);
		Actor->RestoreTargetDir(ZERO_VEC);

		NxVec3 vDisplacement = Target - OriginalPos;
		vDisplacement.normalize();
		if ( OriginalDir.sameDirection(vDisplacement) && OriginalDir.distance(vDisplacement) < 0.01f )
		{
			//	1.λ�Ʒ������ʼ�ٶȷ������һ�£�ֱ������ֱ���˶�
			OrbitParams phase(OriginalPos, Target);
			Path.push_back(phase);
		}
		else if ( OriginalDir.equals(-vDisplacement, 0.1f) )
		{
			//	1.1Ŀ�꼸�������󷽣�ͳһ��CW
			NxVec3 vRight = OriginalDir.cross(-AXIS_VEC_Z);
			vRight.normalize();

			NxVec3 vTangentCW;
			float fAngleCW;
			NxVec3 vCenterCW = OriginalPos + vRight * fRadius;
			DirectionalCircle circleCW(vCenterCW, fRadius, DirectionalCircle::ECW);
			circleCW.CalcTangent(Target, vTangentCW);
			circleCW.CalcArc(OriginalPos, vTangentCW, fAngleCW);

			OrbitParams arcPhase(CW, OriginalPos, vCenterCW, fRadius);
			arcPhase._Critical = fAngleCW;
			Path.push_back(arcPhase);

			OrbitParams linePhase(vTangentCW, Target);
			Path.push_back(linePhase);
		}
		else
		{
			NxVec3 vDir = OriginalDir.cross(-AXIS_VEC_Z);
			vDir.normalize();

			NxVec3 vTangentCW;
			float fAngleCW;
			NxVec3 vCenterCW = OriginalPos + vDir * fRadius;
			DirectionalCircle circleCW(vCenterCW, fRadius, DirectionalCircle::ECW);
			DirectionalCircle::EPointType resultCW = circleCW.CalcTangent(Target, vTangentCW);
			if ( DirectionalCircle::EOnCircle == resultCW )
			{
				//	2.����CWԲ�ϣ���CWԲ
				circleCW.CalcArc(OriginalPos, vTangentCW, fAngleCW);

				OrbitParams arcPhase(CW, OriginalPos, vCenterCW, fRadius);
				arcPhase._Critical = fAngleCW;
				Path.push_back(arcPhase);

				return;
			}

			NxVec3 vTangentCCW;
			float fAngleCCW;
			NxVec3 vCenterCCW = OriginalPos - vDir * fRadius;
			DirectionalCircle circleCCW(vCenterCCW, fRadius, DirectionalCircle::ECCW);
			DirectionalCircle::EPointType resultCCW = circleCCW.CalcTangent(Target, vTangentCCW);
			if ( DirectionalCircle::EOnCircle == resultCCW )
			{
				//	3.����CCWԲ�ϣ���CCWԲ
				circleCCW.CalcArc(OriginalPos, vTangentCCW, fAngleCCW);

				OrbitParams arcPhase(CCW, OriginalPos, vCenterCCW, fRadius);
				arcPhase._Critical = fAngleCCW;
				Path.push_back(arcPhase);

				return;
			}

			if ( DirectionalCircle::EInCircle == resultCCW )
			{
				//	4.����CCWԲ�ڣ���CWԲ
				circleCW.CalcArc(OriginalPos, vTangentCW, fAngleCW);

				OrbitParams arcPhase(CW, OriginalPos, vCenterCW, fRadius);
				arcPhase._Critical = fAngleCW;
				Path.push_back(arcPhase);

				OrbitParams linePhase(vTangentCW, Target);
				Path.push_back(linePhase);
			}
			else if ( DirectionalCircle::EInCircle == resultCW )
			{
				//	5.����CWԲ�ڣ���CCWԲ
				circleCCW.CalcArc(OriginalPos, vTangentCCW, fAngleCCW);

				OrbitParams arcPhase(CCW, OriginalPos, vCenterCCW, fRadius);
				arcPhase._Critical = fAngleCCW;
				Path.push_back(arcPhase);

				OrbitParams linePhase(vTangentCCW, Target);
				Path.push_back(linePhase);
			}
			else 
			{
				//	6.ͬ��Բ����ֱ�����Ƚ�
				float fArcCW = circleCW.CalcArc(OriginalPos, vTangentCW, fAngleCW);
				float fMovementCW = fArcCW + vTangentCW.distance(Target);

				float fArcCCW = circleCCW.CalcArc(OriginalPos, vTangentCCW, fAngleCCW);
				float fMovementCCW = fArcCCW + vTangentCCW.distance(Target);

				if ( fMovementCW < fMovementCCW )
				{
					OrbitParams arcPhase(CW, OriginalPos, vCenterCW, fRadius);
					arcPhase._Critical = fAngleCW;
					Path.push_back(arcPhase);

					OrbitParams linePhase(vTangentCW, Target);
					Path.push_back(linePhase);
				} 
				else
				{
					OrbitParams arcPhase(CCW, OriginalPos, vCenterCCW, fRadius);
					arcPhase._Critical = fAngleCCW;
					Path.push_back(arcPhase);

					OrbitParams phase(vTangentCCW, Target);
					Path.push_back(phase);
				}
			}
		}
	}

#if 0
	MotionList RouteAssist::Route(const NxVec3& SrcPos, const NxVec3& SrcDir, const NxVec3& DesPos, const NxVec3& DesDir, float Radius)
	{
		//	Point-target with Direction

#if 0
		//	0.�ٶȷ������һ�£�ֱ������ֱ���˶�
		if ( SrcDir.sameDirection(DesDir) )
		{
			float Radian = NxMath::acos(SrcDir.dot(DesDir));
			float Angle = NxMath::radToDeg(Radian);
			if ( Angle < 10 )
			{
				MotionList Steps;
				OrbitParams phase(SrcPos, DesPos);
				Steps.push_back(phase);
				return Steps;
			}
		}
#endif

		//	1.����4������Բ
		NxVec3 vDir = SrcDir.cross(-AXIS_VEC_Z);
		vDir.normalize();
		NxVec3 vSrcCenterCW = SrcPos + vDir * Radius;
		DirectionalCircle SrcCircleCW(vSrcCenterCW, Radius, DirectionalCircle::ECW);

		NxVec3 vSrcCenterCCW = SrcPos - vDir * Radius;
		DirectionalCircle SrcCircleCCW(vSrcCenterCCW, Radius, DirectionalCircle::ECCW);

		vDir = DesDir.cross(-AXIS_VEC_Z);
		vDir.normalize();
		NxVec3 vDesCenterCW = DesPos + vDir * Radius;
		DirectionalCircle DesCircleCW(vDesCenterCW, Radius, DirectionalCircle::ECW);

		NxVec3 vDesCenterCCW = DesPos - vDir * Radius;
		DirectionalCircle DesCircleCCW(vDesCenterCCW, Radius, DirectionalCircle::ECCW);

		//	2.�ֱ�����Բ֮ǰ��·��
		MotionList CWCW, CWCCW, CCWCW, CCWCCW, Empty;
		float L1 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCW, DesCircleCW, SrcPos, DesPos, CWCW);
		float L2 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCW, DesCircleCCW, SrcPos, DesPos, CWCCW);
		float L3 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCCW, DesCircleCW, SrcPos, DesPos, CCWCW);
		float L4 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCCW, DesCircleCCW, SrcPos, DesPos, CCWCCW);

		//	3.ȡ���·��
		std::vector<float>	Results;
		if ( L1 > 0 )	Results.push_back(L1);
		if ( L2 > 0 )	Results.push_back(L2);
		if ( L3 > 0 )	Results.push_back(L3);
		if ( L4 > 0 )	Results.push_back(L4);

		if ( !Results.empty() )
		{
			std::sort(Results.begin(), Results.end());

			if ( NxMath::equals(Results.front(), L1, 0.01f) )
				return CWCW;
			else if ( NxMath::equals(Results.front(), L2, 0.01f) )
				return CWCCW;
			else if ( NxMath::equals(Results.front(), L3, 0.01f) )
				return CCWCW;
			else if ( NxMath::equals(Results.front(), L4, 0.01f) )
				return CCWCCW;
		}

		return Empty;
	}
#endif

	void RouteAssist::Route(PhysAircraft* Actor, const NxVec3& DesPos, const NxVec3& DesDir, MotionList& Path)
	{
		//	Point-target with Direction
		if ( !Actor )
			return;

		Path.clear();

		const float fActuralRadius = Actor->GetMinRadius();
		const NxVec3 OriginalPos = Actor->GetGlobalPosition();
		const NxVec3 OriginalDir = Actor->GetForward();
		const NxVec3 Target(DesPos.x, DesPos.y, OriginalPos.z);

		Actor->RestoreTargetPos(Target);
		Actor->RestoreTargetDir(DesDir);

		//	1.����4������Բ
		NxVec3 vDir = OriginalDir.cross(-AXIS_VEC_Z);
		vDir.normalize();
		NxVec3 vSrcCenterCW = OriginalPos + vDir * fActuralRadius;
		DirectionalCircle SrcCircleCW(vSrcCenterCW, fActuralRadius, DirectionalCircle::ECW);

		NxVec3 vSrcCenterCCW = OriginalPos - vDir * fActuralRadius;
		DirectionalCircle SrcCircleCCW(vSrcCenterCCW, fActuralRadius, DirectionalCircle::ECCW);

		vDir = DesDir.cross(-AXIS_VEC_Z);
		vDir.normalize();
		NxVec3 vDesCenterCW = Target + vDir * fActuralRadius;
		DirectionalCircle DesCircleCW(vDesCenterCW, fActuralRadius, DirectionalCircle::ECW);

		NxVec3 vDesCenterCCW = Target - vDir * fActuralRadius;
		DirectionalCircle DesCircleCCW(vDesCenterCCW, fActuralRadius, DirectionalCircle::ECCW);

		//	1.1.���Ŀ��λ�ý��ƴ�������ʼԲ�ϵ���������ݲ��ж�
		if (NxMath::equals(Target.distance(vSrcCenterCW), fActuralRadius, GPhysGameSetting._GPL_Aircraft_Routing_Torlerance * G2PScale))
		{
			NxVec3 Normal = Target - vSrcCenterCW;
			Normal.normalize();
			NxReal Rad = NxMath::acos(DesDir.dot(Normal));
			NxReal DeltaAngle = NxMath::abs(NxMath::radToDeg(NxHalfPi - Rad));
			if (DeltaAngle < GPhysGameSetting._GPL_Aircraft_Routing_AngleTorlerance)
			{
				float OutAngle;
				SrcCircleCW.CalcArc(OriginalPos, Target, OutAngle);
				if (OutAngle > 1.f && OutAngle < 359.f)
				{
					OrbitParams P1(CW, OriginalPos, vSrcCenterCW, fActuralRadius);
					P1._Critical = OutAngle;
					Path.push_back(P1);
				}

				OrbitParams P2(Path.empty() ? OriginalPos : Target - DesDir, Target);
				Path.push_back(P2);
				return;
			}
		}
		else if (NxMath::equals(Target.distance(vSrcCenterCCW), fActuralRadius, 0.1f))
		{
			NxVec3 Normal = Target - vSrcCenterCCW;
			Normal.normalize();
			NxReal Rad = NxMath::acos(DesDir.dot(Normal));
			NxReal DeltaAngle = NxMath::abs(NxMath::radToDeg(NxHalfPi - Rad));
			if (DeltaAngle < GPhysGameSetting._GPL_Aircraft_Routing_AngleTorlerance)
			{
				float OutAngle;
				SrcCircleCCW.CalcArc(OriginalPos, Target, OutAngle);
				if (OutAngle > 1.f && OutAngle < 359.f)
				{
					OrbitParams P1(CW, OriginalPos, vSrcCenterCCW, fActuralRadius);
					P1._Critical = OutAngle;
					Path.push_back(P1);
				}

				OrbitParams P2(Path.empty() ? OriginalPos : Target - DesDir, Target);
				Path.push_back(P2);
				return;
			}
		}

		//	2.�ֱ�����Բ֮ǰ��·��
		MotionList CWCW, CWCCW, CCWCW, CCWCCW;
		float L1 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCW, DesCircleCW, OriginalPos, Target, CWCW);
		float L2 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCW, DesCircleCCW, OriginalPos, Target, CWCCW);
		float L3 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCCW, DesCircleCW, OriginalPos, Target, CCWCW);
		float L4 = RouteAssist::GetSingletonRef().CalcRoutingLength(SrcCircleCCW, DesCircleCCW, OriginalPos, Target, CCWCCW);

		//	3.ȡ���·��
		std::vector<float>	Results;
		if ( L1 > 0 )	Results.push_back(L1);
		if ( L2 > 0 )	Results.push_back(L2);
		if ( L3 > 0 )	Results.push_back(L3);
		if ( L4 > 0 )	Results.push_back(L4);

		if ( !Results.empty() )
		{
			std::sort(Results.begin(), Results.end());

			if ( NxMath::equals(Results.front(), L1, 0.01f) )
			{
				Path = CWCW;
				//gplDebugf(TEXT("Route CW-CW MotionNum=%d"), _Step.size());
			}
			else if ( NxMath::equals(Results.front(), L2, 0.01f) )
			{
				Path = CWCCW;
				//gplDebugf(TEXT("Route CW-CCW MotionNum=%d"), _Step.size());
			}
			else if ( NxMath::equals(Results.front(), L3, 0.01f) )
			{
				Path = CCWCW;
				//gplDebugf(TEXT("Route CCW-CW MotionNum=%d"), _Step.size());
			}
			else if ( NxMath::equals(Results.front(), L4, 0.01f) )
			{
				Path = CCWCCW;
				//gplDebugf(TEXT("Route CCW-CCW MotionNum=%d"), _Step.size());
			}
		}

		if ( Actor->GetLockTarget() && Path.back()._Wise != None )
			Path.back()._Critical = 0;
	}

	void RouteAssist::ObjectTrace(PhysAircraft* Actor, PhysGameObject* TargetObj, MotionList& Path, bool ignoreDir /* = true */)
	{
		if ( !Actor || !TargetObj )
			return;

		NxVec3 Target = TargetObj->GetGlobalPosition();
		if ( ignoreDir )
		{
			RouteWithoutDirection(Actor, Target, Path);
		}
		else
		{
			NxVec3 Dir;

			PhysAircraft* pAircraft = DynamicCast(PhysAircraft, TargetObj);
			if ( NULL != pAircraft )
				Dir = pAircraft->GetForward();
			else
			{
				NxMat33 matDir = TargetObj->GetGlobalOritation();
				matDir.multiplyByTranspose(AXIS_VEC_X, Dir);
				Dir.z = 0;
				Dir.normalize();
			}

			Route(Actor, Target, Dir, Path);
		}

		//	����׷�ٵ��������Ҫ��һ��·���滮�����һ�ζ���ʱ�������ӳ�����ֹ����һ��Ѱ·����ǰ�����ս�
		ExtendLastMotion(Actor, Path);
	}

	float RouteAssist::GetLayerHeight(FlightLayer layer)
	{
		if ( layer < LayerNum )
			return _LayerHeight[layer];

		return 0.f;
	}

	NxVec3 RouteAssist::CalcStepIn(PhysAircraft* Aircraft, AircraftAction Action, FlightLayer Layer, const NxVec3& Target, const NxVec3& Direction)
	{
		if ( !Aircraft )
			return ZERO_VEC;

		if ( Attack == Action )
		{
			float Height = GetLayerHeight(Layer);
			float Trans = (Height - Target.z) / Aircraft->GetSwoopSlope();	//	ˮƽλ��
			NxVec3 StepIn = Target - Direction * Trans;
			//if (Layer != ExtremeLow)
			//	StepIn.z = Height;

			return StepIn;
		}
		else if ( Landing == Action )
		{
			float Slope = NxMath::tan(NxMath::degToRad(GPhysGameSetting._GPL_Aircraft_Landing_Angle));
			float Height = GetLayerHeight(Layer);
			float Trans = (Height - Target.z) / Slope;	//	ˮƽλ��
			NxVec3 StepIn = Target - Direction * Trans;
			StepIn.z = Height;

			return StepIn;
		}

		return ZERO_VEC;
	}

	NxVec3 RouteAssist::CalcStepOut(PhysAircraft* Aircraft, AircraftAction Action, const NxVec3& SrcPos, const NxVec3& Direction)
	{
		if ( !Aircraft )
			return ZERO_VEC;

		if ( Attack == Action )
		{
			float Height = GetLayerHeight(Higher);
			float Trans = (Height - SrcPos.z) / Aircraft->GetAscendSlope();	//	ˮƽλ��
			NxVec3 StepOut = SrcPos + Direction * Trans;
			StepOut.z = Height;

			return StepOut;
		}

		return ZERO_VEC;
	}

	float RouteAssist::CalcRoutingLength(const DirectionalCircle& src, const DirectionalCircle& des, const NxVec3& from, const NxVec3& to, MotionList& Motions)
	{
		NxVec3 OutPoint, InPoint;
		float OutAngle, InAngle;

		Motions.clear();

		if ( src.Direction() == des.Direction() )
		{
			//	ͬ��Բ
			if ( src.RelationWith(des) != DirectionalCircle::EHomocentric )
			{
				//	��ͬ��Բ
				src.CalcTangent(des, OutPoint, InPoint);
				float R1 = src.CalcArc(from, OutPoint, OutAngle);
				float R2 = OutPoint.distance(InPoint);
				float R3 = des.CalcArc(InPoint, to, InAngle);
				float Length = 0.f;
				
				OrbitParams P1(((src.Direction() != DirectionalCircle::ECCW) ? CW : CCW), from, src.Center(), src.Radius());
				OrbitParams P2(OutPoint, InPoint);
				OrbitParams P3(((des.Direction() != DirectionalCircle::ECCW) ? CW : CCW), InPoint, des.Center(), des.Radius());
				if (OutAngle > 1.f && OutAngle < 359.f)
				{
					P1._Critical = OutAngle;
					Motions.push_back(P1);
					Length += R1;
				}

				if (R2 > 0.f)
				{
					Motions.push_back(P2);
					Length += R2;
				}

				if (InAngle > 1.f && InAngle < 359.f)
				{
					P3._Critical = InAngle;
					Motions.push_back(P3);
					Length += R3;
				}

				return Length;
			}
			else
			{
				//	ͬ��Բ���뾶��ͬ��
				//NX_ASSERT(NxMath::equals(src.Radius(), des.Radius(), 0.01f));

				float Length = src.CalcArc(from, to, InAngle);
				
				OrbitParams P1(((src.Direction() != DirectionalCircle::ECCW) ? CW : CCW), from, src.Center(), src.Radius());
				if (InAngle > 1.f && InAngle < 359.f)
				{
					P1._Critical = InAngle;
					Motions.push_back(P1);
					return Length;
				}
				
				return 0.f;
			}
		}
		else
		{
			//	����Բ
			switch ( src.RelationWith(des) )
			{
			case DirectionalCircle::EOutside:
				{
					//	�ⲿԲ
					src.CalcTangent(des, OutPoint, InPoint);
					float R1 = src.CalcArc(from, OutPoint, OutAngle);
					float R2 = OutPoint.distance(InPoint);
					float R3 = des.CalcArc(InPoint, to, InAngle);
					float Length = 0.f;

					OrbitParams P1(((src.Direction() != DirectionalCircle::ECCW) ? CW : CCW), from, src.Center(), src.Radius());
					OrbitParams P2(OutPoint, InPoint);
					OrbitParams P3(((des.Direction() != DirectionalCircle::ECCW) ? CW : CCW), InPoint, des.Center(), des.Radius());
					if (OutAngle > 1.f && OutAngle < 359.f)
					{
						P1._Critical = OutAngle;
						Motions.push_back(P1);
						Length += R1;
					}

					if (R2 > 0.f)
					{
						Motions.push_back(P2);
						Length += R2;
					}

					if (InAngle > 1.f && InAngle < 359.f)
					{
						P3._Critical = InAngle;
						Motions.push_back(P3);
						Length += R3;
					}

					return Length;
				}
				break;

			case DirectionalCircle::EExcircle:
				{
					//	����Բ
					NxVec3 CutPoint = src.Center() + (des.Center() - src.Center()) * 0.5f;	//	�е�
					float R1 = src.CalcArc(from, CutPoint, OutAngle);
					float R2 = des.CalcArc(CutPoint, to, InAngle);
					float Length = 0.f;

					OrbitParams P1(((src.Direction() != DirectionalCircle::ECCW) ? CW : CCW), from, src.Center(), src.Radius());
					OrbitParams P2(((des.Direction() != DirectionalCircle::ECCW) ? CW : CCW), CutPoint, des.Center(), des.Radius());
					if (OutAngle > 1.f && OutAngle < 359.f)
					{
						P1._Critical = OutAngle;
						Motions.push_back(P1);
						Length += R1;
					}

					if (InAngle > 1.f && InAngle < 359.f)
					{
						P2._Critical = InAngle;
						Motions.push_back(P2);
						Length += R2;
					}

					return Length;
				}
				break;
			}
		}

		return -1.f;
	}

	float RouteAssist::CalcDiveDuration(float DiveHeight, float DiveAcceleration, float SpeedLimit)
	{
		const NxReal Movement = NxMath::abs(DiveHeight);
		const NxReal Acceleration = NxMath::abs(DiveAcceleration);
		const NxReal FinalSpeed = NxMath::abs(SpeedLimit);

		if (NxMath::equals(Movement, 0.f, 0.001f))
			return 0.f;

		float AccelerateTime = FinalSpeed / Acceleration;								//	����ʱ��
		float AccelerateDist = 0.5f * Acceleration * AccelerateTime * AccelerateTime;	//	���پ���
		if (AccelerateDist > Movement)
			return NxMath::sqrt(2 * Movement / Acceleration);
		else
			return AccelerateTime + (Movement - AccelerateDist) / FinalSpeed;
	}

	void RouteAssist::ExtendLastMotion(PhysAircraft* Actor, MotionList& Path)
	{
		if ( Path.empty() )
		{
			NxVec3 Pos = Actor->GetGlobalPosition();
			NxVec3 Dir = Actor->GetForward();
			float Radius = Actor->GetMinRadius();

			NxVec3 Right = Dir.cross(-AXIS_VEC_Z);
			Right.normalize();

			NxVec3 Center = Pos + Right * Radius;
			OrbitParams PlusMotion(CW, Pos, Center, Radius);

			Path.push_back(PlusMotion);
		}
		else
		{
			OrbitParams& LastMotion = Path.back();
			if ( LastMotion._Wise != None )
			{
				LastMotion._Critical = 0;
			}
			else
			{
				NxVec3 Dir = LastMotion._Target - LastMotion._Positon;
				Dir.normalize();
				LastMotion._Target = LastMotion._Positon + Dir * GPhysGameSetting._GPL_MAX_HALF_WORLD;
			}
		}
	}
}