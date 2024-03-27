#ifndef RUZHOUXIE_INVOKE_H
#define RUZHOUXIE_INVOKE_H

#include "general.h"
#include "get.h"
#include "tree_adaptor.h"
#include "tape.h"
#include "tree_view.h"
#include "relayout.h"

#include "macro_define.h"

namespace ruzhouxie::detail
{
    template<auto Indices, typename V>
    RUZHOUXIE_CONSTEVAL auto indices_adapt_truncate()
    {
        if constexpr(terminal<V>)
        {
            return indices_of_whole_view;
        }
        else if constexpr(Indices.size() == 0uz)
        {
            return indices_of_whole_view;
        }
        else
        {
            constexpr size_t i = Indices[0];
            return detail::concat_array(array{ i }, indices_adapt_truncate<detail::array_drop<1uz>(Indices), child_type<V, i>>());
        }
    }

    template<auto Seq, typename V>
    RUZHOUXIE_CONSTEVAL auto sequence_adapt_truncate()
    {
        if constexpr(indicesoid<decltype(Seq)>)
        {
            return indices_adapt_truncate<Seq, V>();
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(sequence_adapt_truncate<Seq | child<I>, V>()...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }

    template<typename TSeq>
    RUZHOUXIE_CONSTEVAL auto sequence_flatten(const TSeq& seq)
    {
        if constexpr(indicesoid<TSeq>)
        {
            return rzx::make_tuple(seq);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return detail::concat_to_tuple(sequence_flatten(seq | child<I>)...);
        }(std::make_index_sequence<child_count<TSeq>>{});
    }

    template<auto Indices, typename V>
    RUZHOUXIE_CONSTEVAL auto indices_adapat_to_leaf()
    {
        if constexpr(terminal<child_type<V, Indices>>)
        {
            return make_tuple(Indices);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return detail::concat_to_tuple(indices_adapat_to_leaf<detail::concat_array(Indices, array{ I }), V>()...);
        }(std::make_index_sequence<child_count<V>>{});
    }

    template<auto Seq, typename V>
    RUZHOUXIE_CONSTEVAL auto sequence_adapat_to_leaf()
    {
        if constexpr(indicesoid<decltype(Seq)>)
        {
            constexpr auto indices = indices_adapt_truncate<Seq, V>();
            return indices_adapat_to_leaf<indices, V>();
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return detail::concat_to_tuple(sequence_adapat_to_leaf<Seq | child<I>, V>()...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }

    template<typename TSeq, size_t Num = 0uz, size_t I = 0uz>
    RUZHOUXIE_CONSTEVAL auto sequence_flatten_map()
    {
        if constexpr(indicesoid<TSeq>)
        {
            return array{ Num };
        }
        else if constexpr(I >= child_count<TSeq>)
        {
            return tuple{};
        }
        else
        {
            constexpr auto child_map = sequence_flatten_map<child_type<TSeq, I>, Num>();
            constexpr auto rest_map = sequence_flatten_map<TSeq, Num + leaf_count<decltype(child_map)>, I + 1uz>();
            return detail::concat_to_tuple(make_tuple(child_map), rest_map);
        }
    }

    RUZHOUXIE_CONSTEVAL auto sequence_zip(const auto& seq1, const auto& seq2)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(make_tuple(seq1 | child<I>, seq2 | child<I>)...);
        }(std::make_index_sequence<child_count<decltype(seq1)>>{});
    }

    RUZHOUXIE_CONSTEVAL auto sequence_unzip(const auto& seq_zip)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(make_tuple(seq_zip | child<I, 0uz>...), make_tuple(seq_zip | child<I, 1uz>...));
        }(std::make_index_sequence<child_count<decltype(seq_zip)>>{});
    }
    
    template<typename TArr>
    RUZHOUXIE_CONSTEVAL auto index_array_to_sequence(const TArr& arr)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple{ array{ arr[I] }... };
        }(std::make_index_sequence<std::tuple_size_v<TArr>>{});
    }

    template<typename S>
    RUZHOUXIE_CONSTEVAL auto invoke_view_to_base_sequence(const S& seq)
    {
        if constexpr(indicesoid<S>)
        {
            return tuple{ concat_array(array{ 0uz }, seq), concat_array(array{ 1uz }, seq) };
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(invoke_view_to_base_sequence(seq | child<I>)...);
        }(std::make_index_sequence<child_count<S>>{});
    }
}

namespace ruzhouxie::detail
{
    struct invoke_t
    {
        template<typename F>
        static RUZHOUXIE_CONSTEVAL auto init_in_tape_map()
        {
            if constexpr(terminal<F>)
            {
                return array{ invalid_index };
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(init_in_tape_map<child_type<F, I>>()...);
            }(std::make_index_sequence<child_count<F>>{});
        }

        template<auto Seq, auto Map, size_t I = 0uz, auto Cur = tuple{}>
        static RUZHOUXIE_CONSTEVAL auto get_in_tape_seq_and_map()
        {
            if constexpr(I >= child_count<decltype(Seq)>)
            {
                struct result_t
                {
                    purified<decltype(Cur)> sequence;
                    purified<decltype(Map)> map; 
                };
                return result_t{ Cur, Map };
            }
            else if constexpr(indicesoid<decltype(Seq | child<I>)>)
            {
                constexpr auto indices = Seq | child<I>;
                //todo...
                constexpr size_t map = child<indices>(Map)[0uz];
                if constexpr(map != invalid_index)
                {
                    return get_in_tape_seq_and_map<Seq, Map, I + 1uz, Cur>();
                }
                else
                {
                    constexpr auto new_map = []()
                    {
                        constexpr auto indices = Seq | child<I>;
                        auto map = Map;
                        child<indices>(map)[0uz] = child_count<decltype(Cur)>;
                        return map;
                    }();
                    constexpr auto new_cur = detail::concat_to_tuple(Cur, make_tuple(indices));
                    return get_in_tape_seq_and_map<Seq, new_map, I + 1uz, new_cur>();
                }
            }
            else
            {
                static_assert(I >= child_count<decltype(Seq)>, "invalid seq");
            }
        }

        template<auto UniqueInSeqZip>
        RUZHOUXIE_INLINE static constexpr auto get_result_vec(auto&& base_tape, auto&& fn_tape)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto unzip_unique_seq = detail::sequence_unzip(UniqueInSeqZip);
            
                constexpr auto base_tape_unique_seq = unzip_unique_seq.template get<0uz>();
                constexpr auto fn_tape_unique_seq = unzip_unique_seq.template get<1uz>();
                
                auto base_unique_tape = make_tape<base_tape_unique_seq>(FWD(base_tape, data));
                auto fn_unique_tape = make_tape<fn_tape_unique_seq>(FWD(fn_tape, data));
                
                return combine
                (
                    pass<I>(FWD(fn_unique_tape))(pass<I>(FWD(base_unique_tape)))...
                );
            }(std::make_index_sequence<child_count<decltype(UniqueInSeqZip)>>{});
        }

        template<typename V, typename F>
        RUZHOUXIE_INLINE constexpr auto operator()(V&& view, F&& fn)const
        {
            constexpr auto fn_layout = default_layout<F>;
            constexpr auto adapt_flat_seq = detail::sequence_flatten(fn_layout);
            constexpr auto in_tape_seq_and_map = get_in_tape_seq_and_map<adapt_flat_seq, init_in_tape_map<F>()>();
            constexpr auto in_tape_seq = in_tape_seq_and_map.sequence;
            constexpr auto out_tape_seq = detail::mapped_layout<fn_layout>(in_tape_seq_and_map.map);

            auto view_tape = FWD(view) | get_tape<in_tape_seq>;
            auto fn_tape = FWD(fn) | get_tape<in_tape_seq>;

            constexpr auto base_tape_seq = decltype(view_tape)::sequence;
            constexpr auto fn_tape_seq = decltype(fn_tape)::sequence;

            constexpr auto in_tape_seq_zip = detail::sequence_zip(base_tape_seq, fn_tape_seq);
            constexpr auto unique_in_seq_and_map = detail::get_unique_seq_and_map<in_tape_seq_zip>();
            constexpr auto unique_in_seq_zip = unique_in_seq_and_map.sequence;
            constexpr auto unique_in_map = detail::index_array_to_sequence(unique_in_seq_and_map.map);
            constexpr auto result_vec_seq = detail::mapped_layout<out_tape_seq>(unique_in_map);

            return relayout_view{ 
                get_result_vec<unique_in_seq_zip>(std::move(view_tape), std::move(fn_tape)),
                constant_t<result_vec_seq>{}
            };
        }
    };
}

namespace ruzhouxie
{
    constexpr inline tree_adaptor<detail::invoke_t> invoke{};

    constexpr inline auto transform = tree_adaptor{ [](auto&& view, auto&& fn)
    {
        return invoke(FWD(view), FWD(fn) | repeat<child_count<decltype(view)>>);
    }};

    constexpr inline auto zip_transform = []<typename F, typename...V>(F&& fn, V&&...views)
    {
        return zip(FWD(views)...) | transform(
                    [fn = FWD(fn)](auto&& zip_args)->decltype(auto)
                    {
                        return [&]<size_t...I>(std::index_sequence<I...>)->decltype(auto)
                        {
                            return fn(FWD(zip_args) | child<I> ...);
                        }(std::index_sequence_for<V...>{});
                    }
                );
    };

    template<typename V>
    constexpr auto single_value_tree_layout()
    {
        if constexpr(terminal<V>)
        {
            return indices_of_whole_view;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(single_value_tree_layout<child_type<V, I>>()...);
        }(std::make_index_sequence<child_count<V>>{});
    }

    constexpr inline auto tree_transform = tree_adaptor{ [](auto&& view, auto&& fn)
    {
        return invoke(FWD(view), FWD(fn) | relayout<single_value_tree_layout<decltype(view)>()>);
    }};

    constexpr inline auto tree_zip_transform = []<typename F, typename...V>(F&& fn, V&&...views)
    {
        return zip(FWD(views)...) | transform(
                    [fn = FWD(fn)](auto&& zip_args)->decltype(auto)
                    {
                        return [&]<size_t...I>(std::index_sequence<I...>)->decltype(auto)
                        {
                            return fn(FWD(zip_args) | child<I> ...);
                        }(std::index_sequence_for<V...>{});
                    }
                );
    };

    namespace detail 
    {
        struct grouped_cartesian_transform_t
        {
            template<typename F, typename V1, typename V2>
            RUZHOUXIE_INLINE constexpr auto operator()(F&& fn, V1&& view1, V2 view2)const
            {
                constexpr auto arg_layout = layout_grouped_cartesian(vector_layout<V1>, vector_layout<V2>);
                constexpr array<array<array<size_t, 0uz>, child_count<V1>>, child_count<V2>> fn_layout{};
                return invoke(
                    relayout_view{ combine(FWD(view1), FWD(view2)), constant_t<arg_layout>{} },
                    relayout_view{ [fn = wrapper{ FWD(fn) }](auto&& args){ return fn.value()(child<0uz>(args), child<1uz>(args)); }, constant_t<fn_layout>{} }
                );
            }
        };
    }

    inline constexpr detail::grouped_cartesian_transform_t grouped_cartesian_transform{};
}

#include "macro_undef.h"
#endif