#include "GraphicElementManager.hpp"

#include "BFC/BFCCullableHandle.hpp"
#include "BFC/BFCBlockManagerFactory.hpp"

#include "Graphic/DRBMeshData.hpp"

#include "AssetManagement/Instance/MeshInstance.hh"
#include "AssetManagement/Instance/MaterialInstance.hh"
#include "Utils/Profiler.hpp"


namespace AGE
{
	GraphicElementManager::GraphicElementManager(BFCBlockManagerFactory *factory)
		: _bfcBlockManager(factory)
	{
		AGE_ASSERT(factory != nullptr);
	}

	BFCCullableHandleGroup GraphicElementManager::addMesh(std::shared_ptr<MeshInstance> mesh, std::shared_ptr<MaterialSetInstance> materialInstance)
	{
		SCOPE_profile_cpu_function("DRB");

		BFCCullableHandleGroup result;

		for (auto &submesh : mesh->subMeshs)
		{
			DRBMesh *drbMesh = nullptr;
			
			if (submesh.isSkinned)
			{
				auto drbMeshSkeleton = _skinnedMeshPool.create();
				drbMesh = drbMeshSkeleton;
			}
			else
			{
				drbMesh = _meshPool.create();
			}


			drbMesh->datas->setVerticesKey(submesh.vertices);
			drbMesh->datas->setPainterKey(submesh.painter);
			drbMesh->datas->setAABB(submesh.boundingBox);

			std::size_t materialIndex = submesh.defaultMaterialIndex < materialInstance->datas.size() ? submesh.defaultMaterialIndex : 0;
			auto &material = materialInstance->datas[materialIndex];

			// we set the material unique id
			drbMesh->material = &material;

			BFCCullableHandle resultMesh = _bfcBlockManager->createItem(drbMesh);
			result.getHandles().push_back(resultMesh);
		}
		return result;
	}

	void GraphicElementManager::removeMesh(BFCCullableHandleGroup &handle)
	{
		SCOPE_profile_cpu_function("DRB");

		for (auto &m : handle.getHandles())
		{
			if (m.getPtr<DRBMesh>()->getDatas()->hadRenderMode(RenderModes::AGE_SKINNED))
			{
				_skinnedMeshPool.destroy(m.getPtr());
			}
			else
			{
				_meshPool.destroy(m.getPtr());
			}
			_bfcBlockManager->deleteItem(m);
		}
		handle.getHandles().clear();
	}
}