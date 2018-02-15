#include "parse/SymbolTable.h"
#include "parse/Parse.h"
#include "cgen/LLVM_IR_Generator.h"
#include "token/TokenList.h"
#include "gtest/gtest.h"

TEST(tokenize, t0)
{
    using namespace ty;

    auto t = tokenize(R"(foo = @() -> {5123215})");
    EXPECT_TRUE(!t.empty());
    EXPECT_EQ(10, t.size());
}

TEST(tokenize, t1)
{
    using namespace ty;

    auto t = tokenize(R"(foo = @() {ping()})");
    EXPECT_TRUE(!t.empty());
    EXPECT_EQ(11, t.size());
}

TEST(tokenize, bad0)
{
    using namespace ty;

    ASSERT_THROW(tokenize(R"(foo = @() ? {ping()})"), TokenException);
    //ASSERT_THROW(tokenize(R"(foo = {() {ping()})"), TokenException);
}

TEST(tokenize, export1)
{
    using namespace ty;

    auto t = tokenize(R"(export(foo))");
    ASSERT_TRUE(!t.empty());
    EXPECT_EQ(5, t.size());
    EXPECT_STREQ(LexItem::as_string(LexItem::Type::EXPORT), LexItem::as_string(t.front().type));
    EXPECT_EQ(LexItem::Type::PAREN_OPEN, t[1].type);
    EXPECT_EQ(LexItem::Type::ID, t[2].type);
    EXPECT_EQ(LexItem::Type::PAREN_CLOSE, t[3].type);
}

TEST(parse, export1)
{
    using namespace ty;

    auto t = tokenize(R"(export(foo))");
    auto ast = parse(t);
    EXPECT_TRUE(std::any_of(begin(ast.export_list), end(ast.export_list), [](auto i) { return i == "foo"; }));
}

TEST(parse, single_function_call)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(five = @() -> {5})"));
}

TEST(parse, nested_function_calls)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(export(foo3)
        foo = @() -> { 5 }
        foo2 = @() -> { foo() + 5 }
        foo3 = @() -> { foo() + foo() }
        foo4 = @() -> { foo() + 5 + foo() }
        foo5 = @() -> { foo() + foo() + 4 + 5421 + foo() + foo  ( 4) }
        )"));

    EXPECT_TRUE(!!ast.symbols.expr_at("foo"));
    EXPECT_TRUE(!!ast.symbols.expr_at("foo2"));
    EXPECT_TRUE(!!ast.symbols.expr_at("foo3"));
}
//
//TEST(parse, func_returning_int)
//{
//    using namespace ty;
//
//    auto ast = parse(tokenize(R"(
//        five = @() -> { square(2) }
//)"));
//    ast.print(cct::unique_file{ stdout });
//
//    ASSERT_TRUE(!!ast.symbols.expr_at("five"));
//    auto a = ast.symbols.expr_at("five");
//    auto as_func_def = dynamic_cast<FunctionDefnExpr*>(a);
//    ASSERT_TRUE(!!as_func_def);
//    ASSERT_EQ(1, as_func_def->m_returns.size());
//    auto as_func_call = dynamic_cast<FunctionCallExpr*>(as_func_def->m_returns.front()->child());
//    ASSERT_TRUE(!!as_func_call);
//    EXPECT_EQ(1, as_func_call->m_arguments.size());
//    auto as_int = dynamic_cast<Int32LiteralExpr*>(as_func_call->m_arguments.front().get());
//    ASSERT_TRUE(!!as_int);
//    EXPECT_STREQ("2", as_int->value_as_string().c_str());
//}
//
TEST(generate, foo1)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(foo = @() -> {5} 

    export(foo))"));

    ty::LLVM_IR_Generator g{ cct::unique_file{stdout} };
    for (auto const& export_id : ast.export_list)
    {
        if (auto const* expr = ast.symbols.expr_at(export_id))
        {
            expr->generate(g);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
        }
    }
}

TEST(generate, foo2)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(
    export(foo, goo)
        foo = @() -> {5} 
        goo = @() -> { 5 + 5 }
    )"));

    ty::LLVM_IR_Generator g{ cct::unique_file{stdout} };
    for (auto const& export_id : ast.export_list)
    {
        if (auto const* expr = ast.symbols.expr_at(export_id))
        {
            expr->generate(g);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
        }
    }
}

TEST(generate, func_wrapper)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(
    export(five, five_wrapped)
        five = @() -> {5} 
        five_wrapped = @() -> { five() }
    )"));

    ast.print(cct::unique_file{ stdout });

    ty::LLVM_IR_Generator g{ cct::unique_file{stdout} };
    for (auto const& export_id : ast.export_list)
    {
        if (auto const* expr = ast.symbols.expr_at(export_id))
        {
            expr->generate(g);
        }
        else
        {
            fprintf(stderr, "Cannot find symbol '%s' for export", export_id.c_str());
        }
    }
}



TEST(parse, t0)
{
    using namespace ty;

    auto ast = parse(tokenize(R"(foo = @() -> {5123215})"));
    EXPECT_TRUE(!ast.exprs.empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto a = RUN_ALL_TESTS();
    __debugbreak();
    return a;
}