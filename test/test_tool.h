#ifndef RUZHOUXIE_TEST_TOOL_H
#define RUZHOUXIE_TEST_TOOL_H

#include <gtest/gtest.h>
#include <magic/visualize.h>

namespace ruzhouxie::test
{
    constexpr ::magic::VisualizeOption magic_option
    {
        .utf_support = false 
    };

    template<typename T1, typename T2>
    inline void magic_check_impl(const char* exp1_str, const char* exp2_str, const T1& exp1, const T2& exp2)
    {
        constexpr bool equal_valid = requires{ requires requires{ exp1 == exp2; }; };
        GTEST_ASSERT_TRUE(equal_valid)
    		<< '"' << exp1_str << " == " << exp2_str << "\" is invalid.\n"
    		<< "Type of [" << exp1_str << "] is:\n" << ::magic::visualize<T1>(::ruzhouxie::test::magic_option) << '\n'
    	    << "Type of [" << exp2_str << "] is:\n" << ::magic::visualize<T2>(::ruzhouxie::test::magic_option) << '\n';

    	if constexpr(equal_valid)
    	{
            GTEST_ASSERT_(::testing::internal::EqHelper::Compare(exp1_str, exp2_str, exp1, exp2), GTEST_FATAL_FAILURE_);
    	}
    }
}

#define MAGIC_TCHECK(type1, ...) \
GTEST_ASSERT_TRUE((std::same_as<type1, __VA_ARGS__>)) \
    << "Type [" << #type1 << "] is:\n" << magic::visualize<type1>(::ruzhouxie::test::magic_option) << '\n'\
    << "Type [" << #__VA_ARGS__ << "] is:\n" << magic::visualize<__VA_ARGS__>(::ruzhouxie::test::magic_option) << '\n';

#define MAGIC_CHECK(exp1, ...) \
::ruzhouxie::test::magic_check_impl<decltype(exp1), decltype(__VA_ARGS__)>(#exp1, #__VA_ARGS__, exp1, __VA_ARGS__);

#endif