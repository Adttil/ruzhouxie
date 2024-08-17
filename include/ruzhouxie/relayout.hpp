#ifndef RUZHOUXIE_RELAYOUT_HPP
#define RUZHOUXIE_RELAYOUT_HPP

#include "child.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    namespace detail
    {
        template<auto Layout>
        struct relayout_t;
    }

    template<auto Layout>
    inline constexpr detail::relayout_t<Layout> relayout{};

    namespace detail 
    {
        template<typename V, auto Layout>
        struct relayout_view_storage
        {
            RUZHOUXIE(no_unique_address) V base;
            RUZHOUXIE(no_unique_address) constant_t<Layout> layout;
        };

        enum class relayout_view_child_strategy
        {
            none,
            indices,
            child,
            relayout
        };
    }

    template<typename V, auto Layout>
    struct relayout_view : detail::relayout_view_storage<V, Layout>
    {
    private:
        template<size_t I, typename Self>
        static consteval choice_t<detail::relayout_view_child_strategy> child_choose()
        {
            using strategy_t = detail::relayout_view_child_strategy;
            using layout_type = decltype(Layout);

            if constexpr(indexical_array<layout_type>)
            {
                if constexpr(I < child_count<child_type<V, Layout>>)
                {
                    return { strategy_t::indices, noexcept(FWD(std::declval<Self>(), base) | child<Layout> | child<I>) };
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
            else if constexpr(indexical_array<child_type<layout_type, I>>)
            {
                if constexpr(requires{ FWD(std::declval<Self>(), base) | child<Layout | child<I>>; })
                {
                    return { strategy_t::child, noexcept(FWD(std::declval<Self>(), base) | child<Layout | child<I>>) };
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
        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
            noexcept(child_choose<I, Self>().nothrow)
        {
            using strategy_t = detail::relayout_view_child_strategy;
            constexpr strategy_t strategy = child_choose<I, Self>().strategy;
            
            if constexpr (strategy == strategy_t::none)
            {
                return end();
            }
            else if constexpr(strategy == strategy_t::indices)
            {
                return FWD(self, base) | child<Layout> | child<I>;
            }
            else if constexpr(strategy == strategy_t::child)
            {
                constexpr auto index_pack = Layout | child<I>;
                return FWD(self, base) | child<index_pack>;
            }
            else if constexpr(strategy == strategy_t::relayout)
            {
                return relayout_view<decltype(FWD(self, base)), Layout | child<I>>
                {
                    FWD(self, base)
                };
            }
            else
            {
                static_assert(strategy == strategy_t::none, "Should not reach.");
            }
        }

        template<auto Usage, typename Self>
        constexpr decltype(auto) simplified_data(this Self&& self)
        {
            //constexpr auto layout = detail::apply_layout<Layout_>(Layout);
            constexpr auto base_usage = detail::inverse_apply_layout_on_usage<Layout>(Usage, tree_shape<V>);

            return FWD(self, base) | rzx::simplified_data<base_usage>;
        }

        template<derived_from<relayout_view> Self>
        friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
        {
            constexpr auto base_layout = rzx::simplified_layout<V>;
            return detail::apply_layout<Layout>(base_layout);
        }
    };

    template<typename V, auto Layout>
    relayout_view(V, constant_t<Layout>) -> relayout_view<V, Layout>;

    template<auto Layout>
    struct detail::relayout_t : adaptor_closure<relayout_t<Layout>>
    {
        template<typename T>
        constexpr auto operator()(T&& t)const
        {
            constexpr auto simplified_layout = detail::simplify_layout<Layout, tree_shape_t<T>>();
            if constexpr(wrapped<T>)
            {
                if constexpr(std::is_object_v<T> && std::is_object_v<decltype(t.base)>)
                {
                    return relayout_view<std::decay_t<decltype(t.base)>, simplified_layout>{ FWD(t, base) };
                }
                else
                {
                    return relayout_view<decltype(FWD(t, base)), simplified_layout>{ FWD(t, base) };
                }
            }
            else
            {
                return relayout_view<T, simplified_layout>{ FWD(t) };
            }
        }
    };
}

namespace rzx 
{
    namespace detail 
    {
        template<typename TLayout, size_t N>
        constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
        {
            if constexpr(indexical_array<TLayout>)
            {
                return rzx::array_cat(prefix, layout);
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                return rzx::make_tuple(detail::layout_add_prefix(layout | child<I>, prefix)...);
            }(std::make_index_sequence<child_count<TLayout>>{});
        }
    }

    template<typename T>
    constexpr auto default_layout = []()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(detail::layout_add_prefix(default_layout<child_type<T, I>>, array{I})...);
        }(std::make_index_sequence<child_count<T>>{});    
    }();

    template<class Relayouter>
    struct relayouter_interface;

    template<class Relayouter>
    struct relayouter_interface : adaptor_closure<Relayouter>
    {
        template<typename V, derived_from<Relayouter> Self>
        constexpr auto operator()(this Self&& self, V&& view)
        {
            constexpr auto layout = detail::simplify_layout<Relayouter::relayout(default_layout<V>), tree_shape_t<V>>();
            return FWD(view) | relayout<layout>;
        }
    };

    namespace detail
    {
        template<size_t N>
        struct repeat_t : relayouter_interface<repeat_t<N>>
        {
            static constexpr auto relayout(const auto&)
            {
                return []<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, I - I>{}... };
                }(std::make_index_sequence<N>{});
            }
        };
    }

    template<size_t N>
    inline constexpr detail::repeat_t<N> repeat{};

    namespace detail
    {
        template<size_t I, size_t Axis>
        struct component_t : relayouter_interface<component_t<I, Axis>>
        {
            template<typename TLayout>
            static constexpr auto relayout(const TLayout& layout)
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
                        return make_tuple(component_t<I, Axis - 1uz>::relayout(layout | child<J>)...);
                    }(std::make_index_sequence<child_count<TLayout>>{});
                }
            }
        };
    }

    template<size_t I, size_t Axis>
    inline constexpr detail::component_t<I, Axis> component{}; 

    namespace detail
    {
        template<size_t Axis1, size_t Axis2>
        struct transpose_t : relayouter_interface<transpose_t<Axis1, Axis2>>
        {
            template<typename TLayout>
            static constexpr auto relayout(const TLayout& layout)
            {
                if constexpr (Axis1 == 0uz)
                {
                    constexpr size_t N = tensor_shape<TLayout>[Axis2];
                    return[&]<size_t...I>(std::index_sequence<I...>)
                    {
                        return rzx::make_tuple(component_t<I, Axis2>::relayout(layout)...);
                    }(std::make_index_sequence<N>{});
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return rzx::make_tuple(transpose_t<Axis1 - 1uz, Axis2 - 1uz>::relayout(layout | child<I>)...);
                }(std::make_index_sequence<child_count<TLayout>>{});
            }
        };
    }

    template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
    inline constexpr detail::transpose_t<Axis1, Axis2> transpose{}; 
}

namespace rzx::detail 
{

}

#include "macro_undef.hpp"
#endif