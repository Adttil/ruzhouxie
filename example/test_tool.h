#ifndef RUZHOUXIE_TEST_TOOL_H
#define RUZHOUXIE_TEST_TOOL_H

#include <iostream>

#include <magic/visualize.h>

inline constexpr auto split_line = "=============================================================================\n";

//inline static auto l = std::locale::global(std::locale{ ".UTF-8" });

template<typename RT1, typename RT2, typename T1, typename T2>
inline void magic_assert_impl(const char* file, size_t line, const char* exp_str, const char* exc_str, const T1& expression, const T2& excepted)
{
    if constexpr (not requires{ requires requires{ expression == excepted; }; })
	{
	    std::cout << split_line << file << ":" << line << '\n';
	    std::cout << '"' << exp_str << " == " << exc_str << "\" is invalid.\n";
	    std::cout << "type of [" << exp_str << "] is:\n" <<
		    magic::visualize<RT1>({ false }) << '\n';
	    std::cout << "type of [" << exc_str << "] is:\n" <<
		    magic::visualize<RT2>({ false }) << '\n';
	}
    else
	{
	    if (not(expression == excepted))
		{
		    std::cout << split_line << file << ":" << line << '\n';
		    if constexpr (requires{ std::cout << expression; })
			{
			    std::cout << exp_str << "\n    is\n" << expression << "\n";
			}
		    else
			{
			    std::cout << exp_str << "\n    is not equal to\n" << exc_str << "\n";
			}
		}
	}
}

template<typename T1, typename T2>
inline void magic_type_check_impl(const char* file, size_t line, const char* t1_str, const char* t2_str)
{
    if constexpr (not std::same_as<T1, T2>)
	{
	    std::cout << split_line << file << ":" << line << '\n';
	    std::cout << "type check wrong.\n";
	    std::cout << "type [" << t1_str << "] is:\n" << magic::visualize<T1>({ false }) << '\n';
	    std::cout << "type [" << t2_str << "] is:\n" << magic::visualize<T2>({ false }) << '\n';
	}
}

#define MAGIC_CHECK(expression, excepted) \
magic_assert_impl<decltype(expression), decltype(excepted)>(__FILE__, __LINE__, #expression, #excepted, expression, excepted);

#define MAGIC_TYPE_CHECK(type1, type2) \
magic_type_check_impl<type1, type2>(__FILE__, __LINE__, #type1, #type2);

#define MAGIC_SHOW_TYPE(...) std::cout << "type of [" #__VA_ARGS__ << "] is\n" << magic::visualize<decltype(__VA_ARGS__)>({false}) << '\n';

#endif