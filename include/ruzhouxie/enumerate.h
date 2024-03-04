#ifndef RUZHOUXIE_ENUMERATE_H
#define RUZHOUXIE_ENUMERATE_H

#include "general.h"
#include "get.h"
#include "relayout.h"
#include "transform.h"

#include "macro_define.h"

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

    inline constexpr pipe_closure<detail::enumerate_t> enumerate{};
}

#include "macro_undef.h"
#endif