#include "Barcode.h"
#include <Engine/Entities/EntityData.hh>

class EntityData;

Barcode::Barcode()
: code_(0)
{
	code_.clear();
}

Barcode::Barcode(EntityData &entity)
{
	code_ = entity.getCode().code_;
}

Barcode::Barcode(Barcode &&o)
{
	code_ = std::move(o.code_);
}

Barcode::Barcode(Barcode &other)
{
	code_ = other.code_;
}

Barcode &Barcode::operator=(const Barcode &other)
{
	code_ = other.code_;
	return *this;
}

Barcode::~Barcode()
{}

bool Barcode::match(boost::dynamic_bitset<> &set)
{
	if (code_.size() != set.size())
	{
		if (code_.size() < set.size())
			code_.resize(set.size());
		else
			set.resize(code_.size());
	}
	return code_.is_subset_of(set);
}

bool Barcode::match(EntityData &entity)
{
	return match(entity.getCode().code_);
}

bool Barcode::match(Barcode &other)
{
	return match(other.code_);
}

void Barcode::reset()
{
	code_.reset();
}

void Barcode::add(std::size_t componentId)
{
	applyChange(componentId, true);
}

void Barcode::remove(std::size_t componentId)
{
	applyChange(componentId, false);
}

bool Barcode::isEmpty() const
{
	return (code_.none());
}

bool Barcode::isSet(std::size_t componentId) const
{
	return (code_.size() > componentId && code_.test(componentId));
}