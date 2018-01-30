#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "common/TyObject.h"

namespace ty
{

class Definition;

class SymbolTable : public TyObject<Attribute::HasGlobal>
{
public: // member virtual
	//virtual ISymbolInfo* symbol_at(char const* name);

	//virtual bool is_global_symbol_table() { return this == std::addressof(Global<SymbolTable>()); }

	template <typename T, typename... Args>
	void bind_new(std::string name, Args&&... args)
	{
		CCT_CHECK(definition_at(name) == nullptr);
		
		using pair_t = std::decay_t<decltype(m_definitions)::value_type>;
		//using mapped_data_t = std::decay_t<decltype(m_definitions)::mapped_type::element_type>;

		m_definitions.emplace(pair_t{ std::move(name), std::make_unique<T>(std::forward<Args>(args)...) });
	}
	
	virtual Definition* definition_at(std::string const& name)
	{
		auto const it = m_definitions.find(name);
		return it != m_definitions.end() ? it->second.get() : nullptr;
	}

private:	
	std::unordered_map<std::string, std::unique_ptr<Definition>>	m_definitions;
};

/* 
 * "x : int"
 *
 * g_symbols.add_declaration("x", create_value(T_INT));
 *
 * "x = int{5}"
 * g_symbols.add_definition("x", create_value(T_INT, "5"));
 *
 * "export(x)"
 * g_export_list().add_symbol("x");
 *
 * for(auto const& export : g_export_list())
 * {
 *	if(auto d = g_symbols().try_definition_at(export))
 *	{
 *		generate(d);
 *	}
 *	else
 *	{
 *		throw CannotExportUndefinedSymbol;
 *	}
 * }
*/

class ExportList : public TyObject<Attribute::HasGlobal>, public std::vector<std::string>
{
public:
	template <typename... Args>
	ExportList(Args&&... args)
		: std::vector<std::string>{std::forward<Args>(args)...}
	{}

};

/*! Usage:
 *
 * auto x = begin_expression();
 * x.set_loperand(T_INT, 2);
 * x.set_operation("add");
 * x.set_roperand(T_INT, 1);
 * assert(x.is_legal());
 * x.get_resulting_type();
 * auto s = begin_variable();
 * 
 *
 * auto f = begin_function();
 * f.add_parameter("a", T_ANY_TYPE);
 * f.set_return(T_ANY_TYPE, make_expression([](expr const& e)
 *	{
 *		e.set_loperand(f.symbol_at("a"));
 *		e.set_operation("add");
 *		e.set_roperand(T_ANYTYPE, 1);
 *		return e.finalize();
 *	});
 * g_symbols.add_symbol("foo", std::move(f));
 *
 * for(auto const& export_name : g_export_list)
 * {
 *   auto s = g_symbols.symbol_at(export_name);
 *   
 *
 * }
 */
 } // namespace ty

