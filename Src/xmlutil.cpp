#include "..\inc\xmlutil.h"
using namespace MSXML2;

namespace GPL
{
	std::vector<std::wstring> split_string( const std::wstring& src, TCHAR c )
	{
		std::vector<std::wstring> r;
		std::wstring::size_type pos = 0;
		std::wstring::size_type pos2 = 0;
		if ( src.empty() )
			return r;
		while (true)
		{
			pos2 = pos;
			pos = src.find_first_of( c, pos2 );
			if ( pos == std::wstring::npos )
			{
				if ( !src.substr( pos2 ).empty() )
					r.push_back(src.substr( pos2 ));			
				break;
			}
			r.push_back( src.substr( pos2, pos - pos2 ) );
			pos++;
		}

		return r;
	}
	bool GetChildElemNode(MSXML2::IXMLDOMNodePtr pParentNode, int nIndexChild, MSXML2::IXMLDOMElementPtr& pElemNode)
	{
		MSXML2::IXMLDOMNodePtr pCurrNode;
		if ( pParentNode == NULL )
			return FALSE;
		pParentNode->childNodes->get_item(nIndexChild, &pCurrNode);
		if(pCurrNode->nodeType != MSXML2::NODE_ELEMENT)
		{
			pCurrNode.Release();
			return false;
		}

		HRESULT hr = pCurrNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElemNode);
		if(FAILED(hr))
		{
			pCurrNode.Release();
			return false;
		}

		pCurrNode.Release();
		return true;
	}

	bool GetChildElemNode(MSXML2::IXMLDOMNodePtr pParentNode, const TCHAR *strNodeName, MSXML2::IXMLDOMElementPtr& pElemNode)
	{
		MSXML2::IXMLDOMElementPtr pNode;
		pElemNode = NULL;
		for(int i=0; i<pParentNode->childNodes->length; i++)
		{
			if(!GetChildElemNode(pParentNode, i, pNode))
			{
				continue;
			}
			if(pNode->nodeName==_bstr_t(strNodeName)) 
			{
				pElemNode = pNode;
				pNode.Release();
				return true;
			}
		}
		pNode.Release();
		return FALSE;
	}

	BOOL InitXML( MSXML2::IXMLDOMDocumentPtr& doc )
	{
		HRESULT hr;
		::CoInitialize(NULL);
		hr = CoCreateInstance(__uuidof(MSXML2::DOMDocument30), NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&doc);
		if( FAILED(hr) )
		{
			return FALSE;
		}
		return TRUE;
	}

	void ReleaseXML(MSXML2::IXMLDOMDocumentPtr& doc )
	{
		doc.Release();

		::CoUninitialize();
	}

	BOOL GetAttribute( MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, std::wstring& ret, bool bUseDefaultValue )
	{
#if _DEBUG
		//::wprintf( L"read wstring attr at:%s %s\n", elem->GetnodeName().GetBSTR(), strattr );
#endif
		try{
			::variant_t v;
			v = elem->getAttribute( strattr );
			if ( v.vt == VT_NULL && !bUseDefaultValue )	// 没有此字段，且不使用默认值
			{
				GET_ATTRIBUTE_ERROR
				return FALSE;
			}

			if( v.vt == VT_NULL )
			{
				ret = L"";
			}
			else
			{
				ret = v.bstrVal;
			}
			return TRUE;
		}
		catch(...)
		{
			GET_ATTRIBUTE_ERROR
			return false;
		}
	}

//	BOOL GetAttribute( MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, DWORD& ret )
//	{
//#if _DEBUG
//		//::wprintf( L"read DWORD attr at:%s %s\n", elem->GetnodeName().GetBSTR(), strattr );
//#endif
//		try{
//			::variant_t v;
//			v = elem->getAttribute( strattr );
//			if ( v.vt == VT_NULL )
//			{
//				GET_ATTRIBUTE_ERROR
//					return FALSE;
//			}
//			ret = (DWORD)v.intVal;
//			return TRUE;
//		}
//		catch(...)
//		{
//			GET_ATTRIBUTE_ERROR
//			return false;
//		}
//	}

	BOOL SetAttribute(  MSXML2::IXMLDOMElementPtr elem, const TCHAR* strattr, const std::wstring& val )
	{
		::variant_t v;
		v = val.c_str();
		HRESULT hr = elem->setAttribute( strattr, v );
		return SUCCEEDED(hr);
	}



	BOOL read_x2s( const XML2STRUCT_NODE& node, MSXML2::IXMLDOMElementPtr elem, unsigned char* data, void* AssignFunc )
	{

		std::wstring	strTmp;
		bool			bTmp = false;
		__int64			i64Tmp = 0;
		int				iTmp = 0;
		float			fTmp = 0;
		NxVec3			vec3;
		NxMat34			mat34;

		switch (node.data_type)
		{
		case XML2STRUCT_NODE::t_custom_node:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_custom_node at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				if ( node.opt1 != NULL )
				{
					return ((CUSTOM_NODE_LOAD_FUNC)node.opt1)( elem, data );
				}
				else
					return FALSE;
			}
			break;
		case XML2STRUCT_NODE::t_stdstring:
#if _DEBUG
			//::wprintf( L"read x2snode t_stdstring at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
			if ( GetAttribute( elem, node.name.c_str(), strTmp, node._bUseDefaultValue ) == FALSE )
				return FALSE;
			if( AssignFunc == NULL )
			{
				*((std::wstring*)(data + node.offset)) = strTmp.c_str();
			}
			else
			{
				((ASSIGN_ITEM_FUNC)AssignFunc)(strTmp, (void*)(data + node.offset));
			}
			break;
		case XML2STRUCT_NODE::t_BOOL:
#if _DEBUG
			//::wprintf( L"read x2snode t_BOOL at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
			if ( GetAttribute( elem, node.name.c_str(), bTmp, node._bUseDefaultValue ) == FALSE )
				return FALSE;
			*((bool*)(data + node.offset)) = bTmp;
			break;
		case XML2STRUCT_NODE::t_int64:
#if _DEBUG
			//::wprintf( L"read x2snode t_int64 at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
			if ( GetAttribute( elem, node.name.c_str(), i64Tmp, node._bUseDefaultValue ) == FALSE )
				return FALSE;
			*((__int64*)(data + node.offset)) = i64Tmp;
			break;
		case XML2STRUCT_NODE::t_int:
#if _DEBUG
			//::wprintf( L"read x2snode t_int at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
			if ( GetAttribute( elem, node.name.c_str(), iTmp, node._bUseDefaultValue ) == FALSE )
				return FALSE;
			*((int*)(data + node.offset)) = iTmp;
			break;
		case XML2STRUCT_NODE::t_float:
#if _DEBUG
			//::wprintf( L"read x2snode t_float at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
			if ( GetAttribute( elem, node.name.c_str(), fTmp, node._bUseDefaultValue ) == FALSE )
				return FALSE;
			*((float*)(data + node.offset)) = fTmp;
			break;

		case XML2STRUCT_NODE::t_nxvec3:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_nxvec3 at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				if ( GetAttribute( elem, node.name.c_str(), strTmp, node._bUseDefaultValue ) == FALSE )
					return FALSE;
				std::vector<std::wstring> ss = split_string( strTmp, L',');
				if ( ss.size() >= 3 )
				{
					vec3.x = (NxReal)::_wtof(ss[0].c_str());
					vec3.y = (NxReal)::_wtof(ss[1].c_str());
					vec3.z = (NxReal)::_wtof(ss[2].c_str());
				}
				*((NxVec3*)(data + node.offset)) = vec3;
			}
			break;
		case XML2STRUCT_NODE::t_nxmat34:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_nxmat34 at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				if ( GetAttribute( elem, node.name.c_str(), strTmp, node._bUseDefaultValue ) == FALSE )
					return FALSE;
				std::vector<std::wstring> ss = split_string( strTmp, L',');
				if ( ss.size() >= 12 )
				{
					NxReal data[16];
					data[0] = (NxReal)_wtof(ss[0].c_str());
					data[1] = (NxReal)_wtof(ss[1].c_str());
					data[2] = (NxReal)_wtof(ss[2].c_str());
					data[4] = (NxReal)_wtof(ss[3].c_str());
					data[5] = (NxReal)_wtof(ss[4].c_str());
					data[6] = (NxReal)_wtof(ss[5].c_str());
					data[8] = (NxReal)_wtof(ss[6].c_str());
					data[9] = (NxReal)_wtof(ss[7].c_str());
					data[10] = (NxReal)_wtof(ss[8].c_str());
					data[12] = (NxReal)_wtof(ss[9].c_str());
					data[13] = (NxReal)_wtof(ss[10].c_str());
					data[14] = (NxReal)_wtof(ss[11].c_str());

					mat34.setColumnMajor44(data);
					mat34.M.setRowMajorStride4(data);
				}
				*((NxMat34*)(data + node.offset)) = mat34;
			}
			break;
		case XML2STRUCT_NODE::t_sub_node_array:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_sub_node_array at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				MSXML2::IXMLDOMNodePtr child = elem->GetfirstChild();
				while( child )
				{
#if _DEBUG
					//::wprintf( L"read t_sub_node_array child at:%s\n", child->GetnodeName().GetBSTR() );
#endif
					if ( child->GetnodeName() ==_bstr_t(node.name.c_str()) )
					{
						ALLOC_ITEM_FUNC pfunc = (ALLOC_ITEM_FUNC)node.opt2;
						void* data2 = pfunc(child, data+node.offset);
						if ( data2 == NULL )
						{
							child.Release();
							return FALSE;
						}
						read_x2s_list( *((const std::vector<XML2STRUCT_NODE>*)node.opt1), child, (unsigned char*)data2, AssignFunc);
					}
					child = child->GetnextSibling();
				}
				if( child != NULL )
				{
					child.Release();
				}
				break;
			}
		case XML2STRUCT_NODE::t_sub_node:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_sub_node at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				MSXML2::IXMLDOMNodePtr child = elem->GetfirstChild();
				while( child )
				{
					if ( child->GetnodeName() ==_bstr_t(node.name.c_str()) )
					{
						read_x2s_list( *((const std::vector<XML2STRUCT_NODE>*)node.opt1), child, (unsigned char*)(data+node.offset), AssignFunc);
						break;
					}
					child = child->GetnextSibling();
				}
				if( child != NULL )
				{
					child.Release();
				}
				break;
			}
		case XML2STRUCT_NODE::t_int_array:
			{
#if _DEBUG
				//::wprintf( L"read x2snode t_int_array at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
				if ( GetAttribute( elem, node.name.c_str(), strTmp, node._bUseDefaultValue ) == FALSE )
					return FALSE;
				std::vector<std::wstring> ss = split_string( strTmp, L',');
				std::vector<int> intVec;
				for(std::vector<std::wstring>::iterator it = ss.begin(); it != ss.end(); ++it)
				{

					intVec.push_back(_wtoi(it->c_str()));
				}
				*((std::vector<int>*)(data + node.offset)) = intVec;
				break;
			}
		default:
			assert(0);
			break;
		}

		return TRUE;
	}

	BOOL read_x2s_list ( const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem, unsigned char* data, void* AssignFunc )
	{
#if _DEBUG
		//::wprintf( L"[begin ]read x2s list at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
		BOOL bRet = TRUE;
		for ( size_t i = 0; i < nodes.size(); i++ )
			if ( read_x2s( nodes[i], elem, data, AssignFunc ) == FALSE )
				bRet = FALSE;
#if _DEBUG
		//::wprintf( L"[end   ]read x2s list at:%s\n", elem->GetnodeName().GetBSTR() );
#endif
		return bRet;
	}


	BOOL save_x2s( const XML2STRUCT_NODE& node, MSXML2::IXMLDOMElementPtr elem, unsigned char* data )
	{
		std::wstring	strTmp;
		bool			bTmp = false;
		__int64			i64Tmp = 0;
		int				iTmp = 0;
		float			fTmp = 0;
		switch (node.data_type)
		{
		case XML2STRUCT_NODE::t_stdstring:
			strTmp = *((std::wstring*)(data + node.offset));
			if ( SetAttribute( elem, node.name.c_str(), strTmp ) == FALSE )
				return FALSE;
			break;
		case XML2STRUCT_NODE::t_BOOL:
			bTmp = *((bool*)(data + node.offset));
			if ( SetAttribute( elem, node.name.c_str(), bTmp ) == FALSE )
				return FALSE;
			break;
		case XML2STRUCT_NODE::t_int64:
			i64Tmp = *((__int64*)(data + node.offset));
			if ( SetAttribute( elem, node.name.c_str(), i64Tmp ) == FALSE )
				return FALSE;
			break;
		case XML2STRUCT_NODE::t_int:
			iTmp = *((int*)(data + node.offset));
			if ( SetAttribute( elem, node.name.c_str(), iTmp ) == FALSE )
				return FALSE;
			break;
		case XML2STRUCT_NODE::t_float:
			fTmp = *((float*)(data + node.offset));
			if ( SetAttribute( elem, node.name.c_str(), fTmp ) == FALSE )
				return FALSE;
			break;

			//case XML2STRUCT_NODE::t_sub_node_array:
			//	{
			//		MSXML2::IXMLDOMElementPtr child = elem->GetfirstChild();
			//		while( child )
			//		{
			//			if ( child->GetnodeName() ==_bstr_t(node.name.c_str()) )
			//			{
			//				ALLOC_ITEM_FUNC pfunc = (ALLOC_ITEM_FUNC)node.opt2;
			//				void* data2 = pfunc(data+node.offset);
			//				const std::vector<XML2STRUCT_NODE>* pnode = (std::vector<XML2STRUCT_NODE>*)node.opt1;
			//				read_x2s_list( *((const std::vector<XML2STRUCT_NODE>*)node.opt1), child, (unsigned char*)data2);
			//			}
			//			child = child->GetnextSibling();
			//		}
			//		break;
			//	}
			//case XML2STRUCT_NODE::t_sub_node:
			//	{
			//		MSXML2::IXMLDOMElementPtr child = elem->GetfirstChild();
			//		while( child )
			//		{
			//			if ( child->GetnodeName() ==_bstr_t(node.name.c_str()) )
			//			{
			//				const std::vector<XML2STRUCT_NODE>* pnode = (std::vector<XML2STRUCT_NODE>*)node.opt1;
			//				read_x2s_list( *((const std::vector<XML2STRUCT_NODE>*)node.opt1), child, (unsigned char*)(data+node.offset));
			//				break;
			//			}
			//			child = child->GetnextSibling();
			//		}
			//		break;
			//	}
		}

		return TRUE;
	}

	BOOL save_x2s_list ( const std::vector<XML2STRUCT_NODE>& nodes, MSXML2::IXMLDOMElementPtr elem, unsigned char* data )
	{
		BOOL bRet = TRUE;
		for ( size_t i = 0; i < nodes.size(); i++ )
			if ( save_x2s( nodes[i], elem, data ) == FALSE )
				bRet = FALSE;
		return bRet;
	}


}
