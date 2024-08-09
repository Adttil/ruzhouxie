#ifndef RUZHOUXIE_ADAPTOR_CLOSURE_HPP
#define RUZHOUXIE_ADAPTOR_CLOSURE_HPP

#include<type_traits>
#include<concepts>

#include "general.hpp"

#include "macro_define.hpp"

namespace ruzhouxie
{
    template<class F> 
    struct adaptor_closure{};

    namespace detail
    {
        template <class F>
        F* derived_from_adaptor_closure(adaptor_closure<F>&); // not defined
    }

    template <class T>
    concept adaptor_closuroid = requires(std::remove_cvref_t<T>& t) 
    {
        { detail::derived_from_adaptor_closure(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };
}

namespace ruzhouxie::detail
{
    template<typename ClosureLeft, typename ClosureRight>
    struct pipeline : adaptor_closure<pipeline<ClosureLeft, ClosureRight>>
    {
        RUZHOUXIE_MAYBE_EMPTY ClosureLeft  left;
        RUZHOUXIE_MAYBE_EMPTY ClosureRight right;

        template<typename T, typename Self>
        constexpr auto operator()(this Self&& self, T&& val)
        AS_EXPRESSION(
            FWD(self, right)(FWD(self, left)(FWD(val)))
        )

        friend constexpr bool operator==(const pipeline&, const pipeline&) = default; 
    };
}

namespace ruzhouxie
{
    template<adaptor_closuroid L, adaptor_closuroid R>
    constexpr auto operator|(L&& l, R&& r)
    AS_EXPRESSION(
        detail::pipeline<std::decay_t<L>, std::decay_t<R>>{ {}, FWD(l), FWD(r) }
    )

    template<class L, adaptor_closuroid R>
    requires (not adaptor_closuroid<L>)
    constexpr auto operator|(L&& l, R&& r)
    AS_EXPRESSION(
        FWD(r)(FWD(l))
    )
}

#include "macro_undef.hpp"
#endif