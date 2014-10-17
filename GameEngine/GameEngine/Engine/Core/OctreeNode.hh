#pragma once

#include <Utils/AABoundingBox.hh>
#include <Utils/Containers/Vector.hpp>
#include <Core/CullableInterfaces.hh>

// tmp value waiting for bounding box computation
#define		MAX_ELEMENT_PER_NODE	4096

namespace AGE
{
	class	OctreeNode
	{
	public:
		OctreeNode();
		~OctreeNode();

		OctreeNode	*addElement(CullableObject *toAdd);
		OctreeNode	*removeElement(CullableObject *toRm, bool useCurrentPos = true);
		OctreeNode	*moveElement(CullableObject *toMove);

		void		getElementsCollide(CullableObject *toTest, AGE::Vector<CullableObject*> &toFill) const;

		AABoundingBox const &getNodeBoundingBox() const;
		bool				isLeaf() const;

	private:
		// Utils methods
		void		splitNode();
		OctreeNode *extendNode(CullableObject *toAdd, glm::i8vec3 const &direction);
		void		generateAllSons();

		OctreeNode		*_father;
		OctreeNode		*_sons[8];

		AABoundingBox	_node;

		uint32_t		_elementsNbr;
		CullableObject	*_elements[MAX_ELEMENT_PER_NODE];
	};
}