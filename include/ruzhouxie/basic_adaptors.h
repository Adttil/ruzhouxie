#ifndef RUZHOUXIE_BASIC_ADAPTORS_H
#define RUZHOUXIE_BASIC_ADAPTORS_H

#include "array.h"
#include "general.h"
#include "tape.h"
#include "get.h"
#include "ruzhouxie/macro_define.h"
#include "tuple.h"
#include "processer.h"
#include "macro_define.h"
#include <type_traits>
#include <utility>

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

			template<typename U>
			constexpr operator view<U>(this auto&& self)
				requires requires{ FWD(self, raw_view) | make_tree<U>; }
			{
				return view<U>{ FWD(self, raw_view) | make_tree<U> };
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

		template<auto Seq, auto Indices, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq, Indices>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | get_tape<Seq, Indices>)
		// template<specified<view> Self>
		// RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
		// 	AS_EXPRESSION(id_tree<T>())
	};

	template<typename T>
	view(T&&) -> view<T>;

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
		inline constexpr pipe_closure<detail::as_ref_t> as_ref{};
	}
}



//relayout
namespace ruzhouxie
{
	namespace detail
	{
		// template<auto layout>
		// constexpr auto relayout_id_tree(const auto& tree) 
		// {
		// 	using layout_type = purified<decltype(layout)>;
		// 	return[&]<size_t...I>(std::index_sequence<I...>)
		// 	{
		// 		if constexpr (tensor_rank<layout_type> >= 2uz)
		// 		{
		// 			return tuple{ relayout_id_tree<layout | child<I>>(tree)... };
		// 		}
		// 		else
		// 		{
		// 			return id_tree_get<layout | child<I>...>(tree);
		// 		}
		// 	}(std::make_index_sequence<child_count<layout_type>>{});
		// }

		template<typename T, auto Layout>
		struct relayout_view : view_base<relayout_view<T, Layout>>
		{
			static constexpr auto layout = Layout;
			using layout_type = purified<decltype(Layout)>;

			RUZHOUXIE_MAYBE_EMPTY T raw_tree;

			template<size_t I, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
				//todo... noexcept)
			{
				if constexpr (I >= child_count<layout_type>)
				{
					return;
				}
				else if constexpr(indices<child_type<layout_type, I>>)
				{
					return FWD(self, raw_tree) | child<Layout | child<I>>;
				}
				else
				{
					return relayout_view<decltype(FWD(self, raw_tree)), Layout | child<I>>
					{
						{}, FWD(self, raw_tree)
					};
				}
			}

			template<auto Seq, auto Indices, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq, Indices>>, Self&& self)
			{
				constexpr auto transformed_sequence = []<size_t...I>(std::index_sequence<I...>)
				{
					return tuple{ Layout | child<Seq | child<Indices[I]>> ... };
				}(std::make_index_sequence<Indices.size()>{});
				return FWD(self, raw_tree) | get_tape<transformed_sequence>;
			}
		};
	}

	namespace detail
	{
		template<auto layout>
		struct relayout_t
		{
			using layout_type = purified<decltype(layout)>;

			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (indices<layout_type>)
				{
					return FWD(t) | child<layout>;
				}
				else
				{
					return relayout_view<T, layout>{ {}, FWD(t) };
				}
			}
		};
	};

	inline namespace functors
	{
		template<auto layout>
		inline constexpr pipe_closure<detail::relayout_t<layout>> relayout{};
	}
}

//zip transform
namespace ruzhouxie
{
	namespace detail
	{
		template<typename Fn, branched...T>
		struct zip_transform_view;
	}

	template<typename Fn, branched...T>
	struct detail::zip_transform_view : view_base<zip_transform_view<Fn, T...>>
	{
		RUZHOUXIE_MAYBE_EMPTY Fn fn;
		RUZHOUXIE_MAYBE_EMPTY tuple<T...> trees;

		template<size_t I, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
			//todo...noexcept
		{
			if constexpr ((... || (I >= child_count<T>)))
			{
				return;
			}
			else return[&]<size_t...J>(std::index_sequence<J...>) -> decltype(auto)
			{
				return FWD(self, fn)(FWD(self, trees) | child<J, I>...);
			}(std::make_index_sequence<sizeof...(T)>{});
		}

		template<auto Seq, auto Indices, specified<zip_transform_view> View>
		struct tape_type
		{
			template<size_t...I>
			static constexpr auto init_tapes(View&& view, std::index_sequence<I...>)
			{
				return tuple<decltype(FWD(view, trees) |  child<I> | get_tape<Seq, Indices>)...>
				{
					FWD(view, trees) |  child<I> | get_tape<Seq, Indices>... 
				};
			}
			
			View view;
			decltype(init_tapes(declval<View&&>(), std::index_sequence_for<T...>{})) tapes;

			explicit constexpr tape_type(View&& view) 
				: view(FWD(view))
				, tapes(init_tapes(FWD(view), std::index_sequence_for<T...>{}))
			{}

			template<size_t I, specified<tape_type> Self>
			friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
			{
				if constexpr(I >= Indices.size())
				{
					return;
				}
				else 
				{
					return [&]<size_t...J>(std::index_sequence<J...>) -> decltype(auto)
					{
						return self.view.fn(FWD(self, tapes) | child<J, I> ...);
					}(std::index_sequence_for<T...>{});
				}
			}
		};

		template<auto Seq, auto Indices, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq, Indices>>, Self&& self)
			//todo...noexcept
		{
			return tape_type<Seq, Indices, Self&&>{ FWD(self) };
		}

	private:
		// template<auto Sequence, specified<zip_transform_view> View>
		// static constexpr auto trans_seq_and_map()
		// {
		// 	struct location
		// 	{
		// 		bool is_temp;
		// 		size_t index;
		// 	};
		// }

		// template<auto Sequence, specified<zip_transform_view> View, auto Cur = tuple{}, size_t I = 0>
		// static constexpr auto trans_seq_and_map_impl(auto& map)
		// {
		// 	if constexpr(I >= child_count<decltype(Sequence)>)
		// 	{
		// 		return Cur;
		// 	}
		// 	else
		// 	{

		// 	}
		// }
	};

	namespace detail
	{
		struct zip_transform_t
		{
			template<typename Fn, typename...T>
			RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn, T&&...trees)const
			{
				return detail::zip_transform_view<std::decay_t<Fn>, T...>
				{
					{}, FWD(fn), tuple<T...>{ FWD(trees)... }
				};
			}
		};

		struct transform_t
		{
			template<typename Fn, typename T>
			RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn, T&& tree)const
			{
				return zip_transform_view<std::decay_t<Fn>, T>
				{
					{}, FWD(fn), tuple<T>{ FWD(tree) }
				};
			}
		};
	}

	inline constexpr detail::zip_transform_t zip_transform{};
	inline constexpr pipe_closure<detail::transform_t, 2> transform{};
}




#include "macro_undef.h"
#endif