
#include <Core/LooseOctreeNode.hh>
#include <Core/LooseOctree.hh>
#include <Core/CullableObjects.hh>

namespace AGE
{
	LooseOctreeNode::LooseOctreeNode() :
		_node(glm::vec3(-1), glm::vec3(1))
	{
		_father = UNDEFINED_IDX;
		for (uint32_t i = 0; i < 8; ++i)
			_sons[i] = UNDEFINED_IDX;
		_uniqueSubElements = 0;
		_nbrElements = 0;
		_firstElement = UNDEFINED_IDX;
		computeLooseNode();
	}

	LooseOctreeNode::~LooseOctreeNode()
	{
	}

	uint32_t LooseOctreeNode::addElement(POOL_NODE_ARGS, CullableBoundingBox *toAdd)
	{
		CREATE_THIS_PTR
		glm::i8vec3	direction;
		bool collisionState;
		glm::vec3 objectDimensions;

		collisionState = thisPtr->_node.checkPointIn(toAdd->currentAABB.center);
		direction = thisPtr->_node.getDirection(toAdd->currentAABB.center);
		objectDimensions = toAdd->currentAABB.maxPoint - toAdd->currentAABB.minPoint;
		// if the center is in the loose node
		if (collisionState == true)
		{
			glm::vec3 nodeSize = thisPtr->_node.maxPoint - thisPtr->_node.minPoint;
			glm::vec3 halfNodeSize = nodeSize / 2.0f;

			if (glm::all(glm::greaterThan(nodeSize, objectDimensions)))
			{
				// if the object is entirely in the node
				++thisPtr->_uniqueSubElements;
				if (glm::all(glm::greaterThan(halfNodeSize, objectDimensions)))
				{
					// if the object can fit in the lower level of the tree
					uint32_t sonIdx = (direction.x == 1 ? 4 : 0) +
						(direction.y == 1 ? 2 : 0) +
						(direction.z == 1 ? 1 : 0);

					if (thisPtr->_sons[sonIdx] == UNDEFINED_IDX || thisPtr->_sons[sonIdx] == LEAF_NODE_IDX)
					{
						generateSon(thisIdx, manager, sonIdx);
						UPDATE_THIS_PTR
					}
					addElementRecursive(thisPtr->_sons[sonIdx], manager, toAdd);
				}
				else
				{
					// else we put the object in this node
					toAdd->currentNode = thisIdx;
					thisPtr->addElementToList(manager, toAdd);
				}
				return (thisIdx);
			}
		}
		if (thisPtr->_father == UNDEFINED_IDX)
		{
			return (extendNode(thisIdx, manager, toAdd, direction));
		}
		return (UNDEFINED_IDX);
	}

	void LooseOctreeNode::addElementRecursive(POOL_NODE_ARGS, CullableBoundingBox *toAdd)
	{
		CREATE_THIS_PTR
		glm::i8vec3	direction = thisPtr->_node.getDirection(toAdd->currentAABB.center);
		glm::vec3 objectDimensions = toAdd->currentAABB.maxPoint - toAdd->currentAABB.minPoint;

		glm::vec3 nodeSize = thisPtr->_node.maxPoint - thisPtr->_node.minPoint;
		glm::vec3 halfNodeSize = nodeSize / 2.0f;

		++thisPtr->_uniqueSubElements;
		if (glm::all(glm::greaterThan(halfNodeSize, objectDimensions)))
		{
			// if the object can fit in the lower level of the tree
			uint32_t sonIdx = (direction.x == 1 ? 4 : 0) +
				(direction.y == 1 ? 2 : 0) +
				(direction.z == 1 ? 1 : 0);

			if (thisPtr->_sons[sonIdx] == UNDEFINED_IDX || thisPtr->_sons[sonIdx] == LEAF_NODE_IDX)
			{
				generateSon(thisIdx, manager, sonIdx);
				UPDATE_THIS_PTR
			}
			addElementRecursive(thisPtr->_sons[sonIdx], manager, toAdd);
		}
		else
		{
			// else we put the object in this node
			toAdd->currentNode = thisIdx;
			thisPtr->addElementToList(manager, toAdd);
		}
	}

	void		LooseOctreeNode::getElementsCollide(LooseOctree &manager, CullableFrustum *toTest, AGE::Vector<CullableObject*> &toFill) const
	{
		assert(_uniqueSubElements != 0);
		bool collisionState;

		collisionState = toTest->currentFrustum.checkCollision(_looseNode);
		if (collisionState)
		{
			uint32_t curElementIdx = _firstElement;
			SOctreeElement *curElement;
			CullableObject *curObject;

			while (curElementIdx != UNDEFINED_IDX)
			{
				curElement = &manager.getElementPool().get(curElementIdx);
				curObject = manager.getElementFromPool(curElement->object);
				if (toTest->checkCollision(curObject))
					toFill.push_back(curObject);
				curElementIdx = curElement->next;
			}
			if (!isLeaf())
			{
				for (uint32_t i = 0; i < 8; ++i)
				{
					assert(_sons[i] != UNDEFINED_IDX);
					if (_sons[i] != LEAF_NODE_IDX)
						manager.getNodePool().get(_sons[i]).getElementsCollide(manager, toTest, toFill);
				}
			}
		}
	}

	bool		LooseOctreeNode::isLeaf() const
	{
		// If a node is not a leaf, all his sons are created
		return (_sons[0] == UNDEFINED_IDX);
	}

	uint32_t LooseOctreeNode::tryChangeRoot(POOL_NODE_ARGS)
	{
		CREATE_THIS_PTR
		// if the node is the root, has no elements and has sons, we try to change it
		if (thisPtr->_father == UNDEFINED_IDX && !thisPtr->isLeaf() && thisPtr->_nbrElements == 0)
		{
			// if the node is the root, try to remove it
			uint32_t nbrSonUsed = 0;
			uint32_t sonUsedIdx;

			// if only one of its sons is used
			for (uint32_t i = 0; i < 8; ++i)
			{
				if (thisPtr->_sons[i] != LEAF_NODE_IDX &&
					manager.getNodePool().get(thisPtr->_sons[i])._uniqueSubElements != 0)
				{
					++nbrSonUsed;
					sonUsedIdx = i;
					if (nbrSonUsed > 1)
						break;
				}
			}
			if (nbrSonUsed == 1)
			{
				// replace the root by this son
				uint32_t newRoot = thisPtr->_sons[sonUsedIdx];

				thisPtr->_sons[sonUsedIdx] = UNDEFINED_IDX;
				removeAllSons(thisIdx, manager);
				manager.getNodePool().dealloc(thisIdx);
				manager.getNodePool().get(newRoot)._father = UNDEFINED_IDX;
				return (newRoot);
			}
		}
		return (UNDEFINED_IDX);
	}

	uint32_t LooseOctreeNode::extendNode(POOL_NODE_ARGS, CullableBoundingBox *toAdd, glm::i8vec3 const &direction)
	{
		uint32_t newRootIdx = manager.getNodePool().alloc();
		LooseOctreeNode	*newRoot = &manager.getNodePool().get(newRootIdx);
		// here we create thisPtr after the pool resize
		CREATE_THIS_PTR
		glm::vec3	nodeSize = thisPtr->_node.maxPoint - thisPtr->_node.minPoint;

		if (direction.x == -1)
		{
			newRoot->_node.minPoint.x = thisPtr->_node.minPoint.x - nodeSize.x;
			newRoot->_node.maxPoint.x = thisPtr->_node.maxPoint.x;
		}
		else
		{
			newRoot->_node.minPoint.x = thisPtr->_node.minPoint.x;
			newRoot->_node.maxPoint.x = thisPtr->_node.maxPoint.x + nodeSize.x;
		}
		if (direction.y == -1)
		{
			newRoot->_node.minPoint.y = thisPtr->_node.minPoint.y - nodeSize.y;
			newRoot->_node.maxPoint.y = thisPtr->_node.maxPoint.y;
		}
		else
		{
			newRoot->_node.minPoint.y = thisPtr->_node.minPoint.y;
			newRoot->_node.maxPoint.y = thisPtr->_node.maxPoint.y + nodeSize.y;
		}
		if (direction.z == -1)
		{
			newRoot->_node.minPoint.z = thisPtr->_node.minPoint.z - nodeSize.z;
			newRoot->_node.maxPoint.z = thisPtr->_node.maxPoint.z;
		}
		else
		{
			newRoot->_node.minPoint.z = thisPtr->_node.minPoint.z;
			newRoot->_node.maxPoint.z = thisPtr->_node.maxPoint.z + nodeSize.z;
		}
		newRoot->_node.recomputeCenter();
		newRoot->computeLooseNode();
		newRoot->_uniqueSubElements = thisPtr->_uniqueSubElements;

		if (thisPtr->_uniqueSubElements == 0)
		{
			removeNode(thisIdx, manager);
			newRoot = &manager.getNodePool().get(newRootIdx);
		}
		else
		{
			thisPtr->_father = newRootIdx;
			newRoot->_sons[(direction.x == -1 ? 4 : 0) +
				(direction.y == -1 ? 2 : 0) +
				(direction.z == -1 ? 1 : 0)] = thisIdx;
			for (int i = 0; i < 8; ++i)
			{
				if (newRoot->_sons[i] == UNDEFINED_IDX)
					newRoot->_sons[i] = LEAF_NODE_IDX;
			}
		}

		return (addElement(newRootIdx, manager, toAdd));
	}

	void		LooseOctreeNode::generateSon(POOL_NODE_ARGS, uint32_t toGenIdx)
	{
		glm::u8vec3	currentSon((toGenIdx & 4) ? 1 : 0, (toGenIdx & 2) ? 1 : 0, toGenIdx & 1);
		uint32_t newSonIdx = manager.getNodePool().alloc();
		LooseOctreeNode	*newSon = &manager.getNodePool().get(newSonIdx);
		// here we create thisPtr after the pool resize
		CREATE_THIS_PTR
		glm::vec3	nodeHalfSize = (thisPtr->_node.maxPoint - thisPtr->_node.minPoint) / 2.0f;

		if (thisPtr->_sons[toGenIdx] == UNDEFINED_IDX)
		{
			for (int i = 0; i < 8; ++i)
				thisPtr->_sons[i] = LEAF_NODE_IDX;
		}
		assert(newSonIdx != UNDEFINED_IDX);
		newSon->_father = thisIdx;
		newSon->_node.minPoint = thisPtr->_node.minPoint + glm::vec3(currentSon) * nodeHalfSize;
		newSon->_node.maxPoint = thisPtr->_node.maxPoint - glm::vec3(currentSon.x == 0 ? 1 : 0,
			currentSon.y == 0 ? 1 : 0,
			currentSon.z == 0 ? 1 : 0) * nodeHalfSize;
		newSon->_node.recomputeCenter();
		newSon->computeLooseNode();
		thisPtr->_sons[toGenIdx] = newSonIdx;
	}

	void		LooseOctreeNode::computeLooseNode()
	{
		glm::vec3 dimensions = _node.maxPoint - _node.minPoint;
		_looseNode.minPoint = _node.minPoint - (dimensions / 2.0f);
		_looseNode.maxPoint = _node.maxPoint + (dimensions / 2.0f);
		_looseNode.recomputeCenter();
	}

	void LooseOctreeNode::removeElementFromNode(POOL_NODE_ARGS, CullableBoundingBox *toRm)
	{
		CREATE_THIS_PTR
		// remove the element from the node
		thisPtr->removeElementFromList(manager, toRm);

		uint32_t toCleanIdx = UNDEFINED_IDX;

		for (uint32_t currentIdx = thisIdx; currentIdx != UNDEFINED_IDX; currentIdx = manager.getNodePool().get(currentIdx)._father)
		{
			LooseOctreeNode *current = &manager.getNodePool().get(currentIdx);

			--current->_uniqueSubElements;
			if (current->_uniqueSubElements == 0)
				toCleanIdx = currentIdx;
		}
		if (toCleanIdx != UNDEFINED_IDX)
			removeNode(toCleanIdx, manager);
		toRm->currentNode = UNDEFINED_IDX;
	}

	uint32_t LooseOctreeNode::moveElementFromNode(POOL_NODE_ARGS, CullableBoundingBox *toMv)
	{
		CREATE_THIS_PTR
		glm::vec3 nodeSize = thisPtr->_node.maxPoint - thisPtr->_node.minPoint;
		glm::vec3 halfNodeSize = nodeSize / 2.0f;

		bool currentCollisionState;
		glm::vec3 currentObjectDimensions;

		currentCollisionState = thisPtr->_node.checkPointIn(toMv->currentAABB.center);
		currentObjectDimensions = toMv->currentAABB.maxPoint - toMv->currentAABB.minPoint;

		if (currentCollisionState &&
			glm::all(glm::greaterThan(nodeSize, currentObjectDimensions)))
		{
			if (glm::all(glm::greaterThan(halfNodeSize, currentObjectDimensions)))
			{
				glm::i8vec3	direction = thisPtr->_node.getDirection(toMv->currentAABB.center);
				uint32_t sonIdx = (direction.x == 1 ? 4 : 0) +
								(direction.y == 1 ? 2 : 0) +
								(direction.z == 1 ? 1 : 0);
				// remove element from node
				thisPtr->removeElementFromList(manager, toMv);

				if (thisPtr->_sons[sonIdx] == UNDEFINED_IDX || thisPtr->_sons[sonIdx] == LEAF_NODE_IDX)
				{
					generateSon(thisIdx, manager, sonIdx);
					UPDATE_THIS_PTR
				}
				addElementRecursive(thisPtr->_sons[sonIdx], manager, toMv);
			}
			// The object is still contained in the same node
			return (UNDEFINED_IDX);
		}
		// The object has changed node
		--thisPtr->_uniqueSubElements;

		// remove the element from the node
		thisPtr->removeElementFromList(manager, toMv);

		uint32_t toCleanIdx = (thisPtr->_uniqueSubElements == 0) ? thisIdx : UNDEFINED_IDX;

		uint32_t prevNodeIdx = thisIdx;

		uint32_t currentNodeIdx = thisPtr->_father;

		while (currentNodeIdx != UNDEFINED_IDX)
		{
			LooseOctreeNode *currentNode = &manager.getNodePool().get(currentNodeIdx);
			LooseOctreeNode *prevNode = &manager.getNodePool().get(prevNodeIdx);

			glm::vec3 nodeSize = currentNode->_node.maxPoint - currentNode->_node.minPoint;

			currentCollisionState = currentNode->_node.checkPointIn(toMv->currentAABB.center);
			currentObjectDimensions = toMv->currentAABB.maxPoint - toMv->currentAABB.minPoint;

			--currentNode->_uniqueSubElements;

			if (currentCollisionState &&
				glm::all(glm::greaterThan(nodeSize, currentObjectDimensions)))
			{
				if (toCleanIdx != UNDEFINED_IDX)
					removeNode(toCleanIdx, manager);
				addElementRecursive(currentNodeIdx, manager, toMv);
				return (UNDEFINED_IDX);
			}
			else if (currentNode->_uniqueSubElements == 0)
			{
				toCleanIdx = currentNodeIdx;
			}

			prevNodeIdx = currentNodeIdx;
			currentNodeIdx = currentNode->_father;
		}
		if (toCleanIdx != UNDEFINED_IDX)
			removeNode(toCleanIdx, manager);
		return (addElement(prevNodeIdx, manager, toMv));
	}

	void LooseOctreeNode::removeAllSons(POOL_NODE_ARGS)
	{
		CREATE_THIS_PTR
		for (uint32_t i = 0; i < 8; ++i)
		{
			if (thisPtr->_sons[i] != UNDEFINED_IDX)
			{
				if (thisPtr->_sons[i] != LEAF_NODE_IDX)
				{
					removeAllSons(thisPtr->_sons[i], manager);
					UPDATE_THIS_PTR
					manager.getNodePool().dealloc(thisPtr->_sons[i]);
					UPDATE_THIS_PTR
				}
				thisPtr->_sons[i] = UNDEFINED_IDX;
			}
		}
	}

	void LooseOctreeNode::removeNode(POOL_NODE_ARGS)
	{
		CREATE_THIS_PTR

		removeAllSons(thisIdx, manager);
		UPDATE_THIS_PTR
		if (thisPtr->_father != UNDEFINED_IDX)
		{
			uint32_t idx = 0;
			LooseOctreeNode *father = &manager.getNodePool().get(thisPtr->_father);

			while (father->_sons[idx] != thisIdx)
				++idx;
			father->_sons[idx] = LEAF_NODE_IDX;
		}
		manager.getNodePool().dealloc(thisIdx);
	}

	void LooseOctreeNode::addElementToList(LooseOctree &manager, CullableObject *toAdd)
	{
		++_nbrElements;
		uint32_t elementIdx = manager.getElementPool().alloc();
		SOctreeElement &added = manager.getElementPool().get(elementIdx);

		added.object = toAdd->key;
		added.next = _firstElement;
		if (_firstElement != UNDEFINED_IDX)
			manager.getElementPool().get(_firstElement).prev = elementIdx;
		added.prev = UNDEFINED_IDX;
		_firstElement = elementIdx;
		toAdd->currentElementInNode = elementIdx;
	}

	void LooseOctreeNode::removeElementFromList(LooseOctree &manager, CullableObject *toRm)
	{
		--_nbrElements;
		SOctreeElement &removed = manager.getElementPool().get(toRm->currentElementInNode);
		if (removed.prev != UNDEFINED_IDX)
			manager.getElementPool().get(removed.prev).next = removed.next;
		else
			_firstElement = removed.next;
		if (removed.next != UNDEFINED_IDX)
			manager.getElementPool().get(removed.next).prev = removed.prev;
		manager.getElementPool().dealloc(toRm->currentElementInNode);
		toRm->currentElementInNode = UNDEFINED_IDX;
	}

	void LooseOctreeNode::checkOctreeIntegrity(LooseOctree &manager) const
	{
		assert(_uniqueSubElements != 0);
		if (!isLeaf())
		{
			for (uint32_t i = 0; i < 8; ++i)
			{
				assert(_sons[i] != UNDEFINED_IDX);
				if (_sons[i] != LEAF_NODE_IDX)
					manager.getNodePool().get(_sons[i]).checkOctreeIntegrity(manager);
			}
		}
	}
}