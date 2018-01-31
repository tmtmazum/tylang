#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "common/TyObject.h"

namespace ty
{

class Definition;

//! Table of symbols for a given scope
class SymbolTable : public TyObject<Attribute::HasGlobal>
{
public: // member virtual

	//! Binds a symbol to a definition.
	//! \pre	'name' must not already be in the SymbolTable
	void add_definition(std::string name, std::unique_ptr<Definition> definition)
	{
		CCT_CHECK(definition_at(name) == nullptr);
		
		using pair_t = std::decay_t<decltype(m_definitions)::value_type>;

		m_definitions.emplace(pair_t{ std::move(name), std::move(definition)});
	}

	//! Binds a symbol to a construct
	template <typename T, typename... Args>
	void add(std::string name, Args&&... args)
	{
		if (std::is_base_of<Definition, T>::value)
		{
			add_definition(std::move(name), std::make_unique<T>(std::forward<Args>(args)...));
		}
		else
		{
			std::abort();
		}
	}

	//! Search for the definition with 'name' in this SymbolList
	//!		\returns	pointer to definition, if found
	//!					nullptr, otherwise
	virtual Definition* definition_at(std::string const& name)
	{
		auto const it = m_definitions.find(name);
		return it != m_definitions.end() ? it->second.get() : nullptr;
	}

private:	
	std::unordered_map<std::string, std::unique_ptr<Definition>>	m_definitions;
};

//! List of symbols chosen for export to outside of the module
class ExportList : public TyObject<Attribute::HasGlobal>, public std::vector<std::string>
{
public:
	template <typename... Args>
	explicit ExportList(Args&&... args)
		: std::vector<std::string>{std::forward<Args>(args)...}
	{}

};

} // namespace ty

