#pragma once
#pragma warning(disable:4800)
#include "..\Inc\PhysXSupport.h"
#import <msxml3.dll>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <bitset>

#pragma warning(disable:4127)

namespace GPL
{

#ifdef _DEBUG
#ifdef _WINDOWS_
	//modify by yeleiyu@icee.cn for error alert
#define ErrorMessage(content) gplDebugf(content);

#endif
#else
#ifdef _WINDOWS_
#define ErrorMessage(content) MessageBox(NULL,content,NULL,MB_OK);
#endif
#endif

#define GET_ATTRIBUTE_ERROR \
	{	\
	std::wstring strErr = L"文件[";	\
	MSXML2::IXMLDOMDocument *pDOMDocument = NULL;	\
	elem->get_ownerDocument(&pDOMDocument);	\
	if(pDOMDocument!=NULL){	\
	BSTR strFilename;	\
	pDOMDocument->get_url(&strFilename);	\
	if(strFilename!=NULL){	\
	strErr += strFilename;	\
	}	\
	}	\
	strErr += L"]中，";	\
	strErr += L"变量[";	\
	strErr += strattr;	\
	strErr += L"]不存在或类型不对.\n\n";	\
	BSTR strText;	\
	elem->get_xml(&strText);	\
	if(strText!=NULL){	\
	strErr += strText;	\
	}	\
	ErrorMessage(strErr.c_str());	\
}

#define SIMPE_XML_ATTRIBUTE(a,b,c) ( XML2STRUCT_NODE( (TEXT(#a)),	XML2STRUCT_NODE::b,	ADDR_OFFSET(&c.a, &c ) ))
#define SIMPE_XML_ATTRIBUTE_DEFAULT(a,b,c) ( XML2STRUCT_NODE( (TEXT(#a)),	XML2STRUCT_NODE::b,	ADDR_OFFSET(&c.a, &c ), NULL, NULL, true ))
#define SIMPE_XML_ATTRIBUTE_EX(atrribute,type,struct,var) ( XML2STRUCT_NODE( (TEXT(#atrribute)),	XML2STRUCT_NODE::type,	ADDR_OFFSET(&struct.var, &struct ) ))
#define SIMPE_XML_ATTRIBUTE_EX_DEFAULT(atrribute,type,struct,var) ( XML2STRUCT_NODE( (TEXT(#atrribute)),	XML2STRUCT_NODE::type,	ADDR_OFFSET(&struct.var, &struct ), NULL, NULL, true ))

	bool GetChildElemNode(MSXML2::IXMLDOMNodePtr pParentNode, int nIndexChild, MSXML2::IXMLDOMElementPtr& pElemNode);

	bool GetChildElemNode(MSXML2::IXMLDOMNodePtr pParentNode, const TCHAR *strNodeName, MSXML2::IXMLDOMElementPtr& pElemNode);

	BOOL InitXML( MSXML2::IXMLDOMDocumentPtr& doc );

	void ReleaseXML(MSXML2::IXMLDOMDocumentPtr& doc );

	template <typename t> 
	BOOL GetAttribute( MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, t& ret, bool bUseDefaultValue = false )
	{
		try{
			::variant_t v;
			v = elem->getAttribute( strattr );
			if ( v.vt == VT_NULL && !bUseDefaultValue )
			{
				GET_ATTRIBUTE_ERROR
				return false;
			}

			if( v.vt != VT_NULL )
			{
				float tmpFloat = 0.0f;
				double tmpDouble = 0.0f;
				if (typeid(ret)==typeid(tmpFloat))
				{
					_com_util::CheckError(::VariantChangeTypeEx( &v, &v, 0x0804, 0, VT_R4 ));	// VariantChangeTypeEx 解决俄语/德语，分割的问题
					tmpFloat = v;
					ret = t(tmpFloat);
				} 
				else if(typeid(ret)==typeid(tmpDouble))
				{
					_com_util::CheckError(::VariantChangeTypeEx( &v, &v, 0x0804, 0, VT_R8 ));	// VariantChangeTypeEx 解决俄语/德语，分割的问题
					tmpDouble = v;
					ret = t(tmpDouble);
				}
				else if(typeid(ret)==typeid(int)) // 容错：配置表中有的整数配成了浮点数，导致在俄语环境下读不出来
				{
					_com_util::CheckError(::VariantChangeTypeEx( &v, &v, 0x0804, 0, VT_R8 ));	// VariantChangeTypeEx 解决俄语/德语，分割的问题
					tmpDouble = v;
					ret = t(tmpDouble);
				}
				else
				{
					ret = v;
				}
			}
			return TRUE;
		}
		catch(...)
		{
			GET_ATTRIBUTE_ERROR
			return false;
		}
	}

	BOOL GetAttribute( MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, std::wstring& ret, bool bUseDefaultValue = false );

	template <typename t> 
	BOOL SetAttribute( MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, const t& val )
	{
		::variant_t v;
		v = val;
		HRESULT hr = elem->setAttribute( strattr, v );
		return SUCCEEDED(hr);
	}

	BOOL SetAttribute(  MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, const std::wstring& val );

#define ADDR_OFFSET(a,b) (((unsigned int)((void*)(a))) - ((unsigned int)((void*)(b))))




	struct XML2STRUCT_NODE
	{
		enum type
		{
			t_stdstring,
			t_BOOL,
			t_int64,
			t_float,
			t_int,
			t_array,
			t_sub_node_array,
			t_sub_node_pointer_array,
			t_sub_node,
			t_nxvec3,
			t_nxmat34,
			t_node_switch,
			t_node,
			t_custom_node,
			t_int_array,
		};

		XML2STRUCT_NODE( const TCHAR* Aname, type t, int Aoffset, void* Aopt1 = NULL, void* Aopt2 = NULL, bool bUseDefaultValue = false )
			:name(Aname),data_type(t),offset(Aoffset),opt1(Aopt1), opt2(Aopt2), _bUseDefaultValue(bUseDefaultValue)
		{
		}


		void*	opt1;
		void*	opt2;
		int		offset;
		type	data_type;
		std::wstring name;
		bool	_bUseDefaultValue;
	};

	BOOL read_x2s( const XML2STRUCT_NODE& node, MSXML2::IXMLDOMElementPtr elem, unsigned char* data, void* AssignFunc );
	BOOL save_x2s_list ( const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem, unsigned char* data );
	BOOL read_x2s_list ( const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem, unsigned char* data, void* AssignFunc = NULL );
	template <typename t>
BOOL read_std_array( std::wstring node_name, std::vector<t>& Aarray, const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem )
	{
		for ( int i = 0; i < elem->GetchildNodes()->Getlength(); i++ )
		{
			MSXML2::IXMLDOMElementPtr p;
			if ( !xmlutil::GetChildElemNode( elem, i, p ) )
			{
				p.Release();
				return FALSE;
			}

			t tv;
			read_x2s_list( nodes, p, (unsigned char*)&tv );
			Aarray.push_back(tv);

			p.Release();
		}
		return FALSE;
	}
	//template <typename t>
	//	BOOL read_nx_array( std::wstring node_name, NxArray<t>& Aarray, const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem )
	//	{
	//		for ( int i = 0; i < elem->GetchildNodes()->Getlength(); i++ )
	//		{
	//			MSXML2::IXMLDOMElementPtr p;
	//			if ( !xmlutil::GetChildElemNode( elem, i, p ) )
	//				return FALSE;
	//			t tv;
	//			read_x2s_list( nodes, p, (unsigned char*)&tv );
	//			Aarray.pushBack(tv);
	//		}
	//		return FALSE;
	//	}
		struct node_switcher
	{
		INT_PTR v;
		std::vector<XML2STRUCT_NODE> x2s_nodes;
	};

	typedef void* (CALLBACK* ALLOC_ITEM_FUNC)( MSXML2::IXMLDOMElementPtr elem, void* container );
	typedef void* (CALLBACK* SWITCH_NODE_FUNC)(  MSXML2::IXMLDOMElementPtr elem, const void* cur_item );
	typedef BOOL (CALLBACK* CUSTOM_NODE_LOAD_FUNC)( MSXML2::IXMLDOMElementPtr elem, unsigned char* data );
	typedef void (CALLBACK* ASSIGN_ITEM_FUNC)(std::wstring& Src, void* Dst);
}