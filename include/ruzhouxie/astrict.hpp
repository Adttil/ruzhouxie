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

    template<typename V, auto StrictureTable>
    struct astrict_view : detail::astrict_view_storage<V, StrictureTable>, view_interface<astrict_view<V, StrictureTable>>
    {
        // template<typename Self>
        // constexpr decltype(auto) self(this Self&& self)
        // {
        //     if constexpr(std::is_object_v<Self> && std::is_reference_v<V>)
        //     {
        //         return astrict_view{ FWD(self) };
        //     }
        //     else
        //     {
        //         return FWD(self);
        //     }
        // }

        template<size_t I, typename Self>
        constexpr decltype(auto) get(this Self&& self)
        {            
            if constexpr (I >= child_count<V>)
            {
                return end();
            }
            else if constexpr(std::same_as<decltype(StrictureTable), stricture_t>)
            {
                if constexpr(StrictureTable == stricture_t::readonly)
                {
                    return std::as_const(self.base) | child<I>;
                }
                else
                {
                    return FWD(self, base) | child<I>;
                }
            }
            else if constexpr(terminal<child_type<V, I>>)
            {
                if constexpr(std::is_object_v<decltype(FWD(self, base) | child<I>)> || I >= child_count<decltype(StrictureTable)>)
                {
                    return FWD(self, base) | child<I>;
                }
                else
                {
                    return FWD(self, base) | child<I> | astrict<StrictureTable | child<I>>;
                } 
            }
            else if constexpr(I >= child_count<decltype(StrictureTable)>)
            {
                return FWD(self, base) | child<I>;
            }
            else
            {
                //can simplify.
                return astrict_view<decltype(FWD(self, base) | child<I>), StrictureTable | child<I>>
                {
                    FWD(self, base) | child<I>
                };
            } 
        }

        template<auto UsageTable, typename Self>
        constexpr auto simplifier(this Self&& self)
        {
            struct simplifier_t
            {
                decltype(FWD(self, base)) base;
                
                static constexpr auto layout(){ return simplified_layout<decltype(FWD(self, base)), UsageTable>; }

                constexpr decltype(auto) data()const
                {
                    return astrict_view<decltype(FWD(base) | rzx::simplified_data<UsageTable>), StrictureTable>
                    {
                        FWD(base) | rzx::simplified_data<UsageTable>
                    };
                }
            };
            return simplifier_t{ FWD(self, base) };
        }

        // template<auto UsageTable, typename Self>
        // constexpr decltype(auto) simplified_data(this Self&& self)
        // {
        //     return astrict_view<decltype(FWD(self, base) | rzx::simplified_data<UsageTable>), Stricture>
        //     {
        //         FWD(self, base) | rzx::simplified_data<UsageTable>
        //     };
        // }

        // template<auto Usage, derived_from<astrict_view> Self>
        // friend constexpr decltype(auto) get_simplified_layout(type_tag<Self>)
        // {
        //     return rzx::simplified_layout<V>;
        // }
    };


    template<auto Stricture>
    struct detail::astrict_t : adaptor_closure<astrict_t<Stricture>>
    {
        template<typename V>
        constexpr decltype(auto) operator()(V&& view)const
        {
            //return astrict_view<unwrap_t<V>, Stricture>{ unwrap(FWD(view)) };
            
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