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
    ASSERT_THROW(tokenize(R"(foo = {() {ping()})"), TokenException);
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