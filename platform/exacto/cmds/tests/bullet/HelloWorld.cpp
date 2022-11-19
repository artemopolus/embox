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
	printf("Start0\n");
	btDefaultCollisionConstructionInfo constructionInfo;
	btCollisionAlgorithmCreateFunc* m_convexConvexCreateFunc;
	btCollisionAlgorithmCreateFunc* m_convexConcaveCreateFunc;
	btCollisionAlgorithmCreateFunc* m_swappedConvexConcaveCreateFunc;
	btCollisionAlgorithmCreateFunc* m_compoundCreateFunc;
	btCollisionAlgorithmCreateFunc* m_compoundCompoundCreateFunc;

	btCollisionAlgorithmCreateFunc* m_swappedCompoundCreateFunc;
	btCollisionAlgorithmCreateFunc* m_emptyCreateFunc;
	btCollisionAlgorithmCreateFunc* m_sphereSphereCF;
	btCollisionAlgorithmCreateFunc* m_sphereBoxCF;
	btCollisionAlgorithmCreateFunc* m_boxSphereCF;

	btCollisionAlgorithmCreateFunc* m_boxBoxCF;
	btCollisionAlgorithmCreateFunc* m_sphereTriangleCF;
	btCollisionAlgorithmCreateFunc* m_triangleSphereCF;
	btCollisionAlgorithmCreateFunc* m_planeConvexCF;
	btCollisionAlgorithmCreateFunc* m_convexPlaneCF;
	btConvexPenetrationDepthSolver* m_pdSolver;
	void* mem = NULL;
	#ifdef PartON
	if (constructionInfo.m_useEpaPenetrationAlgorithm)
	{
		mem = btAlignedAlloc(sizeof(btGjkEpaPenetrationDepthSolver), 16);
		m_pdSolver = new (mem) btGjkEpaPenetrationDepthSolver;
	}
	else
	{
		mem = btAlignedAlloc(sizeof(btMinkowskiPenetrationDepthSolver), 16);
		m_pdSolver = new (mem) btMinkowskiPenetrationDepthSolver;
	}

	//default CreationFunctions, filling the m_doubleDispatch table
	mem = btAlignedAlloc(sizeof(btConvexConvexAlgorithm::CreateFunc), 16);
	m_convexConvexCreateFunc = new (mem) btConvexConvexAlgorithm::CreateFunc(m_pdSolver);
	mem = btAlignedAlloc(sizeof(btConvexConcaveCollisionAlgorithm::CreateFunc), 16);
	m_convexConcaveCreateFunc = new (mem) btConvexConcaveCollisionAlgorithm::CreateFunc;
	mem = btAlignedAlloc(sizeof(btConvexConcaveCollisionAlgorithm::CreateFunc), 16);
	m_swappedConvexConcaveCreateFunc = new (mem) btConvexConcaveCollisionAlgorithm::SwappedCreateFunc;
	mem = btAlignedAlloc(sizeof(btCompoundCollisionAlgorithm::CreateFunc), 16);
	m_compoundCreateFunc = new (mem) btCompoundCollisionAlgorithm::CreateFunc;

	mem = btAlignedAlloc(sizeof(btCompoundCompoundCollisionAlgorithm::CreateFunc), 16);
	m_compoundCompoundCreateFunc = new (mem) btCompoundCompoundCollisionAlgorithm::CreateFunc;

	mem = btAlignedAlloc(sizeof(btCompoundCollisionAlgorithm::SwappedCreateFunc), 16);
	m_swappedCompoundCreateFunc = new (mem) btCompoundCollisionAlgorithm::SwappedCreateFunc;
	mem = btAlignedAlloc(sizeof(btEmptyAlgorithm::CreateFunc), 16);
	m_emptyCreateFunc = new (mem) btEmptyAlgorithm::CreateFunc;

	mem = btAlignedAlloc(sizeof(btSphereSphereCollisionAlgorithm::CreateFunc), 16);
	m_sphereSphereCF = new (mem) btSphereSphereCollisionAlgorithm::CreateFunc;
// #ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
// 	mem = btAlignedAlloc(sizeof(btSphereBoxCollisionAlgorithm::CreateFunc), 16);
// 	m_sphereBoxCF = new (mem) btSphereBoxCollisionAlgorithm::CreateFunc;
// 	mem = btAlignedAlloc(sizeof(btSphereBoxCollisionAlgorithm::CreateFunc), 16);
// 	m_boxSphereCF = new (mem) btSphereBoxCollisionAlgorithm::CreateFunc;
// 	m_boxSphereCF->m_swapped = true;
// #endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM

mem = btAlignedAlloc(sizeof(btSphereTriangleCollisionAlgorithm::CreateFunc), 16);
	m_sphereTriangleCF = new (mem) btSphereTriangleCollisionAlgorithm::CreateFunc;
	mem = btAlignedAlloc(sizeof(btSphereTriangleCollisionAlgorithm::CreateFunc), 16);
	m_triangleSphereCF = new (mem) btSphereTriangleCollisionAlgorithm::CreateFunc;
	m_triangleSphereCF->m_swapped = true;

	mem = btAlignedAlloc(sizeof(btBoxBoxCollisionAlgorithm::CreateFunc), 16);
	m_boxBoxCF = new (mem) btBoxBoxCollisionAlgorithm::CreateFunc;
	printf("Start1\n");

	//convex versus plane
	mem = btAlignedAlloc(sizeof(btConvexPlaneCollisionAlgorithm::CreateFunc), 16);
	m_convexPlaneCF = new (mem) btConvexPlaneCollisionAlgorithm::CreateFunc;
	mem = btAlignedAlloc(sizeof(btConvexPlaneCollisionAlgorithm::CreateFunc), 16);
	m_planeConvexCF = new (mem) btConvexPlaneCollisionAlgorithm::CreateFunc;
	m_planeConvexCF->m_swapped = true;

	///calculate maximum element size, big enough to fit any collision algorithm in the memory pool
	int maxSize = sizeof(btConvexConvexAlgorithm);
	int maxSize2 = sizeof(btConvexConcaveCollisionAlgorithm);
	int maxSize3 = sizeof(btCompoundCollisionAlgorithm);
	int maxSize4 = sizeof(btCompoundCompoundCollisionAlgorithm);

	int collisionAlgorithmMaxElementSize = btMax(maxSize, constructionInfo.m_customCollisionAlgorithmMaxElementSize);
	collisionAlgorithmMaxElementSize = btMax(collisionAlgorithmMaxElementSize, maxSize2);
	collisionAlgorithmMaxElementSize = btMax(collisionAlgorithmMaxElementSize, maxSize3);
	collisionAlgorithmMaxElementSize = btMax(collisionAlgorithmMaxElementSize, maxSize4);
	printf("Start2\n");
#endif

	int m_persistentManifoldPoolSize;

	btPoolAllocator* m_persistentManifoldPool;
	bool m_ownsPersistentManifoldPool;

	btPoolAllocator* m_collisionAlgorithmPool;
	bool m_ownsCollisionAlgorithmPool;

	if (constructionInfo.m_persistentManifoldPool)
	{
		m_ownsPersistentManifoldPool = false;
		m_persistentManifoldPool = constructionInfo.m_persistentManifoldPool;
	}
	else
	{
		m_ownsPersistentManifoldPool = true;
	printf("Start21\n");
		void* mem = btAlignedAlloc(sizeof(btPoolAllocator), 16);
	int tp = sizeof(btPersistentManifold);
	int sz = constructionInfo.m_defaultMaxPersistentManifoldPoolSize;
	printf("Start22 %d %d\n", tp, sz);
	//here problem
		m_persistentManifoldPool = new (mem) btPoolAllocator(sizeof(btPersistentManifold), constructionInfo.m_defaultMaxPersistentManifoldPoolSize);
	}
	printf("Start23\n");

	// collisionAlgorithmMaxElementSize = (collisionAlgorithmMaxElementSize + 16) & 0xffffffffffff0;
	// if (constructionInfo.m_collisionAlgorithmPool)
	// {
	// 	m_ownsCollisionAlgorithmPool = false;
	// 	m_collisionAlgorithmPool = constructionInfo.m_collisionAlgorithmPool;
	// }
	// else
	// {
	// 	m_ownsCollisionAlgorithmPool = true;
	// 	void* mem = btAlignedAlloc(sizeof(btPoolAllocator), 16);
	// 	m_collisionAlgorithmPool = new (mem) btPoolAllocator(collisionAlgorithmMaxElementSize, constructionInfo.m_defaultMaxCollisionAlgorithmPoolSize);
	// }

	printf("Start3\n");

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	printf("Start4\n");

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

/*	
///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
		///-----initialization_end-----

	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	///create a few basic rigid bodies

	//the ground is a cube of side 100 at position y = -56.
	//the sphere will hit it at y = -6, with center at -5
	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -56, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(2, 10, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	/// Do some simulation

	///-----stepsimulation_start-----
	for (i = 0; i < 150; i++)
	{
		dynamicsWorld->stepSimulation(1.f / 60.f, 10);

		//print positions of all objects
		for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
				body->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
			}
			printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
		}
	}

	///-----stepsimulation_end-----

	//cleanup in the reverse order of creation/initialization

	///-----cleanup_start-----

	//remove the rigidbodies from the dynamics world and delete them
	for (i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynamicsWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
	*/
	return 0;
}