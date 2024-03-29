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
    namespace detail
    {
        template<typename View>
        struct convert_wrapper
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

            RUZHOUXIE_INLINE friend constexpr bool operator==(const view_base&, const view_base&) = default;
	    };
    }
    
    template<typename View>
    struct view_interface
    {
        template<typename Self>
        RUZHOUXIE_INLINE constexpr auto operator+(this Self&& self)noexcept
        {
            return detail::convert_wrapper<Self&&>{ FWD(self) };
        }

        static RUZHOUXIE_CONSTEVAL size_t size()noexcept
        {
            return child_count<View>;
        }

        static RUZHOUXIE_CONSTEVAL size_t shape()noexcept
        {
            return tree_shape<View>;
        }

        static RUZHOUXIE_CONSTEVAL size_t tensor_rank()noexcept
        {
            return rzx::tensor_rank<View>;
        }

        static RUZHOUXIE_CONSTEVAL size_t tensor_shape()noexcept
        {
            return rzx::tensor_shape<View>;
        }

        RUZHOUXIE_INLINE friend constexpr bool operator==(const view_interface&, const view_interface&) = default;
    };

    template<typename T>
    concept tree_view = std::derived_from<purified<T>, view_interface<purified<T>>>;
}

//view
namespace ruzhouxie
{
    template<typename T>
    struct view : detail::view_base<T>, view_interface<view<T>>
    {
        using base_type = T;

        template<size_t I, specified<view> Self>
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
            AS_EXPRESSION(getter<T>{}.template get<I>(FWD(self).base()))

        template<auto Seq, specified<view> Self>
            requires (not std::same_as<decltype(Seq), size_t>)//must have this in msvc, but I don't know why.
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
            AS_EXPRESSION(FWD(self).base() | get_tape<Seq>)

        template<std::same_as<view> V>
        friend RUZHOUXIE_CONSTEVAL auto tag_invoke(tag_t<make_tree<V>>)
        {
            return tree_maker<T>{};
        }

        RUZHOUXIE_INLINE friend constexpr bool operator==(const view&, const view&) = default;
    };

    template<typename T>
    view(T&&) -> view<T>;

    template<typename T>
    concept view_instantiated = std::same_as<purified<T>, view<typename purified<T>::base_type>>;

    namespace detail
    {
        struct as_ref_t
        {
            template<typename T> requires (not view_instantiated<T>)
            RUZHOUXIE_INLINE constexpr T& operator()(T& t) const noexcept
            {
                return t;
            }
            
            template<typename T> requires std::is_rvalue_reference_v<T&&> && (not view_instantiated<T>)
            RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const AS_EXPRESSION
            (
                view<T&&>{ FWD(t) }
            )

            template<view_instantiated T>
            RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const AS_EXPRESSION
            (
                (*this)(FWD(t).base())
            )
        };
    };

    inline constexpr tree_adaptor_closure<detail::as_ref_t> as_ref{};
}

namespace ruzhouxie 
{
    template<size_t I = 0uz, typename L, typename R>
    RUZHOUXIE_INLINE constexpr bool tree_equal(const L& l, const R& r)noexcept
    {
		if constexpr(terminal<L>)
		{
			return l == r;
		}
        else if constexpr(I >= child_count<L>)
		{
			return true;
		}
		else
		{ 
			if (not tree_equal(l | child<I>, r | child<I>))
			{
				return false;
			}
			else
			{
				return tree_equal<I + 1uz>(l, r);
			}
		}
    }

	template<typename L, typename R> requires tree_view<L> || tree_view<R>
    RUZHOUXIE_INLINE constexpr bool operator==(const L& l, const R& r)noexcept
    {
        return rzx::tree_equal(l, r);
    }
}

#include "macro_undef.h"
#endif