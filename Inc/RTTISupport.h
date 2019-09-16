//------------------------------------------------------------------------
//	@brief	动态类型支持
//	@note	实现了继承关系自管理
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
		/* 运算符重载                                                           */
		/************************************************************************/
		bool operator == (const DynamicTypeInfo& ref) const {

			return ( this->_TypeID == ref._TypeID );
		}

		bool operator != (const DynamicTypeInfo& ref) const {

			return ( this->_TypeID != ref._TypeID );
		}

		/************************************************************************/
		/* 类型转换检查															*/
		/************************************************************************/
		bool isTypeOf(const DynamicTypeInfo& ref) const
		{
			if ( *this == ref )
				return true;

			return (NULL != _ParentType) ? _ParentType->isTypeOf(ref) : false;
		}

		/************************************************************************/
		/* 屏蔽赋值运算符														*/
		/************************************************************************/
	private:
		const DynamicTypeInfo& operator = (const DynamicTypeInfo& ref);
	};

	//	RTTI使用声明
#define DECLARE_RTTI	\
public:\
	static const DynamicTypeInfo ms_RTTI;\
	virtual const DynamicTypeInfo& GetRTTI()	{ return this->ms_RTTI; }

	//	实现RTTI基类
#define IMPLEMENT_RTTI_BASE(this_class)	\
	const DynamicTypeInfo this_class::ms_RTTI(#this_class, NULL);

	//	实现RTTI派生类
#define IMPLEMENT_RTTI(this_class, parent_class)	\
	const DynamicTypeInfo this_class::ms_RTTI(#this_class, &parent_class::ms_RTTI);

	//	动态类型判断
#define IsKind(class_name, base_ptr)	\
	((base_ptr != NULL) && base_ptr->GetRTTI().isTypeOf(class_name::ms_RTTI))

	//	精确类型判断
#define ExactlyIs(class_name, base_ptr)	\
	((base_ptr != NULL) && (base_ptr->GetRTTI() == class_name::ms_RTTI))

	//	动态类型转换
#define DynamicCast(class_name, base_ptr)	\
	IsKind(class_name, base_ptr) ? (class_name*)base_ptr : NULL

	//	精确类型转换
#define ExactlyCast(class_name, base_ptr)	\
	ExactlyIs(class_name, base_ptr) ? (class_name*)base_ptr : NULL

}	//	namespace GPL