//------------------------------------------------------------------------
//	@brief	有向圆
//	@author	chenpu
//	@date	2013-4-11
//------------------------------------------------------------------------
#include "PhysCommonDefine.h"

#pragma once

namespace GPL {
	
	class DirectionalCircle
	{
	public:
		enum EDirection
		{
			ECW = 0,	//	顺时针方向
			ECCW		//	逆时针方向
		};

		enum EPointType
		{
			EOnCircle = 0,
			EInCircle,
			EOutOfCircle,
		};

		enum ECircleRelation
		{
			EHomocentric = 0,	//	同心圆
			EIntersect,			//	相交
			EInscribed,			//	内切
			EExcircle,			//	外切
			EInside,			//	内部
			EOutside,			//	外部
		};

		DirectionalCircle()	{}
		DirectionalCircle(const NxVec3& center, float radius, EDirection wise);
		DirectionalCircle(const DirectionalCircle& ref);
		virtual ~DirectionalCircle()	{}

		DirectionalCircle& operator = (const DirectionalCircle& ref);

		//	@brief	通过圆弧参数拟合有向圆
		//	@param	[beginPos]	圆弧起点
		//	@param	[beginDir]	起点朝向
		//	@param	[endPos]	圆弧终点
		//	@param	[endDir]	终点朝向
		//	@param	[circle]	out	圆弧所在的有向圆
		//	@return	是否能成功构造
		static bool Construct(
			const NxVec3& beginPos, 
			const NxVec3& beginDir, 
			const NxVec3& endPos, 
			const NxVec3& endDir,
			DirectionalCircle& circle);

		//	@brief	通过矢量和方向拟合有向圆
		//	@param	[beginPos]	圆弧起点
		//	@param	[beginDir]	起点朝向
		//	@param	[wise]		圆周方向
		//	@return	构造的有向圆
		static DirectionalCircle Construct(
			const NxVec3& beginPos, 
			const NxVec3& beginDir, 
			float radius,
			EDirection wise
			);

		//	@brief	计算点到圆的切线
		//	@brief	[point]		目标点
		//	@brief	[stepOutPt]	圆弧切出点
		//	@return	点和圆的位置关系
		EPointType			CalcTangent(const NxVec3& point, NxVec3& stepOutPt);

		//	@brief	计算圆跟圆的切线
		//	@param	[des]		目标有向圆
		//	@param	[stepOutPt]	当前圆切出点
		//	@param	[stepInPt]	目标圆切入点
		void				CalcTangent(const DirectionalCircle& des, NxVec3& stepOutPt, NxVec3& stepInPt) const;

		//	@brief	计算弧
		//	@param	[from]	起始位置
		//	@param	[to]	目标位置
		//	@param	[angle]	out	经过角度
		//	@return	弧长
		float				CalcArc(const NxVec3& from, const NxVec3& to, float& angle) const;

		//	@brief	与另一圆的空间关系
		//	@param	[other]	另一有向圆
		//	@return	空间关系
		ECircleRelation		RelationWith(const DirectionalCircle& other) const;

		//	@brief	获取圆心
		inline NxVec3		Center		()	const				{ return _Center; }
		inline void			Move		(const NxVec3& center)	{ _Center = center; }

		//	@brief	获取半径
		inline float		Radius		()	const				{ return _Radius; }

		//	@brief	获取方向
		inline EDirection	Direction	()	const				{ return _Wise; }

	protected:
		//	@brief	圆心
		NxVec3		_Center;

		//	@brief	半径
		float		_Radius;

		//	@brief	方向
		EDirection	_Wise;
	};
}