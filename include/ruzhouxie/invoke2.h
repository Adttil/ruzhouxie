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

namespace ruzhouxie
{
    template<typename V>
    struct invoke_view : detail::view_base<V>, view_interface<invoke_view<V>>
    {
        using base_type = V;

        template<specified<invoke_view> Self>
        RUZHOUXIE_INLINE constexpr auto&& arg_tree(this Self&& self)noexcept
        {
            return FWD(self).base() | child<0uz>;
        }

        template<specified<invoke_view> Self>
        RUZHOUXIE_INLINE constexpr auto&& fn_tree(this Self&& self)noexcept
        {
            return FWD(self).base() | child<1uz>;
        }

        template<size_t I, specified<invoke_view> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
        {
            if constexpr(I >= child_count<child_type<V, 1uz>>)
            {
                return end();
            }      
            else if constexpr(terminal<child_type<V, 1uz, I>>)
            {
                return (FWD(self).fn_tree() | child<I>)(FWD(self).arg_tree() | child<I>);
            }
            else
            {
                using tpl_t = tuple<decltype(FWD(self).arg_tree() | child<I>), decltype(FWD(self).fn_tree() | child<I>)>;
                return invoke_view<tpl_t>{tpl_t
                {
                    FWD(self).arg_tree() | child<I>,
                    FWD(self).fn_tree() | child<I>
                }};
            }
        }

        template<typename T = child_type<V, 1uz>>
        static RUZHOUXIE_CONSTEVAL auto init_in_tape_map()
        {
            if constexpr(terminal<T>)
            {
                return array{ invalid_index };
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(init_in_tape_map<child_type<T, I>>()...);
            }(std::make_index_sequence<child_count<T>>{});
        }

        template<auto Seq, size_t I = 0uz, auto Cur = tuple{}, auto Map = init_in_tape_map()>
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
                    return get_in_tape_seq_and_map<Seq, I + 1uz, Cur, Map>();
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
                    return get_in_tape_seq_and_map<Seq, I + 1uz, new_cur, new_map>();
                }
            }
            else
            {
                static_assert(I >= child_count<decltype(Seq)>, "invalid seq");
            }
        }

        RUZHOUXIE_INLINE static constexpr decltype(auto) apply(auto&& fn_arg)
        {
            return (FWD(fn_arg) | child<1uz>)(FWD(fn_arg) | child<0uz>);
        }

        template<auto UniqueInSeqZip>
        RUZHOUXIE_INLINE static constexpr auto get_result_vec(auto&& base_tape)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {                
                auto base_unique_tape = make_tape<UniqueInSeqZip>(FWD(base_tape, data));
                
                return tuple<decltype(apply(pass<I>(FWD(base_unique_tape))))...>
                {
                    apply(pass<I>(FWD(base_unique_tape)))...
                };
            }(std::make_index_sequence<child_count<decltype(UniqueInSeqZip)>>{});
        }

        template<auto out_tape_seq, typename BaseTape>
        struct data_type
        {
            static constexpr auto base_tape_seq = BaseTape::sequence;
 
            static constexpr auto unique_in_seq_and_map = detail::get_unique_seq_and_map<base_tape_seq>();
            static constexpr auto unique_in_seq_zip = unique_in_seq_and_map.sequence;
           
            static constexpr auto unique_in_map = detail::index_array_to_sequence(unique_in_seq_and_map.map);
     
            static constexpr auto result_vec_seq = detail::mapped_layout<out_tape_seq>(unique_in_map);

            using result_vec_t = decltype(get_result_vec<unique_in_seq_zip>(std::declval<BaseTape>()));
            
            using result_tape_t = decltype(std::declval<result_vec_t>() | get_tape<result_vec_seq>);
            
            BaseTape base_tape;
            result_vec_t result_vec =  get_result_vec<unique_in_seq_zip>(std::move(base_tape));
            // result_tape_t result_tape = std::move(result_vec) | get_tape<result_vec_seq>;

            // static constexpr auto sequence = result_tape_t::sequence;

            // template<size_t I, specified<data_type> S>
            // RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, S&& self)
            // AS_EXPRESSION(getter<purified<decltype(FWD(self, result_tape, data))>>{}.template get<I>(FWD(self, result_tape, data)))
            
            static constexpr auto sequence = result_vec_seq;
            
            template<size_t I, specified<data_type> S>
            RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, S&& self)
            AS_EXPRESSION(getter<purified<decltype(FWD(self, result_vec))>>{}.template get<I>(FWD(self, result_vec)))
        };

        template<auto out_tape_seq, typename BaseTape>
        data_type(constant_t<out_tape_seq>, BaseTape&&) -> data_type<out_tape_seq, std::decay_t<BaseTape>>;

        template<auto Seq, specified<invoke_view> Self> requires(not std::same_as<decltype(Seq), size_t>)
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
        {
            constexpr auto adapt_flat_seq = detail::sequence_adapat_to_leaf<Seq, child_type<V, 1uz>>();
            constexpr auto in_tape_seq_and_map = get_in_tape_seq_and_map<adapt_flat_seq>();
            constexpr auto in_tape_seq = detail::invoke_view_to_base_sequence(in_tape_seq_and_map.sequence);
            constexpr auto out_tape_seq = detail::mapped_layout<Seq>(in_tape_seq_and_map.map);

            using data_t = data_type<out_tape_seq, decltype(FWD(self).base() | get_tape<in_tape_seq>)>;
            constexpr auto sequence = data_t::sequence;
            return tape_t<data_t, sequence>{ data_t{ FWD(self).base() | get_tape<in_tape_seq> } };
        }
    };

    template<typename V>
    invoke_view(V&&) -> invoke_view<V>;

    constexpr inline auto invoke = tree_adaptor{ []<typename V, typename F>(V&& view, F&& fn)
    {
        return invoke_view{ combine(FWD(view), FWD(fn)) };
    }};

    constexpr inline auto transform = tree_adaptor{ [](auto&& view, auto&& fn)
    {
        return invoke_view{ combine(FWD(view), FWD(fn) | repeat<child_count<decltype(view)>>) };
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
        return invoke_view{ combine(FWD(view), FWD(fn) | relayout<single_value_tree_layout<decltype(view)>()>) };
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
                return invoke_view{
                    combine(
                    relayout_view{ tuple<V1, V2>{ FWD(view1), FWD(view2) }, constant_t<arg_layout>{} },
                    relayout_view{ [fn = wrapper{ FWD(fn) }](auto&& args){ return fn.value()(child<0uz>(args), child<1uz>(args)); }, constant_t<fn_layout>{} }
                    )
                };
            }
        };
    }

    inline constexpr detail::grouped_cartesian_transform_t grouped_cartesian_transform{};
}

#include "macro_undef.h"
#endif