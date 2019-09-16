#include "..\Inc\PhysXSupport.h"
#include "..\inc\PhysGameObject.h"

namespace GPL
{
	IMPLEMENT_RTTI(PhysTerrain, PhysStatic)

	bool PhysTerrain::Init()
	{
		if( !PhysStatic::Init() )
		{
			return false;
		}

		return true;
	}

	void PhysTerrain::TermPhys()
	{
		std::vector<NxActor*>::iterator it = _TerrainActorList.begin();
		for(; it != _TerrainActorList.end(); ++it)
		{
			DoTermPhysActor(_Scene->_PhysScene, *it);
		}
		_TerrainActorList.clear();
	}
	
	void PhysTerrain::DoTermPhysActor(NxScene* Scene, NxActor* Actor)
	{
		if( Scene && Actor )
		{
			NxU32 NumShape = Actor->getNbShapes();
			std::vector<NxHeightField*> PendingKillHF;
			for(NxU32 i = 0; i < NumShape; ++i)
			{
				NxHeightField* HFMesh = NULL;
				NxShape* Shape = Actor->getShapes()[i];
				if( Shape->isHeightField() )
				{
					HFMesh = &(Shape->isHeightField()->getHeightField());
				}

				if( HFMesh != NULL )
				{
					PendingKillHF.push_back(HFMesh);
				}
			}

			Scene->releaseActor(*Actor);

			while( PendingKillHF.size() > 0 )
			{
				NxHeightField* HFMesh = PendingKillHF.at(PendingKillHF.size()-1);
				PendingKillHF.pop_back();
				if( HFMesh->getReferenceCount() == 0 )
				{
					GPhysXSDK->releaseHeightField(*HFMesh);
				}
			}
		}
	}
	void PhysTerrain::InitPhys()
	{
		SceneTerrainData* CompData = (SceneTerrainData*)_ComponentData;
		if( CompData == NULL || CompData->_PhysModelDesc == NULL )
		{
			return;
		}

		SceneTerrainCompDesc* CompDesc = (SceneTerrainCompDesc*)CompData->_CompDesc;
		if( CompDesc == NULL )
		{
			return;
		}
		
		for(NxU32 ShapeIndex = 0; ShapeIndex < CompData->_PhysModelDesc->_ShapeDescList.size(); ++ShapeIndex)
		{
			std::vector<NxShapeDesc*> CachedShapeDescList;
			NxHeightFieldShapeDesc* XShapeDesc = NULL;
			HeightFieldShapeDesc* CustomShapeDesc = (HeightFieldShapeDesc*)CompData->_PhysModelDesc->_ShapeDescList[ShapeIndex];
			for (unsigned int i = 0; i < CustomShapeDesc->_NxHeightFieldDescList.size(); ++i)
			{
				NxActorDesc ActorDesc;

				XShapeDesc = (NxHeightFieldShapeDesc*)CreateComponentShape(CustomShapeDesc);
				if( XShapeDesc != NULL )
				{
					NxHeightField* heightField = _Scene->_PhysScene->getPhysicsSDK().createHeightField(CustomShapeDesc->_NxHeightFieldDescList[i]); 
				
					XShapeDesc->heightField	= heightField;
					XShapeDesc->heightScale	= CustomShapeDesc->_Scale.z/128.0f*G2PScale;
					XShapeDesc->rowScale		= (CustomShapeDesc->_Scale.x) * G2PScale;
					XShapeDesc->columnScale	= (-CustomShapeDesc->_Scale.y) * G2PScale;
					XShapeDesc->meshFlags		= 0;
					XShapeDesc->materialIndexHighBits = 0;
					XShapeDesc->holeMaterial	= 2;
					XShapeDesc->group = GPL_SHAPE_GROUP_TERRAIN;
					ActorDesc.shapes.push_back(XShapeDesc);
				}
				else
				{
					gplDebugf(TEXT("ERROR: Faied to CreateComponentShape CompId[%d] PhysId[%s] ShapeIndex[%d]"), CompData->_CompDesc->_CompId, CompData->_PhysModelDesc->_PhysId.c_str(), ShapeIndex);
				}
				CachedShapeDescList.push_back(XShapeDesc);
				
				ActorDesc.body = NULL;
				NxMat34 mat = CustomShapeDesc->_HeightFieldTMList[i];
				mat.t.x *= G2PScale;
				mat.t.y *= G2PScale;
				mat.t.z *= G2PScale;
				ActorDesc.globalPose = mat;

				NxU32 ValidCode = ActorDesc.checkValid();
				if( ValidCode == 0 )
				{						
					NxActor* PhysActor = _Scene->_PhysScene->createActor(ActorDesc);	
					if( PhysActor != NULL )
					{
						PhysActor->setGroup(GPL_GROUP_DEFAULT);		// default generate contact information
						PhysActor->userData = this;		// 绑定逻辑对象至NxActor上
					
						_TerrainActorList.push_back(PhysActor);
					}
				}
			}
			ReleaseShapeDescList(CachedShapeDescList);
		}
	}

	EGameObjectType PhysTerrain::GetObjectType()
	{
		return EGOT_Terrain;
	}
}