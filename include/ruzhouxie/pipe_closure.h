#ifndef RUZHOUXIE_PIPE_CLOSURE_H
#define RUZHOUXIE_PIPE_CLOSURE_H

#include "general.h"
#include "macro_define.h"

namespace ruzhouxie
{
	namespace detail::pipe_closure_ns
	{
		template<typename Fn, size_t NArgsMin = 1uz> struct pipe_closure;
		template<typename Fn> pipe_closure(Fn) -> pipe_closure<std::decay_t<Fn>>;
	}
	using detail::pipe_closure_ns::pipe_closure;

	template<size_t NArgsMin = 1uz>
	RUZHOUXIE_INLINE constexpr auto make_pipe_closure(auto&& fn)
	{
		return pipe_closure<purified<decltype(fn)>, NArgsMin>{ FWD(fn) };
	}

	template<typename Fn, size_t NArgsMin>
	struct detail::pipe_closure_ns::pipe_closure : Fn
	{
		using fn_type = Fn;
		static constexpr size_t n_args_min = NArgsMin;
		using Fn::operator();

		/*template<taggedable T, specified<pipe_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self)
			AS_EXPRESSION(as_base<Fn>(FWD(self))(view{  FWD(t) })*/


		template<typename T, specified<pipe_closure> Self>
		requires (not requires{ requires std::same_as<pipe_closure<typename purified<T>::fn_type, purified<T>::n_args_min>, purified<T>>; })
		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self) 
			AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(t)))

		template<typename Pipe, specified<pipe_closure> Self>
		RUZHOUXIE_INLINE friend constexpr auto operator|(Pipe&& t, Self&& self)
			requires std::same_as<pipe_closure<typename purified<Pipe>::fn_type, purified<Pipe>::n_args_min>, purified<Pipe>>
		{
			return pipe_closure_ns::pipe_closure
			{
				[&](auto&& arg) AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(arg) | FWD(t)))
			};
		}

		RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this auto&& self, auto&&...args)noexcept
			requires (not requires{ as_base<Fn>(FWD(self))(FWD(args)...); }) && (NArgsMin > sizeof...(args))
		{
			return pipe_closure_ns::pipe_closure
			{
				[&] (auto&&...appended_args)
					AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(args)..., FWD(appended_args)...))
				/*[&] (this auto&& new_fn, auto&&...appended_args)
					AS_EXPRESSION(as_base<Fn>(FWDLIKE(new_fn, self))(FWDLIKE(new_fn, args)..., FWD(appended_args)...))*/
			};
		}
	};

	template<auto fn>
	using tag_t = purified<decltype(fn)>;
}

#include "macro_undef.h"
#endif