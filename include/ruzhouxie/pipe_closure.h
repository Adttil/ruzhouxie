#ifndef RUZHOUXIE_PIPE_CLOSURE_H
#define RUZHOUXIE_PIPE_CLOSURE_H

#include "general.h"
#include "macro_define.h"
#include "ruzhouxie/general.h"
#include "ruzhouxie/macro_define.h"

namespace ruzhouxie
{
	namespace detail::tree_adaptor_closure_ns
	{
		template<typename Fn> struct tree_adaptor_closure;
		template<typename Fn> tree_adaptor_closure(Fn) -> tree_adaptor_closure<std::decay_t<Fn>>;
	}
	using detail::tree_adaptor_closure_ns::tree_adaptor_closure;

	template<typename Fn>
	struct detail::tree_adaptor_closure_ns::tree_adaptor_closure : Fn
	{
		using fn_type = Fn;
		using Fn::operator();

		/*template<taggedable T, specified<tree_adaptor_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self)
			AS_EXPRESSION(as_base<Fn>(FWD(self))(view{  FWD(t) })*/


		template<typename T, specified<tree_adaptor_closure> Self>
		requires (not requires{ requires std::same_as<tree_adaptor_closure<typename purified<T>::fn_type>, purified<T>>; })
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self) 
			AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(t)))

		template<typename Pipe, specified<tree_adaptor_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(Pipe&& t, Self&& self) noexcept
			requires std::same_as<tree_adaptor_closure<typename purified<Pipe>::fn_type>, purified<Pipe>>
		{
			return tree_adaptor_closure_ns::tree_adaptor_closure
			{
				[&](auto&& arg) AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(arg) | FWD(t)))
			};
		}

		RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this auto&& self)noexcept
		{
			return FWD(self);
		}

		// RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this auto&& self, auto&&...args)noexcept
		// 	requires (not requires{ as_base<Fn>(FWD(self))(FWD(args)...); }) && (NArgsMin > sizeof...(args))
		// {
		// 	return tree_adaptor_closure_ns::tree_adaptor_closure
		// 	{
		// 		[&] (auto&&...appended_args)
		// 			AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(args)..., FWD(appended_args)...))
		// 		/*[&] (this auto&& new_fn, auto&&...appended_args)
		// 			AS_EXPRESSION(as_base<Fn>(FWDLIKE(new_fn, self))(FWDLIKE(new_fn, args)..., FWD(appended_args)...))*/
		// 	};
		// }
	};



	template<auto fn>
	using tag_t = purified<decltype(fn)>;
}

namespace ruzhouxie
{
	template<typename Fn>
	struct tree_adaptor : Fn
	{
		using Fn::operator();

		template<typename Self, typename...Args>
		RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this Self&& self, Args&&...args) noexcept
			requires (not requires{ as_base<Fn>(FWD(self))(FWD(args)...); })
		{
			return tree_adaptor_closure
			{
				[fn = as_base<Fn>(FWD(self)), ...args_ = FWD(args)](this auto&& self, auto&& view)
					//noexcept(noexcept(declval<Fn>()(FWD(view), FWD(args)...)))//clang bug.
					->decltype(auto)
					requires requires{ declval<Fn>()(FWD(view), FWDLIKE(self, args)...); }
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