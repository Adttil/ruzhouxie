#ifndef RUZHOUXIE_ENUMERATE_H
#define RUZHOUXIE_ENUMERATE_H

#include "general.h"
#include "get.h"
#include "tree_adaptor.h"
#include "relayout.h"
#include "transform.h"

#include "macro_define.h"

namespace ruzhouxie
{
    namespace detail
    {
        template<size_t Start, size_t Size, size_t Stride>
        struct range_view
        {
            template<size_t I, specified<range_view> Self>
		    friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)noexcept
		    {
                if constexpr(I >= Size)
                {
                    return end();
                }
                else
                {
                    return constant_t<Start + Stride * I>{};
                }
            };
        };
    }

    template<size_t Start, size_t Size, size_t Stride = 1uz>
    inline constexpr detail::range_view<Start, Size, Stride> range{};
}

namespace ruzhouxie
{
    namespace detail
    {
        struct enumerate_t
        {
            template<typename View>
		    RUZHOUXIE_INLINE constexpr auto operator()(View&& view)const
			{
			    return zip(range<0uz, child_count<View>>, FWD(view));
			}
        };
    }

    inline constexpr tree_adaptor_closure<detail::enumerate_t> enumerate{};
}

#include "macro_undef.h"
#endif