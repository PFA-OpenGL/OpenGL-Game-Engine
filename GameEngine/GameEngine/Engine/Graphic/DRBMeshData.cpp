#include "DRBMeshData.hpp"

#include "Render\GeometryManagement\Painting\Painter.hh"
#include "Render\GeometryManagement\Data\Vertices.hh"

namespace AGE
{

	DRBMeshData::DRBMeshData()
		: DRBData()
	{
	}

	DRBMeshData::~DRBMeshData()
	{
	}

	void DRBMeshData::setPainterKey(const Key<Painter> &key)
	{
		RWLockGuard(_lock, true);
		_painter = key;
	}

	void DRBMeshData::setVerticesKey(const Key<Vertices> &key)
	{
		RWLockGuard(_lock, true);
		_vertices = key;
	}

	void DRBMeshData::setRenderMode(RenderModes mode, bool activate)
	{
		RWLockGuard(_lock, true);
		_renderMode[mode] = activate;
	}

	void DRBMeshData::setRenderModes(const RenderModeSet &modes)
	{
		RWLockGuard(_lock, true);
		_renderMode = modes;
	}

	void DRBMeshData::setAABB(const AABoundingBox &box)
	{
		RWLockGuard(_lock, true);
		_boundingBox = box;
	}

	const Key<Painter> DRBMeshData::getPainterKey() const
	{
		RWLockGuard(_lock, false);
		return _painter;
	}

	const Key<Vertices> DRBMeshData::getVerticesKey() const
	{
		RWLockGuard(_lock, false);
		return _vertices;
	}

	bool DRBMeshData::hadRenderMode(RenderModes mode) const
	{
		RWLockGuard(_lock, false);
		return _renderMode.test(mode);
	}

	AABoundingBox DRBMeshData::getAABB() const
	{
		RWLockGuard(_lock, false);
		return _boundingBox;
	}
}