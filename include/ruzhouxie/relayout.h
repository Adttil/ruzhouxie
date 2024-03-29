#ifndef RUZHOUXIE_RELAYOUT_H
#define RUZHOUXIE_RELAYOUT_H

#include "general.h"
#include "get.h"
#include "tree_adaptor.h"
#include "array.h"
#include "tuple.h"
#include "tree_view.h"
#include "constant.h"
#include "math.h"

#include "macro_define.h"

namespace ruzhouxie
{
    namespace detail 
    {
        enum class relayout_view_child_Strategy
        {
            none,
            indices,
            child,
            relayout
        };

        template<auto Indices, typename TLayout>
        RUZHOUXIE_CONSTEVAL auto mapped_indices(const TLayout& layout)
        {
            if constexpr(indicesoid<TLayout>)
            {
                return detail::concat_array(layout, Indices);
            }
            else if constexpr(Indices.size() == 0uz)
            {
                return layout;
            }
            else
            {
                return mapped_indices<detail::array_drop<1uz>(Indices)>(layout | child<Indices[0uz]>);
            }
        }

        template<auto Layout, typename Trans>
        RUZHOUXIE_CONSTEVAL auto mapped_layout(const Trans& trans)
        {
            if constexpr(indicesoid<decltype(Layout)>)
            {
                return mapped_indices<Layout>(trans);
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype(mapped_layout<Layout | child<I>>(trans))...>
                {
                    mapped_layout<Layout | child<I>>(trans)...
                };
            }(std::make_index_sequence<child_count<decltype(Layout)>>{});
        }
    }

    template<typename V, auto Layout>
    struct relayout_view_maker : processer<relayout_view_maker<V, Layout>>
    {
        template<auto Indices, auto L, size_t I = child_count<decltype(L)> - 1uz>
        static RUZHOUXIE_CONSTEVAL auto unmapped_indices()
        {
            if constexpr(I >= child_count<decltype(L)>)
            {
                return;
            }
            else if constexpr(indicesoid<child_type<decltype(L), I>> && equal(L | child<I>, Indices))
            {
                return array{ I };   
            }
            else if constexpr(concrete<decltype(unmapped_indices<Indices, L | child<I>>())>)
            {
                return detail::concat_array(array{ I }, unmapped_indices<Indices, L | child<I>>());
            }
            else
            {
                return unmapped_indices<Indices, L, I - 1uz>();
            }
        }

        template<auto Layout_, auto Trans>
        static RUZHOUXIE_CONSTEVAL auto unmapped_layout()
        {
            if constexpr(indicesoid<decltype(Layout_)>)
            {
                return unmapped_indices<Layout_, Trans>();
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(unmapped_layout<Layout_ | child<I>, Trans>()...);
            }(std::make_index_sequence<child_count<decltype(Layout_)>>{});
        }

        template<typename T>
        static RUZHOUXIE_CONSTEVAL auto get_sequence()
        {
            constexpr auto raw_seq = tree_maker<V>::template get_sequence<T>();
            return unmapped_layout<raw_seq, Layout>();
        };

        template<typename T, size_t Offset, typename Tape>
        RUZHOUXIE_INLINE constexpr auto process_tape(Tape&& tape)const
            AS_EXPRESSION(relayout_view<V, Layout>{ V{ tree_maker<V>{}.template process_tape<T, 0uz>(FWD(tape)) } })
    };

    template<typename V, auto Layout>
    struct relayout_view : detail::view_base<V>, constant_t<Layout>, view_interface<relayout_view<V, Layout>>
    {
        using base_type = V;
        static constexpr auto layout = Layout;
    private:
        template<size_t I, specified<relayout_view> Self>
        static RUZHOUXIE_CONSTEVAL choice_t<detail::relayout_view_child_Strategy> child_Choose()
        {
            using strategy_t = detail::relayout_view_child_Strategy;
            using layout_type = purified<decltype(Layout)>;

            if constexpr(indicesoid<layout_type>)
            {
                if constexpr(I < child_count<child_type<V, Layout>>)
                {
                    return { strategy_t::indices, noexcept(std::declval<Self>().base() | child<Layout> | child<I>) };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }
            if constexpr(I >= child_count<layout_type>)
            {
                return { strategy_t::none, true };
            }
            else if constexpr(indicesoid<child_type<layout_type, I>>)
            {
                if constexpr(requires{ { std::declval<Self>().base() | child<Layout | child<I>> } -> concrete; })
                {
                    return { strategy_t::child, noexcept(std::declval<Self>().base() | child<Layout | child<I>>) };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }
            else 
            {
                return { strategy_t::relayout, true };
            }
        }
    
    public:
        template<size_t I, specified<relayout_view> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
            noexcept(child_Choose<I, Self>().nothrow)
        {
            using strategy_t = detail::relayout_view_child_Strategy;
            constexpr strategy_t strategy = child_Choose<I, Self>().strategy;
            
            if constexpr (strategy == strategy_t::none)
            {
                return end();
            }
            else if constexpr(strategy == strategy_t::indices)
            {
                return FWD(self).base() | child<Layout> | child<I>;
            }
            else if constexpr(strategy == strategy_t::child)
            {
                constexpr auto index_pack = Layout | child<I>;
                return FWD(self).base() | child<index_pack>;
            }
            else if constexpr(strategy == strategy_t::relayout)
            {
                return relayout_view<decltype(FWD(self, base_view)), Layout | child<I>>
                {
                    FWD(self).base()
                };
            }
            else
            {
                static_assert(strategy == strategy_t::none, "Should not reach.");
            }
        }

        template<auto Seq, specified<relayout_view> Self> requires(not std::same_as<decltype(Seq), size_t>)
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
            AS_EXPRESSION(FWD(self).base() | get_tape<detail::mapped_layout<Seq>(Layout)>)

        template<std::same_as<relayout_view> Self>
        friend RUZHOUXIE_CONSTEVAL auto tag_invoke(tag_t<make_tree<Self>>)
        {
            return relayout_view_maker<V, Layout>{};
        }

        RUZHOUXIE_INLINE friend constexpr bool operator==(const relayout_view&, const relayout_view&) = default;
    };
    
    template<typename V, auto Layout>
    relayout_view(V&&, constant_t<Layout>) -> relayout_view<V, Layout>;
    
    template<typename T>
    concept relayout_view_instantiated = std::same_as<purified<T>, relayout_view<typename purified<T>::base_type, purified<T>::layout>>;

    namespace detail
    {
        template<auto Layout>
        struct relayout_t
        {
            using layout_type = purified<decltype(Layout)>;

            template<typename V> requires indicesoid<layout_type>
            RUZHOUXIE_INLINE constexpr auto operator()(V&& view) const
            AS_EXPRESSION(FWD(view) | child<Layout>)

            template<typename V> requires (not indicesoid<layout_type>) && (not relayout_view_instantiated<V>)
            RUZHOUXIE_INLINE constexpr auto operator()(V&& view) const
            AS_EXPRESSION(relayout_view{ FWD(view), constant_t<normalize_layout<Layout, V>()>{} })

            template<relayout_view_instantiated V> requires (not indicesoid<layout_type>) 
            RUZHOUXIE_INLINE constexpr auto operator()(V&& view) const
            AS_EXPRESSION(relayout_view{ 
                FWD(view).base(),
                constant_t<detail::mapped_layout<normalize_layout<Layout, V>()>(purified<V>::layout)>{}
            })
        };

        template<size_t N>
        inline constexpr auto repeat_layout = []<size_t...I>(std::index_sequence<I...>)
        {
            return tuple{ array<size_t, I - I>{}... };
        }(std::make_index_sequence<N>{});
    };
      
    template<auto Layout>
    inline constexpr tree_adaptor_closure<detail::relayout_t<Layout>> relayout{};

    template<size_t N>
    inline constexpr auto repeat = relayout<detail::repeat_layout<N>>;
}

namespace ruzhouxie
{
    namespace detail
    {
        
    }

    template<typename T>
    constexpr auto default_layout = []()
    {
        if constexpr (terminal<T>)
        {
            return indices_of_whole_view;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(default_layout<child_type<T, I>>, array{I})...);
        }(std::make_index_sequence<child_count<T>>{});    
    }();

    template<typename Impl>
    struct relayouter
    {
        template<typename V, specified<Impl> Self>
        RUZHOUXIE_INLINE constexpr auto operator()(this Self&& self, V&& view)
            AS_EXPRESSION(relayout_view{ 
                FWD(view),
                constant_t<detail::normalize_layout<purified<Self>::relayout(default_layout<V>), V>()>{} 
            })
    };
}

//component
namespace ruzhouxie
{
    namespace detail
    {
        template<size_t I, size_t Axis>
        struct component_t;
    }

    template<size_t J, size_t Axis = 0uz>
    inline constexpr tree_adaptor_closure<detail::component_t<J, Axis>> component{};

    template<size_t I, size_t Axis>
    struct detail::component_t : relayouter<component_t<I, Axis>>
    {
        template<typename TLayout>
        static RUZHOUXIE_CONSTEVAL auto relayout(const TLayout& layout)
        {
            if constexpr (Axis == 0uz)
            {
                static_assert(I < child_count<TLayout>, "Component index out of range.");
                return layout | child<I>;
            }
            else
            {
                static_assert(branched<TLayout>, "Axis index out of range.");
                return[&]<size_t...J>(std::index_sequence<J...>)
                {
                    return make_tuple(component<I, Axis - 1uz>.relayout(layout | child<J>)...);
                }(std::make_index_sequence<child_count<TLayout>>{});
            }
        }
    };
}

//transpose
namespace ruzhouxie
{
    namespace detail
    {
        template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
        struct transpose_t;
    }

    template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
    inline constexpr tree_adaptor_closure<detail::transpose_t<Axis1, Axis2>> transpose{};
    
    template<size_t Axis1, size_t Axis2>
    struct detail::transpose_t : relayouter<transpose_t<Axis1, Axis2>>
    {
        template<typename TLayout>
        static RUZHOUXIE_CONSTEVAL auto relayout(const TLayout& layout)
        {
            if constexpr (Axis1 == 0uz)
            {
                constexpr size_t N = tensor_shape<TLayout>[Axis2];
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(component<I, Axis2>.relayout(layout)...);
                }(std::make_index_sequence<N>{});
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(transpose<Axis1 - 1uz, Axis2 - 1uz>.relayout(layout | child<I>)...);
            }(std::make_index_sequence<child_count<TLayout>>{});
        }
    };
}

namespace ruzhouxie
{
    namespace detail
    {
        struct zip_t
        {
            template<typename...T>
            RUZHOUXIE_INLINE constexpr auto operator()(T&&...trees)const
                AS_EXPRESSION(tuple<T...>{ FWD(trees)...} | transpose<>)
        };
    }

    inline constexpr detail::zip_t zip{};
}

//span
namespace ruzhouxie
{
    namespace detail
    {
        template<size_t Begin, size_t Count/*, size_t Axis = 0uz*/>
        struct span_t : relayouter<span_t<Begin, Count>>
        {
            template<typename L>
            static RUZHOUXIE_CONSTEVAL auto relayout(const L& layout)
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<purified<decltype(layout | child<normalize_index(Begin + I, child_count<L>)>)>...>
                {
                    layout | child<normalize_index(Begin + I, child_count<L>)>...
                };
            }(std::make_index_sequence<Count>{});
            }
        };
    }
    
    template<size_t Begin, size_t Count>
    inline constexpr tree_adaptor_closure<detail::span_t<Begin, Count>> span{};
}

//combine
namespace ruzhouxie
{
    namespace detail
    {
        struct combine_t
        {
            template<typename V>
            static constexpr decltype(auto) get_base(V&& view)noexcept
            {
                if constexpr(relayout_view_instantiated<V>)
                {
                    return FWD(view).base();
                }
                else
                {
                    return FWD(view);
                }
            }

            template<typename V>
            static constexpr auto get_layout()noexcept
            {
                if constexpr(relayout_view_instantiated<V>)
                {
                    return purified<V>::layout;
                }
                else
                {
                    return default_layout<V>;
                }
            }

            template<typename...T>
            static constexpr tuple<T...> pass_tuple(T&&...args)
            {
                return { FWD(args)... };
            }

            template<typename...V>
            RUZHOUXIE_INLINE constexpr auto operator()(V&&...view)const
            {
                if constexpr(not (false || ... || relayout_view_instantiated<V>))
                {
                    return tuple<V...>{ FWD(view)... };
                }
                else return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto layout = make_tuple(layout_add_prefix(get_layout<V>(), array{ I })...);
                    return relayout_view{ pass_tuple(get_base(FWD(view))...), constant_t<layout>{} };
                }(std::index_sequence_for<V...>{});
            }
        };
    }
    
    inline constexpr detail::combine_t combine{};
}

//grouped_cartesian
namespace ruzhouxie
{
    namespace detail
    {
        template<typename V>
        constexpr inline auto vector_layout = []<size_t...I>(std::index_sequence<I...>)
        {
            return tuple{ array{I}... };
        }(std::make_index_sequence<child_count<V>>{});

        template<typename L1, typename L2>
        RUZHOUXIE_CONSTEVAL auto layout_grouped_cartesian(const L1& layout1, const L2& layout2)
        {
            if constexpr(indicesoid<L1>)
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(make_tuple(
                        detail::concat_array(array{ 0uz }, layout1), 
                        detail::concat_array(array{ 1uz }, layout2 | child<I>)
                        )...);
                }(std::make_index_sequence<child_count<L2>>{});
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(layout_grouped_cartesian(layout1 | child<I>, layout2)...);
            }(std::make_index_sequence<child_count<L1>>{});
        }

        struct grouped_cartesian_t
        {
            template<typename V1, typename V2>
            RUZHOUXIE_INLINE constexpr auto operator()(V1&& view1, V2 view2)const
            {
                return tuple<V1, V2>{ FWD(view1), FWD(view2) } | relayout<layout_grouped_cartesian(vector_layout<V1>, vector_layout<V2>)>;
            }
        };
    }
    
    inline constexpr detail::grouped_cartesian_t grouped_cartesian{};
}

#include "macro_undef.h"
#endif