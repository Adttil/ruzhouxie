#ifndef RUZHOUXIE_PIPE_CLOSURE_H
#define RUZHOUXIE_PIPE_CLOSURE_H

#include "general.h"
#include "tuple.h"

#include "macro_define.h"

namespace ruzhouxie::detail
{
    template<typename F, typename...Args>
    struct bind_suffix_t
    {
        F fn;
        tuple<Args...> args;

        template<typename V, size_t...I, typename Self>
        RUZHOUXIE_INLINE constexpr auto impl(this Self&& self, V&& view, std::index_sequence<I...>) AS_EXPRESSION
        (
            FWD(self, fn)(FWD(view), FWD(self, args).template get<I>()...)
        )

        template<typename V, typename Self>
        RUZHOUXIE_INLINE constexpr auto operator()(this Self&& self, V&& view)
            noexcept(noexcept(FWD(self).impl(FWD(view), std::index_sequence_for<Args...>{})))
            ->decltype(auto)
            //Here must use "std::declval<V>()" in MSVC.
            requires requires{ std::declval<Self>().impl(std::declval<V>(), std::index_sequence_for<Args...>{});}
        {
            return FWD(self).impl(FWD(view), std::index_sequence_for<Args...>{});
        }
    };

    template<typename F, typename...Args>
    bind_suffix_t(F&&, Args&&...) -> bind_suffix_t<F, Args...>;
}

namespace ruzhouxie
{
    template<typename Fn> struct tree_adaptor_closure;
    template<typename Fn> tree_adaptor_closure(Fn) -> tree_adaptor_closure<std::decay_t<Fn>>;

    template<typename T>
    concept tree_adaptor_closuroid = std::same_as<tree_adaptor_closure<typename purified<T>::fn_type>, purified<T>>;

    template<typename Fn>
    struct tree_adaptor_closure : Fn
    {
        using fn_type = Fn;
        using Fn::operator();

        template<typename V, specified<tree_adaptor_closure> Self> requires (not tree_adaptor_closuroid<V>)
        RUZHOUXIE_INLINE friend constexpr auto operator|(V&& view, Self&& self) AS_EXPRESSION
        (
            rzx::as_base<Fn>(FWD(self))(FWD(view))
        )

        template<tree_adaptor_closuroid C, specified<tree_adaptor_closure> Self>
        RUZHOUXIE_INLINE friend constexpr auto operator|(C&& closure, Self&& self) noexcept
        {
            return rzx::tree_adaptor_closure{detail::bind_suffix_t
            {
                [] RUZHOUXIE_INLINE_LAMBDA (auto&& arg, auto&& closure1, auto&& closure2) AS_EXPRESSION
                (
                    FWD(arg) | FWD(closure1) | FWD(closure2)
                ),
                FWD(closure),
                FWD(self)
            }};
        }

        RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this auto&& self)noexcept
        {
            return FWD(self);
        }
    };
}

namespace ruzhouxie
{
    template<typename Fn>
    struct tree_adaptor : Fn
    {
        using Fn::operator();

        template<typename Self, typename...Args>
        RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this Self&& self, Args&&...args) noexcept
            requires (not requires{ rzx::as_base<Fn>(FWD(self))(FWD(args)...); })
        {
            return tree_adaptor_closure
            {
                //Requires on lambda have too many different behaviors in different compilers.
                detail::bind_suffix_t{ rzx::as_base<Fn>(FWD(self)), FWD(args)... }
            };
        }
    };

    template<typename Fn>
    tree_adaptor(Fn&&) -> tree_adaptor<std::decay_t<Fn>>;
}

#include "macro_undef.h"
#endif