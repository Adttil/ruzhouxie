#ifndef RUZHOUXIE_PIPE_CLOSURE_H
#define RUZHOUXIE_PIPE_CLOSURE_H

#include "general.h"

#include "macro_define.h"

namespace ruzhouxie
{
	namespace detail::tree_adaptor_closure_ns
	{
		template<typename Fn> struct tree_adaptor_closure;
		template<typename Fn> tree_adaptor_closure(Fn) -> tree_adaptor_closure<std::decay_t<Fn>>;
	}
	using detail::tree_adaptor_closure_ns::tree_adaptor_closure;

	template<typename T>
	concept tree_adaptor_closuroid = std::same_as<tree_adaptor_closure<typename purified<T>::fn_type>, purified<T>>;

	template<typename Fn>
	struct detail::tree_adaptor_closure_ns::tree_adaptor_closure : Fn
	{
		using fn_type = Fn;
		using Fn::operator();

		template<typename V, specified<tree_adaptor_closure> Self> requires (not tree_adaptor_closuroid<V>)
		RUZHOUXIE_INLINE friend constexpr auto operator|(V&& view, Self&& self) 
			AS_EXPRESSION(rzx::as_base<Fn>(FWD(self))(FWD(view)))

		template<tree_adaptor_closuroid C, specified<tree_adaptor_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(C&& closure, Self&& self) noexcept
		{
			return tree_adaptor_closure_ns::tree_adaptor_closure
			{
				[&](auto&& arg) AS_EXPRESSION(rzx::as_base<Fn>(FWD(self))(FWD(arg) | FWD(closure)))
			};
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
				[fn = rzx::as_base<Fn>(FWD(self)), ...args_ = FWD(args)](this auto&& self, auto&& view)
					//noexcept(noexcept(std::declval<Fn>()(FWD(view), FWD(args)...)))//clang bug.
					->decltype(auto)
					requires requires{ std::declval<Fn>()(FWD(view), FWDLIKE(self, args)...); }
				{
					return fn(FWD(view), FWDLIKE(self, args_)...);
				}
					//AS_EXPRESSION(Fn{}(FWD(view), FWDLIKE(self, args)...))
			};
		}
	};
}

#include "macro_undef.h"
#endif