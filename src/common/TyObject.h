#pragma once

#include <cinttypes>
#include <type_traits>

namespace ty { namespace Attribute
{

//! List of static attributes for TyObjects
enum TyObjectAttribute
{
	HasGlobal = 0x0001
};


} // namespace Attribute

//! Base object for all types in the tylang libraries
class TyObjectBase { };

//! Object that all tylang objects must derive from
template <uint64_t attributes = 0>
class TyObject : public TyObjectBase
{
public:
	//! Returns true if this type has a top-level global instance
	constexpr static auto has_global() { return attributes & Attribute::HasGlobal; }
};

//! Returns a reference to the top-level global instance of a TyObject
template <typename T>
static T& Global()
{
	static_assert(std::is_base_of<TyObjectBase, T>::value, "T must derive TyObject");
	static_assert(T::has_global(), "T is not defined to have a global instance");

	static T object;
	return object;
}

} // namespace ty::Attribute

#define TY_ASSERTF(cond, format, ...) if(!(cond)) {\
  fprintf(stderr, "[%s][%s][%d] Assertion Failed: '" #cond "'\n" format "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
  std::abort(); \
}
