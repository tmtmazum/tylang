#pragma once

#include <cinttypes>
#include <type_traits>

namespace ty { namespace Attribute
{

enum TyObjectFlags
{
	HasGlobal = 0x0001
};


} // namespace Attribute

class TyObjectBase { };

template <uint64_t attributes>
class TyObject : public TyObjectBase
{
public:
	constexpr static auto has_global() { return attributes & Attribute::HasGlobal; }
};

template <typename T>
static T& Global()
{
	static_assert(std::is_base_of<TyObjectBase, T>::value, "T must derive TyObject");
	static_assert(T::has_global(), "T is not defined to have a global instance");

	static T object;
	return object;
}

} // namespace ty::Attribute
