//------------------------------------------------------------------------
//	@brief	有向圆
//	@author	chenpu
//	@date	2013-4-11
//------------------------------------------------------------------------
#include "PhysXSupport.h"
#include "DirectionalCircle.h"

namespace GPL{

	DirectionalCircle::DirectionalCircle(const NxVec3& center, float radius, EDirection wise)
		: _Center(center)
		, _Radius(radius)
		, _Wise(wise)
	{
	}

	DirectionalCircle::DirectionalCircle(const DirectionalCircle& ref)
		: _Center(ref.Center())
		, _Radius(ref.Radius())
		, _Wise(ref.Direction())
	{
	}

	DirectionalCircle& DirectionalCircle::operator=(const DirectionalCircle& ref)
	{
		this->_Center = ref.Center();
		this->_Radius = ref.Radius();
		this->_Wise = ref.Direction();

		return *this;
	}

	bool DirectionalCircle::Construct(
		const NxVec3& beginPos, 
		const NxVec3& beginDir, 
		const NxVec3& endPos, 
		const NxVec3& endDir, 
		DirectionalCircle& circle)
	{
		//	To Implement
		return false;
	}

	DirectionalCircle DirectionalCircle::Construct(
		const NxVec3& beginPos, 
		const NxVec3& beginDir,
		float radius, 
		EDirection wise)
	{
		if ( ECW == wise )
		{
			NxVec3 vDir = beginDir.cross(-AXIS_VEC_Z);
			vDir.normalize();

			return DirectionalCircle(beginPos + vDir * radius, radius, ECW);
		}
		else /*if ( ECCW == wise )*/
		{
			NxVec3 vDir = beginDir.cross(AXIS_VEC_Z);
			vDir.normalize();

			return DirectionalCircle(beginPos + vDir * radius, radius, ECCW);
		}
	}

	DirectionalCircle::EPointType DirectionalCircle::CalcTangent(const NxVec3& point, NxVec3& stepOutPt)
	{
		NxVec3 vDir = point - _Center;
		float fDistance = vDir.magnitude();

		//	点在圆内
		if ( fDistance < _Radius )
			return EInCircle;

		//	点在圆上
		if ( NxMath::equals(fDistance, _Radius, 0.01f) )
		{
			stepOutPt = point;
			return EOnCircle;
		}

		//	angle about center-target vector and center-tangent
		//	without wise
		float angle = NxMath::acos(_Radius / fDistance);
		float degree = NxMath::radToDeg(angle);

		//	逆旋转，切点在目标之前
		NxQuat Rot;
		if ( ECW == _Wise )
			Rot.fromAngleAxis(-degree, AXIS_VEC_Z);
		else if ( ECCW == _Wise )
			Rot.fromAngleAxis(degree, AXIS_VEC_Z);

		vDir.normalize();
		NxVec3 vTangent = Rot.rot(vDir);
		stepOutPt = _Center + vTangent * _Radius;

		//	点在圆外
		return EOutOfCircle;
	}

	void DirectionalCircle::CalcTangent(const DirectionalCircle& des, NxVec3& stepOutPt, NxVec3& stepInPt) const
	{
		//	暂时限制不同半径的圆比较
		if ( des.Radius() != _Radius )
			return;

		switch ( _Wise )
		{
		case ECW:
			{
				if ( ECW == des.Direction() )
				{
					NxVec3 vec = des.Center() - _Center;
					vec.normalize();
					NxVec3 t = vec.cross(AXIS_VEC_Z);
					stepOutPt = _Center + t * _Radius;
					stepInPt = des.Center() + t * _Radius;
				}
				else if ( ECCW == des.Direction() )
				{
					NxVec3 vec = des.Center() - _Center;
					float radian = NxMath::acos(2 * _Radius / vec.magnitude());
					float angle = NxMath::radToDeg(radian);
					vec.normalize();

					//	此时只确定切点位置，无需考虑绕行方向
					NxQuat Rot;
					Rot.fromAngleAxis(360 - angle, AXIS_VEC_Z);
					stepOutPt = _Center + Rot.rot(vec) * _Radius;

					Rot.fromAngleAxis(180 - angle, AXIS_VEC_Z);
					stepInPt = des.Center() + Rot.rot(vec) * _Radius;
				}
			}
			break;

		case ECCW:
			{
				if ( ECW == des.Direction() )
				{
					NxVec3 vec = des.Center() - _Center;
					float radian = NxMath::acos(2 * _Radius / vec.magnitude());
					float angle = NxMath::radToDeg(radian);
					vec.normalize();

					NxQuat Rot;
					Rot.fromAngleAxis(angle, AXIS_VEC_Z);
					stepOutPt = _Center + Rot.rot(vec) * _Radius;

					Rot.fromAngleAxis(180 + angle, AXIS_VEC_Z);
					stepInPt = des.Center() + Rot.rot(vec) * _Radius;
				}
				else if ( ECCW == des.Direction() )
				{
					NxVec3 vec = des.Center() - _Center;
					vec.normalize();
					NxVec3 t = vec.cross(-AXIS_VEC_Z);
					stepOutPt = _Center + t * _Radius;
					stepInPt = des.Center() + t * _Radius;
				}
			}
			break;
		}
	}

	float DirectionalCircle::CalcArc(const NxVec3& from, const NxVec3& to, float& angle) const
	{
		NxVec3 src = from - _Center;
		NxVec3 des = to - _Center;
		src.normalize();
		des.normalize();

		NxVec3 normal = src.cross(des);

		float radian = asin(normal.magnitude());

		if ( ECW == _Wise )
		{
			if ( src.dot(des) > 0 )
			{
				if ( normal.sameDirection(AXIS_VEC_Z) )
					radian = radian;
				else
					radian = NxPi * 2 - radian;
			}
			else
			{
				if ( normal.sameDirection(AXIS_VEC_Z) )
					radian = NxPi - radian;
				else
					radian = NxPi + radian;
			}
		} 
		else if ( ECCW == _Wise )
		{
			if ( src.dot(des) > 0 )
			{
				if ( normal.sameDirection(AXIS_VEC_Z) )
					radian = NxPi * 2 - radian;
				else
					radian = radian;
			}
			else
			{
				if ( normal.sameDirection(AXIS_VEC_Z) )
					radian = NxPi + radian;
				else
					radian = NxPi - radian;
			}
		}

		angle = NxMath::radToDeg(radian);
		return radian * _Radius;
	}

	DirectionalCircle::ECircleRelation DirectionalCircle::RelationWith(const DirectionalCircle& other) const
	{
		//	同心圆
		if ( _Center.equals(other.Center(), 0.01f) )
			return EHomocentric;

		float SquaredDistance = _Center.distanceSquared(other.Center());
		float SumRadius = _Radius + other.Radius();
		float SumEval = SumRadius * SumRadius;

		if ( NxMath::equals(SumEval, SquaredDistance, 0.01f) )
			return EExcircle;	//	外切圆
		else if ( SumEval < SquaredDistance )
			return EOutside;	//	外部
		else 
		{
			float DiffRadius = _Radius - other.Radius();
			float DiffEval = DiffRadius * DiffRadius;

			if ( NxMath::equals(DiffEval, SquaredDistance, 0.01f) )
				return EInscribed;	//	内切
			else if ( DiffEval < SquaredDistance )
				return EIntersect;	//	相交
			else 
				return EInside;		//	内部
		}
	}
}