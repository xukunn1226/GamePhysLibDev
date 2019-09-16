//=============================================================================
// PhysXSupport: PhysX SDK��ʼ�������٣��������������٣�
//=============================================================================
#ifndef _PHYSX_SUPPORT_H_
#define _PHYSX_SUPPORT_H_

#pragma warning( disable : 4305 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4263 )
#pragma warning( disable : 4002 )
#pragma warning( disable : 4244 )

#pragma comment(lib, "PhysXLoader.lib")
#pragma comment(lib, "PhysXCooking.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "NxFoundation.h"
#include "NxPhysics.h"
#include "NxCooking.h"
#include "NxSceneQuery.h"
#include "NxStream.h"

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <functional>

#include "Utils.h"
#include "RTTISupport.h"
#include "SingletonSupport.h"
#include "PhysMath.h"

#include "PhysCommonDefine.h"
#include "PhysPublic.h"
#include "PhysUtility.h"
#include "OutputStream.h"
#include "PhysLoader.h"
#include "GameObjects.h"
#include "GameObjectState.h"
#include "GameObjectFSM.h"
#include "PhysGameObject.h"
#include "PhysGameObjectAircraft.h"
#include "PhysGameObjectProjectile.h"
#include "PhysGameObjectStatic.h"
#include "PhysGameObjectTerrain.h"
#include "PhysGameObjectBlockingBrush.h"
#include "PhysGameObjectTriggerBrush.h"
#include "PhysGameObjectOcean.h"
#include "PhysGameObjectPlayerStart.h"
#include "VehicleSimulation.h"
#include "PhysGameObjectVehicle.h"
#include "PhysTerrain.h"
#include "GameObjectFactory.h"
#include "RouteAssist.h"
#include "xmlutil.h"

namespace GPL
{ // begin namespace
	
extern ERunMode					GRunMode;		// �������ģʽ
extern NxCookingInterface*		GCooking;		// cooking interface
extern NxPhysicsSDK*			GPhysXSDK;		// physics engine SDK
extern PhysUserOutputStream*	GOutputStream;	// ��־���ӿ�
extern std::wstring				GWorkingPath;	// working directory
extern PhysGameSetting			GPhysGameSetting;
extern bool						GGaussianUseLlast;
extern float					GGaussianY2;
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
extern bool GEnableReadEncryptedXML;
#if 0
extern bool GFireAimingCheck;					//	����ж��Ƿ��������ת��
#endif

// �������������ľ�̬���ã���ʼ��SDK֮ǰ����
bool LoadLibConfig(const std::wstring WorkingPath,bool EnableReadEncryptedXML = FALSE);

bool LoadMapConfig(const std::wstring MapName);

/**
 * @brief	�ͻ������������ʼ���ӿڣ����������UE3�����������ʼ�����̣��������˳�ʼ���ӿڷֿ�����
 *
 * @param	[NxCookingInterface]	��NULL������COOKING���ⲿ���ɣ��˺���ִֻ��setParameter��ΪNULL���ڲ�����
 * @param	[NxUserAllocator]		�ڴ�������
 * @param	[PhysUserOutputStream]	��־ϵͳ
 * @return	NONE
 */
void ClientInitGamePhysLib(NxCookingInterface* InCooking, NxUserAllocator* InAllocator, PhysUserOutputStream* Stream = NULL, bool bEnableDebugger = false);

/**
 * @brief	��������������ʼ���ӿ�
 *
 * @param	[PhysUserOutputStream]	��־ϵͳ
 * @return	NONE
 */
void ServerInitGamePhysLib(NxUserAllocator* InAllocator = NULL, PhysUserOutputStream* Stream = NULL);

/**
 * @brief	��������SDK
 */
void DestroyGamePhysLib();

/**
 * @brief	���ȫ�־�̬���� call in DestroyGamePhysLib
 */
void StaticExit();

/**
 * �������ع����е��¼�֪ͨ
 */
class UserLoadingReport
{
public:
	// ������سɹ���֪ͨ
	virtual void OnFinishedLoadPhysGameObject(PhysGameObject* Object) {}
};

/**
 * @brief	��Ϸ�������󣬷�װ��NxScene, NxSceneQuery, NxArray<PhysGameObject*>, myRaycastQueryReport
 */
class PhysGameScene
{
public:
	PhysGameScene();

	class myRaycastQueryReport : public NxSceneQueryReport
	{
		virtual	NxQueryReportResult	onBooleanQuery(void* userData, bool result){ return NX_SQR_CONTINUE; };
		virtual	NxQueryReportResult	onShapeQuery(void* userData, NxU32 nbHits, NxShape** hits){ return NX_SQR_CONTINUE; };
		virtual	NxQueryReportResult	onSweepQuery(void* userData, NxU32 nbHits, NxSweepQueryHit* hits){ return NX_SQR_CONTINUE; };

		virtual	NxQueryReportResult	onRaycastQuery(void* userData, NxU32 nbHits, const NxRaycastHit* hits) { return NX_SQR_CONTINUE; }
	};

	class myNxContactReport : public NxUserContactReport
	{
	public:
		virtual void  onContactNotify(NxContactPair& pair, NxU32 events);
	};

	class myUserTriggerReport : public NxUserTriggerReport
	{
		virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status);
	};

	void AddGameObject(PhysGameObject* GameObject);

	void RemoveGameObject(PhysGameObject* GameObject);
	
	bool InitScene(std::wstring mapName);

	void TermScene(const std::wstring& mapName);

	void GetPlayerStart(int& ListCnt, PhysPlayerStart* StartList[PLAYER_START_SIZE_MAX]);

	void GetPlayerRespawn(int& ListCnt, PhysPlayerRespawn* RespawnList[PLAYER_RESPAWN_SIZE_MAX]);

	void SetUserLoadingReport(UserLoadingReport* Report);

	void ForceProjectileSelfDestruction(PhysProjectile* Projectile);

	/**
	 * ���[Start��End]�������PhysGameObject
	 */
	int GetAllObjectList(NxVec3 Start, NxVec3 End, int groups, int& iObjectNum, PhysGameObject* OutBlockedObjList[MAX_BLOCK_OBJECT_PER_LINE]);

public:
	NxBounds3						_SceneBounds;

	NxScene*						_PhysScene;

	std::vector<PhysGameObject*>	_PhysGameObjectList;

	NxSceneQuery*					_SceneQuery;

	myRaycastQueryReport			_QueryReport;

	myNxContactReport				_ContactReport;

	myUserTriggerReport				_TriggerReport;

	UserLoadingReport*				_LoadingReport;

	std::vector<CollisionNotifyInfo>	_PendingCollisionNotifies;

	std::wstring _MapName;
};

/**
 * @brief	��������
 */
PhysGameScene* CreatePhysScene();

/**
 * @brief	��������
 */
void DestroyPhysScene(PhysGameScene* Scene);
//add  by yeleiyu@icee.cn TickPhysGameScene���ͳ��
void TickPhysScene(PhysGameScene* Scene, float DeltaTime);
bool FetchPhysSceneResults(PhysGameScene* Scene);
void TickAsyncWork(PhysGameScene* Scene, float DeltaTime);
void DispatchCollisionNotifies(PhysGameScene* Scene);

/**
 * @brief	call in main loop
 */
void TickPhysGameScene(PhysGameScene* Scene, float DeltaTime);

/**
 * @brief	
 */
ComponentDesc*	GetPhysComponentDesc(int CompId);

} // end namespace


#endif