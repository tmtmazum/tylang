#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <atomic>
#include "common/TyObject.h"

namespace ty
{

class SymbolTable;
class Expr;

template <typename T>
class Referencable
{
    using move_safe_ptr_t = std::atomic<T const*>;
    using move_safe_mutable_ptr_t = std::atomic<T*>;
    
    std::unique_ptr<move_safe_ptr_t>  m_original;

    struct Ref
    {
        move_safe_ptr_t*         m_owner;

        auto operator->() const  { return m_owner->load(); }
    };

public:
    using ref_t = Ref;

    explicit Referencable(T const* t)
        : m_original{ std::make_unique<move_safe_ptr_t>(t) }
    {}

    Referencable(Referencable&& other)
        : m_original{std::move(other.m_original)}
    {
        *m_original = static_cast<T const*>(this);
        TY_ASSERTF(m_original->load(), "Invalid dynamic_cast");
    }

    //! Creates a move-safe reference through indirection
    auto create_reference()
    {
        return Ref{ m_original.get() };
    }
};

//! Table of symbols for a given scope
class SymbolTable : public TyObject<Attribute::HasGlobal>, public Referencable<SymbolTable>
{
public: // member virtual

	explicit SymbolTable(SymbolTable const* parent = nullptr)
		: Referencable(this)
        , m_parent{ parent } 
    {}

	//! Binds a symbol to a Expr.
	//! \pre	'name' must not already be in the SymbolTable
	void add_expr(std::string name, Expr* expr)
	{
		TY_ASSERTF(expr_at(name) == nullptr, "name:%s", name.c_str());
		
		using pair_t = std::decay_t<decltype(m_definitions)::value_type>;

		m_definitions.emplace(pair_t{ std::move(name), std::move(expr)});
	}

    void set_parent(SymbolTable const* parent)
    {
        m_parent = parent;
    }

	//! Search for the definition with 'name' in this SymbolList
	//!		\returns	pointer to definition, if found
	//!					nullptr, otherwise
	virtual Expr* expr_at(std::string const& name) const
	{
		auto const it = m_definitions.find(name);
		if (it != m_definitions.end())
		{
			return it->second;
		}
		return m_parent ? m_parent->expr_at(name) : nullptr;
	}

	auto count() const { return m_definitions.size(); }

private:	
    std::unordered_map<std::string, Expr*>	                        m_definitions;
	
    SymbolTable const*												m_parent = nullptr;
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

