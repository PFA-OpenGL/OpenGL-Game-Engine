#include <Render/Properties/AProperty.hh>
#include <Render/Program.hh>

namespace AGE
{
	AProperty::AProperty(std::string &&name) :
		_name(std::move(name))
	{

	}

	AProperty::AProperty(AProperty &&move) :
		_name(std::move(move._name)),
		_resources(std::move(move._resources))
	{

	}

	std::string const & AProperty::name() const
	{
		return (_name);
	}

	std::shared_ptr<IProgramResources> AProperty::get_resource(std::shared_ptr<Program> const &p)
	{
		for (auto &resource : _resources) {
			if (*resource.first == *p) {
				return (resource.second);
			}
		}
		return (nullptr);
	}

	IProperty & AProperty::set_program(std::vector<std::shared_ptr<Program>> const &programs)
	{
		for (auto &program : programs) {
			auto &resource = program->get_resource_interface(_name);
			if (resource) {
				_resources.emplace_back(std::make_pair(program, resource));
			}
		}
		return (*this);
	}
}