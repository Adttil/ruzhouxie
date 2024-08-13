#ifndef RUZHOUXIE_RESTRICT_HPP
#define RUZHOUXIE_RESTRICT_HPP

#include "constant.hpp"
#include "general.hpp"
#include "simplify.hpp"
#include "view_interface.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    enum class stricture_t
    {
        none,
        readonly
    };

    namespace detail
    {
        template<auto StrictureTable>
        struct restrict_t;
    }

    template<auto StrictureTable>
    inline constexpr detail::restrict_t<StrictureTable> restrict{};

    namespace detail
    {
        template<typename V, auto Stricture>
        struct restrict_view_storage
        {
            RUZHOUXIE(no_unique_address) V                     base;
            RUZHOUXIE(no_unique_address) constant_t<Stricture> stricture;
        };
    }

    template<typename V, auto Stricture>
    struct restrict_view : detail::restrict_view_storage<V, Stricture>, view_interface<restrict_view<V, Stricture>>
    {
        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
        {            
            if constexpr (I >= child_count<V>)
            {
                return end();
            }
            else if constexpr(terminal<child_type<V, I>>)
            {
                if constexpr(std::is_object_v<decltype(FWD(self, base) | child<I>)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else if constexpr(std::same_as<decltype(Stricture), stricture_t>)
                {
                    return FWD(self, base) | child<I> | restrict<Stricture>;
                }
                else if constexpr(I >= child_count<decltype(Stricture)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else
                {
                    return FWD(self, base) | child<I> | restrict<Stricture | child<I>>;
                } 
            }
            else if constexpr(std::same_as<decltype(Stricture), stricture_t>)
            {                
                //can simplify.
                return restrict_view<decltype(FWD(self, base) | child<I>), Stricture>
                {
                    FWD(self, base) | child<I>
                };
            }
            else if constexpr(I >= child_count<decltype(Stricture)>)
            {
                return FWD(self, base) | child<I>;
            }
            else
            {
                //can simplify.
                return restrict_view<decltype(FWD(self, base) | child<I>), Stricture | child<I>>
                {
                    FWD(self, base) | child<I>
                };
            } 
        }

        template<auto Usage, auto Layout, typename Self>
        constexpr decltype(auto) simplify(this Self&& self)
        {
            return restrict_view<decltype(FWD(self, base) | rzx::simplify<Usage, Layout>), apply_layout<Layout>(Stricture)>
            {
                FWD(self, base) | rzx::simplify<Usage, Layout>
            };
        }
    };


    template<auto Stricture>
    struct detail::restrict_t : adaptor_closure<restrict_t<Stricture>>
    {
        template<typename V>
        constexpr decltype(auto) operator()(V&& view)const
        {
            if constexpr(terminal<V>)
            {
                if constexpr(Stricture == stricture_t::none)
                {
                    return FWD(view);
                }
                else if constexpr(Stricture == stricture_t::readonly)
                {
                    return std::as_const(view);
                }
            }
            else
            {
                return restrict_view<V&&, Stricture>{ FWD(view) };
            }
        }
    };
}

#include "macro_undef.hpp"
#endif