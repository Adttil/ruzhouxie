#ifndef RUZHOUXIE_RESULT_H
#define RUZHOUXIE_RESULT_H

#include "general.h"
#include "evaluate.h"
#include "get.h"
#include "macro_define.h"

namespace ruzhouxie
{
	namespace detail
	{
		template<typename Target>
		struct make_tree_t;
	}

	inline namespace functors
	{
		template<typename T>
		inline constexpr pipe_closure<detail::make_tree_t<T>> make_tree{};
	}

	template<typename Target, typename Source>
	struct tree_maker_trait;

	template<typename Target, typename Source>
	using tree_maker = tree_maker_trait<Target, Source>::type;

	template<typename Target>
	struct detail::make_tree_t
	{
		template<typename T>
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(tree_maker<Target, T&&>{}(FWD(t)))
	};

	template<typename View, typename Tree>
	concept makeable = requires{ declval<View>() | make_tree<Tree>; };
}

namespace ruzhouxie
{
	struct invalid_tree_maker {};

	namespace detail::tag_invoke_tree_maker_ns
	{
		template<typename T>
		void tag_invoke();

		template<typename Target>
		struct tag_invoke_tree_maker
		{
			RUZHOUXIE_INLINE constexpr auto operator()(auto&& source)const
				AS_EXPRESSION(tag_invoke<Target>(make_tree<Target>, FWD(source)))
		};
	}
	using detail::tag_invoke_tree_maker_ns::tag_invoke_tree_maker;

	template<typename Tuple>
	struct tuple_maker
	{
		/*template<id_set Ids, typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) impl(T&& t);*/

		template<size_t...J, typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) elem(std::index_sequence<J...>, T&& t)
		{
			constexpr size_t cur = child_count<T> - sizeof...(J);
			constexpr auto ids = merge_id_set<id_tree_to_set<id_tree_get<cur + J>(id_tree<T>())>()...>();
			return FWD(t) | child<cur - 1, ids>;
			// if constexpr (requires{
			// 	{FWD(t) | child<cur - 1, ids> | make_tree<std::tuple_element_t<cur - 1, Tuple>>} -> concrete;
			// })
			// {
			// 	return FWD(t) | child<cur - 1, ids> | make_tree<std::tuple_element_t<cur - 1, Tuple>>;
			// }
		}

		template<typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) impl(T&& t)
		{
			if constexpr (terminal<T>)
			{
				if constexpr (requires { Tuple{ FWD(t) }; })
				{
					return FWD(t);
				}
				else
				{
					return;
				}
			}
			else
			{
				constexpr auto foo = []<size_t...I>(std::index_sequence<I...>, T && t) -> decltype(auto)
				{
					if constexpr (requires{ Tuple{ elem(std::make_index_sequence<child_count<T> -I - 1>{}, FWD(t))... }; }) 
					{
						return Tuple{ elem(std::make_index_sequence<child_count<T> -I - 1>{}, FWD(t))... };
					}
					else if constexpr(requires{ Tuple{ +elem(std::make_index_sequence<child_count<T> -I - 1>{}, FWD(t))... }; })
					{
						return Tuple{ +elem(std::make_index_sequence<child_count<T> -I - 1>{}, FWD(t))... };
					}
				};

				//RUZHOUXIE_INLINE_CALLS
				return foo(std::make_index_sequence<child_count<T>>{}, FWD(t));
			}
		}

		template<typename T>
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const
			requires requires{ {impl(FWD(t))} ->concrete; }
		{
			static_assert(not std::is_array_v<purified<array<array<double, 2>, 2>>>);
			return Tuple( impl(FWD(t)) );
		}
	};

	template<typename Target, typename Source>
	struct tree_maker_trait
	{
		static consteval auto choose_default_tree_maker() noexcept
		{
			if constexpr (requires{ tag_invoke_tree_maker<Target>{}(declvalue<Source>()); })
			{
				return tag_invoke_tree_maker<Target>{};
			}
			else if constexpr (requires{ std::tuple_size<purified<Target>>::value; })
			{
				return tuple_maker<Target>{};
			}
			else
			{
				return invalid_tree_maker{};
			}
		}

		using type = decltype(choose_default_tree_maker());
	};
}

namespace ruzhouxie
{
	namespace detail
	{
		template<template<typename...> typename Tpl>
		struct to_tpl_temp_t;
	}

	template<template<typename...> typename Tpl = tuple>
	RUZHOUXIE_INLINE constexpr auto to()
	{
		return pipe_closure<detail::to_tpl_temp_t<Tpl>>{};
	}

	template<typename Tpl>
	RUZHOUXIE_INLINE constexpr auto to()
	{
		return make_tree<Tpl>;
	}

	template<template<typename...> typename Tpl = tuple, typename T>
	RUZHOUXIE_INLINE constexpr decltype(auto) to(T&& t)
	{
		return to<Tpl>()(t);
	}

	template<template<typename...> typename Tpl>
	struct detail::to_tpl_temp_t
	{
		/*template<id_set Ids, typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) impl(T&& t);*/

		template<id_set Ids, size_t...J, typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) elem(std::index_sequence<J...>, T&& t)
		{
			constexpr size_t cur = child_count<T> -sizeof...(J);
			constexpr auto ids = merge_id_set<Ids, id_tree_to_set<id_tree_get<cur + J>(id_tree<T>())>()...>();
			return impl<ids>(FWD(t) | child<cur - 1, ids>);
		}

		template<id_set Ids, typename T>
		RUZHOUXIE_INLINE static constexpr decltype(auto) impl(T&& t)
		{
			if constexpr (terminal<T>)
			{
				return FWD(t);
			}
			else
			{
				constexpr auto foo = []<size_t...I>(std::index_sequence<I...>, T && t)
				{
					return Tpl{ elem<Ids>(std::make_index_sequence<child_count<T> -I - 1>{}, FWD(t))... };
				};

				//RUZHOUXIE_INLINE_CALLS
				return foo(std::make_index_sequence<child_count<T>>{}, FWD(t));
			}
		}

		template<typename T>
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const
		{
			return impl<empty_id_set>(FWD(t));
		}
	};


}

//for_each
namespace ruzhouxie
{
	namespace detail 
	{
		struct for_each_t 
		{
			template<branched Tree>
			RUZHOUXIE_INLINE constexpr void operator()(auto&& fn, Tree&& tree)const
				noexcept//todo...
			{
				auto each_fn = [&]<size_t...I>(std::index_sequence<I...>)
				{
					constexpr size_t cur = child_count<Tree> - sizeof...(I) - 1uz;
					constexpr auto rest_ids = merge_id_set<id_tree_to_set<id_tree_get<I + cur + 1uz>(id_tree<Tree>())>()...>();
					fn(FWD(tree) | child<cur, rest_ids>);
				};

				[&] <size_t...I>(std::index_sequence<I...>) 
				{
					(..., each_fn(std::make_index_sequence<child_count<Tree> - I - 1uz>{}));
				}(std::make_index_sequence<child_count<Tree>>{});
			}
		};
	}

	inline constexpr adaptor_closure<detail::for_each_t> for_each{};
}

#include "macro_undef.h"
#endif