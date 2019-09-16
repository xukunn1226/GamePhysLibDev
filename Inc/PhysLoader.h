//=============================================================================
// PhysLoader: ����������ݣ�ģ���������߼����á�ʵ�������ݽṹ
// ���ܽӿڣ�
//	// ��ȡģ�������ļ�
//	bool LoadPhysModels( const std::wstring& filename, std::vector<GPL::PhysModelDesc> &models );
//	// ����PhysId����ģ������
//	bool LoadPhysModelDesc(std::wstring PhysId, PhysModelDesc* &ModelDesc);
//  // ������������߼���������
//	bool LoadComponents( const std::wstring& filename, std::vector<GPL::ComponentDesc*> &components );
//	// ����CompId�����߼�����
//	bool LoadComponentDesc(int CompId, ComponentDesc* &CompDesc);
//	// �����������ʵ��������
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

// ���ڶ�λ�����ϵ�λ����Ϣ
struct Socket
{
	Socket()
	:_EnableLimit(FALSE), _SocketType(EST_Socket_Part), _AttachedId(0), _PartType(EPT_Invalid)
	{
	}
	ESocketType	_SocketType;			// ������SOCKET����;
	EPartType	_PartType;				// �������ͣ��߼�����
	int			_AttachedId;			// ȫ��Ψһ��ID����ʶ�������߼�����
	NxMat34		_LocalPose;				// ��Խ����ľֲ�POSE����λ����Ϣ��Ч
	float		_YawCWLimit;			// in degree,��Գ�ʼ����_InitRotation����˳ʱ��Ƕ�����
	float		_YawCCWLimit;			// in degree����Գ�ʼ����_InitRotation������ʱ��Ƕ�����
	bool		_EnableLimit;			// �Ƿ�ת��Ŀ����������true��ת��Ŀ����������false����֮
	float		_InitialRotation;		// in degree, ��ʼ����
	std::vector<LimitAngle> _FireLimitAngles;  // in degree,���ԭʼλ�ÿ�������ļн�
	std::vector<LimitAngle> _FirePressureLimitAngles;  // in degree,���ԭʼλ�ÿ����γ�ˮ���ѹ�ļн�
};

/*
 * @brief	ģ��������������
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
	NxMat34			_LocalPose;			// ���Actor frame����
	std::wstring	_DatSrc;			// �Ƿ���pml�ļ���������
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
 * ��̬���ݣ�����ģ������
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
	
	// ���ص�Socketλ����Ϣ��ת����Phys scale��ʽ��
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

	std::wstring				_PhysId;					// ģ����������ID
	std::vector<ShapeDesc*>		_ShapeDescList;				// collection of all shapes
	std::vector<Socket>			_SocketDescList;			// collection of all sockets
};
extern std::vector<PhysModelDesc>		GPhysModelDescList;		// ���ж�̬���������ģ�����ݣ����������������ţ������������ͼ�������
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
/** ��ȡ����ģ������ */
bool LoadPhysModels( const std::wstring& filename, std::vector<GPL::PhysModelDesc> &models ,bool EnableReadEncryptedXML =FALSE );
/** ����PhysId����ģ������ */
bool LoadPhysModelDesc(std::wstring PhysId, PhysModelDesc* &ModelDesc);


/**
 * ��̬���ݣ�����߼�����
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

	//���½ӿڲ��Ƽ�ʹ�ã����𽥷�����Ŀǰ������������ֻ�ã���ʹ��RTTI 
	virtual bool IsVehicleAttachedComp() { return false; }
	virtual bool IsWeaponComp() { return false; }
	virtual bool IsVehicleComp() { return false; }
	virtual bool IsProjectileComp() { return false; }
	virtual bool IsStaticComp() {return false;}

public:
	int					_CompId;			// �������ID
	std::wstring		_PhysId;			// ���õ�ģ������ID
	EConfigType			_ConfigType;		// ����������ͣ����õ���ʱ����ʵ��������
	EGameObjectType		_ObjectType;		// �������ͣ����ݴ�����ʵ������ͬ����
	ELoadingFlag		_LoadingFlag;		// ���ر��
	NxMat34				_InitGlobalPose;	// ��ʼ��λ��
	int					_PhyMaterialType;

	virtual bool IsValid()
	{
		return true;
	}
};

// �ɱ��ؾ�װ��������߼����ݣ��������������š���������
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

	ETrajectoryMethod	_TrajectoryMethod;				// ֧�ֵ��ڵ����й켣
	float				_GunTubeLimitAngleLower;		// in degree, �ڹܴ�ֱת��(pitch)�Ƕ�����
	float				_GunTubeLimitAngleUpper;		// �ڹܴ�ֱת��(pitch)�Ƕ�����
	float				_GunTubeAngleVelocity;			// in degree, �ڹ�ת�����ٶ�
	float				_AccuracyTurrentMax;			//��̨ת��������ȷ�ȵ�Ӱ�����ֵ
	float				_AccuracyTotalMax;				//�ܾ�ȷ�ȵ�Ӱ�����ֵ

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

	float			_MaxFlightTime;			//	�����ʱ�䣬�������Զ���ʧ	
	float			_HitRange;				//	������ⷶΧ��=0��ʾֻ�����������巢����ײ�ű�ը��>0��ʾ��Χ����
	int				_MagneticSensorInterval;//	�Ÿ����ż��ʱ����

};

//------------------------------------------------------------------------
//	@brief	���ػ���������
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
	
	ETrajectoryMethod	_TrajectoryMethod;		//	�ڵ���������
	float				_FormationAngle;		//	���μн�
	float				_FormationSpace;		//	���μ��
	float				_WarningDistance;		//	������루�ϰ���⣩
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


	ESimulationType		_SimType;					// �ƶ���������	
	
	float				_StopThreshold;				// ֹͣ�ƶ���ֵ	
	float				_LongDampingForce;			// ���������С
	float				_LatDampingForce;			// ���������С����ֹƯ��
	float				_ScaleDampingSpeed;			// �������򡢺�������ʱ��Scale

	float				_MaxTurnRate;				// �����ٶ�, degree/sec
	float				_StopAngThreshold;			// in degree, ֹͣ��ת��ֵ	
	float				_TurnDampingTorque;			// û��Ť��ʱ��ת�������С
	float				_ScaleDampingTurn;			// ����ת������ʱ��Scale

	int					_MaxDriveGear;				// ���ǰ����
	int					_MaxReverseGear;			// �����˵�
	int					_MaxTurnGear;				// �����ת��

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
	
	NxVec3			_ModelScale;				//���Ŵ�С
	std::wstring	_MapName;					//���������ͼ��
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

extern ComponentDescList				GComponentDescList;		//	���ж�̬����߼����ã����������ͼ�������
extern ComponentDescMap					GComponentDescTable;	//	�����������������Ż���������Map����

//modify by yeleiyu@icee.cn for xml encrypt and decrypt
/** ������������߼��������� */
bool LoadComponents( const std::wstring& filename, ComponentDescList& components ,bool EnableReadEncryptedXML =FALSE);
/**	����Components���������ٲ���*/
void IndexComponents(const ComponentDescList& components, ComponentDescMap& indexed_component);
/** ����CompId�����߼����� */
bool LoadComponentDesc(int CompId, ComponentDesc* &CompDesc);
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
bool LoadPhysDataConfig(const std::wstring& filename,PhysGameSetting &config,bool EnableReadEncryptedXML =FALSE);
/** ���������� */
bool LoadScenePhys(const std::wstring& mapName, std::vector<ComponentDesc*>** pCompDescList, std::vector<PhysModelDesc>** pModelDescList);
/**	�ͷ������� */
void ReleaseScenePhys(const std::wstring& mapName);


/**
 * ���ʵ�����ࣨ���磬���������������ŵȣ�
 * WARNING: ʵ����������ʹ�����ṩ���м�ת����Phys scale��ʽ��
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
	NxMat34					_InitGlobalPose;			// ��ʼ��λ��

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

	int		_AttachedId;		//	����ID
	int		_PartType;			//	��������

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

	float	_InitVelocity;					//	�����ڵ����ٶ�
	float	_Gravity;						//	������ڵ������������ٶȣ����������������ڵ�
	float	_TurretAngleVelocity;			//	degree/sec, ��̨ת�����ٶ�
	int		_FirePartNum;					//	�ڹ�����
	float	_AccuracyDefault;				//	Ĭ�����ɢ��뾶
	float	_AccuracyTurrent;				//	��̨ת��������ȷ�ȵ�Ӱ��
	float	_AccuracyShipDrive;				//	������ʻ�������ȷ�ȵ�Ӱ��
	float	_AccuracyShipSteer;				//	����ת��������ȷ�ȵ�Ӱ��
	float	_AccuracyRecover;				//	�����ȷ��ÿ��ָ�ֵ
	float	_AccuracyFire;					//	���ڶ������ȷ�ȵ�Ӱ��
	float   _AccuracyContinuousFire;		//	���������׼�����߽��͵ı���	
	float	_ArtilleryDiffuseParam;			//	����ɢ�����

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

	// ���ص�Socketλ����Ϣ��ת����Phys scale��ʽ��
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
	
	std::vector<VehicleAttachedCompData*>		_AttachedCompList;		// ���ص������������

	NxVec3				_LinearVelocity;
	float				_MaxForwardSpeed;			// ���ǰ�����ٶ�
	float				_MaxForwardSpeedCopy;		// ���ǰ�����ٶȱ���ֵ
	float				_MaxForwardForce;			// ǰ��ʱ���Ĵ�С
	float				_TurnTorque;				// ת��Ť��
	float				_MaxRiseSinkForce;			// �ϸ���ǱFORCE��С
	float				_Mass;						// ����
	float				_CMassOffsetX;				// ������X�᷽���OFFSET
	float				_Length;
	float				_Width;
	float				_Height;

protected:
	VehicleData();
	VehicleData(const VehicleData&);
	VehicleData& operator=(const VehicleData& right);
};

//------------------------------------------------------------------------
//	@brief	���ػ�ʵ������
//------------------------------------------------------------------------
class AircraftData : public ComponentData
{
	DECLARE_RTTI
public:
	AircraftData(ComponentDesc* Desc)
		: ComponentData(Desc)	
	{
		_AircraftType = EPT_Scout;			//	���ػ�����
		_Speed = 10;						//	����
		_Consume = 0.1f;					//	�ͺ�
		_MinRadius = 20;					//	��Сת��뾶
		_SensorRange = 10;					//	��鷶Χ
		_InterceptRange = 10;				//	���ط�Χ
		_ChaseRange = 10;					//	׷����Χ
		_AttackExtent = 2;					//	������Χ�뾶
		_AttackScope = 30;					//	������Χ�н�
		_EchoInterval = 2.f;				//	��Ӧʱ��
		_Gravity = 0.f;
		_InitPrecision = 0.f;
		_Precision = 0.f;
		_Collimation = 0.f;
		_TurningAffect = 0.f;
		_LayerAffect = 0.f;
		_DiveAcceleration = -10.f;			//	������ٶ�
		_PullAcceleration = 40.f;			//	�������ٶ�
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

	EPartType			_AircraftType;			//	����
	float				_Speed;					//	����
	float				_Consume;				//	�ͺ�
	float				_MinRadius;				//	��Сת��뾶
	float				_SensorRange;			//	��鷶Χ
	float				_InterceptRange;		//	���ط�Χ
	float				_ChaseRange;			//	׷����Χ
	float				_AttackExtent;			//	������Χ�뾶
	float				_AttackScope;			//	������Χ�н�
	float				_EchoInterval;			//	��Ӧʱ��
	NxMat34				_FormationOffset[5];	//	����ƫ����
	float				_Gravity;				//	�ڵ�����
	float				_InitPrecision;			//	��ʼͶ������
	float				_Precision;				//	��׼Ͷ�����ȣ��ٷֱȣ�
	float				_Collimation;			//	��Ȧ�ٶȣ��ٷֱȣ�
	float				_TurningAffect;			//	ת�򾫶�Ӱ�죨�ٷֱȣ�
	float				_LayerAffect;			//	���㾫��Ӱ�죨�ٷֱȣ�
	float				_AscendSlope;			//	����б��
	float				_SwoopSlope;			//	����б��
	float				_ExLowFlyingTime;		//	��������ʱ��
	float				_LayerChangeSpeed;		//	�����ٶ�
	float				_DiveAcceleration;		//	������ٶ�
	float				_PullAcceleration;		//	�������ٶ�
	float				_DiffuseEccentricity;	//	Ͷ��ɢ��������

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

		//// ���з�����Ŀ�귽��һ��
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
	float				_SafeTime;				//	��ȫʱ�䣬��������ײ���ڵ���ʧ
	float				_BurstTime;				//	������ʱ��ͱ�ը
	float				_BurstRange;			//	����������˺��ķ�Χ
	float				_MagneticSensorRange;	//	�Ÿ����ż�ⷶΧ
	NxVec3				_BrakingVelocity;		//	�����ƶ��ٶ�,���������ס���ˮը����ˮ������ʱ���ٶȣ������ٶ�����������
	NxVec3				_TargetPos;				//	���Ŀ��㣬��׼���߼���һ������

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

// �����������ʵ��������
ComponentData* ConstructComponentData(ComponentDesc* CompDesc);


}

#endif