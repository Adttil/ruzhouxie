#ifndef RUZHOUXIE_ASTRICT_HPP
#define RUZHOUXIE_ASTRICT_HPP

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
        struct astrict_t;
    }

    template<auto StrictureTable>
    inline constexpr detail::astrict_t<StrictureTable> astrict{};

    namespace detail
    {
        template<typename V, auto Stricture>
        struct astrict_view_storage
        {
            RUZHOUXIE(no_unique_address) V                     base;
            RUZHOUXIE(no_unique_address) constant_t<Stricture> stricture;
        };
    }

    template<typename V, auto Stricture>
    struct astrict_view : detail::astrict_view_storage<V, Stricture>, view_interface<astrict_view<V, Stricture>>
    {
        template<typename Self>
        constexpr decltype(auto) self(this Self&& self)
        {
            if constexpr(/*std::is_object_v<Self> && */std::is_reference_v<V>)
            {
                return astrict_view{ FWD(self) };
            }
            else
            {
                return FWD(self);
            }
        }

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
                    return FWD(self, base) | child<I> | astrict<Stricture>;
                }
                else if constexpr(I >= child_count<decltype(Stricture)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else
                {
                    return FWD(self, base) | child<I> | astrict<Stricture | child<I>>;
                } 
            }
            else if constexpr(std::same_as<decltype(Stricture), stricture_t>)
            {                
                //can simplify.
                return astrict_view<decltype(FWD(self, base) | child<I>), Stricture>
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
                return astrict_view<decltype(FWD(self, base) | child<I>), Stricture | child<I>>
                {
                    FWD(self, base) | child<I>
                };
            } 
        }

        template<auto UsageTable, typename Self>
        constexpr decltype(auto) simplified_data(this Self&& self)
        {
            return astrict_view<decltype(FWD(self, base) | rzx::simplify<UsageTable>), Stricture>
            {
                FWD(self, base) | rzx::simplify<UsageTable>
            };
        }

        template<derived_from<astrict_view> Self>
        friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
        {
            return rzx::simplified_layout<V>;
        }
    };


    template<auto Stricture>
    struct detail::astrict_t : adaptor_closure<astrict_t<Stricture>>
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
                return astrict_view<V&&, Stricture>{ FWD(view) };
            }
        }
    };
}

#include "macro_undef.hpp"
#endif