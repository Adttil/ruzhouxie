#ifndef RUZHOUXIE_ADAPTOR_CLOSURE_HPP
#define RUZHOUXIE_ADAPTOR_CLOSURE_HPP

#include<type_traits>
#include<concepts>

#include "general.hpp"
#include "tuple.hpp"

#include "macro_define.hpp"

namespace rzx
{
    template<class F> 
    struct adaptor_closure{};

    namespace detail
    {
        template<class Adaptor, class...Args>
        struct closure : adaptor_closure<closure<Adaptor, Args...>>
        {
            tuple<Args...> captures;

            template<typename T, size_t...I>
            constexpr auto impl(T&& t, std::index_sequence<I...>)const
            AS_EXPRESSION(
                Adaptor{}(FWD(t), get<I>(captures)...)
            )

            template<typename T>
            constexpr auto operator()(T&& t)const
            AS_EXPRESSION(
                impl(FWD(t), std::index_sequence_for<Args...>{})
            )
        };
    }

    template<class F> 
    struct adaptor
    {
        template<typename...Args>
        constexpr auto operator()(Args&&...args)const
        AS_EXPRESSION(
            F{}.result(FWD(args)...)
        )

        template<typename...Args> //requires (not requires{ F{}.result(std::declval<Args>()...); })
        constexpr auto operator()(Args&&...args)const
            //requires (not requires{ F{}.result(FWD(args)...); })
        {
            return detail::closure<F, std::decay_t<Args>...>{ {}, FWD(args)... };
        }
    };
}

namespace rzx::detail
{        
    template<class T>
    concept adaptor_closuroid = requires(std::remove_cvref_t<T>& t) 
    {
        { []<class F>(adaptor_closure<F>&)->F*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    template<class ClosureLeft, class ClosureRight>
    struct pipeline : adaptor_closure<pipeline<ClosureLeft, ClosureRight>>
    {
        RUZHOUXIE(no_unique_address) ClosureLeft  left;
        RUZHOUXIE(no_unique_address) ClosureRight right;

        template<class T, class Self>
        constexpr auto operator()(this Self&& self, T&& val)
        AS_EXPRESSION(
            FWD(self, right)(FWD(self, left)(FWD(val)))
        )

        friend constexpr bool operator==(const pipeline&, const pipeline&) = default; 
    };
}

namespace rzx
{
    template<detail::adaptor_closuroid L, detail::adaptor_closuroid R>
    constexpr auto operator|(L&& l, R&& r)
    AS_EXPRESSION(
        detail::pipeline<std::decay_t<L>, std::decay_t<R>>{ {}, FWD(l), FWD(r) }
    )

    template<class L, detail::adaptor_closuroid R>
    requires (not detail::adaptor_closuroid<L>)
    constexpr auto operator|(L&& l, R&& r)
    AS_EXPRESSION(
        FWD(r)(FWD(l))
    )
}

#include "macro_undef.hpp"
#endif