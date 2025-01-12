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
        template<typename V, auto FoldedLayout>
        struct relayout_view_storage
        {
            RUZHOUXIE(no_unique_address) V                        base;
            RUZHOUXIE(no_unique_address) constant_t<FoldedLayout> layout;
        };

        enum class relayout_view_child_strategy
        {
            none,
            indices,
            child,
            relayout
        };
    }

    template<typename V, auto FoldedLayout>
    struct relayout_view : detail::relayout_view_storage<V, FoldedLayout>
    {
    private:
        static constexpr auto unfolded_layout = detail::unfold_layout<FoldedLayout>(tree_shape<V>);

        template<size_t I, typename Self>
        static consteval choice_t<detail::relayout_view_child_strategy> child_choose()
        {
            using strategy_t = detail::relayout_view_child_strategy;
            using layout_type = decltype(FoldedLayout);

            if constexpr(indexical_array<layout_type>)
            {
                if constexpr(I < child_count<child_type<V, FoldedLayout>>)
                {
                    return { strategy_t::indices, noexcept(FWD(std::declval<Self>(), base) | child<FoldedLayout> | child<I>) };
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
                if constexpr(requires{ FWD(std::declval<Self>(), base) | child<FoldedLayout | child<I>>; })
                {
                    return { strategy_t::child, noexcept(FWD(std::declval<Self>(), base) | child<FoldedLayout | child<I>>) };
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
        // template<class Self>
        // constexpr decltype(auto) self(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_reference_v<V>)
        //     {
        //         return relayout_view{ FWD(self) };
        //     }
        //     else
        //     {
        //         return FWD(self);
        //     }
        // }

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
                return FWD(self, base) | child<FoldedLayout> | child<I>;
            }
            else if constexpr(strategy == strategy_t::child)
            {
                constexpr auto index_pack = FoldedLayout | child<I>;
                return FWD(self, base) | child<index_pack>;
            }
            else if constexpr(strategy == strategy_t::relayout)
            {
                return relayout_view<decltype(FWD(self, base)), FoldedLayout | child<I>>
                {
                    FWD(self, base)
                };
            }
            else
            {
                static_assert(strategy == strategy_t::none, "Should not reach.");
            }
        }

        template<auto UnfoldedUsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            struct simplifier_t
            {
                Self&& self;

                static constexpr auto base_simplifer(Self&& self)
                {
                    constexpr auto base_usage = detail::inverse_apply_layout_on_usage<unfolded_layout>(UnfoldedUsageTable, tree_shape<V>);
                    if constexpr(std::is_reference_v<V>)
                    {
                        return FWD(self, base) | refer | rzx::simplifier<base_usage>;
                    }
                    else
                    {
                        return FWD(self, base) | rzx::simplifier<base_usage>;
                    }
                }

                using base_simplifer_t = decltype(base_simplifer(std::declval<Self>()));

                static consteval auto layout()
                {
                    constexpr auto base_layout = detail::unfold_layout_by_relayouted_shape(base_simplifer_t::layout(), tree_shape<V>);
                    return detail::apply_layout<FoldedLayout>(base_layout);
                }
                
                static consteval auto stricture_table()
                {
                    //constexpr auto unfolded_base_stricture_table = detail::unfold_stricture_table(base_simplifer_t::stricture_table(), tree_shape<V>);
                    //return detail::apply_layout<FoldedLayout>(unfolded_base_stricture_table);
                }

                static consteval auto operation_table()
                {

                    //return detail::apply_layout<FoldedLayout>(base_simplifer_t::stricture_table());
                    return no_operation;
                }

                constexpr decltype(auto) data()const
                {
                    return base_simplifer(FWD(self)).data();
                    // if constexpr(std::is_reference_v<V>)
                    // {
                    //     return FWD(self, base) | refer | simplified_data<base_usage()>;
                    // }
                    // else
                    // {
                    //     return FWD(self, base) | simplified_data<base_usage()>;
                    // }
                }
            };

            return simplifier_t{ FWD(self) };
        }
        

        // template<auto Usage, typename Self>
        // constexpr decltype(auto) simplified_data(this Self&& self)
        // {
        //     constexpr auto layout = detail::unfold_layout(Layout, tree_shape<Self>);
        //     constexpr auto base_usage = detail::inverse_apply_layout_on_usage<layout>(Usage, tree_shape<V>);

        //     return FWD(self, base) | rzx::simplified_data<base_usage>;
        // }

        // template<auto Usage, derived_from<relayout_view> Self>
        // friend constexpr auto get_simplified_layout(type_tag<Self>)
        // {
        //     constexpr auto layout = detail::unfold_layout(Layout, tree_shape<Self>);
        //     constexpr auto base_usage = detail::inverse_apply_layout_on_usage<layout>(Usage, tree_shape<V>);
        //     constexpr auto base_layout = detail::unfold_layout(rzx::simplified_layout<V, base_usage>, tree_shape<V>);

        //     return detail::apply_layout<Layout>(base_layout);
        // }
    };

    template<typename V, auto FoldedLayout>
    relayout_view(V, constant_t<FoldedLayout>) -> relayout_view<V, FoldedLayout>;

    template<auto Layout>
    struct detail::relayout_t : adaptor_closure<relayout_t<Layout>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            constexpr auto folded_layout = detail::fold_layout<Layout>(tree_shape<T>);
            if constexpr(indexical<decltype(folded_layout)>)
            {
                return decltype(wrap(FWD(t) | child<folded_layout>)){ rzx::unwrap(FWD(t) | child<folded_layout>) };
                
                //using type = decltype(FWD(t) | child<simplified_layout>);
                //return std::remove_cvref_t<type>{ FWD(t) | child<simplified_layout> };
                
                // if constexpr(std::is_rvalue_reference_v<type>)
                // {
                //     return std::remove_cvref_t<type>{ FWD(t) | child<simplified_layout> };
                // }
                // else
                // {
                //     return FWD(t) | child<simplified_layout>;
                // }
            }
            else
            {
                return relayout_view<unwrap_t<T>, folded_layout>{ rzx::unwrap(FWD(t)) };
            }
            // else if constexpr(wrapped<T>)
            // {
            //     if constexpr(std::is_object_v<T> && std::is_object_v<decltype(t.base)>)
            //     {
            //         return relayout_view<std::decay_t<decltype(t.base)>, simplified_layout>{ FWD(t, base) };
            //     }
            //     else
            //     {
            //         return relayout_view<decltype(FWD(t, base)), simplified_layout>{ FWD(t, base) };
            //     }
            // }
            // else
            // {
            //     return relayout_view<T, simplified_layout>{ FWD(t) };
            // }
        }
    };
}

namespace rzx 
{
    namespace detail 
    {
        
    }

    template<typename T>
    constexpr auto default_unfolded_layout = []()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return rzx::make_tuple(detail::layout_add_prefix(default_unfolded_layout<child_type<T, I>>, array{I})...);
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
            constexpr auto layout = Relayouter::relayout(default_unfolded_layout<V>);
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
            static constexpr auto relayout(const TLayout& unfolded_layout)
            {
                if constexpr (Axis == 0uz)
                {
                    static_assert(I < child_count<TLayout>, "Component index out of range.");
                    return unfolded_layout | child<I>;
                }
                else
                {
                    static_assert(branched<TLayout>, "Axis index out of range.");
                    return[&]<size_t...J>(std::index_sequence<J...>)
                    {
                        return make_tuple(component_t<I, Axis - 1uz>::relayout(unfolded_layout | child<J>)...);
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
            static constexpr auto relayout(const TLayout& unfolded_layout)
            {
                if constexpr (Axis1 == 0uz)
                {
                    constexpr size_t N = tensor_shape<TLayout>[Axis2];
                    return[&]<size_t...I>(std::index_sequence<I...>)
                    {
                        return rzx::make_tuple(component_t<I, Axis2>::relayout(unfolded_layout)...);
                    }(std::make_index_sequence<N>{});
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return rzx::make_tuple(transpose_t<Axis1 - 1uz, Axis2 - 1uz>::relayout(unfolded_layout | child<I>)...);
                }(std::make_index_sequence<child_count<TLayout>>{});
            }
        };
    }

    template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
    inline constexpr detail::transpose_t<Axis1, Axis2> transpose{}; 

    namespace detail
    {
        struct inverse_t : relayouter_interface<inverse_t>
        {
            template<typename TLayout>
            static constexpr auto relayout(const TLayout& unfolded_layout)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto last_index = child_count<TLayout> - 1uz;
                    return rzx::make_tuple(unfolded_layout | child<last_index - I> ...);
                }(std::make_index_sequence<child_count<TLayout>>{});
            }
        };
    }

    inline constexpr detail::inverse_t inverse{}; 

    namespace detail
    {
        struct zip_t : adaptor<zip_t>
        {
            template<typename...T>
            constexpr auto result(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... } | transpose<>;
            }
        };
    }

    inline constexpr detail::zip_t zip{};
}

namespace rzx 
{  
    // namespace detail
    // {
    //     template<auto UsageTable>
    //     struct simplify_t;
    // }

    // template<auto UsageTable = usage_t::repeatedly>
    // inline constexpr detail::simplify_t<UsageTable> simplify{};

    // template<auto UsageTable>
    // struct detail::simplify_t : adaptor_closure<simplify_t<UsageTable>>
    // {
    //     template<typename T>
    //     constexpr auto operator()(T&& t)const
    //     {
    //         auto simplifier = FWD(t) | rzx::simplifier<UsageTable>;
    //         using data_type = decltype(simplifier.data());
    //         constexpr auto layout = detail::fold_layout<simplifier.layout()>(tree_shape<data_type>);
    //         if constexpr(equal(layout, indexes_of_whole))
    //         {
    //             return simplifier.data();
    //         }
    //         else
    //         {
    //             return relayout_view<data_type, layout>{ simplifier.data() };
    //         }
    //     }
    // };
}

#include "macro_undef.hpp"
#endif