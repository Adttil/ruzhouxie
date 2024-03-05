#ifndef RUZHOUXIE_SLICE_H
#define RUZHOUXIE_SLICE_H

#include "general.h"
#include "get.h"
#include "pipe_closure.h"
#include "relayout.h"
#include "ruzhouxie/array.h"
#include "ruzhouxie/tuple.h"
#include "tree_view.h"
#include "constant.h"
#include "math.h"

#include "macro_define.h"
#include <concepts>
#include <cstddef>
#include <utility>

namespace ruzhouxie::detail
{
    enum class optional_int_type
    {
        default_value,
        signed_value,
        unsigned_value
    };

    struct optional_int
    {
        std::intmax_t value{};
        bool is_default = true;

        constexpr optional_int() = default;
        constexpr optional_int(std::integral auto value) : value(value), is_default(false) {}

        constexpr optional_int operator-()const
        {
            return is_default ? optional_int{} : optional_int{ -value };
        }
    };

    struct slice_param_t
    {
        optional_int start{};
        optional_int size{};
        optional_int stride{};

        constexpr slice_param_t(std::integral auto value) : start(value), size(0) {}
        constexpr slice_param_t(optional_int start = {}, optional_int size = {}, optional_int stride = {})
            : start(start), size(size), stride(stride)
        {

        }

        constexpr auto normalize(size_t len)const
        {
            //ptrdiff_t stride = size.
            optional_int stride = size.value >= 0 ? this->stride : -this->stride;

            struct result_t
            {
                size_t start;
                size_t size;
                size_t stride;
            } result{};

            return result_t{ 
                static_cast<size_t>(start.value),
                static_cast<size_t>(size.value), 
                static_cast<size_t>(stride.value)
            };
        }
    };

    namespace detail
    {
        template<slice_param_t SliceParam, slice_param_t...Rest>
        struct slice_t;
    }

    template<slice_param_t...SliceParams>
    inline constexpr tree_adaptor_closure<detail::slice_t<SliceParams...>> slice{};

    template<slice_param_t SliceParam, slice_param_t...Rest>
    struct detail::slice_t : relayouter<slice_t<SliceParam, Rest...>>
    {
        template<typename Layout>
        static consteval auto relayout(Layout layout)
        {
            constexpr auto normalized_slice = SliceParam.normalize(child_count<Layout>);
            if constexpr(sizeof...(Rest) == 0uz)
            {
                if constexpr(normalized_slice.size == 0uz)
                {
                    return layout | child<normalized_slice.start>;
                }
                else return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(layout | child<normalized_slice.start + normalized_slice.stride * I>...);
                }(std::make_index_sequence<normalized_slice.size>{});
            }
            else
            {
                if constexpr(normalized_slice.size == 0uz)
                {
                    return slice<Rest...>.relayout(layout | child<normalized_slice.start>);
                }
                else return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(slice<Rest...>.relayout(layout | child<normalized_slice.start + normalized_slice.stride * I>)...);
                }(std::make_index_sequence<normalized_slice.size>{});
            }
        }
    };
}

#include "macro_undef.h"
#endif