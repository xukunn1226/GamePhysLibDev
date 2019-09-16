//=============================================================================
// PhysLoader: 定义各种数据，模型描述、逻辑配置、实例化数据结构
// 功能接口：
//	// 读取模型配置文件
//	bool LoadPhysModels( const std::wstring& filename, std::vector<GPL::PhysModelDesc> &models );
//	// 根据PhysId加载模型数据
//	bool LoadPhysModelDesc(std::wstring PhysId, PhysModelDesc* &ModelDesc);
//  // 加载所有组件逻辑配置数据
//	bool LoadComponents( const std::wstring& filename, std::vector<GPL::ComponentDesc*> &components );
//	// 根据CompId加载逻辑数据
//	bool LoadComponentDesc(int CompId, ComponentDesc* &CompDesc);
//	// 生成所有组件实例化数据
//	ComponentData* ConstructComponentData(ComponentDesc* CompDesc);
//=============================================================================
#ifndef _PHYS_LOADER_H_
#define _PHYS_LOADER_H_

namespace GPL
{
	

class MemoryWriteBuffer : public NxStream
{
	public:
								MemoryWriteBuffer();
	virtual						~MemoryWriteBuffer();
				void			clear();

	virtual		NxU8			readByte()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		NxU16			readWord()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		NxU32			readDword()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		float			readFloat()								const	{ NX_ASSERT(0);	return 0.0f;}
	virtual		double			readDouble()							const	{ NX_ASSERT(0);	return 0.0;	}
	virtual		void			readBuffer(void* buffer, NxU32 size)	const	{ NX_ASSERT(0);				}

	virtual		NxStream&		storeByte(NxU8 b);
	virtual		NxStream&		storeWord(NxU16 w);
	virtual		NxStream&		storeDword(NxU32 d);
	virtual		NxStream&		storeFloat(NxReal f);
	virtual		NxStream&		storeDouble(NxF64 f);
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size);

				NxU32			currentSize;
				NxU32			maxSize;
				NxU8*			data;
	};

class MemoryReadBuffer : public NxStream
	{
	public:
								MemoryReadBuffer(const NxU8* data);
	virtual						~MemoryReadBuffer();

	virtual		NxU8			readByte()								const;
	virtual		NxU16			readWord()								const;
	virtual		NxU32			readDword()								const;
	virtual		float			readFloat()								const;
	virtual		double			readDouble()							const;
	virtual		void			readBuffer(void* buffer, NxU32 size)	const;

	virtual		NxStream&		storeByte(NxU8 b)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeWord(NxU16 w)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeDword(NxU32 d)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeFloat(NxReal f)						{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeDouble(NxF64 f)						{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)	{ NX_ASSERT(0);	return *this;	}

	mutable		const NxU8*		buffer;
	};



struct LimitAngle
{
	float _AngleMin;
	float _AngleMax;
};

// 用于定位舰船上的位置信息
struct Socket
{
	Socket()
	:_EnableLimit(FALSE), _SocketType(EST_Socket_Part), _AttachedId(0), _PartType(EPT_Invalid)
	{
	}
	ESocketType	_SocketType;			// 描述此SOCKET的用途
	EPartType	_PartType;				// 部件类型，逻辑数据
	int			_AttachedId;			// 全局唯一的ID，标识部件，逻辑数据
	NxMat34		_LocalPose;				// 相对舰船的局部POSE，仅位置信息有效
	float		_YawCWLimit;			// in degree,相对初始朝向（_InitRotation）的顺时针角度限制
	float		_YawCCWLimit;			// in degree，相对初始朝向（_InitRotation）的逆时针角度限制
	bool		_EnableLimit;			// 是否转向到目标才能射击，true：转向目标才能射击；false，反之
	float		_InitialRotation;		// in degree, 初始朝向
	std::vector<LimitAngle> _FireLimitAngles;  // in degree,相对原始位置可以射击的夹角
	std::vector<LimitAngle> _FirePressureLimitAngles;  // in degree,相对原始位置可以形成水面风压的夹角
};

/*
 * @brief	模型数据描述对象
 */
class ShapeDesc
{
public:
	ShapeDesc()
	{
		_bTriggerShape = false;
	}

	virtual ~ShapeDesc() {}

	virtual void Cleanup() {}

	EShapeType		_ShapeType;
	NxMat34			_LocalPose;			// 相对Actor frame矩阵
	std::wstring	_DatSrc;			// 是否是pml文件保存数据
	bool			_bTriggerShape;
};

class SphereShapeDesc : public ShapeDesc
{
public:
	float	_Radius;
};

class BoxShapeDesc : public ShapeDesc
{
public:
	NxVec3	_Dimensions;
};

class PlaneShapeDesc : public ShapeDesc
{
public:
	NxVec3	_Normal;	// unit length
	NxReal	_D;			// the distance from the origin
};

class CapsuleShapeDesc : public ShapeDesc
{
public:
	NxReal	_Radius;
	NxReal	_Height;
};

class ConvexShapeDesc : public ShapeDesc
{
public:
	virtual void Cleanup()
	{
	}

	bool IsValidCachedData()
	{
		return _CachedMeshData.data != NULL ? true : false;
	}

	std::vector<NxVec3>	_Verts;
	std::vector<NxVec3>	_Indeices;
	
	MemoryWriteBuffer	_CachedMeshData;
};

class HeightFieldShapeDesc : public ShapeDesc
{
public:
	NxVec3 _Scale;
	std::vector<NxHeightFieldDesc> _NxHeightFieldDescList;
	std::vector<NxMat34> _HeightFieldTMList;

	virtual void Cleanup()
	{
		for (std::vector<NxHeightFieldDesc>::iterator it = _NxHeightFieldDescList.begin(); 
				it != _NxHeightFieldDescList.end(); ++it)
		{
			delete[] it->samples;
		}
		_NxHeightFieldDescList.clear();
	}
};

/** 
 * 静态数据，物理模型数据
 */
class PhysModelDesc
{
public:
	PhysModelDesc()
	{
		_PhysId = TEXT("");
	}

	void Cleanup()
	{
		std::vector<ShapeDesc*>::iterator it = _ShapeDescList.begin();
		for(; it != _ShapeDescList.end(); ++it)
		{
			(*it)->Cleanup();
			delete *it;
		}
		_ShapeDescList.clear();
	}

	bool IsValid() { return true; }
	
	// 返回的Socket位置信息已转换至Phys scale制式下
	bool FindSocket(int AttachedId, Socket& Sk)
	{
		for(NxU32 i = 0; i < _SocketDescList.size(); ++i)
		{
			if( _SocketDescList[i]._AttachedId == AttachedId )
			{
				Sk = _SocketDescList[i];
				Sk._LocalPose.t *= G2PScale;
				return true;
			}
		}
		return false;
	}

	bool FindSocket(ESocketType SocketType, std::vector<Socket>& SocketsList)
	{
		SocketsList.clear();
		for(NxU32 i = 0; i < _SocketDescList.size(); ++i)
		{
			if( _SocketDescList[i]._SocketType == SocketType )
			{
				Socket Temp = _SocketDescList[i];
				Temp._LocalPose.t *= G2PScale;
				SocketsList.push_back(Temp);
			}
		}
		return SocketsList.size() > 0;
	}

	void GetSocketListByPartType(const int PartType, std::vector<int>& AttachedIdList) const
	{
		std::vector<Socket> Sockets = _SocketDescList;
		for ( std::vector<Socket>::iterator it = Sockets.begin(); it != Sockets.end(); ++it )
		{
			if( it->_PartType == PartType )
			{
				AttachedIdList.push_back((*it)._AttachedId);
			}
		}
	}

	std::wstring				_PhysId;					// 模型数据配置ID
	std::vector<ShapeDesc*>		_ShapeDescList;				// collection of all shapes
	std::vector<Socket>			_SocketDescList;			// collection of all sockets
};
extern std::vector<PhysModelDesc>		GPhysModelDescList;		// 所有动态物体的物理模型数据（武器，舰船，舰桥），不包含与地图相关配置
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
/** 读取所有模型数据 */
bool LoadPhysModels( const std::wstring& filename, std::vector<GPL::PhysModelDesc> &models ,bool EnableReadEncryptedXML =FALSE );
/** 根据PhysId加载模型数据 */
bool LoadPhysModelDesc(std::wstring PhysId, PhysModelDesc* &ModelDesc);


/**
 * 静态数据，组件逻辑数据
 */
class ComponentDesc
{
	DECLARE_RTTI
public:
	ComponentDesc()
	{ 
		_CompId = -1;
		_PhysId = TEXT("");
		_ConfigType = EConfigType(-1);
		_ObjectType = EGameObjectType(-1);
		_LoadingFlag = ELF_None;
		_InitGlobalPose.id();
		_PhyMaterialType=-1;
	}

	virtual ~ComponentDesc() {}

	//以下接口不推荐使用，将逐渐废弃，目前保留仅作兼容只用，请使用RTTI 
	virtual bool IsVehicleAttachedComp() { return false; }
	virtual bool IsWeaponComp() { return false; }
	virtual bool IsVehicleComp() { return false; }
	virtual bool IsProjectileComp() { return false; }
	virtual bool IsStaticComp() {return false;}

public:
	int					_CompId;			// 组件配置ID
	std::wstring		_PhysId;			// 引用的模型数据ID
	EConfigType			_ConfigType;		// 组件配置类型，配置导入时决定实例化对象
	EGameObjectType		_ObjectType;		// 对象类型，根据此类型实例化不同对象
	ELoadingFlag		_LoadingFlag;		// 加载标记
	NxMat34				_InitGlobalPose;	// 初始化位置
	int					_PhyMaterialType;

	virtual bool IsValid()
	{
		return true;
	}
};

// 可被载具装备的组件逻辑数据，例如武器、舰桥、发动机等
class VehicleAttachedCompDesc : public ComponentDesc
{
	DECLARE_RTTI
public:
	VehicleAttachedCompDesc()
		: ComponentDesc()
	{
		_ConfigType = ECT_Attached_Comp;
		_ObjectType = EGOT_Attached_Comp;
	}

	virtual bool IsValid()
	{
		if( _CompId > -1 
			&& _ConfigType == ECT_Attached_Comp
			&& _ObjectType == EGOT_Attached_Comp )
		{
			return true;
		}
		gplDebugf(TEXT("[GPL] ComponentDesc::IsValid() return FALSE!!! CompId[%d]"), _CompId);
		return false;
	}

	virtual bool IsVehicleAttachedComp() { return true; }
};

class WeaponCompDesc : public VehicleAttachedCompDesc
{
	DECLARE_RTTI
public:
	WeaponCompDesc()
		: VehicleAttachedCompDesc()
	{
		_ConfigType = ECT_Attached_Weapon;
		_ObjectType = EGOT_Attached_Weapon;
				
		_GunTubeLimitAngleLower = -10.f;
		_GunTubeLimitAngleUpper = 60.f;
		_GunTubeAngleVelocity = 10.f;
		_AccuracyTurrentMax=2000.0f;											
		_AccuracyTotalMax=3000.0f;	
	}

	ETrajectoryMethod	_TrajectoryMethod;				// 支持的炮弹飞行轨迹
	float				_GunTubeLimitAngleLower;		// in degree, 炮管垂直转向(pitch)角度下限
	float				_GunTubeLimitAngleUpper;		// 炮管垂直转向(pitch)角度上限
	float				_GunTubeAngleVelocity;			// in degree, 炮管转动角速度
	float				_AccuracyTurrentMax;			//炮台转向对射击精确度的影响最大值
	float				_AccuracyTotalMax;				//总精确度的影响最大值

	virtual bool IsValid()
	{
		if( _CompId > -1 
			&& _ConfigType == ECT_Attached_Weapon
			&& _ObjectType == EGOT_Attached_Weapon )
		{
			return true;
		}
		gplDebugf(TEXT("[GPL] ComponentDesc::IsValid() return FALSE!!! CompId[%d]"), _CompId);
		return false;
	}

	virtual bool IsWeaponComp() { return true; }
};

class ProjectileCompDesc : public ComponentDesc
{
	DECLARE_RTTI
public:
	ProjectileCompDesc()
		: ComponentDesc()
	{		
		_ConfigType = ECT_Projectile;
		_ObjectType = EGOT_Projectile;
	}

	virtual bool IsValid()
	{
		if( _CompId > -1
			&& _ConfigType == ECT_Projectile )
		{
			return true;
		}
		gplDebugf(TEXT("[GPL] ComponentDesc::IsValid() return FALSE!!! CompId[%d]"), _CompId);
		return false;
	}

	virtual bool IsProjectileComp() { return true; }

	float			_MaxFlightTime;			//	最长飞行时间，超过则自动消失	
	float			_HitRange;				//	引爆检测范围，=0表示只能与其他物体发生碰撞才爆炸；>0表示范围引爆
	int				_MagneticSensorInterval;//	磁感引信检测时间间隔

};

//------------------------------------------------------------------------
//	@brief	舰载机配置数据
//------------------------------------------------------------------------
class AircraftCompDesc : public ComponentDesc
{
	DECLARE_RTTI
public:
	AircraftCompDesc()
		: ComponentDesc()	
	{		
		_ConfigType = ECT_Aircraft;
		_ObjectType = EGOT_Vehicle_Aircraft;
	}

	virtual bool IsValid()
	{
		if( _CompId > -1 && _ConfigType == ECT_Aircraft )
			return true;

		gplDebugf(TEXT("[GPL] ComponentDesc::IsValid() return FALSE!!! CompId[%d]"), _CompId);
		return false;
	}
	
	ETrajectoryMethod	_TrajectoryMethod;		//	炮弹弹道类型
	float				_FormationAngle;		//	队形夹角
	float				_FormationSpace;		//	队形间距
	float				_WarningDistance;		//	警戒距离（障碍检测）
};

class VehicleCompDesc : public ComponentDesc
{
	DECLARE_RTTI
public:
	VehicleCompDesc()
		: ComponentDesc()
	{
		_ConfigType = ECT_Vehicle;
		_ObjectType = EGOT_Vehicle_Ship;
		_SimType = EST_Normal;
		_LatDampingForce = 0.f;
		_MaxDriveGear = 4;
		_MaxReverseGear = 2;
		_MaxTurnGear = 4;
	}

	virtual bool IsValid()
	{
		if( _CompId > -1 
			&& _PhysId != TEXT("")
			&& _ConfigType == ECT_Vehicle )
		{
			return true;
		}
		gplDebugf(TEXT("[GPL] ComponentDesc::IsValid() return FALSE!!! CompId[%d]"), _CompId);
		return false;
	}
		
	virtual bool IsVehicleComp() { return true; }


	ESimulationType		_SimType;					// 移动控制类型	
	
	float				_StopThreshold;				// 停止移动阀值	
	float				_LongDampingForce;			// 纵向阻尼大小
	float				_LatDampingForce;			// 横向阻尼大小，防止漂移
	float				_ScaleDampingSpeed;			// 计算纵向、横向阻尼时的Scale

	float				_MaxTurnRate;				// 最大角速度, degree/sec
	float				_StopAngThreshold;			// in degree, 停止旋转阀值	
	float				_TurnDampingTorque;			// 没有扭矩时的转向阻尼大小
	float				_ScaleDampingTurn;			// 计算转向阻尼时的Scale

	int					_MaxDriveGear;				// 最大前进档
	int					_MaxReverseGear;			// 最大后退档
	int					_MaxTurnGear;				// 最大旋转档

};

class StaticCompDesc : public ComponentDesc
{
	DECLARE_RTTI
public:
	StaticCompDesc()
		: ComponentDesc()
	{}

	virtual bool IsStaticComp() {return true;}
	virtual bool IsValid() {return true;}
	
	NxVec3			_ModelScale;				//缩放大小
	std::wstring	_MapName;					//组件所属地图名
	NxU32			_StaticID;
};

class SceneBlockingCompDesc :public StaticCompDesc
{
	DECLARE_RTTI
public:
	SceneBlockingCompDesc()
		: StaticCompDesc()
	{}

	virtual bool IsValid() {return true;}

	bool	_bBlockCamera;
};

class SceneTriggerCompDesc :public StaticCompDesc
{
	DECLARE_RTTI
public:
	SceneTriggerCompDesc()
		: StaticCompDesc()
	{}

	virtual bool IsValid() {return true;}
	int _PhyAreaType;
	int _AreaId;
};

class SceneOceanCompDesc :public StaticCompDesc
{
	DECLARE_RTTI
public:
	SceneOceanCompDesc()
		: StaticCompDesc()
	{}

	virtual bool IsValid() {return true;}
};

class SceneTerrainCompDesc :public StaticCompDesc
{
	DECLARE_RTTI
public:
	SceneTerrainCompDesc()
		: StaticCompDesc()
	{}

	virtual bool IsValid() {return true;}
};

class ScenePlayerStartCompDesc :public StaticCompDesc
{
	DECLARE_RTTI
public:
	ScenePlayerStartCompDesc()
		: StaticCompDesc()
	{}

	virtual bool IsValid() {return true;}

	int _TeamIndex;
	int _SeatIndex;
	std::vector<int> _PriorVehicleType;
};
class ScenePlayerRespawnCompDesc :public ScenePlayerStartCompDesc
{
	DECLARE_RTTI
public:
	ScenePlayerRespawnCompDesc()
		: ScenePlayerStartCompDesc()
	{}

	virtual bool IsValid() {return true;}
};

class ScenePhysDesc
{
public:
	std::vector<ComponentDesc*>		_SceneComponentDescList;
	std::vector<PhysModelDesc>		_ScenePhysModelDescList;

	~ScenePhysDesc()
	{
		std::vector<ComponentDesc*>::iterator it1 = _SceneComponentDescList.begin();
		for(; it1 != _SceneComponentDescList.end(); ++it1)
		{
			SAFE_DELETE(*it1);
		}
		_SceneComponentDescList.clear();

		std::vector<PhysModelDesc>::iterator it = _ScenePhysModelDescList.begin();
		for(; it != _ScenePhysModelDescList.end(); ++it)
		{
			it->Cleanup();
		}
		_ScenePhysModelDescList.clear();
	}
};
extern std::map<std::wstring, ScenePhysDesc*> GScenePhysDescList;

typedef std::vector<ComponentDesc*>		ComponentDescList;
typedef std::map<int, ComponentDesc*>	ComponentDescMap;

extern ComponentDescList				GComponentDescList;		//	所有动态组件逻辑配置，不包括与地图相关配置
extern ComponentDescMap					GComponentDescTable;	//	对上述配置作查找优化而建立的Map索引

//modify by yeleiyu@icee.cn for xml encrypt and decrypt
/** 加载所有组件逻辑配置数据 */
bool LoadComponents( const std::wstring& filename, ComponentDescList& components ,bool EnableReadEncryptedXML =FALSE);
/**	建立Components索引，加速查找*/
void IndexComponents(const ComponentDescList& components, ComponentDescMap& indexed_component);
/** 根据CompId加载逻辑数据 */
bool LoadComponentDesc(int CompId, ComponentDesc* &CompDesc);
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
bool LoadPhysDataConfig(const std::wstring& filename,PhysGameSetting &config,bool EnableReadEncryptedXML =FALSE);
/** 加载物理场景 */
bool LoadScenePhys(const std::wstring& mapName, std::vector<ComponentDesc*>** pCompDescList, std::vector<PhysModelDesc>** pModelDescList);
/**	释放物理场景 */
void ReleaseScenePhys(const std::wstring& mapName);


/**
 * 组件实例基类（例如，舰船、武器、舰桥等）
 * WARNING: 实例化数据由使用者提供，切记转换至Phys scale制式下
 */
class ComponentData
{
	DECLARE_RTTI
public:
	ComponentData(ComponentDesc* Desc)
		: _CompDesc(Desc), _PhysModelDesc(NULL)
	{ _InitGlobalPose.id(); }

	virtual ~ComponentData() {}

	virtual void Init()
	{
		InitPhysModelDesc();
		if( _CompDesc != NULL )
		{
			_InitGlobalPose = _CompDesc->_InitGlobalPose;
			_InitGlobalPose.t *= G2PScale;
		}
	}
	virtual bool IsValid() { return true; }

	virtual bool IsVehicleAttachedComp() { return false; }
	virtual bool IsWeaponComp() { return false; }
	virtual bool IsVehicleComp() { return false; }
	virtual bool IsAircraftComp() { return false; }
	virtual bool IsProjectileComp() { return false; }
	virtual bool IsStaticComp() { return false;}
	virtual bool IsTriggerComp() {return false;}
	virtual bool IsBlockingComp() {return false;}

	bool FindSocket(ESocketType SocketType, std::vector<Socket>& SocketsList)
	{
		SocketsList.clear();
		if( _PhysModelDesc != NULL )
		{
			_PhysModelDesc->FindSocket(SocketType, SocketsList);
		}
		return SocketsList.size() > 0;
	}

	ComponentDesc*			_CompDesc;
	PhysModelDesc*			_PhysModelDesc;
	NxMat34					_InitGlobalPose;			// 初始化位置

protected:
	ComponentData();
	ComponentData(const ComponentData&);
	ComponentData& operator=(const ComponentData& right);
	
	void InitPhysModelDesc();
};

class VehicleAttachedCompData : public ComponentData
{
	DECLARE_RTTI
public:
	VehicleAttachedCompData(ComponentDesc* Desc)
		: ComponentData(Desc)
	{}
	virtual bool IsVehicleAttachedComp() { return true; }

	int		_AttachedId;		//	部件ID
	int		_PartType;			//	部件类型

protected:
	VehicleAttachedCompData();
	VehicleAttachedCompData(const VehicleAttachedCompData&);
	VehicleAttachedCompData& operator=(const VehicleAttachedCompData& right);
};

class WeaponData : public VehicleAttachedCompData
{
	DECLARE_RTTI
public:
	WeaponData(ComponentDesc* Desc)
		: VehicleAttachedCompData(Desc)
	{
		_TurretAngleVelocity = 10.f;
		_AccuracyDefault=10.0f;
		_AccuracyTurrent=300.0f;
		_AccuracyShipDrive =1000.0f;				
		_AccuracyShipSteer =1000.0f;
		_AccuracyRecover=30.0f;
		_AccuracyFire=1000.0f;
		_AccuracyContinuousFire=1.0f;
		_ArtilleryDiffuseParam=1.0f;
	}

	virtual bool IsWeaponComp() { return true; }
	virtual void Init()
	{
		VehicleAttachedCompData::Init();
	}
	
	void UGetWeaponDescInfo(float& Gravity, float& InitVelocity, ETrajectoryMethod& Method) const
	{
		WeaponCompDesc* Desc = (WeaponCompDesc*)_CompDesc;
		if( Desc != NULL )
		{
			Gravity = _Gravity * P2GScale;
			InitVelocity = _InitVelocity * P2GScale;
			Method = Desc->_TrajectoryMethod;
		}
	}

	float	_InitVelocity;					//	武器炮弹初速度
	float	_Gravity;						//	射出的炮弹所受重力加速度，仅作用于抛物线炮弹
	float	_TurretAngleVelocity;			//	degree/sec, 炮台转动角速度
	int		_FirePartNum;					//	炮管数量
	float	_AccuracyDefault;				//	默认射击散射半径
	float	_AccuracyTurrent;				//	炮台转向对射击精确度的影响
	float	_AccuracyShipDrive;				//	舰船行驶对射击精确度的影响
	float	_AccuracyShipSteer;				//	舰船转向对射击精确度的影响
	float	_AccuracyRecover;				//	射击精确度每秒恢复值
	float	_AccuracyFire;					//	开炮对射击精确度的影响
	float   _AccuracyContinuousFire;		//	持续射击精准度上线降低的比例	
	float	_ArtilleryDiffuseParam;			//	火炮散射参数

protected:
	WeaponData();
	WeaponData(const WeaponData&);
	WeaponData& operator=(const WeaponData& right);	
};

class VehicleData : public ComponentData
{
	DECLARE_RTTI
public:
	VehicleData(ComponentDesc* Desc)
		: ComponentData(Desc)
	{}

	virtual ~VehicleData()
	{
		std::vector<VehicleAttachedCompData*>::iterator it = _AttachedCompList.begin();
		for(; it != _AttachedCompList.end(); ++it)
		{
			if( *it != NULL )
			{
				delete *it;
				*it = NULL;
			}
		}
		_AttachedCompList.clear();
	}

	virtual void Init()
	{
		ComponentData::Init();

		_LinearVelocity.zero();
	}
	
	virtual bool IsVehicleComp() { return true; }

	// 返回的Socket位置信息已转换至Phys scale制式下
	bool FindSocket(int AttachedId, Socket& Sk)
	{
		for(NxU32 i = 0; _PhysModelDesc != NULL && i < _PhysModelDesc->_SocketDescList.size(); ++i)
		{
			if( _PhysModelDesc->_SocketDescList[i]._AttachedId == AttachedId )
			{
				Sk = _PhysModelDesc->_SocketDescList[i];
				Sk._LocalPose.t *= G2PScale;
				return true;
			}
		}
		return false;
	}

	void GetSocketListByPartType(const int PartType, std::vector<int>& AttachedIdList) const
	{
		if( _PhysModelDesc != NULL )
		{
			std::vector<Socket> Sockets = _PhysModelDesc->_SocketDescList;
			for ( std::vector<Socket>::iterator it = Sockets.begin(); it != Sockets.end(); ++it )
			{
				if( it->_PartType == PartType )
				{
					AttachedIdList.push_back((*it)._AttachedId);
				}
			}
		}
	}
	
	std::vector<VehicleAttachedCompData*>		_AttachedCompList;		// 搭载的所有组件数据

	NxVec3				_LinearVelocity;
	float				_MaxForwardSpeed;			// 最大前进档速度
	float				_MaxForwardSpeedCopy;		// 最大前进挡速度备份值
	float				_MaxForwardForce;			// 前进时力的大小
	float				_TurnTorque;				// 转向扭矩
	float				_MaxRiseSinkForce;			// 上浮下潜FORCE大小
	float				_Mass;						// 质量
	float				_CMassOffsetX;				// 质心在X轴方向的OFFSET
	float				_Length;
	float				_Width;
	float				_Height;

protected:
	VehicleData();
	VehicleData(const VehicleData&);
	VehicleData& operator=(const VehicleData& right);
};

//------------------------------------------------------------------------
//	@brief	舰载机实例数据
//------------------------------------------------------------------------
class AircraftData : public ComponentData
{
	DECLARE_RTTI
public:
	AircraftData(ComponentDesc* Desc)
		: ComponentData(Desc)	
	{
		_AircraftType = EPT_Scout;			//	舰载机类型
		_Speed = 10;						//	航速
		_Consume = 0.1f;					//	油耗
		_MinRadius = 20;					//	最小转弯半径
		_SensorRange = 10;					//	侦查范围
		_InterceptRange = 10;				//	拦截范围
		_ChaseRange = 10;					//	追击范围
		_AttackExtent = 2;					//	攻击范围半径
		_AttackScope = 30;					//	攻击范围夹角
		_EchoInterval = 2.f;				//	反应时间
		_Gravity = 0.f;
		_InitPrecision = 0.f;
		_Precision = 0.f;
		_Collimation = 0.f;
		_TurningAffect = 0.f;
		_LayerAffect = 0.f;
		_DiveAcceleration = -10.f;			//	俯冲加速度
		_PullAcceleration = 40.f;			//	拉升加速度
		_DiffuseEccentricity = 0.6f;

		//------------------------------------------------------------------------
		//	offset means vector from a point of any index to the 0-point
		//	their distance was also consisted
		//							0		
		//						   / \		
		//						  1   2		
		//						 /     \	
		//						3       4	
		//------------------------------------------------------------------------
		for ( int idx = 0; idx < 5; idx++ )
		{
			_FormationOffset[idx].zero();
		}

		AircraftCompDesc* pCompDesc = DynamicCast(AircraftCompDesc, _CompDesc);
		if ( !pCompDesc )
			return;

		NxQuat OuterRot, InnerRot;
		OuterRot.fromAngleAxis(-pCompDesc->_FormationAngle, AXIS_VEC_Z);
		InnerRot.fromAngleAxis(pCompDesc->_FormationAngle, AXIS_VEC_Z);

		NxVec3 OuterExtension = OuterRot.rot(AXIS_VEC_X);
		NxVec3 InnerExtension = InnerRot.rot(AXIS_VEC_X);
		OuterExtension.normalize();
		InnerExtension.normalize();

		_FormationOffset[1].t = OuterExtension * pCompDesc->_FormationSpace;
		_FormationOffset[2].t = InnerExtension * pCompDesc->_FormationSpace;
		_FormationOffset[3].t = OuterExtension * pCompDesc->_FormationSpace * 2;
		_FormationOffset[4].t = InnerExtension * pCompDesc->_FormationSpace * 2;
	}

	virtual void Init()
	{
		ComponentData::Init();
	}

	virtual bool IsValid()
	{
		return true;
	}

	EPartType			_AircraftType;			//	类型
	float				_Speed;					//	航速
	float				_Consume;				//	油耗
	float				_MinRadius;				//	最小转弯半径
	float				_SensorRange;			//	侦查范围
	float				_InterceptRange;		//	拦截范围
	float				_ChaseRange;			//	追击范围
	float				_AttackExtent;			//	攻击范围半径
	float				_AttackScope;			//	攻击范围夹角
	float				_EchoInterval;			//	反应时间
	NxMat34				_FormationOffset[5];	//	阵型偏移量
	float				_Gravity;				//	炮弹重力
	float				_InitPrecision;			//	初始投弹精度
	float				_Precision;				//	最准投弹精度（百分比）
	float				_Collimation;			//	缩圈速度（百分比）
	float				_TurningAffect;			//	转向精度影响（百分比）
	float				_LayerAffect;			//	换层精度影响（百分比）
	float				_AscendSlope;			//	上升斜率
	float				_SwoopSlope;			//	俯冲斜率
	float				_ExLowFlyingTime;		//	贴海飞行时间
	float				_LayerChangeSpeed;		//	换层速度
	float				_DiveAcceleration;		//	俯冲加速度
	float				_PullAcceleration;		//	拉升加速度
	float				_DiffuseEccentricity;	//	投弹散布离心率

protected:
	AircraftData();
	AircraftData(const AircraftData&);
	AircraftData& operator=(const AircraftData& right);
};

class ProjectileData : public ComponentData
{
	DECLARE_RTTI
public:
	ProjectileData(ComponentDesc* Desc)
		: ComponentData(Desc)
	{}

	virtual void Init()
	{
		ComponentData::Init();

		_StartPos.zero();
		_EndPos.zero();
		_InitVelocity.zero();
		_TrajectoryMethod = ETM_Parabola;
		_TargetPos.zero();
		_BrakingVelocity.zero();
	}

	virtual bool IsValid()
	{
		if( _InitVelocity.magnitudeSquared() == 0.f )
		{
			return false;
		}
		//TODO 
		return true;
		//NxVec3 DirPos;
		//NxVec3 DirVel;
		//if( _TrajectoryMethod == ETM_Parabola )
		//{
		//	DirPos = _EndPos - _StartPos;
		//	DirPos.z = 0;
		//	DirPos.normalize();

		//	DirVel = _InitVelocity;
		//	DirVel.z = 0;
		//	DirVel.normalize();
		//}
		//else if( _TrajectoryMethod == ETM_Straight )
		//{
		//	DirVel = _InitVelocity;
		//	DirVel.normalize();

		//	DirPos = _EndPos - _StartPos;
		//	DirPos.normalize();
		//}

		//// 飞行方向与目标方向一致
		//if( DirVel.equals(DirPos, GPL_KINDA_SMALL_NUMBER) )
		//{
		//	return true;
		//}

		//return false;
	}

	virtual bool IsProjectileComp() { return true; }

	NxVec3				_StartPos;
	NxVec3				_EndPos;
	NxVec3				_InitVelocity;
	NxVec3				_Gravity;
	ETrajectoryMethod	_TrajectoryMethod;
	float				_SafeTime;				//	安全时间，若发生碰撞则炮弹消失
	float				_BurstTime;				//	超过该时间就爆炸
	float				_BurstRange;			//	引爆后计算伤害的范围
	float				_MagneticSensorRange;	//	磁感引信检测范围
	NxVec3				_BrakingVelocity;		//	自身制动速度,仅限于鱼雷、深水炸弹与水面运行时的速度，抛射速度由武器决定
	NxVec3				_TargetPos;				//	射击目标点，因精准度逻辑不一定命中

protected:
	ProjectileData();
	ProjectileData(const ProjectileData&);
	ProjectileData& operator=(const ProjectileData& right);
};

class SceneStaticData : public ComponentData
{
	DECLARE_RTTI
public:
	SceneStaticData(ComponentDesc* Desc)
		: ComponentData(Desc)
	{}

	virtual void Init()
	{
		ComponentData::Init();
	}

	virtual bool IsStaticComp()
	{
		return true;
	}

protected:
	SceneStaticData();
	SceneStaticData(const SceneStaticData&);
	SceneStaticData& operator=(const SceneStaticData& right);	
};

class ScenePlayerStartData : public SceneStaticData
{
	DECLARE_RTTI
public:
	ScenePlayerStartData(ComponentDesc* Desc)
		: SceneStaticData(Desc)
	{}

	virtual void Init()
	{
		SceneStaticData::Init();
	}

	virtual bool IsStaticComp()
	{
		return true;
	}

protected:
	ScenePlayerStartData();
	ScenePlayerStartData(const ScenePlayerStartData&);
	ScenePlayerStartData& operator=(const ScenePlayerStartData& right);
};

class ScenePlayerRespawnData : public ScenePlayerStartData
{
	DECLARE_RTTI
public:
	ScenePlayerRespawnData(ComponentDesc* Desc)
		: ScenePlayerStartData(Desc)
	{}

protected:
	ScenePlayerRespawnData();
	ScenePlayerRespawnData(const ScenePlayerRespawnData&);
	ScenePlayerRespawnData& operator=(const ScenePlayerRespawnData& right);
};

class SceneBlockingData : public SceneStaticData
{
	DECLARE_RTTI
public:
	SceneBlockingData(ComponentDesc* Desc)
		: SceneStaticData(Desc)
	{}

	virtual void Init()
	{
		SceneStaticData::Init();
	}
	
	virtual bool IsBlockingComp() {return true;}

protected:
	SceneBlockingData();
	SceneBlockingData(const SceneBlockingData&);
	SceneBlockingData& operator=(const SceneBlockingData& right);
};

class SceneTriggerData : public SceneStaticData
{
	DECLARE_RTTI
public:
	SceneTriggerData(ComponentDesc* Desc)
		: SceneStaticData(Desc)
	{}

	virtual void Init()
	{
		SceneStaticData::Init();
	}

	virtual bool IsTriggerComp() {return true;}

protected:
	SceneTriggerData();
	SceneTriggerData(const SceneTriggerData&);
	SceneTriggerData& operator=(const SceneTriggerData& right);
};

class SceneOceanData : public SceneStaticData
{
	DECLARE_RTTI
public:
	SceneOceanData(ComponentDesc* Desc)
		: SceneStaticData(Desc)
	{}

	virtual void Init()
	{
		SceneStaticData::Init();
	}

	//virtual bool IsBlockingComp() {return true;}

protected:
	SceneOceanData();
	SceneOceanData(const SceneOceanData&);
	SceneOceanData& operator=(const SceneStaticData& right);
};

class SceneTerrainData : public SceneStaticData
{
	DECLARE_RTTI
public:
	SceneTerrainData(ComponentDesc* Desc)
		: SceneStaticData(Desc)
	{}

	virtual void Init()
	{
		SceneStaticData::Init();
	}

	//virtual bool IsBlockingComp() {return true;}

protected:
	SceneTerrainData();
	SceneTerrainData(const SceneTerrainData&);
	SceneTerrainData& operator=(const SceneStaticData& right);
};

// 生成所有组件实例化数据
ComponentData* ConstructComponentData(ComponentDesc* CompDesc);


}

#endif