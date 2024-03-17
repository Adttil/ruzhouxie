#ifndef RUZHOUXIE_TREE_VIEW_H
#define RUZHOUXIE_TREE_VIEW_H

#include "tree_adaptor.h"
#include "tape.h"
#include "get.h"
#include "processer.h"
#include "macro_define.h"

//view_interface
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
            RUZHOUXIE_INLINE constexpr operator U(this auto&& self)
                noexcept(noexcept(FWD(self, raw_view) | make_tree<U>))
                requires requires{ FWD(self, raw_view) | make_tree<U>; }
            {
                return FWD(self, raw_view) | make_tree<U>;
            }
        };
        
        template<typename V>
        struct view_base
	    {
	        V base_view;

            template<specified<view_base> Self>
	        RUZHOUXIE_INLINE constexpr decltype(auto) base(this Self&& self)noexcept
		    {
		        return FWD(self, base_view);
		    }
	    };
    }
    
    template<typename View>
    struct view_interface
    {
        template<typename Self>
        RUZHOUXIE_INLINE constexpr auto operator+(this Self&& self)noexcept
        {
            return detail::universal_view<Self&&>{ FWD(self) };
        }
    };

    
}

//view
namespace ruzhouxie
{
    template<typename T>
    struct view : detail::view_base<T>, view_interface<view<T>>
    {
        template<size_t I, specified<view> Self> requires (I >= child_count<T>)
        RUZHOUXIE_INLINE friend constexpr void tag_invoke(tag_t<child<I>>, Self&& self){}

        template<size_t I, specified<view> Self> requires (I < child_count<T>)
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
            AS_EXPRESSION(FWD(self).base() | child<I>)

        template<auto Seq, specified<view> Self>
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
            AS_EXPRESSION(FWD(self).base() | get_tape<Seq>)

        template<std::same_as<view> V>
        friend consteval auto tag_invoke(tag_t<make_tree<V>>)
        {
            return tree_maker<T>{};
        }
    };

    template<typename T>
    view(T&&) -> view<T>;

    // template<typename T>
    // struct tree_maker_trait<view<T>>
    // {
    //     struct type : processer<type>
    //     {
    //         static constexpr tree_maker<T> maker{};

    //         template<typename U>
    //         static consteval auto get_sequence()
    //         {
    //             return tree_maker<T>::template get_sequence<U>();
    //         }

    //         template<typename U, size_t Offset, typename Tape>
    //         constexpr auto process_tape(Tape&& tape)const
    //         {
    //             return view<T>{ maker.template process_tape<U, Offset>(FWD(tape)) };
    //         }
    //     };
    // };

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