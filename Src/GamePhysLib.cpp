#include "..\Inc\PhysXSupport.h"

namespace GPL
{ // begin namespace

ERunMode						GRunMode  = ERM_Client;
NxCookingInterface*				GCooking = NULL; 
NxPhysicsSDK*					GPhysXSDK = NULL;
PhysUserOutputStream*			GOutputStream = NULL;
std::wstring					GWorkingPath;
std::vector<PhysModelDesc>		GPhysModelDescList;
ComponentDescList				GComponentDescList;
ComponentDescMap				GComponentDescTable;
PhysGameSetting					GPhysGameSetting;
bool							GGaussianUseLlast;
float							GGaussianY2;
//modify by yeleiyu@icee.cn for xml encrypt and decrypt
bool GEnableReadEncryptedXML;
#if 0
bool GFireAimingCheck = true;
#endif

} // end namespace