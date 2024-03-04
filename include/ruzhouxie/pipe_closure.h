#ifndef RUZHOUXIE_PIPE_CLOSURE_H
#define RUZHOUXIE_PIPE_CLOSURE_H

#include "general.h"
#include "macro_define.h"
#include <concepts>

namespace ruzhouxie
{
	namespace detail::pipe_closure_ns
	{
		template<typename Fn, size_t NArgsMin = 1uz> struct pipe_closure;

		

		
	}
	using detail::pipe_closure_ns::pipe_closure;
	
	template<typename Fn, size_t NArgsMin = 1uz>
	struct pipe_wrapper : Fn, pipe_closure<pipe_wrapper<Fn, NArgsMin>>
	{
		using Fn::operator();
	};
	
	template<typename Fn> pipe_wrapper(Fn) -> pipe_wrapper<std::decay_t<Fn>>;
	
	// template<size_t NArgsMin = 1uz>
	// RUZHOUXIE_INLINE constexpr auto make_pipe_closure(auto&& fn)
	// {
	// 	return pipe_closure<purified<decltype(fn)>, NArgsMin>{ FWD(fn) };
	// }

	template<typename Fn, size_t NArgsMin>
	struct detail::pipe_closure_ns::pipe_closure
	{
		using fn_type = Fn;
		static constexpr size_t n_args_min = NArgsMin;
		//using Fn::operator();

		/*template<taggedable T, specified<pipe_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self)
			AS_EXPRESSION(as_base<Fn>(FWD(self))(view{  FWD(t) })*/

		template<typename T, specified<Fn> Self>
		requires (not requires{ requires std::derived_from<purified<T>, pipe_closure<purified<T>>>; })
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self) 
			AS_EXPRESSION(FWD(self)(FWD(t)))

		template<typename Pipe, specified<Fn> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(Pipe&& t, Self&& self)
			requires std::derived_from<purified<Pipe>, pipe_closure<purified<Pipe>>>
		{
			return pipe_wrapper
			{
				[&](auto&& arg) AS_EXPRESSION(FWD(self)(FWD(arg) | FWD(t)))
			};
		}

		RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this specified<Fn> auto&& self, auto&&...args)noexcept
			requires //(not requires{ FWD(self)(FWD(args)...); }) && 
			(NArgsMin > sizeof...(args))
		{
			return pipe_wrapper
			{
				[&] (auto&&...appended_args)
					AS_EXPRESSION(FWD(self)(FWD(args)..., FWD(appended_args)...))
				/*[&] (this auto&& new_fn, auto&&...appended_args)
					AS_EXPRESSION(FWDLIKE(new_fn, self)(FWDLIKE(new_fn, args)..., FWD(appended_args)...))*/
			};
		}
	};

	template<auto fn>
	using tag_t = purified<decltype(fn)>;
}

#include "macro_undef.h"
#endif