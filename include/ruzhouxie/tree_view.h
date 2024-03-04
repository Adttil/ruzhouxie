#ifndef RUZHOUXIE_TREE_VIEW_H
#define RUZHOUXIE_TREE_VIEW_H

#include "pipe_closure.h"
#include "tape.h"
#include "get.h"
#include "processer.h"
#include "macro_define.h"

//view_base
namespace ruzhouxie
{
	template<typename T>
	struct view;

	namespace detail
	{
		template<typename View>
		struct universal_view
		{
			View raw_view;

			template<typename U>
			constexpr operator U(this auto&& self) 
				requires requires{ FWD(self, raw_view) | make_tree<U>; }
			{
				return FWD(self, raw_view) | make_tree<U>;
			}
		};

		template<typename View>
		struct view_base
		{
			template<typename Self>
			constexpr auto operator+(this Self&& self)
			{
				return universal_view<Self&&>{ FWD(self) };
			}
		};
	}
}

//view
namespace ruzhouxie
{
	template<typename T>
	struct view : wrapper<T>, detail::view_base<view<T>>
	{
		template<size_t I, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | child<I>)

		template<auto Seq, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | get_tape<Seq>)
		// template<specified<view> Self>
		// RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
		// 	AS_EXPRESSION(id_tree<T>())
	};

	template<typename T>
	view(T&&) -> view<T>;

	template<typename T>
	struct tree_maker_trait<view<T>>
	{
		struct type1// : processer<type1>
		{
			static constexpr tree_maker<T> maker{};

			template<typename U>
			static consteval auto get_sequence()
			{
				return maker.template get_sequence<U>();
			}

			template<typename U, typename Tape>
			constexpr auto process_tape(Tape&& tape)const
			{
				return view<T>{ maker.template process_tape<U>(FWD(tape)) };
			}

			template<typename Tree>
			RUZHOUXIE_INLINE constexpr auto operator()(Tree&& tree)const
			{
				return process_tape<Tree&&>(FWD(tree) | get_tape<get_sequence<Tree&&>()>);
			}
			//AS_EXPRESSION(process_tape<Tree&&>(FWD(tree) | get_tape<get_sequence<Tree&&>()>))
		};

		using type = type1;
	};

	namespace detail
	{
		struct as_ref_t
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (std::is_rvalue_reference_v<T&&>)
				{
					return view<T&&>{ FWD(t) };
				}
				else
				{
					return t;
				}
			}
		};
	};

	inline namespace functors
	{
		inline constexpr tree_adaptor_closure<detail::as_ref_t> as_ref{};
	}
}

#include "macro_undef.h"
#endif