#ifndef RUZHOUXIE_ADAPTORS_H
#define RUZHOUXIE_ADAPTORS_H

#include "general.h"
#include "evaluate.h"
#include "get.h"
#include "tuple.h"
#include "result.h"
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
		template<auto...I, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I...>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | child<I...>)

		// template<specified<view> Self>
		// RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
		// 	AS_EXPRESSION(id_tree<T>())
	};

	template<typename T>
	view(T&&) -> view<T>;
}



//relayout
namespace ruzhouxie
{
	namespace detail
	{
		template<auto layout>
		constexpr auto relayout_id_tree(const auto& tree) 
		{
			using layout_type = purified<decltype(layout)>;
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				if constexpr (tensor_rank<layout_type> >= 2uz)
				{
					return tuple{ relayout_id_tree<layout | child<I>>(tree)... };
				}
				else
				{
					return id_tree_get<layout | child<I>...>(tree);
				}
			}(std::make_index_sequence<child_count<layout_type>>{});
		}

		template<typename T, auto layout>
		struct relayout_view : view_base<relayout_view<T, layout>>
		{
			using layout_type = purified<decltype(layout)>;

			RUZHOUXIE_MAYBE_EMPTY T tree;

			template<size_t I, id_set Ids, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I, Ids>>, Self&& self)
				//todo... noexcept)
			{
				if constexpr (I >= child_count<layout_type>)
				{
					return;
				}
				else if constexpr (tensor_rank<layout_type> > 2uz)
				{
					if constexpr (Ids.empty())
					{
						return relayout_view<decltype(FWD(self, tree)), layout | child<I>>
						{
							{}, FWD(self, tree)
						};
					}
					else
					{
						return relayout_view<decltype(FWD(self, tree)), layout | child<I>>
						{
							{}, FWD(self, tree)
						}
						| reserved<Ids>;
					}
				}
				else if constexpr(tensor_rank<layout_type> == 2uz)
				{
					return[&]<size_t...J>(std::index_sequence<J...>)->decltype(auto)
					{
						return FWD(self, tree) | child<layout | child<I, J> ..., Ids>;
					}(std::make_index_sequence<child_count<child_type<layout_type, I>>>{});
				}
				else 
				{
					static_assert(tensor_rank<layout_type> >= 2uz, "invalid layout.");
				}
			}

			template<specified<relayout_view> Self>
			friend constexpr auto tag_invoke(tag_t<id_tree<Self>>) noexcept
			{
				return relayout_id_tree<layout>(id_tree<T>());
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
				if constexpr (tensor_rank<layout_type> > 1uz)
				{
					return relayout_view<T, layout>{ {}, FWD(t) };
				}
				else return[&]<size_t...I>(std::index_sequence<I...>)->decltype(auto)
				{
					return FWD(t) | child<(layout | child<I>)...>;
				}(std::make_index_sequence<child_count<layout_type>>{});
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
				return FWD(self, fn)
				(
					FWD(self, trees) | child<J> | child<I>...
				);
			}(std::make_index_sequence<sizeof...(T)>{});
		}

		template<specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
		{
			/*return[]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ tuple{ id_tree_get<I>(id_tree<T>())... }... };
			}(std::make_index_sequence<child_count<Self>>{});*/
			
			return[]<size_t...I>(std::index_sequence<I...>)
			{
				constexpr auto child_id_tree = []<size_t J>()
				{
					return tuple{ id_tree_get<J>(id_tree<T>())... };
				};
				return tuple{ child_id_tree.template operator()<I>()... };
			}(std::make_index_sequence<child_count<Self>>{});
		}
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