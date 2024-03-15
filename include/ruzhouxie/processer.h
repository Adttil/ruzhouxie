#ifndef RUZHOUXIE_RESULT_H
#define RUZHOUXIE_RESULT_H

#include "general.h"
#include "tree_adaptor.h"
#include "array.h"
#include "tape.h"
#include "get.h"
#include "tuple.h"

#include "macro_define.h"

namespace ruzhouxie
{
	template<typename P>
    struct processer 
	{
	    template<typename V>
	    RUZHOUXIE_INLINE constexpr auto operator()(this const P& self, V&& view)
		    AS_EXPRESSION(self.template process_tape<V&&, 0uz>(FWD(view) | get_tape<P::template get_sequence<V&&>()>))
	};
}

namespace ruzhouxie
{
    namespace detail
	{
	    template<typename Tree>
	    struct make_tree_t;
	}

    inline namespace functors
	{
	    template<typename Tree>
	    inline constexpr tree_adaptor_closure<detail::make_tree_t<Tree>> make_tree{};
	}

    template<typename Tree>
    struct tree_maker_trait;

    template<typename Tree>
    using tree_maker = tree_maker_trait<Tree>::type;

    template<typename Tree>
    struct detail::make_tree_t
	{
	    template<typename V>
	    RUZHOUXIE_INLINE constexpr auto operator()(V&& view)const
		    AS_EXPRESSION(tree_maker<Tree>{}(FWD(view)))
	};

    template<typename View, typename Tree>
    concept makeable = requires{ std::declval<View>() | make_tree<Tree>; };
}

namespace ruzhouxie
{
    struct invalid_tree_maker {};

    namespace detail::tag_invoke_tree_maker_ns
	{
	    template<typename T>
	    void tag_invoke();

	    template<typename Tree>
	    struct tag_invoke_tree_maker
		{
		    RUZHOUXIE_INLINE constexpr auto operator()(auto&& view)const
			    AS_EXPRESSION(tag_invoke<Tree>(make_tree<Tree>, FWD(view)))
		};
	}
    using detail::tag_invoke_tree_maker_ns::tag_invoke_tree_maker;

    namespace detail
	{
	    consteval auto sequence_add_prefix(const auto& sequence, const auto& prefix)
		{
		    return [&]<size_t...I>(std::index_sequence<I...>)
			{
			    return tuple{ detail::concat_array(prefix, sequence | child<I>)... };
			}(std::make_index_sequence<child_count<decltype(sequence)>>{});
		}

		// template<size_t Offset>
		// consteval auto sequence_drop(const auto& prefix)
		// {
		//     return [&]<size_t...I>(std::index_sequence<I...>)
		// 	{
		// 	    return tuple{ array_drop<Offset>(prefix, sequence | child<I>)... };
		// 	}(std::make_index_sequence<child_count<decltype(sequence)>>{});
		// }
	}

    template<typename Tuple>
    struct tuple_maker : processer<tuple_maker<Tuple>>
	{
	    template<size_t I, typename V>
	    static consteval auto child_sequence()
		{
		    using child_t = std::tuple_element_t<I, Tuple>;
		    auto seq = tree_maker<child_t>{}.template get_sequence<child_type<V, I>>();
		    return detail::sequence_add_prefix(seq, array{ I } );
		};
		
	    template<typename V>
	    static consteval auto get_sequence()
		{
		    if constexpr(terminal<Tuple>)
			{
			    return tuple{ indices_of_whole_view };
			}
		    else return []<size_t...I>(std::index_sequence<I...>)
			{
			    return tuple_cat(child_sequence<I, V>()...);
			}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
		}

	    template<typename T, size_t Offset, size_t I, typename Tape>
	    static constexpr auto child_process_tape(Tape&& tape)
		{
		    constexpr size_t offset = []<size_t...J>(std::index_sequence<J...>)
			{
			    return (Offset + ... + std::tuple_size_v<decltype(child_sequence<J, T>())>); 
			}(std::make_index_sequence<I>{});

			//auto child_tape = tape_drop<offset>(FWD(tape));
		    using child_t = std::tuple_element_t<I, Tuple>;
		    return tree_maker<child_t>{}.template process_tape<child_type<T, I>, offset>(FWD(tape));
		};

	    template<typename T, size_t Offset, typename Tape>
	    constexpr auto process_tape(Tape&& tape)const
		{
		    if constexpr(terminal<Tuple>)
			{
			    if constexpr(std::is_object_v<Tuple>)
				{
				    return static_cast<Tuple>(access<Offset>(FWD(tape)));
				}
			    else
				{
				    return static_cast<Tuple>(pass<Offset>(FWD(tape)));
				}
			}
		    else return [&]<size_t...I>(std::index_sequence<I...>)
			{
			    return Tuple{ child_process_tape<T, Offset, I>(FWD(tape))... };
			}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
		}
	};

	// template<typename Tuple>
	// struct inverse_tuple_maker : processer<tuple_maker<Tuple>>
	// {
	//     template<size_t I, typename T> requires branched<Tuple>
	//     static consteval auto child_sequence()
	// 	{
	// 	    using child_t = std::tuple_element_t<I, Tuple>;
	// 	    auto seq = tree_maker<child_t>{}.get_sequence<child_type<T, I>>();
	// 	    return detail::sequence_add_prefix(seq, array{ I } );
	// 	};
		
	//     template<typename T>
	//     static consteval auto get_sequence()
	// 	{
	// 	    if constexpr(terminal<Tuple>)
	// 		{
	// 		    return tuple{ indices_of_whole_view };
	// 		}
	// 	    else return []<size_t...I>(std::index_sequence<I...>)
	// 		{
	// 		    return tuple_cat(child_sequence<I, T>()...);
	// 		}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
	// 	}

	//     template<typename T, size_t I, typename Tape> requires branched<Tuple>
	//     static constexpr decltype(auto) child_process_tape(Tape&& tape)
	// 	{
	// 	    constexpr size_t offset = []<size_t...J>(std::index_sequence<J...>)
	// 		{
	// 		    return (0uz + ... + std::tuple_size_v<decltype(child_sequence<J, T>())>); 
	// 		}(std::make_index_sequence<I>{});

	// 	    auto child_tape = sub_tape<offset>(FWD(tape));
	// 	    using child_t = std::tuple_element_t<I, Tuple>;
	// 	    return tree_maker<child_t>{}.process_tape<child_type<T, I>>(move(child_tape));
	// 	};

	//     template<typename T, typename Tape>
	//     constexpr auto process_tape(Tape&& tape)const
	// 	{
	// 	    if constexpr(terminal<Tuple>)
	// 		{
	// 		    return Tuple{ FWD(tape) | child<0uz> };
	// 		}
	// 	    else return [&]<size_t...I>(std::index_sequence<I...>)
	// 		{
	// 		    return Tuple{ child_process_tape<T, I>(FWD(tape))... };
	// 		}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
	// 	}
	// };

	// template<typename T>
	// struct terminal_maker : processer<terminal_maker<T>>
	// {
		
	// };

    template<typename Target>
    struct tree_maker_trait
	{
	    static consteval auto choose_default_tree_maker() noexcept
		{
		    return tuple_maker<Target>{};
			// if constexpr (requires{ std::tuple_size<purified<Target>>::value; })
			// {
			//     return tuple_maker<Target>{};
			// }
			// else if constexpr(terminal<Target>)
			// {

			// }
			// else
			// {
			//     static_assert(requires{ std::tuple_size<purified<Target>>::value; });
			//     return invalid_tree_maker{};
			// }
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
	    return tree_adaptor_closure<detail::to_tpl_temp_t<Tpl>>{};
	}

    template<typename Tpl>
    RUZHOUXIE_INLINE constexpr auto to()
	{
	    return make_tree<Tpl>;
	}

    template<template<typename...> typename Tpl = tuple, branched T>
    RUZHOUXIE_INLINE constexpr decltype(auto) to(T&& t)
	{
	    return to<Tpl>()(t);
	}

    template<template<typename...> typename Tpl>
    struct detail::to_tpl_temp_t : processer<detail::to_tpl_temp_t<Tpl>>
	{
		//using processer<tuple_maker<Tuple>>::operator();
		
	    template<size_t I, typename T>
	    static consteval auto child_sequence()
		{
		    auto seq = get_sequence<child_type<T, I>>();
		    return detail::sequence_add_prefix(seq, array{ I } );
		};

	    template<typename T>
	    static consteval auto get_sequence()
		{
		    if constexpr(terminal<T>)
			{
			    return tuple{ indices_of_whole_view };
			}
		    else return []<size_t...I>(std::index_sequence<I...>)
			{
			    return tuple_cat(child_sequence<I, T>()...);
			}(std::make_index_sequence<child_count<T>>{});
		}

	    template<typename T, size_t Offset, size_t I, typename Tape>
	    constexpr auto child_process_tape(Tape&& tape)const
		{
		    constexpr size_t offset = []<size_t...J>(std::index_sequence<J...>)
			{
			    return (Offset + ... + std::tuple_size_v<decltype(get_sequence<child_type<T, J>>())>); 
			}(std::make_index_sequence<I>{});

		    return process_tape<child_type<T, I>, offset>(FWD(tape));
		};

	    template<typename T, size_t Offset, typename Tape>
	    constexpr auto process_tape(Tape&& tape)const
		{
		    if constexpr(terminal<T>)
			{
			    return access<Offset>(FWD(tape));
			}
		    else return [&]<size_t...I>(std::index_sequence<I...>)
			{
			    return Tpl{ child_process_tape<T, Offset, I>(FWD(tape))... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	};


}

//for_each
namespace ruzhouxie
{
	// namespace detail 
	// {
	//     struct for_each_t 
	// 	{
	// 	    template<branched Tree>
	// 	    RUZHOUXIE_INLINE constexpr void operator()(auto&& fn, Tree&& tree)const
	// 		    noexcept//todo...
	// 		{
	// 		    auto each_fn = [&]<size_t...I>(std::index_sequence<I...>)
	// 			{
	// 			    constexpr size_t cur = child_count<Tree> - sizeof...(I) - 1uz;
	// 			    constexpr auto rest_ids = merge_id_set<id_tree_to_set<id_tree_get<I + cur + 1uz>(id_tree<Tree>())>()...>();
	// 			    fn(FWD(tree) | child<cur, rest_ids>);
	// 			};

	// 			[&] <size_t...I>(std::index_sequence<I...>) 
	// 			{
	// 				(..., each_fn(std::make_index_sequence<child_count<Tree> - I - 1uz>{}));
	// 			}(std::make_index_sequence<child_count<Tree>>{});
	// 		}
	// 	};
	// }

	// inline constexpr adaptor_closure<detail::for_each_t> for_each{};
}

#include "macro_undef.h"
#endif