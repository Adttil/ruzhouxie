#ifndef RUZHOUXIE_SIMPLIFY_HPP
#define RUZHOUXIE_SIMPLIFY_HPP

#include "child.hpp"
#include "general.hpp"

#include "macro_define.hpp"


namespace rzx 
{  
    enum class usage_t
    {
        discard,
        once,
        repeatedly
    };

    namespace detail::simplify_t_ns
    {
        template<auto UsageTable, auto Layout>
        struct simplify_t;
        
        template<auto UsageTable, auto Layout>
        void simplify();
    }

    template<auto UsageTable = usage_t::repeatedly, auto Layout = indexes_of_whole>
    inline constexpr detail::simplify_t_ns::simplify_t<UsageTable, Layout> simplify{};

    template<auto UsageTable, auto Layout>
    struct detail::simplify_t_ns::simplify_t : adaptor_closure<simplify_t<UsageTable, Layout>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr(requires{ FWD(t).template simplify<UsageTable, Layout>(custom_t{}); })
            {
                return FWD(t).template simplify<UsageTable, Layout>(custom_t{});
            }
            else if constexpr(requires{ simplify<UsageTable, Layout>(FWD(t), custom_t{}); })
            {
                return simplify<UsageTable, Layout>(FWD(t), custom_t{});
            }
            else if constexpr(requires{ FWD(t).template simplify<UsageTable, Layout>(); })
            {
                return FWD(t).template simplify<UsageTable, Layout>();
            }
            else if constexpr(requires{ simplify<UsageTable, Layout>(FWD(t)); })
            {
                return simplify<UsageTable, Layout>(FWD(t));
            }
            else
            {
                return FWD(t);
            }
        }
    };

    template<typename V>
    concept simple = std::same_as<decltype(std::declval<V>() | simplify<>), V&&>;
}

#include "macro_undef.hpp"
#endif