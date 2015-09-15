#pragma once

#include <vector>
#include <memory>
#include <Render/Properties/IProperty.hh>
#include <Render/Key.hh>
#include <Utils/RWLock.hpp>

namespace AGE
{

	class Properties
	{
	public:
		Properties() = default;
		Properties(std::vector<std::shared_ptr<IProperty>> const &properties);
		Properties(Properties &&other);
		Properties(Properties const &);
		Properties &operator=(Properties const &) = delete;
		virtual ~Properties();

	public:
		Key<Property> add_property(std::shared_ptr<IProperty> const &prop);
		void remove_property(Key<Property> const &prop);
		void update_property(IProgramResources *p, std::size_t index) const;
		void update_instancied_property(IProgramResources *p, std::size_t index) const;
		void merge_properties(const Properties &other);
		std::size_t getProgramId(std::size_t programId);
		inline std::size_t getHash() { if (_hashToRefresh){ _computeHash(); }; return _shaderHash; }
		void setProgramId(std::size_t programUniqueId, std::size_t givenId);
		bool empty() const { return _properties.empty(); }
	public:
		template <typename type_t> std::shared_ptr<type_t> get_property(Key<Property> const &key) const;

		std::size_t getPropertyIndex(const std::string &name) const
		{
			RWLockGuard lock(_lock, false);
			std::size_t res = 0;
			for (auto &e : _properties)
			{
				if (e->name() == name)
				{
					return res;
				}
				++res;
			}
			return -1;
		}

	private:
		void _computeHash();

		std::vector<std::shared_ptr<IProperty>> _properties;
		std::vector<std::size_t> _shadersGivenId;
		std::size_t _shaderHash;
		bool        _hashToRefresh;
		mutable RWLock _lock;
	};

	template <typename type_t>
	std::shared_ptr<type_t> Properties::get_property(Key<Property> const &key) const
	{
		RWLockGuard lock(_lock, false);
		return (std::static_pointer_cast<type_t>(_properties[key.getId()]));
	}
}