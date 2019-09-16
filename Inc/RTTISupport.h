//------------------------------------------------------------------------
//	@brief	��̬����֧��
//	@note	ʵ���˼̳й�ϵ�Թ���
//	@author	chenpu
//	@date	2013-2-24
//------------------------------------------------------------------------
#pragma once

namespace GPL
{

	extern int _RTTI_SEED;

	struct DynamicTypeInfo
	{
		std::string				_TypeName;
		const int				_TypeID;
		const DynamicTypeInfo*	_ParentType;

		DynamicTypeInfo(const char* _name, const DynamicTypeInfo* _parent)
			: _TypeName(_name)
			, _TypeID(_RTTI_SEED++)
			, _ParentType(_parent)
		{
		}

		/************************************************************************/
		/* ���������                                                           */
		/************************************************************************/
		bool operator == (const DynamicTypeInfo& ref) const {

			return ( this->_TypeID == ref._TypeID );
		}

		bool operator != (const DynamicTypeInfo& ref) const {

			return ( this->_TypeID != ref._TypeID );
		}

		/************************************************************************/
		/* ����ת�����															*/
		/************************************************************************/
		bool isTypeOf(const DynamicTypeInfo& ref) const
		{
			if ( *this == ref )
				return true;

			return (NULL != _ParentType) ? _ParentType->isTypeOf(ref) : false;
		}

		/************************************************************************/
		/* ���θ�ֵ�����														*/
		/************************************************************************/
	private:
		const DynamicTypeInfo& operator = (const DynamicTypeInfo& ref);
	};

	//	RTTIʹ������
#define DECLARE_RTTI	\
public:\
	static const DynamicTypeInfo ms_RTTI;\
	virtual const DynamicTypeInfo& GetRTTI()	{ return this->ms_RTTI; }

	//	ʵ��RTTI����
#define IMPLEMENT_RTTI_BASE(this_class)	\
	const DynamicTypeInfo this_class::ms_RTTI(#this_class, NULL);

	//	ʵ��RTTI������
#define IMPLEMENT_RTTI(this_class, parent_class)	\
	const DynamicTypeInfo this_class::ms_RTTI(#this_class, &parent_class::ms_RTTI);

	//	��̬�����ж�
#define IsKind(class_name, base_ptr)	\
	((base_ptr != NULL) && base_ptr->GetRTTI().isTypeOf(class_name::ms_RTTI))

	//	��ȷ�����ж�
#define ExactlyIs(class_name, base_ptr)	\
	((base_ptr != NULL) && (base_ptr->GetRTTI() == class_name::ms_RTTI))

	//	��̬����ת��
#define DynamicCast(class_name, base_ptr)	\
	IsKind(class_name, base_ptr) ? (class_name*)base_ptr : NULL

	//	��ȷ����ת��
#define ExactlyCast(class_name, base_ptr)	\
	ExactlyIs(class_name, base_ptr) ? (class_name*)base_ptr : NULL

}	//	namespace GPL