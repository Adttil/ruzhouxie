#ifndef RUZHOUXIE_RELAYOUT_SEPERATE_HPP
#define RUZHOUXIE_RELAYOUT_SEPERATE_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"

#include "macro_define.hpp"

namespace rzx::detail
{
    template<auto Layout, class S>
    constexpr auto seperate_simplify_layout(S shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(rzx::array_cat(Layout, array{ I })...);
            }(std::make_index_sequence<child_count<child_type<S, Layout>>>{});
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(detail::simplify_layout<Layout | child<I>, S>()...);
        }(std::make_index_sequence<child_count<decltype(Layout)>>{});
    }
}

namespace rzx 
{
    namespace detail::relayout_seperate_t_ns
    {
        template<auto Layout, bool Sequential>
        struct relayout_seperate_t;
        
        template<auto Layout, bool Sequential>
        void relayout_seperate();

        template<class T, auto SimplifiedLayout = detail::seperate_simplify_layout<indexes_of_whole>(tree_shape<T>)>
        constexpr bool is_simple()
        {
            constexpr bool no_custom = not bool
            {
                requires{ std::declval<T>().template relayout_seperate<SimplifiedLayout, false>(custom_t{}); }
                ||
                requires{ relayout_seperate<SimplifiedLayout, false>(std::declval<T>(), custom_t{}); }
                ||
                requires{ std::declval<T>().template relayout_seperate<SimplifiedLayout, false>(); }
                ||
                requires{ relayout_seperate<SimplifiedLayout, false>(std::declval<T>()); }
            };
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return (no_custom && ... && is_simple<child_type<T, I>>());
            }(std::make_index_sequence<child_count<T>>{});
        }
    }

    template<auto Layout, bool Sequential>
    inline constexpr detail::relayout_seperate_t_ns::relayout_seperate_t<Layout, Sequential> relayout_seperate{};

    template<class T>
    concept simple = detail::relayout_seperate_t_ns::is_simple<T>();
}



#include "macro_undef.hpp"
#endif