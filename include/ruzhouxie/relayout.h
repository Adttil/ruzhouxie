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
    template<typename T, auto Layout>
    struct detail::relayout_view : view_base<relayout_view<T, Layout>>
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
                else if constexpr(indicesoid<child_type<layout_type, I>>)
                {
                    constexpr auto index_pack = Layout | child<I>;
                    return FWD(self, raw_tree) | child<index_pack>;
                }
                else
                {
                    return relayout_view<decltype(FWD(self, raw_tree)), Layout | child<I>>
                    {
                        {}, FWD(self, raw_tree)
                    };
                }
            }

            template<auto Indices, typename TLayout>
            static constexpr auto mapped_indices(const TLayout& layout)
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
            static constexpr auto mapped_layout(const Trans& trans)
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

            template<auto Seq, specified<relayout_view> Self>
            RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
            {
                // constexpr auto transformed_sequence = []<size_t...I>(std::index_sequence<I...>)
                // {
                //     return tuple{ Layout | child<Seq | child<I>> ... };
                // }(std::make_index_sequence<child_count<decltype(Seq)>>{});

                constexpr auto transformed_sequence = mapped_layout<Seq>(Layout);
                return FWD(self, raw_tree) | get_tape<transformed_sequence>;
            }
    };
    
    namespace detail
    {
        template<auto Layout>
        struct relayout_t
        {
            using layout_type = purified<decltype(Layout)>;

            template<typename T>
            RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
            {
                if constexpr (indicesoid<layout_type>)
                {
                    return FWD(t) | child<Layout>;
                }
                else
                {
                    return relayout_view<T, Layout>{ {}, FWD(t) };
                }
            }
        };

        template<size_t N>
        struct repeat_t
        {
            static constexpr auto layout = []<size_t...I>(std::index_sequence<I...>)
            {
                return tuple{ array<size_t, I - I>{}... };
            }(std::make_index_sequence<N>{});

            template<typename T>
            RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
            {
                return relayout_view<T, layout>{ {}, FWD(t) };
            }
        };
    };

    inline namespace functors
    {
        template<auto Layout>
        inline constexpr tree_adaptor_closure<detail::relayout_t<Layout>> relayout{};

        template<size_t N>
        inline constexpr tree_adaptor_closure<detail::repeat_t<N>> repeat{};
    }
}

namespace ruzhouxie
{
    namespace detail
    {
        template<typename TLayout, size_t N>
            constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
        {
            if constexpr(indicesoid<TLayout>)
            {
                return detail::concat_array(prefix, layout);
            }
            else return[&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(layout_add_prefix(layout | child<I>, prefix)...);
            }(std::make_index_sequence<child_count<TLayout>>{});
        }
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
        template<typename View, specified<Impl> Self>
        RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this Self&& self, View&& view)
        {
            constexpr auto layout = self.relayout(default_layout<View>);
            return detail::relayout_view<View, layout>
            {
                {}, FWD(view)
            };
        }
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
                    return make_tuple(component<I, Axis - 1uz>.relayout(layout | child<J>)...);
                }(std::make_index_sequence<child_count<TLayout>>{});
            }
        }
            // RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
            // {
            //     if constexpr (Axis == 0)
            //     {
            //         return t | child<I>;
            //     }
            //     else
            //     {
            //         constexpr auto tensor_layout = default_layout<T>;
            //         return relayout_view<T, layouts::component_copy<I, Axis>(tensor_layout)>
            //         {
            //             {}, FWD(t)
            //         };
            //     }
            // }
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
        static consteval auto relayout(const TLayout& layout)
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

//transpose
namespace ruzhouxie
{
    //todo...add Aixs
    template<size_t Begin, size_t Count/*, size_t Axis = 0uz*/, typename T = void>
    constexpr auto range_copy(const T& t)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple<purified<decltype(t | child<normalize_index(Begin + I, child_count<T>)>)>...>
            {
                t | child<normalize_index(Begin + I, child_count<T>)>...
            };
        }(std::make_index_sequence<Count>{});
    }

    namespace detail
    {
        template<size_t Begin, size_t Count>
        struct span_t
        {
            template<typename T>
            RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
            {
                constexpr auto tensor_layout = default_layout<T>;
                return relayout_view<T, range_copy<Begin, Count>(tensor_layout)>
                {
                    {}, FWD(t)
                };
            }

            template<size_t I, specified<span_t> T> requires(I < Count)
            friend constexpr auto tag_invoke(tag_t<child<I>>, T&&)noexcept
            {
                return constant_t<Begin + I>{};
            }
        };

        
    };

    template<size_t Begin, size_t Count>
    inline constexpr tree_adaptor_closure<detail::span_t<Begin, Count>> span{};
    
    
}

#include "macro_undef.h"
#endif