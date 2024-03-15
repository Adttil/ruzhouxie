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
            AS_EXPRESSION(Tree{ tree_maker<Tree>{}(FWD(view)) })
    };
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
        //     {
        //         return tuple{ array_drop<Offset>(prefix, sequence | child<I>)... };
        //     }(std::make_index_sequence<child_count<decltype(sequence)>>{});
        // }
    }

    template<typename T>
    struct leaf_maker : processer<leaf_maker<T>>
    {
        template<typename V>
        static consteval auto get_sequence()
        {
            return tuple{ indices_of_whole_view };
        };

        template<typename V, size_t Offset, typename Tape>
        RUZHOUXIE_INLINE constexpr auto process_tape(Tape&& tape)const
            AS_EXPRESSION(static_cast<T>(access<Offset>(FWD(tape))))
    };

    template<typename Tuple>
    struct tuple_maker : processer<tuple_maker<Tuple>>
    {
    private:
        template<size_t I, typename V>
        static consteval auto child_sequence()
        {
            using child_t = std::tuple_element_t<I, Tuple>;
            auto seq = tree_maker<child_t>::template get_sequence<child_type<V, I>>();
            return detail::sequence_add_prefix(seq, array{ I } );
        };

    public:
        template<typename V>
        static consteval auto get_sequence()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return tuple_cat(child_sequence<I, V>()...);
            }(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
        }

    private:
        template<typename T, size_t Offset, size_t I>
        static consteval size_t child_tape_offset()
        {
            return []<size_t...J>(std::index_sequence<J...>)
            {
                return (Offset + ... + std::tuple_size_v<decltype(child_sequence<J, T>())>); 
            }(std::make_index_sequence<I>{});
        }

        template<typename T, size_t Offset, size_t I, typename Tape>
        RUZHOUXIE_INLINE static constexpr auto child_process_tape(Tape&& tape)
        AS_EXPRESSION(
            tree_maker<std::tuple_element_t<I, Tuple>>{}
                .template process_tape<child_type<T, I>, child_tape_offset<T, Offset, I>()>(FWD(tape))
        )

        template<typename T, size_t Offset, typename Tape, size_t...I>
        RUZHOUXIE_INLINE constexpr auto process_tape_Impl(Tape&& tape, std::index_sequence<I...>)const
            AS_EXPRESSION(Tuple{ child_process_tape<T, Offset, I>(FWD(tape))... })

    public:
        template<typename T, size_t Offset, typename Tape>
        RUZHOUXIE_INLINE constexpr auto process_tape(Tape&& tape)const
            AS_EXPRESSION(process_tape_Impl<T, Offset>(FWD(tape), std::make_index_sequence<std::tuple_size_v<Tuple>>{}))
    };

    template<typename Tree>
    struct tree_maker_trait
    {
        static consteval auto choose_default_tree_maker() noexcept
        {
            if constexpr(requires{ tag_invoke<Tree>(make_tree<Tree>); })
            {
                return tag_invoke<Tree>(make_tree<Tree>);
            }
            else if constexpr (requires{ std::tuple_size<purified<Tree>>::value; })
            {
                return tuple_maker<Tree>{};
            }
            else if constexpr(terminal<Tree>)
            {
                return leaf_maker<Tree>{};
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
    private:
        template<size_t I, typename T>
        static consteval auto child_sequence()
        {
            auto seq = get_sequence<child_type<T, I>>();
            return detail::sequence_add_prefix(seq, array{ I } );
        };

    public:
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

        template<terminal T, size_t Offset, typename Tape>
        RUZHOUXIE_INLINE constexpr auto process_tape(Tape&& tape)const
            AS_EXPRESSION(T{ access<Offset>(FWD(tape)) })
    private:
        template<typename T, size_t Offset, size_t I>
        static consteval size_t child_tape_offset()
        {
            return []<size_t...J>(std::index_sequence<J...>)
            {
                return (Offset + ... + std::tuple_size_v<decltype(child_sequence<J, T>())>); 
            }(std::make_index_sequence<I>{});
        }

        template<typename T, size_t Offset, size_t I, typename Tape>
        RUZHOUXIE_INLINE static constexpr auto child_process_tape(Tape&& tape)
            AS_EXPRESSION(to_tpl_temp_t{}.template process_tape<child_type<T, I>, child_tape_offset<T, Offset, I>()>(FWD(tape)))

        template<typename T, size_t Offset, typename Tape, size_t...I>
        RUZHOUXIE_INLINE static constexpr auto children_process_tape(Tape&& tape, std::index_sequence<I...>)
            AS_EXPRESSION(Tpl{ child_process_tape<T, Offset, I>(FWD(tape))... })

    public:
        template<branched T, size_t Offset, typename Tape>
        RUZHOUXIE_INLINE constexpr auto process_tape(Tape&& tape)const
            AS_EXPRESSION(children_process_tape<T, Offset>(FWD(tape), std::make_index_sequence<child_count<T>>{}))
    };


}

//for_each
namespace ruzhouxie
{
    // namespace detail 
    // {
    //     struct for_each_t 
    //     {
    //         template<branched Tree>
    //         RUZHOUXIE_INLINE constexpr void operator()(auto&& fn, Tree&& tree)const
    //             noexcept//todo...
    //         {
    //             auto each_fn = [&]<size_t...I>(std::index_sequence<I...>)
    //             {
    //                 constexpr size_t cur = child_count<Tree> - sizeof...(I) - 1uz;
    //                 constexpr auto rest_ids = merge_id_set<id_tree_to_set<id_tree_get<I + cur + 1uz>(id_tree<Tree>())>()...>();
    //                 fn(FWD(tree) | child<cur, rest_ids>);
    //             };

    //             [&] <size_t...I>(std::index_sequence<I...>) 
    //             {
    //                 (..., each_fn(std::make_index_sequence<child_count<Tree> - I - 1uz>{}));
    //             }(std::make_index_sequence<child_count<Tree>>{});
    //         }
    //     };
    // }

    // inline constexpr adaptor_closure<detail::for_each_t> for_each{};
}

#include "macro_undef.h"
#endif