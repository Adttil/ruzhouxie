#ifndef RUZHOUXIE_VIEW_INTERFACE_HPP
#define RUZHOUXIE_VIEW_INTERFACE_HPP

#include "general.hpp"

#include "macro_define.hpp"

namespace rzx 
{
    template<typename View>
    struct view_interface
    {};

    template<class T>
    concept viewed = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(view_interface<V>&)->V*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };
}

#include "macro_undef.hpp"
#endif