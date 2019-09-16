#include "..\Inc\PhysXSupport.h"

namespace GPL
{
	IMPLEMENT_RTTI_BASE(ComponentDesc)
	IMPLEMENT_RTTI(VehicleAttachedCompDesc, ComponentDesc)
	IMPLEMENT_RTTI(WeaponCompDesc, VehicleAttachedCompDesc)
	IMPLEMENT_RTTI(AircraftCompDesc, ComponentDesc)
	IMPLEMENT_RTTI(VehicleCompDesc, ComponentDesc)
	IMPLEMENT_RTTI(ProjectileCompDesc, ComponentDesc)
	IMPLEMENT_RTTI(StaticCompDesc, ComponentDesc)
	IMPLEMENT_RTTI(SceneBlockingCompDesc, StaticCompDesc)
	IMPLEMENT_RTTI(SceneTriggerCompDesc, StaticCompDesc)
	IMPLEMENT_RTTI(SceneOceanCompDesc, StaticCompDesc)
	IMPLEMENT_RTTI(SceneTerrainCompDesc, StaticCompDesc)
	IMPLEMENT_RTTI(ScenePlayerStartCompDesc, StaticCompDesc)
	IMPLEMENT_RTTI(ScenePlayerRespawnCompDesc, ScenePlayerStartCompDesc)

	IMPLEMENT_RTTI_BASE(ComponentData)
	IMPLEMENT_RTTI(VehicleAttachedCompData, ComponentData)
	IMPLEMENT_RTTI(WeaponData, VehicleAttachedCompData)
	IMPLEMENT_RTTI(VehicleData, ComponentData)
	IMPLEMENT_RTTI(AircraftData, ComponentData)
	IMPLEMENT_RTTI(ProjectileData, ComponentData)
	IMPLEMENT_RTTI(SceneStaticData, ComponentData)
	IMPLEMENT_RTTI(ScenePlayerStartData, SceneStaticData)
	IMPLEMENT_RTTI(ScenePlayerRespawnData, ScenePlayerStartData)
	IMPLEMENT_RTTI(SceneBlockingData, SceneStaticData)
	IMPLEMENT_RTTI(SceneTriggerData, SceneStaticData)
	IMPLEMENT_RTTI(SceneOceanData, SceneStaticData)
	IMPLEMENT_RTTI(SceneTerrainData, SceneStaticData)

//add by yeleiyu@icee.cn for xml encrypt and decrypt
/** Class that handles the ANSI to TCHAR conversion */
class UtilityANSIToTCHAR_Convert
{
public:
	static  void Convert(const unsigned char* Source, std::wstring& str,DWORD Size)
	{
		DWORD Length =Size + 2;	
		TCHAR* Dest = new TCHAR[Length * sizeof(TCHAR)];
		MultiByteToWideChar(CP_ACP,0,(char*)Source,Length,Dest,Length);
		str = (TCHAR*)Dest;
		delete [] Dest;		
	}
};
//add by yeleiyu@icee.cn for xml encrypt and decrypt
void DecryptXML( unsigned char* pData, unsigned int len )
{
	unsigned char Mask[1024];
	Mask[0] = ((len + 6) * 13) % 256;
	for ( int i = 1; i < 1024; i++ )
		Mask[i] = (Mask[i-1] * 17 + 35) & 0xff;
	for ( unsigned int i = 0; i < len; i++ )
	{
		pData[i] ^= Mask[i%1024];
	}
}
//add by yeleiyu@icee.cn for xml encrypt and decrypt
BOOL LoadFileToString( const TCHAR* filename, std::wstring& str,bool EnableReadEncryptedXML =FALSE)
{
	HANDLE f = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( f == INVALID_HANDLE_VALUE )
		return FALSE;
	DWORD dwSize = ::GetFileSize( f, NULL );
	DWORD dwReaded = 0;
	unsigned char* Content = new unsigned char[dwSize+2];
	ZeroMemory( Content, dwSize+2 );
	BOOL bRet = ReadFile( f, Content, dwSize, &dwReaded, NULL );	
	CloseHandle( f );
	if ( bRet == FALSE || dwReaded != dwSize )
	{
		delete [] Content;
		return FALSE;
	}
	if ( EnableReadEncryptedXML == TRUE )
	{
		unsigned char Mask[1024];
		Mask[0] = ((dwSize + 6) * 13) % 256;
		for ( int i = 1; i < 1024; i++ )
			Mask[i] = (Mask[i-1] * 17 + 35) & 0xff;
		for ( DWORD i = 0; i < dwSize; i++ )
		{
			Content[i] ^= Mask[i%1024];
		}
	}

	if((Content[0] == 0xFF && Content[1] == 0xFE)
		|| (Content[0] == 0xFE&& Content[1] == 0xFF ))
	{
		TCHAR* temp  =new TCHAR[dwSize+2];
		ZeroMemory( temp, dwSize+2 );
		memcpy((unsigned char*) temp,(unsigned char*) (Content+2), dwSize );
		str =(TCHAR*)temp;

		delete [] temp;
	}
	else if(Content[0] == 0xEF && Content[1] == 0xBB && Content[2] == 0xBF)
	{
		TCHAR* temp  =new TCHAR[dwSize+3];
		ZeroMemory( temp, dwSize+2 );
		memcpy((unsigned char*) temp,(unsigned char*) (Content+3), dwSize );
		str =(TCHAR*)temp;

		delete [] temp;
	}
	else 
	{
		UtilityANSIToTCHAR_Convert::Convert(Content,str,dwSize);

	}


	delete [] Content;
	return TRUE;
}
//add by yeleiyu@icee.cn for xml encrypt and decrypt
bool LoadXML(const TCHAR* filename,MSXML2::IXMLDOMDocumentPtr& pXMLDoc ,bool EnableReadEncryptedXML = FALSE)
{
	if( EnableReadEncryptedXML == TRUE )
	{
		std::wstring content;

		if ( LoadFileToString( filename, content ,EnableReadEncryptedXML ) == FALSE )
			return false;

		if(pXMLDoc->loadXML(content.c_str()) != VARIANT_TRUE)
		{

			return false;		}
	
	}
	else
	{
		if(pXMLDoc->load(filename) != VARIANT_TRUE)
		{

			return false;
		}
	}

	return true;
}

	void ToUpperString(std::wstring& Str)
	{
		transform(Str.begin(), Str.end(), Str.begin(), toupper);
	}

	void ToLowerString(std::wstring& Str)
	{
		transform(Str.begin(), Str.end(), Str.begin(), tolower);
	}

	std::map<std::wstring, ScenePhysDesc*> GScenePhysDescList;		// map name to cached desc	

	bool LoadScenePhys(const std::wstring& mapName, std::vector<ComponentDesc*>** pCompDescList, std::vector<PhysModelDesc>** pModelDescList)
	{
		if (GScenePhysDescList.find(mapName) != GScenePhysDescList.end())
		{
			*pCompDescList = &(GScenePhysDescList[mapName]->_SceneComponentDescList);
			*pModelDescList = &(GScenePhysDescList[mapName]->_ScenePhysModelDescList);
		}
		else
		{
			ScenePhysDesc* SceneDesc = new ScenePhysDesc();
			//try to load the map
			std::wstring workDir = mapName;
			workDir += TEXT("\\");
			workDir += mapName;
			std::wstring modelName = workDir + TEXT("_Model.xml");
			//modify by yeleiyu@icee.cn
			if (LoadPhysModels(modelName, SceneDesc->_ScenePhysModelDescList,GEnableReadEncryptedXML) == false)
			{
				delete SceneDesc;
				return false;
			}
			//modify by yeleiyu@icee.cn
			std::wstring compName = workDir + TEXT("_Component.xml");
			if (LoadComponents(compName, SceneDesc->_SceneComponentDescList,GEnableReadEncryptedXML) == false)
			{
				delete SceneDesc;
				return false;
			}

			GScenePhysDescList[mapName] = SceneDesc;
			*pCompDescList = &(GScenePhysDescList[mapName]->_SceneComponentDescList);
			*pModelDescList = &(GScenePhysDescList[mapName]->_ScenePhysModelDescList);
			return true;
		}

		return true;
	}

	void ReleaseScenePhys(const std::wstring& mapName)
	{
		std::map<std::wstring, ScenePhysDesc*>::iterator iter = GScenePhysDescList.find(mapName);
		if (iter != GScenePhysDescList.end())
		{
			SAFE_DELETE(iter->second);
			GScenePhysDescList.erase(iter);
		}		
	}

	bool LoadPhysModelDesc(std::wstring PhysId, PhysModelDesc* &ModelDesc)
	{
		
		std::transform(PhysId.begin(),PhysId.end(),PhysId.begin(),tolower);

		std::vector<PhysModelDesc>::iterator it = GPhysModelDescList.begin();
		for(; it != GPhysModelDescList.end(); ++it)
		{
			std::wstring FindID = (*it)._PhysId;
			std::transform(FindID.begin(),FindID.end(),FindID.begin(),tolower);
			if( FindID == PhysId )
			{
				ModelDesc = &(*it);
				return true;
			}
		}
		gplDebugf(TEXT("加载模型数据失败，PhysId[%d]"), PhysId);
		return false;
	}

	bool LoadComponentDesc(int CompId, ComponentDesc* &CompDesc)
	{
		ComponentDescMap::iterator mapIt = GComponentDescTable.find(CompId);
		if (mapIt != GComponentDescTable.end())
		{
			CompDesc = mapIt->second;
			return true;
		}
		else
		{
			//gplDebugf(TEXT("Unable to find ComponentDesc from GComponentDescTable"));
			ComponentDescList::iterator it = GComponentDescList.begin();
			for (; it != GComponentDescList.end(); ++it)
			{
				if( (*it)->_CompId == CompId )
				{
					CompDesc = (*it);
					return true;
				}
			}
		}

		gplDebugf(TEXT("加载组件逻辑数据失败，CompId[%d]"), CompId);
		return false;
	}

	void ComponentData::InitPhysModelDesc()
	{
		if( _CompDesc != NULL )
		{
			if( _CompDesc->IsValid() )
			{
				if(IsKind(SceneStaticData, this) && _CompDesc->_PhysId != TEXT(""))
				{
					//找到对应场景配置文件
					StaticCompDesc* scd = (StaticCompDesc*)_CompDesc;

					if (scd->_MapName == TEXT(""))
					{ 
						//	没有MapName时，在PhysModels.xml中查找
						if( !LoadPhysModelDesc(_CompDesc->_PhysId, _PhysModelDesc) )
						{
							gplDebugf(TEXT("[GPL] ERROR: 配置数据错误 InitPhysModelDesc[%d]"), _CompDesc->_CompId);
						}
					}
					else
					{
						//	尝试在地图目录下查找
						std::vector<ComponentDesc*>* pCompDescList = NULL;
						std::vector<PhysModelDesc>* pModelDescList = NULL;
						if (LoadScenePhys(scd->_MapName, &pCompDescList, &pModelDescList))
						{
							std::vector<PhysModelDesc>::iterator it = pModelDescList->begin();
							for (; it != pModelDescList->end(); ++it)
							{
								if ( (*it)._PhysId == _CompDesc->_PhysId )
								{
									_PhysModelDesc = &(*it);
									break;
								}
							}
						}
						else 
						{
							// 根据MapName查找失败时，再次尝试在PhysModels.xml中查找
							if ( !LoadPhysModelDesc(_CompDesc->_PhysId, _PhysModelDesc) )
							{
								gplDebugf(TEXT("[GPL] ERROR: 配置数据错误 InitPhysModelDesc[%d]"), _CompDesc->_CompId);
								gplDebugf(TEXT("加载场景[%s]中的模型数据失败，PhysId[%s]"), scd->_MapName.c_str(), _CompDesc->_PhysId);
							}
						}
					}
					
				}
				else
				{
					if( _CompDesc->_PhysId != TEXT("") )	// 炮弹无碰撞体
					{
						LoadPhysModelDesc(_CompDesc->_PhysId, _PhysModelDesc);
					}
				}				
			}
			else
			{
				gplDebugf(TEXT("[GPL] ERROR: 配置数据错误 InitPhysModelDesc[%d]"), _CompDesc->_CompId);
			}
		}
	}
	
	ComponentData* ConstructComponentData(ComponentDesc* CompDesc)
	{
		if(CompDesc == NULL)
		{
			return NULL;
		}

		ComponentData* NewData = NULL;
		switch(CompDesc->_ConfigType)
		{
		case ECT_Default:
			NewData = new ComponentData(CompDesc);
			break;
		case ECT_Attached_Comp:
			NewData = new VehicleAttachedCompData(CompDesc);
			break;
		case ECT_Attached_Weapon:
			NewData = new WeaponData(CompDesc);
			break;
		case ECT_Vehicle:
			NewData = new VehicleData(CompDesc);
			break;
		case ECT_Aircraft:
			NewData = new AircraftData(CompDesc);
			break;
		case ECT_Projectile:
			NewData = new ProjectileData(CompDesc);
			break;
		case ECT_Static:
			NewData = new SceneStaticData(CompDesc);
			break;
		case ECT_Ocean:
			NewData = new SceneOceanData(CompDesc);
			break;
		case ECT_Terrain:
			NewData = new SceneTerrainData(CompDesc);
			break;
		case ECT_BlockingBrush:
			NewData = new SceneBlockingData(CompDesc);
			break;
		case ECT_TriggerBrush:
			NewData = new SceneTriggerData(CompDesc);
			break;
		case ECT_PlayerStart:
			NewData = new ScenePlayerStartData(CompDesc);
			break;
		case ECT_PlayerRespawn:
			NewData = new ScenePlayerRespawnData(CompDesc);
			break;
		default:
			gplDebugf(TEXT("[GPL] Failed to ConstructComponentData, because unknown config type[%d]"), CompDesc->_ConfigType);
		}
		if( NewData != NULL )
		{
			NewData->Init();
		}

		return NewData;
	}
	
	

	MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
	{
	}

	MemoryWriteBuffer::~MemoryWriteBuffer()
	{
		NX_DELETE_ARRAY(data);
	}

	void MemoryWriteBuffer::clear()
	{
		currentSize = 0;
	}

	NxStream& MemoryWriteBuffer::storeByte(NxU8 b)
	{
		storeBuffer(&b, sizeof(NxU8));
		return *this;
	}
	NxStream& MemoryWriteBuffer::storeWord(NxU16 w)
	{
		storeBuffer(&w, sizeof(NxU16));
		return *this;
	}
	NxStream& MemoryWriteBuffer::storeDword(NxU32 d)
	{
		storeBuffer(&d, sizeof(NxU32));
		return *this;
	}
	NxStream& MemoryWriteBuffer::storeFloat(NxReal f)
	{
		storeBuffer(&f, sizeof(NxReal));
		return *this;
	}
	NxStream& MemoryWriteBuffer::storeDouble(NxF64 f)
	{
		storeBuffer(&f, sizeof(NxF64));
		return *this;
	}
	NxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, NxU32 size)
	{
		NxU32 expectedSize = currentSize + size;
		if(expectedSize > maxSize)
		{
			maxSize = expectedSize + 4096;

			NxU8* newData = new NxU8[maxSize];
			NX_ASSERT(newData!=NULL);

			if(data)
			{
				memcpy(newData, data, currentSize);
				delete[] data;
			}
			data = newData;
		}
		memcpy(data+currentSize, buffer, size);
		currentSize += size;
		return *this;
	}


	MemoryReadBuffer::MemoryReadBuffer(const NxU8* data) : buffer(data)
	{
	}

	MemoryReadBuffer::~MemoryReadBuffer()
	{
		// We don't own the data => no delete
	}

	NxU8 MemoryReadBuffer::readByte() const
	{
		NxU8 b;
		memcpy(&b, buffer, sizeof(NxU8));
		buffer += sizeof(NxU8);
		return b;
	}

	NxU16 MemoryReadBuffer::readWord() const
	{
		NxU16 w;
		memcpy(&w, buffer, sizeof(NxU16));
		buffer += sizeof(NxU16);
		return w;
	}

	NxU32 MemoryReadBuffer::readDword() const
	{
		NxU32 d;
		memcpy(&d, buffer, sizeof(NxU32));
		buffer += sizeof(NxU32);
		return d;
	}

	float MemoryReadBuffer::readFloat() const
	{
		float f;
		memcpy(&f, buffer, sizeof(float));
		buffer += sizeof(float);
		return f;
	}

	double MemoryReadBuffer::readDouble() const
	{
		double f;
		memcpy(&f, buffer, sizeof(double));
		buffer += sizeof(double);
		return f;
	}

	void MemoryReadBuffer::readBuffer(void* dest, NxU32 size) const
	{
		memcpy(dest, buffer, size);
		buffer += size;
	}




	//////////////////////////// 配置数据读取功能
	void* CALLBACK socket_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		std::vector<Socket>* sockets = (std::vector<Socket>*)arr;
		Socket t;
		sockets->push_back(t);
		return &(sockets->back());
	}

	void* CALLBACK shape_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		if ( elem == NULL )
			return NULL;
		int type;
		if ( GetAttribute( elem, L"_ShapeType", type ) == FALSE )
			return NULL;
		ShapeDesc* shape;
		std::wstring rlst;
		if(GetAttribute(elem, L"_DatSrc", rlst) == FALSE)
			return NULL;
		switch (type)
		{
		case EST_SHAPE_BOX:
			shape = new BoxShapeDesc();
			break;
		case EST_SHAPE_PLANE:
			shape = new PlaneShapeDesc();
			break;
		case EST_SHAPE_CAPSULE:
			shape = new CapsuleShapeDesc();
			break;
		case EST_SHAPE_CONVEX:
			shape = new ConvexShapeDesc();
			break;
		case EST_SHAPE_SPHERE:
			shape = new SphereShapeDesc();
			break;
		case EST_SHAPE_HEIGHTFIELD:
			shape = new HeightFieldShapeDesc();
			break;
		default:
			return NULL;
		}
		std::vector<GPL::ShapeDesc*>* p;
		p = (std::vector<GPL::ShapeDesc*>*)arr;
		p->push_back(shape);
		return (p->back());
	}

	void* CALLBACK model_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		GPL::PhysModelDesc tmp;
		std::vector<GPL::PhysModelDesc>* p;
		p = (std::vector<GPL::PhysModelDesc>*)arr;
		p->push_back(tmp);
		return &(p->back());
	}

	void* CALLBACK vector_nxvec3_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		NxVec3 tmp;
		std::vector<NxVec3>* p;
		p = (std::vector<NxVec3>*)arr;
		p->push_back(tmp);
		return &(p->back());
	}


	void* CALLBACK LimitAngle_alloc_item(MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		LimitAngle tmp ;
		std::vector<LimitAngle>* p;
		p = (std::vector<LimitAngle>*)arr;
		p->push_back(tmp);
		return &(p->back());
	}

	BOOL CALLBACK shape_custom_load( MSXML2::IXMLDOMElementPtr elem, unsigned char* data )
	{
		std::vector<XML2STRUCT_NODE> x2s_shape;
		std::vector<XML2STRUCT_NODE> x2s_shape_box;
		std::vector<XML2STRUCT_NODE> x2s_shape_plane;
		std::vector<XML2STRUCT_NODE> x2s_shape_capsule;
		std::vector<XML2STRUCT_NODE> x2s_shape_convex;
		std::vector<XML2STRUCT_NODE> x2s_shape_convex_vert;
		std::vector<XML2STRUCT_NODE> x2s_shape_convex_face;
		std::vector<XML2STRUCT_NODE> x2s_shape_convex_verts;
		std::vector<XML2STRUCT_NODE> x2s_shape_convex_faces;
		std::vector<XML2STRUCT_NODE> x2s_shape_sphere;
		
		{
			ConvexShapeDesc t;
			x2s_shape_convex_vert.push_back( XML2STRUCT_NODE(L"Position",XML2STRUCT_NODE::t_nxvec3,0 ));
			x2s_shape_convex_face.push_back( XML2STRUCT_NODE(L"Triangle",XML2STRUCT_NODE::t_nxvec3,0 ));
			x2s_shape_convex_verts.push_back( XML2STRUCT_NODE(L"Vert",XML2STRUCT_NODE::t_sub_node_array,0,&x2s_shape_convex_vert,(void*)vector_nxvec3_alloc_item ));
			x2s_shape_convex_faces.push_back( XML2STRUCT_NODE(L"Face",XML2STRUCT_NODE::t_sub_node_array,0,&x2s_shape_convex_face,(void*)vector_nxvec3_alloc_item ));
			x2s_shape_convex.push_back( XML2STRUCT_NODE( L"Faces",	XML2STRUCT_NODE::t_sub_node,ADDR_OFFSET( &t._Indeices,&t), (unsigned char*)&x2s_shape_convex_faces ) );
			x2s_shape_convex.push_back( XML2STRUCT_NODE( L"Verts",	XML2STRUCT_NODE::t_sub_node,ADDR_OFFSET( &t._Verts,&t), (unsigned char*)&x2s_shape_convex_verts ));
			x2s_shape_convex.push_back( SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t ) );
			x2s_shape_convex.push_back( SIMPE_XML_ATTRIBUTE(_bTriggerShape, t_BOOL, t ) );
		}
		{
			BoxShapeDesc t;
			x2s_shape_box.push_back( SIMPE_XML_ATTRIBUTE(_Dimensions, t_nxvec3, t ) );
			x2s_shape_box.push_back( SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t ) );			
			x2s_shape_box.push_back( SIMPE_XML_ATTRIBUTE(_bTriggerShape, t_BOOL, t ) );
		}
		{
			SphereShapeDesc t;
			x2s_shape_sphere.push_back( SIMPE_XML_ATTRIBUTE(_Radius, t_float, t ) );
			x2s_shape_sphere.push_back( SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t ) );
			x2s_shape_sphere.push_back( SIMPE_XML_ATTRIBUTE(_bTriggerShape, t_BOOL, t ) );
		}
		{
			PlaneShapeDesc t;
			x2s_shape_plane.push_back( SIMPE_XML_ATTRIBUTE(_Normal, t_nxvec3, t ) );
			x2s_shape_plane.push_back( SIMPE_XML_ATTRIBUTE(_D, t_float, t ) );
			x2s_shape_plane.push_back( SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t ) );
			x2s_shape_plane.push_back( SIMPE_XML_ATTRIBUTE(_bTriggerShape, t_BOOL, t ) );
		} 
		{  
			CapsuleShapeDesc t;
			x2s_shape_capsule.push_back( SIMPE_XML_ATTRIBUTE(_Radius, t_float, t ) );
			x2s_shape_capsule.push_back( SIMPE_XML_ATTRIBUTE(_Height, t_float, t ) );
			x2s_shape_capsule.push_back( SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t ) );
			x2s_shape_capsule.push_back( SIMPE_XML_ATTRIBUTE(_bTriggerShape, t_BOOL, t ) );
		}
		{
			ShapeDesc t;
			x2s_shape.push_back( SIMPE_XML_ATTRIBUTE(_ShapeType, t_int, t ) );
			x2s_shape.push_back( SIMPE_XML_ATTRIBUTE(_DatSrc, t_stdstring, t ) );
		}

		read_x2s_list( x2s_shape, elem, data );

		if ( ((ShapeDesc*)(data))->_DatSrc != TEXT("") ) 
			return TRUE; //延迟加载pml文件

		switch( ((ShapeDesc*)(data))->_ShapeType )
		{
		case EST_SHAPE_BOX:
			read_x2s_list( x2s_shape_box, elem, data );
			break;
		case EST_SHAPE_PLANE:
			read_x2s_list( x2s_shape_plane, elem, data );
			break;
		case EST_SHAPE_CAPSULE:
			read_x2s_list( x2s_shape_capsule, elem, data );
			break;
		case EST_SHAPE_CONVEX:
			read_x2s_list( x2s_shape_convex, elem, data );
			break;
		case EST_SHAPE_SPHERE:
			read_x2s_list( x2s_shape_sphere, elem, data );
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	bool ReadPMLShapes(std::vector<ShapeDesc*>& ShapeDescCopy, const std::wstring& datSrc)
	{		
		if ( datSrc != TEXT("") )
		{
			std::wstring FullPath = GWorkingPath + datSrc;

			//解析pml文件装配成ShapeDesc
			FILE* pf;
			errno_t err = _wfopen_s( &pf, FullPath.c_str(), L"rb" );
			if( err != 0 )
			{
				gplDebugf(TEXT("ERROR: Faied to Load PML files :%s "), FullPath.c_str());
				return FALSE;
			}			

			float TM[16];
			while(!feof(pf) )
			{
				char cType;
				fread(&cType, sizeof(char), 1, pf);
				if(feof(pf)) break;
				switch(cType)
				{
				case GPL::EST_SHAPE_BOX:
					{
						BoxShapeDesc* boxShapeDesc = new BoxShapeDesc();
						boxShapeDesc->_DatSrc = FullPath;
						boxShapeDesc->_ShapeType = EST_SHAPE_BOX;
						fread(&boxShapeDesc->_Dimensions.x, sizeof(float), 1, pf);
						fread(&boxShapeDesc->_Dimensions.y, sizeof(float), 1, pf);
						fread(&boxShapeDesc->_Dimensions.z, sizeof(float), 1, pf);
						fread(TM, sizeof(float), 16, pf);
						boxShapeDesc->_LocalPose.setColumnMajor44(TM);			
						ShapeDescCopy.push_back(boxShapeDesc);
					}
					break;
				case GPL::EST_SHAPE_SPHERE:
					{
						SphereShapeDesc* sphereShapeDesc = new SphereShapeDesc();
						sphereShapeDesc->_DatSrc = FullPath;
						sphereShapeDesc->_ShapeType = EST_SHAPE_SPHERE;
						fread(&sphereShapeDesc->_Radius, sizeof(float), 1, pf);
						fread(TM, sizeof(float), 16, pf);
						sphereShapeDesc->_LocalPose.setColumnMajor44(TM);
						ShapeDescCopy.push_back(sphereShapeDesc);
					}
					break;
				case GPL::EST_SHAPE_CAPSULE:
					{
						CapsuleShapeDesc* capsuleShapeDesc = new CapsuleShapeDesc();
						capsuleShapeDesc->_DatSrc = FullPath;
						capsuleShapeDesc->_ShapeType = EST_SHAPE_CAPSULE;
						fread(&capsuleShapeDesc->_Radius, sizeof(float), 1, pf);
						fread(&capsuleShapeDesc->_Height, sizeof(float), 1, pf);
						fread(TM, sizeof(float), 16, pf);
						capsuleShapeDesc->_LocalPose.setColumnMajor44(TM);
						ShapeDescCopy.push_back(capsuleShapeDesc);
					}
					break;
				case GPL::EST_SHAPE_CONVEX:
					{
						int numVer;
						int ptStrideByte;
						int flag;
						int numInd;
						ConvexShapeDesc* conMeshDesc = new ConvexShapeDesc();
						conMeshDesc->_DatSrc = FullPath;
						conMeshDesc->_ShapeType = EST_SHAPE_CONVEX;
						fread(&numVer, sizeof(int), 1, pf);
						fread(&ptStrideByte, sizeof(int), 1, pf);
						fread(&flag, sizeof(int), 1, pf);
						NxVec3* buff = new NxVec3[numVer];
						fread(buff, ptStrideByte, numVer, pf);
						for (int i = 0; i<numVer; i++)
						{
							conMeshDesc->_Verts.push_back(buff[i]);
						}
						fread(&numInd, sizeof(int), 1, pf);
						int* ibuff = new int[numInd];
						fread(ibuff, sizeof(int), numInd, pf);
						for (int i = 0; i<numInd; i=i+3)
						{
							conMeshDesc->_Indeices.push_back(NxVec3(NxReal(ibuff[i]), NxReal(ibuff[i+1]), NxReal(ibuff[i+2])) );
						}
						ShapeDescCopy.push_back(conMeshDesc);
						delete[] buff;
						delete[] ibuff;
					}
					break;
				case GPL::EST_SHAPE_HEIGHTFIELD:
					{
						HeightFieldShapeDesc* hfDesc = new HeightFieldShapeDesc();
						hfDesc->_DatSrc = FullPath;
						hfDesc->_ShapeType = EST_SHAPE_HEIGHTFIELD;
						NxVec3 scale;
						int component_count;
						fread( &scale, sizeof NxVec3, 1, pf ) ;
						if ( feof (pf) )
							break;
						hfDesc->_Scale = scale;
						fread( &component_count, sizeof(component_count), 1, pf);
						for( int i = 0; i < component_count; i++ )
						{
							NxMat34 mat;
							float Entries[9];
							fread( Entries, sizeof(float), 9, pf );
							mat.M.setColumnMajor(Entries);
							fread( &mat.t.x, sizeof(float), 1, pf );
							fread( &mat.t.y, sizeof(float), 1, pf );
							fread( &mat.t.z, sizeof(float), 1, pf );
							hfDesc->_HeightFieldTMList.push_back(mat);
							NxHeightFieldDesc desc;
							//desc.flags = 0;
							desc.convexEdgeThreshold = 0;
							desc.thickness = 0;
							desc.verticalExtent = 0;
							desc.format = NX_HF_S16_TM;
							desc.sampleStride = sizeof(NxHeightFieldSample);
							hfDesc->_NxHeightFieldDescList.push_back(desc);

							fread(&hfDesc->_NxHeightFieldDescList[i].nbColumns, sizeof(int), 1, pf);
							fread(&hfDesc->_NxHeightFieldDescList[i].nbRows, sizeof(int), 1, pf);
							hfDesc->_NxHeightFieldDescList[i].samples = new NxHeightFieldSample[(hfDesc->_NxHeightFieldDescList[i].nbColumns)*(hfDesc->_NxHeightFieldDescList[i].nbRows)];

							NxI16* Heights = new NxI16[(hfDesc->_NxHeightFieldDescList[i].nbColumns)*(hfDesc->_NxHeightFieldDescList[i].nbRows)];
							fread( Heights, 2, (hfDesc->_NxHeightFieldDescList[i].nbColumns)*(hfDesc->_NxHeightFieldDescList[i].nbRows), pf );

							for ( NxU32 row=0; row < hfDesc->_NxHeightFieldDescList[i].nbRows; row++ )
								for ( NxU32 col=0; col < hfDesc->_NxHeightFieldDescList[i].nbColumns; col++ )
								{
									int x = col;
									int y = row;
									NxHeightFieldSample* currentSample = &((NxHeightFieldSample*)(hfDesc->_NxHeightFieldDescList[i].samples))[y*hfDesc->_NxHeightFieldDescList[i].nbColumns + x];

									currentSample->height = Heights[y*hfDesc->_NxHeightFieldDescList[i].nbColumns + x];//(((h&0xff)<<8) | (h&0xff00 >> 8)) -32768;
									currentSample->materialIndex0 = 0;
									currentSample->materialIndex1 = 0;
									currentSample->tessFlag = 0;
									currentSample->unused = 0;
								}
								delete[] Heights;
						}				
						ShapeDescCopy.push_back(hfDesc);
					}
					break;
				}
			}
			fclose(pf);
			return TRUE;
		}
		return FALSE;
	}
	//modify by yeleiyu@icee.cn for xml encrypt and decrypt
	bool LoadPhysModels( const std::wstring& filename, std::vector<GPL::PhysModelDesc> &models ,bool EnableReadEncryptedXML  )
	{
		std::vector<XML2STRUCT_NODE> x2s_components;
		std::vector<XML2STRUCT_NODE> x2s_component;
		std::vector<XML2STRUCT_NODE> x2s_shapes;
		std::vector<XML2STRUCT_NODE> x2s_shape;
		std::vector<XML2STRUCT_NODE> x2s_socket;
		std::vector<XML2STRUCT_NODE> x2s_angle;
		std::vector<XML2STRUCT_NODE> x2s_angles;
		std::vector<XML2STRUCT_NODE> x2s_pressure_area;
		std::vector<XML2STRUCT_NODE> x2s_sockets;
		PhysModelDesc t;
		Socket t_socket;
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_SocketType, t_int, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_PartType, t_int, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_AttachedId, t_int, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_LocalPose, t_nxmat34, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_YawCWLimit, t_float, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_YawCCWLimit, t_float, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_EnableLimit, t_BOOL, t_socket));
		x2s_socket.push_back(SIMPE_XML_ATTRIBUTE(_InitialRotation, t_float, t_socket));

		LimitAngle t_angle;
		x2s_angle.push_back(SIMPE_XML_ATTRIBUTE(_AngleMin, t_float, t_angle));
		x2s_angle.push_back(SIMPE_XML_ATTRIBUTE(_AngleMax, t_float, t_angle));

		x2s_angles.push_back(XML2STRUCT_NODE(L"AngleLimit", XML2STRUCT_NODE::t_sub_node_array, ADDR_OFFSET( &t_socket._FireLimitAngles,&t_socket),&x2s_angle,(void*)&LimitAngle_alloc_item));
		x2s_socket.push_back( XML2STRUCT_NODE( L"AngleLimits",	XML2STRUCT_NODE::t_sub_node, 0, &x2s_angles ) );

		x2s_pressure_area.push_back(XML2STRUCT_NODE(L"AngleLimit", XML2STRUCT_NODE::t_sub_node_array, ADDR_OFFSET( &t_socket._FirePressureLimitAngles,&t_socket),&x2s_angle,(void*)&LimitAngle_alloc_item));
		x2s_socket.push_back( XML2STRUCT_NODE( L"PressureArea",	XML2STRUCT_NODE::t_sub_node, 0, &x2s_pressure_area ) );
		
		x2s_sockets.push_back( XML2STRUCT_NODE( L"Socket",	XML2STRUCT_NODE::t_sub_node_array,ADDR_OFFSET( &t._SocketDescList,&t), (unsigned char*)&x2s_socket, (void*)socket_alloc_item ) );
		x2s_component.push_back( XML2STRUCT_NODE( L"Sockets",	XML2STRUCT_NODE::t_sub_node, 0, &x2s_sockets ) );

		x2s_shape.push_back( XML2STRUCT_NODE( L"Shape",	XML2STRUCT_NODE::t_custom_node, 0, shape_custom_load ) );
		x2s_shapes.push_back( XML2STRUCT_NODE( L"Shape",	XML2STRUCT_NODE::t_sub_node_array,ADDR_OFFSET( &t._ShapeDescList,&t), (unsigned char*)&x2s_shape, (void*)shape_alloc_item ) );

		x2s_component.push_back( XML2STRUCT_NODE( L"Shapes",	XML2STRUCT_NODE::t_sub_node, 0, &x2s_shapes ) );
		x2s_component.push_back( SIMPE_XML_ATTRIBUTE( _PhysId, t_stdstring, t ));
		
		x2s_components.push_back( XML2STRUCT_NODE( L"Component",	XML2STRUCT_NODE::t_sub_node_array, 0, (unsigned char*)&x2s_component, (void*)model_alloc_item ) );

		MSXML2::IXMLDOMDocumentPtr pXMLDoc;
		if ( InitXML(pXMLDoc) == FALSE )
			return FALSE;

		pXMLDoc->async = VARIANT_FALSE;
		pXMLDoc->validateOnParse = VARIANT_FALSE;
		pXMLDoc->resolveExternals = VARIANT_FALSE;

		std::wstring FullFilepath = GWorkingPath + filename;
		//modify by yeleiyu@icee.cn for xml encrypt and decrypt
		if(!LoadXML(FullFilepath.c_str(),pXMLDoc,EnableReadEncryptedXML))
		{
			gplDebugf(TEXT("[GPL] failed to Load PhysModels file, filename[%s]  may be not unicode"),filename);
			return FALSE;
		}
		/*if(pXMLDoc->load(FullFilepath.c_str()) != VARIANT_TRUE)
		{
			return FALSE;
		}*/
		MSXML2::IXMLDOMElementPtr root;

		if( !GetChildElemNode( pXMLDoc, L"Components", root) )
			return FALSE;

		read_x2s_list( x2s_components, root, (unsigned char*)&models );

		// _PhysId统一处理为小写，避免大小敏感
		std::vector<GPL::PhysModelDesc>::iterator it = models.begin();
		for(; it != models.end(); ++it)
		{			
			ToLowerString((*it)._PhysId);
		}

		//加载pml文件  /////////////////////////////////////
		for (std::vector<GPL::PhysModelDesc>::iterator it = models.begin();
			it != models.end(); ++it)
		{
			std::vector<ShapeDesc*> ShapeDescCopy;
			for (std::vector<ShapeDesc*>::iterator shapeDesc_it = it->_ShapeDescList.begin();
					shapeDesc_it != it->_ShapeDescList.end(); ++shapeDesc_it)
			{
				ShapeDesc* shapeDesc = *shapeDesc_it;
				if(shapeDesc->_DatSrc != TEXT(""))
				{
					//int shapeType = shapeDesc->_ShapeType;		//pml中有type标记
					//NxMat34 localPose = shapeDesc->_LocalPose; //从pml中读出
					std::wstring datSrc = shapeDesc->_DatSrc;
					delete shapeDesc; ////这个shapeDesc已经没用了，要从pml中可能读出N个shape
					ReadPMLShapes(ShapeDescCopy, datSrc);
				}
				else
				{
					ShapeDescCopy.push_back(shapeDesc);
				}
			}
			
			it->_ShapeDescList = ShapeDescCopy; 
		}
		////////////////////////////////////////////////////

		root.Release();
		ReleaseXML(pXMLDoc);

		return TRUE;
	}

	void* CALLBACK component_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		int type;
		if ( GetAttribute( elem, L"_ConfigType", type ) == FALSE )
			return NULL;
		std::vector<GPL::ComponentDesc*>* p = (std::vector<GPL::ComponentDesc*>*)arr;

		GPL::ComponentDesc* comp = NULL;
		switch( type )
		{
		case ECT_Default:			comp = new ComponentDesc();				break;
		case ECT_Vehicle:			comp = new VehicleCompDesc();			break;
		case ECT_Attached_Comp:		comp = new VehicleAttachedCompDesc();	break;
		case ECT_Attached_Weapon:	comp = new WeaponCompDesc();			break;
		case ECT_Projectile:		comp = new ProjectileCompDesc();		break;
		case ECT_Aircraft:			comp = new AircraftCompDesc();			break;
		case ECT_Static:			comp = new StaticCompDesc();			break;
		case ECT_PlayerStart:		comp = new ScenePlayerStartCompDesc();	break;
		case ECT_PlayerRespawn:		comp = new ScenePlayerRespawnCompDesc();break;
		case ECT_Ocean:				comp = new SceneOceanCompDesc();		break;
		case ECT_Terrain:			comp = new SceneTerrainCompDesc();		break;
		case ECT_BlockingBrush:		comp = new SceneBlockingCompDesc();		break;
		case ECT_TriggerBrush:		comp = new SceneTriggerCompDesc();		break;
		}

		if ( NULL != comp )
			p->push_back(comp);
		else
			gplDebugf(TEXT("[GPL] component_alloc_item: can not alloc item for _ConfigType[%d]"), type);

		return comp;
	}
	BOOL CALLBACK component_custom_load( MSXML2::IXMLDOMElementPtr elem, unsigned char* data )
	{
		std::vector<XML2STRUCT_NODE> x2s_component;
		std::vector<XML2STRUCT_NODE> x2s_vehicle_attachment;
		std::vector<XML2STRUCT_NODE> x2s_vehicle_component;
		std::vector<XML2STRUCT_NODE> x2s_weapon_component;
		std::vector<XML2STRUCT_NODE> x2s_projectile_component;
		std::vector<XML2STRUCT_NODE> x2s_aircraft_component;
		std::vector<XML2STRUCT_NODE> x2s_static_component;
		std::vector<XML2STRUCT_NODE> x2s_playerStart_component;
		std::vector<XML2STRUCT_NODE> x2s_blockingVolume_component;
		std::vector<XML2STRUCT_NODE> x2s_triggerVolume_component;
		//std::vector<XML2STRUCT_NODE> x2s_oceanVolume_component;
		//std::vector<XML2STRUCT_NODE> x2s_terrainVolume_component;

		{
			ComponentDesc t;
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE( _CompId, t_int, t ));
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE_DEFAULT( _PhysId, t_stdstring, t ));
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE( _ConfigType, t_int, t ));
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE( _ObjectType, t_int, t ));
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE_DEFAULT( _LoadingFlag, t_int, t ));
			x2s_component.push_back( SIMPE_XML_ATTRIBUTE_DEFAULT( _PhyMaterialType, t_int, t ));
		}

		{
			VehicleCompDesc t;
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _SimType, t_int, t ));

			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _StopThreshold, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _LongDampingForce, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _LatDampingForce, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _ScaleDampingSpeed, t_float, t ));

			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _MaxTurnRate, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _StopAngThreshold, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _TurnDampingTorque, t_float, t ));
			x2s_vehicle_component.push_back( SIMPE_XML_ATTRIBUTE( _ScaleDampingTurn, t_float, t ));
		}

		// WeaponCompDesc
		{
			WeaponCompDesc t;
			
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _GunTubeLimitAngleLower, t_float, t ));
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _GunTubeLimitAngleUpper, t_float, t ));
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _GunTubeAngleVelocity, t_float, t ));
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _TrajectoryMethod, t_int, t ));
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _AccuracyTurrentMax, t_float, t ));
			x2s_weapon_component.push_back( SIMPE_XML_ATTRIBUTE( _AccuracyTotalMax, t_float, t ));
		}

		// projectile compdesc
		{
			ProjectileCompDesc t;
			x2s_projectile_component.push_back( SIMPE_XML_ATTRIBUTE(_MaxFlightTime, t_float, t));
			x2s_projectile_component.push_back( SIMPE_XML_ATTRIBUTE(_HitRange, t_float, t));
			x2s_projectile_component.push_back( SIMPE_XML_ATTRIBUTE_DEFAULT(_MagneticSensorInterval, t_int, t));
		}

		// aircraft compdesc
		{
			AircraftCompDesc t;
			x2s_aircraft_component.push_back( SIMPE_XML_ATTRIBUTE( _TrajectoryMethod, t_int, t ));
			x2s_aircraft_component.push_back( SIMPE_XML_ATTRIBUTE( _FormationAngle, t_float, t ));
			x2s_aircraft_component.push_back( SIMPE_XML_ATTRIBUTE( _FormationSpace, t_float, t ));
			x2s_aircraft_component.push_back( SIMPE_XML_ATTRIBUTE( _WarningDistance, t_float, t ));
		}

		// playerstart compdesc
		{
			ScenePlayerStartCompDesc t;
			x2s_playerStart_component.push_back( SIMPE_XML_ATTRIBUTE(_TeamIndex, t_int, t));
			x2s_playerStart_component.push_back( SIMPE_XML_ATTRIBUTE(_SeatIndex, t_int, t));
			x2s_playerStart_component.push_back( XML2STRUCT_NODE( L"_PriorVehicleType",	XML2STRUCT_NODE::t_int_array, ADDR_OFFSET( &t._PriorVehicleType, &t)));
		}

		// SceneBlockingCompDesc
		{
			SceneBlockingCompDesc t;
			x2s_blockingVolume_component.push_back( SIMPE_XML_ATTRIBUTE(_bBlockCamera, t_BOOL, t));
		}

		// scene static compdesc
		{
			StaticCompDesc t;
			x2s_static_component.push_back( SIMPE_XML_ATTRIBUTE( _InitGlobalPose, t_nxmat34, t));
			x2s_static_component.push_back( SIMPE_XML_ATTRIBUTE( _ModelScale, t_nxvec3, t));
			x2s_static_component.push_back( SIMPE_XML_ATTRIBUTE_DEFAULT( _MapName, t_stdstring, t));
			x2s_static_component.push_back( SIMPE_XML_ATTRIBUTE( _StaticID, t_int, t));
		}

		// SceneTriggerCompDesc
		{
			SceneTriggerCompDesc t;
			x2s_triggerVolume_component.push_back(SIMPE_XML_ATTRIBUTE(_PhyAreaType, t_int, t));
			x2s_triggerVolume_component.push_back(SIMPE_XML_ATTRIBUTE(_AreaId, t_int, t));
		}

		// SceneOceanCompDesc
		{
		}

		// SceneTerrainCompDesc
		{
		}

		read_x2s_list( x2s_component, elem, data );

		switch( ((ComponentDesc*)(data))->_ConfigType )
		{
		case ECT_Attached_Comp:
			break;
		case ECT_Attached_Weapon:
			read_x2s_list( x2s_weapon_component, elem, data );
			break;
		case ECT_Projectile:
			read_x2s_list( x2s_projectile_component, elem, data );
			break;
		case ECT_Aircraft:
			read_x2s_list( x2s_aircraft_component, elem, data );
			break;
		case ECT_Vehicle:
			read_x2s_list( x2s_vehicle_component, elem, data );
			break;
		case ECT_PlayerStart:
		case ECT_PlayerRespawn:		// 与ECT_PlayerStart数据结构一致
			read_x2s_list(x2s_static_component, elem, data);
			read_x2s_list(x2s_playerStart_component, elem, data);
			break;
		case ECT_Static:
			read_x2s_list(x2s_static_component, elem, data);
			break;
		case ECT_BlockingBrush:
			read_x2s_list(x2s_static_component, elem, data);
			read_x2s_list(x2s_blockingVolume_component, elem, data);
			break;
		case ECT_TriggerBrush:
			read_x2s_list(x2s_static_component, elem, data);
			read_x2s_list(x2s_triggerVolume_component, elem, data);			
			break;
		case ECT_Ocean:
			read_x2s_list(x2s_static_component, elem, data);
			//read_x2s_list(x2s_oceanVolume_component, elem, data);			
			break;
		case ECT_Terrain:
			read_x2s_list(x2s_static_component, elem, data);
			//read_x2s_list(x2s_terrainVolume_component, elem, data);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	//modify by yeleiyu@icee.cn for xml encrypt and decrypt
	bool LoadComponents( const std::wstring& filename, ComponentDescList& components ,bool EnableReadEncryptedXML )
	{
		std::vector<XML2STRUCT_NODE> x2s_components;
		std::vector<XML2STRUCT_NODE> x2s_component;

		x2s_component.push_back( XML2STRUCT_NODE( L"Component",	XML2STRUCT_NODE::t_custom_node, 0, (void*)component_custom_load ) );
		x2s_components.push_back( XML2STRUCT_NODE( L"Component",	XML2STRUCT_NODE::t_sub_node_array, 0, (unsigned char*)&x2s_component, (void*)component_alloc_item ) );

		MSXML2::IXMLDOMDocumentPtr pXMLDoc;
		if ( InitXML(pXMLDoc) == FALSE )
		{
			gplDebugf(TEXT("[GPL] failed to LoadComponents, filename[%s]"), filename.c_str());
			return FALSE;
		}

		pXMLDoc->async = VARIANT_FALSE;
		pXMLDoc->validateOnParse = VARIANT_FALSE;
		pXMLDoc->resolveExternals = VARIANT_FALSE;

		std::wstring FullFilepath = GWorkingPath + filename;
		//modify by yeleiyu@icee.cn for xml encrypt and decrypt
		if(!LoadXML(FullFilepath.c_str(),pXMLDoc,EnableReadEncryptedXML))
		{
			gplDebugf(TEXT("[GPL] failed to LoadComponents, filename[%s]  may be not unicode"), filename.c_str());
			return FALSE;
		}
		/*if(pXMLDoc->load(FullFilepath.c_str()) != VARIANT_TRUE)
		{
		gplDebugf(TEXT("[GPL] failed to LoadComponents, filename[%s]"), filename.c_str());
		return FALSE;
		}*/
		MSXML2::IXMLDOMElementPtr root;

		if( !GetChildElemNode( pXMLDoc, L"Components", root) )
		{
			gplDebugf(TEXT("[GPL] failed to LoadComponents, filename[%s]"), filename.c_str());
			return FALSE;
		}

		read_x2s_list( x2s_components, root, (unsigned char*)&components );

		// _PhysId统一处理为小写，避免大小敏感
		std::vector<GPL::ComponentDesc*>::iterator it = components.begin();
		for(; it != components.end(); ++it)
		{			
			ToLowerString((*it)->_PhysId);
		}

		root.Release();
		ReleaseXML(pXMLDoc);

		return TRUE;
	}

	void IndexComponents(const ComponentDescList& components, ComponentDescMap& indexed_component)
	{
		indexed_component.clear();

		ComponentDescList::const_iterator it = components.begin();
		while (it != components.end())
		{
			ComponentDesc* desc = *it;
			if (!desc)
			{
				gplDebugf(TEXT("Invalid ComponentDesc Found!"));
				continue;
			}

#ifdef _DEBUG
			if (indexed_component.find(desc->_CompId) != indexed_component.end())
				gplDebugf(TEXT("Repeat ComponentID Found! _CompId=%d"), desc->_CompId);
#endif // _DEBUG

			indexed_component[desc->_CompId] = desc;
			it++;
		}
	}

	void* CALLBACK fragment_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		std::vector<DampingFragment>* Fragments = (std::vector<DampingFragment>*)arr;
		DampingFragment t;
		Fragments->push_back(t);
		return &(Fragments->back());
	}

	void* CALLBACK reducelevel_alloc_item( MSXML2::IXMLDOMElementPtr elem, void* arr )
	{
		std::vector<ReduceLevel>* Levels = (std::vector<ReduceLevel>*)arr;
		ReduceLevel t;
		Levels->push_back(t);
		return &(Levels->back());
	}

	//modify by yeleiyu@icee.cn for xml encrypt and decrypt
	bool LoadPhysDataConfig( const std::wstring& filename,PhysGameSetting &config ,bool EnableReadEncryptedXML )
	{
		std::vector<XML2STRUCT_NODE> x2s_Data;
		std::vector<XML2STRUCT_NODE> x2s_Fragment;
		std::vector<XML2STRUCT_NODE> x2s_DampingFragments;
		std::vector<XML2STRUCT_NODE> x2s_ReduceLevel;
		std::vector<XML2STRUCT_NODE> x2s_AccuracyDrivingReduce;

		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_FirePointLastTime, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_RecordAimingOccasion, t_BOOL, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_AccuracyFormulaA, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_AccuracyFormulaB, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_AccuracyFormulaC, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_AccuracyFormulaD, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_AccuracyLimitMin, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PhysWorld_HalfLength, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PhysWorld_HalfWidth, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PhysWorld_HalfHeight, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_MAX_HALF_WORLD, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_SKIN_WIDTH, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PHYS_GRAVITY, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PHYS_CLIENT_MAX_TIMESTEP, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PROJECTILE_TICK_COUNT, t_int, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PROJECTILE_SIMULATED_TIME_MAX, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_FIRE_AVAILABLE_TOLERANCE, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_WaterLayerOneHeight, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_WaterLayerTwoHeight, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_SubmarineAccelerate, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_SubmarineConstant, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_FlightLayer_CruiseHeight, t_int, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_FlightLayer_DiveLimit, t_int, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_FlightLayer_TorpedoRelease, t_int, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_ReverseForce_DampingFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_ForwardForce_DampingFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_ReverseSpeed_Rate, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_ReverseForce_Rate, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_TorqueDampingFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_FactorOfAngularVelocity, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_TakeOff_Angle, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_Landing_Angle, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_PickTrace_Range, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_Process_Blocks, t_BOOL, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_Block_Detect_Interval, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_ActionCache_Length, t_int, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_Aircraft_Interpose_Speed, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_PhysAircraft_PositionInterp_Speed, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_PhysAircraft_RotationInterp_Speed, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_Roll_Limit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_Pitch_Limit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_Speed_Limit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_ForceFactor_LowerLimit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_ForceFactor_UpperLimit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_TorqueFactor_LowerLimit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_TorqueFactor_UpperLimit, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_DampingFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Sink_Torque_DampingFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_TorpedoSalvo_DeltaTangentVelocity, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_TorpedoSalvo_DeltaAngle, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Bomber_ReCollimate_PrecisionFactorA, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Bomber_ReCollimate_PrecisionFactorB, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_TorpedoPlane_LaunchTorlerance, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_TorpedoPlane_LaunchAngleTorlerance, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_Routing_Torlerance, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_Routing_AngleTorlerance, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_DiveAttack_SpeedFactor, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_DiveAttack_LimitSpeed, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_DiveAttack_MinInterval, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_Aircraft_DiveAttack_MaxInterval, t_float, config));
		x2s_Data.push_back(SIMPE_XML_ATTRIBUTE(_GPL_MagneticTorpedo_FriendlyDetective, t_BOOL, config));
		x2s_Data.push_back( XML2STRUCT_NODE( TEXT("SupportContinuousFireList"), XML2STRUCT_NODE::t_int_array, ADDR_OFFSET( &config._PartTypeListOfSupportContinuousFire, &config)));
		
		DampingFragment Fragment;
		x2s_Fragment.push_back(SIMPE_XML_ATTRIBUTE(Speed, t_float, Fragment));
		x2s_Fragment.push_back(SIMPE_XML_ATTRIBUTE(Damping, t_float, Fragment));
		x2s_DampingFragments.push_back(XML2STRUCT_NODE(L"Fragment", XML2STRUCT_NODE::t_sub_node_array, 0,&x2s_Fragment,(void*)&fragment_alloc_item));
		x2s_Data.push_back( XML2STRUCT_NODE( L"DampingFragment",XML2STRUCT_NODE::t_sub_node, ADDR_OFFSET( &config._DampingFragmentList,&config), (unsigned char*)&x2s_DampingFragments ) );

		ReduceLevel Level;
		x2s_ReduceLevel.push_back(SIMPE_XML_ATTRIBUTE(SpeedRate, t_float, Level));
		x2s_ReduceLevel.push_back(SIMPE_XML_ATTRIBUTE(ReduceFactor, t_float, Level));
		x2s_AccuracyDrivingReduce.push_back(XML2STRUCT_NODE(L"ReduceLevel", XML2STRUCT_NODE::t_sub_node_array, 0,&x2s_ReduceLevel,(void*)&reducelevel_alloc_item));
		x2s_Data.push_back( XML2STRUCT_NODE( L"AccuracyDrivingReduce",XML2STRUCT_NODE::t_sub_node, ADDR_OFFSET( &config._AccuracyDrivingReduce,&config), (unsigned char*)&x2s_AccuracyDrivingReduce ) );

		MSXML2::IXMLDOMDocumentPtr pXMLDoc;
		if ( InitXML(pXMLDoc) == FALSE )
		{
			gplDebugf(TEXT("[GPL] failed to PhysConfigData, filename[%s]"),filename);
			return FALSE;
		}
		pXMLDoc->async = VARIANT_FALSE;
		pXMLDoc->validateOnParse = VARIANT_FALSE;
		pXMLDoc->resolveExternals = VARIANT_FALSE;

		std::wstring FullFilepath = GWorkingPath + filename;
		//modify by yeleiyu@icee.cn for xml encrypt and decrypt
		if(!LoadXML(FullFilepath.c_str(),pXMLDoc,EnableReadEncryptedXML))
		{
			gplDebugf(TEXT("[GPL] failed to PhysConfigData, filename[%s]  may be not unicode"),filename);
			return FALSE;
		}
		/*if(pXMLDoc->load(FullFilepath.c_str()) != VARIANT_TRUE)
		{
		gplDebugf(TEXT("[GPL] failed to PhysConfigData, filename[%s]"),filename);
		return FALSE;
		}*/
		MSXML2::IXMLDOMElementPtr root;

		if( !GetChildElemNode( pXMLDoc, L"PhysDataConfig", root) )
		{
			gplDebugf(TEXT("[GPL] failed to PhysConfigData, filename[%s]"),filename);
			return FALSE;
		}

		read_x2s_list( x2s_Data, root, (unsigned char*)&config );
		
		root.Release();
		ReleaseXML(pXMLDoc);

		return TRUE;
	}

	
}