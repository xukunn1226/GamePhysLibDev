//------------------------------------------------------------------------
//	@brief	����Բ
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
			ECW = 0,	//	˳ʱ�뷽��
			ECCW		//	��ʱ�뷽��
		};

		enum EPointType
		{
			EOnCircle = 0,
			EInCircle,
			EOutOfCircle,
		};

		enum ECircleRelation
		{
			EHomocentric = 0,	//	ͬ��Բ
			EIntersect,			//	�ཻ
			EInscribed,			//	����
			EExcircle,			//	����
			EInside,			//	�ڲ�
			EOutside,			//	�ⲿ
		};

		DirectionalCircle()	{}
		DirectionalCircle(const NxVec3& center, float radius, EDirection wise);
		DirectionalCircle(const DirectionalCircle& ref);
		virtual ~DirectionalCircle()	{}

		DirectionalCircle& operator = (const DirectionalCircle& ref);

		//	@brief	ͨ��Բ�������������Բ
		//	@param	[beginPos]	Բ�����
		//	@param	[beginDir]	��㳯��
		//	@param	[endPos]	Բ���յ�
		//	@param	[endDir]	�յ㳯��
		//	@param	[circle]	out	Բ�����ڵ�����Բ
		//	@return	�Ƿ��ܳɹ�����
		static bool Construct(
			const NxVec3& beginPos, 
			const NxVec3& beginDir, 
			const NxVec3& endPos, 
			const NxVec3& endDir,
			DirectionalCircle& circle);

		//	@brief	ͨ��ʸ���ͷ����������Բ
		//	@param	[beginPos]	Բ�����
		//	@param	[beginDir]	��㳯��
		//	@param	[wise]		Բ�ܷ���
		//	@return	���������Բ
		static DirectionalCircle Construct(
			const NxVec3& beginPos, 
			const NxVec3& beginDir, 
			float radius,
			EDirection wise
			);

		//	@brief	����㵽Բ������
		//	@brief	[point]		Ŀ���
		//	@brief	[stepOutPt]	Բ���г���
		//	@return	���Բ��λ�ù�ϵ
		EPointType			CalcTangent(const NxVec3& point, NxVec3& stepOutPt);

		//	@brief	����Բ��Բ������
		//	@param	[des]		Ŀ������Բ
		//	@param	[stepOutPt]	��ǰԲ�г���
		//	@param	[stepInPt]	Ŀ��Բ�����
		void				CalcTangent(const DirectionalCircle& des, NxVec3& stepOutPt, NxVec3& stepInPt) const;

		//	@brief	���㻡
		//	@param	[from]	��ʼλ��
		//	@param	[to]	Ŀ��λ��
		//	@param	[angle]	out	�����Ƕ�
		//	@return	����
		float				CalcArc(const NxVec3& from, const NxVec3& to, float& angle) const;

		//	@brief	����һԲ�Ŀռ��ϵ
		//	@param	[other]	��һ����Բ
		//	@return	�ռ��ϵ
		ECircleRelation		RelationWith(const DirectionalCircle& other) const;

		//	@brief	��ȡԲ��
		inline NxVec3		Center		()	const				{ return _Center; }
		inline void			Move		(const NxVec3& center)	{ _Center = center; }

		//	@brief	��ȡ�뾶
		inline float		Radius		()	const				{ return _Radius; }

		//	@brief	��ȡ����
		inline EDirection	Direction	()	const				{ return _Wise; }

	protected:
		//	@brief	Բ��
		NxVec3		_Center;

		//	@brief	�뾶
		float		_Radius;

		//	@brief	����
		EDirection	_Wise;
	};
}