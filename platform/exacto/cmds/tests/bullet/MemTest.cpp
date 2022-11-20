#include "btBulletDynamicsCommon.h"
#include <string>
#include <stdio.h>


#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"

#include "BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.h"

#include "BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h"
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
#include "BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.h"
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM
#include "BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h"

#include "LinearMath/btPoolAllocator.h"

/// This is a Hello World program for running a basic Bullet physics simulation

int main(int argc, char** argv)
{
	int i;
	#ifdef PART1
	printf("Start0\n");
	btDefaultCollisionConstructionInfo constructionInfo;

	constructionInfo.m_defaultMaxPersistentManifoldPoolSize = 64;



	int m_persistentManifoldPoolSize;

	btPoolAllocator* m_persistentManifoldPool;
	bool m_ownsPersistentManifoldPool;

	btPoolAllocator* m_collisionAlgorithmPool;
	bool m_ownsCollisionAlgorithmPool;

	
	printf("Start21\n");
		void* mem = btAlignedAlloc(sizeof(btPoolAllocator), 16);
	int tp = sizeof(btPersistentManifold);
	int sz = constructionInfo.m_defaultMaxPersistentManifoldPoolSize;
	printf("Start22 %d %d\n", tp, sz);
	//here problem
		m_persistentManifoldPool = new (mem) btPoolAllocator(sizeof(btPersistentManifold), constructionInfo.m_defaultMaxPersistentManifoldPoolSize);
		#endif

	btDefaultCollisionConstructionInfo constructionInfo;
	constructionInfo.m_defaultMaxPersistentManifoldPoolSize = 16;
	constructionInfo.m_defaultMaxCollisionAlgorithmPoolSize = 16;

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(constructionInfo);
	printf("Start4\n");

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)

	int m_dispatcherFlags;

	btAlignedObjectArray<btPersistentManifold*> m_manifoldsPtr;

	btNearCallback m_nearCallback;

	btPoolAllocator* m_collisionAlgorithmPoolAllocator;

	btPoolAllocator* m_persistentManifoldPoolAllocator;

	btCollisionAlgorithmCreateFunc* m_doubleDispatchContactPoints[MAX_BROADPHASE_COLLISION_TYPES][MAX_BROADPHASE_COLLISION_TYPES];

	btCollisionAlgorithmCreateFunc* m_doubleDispatchClosestPoints[MAX_BROADPHASE_COLLISION_TYPES][MAX_BROADPHASE_COLLISION_TYPES];

	btCollisionConfiguration* m_collisionConfiguration;

	
	
	int i;

	// setNearCallback(defaultNearCallback);

	m_collisionAlgorithmPoolAllocator = collisionConfiguration->getCollisionAlgorithmPool();
	printf("Start5\n");

	m_persistentManifoldPoolAllocator = collisionConfiguration->getPersistentManifoldPool();
	printf("Start6: %d\n", MAX_BROADPHASE_COLLISION_TYPES);

	for (i = 0; i < MAX_BROADPHASE_COLLISION_TYPES; i++)
	{
		printf("Start7:[%d]\n",i);
		for (int j = 0; j < MAX_BROADPHASE_COLLISION_TYPES; j++)
		{
			printf("Start71:[%d]\n",j);
			m_doubleDispatchContactPoints[i][j] = m_collisionConfiguration->getCollisionAlgorithmCreateFunc(i, j);
			btAssert(m_doubleDispatchContactPoints[i][j]);
			m_doubleDispatchClosestPoints[i][j] = m_collisionConfiguration->getClosestPointsAlgorithmCreateFunc(i, j);
		}
	}

	printf("Start8\n");


	// btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	printf("End\n");

	return 0;
}