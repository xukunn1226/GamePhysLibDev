//<<<<<<< .mine
//#include "..\Inc\PhysXSupport.h"
//
//namespace GPL
//{
//bool LoadTerrain( PhysGameScene* scene, const std::wstring& filename )
//{
//	PhysTerrain *Terrain = new PhysTerrain(scene,NULL);
//
//	FILE* pf;
//	errno_t err = _wfopen_s( &pf, filename.c_str(), L"rb" );
//	assert( err == 0 );
//	int component_count;
//
//	while(!feof(pf) )
//	{
//		float zscale;
//		fread( &zscale, sizeof zscale, 1, pf ) ;
//		if ( feof (pf) )
//			break;
//		fread( &component_count, sizeof(component_count), 1, pf);
//
//		for( int i = 0; i < component_count; i++ )
//		{
//			NxMat34 mat;
//			float Entries[9];
//			fread( Entries, sizeof(float), 9, pf );
//			mat.M.setColumnMajor(Entries);
//			fread( &mat.t.x, sizeof(float), 1, pf );
//			fread( &mat.t.y, sizeof(float), 1, pf );
//			fread( &mat.t.z, sizeof(float), 1, pf );
//
//			NxHeightFieldDesc heightFieldDesc;
//			fread( &heightFieldDesc.nbColumns, sizeof(int), 1, pf );
//			fread( &heightFieldDesc.nbRows, sizeof(int), 1, pf );
//			heightFieldDesc.format	= NX_HF_S16_TM;
//			heightFieldDesc.verticalExtent      = 0;
//			heightFieldDesc.thickness      = 0;
//			heightFieldDesc.convexEdgeThreshold = 0;
//			heightFieldDesc.samples             = new NxHeightFieldSample[(heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows)];
//			heightFieldDesc.sampleStride        = sizeof(NxHeightFieldSample);
//			//heightFieldDesc.flags = 0;
//			NxI16* Heights = new NxI16[(heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows)];
//			fread( Heights, 2, (heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows), pf );
//			//NxU8* startbyte = (NxU8*)heightFieldDesc.samples;
//			for ( NxU32 row=0; row < heightFieldDesc.nbRows; row++ )
//				for ( NxU32 col=0; col < heightFieldDesc.nbColumns; col++ )
//				{
//					int x = col;
//					int y = row;
//					NxHeightFieldSample* currentSample = &((NxHeightFieldSample*)(heightFieldDesc.samples))[y*heightFieldDesc.nbColumns + x];
//
//					currentSample->height = Heights[y*heightFieldDesc.nbColumns + x];//(((h&0xff)<<8) | (h&0xff00 >> 8)) -32768;
//					currentSample->materialIndex0 = 0;
//					currentSample->materialIndex1 = 0;
//					currentSample->tessFlag = 0;
//					currentSample->unused = 0;
//				}
//
//				NxHeightField* heightField = scene->_PhysScene->getPhysicsSDK().createHeightField(heightFieldDesc); 
//				//Data has been copied therefore free the buffer.
//
//
//				NxHeightFieldShapeDesc TerrainShapeDesc;
//				TerrainShapeDesc.heightField	= heightField;
//				//TerrainShapeDesc.shapeFlags		= NX_SF_FEATURE_INDICES | NX_SF_VISUALIZATION;
//				TerrainShapeDesc.heightScale	= zscale/128.0f*G2PScale;
//				TerrainShapeDesc.rowScale		= 128*G2PScale;
//				TerrainShapeDesc.columnScale	= -128*G2PScale;
//				TerrainShapeDesc.meshFlags		= 0;
//				TerrainShapeDesc.materialIndexHighBits = 0;
//				TerrainShapeDesc.holeMaterial	= 2;
//				TerrainShapeDesc.group = GPL_SHAPE_GROUP_TERRAIN;
//
//				NxActorDesc TerrainActorDesc;
//				TerrainActorDesc.group = GPL_GROUP_NOTIFYCOLLIDE;
//
//				TerrainActorDesc.shapes.pushBack(&TerrainShapeDesc);
//				mat.t.x *= G2PScale;
//				mat.t.y *= G2PScale;
//				mat.t.z *= G2PScale;
//
//				TerrainActorDesc.globalPose = mat;
//				NxActor *pActor =scene->_PhysScene->createActor(TerrainActorDesc);
//				Terrain->TerrionInit(pActor);
//				delete[] heightFieldDesc.samples;
//				delete[] Heights;
//		}
//	}
//	fclose(pf);
//	return TRUE;
//}
//
//
//
//void PhysTerrain::InitPhys()
//{
//
//}
//
//void PhysTerrain::TerrionInit(NxActor *pActor)
//{
//	pActor->userData=(void*)this;
//}
//
//}=======
//#include "..\Inc\PhysXSupport.h"
//
//namespace GPL
//{
//bool LoadTerrain( PhysGameScene* scene, const std::wstring& filename )
//{
//	PhysTerrain *Terrain = new PhysTerrain(scene,NULL);
//
//	FILE* pf;
//	errno_t err = _wfopen_s( &pf, filename.c_str(), L"rb" );
//	assert( err == 0 );
//	int component_count;
//
//	while(!feof(pf) )
//	{
//		float zscale;
//		fread( &zscale, sizeof zscale, 1, pf ) ;
//		if ( feof (pf) )
//			break;
//		fread( &component_count, sizeof(component_count), 1, pf);
//
//		for( int i = 0; i < component_count; i++ )
//		{
//			NxMat34 mat;
//			float Entries[9];
//			fread( Entries, sizeof(float), 9, pf );
//			mat.M.setColumnMajor(Entries);
//			fread( &mat.t.x, sizeof(float), 1, pf );
//			fread( &mat.t.y, sizeof(float), 1, pf );
//			fread( &mat.t.z, sizeof(float), 1, pf );
//
//			NxHeightFieldDesc heightFieldDesc;
//			fread( &heightFieldDesc.nbColumns, sizeof(int), 1, pf );
//			fread( &heightFieldDesc.nbRows, sizeof(int), 1, pf );
//			heightFieldDesc.format	= NX_HF_S16_TM;
//			heightFieldDesc.verticalExtent      = 0;
//			heightFieldDesc.thickness      = 0;
//			heightFieldDesc.convexEdgeThreshold = 0;
//			heightFieldDesc.samples             = new NxHeightFieldSample[(heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows)];
//			heightFieldDesc.sampleStride        = sizeof(NxHeightFieldSample);
//			//heightFieldDesc.flags = 0;
//			NxI16* Heights = new NxI16[(heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows)];
//			fread( Heights, 2, (heightFieldDesc.nbColumns)*(heightFieldDesc.nbRows), pf );
//			//NxU8* startbyte = (NxU8*)heightFieldDesc.samples;
//			for ( NxU32 row=0; row < heightFieldDesc.nbRows; row++ )
//				for ( NxU32 col=0; col < heightFieldDesc.nbColumns; col++ )
//				{
//					int x = col;
//					int y = row;
//					NxHeightFieldSample* currentSample = &((NxHeightFieldSample*)(heightFieldDesc.samples))[y*heightFieldDesc.nbColumns + x];
//
//					currentSample->height = Heights[y*heightFieldDesc.nbColumns + x];//(((h&0xff)<<8) | (h&0xff00 >> 8)) -32768;
//					currentSample->materialIndex0 = 0;
//					currentSample->materialIndex1 = 0;
//					currentSample->tessFlag = 0;
//					currentSample->unused = 0;
//				}
//
//				NxHeightField* heightField = scene->_PhysScene->getPhysicsSDK().createHeightField(heightFieldDesc); 
//				//Data has been copied therefore free the buffer.
//
//
//				NxHeightFieldShapeDesc TerrainShapeDesc;
//				TerrainShapeDesc.heightField	= heightField;
//				//TerrainShapeDesc.shapeFlags		= NX_SF_FEATURE_INDICES | NX_SF_VISUALIZATION;
//				TerrainShapeDesc.heightScale	= zscale/128.0f*G2PScale;
//				TerrainShapeDesc.rowScale		= 128*G2PScale;
//				TerrainShapeDesc.columnScale	= -128*G2PScale;
//				TerrainShapeDesc.meshFlags		= 0;
//				TerrainShapeDesc.materialIndexHighBits = 0;
//				TerrainShapeDesc.holeMaterial	= 2;
//				TerrainShapeDesc.group = GPL_SHAPE_GROUP_TERRAIN;
//
//				NxActorDesc TerrainActorDesc;
//				TerrainActorDesc.group = GPL_GROUP_NOTIFYCOLLIDE;
//
//				TerrainActorDesc.shapes.pushBack(&TerrainShapeDesc);
//				mat.t.x *= G2PScale;
//				mat.t.y *= G2PScale;
//				mat.t.z *= G2PScale;
//
//				TerrainActorDesc.globalPose = mat;
//				NxActor *pActor =scene->_PhysScene->createActor(TerrainActorDesc);
//				Terrain->TerrionInit(pActor);
//				delete[] heightFieldDesc.samples;
//				delete[] Heights;
//		}
//	}
//	fclose(pf);
//	return TRUE;
//}
//
//
//
//void PhysTerrain::InitPhys()
//{
//
//}
//
//void PhysTerrain::TerrionInit(NxActor *pActor)
//{
//	pActor->userData=(void*)this;
//}
//
//EGameObjectType PhysTerrain::GetObjectType()
//{
//	return EGOT_Terrain;
//}
//
//}>>>>>>> .r2163
