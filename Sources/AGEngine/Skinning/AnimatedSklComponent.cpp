#include "AnimatedSklComponent.hpp"
#include "Skeleton.hpp"

#include "assetmanagement/assetmanager.hh"

#include "Entity/Entity.hh"
#include "Entity/EntityData.hh"
#include "Core/AScene.hh"

namespace AGE
{
	AnimatedSklComponent::AnimatedSklComponent()
		: _skeletonAsset(nullptr)
	{}

	AnimatedSklComponent::~AnimatedSklComponent(void)
	{}

	void AnimatedSklComponent::_copyFrom(const ComponentBase *model)
	{}

	void AnimatedSklComponent::init(const std::string &skeletonPath /*= ""*/, std::shared_ptr<Skeleton> skeletonAsset /*= nullptr*/)
	{
		if (skeletonAsset != nullptr)
			_skeletonAsset = skeletonAsset;
		else if (skeletonPath.empty() == false)
			_loadAndSetSkeleton(skeletonPath);
	}

	void AnimatedSklComponent::reset()
	{
		_skeletonAsset = nullptr;
		_skeletonFilePath.clear();
	}

	void AnimatedSklComponent::setAnimation(const std::string &animationPath)
	{
		if (animationPath.empty() == false)
		{
			_loadAndSetAnimation(animationPath);
		}
	}

	void AnimatedSklComponent::setAnimation(std::shared_ptr<AnimationData> animationAssetPtr)
	{
		AGE_ASSERT(animationAssetPtr != nullptr);
		// if there is already an animation
		if (_animationAsset != nullptr)
		{
			AGE_ASSERT(false && "TODO");
		}
		_animationAsset = animationAssetPtr;
		_setAnimation();
	}

	void AnimatedSklComponent::_loadAndSetSkeleton(const std::string &skeletonPath)
	{
		AGE_ASSERT(skeletonPath.empty() == false);

		_skeletonFilePath = skeletonPath;
		auto assetsManager = entity->getScene()->getInstance<AssetsManager>();
		AGE_ASSERT(assetsManager != nullptr);
		_skeletonAsset = assetsManager->getSkeleton(skeletonPath);
		if (_skeletonAsset == nullptr)
		{
			assetsManager->pushNewCallback(skeletonPath, entity->getScene(),
				std::function<void()>([=](){
				_skeletonAsset = assetsManager->getSkeleton(skeletonPath);
			}));
			assetsManager->loadSkeleton(skeletonPath);
		}
	}

	void AnimatedSklComponent::_loadAndSetAnimation(const std::string &animationPath)
	{
		AGE_ASSERT(animationPath.empty() == false);

		_animationFilePath = animationPath;
		auto assetsManager = entity->getScene()->getInstance<AssetsManager>();
		AGE_ASSERT(assetsManager != nullptr);

		_animationAsset = assetsManager->getAnimation(animationPath);
		if (_animationAsset == nullptr)
		{
			assetsManager->pushNewCallback(animationPath, entity->getScene(),
				std::function<void()>([=](){
				this->_setAnimation();
			}));
			assetsManager->loadAnimation(animationPath);
		}
		else
		{
			_setAnimation();
		}
	}

	void AnimatedSklComponent::_setAnimation()
	{

	}

#ifdef EDITOR_ENABLED
	bool AnimatedSklComponent::editorUpdate()
	{ return true;}
#endif
}