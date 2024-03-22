#ifndef RUZHOUXIE_TAPE_H
#define RUZHOUXIE_TAPE_H

#include "general.h"
#include "tree_adaptor.h"
#include "tuple.h"
#include "array.h"
#include "get.h"

#include "macro_define.h"

namespace ruzhouxie
{
    template<typename T, auto Layout>
    struct relayout_view;
    
    template<typename T, auto...ReservedLayouts>
    struct reserved_view;
}

namespace ruzhouxie::detail
{
    enum class layout_relation
    {
        independent,
        intersectant,
        included
    };

    RUZHOUXIE_CONSTEVAL layout_relation operator|(layout_relation l, layout_relation r)
    {
        if(l == layout_relation::included || r == layout_relation::included)
        {
            return layout_relation::included;
        }
        else if (l == layout_relation::intersectant || r == layout_relation::intersectant)
        {
            return layout_relation::intersectant;
        }
        else
        {
            return layout_relation::independent;
        }
    }

    RUZHOUXIE_CONSTEVAL layout_relation operator&(layout_relation l, layout_relation r)
    {
        if (l == layout_relation::independent && r == layout_relation::independent)
        {
            return layout_relation::independent;
        }
        else if(l == layout_relation::included && r == layout_relation::included)
        {
            return layout_relation::included;
        }
        else
        {
            return layout_relation::intersectant;
        }
    }

    template<size_t N1, size_t N2>
    RUZHOUXIE_CONSTEVAL layout_relation indices_relation_to(const array<size_t, N1>& index_pack, const array<size_t, N2>& other)
    {
        if constexpr(N1 >= N2)
        {
            if(detail::array_take<N2>(index_pack) == other)
            {
                return layout_relation::included;
            }
            else
            {
                return layout_relation::independent;
            }
        }
        else
        {
            if(detail::array_take<N1>(other) == index_pack)
            {
                return layout_relation::intersectant;
            }
            else
            {
                return layout_relation::independent;
            }
        }
    }

    RUZHOUXIE_CONSTEVAL layout_relation indices_relation_to_layout(const indicesoid auto& index_pack, const auto& layout)
    {
        if constexpr(indicesoid<decltype(layout)>)
        {
            return indices_relation_to(index_pack, layout);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (layout_relation::independent | ... | indices_relation_to_layout(index_pack, layout | child<I>));
        }(std::make_index_sequence<child_count<decltype(layout)>>{});
    }

    RUZHOUXIE_CONSTEVAL layout_relation layout_relation_to(const auto& layout, const auto& other)
    {
        if constexpr(indicesoid<decltype(layout)>)
        {
            return indices_relation_to_layout(layout, other);
        }
        else if constexpr(child_count<decltype(layout)> == 0uz)
        {
            return layout_relation::independent;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... & layout_relation_to(layout | child<I>, other));
        }(std::make_index_sequence<child_count<decltype(layout)>>{});
    }
    
    enum class tape_access_strategy_t
    {
        none,
        last,
        not_last,
        relayout_last,
        relayout_not_last
    };
}

namespace ruzhouxie
{
    // enum class access_mode
    // {
    //     unknown,
    //     pass,
    //     once
    // };

    template<typename Data, auto Sequence>
    struct tape_t
    {
        using data_type = Data;
        static constexpr auto sequence = Sequence;
        //Here, it is necessary to ensure that each child of data is independent.
        //Each child of data is also necessary to meet the above requirements.
        Data data;

#ifdef RUZHOUXIE_DEBUG_TAPE
        //This help to check if the access order is legal.
        mutable size_t min_valid_index = 0;
        mutable bool is_for_pass = false;
#endif

    private:
        using strategy_t = detail::tape_access_strategy_t;

        template<size_t I, specified<tape_t> Self>
        static RUZHOUXIE_CONSTEVAL choice_t<strategy_t> access_choose()
        {
            if constexpr(I >= child_count<decltype(Sequence)>)
            {
                return { strategy_t::none };
            }
            else
            {
                constexpr auto layout = Sequence | child<I>;
                constexpr auto relation_to_rest = []<size_t...J>(std::index_sequence<J...>)
                {
                    //for some compiler.
                    constexpr auto layout = Sequence | child<I>;
                    return (detail::layout_relation::independent | ... | detail::layout_relation_to(layout, Sequence | child<J + I + 1uz>));
                }(std::make_index_sequence<child_count<decltype(Sequence)> - I - 1uz>{});

                if constexpr(indicesoid<decltype(layout)>)
                {
                    if constexpr(relation_to_rest == detail::layout_relation::independent)
                    {
                        return { strategy_t::last, noexcept(FWD(std::declval<Self>(), data) | child<layout>) };
                    }
                    else
                    {
                        return { strategy_t::not_last, noexcept(std::as_const(std::declval<Self&>().data) | child<layout>) };
                    }
                }
                else if constexpr(relation_to_rest == detail::layout_relation::independent)
                {
                    return { strategy_t::relayout_last, true };
                }
                // else if constexpr(relation_to_rest == layout_relation::intersectant)
                // {
                    //todo...
                // }
                else
                {
                    return { strategy_t::relayout_not_last, true };
                }
            }
        }

        template<size_t I, specified<tape_t> Self>
        static RUZHOUXIE_CONSTEVAL choice_t<strategy_t> pass_choose()
        {
            if constexpr(I >= child_count<decltype(Sequence)>)
            {
                return { strategy_t::none };
            }
            else
            {
                constexpr auto layout = Sequence | child<I>;
                 constexpr auto relation_to_rest = []<size_t...J, size_t...K>(std::index_sequence<J...>, std::index_sequence<K...>)
                {
                    //for some compiler.
                    constexpr auto layout = Sequence | child<I>;
                    constexpr auto front = (detail::layout_relation::independent | ... | detail::layout_relation_to(layout, Sequence | child<J>));
                    constexpr auto behind = (detail::layout_relation::independent | ... | detail::layout_relation_to(layout, Sequence | child<K + I + 1uz>));
                    return front | behind;
                }(std::make_index_sequence<I>{}, std::make_index_sequence<child_count<decltype(Sequence)> - I - 1uz>{});

                if constexpr(indicesoid<decltype(layout)>)
                {
                    if constexpr(relation_to_rest == detail::layout_relation::independent)
                    {
                        return { strategy_t::last, noexcept(FWD(std::declval<Self>(), data) | child<layout>) };
                    }
                    else
                    {
                        return { strategy_t::not_last, noexcept(std::as_const(std::declval<Self&>().data) | child<layout>) };
                    }
                }
                else if constexpr(relation_to_rest == detail::layout_relation::independent)
                {
                    return { strategy_t::relayout_last, true };
                }
                // else if constexpr(relation_to_rest == layout_relation::intersectant)
                // {
                    //todo...
                // }
                else
                {
                    return { strategy_t::relayout_not_last, true };
                }
            }
        }

    public:
        template<size_t I, specified<tape_t> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) access(Self&& self)
            noexcept(access_choose<I, Self>().nothrow)
            requires(access_choose<I, Self>().strategy != strategy_t::none)
        {
#ifdef RUZHOUXIE_DEBUG_TAPE
            if(I < self.min_valid_index || self.is_for_pass)
            {
                //Tape should be accessed by order of sequence.
                std::unreachable();
            }
            self.min_valid_index = I + 1uz;
#endif
            constexpr auto layout = Sequence | child<I>;
            constexpr strategy_t strategy = access_choose<I, Self>().strategy;
            
            if constexpr(strategy == strategy_t::last)
            {
                return FWD(self, data) | child<layout>;
            }
            else if constexpr(strategy == strategy_t::not_last)
            {
                return std::as_const(self.data) | child<layout>;
            }
            else if constexpr(strategy == strategy_t::relayout_last)
            {
                return relayout_view<decltype(FWD(self, data)), layout>
                {
                   FWD(self, data)
                };
            }
            else if constexpr(strategy == strategy_t::relayout_not_last)
            {
                return relayout_view<decltype(as_const(self.data)), layout>
                {
                    as_const(self.data)
                };
            }
            else
            {
                static_assert(strategy == strategy_t::relayout_not_last, "Should not reach.");
            }
        }

        template<size_t I, specified<tape_t> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) pass(Self&& self)
            noexcept(pass_choose<I, Self>().nothrow)
            requires(pass_choose<I, Self>().strategy != strategy_t::none)
        {
#ifdef RUZHOUXIE_DEBUG_TAPE
            if(I < self.min_valid_index)
            {
                //Tape should be accessed by order of sequence.
                std::unreachable();
            }
            self.is_for_pass = true;
            self.min_valid_index = I + 1uz;
#endif
            constexpr auto layout = Sequence | child<I>;
            constexpr strategy_t strategy = pass_choose<I, Self>().strategy;
            
            if constexpr(strategy == strategy_t::last)
            {
                return FWD(self, data) | child<layout>;
            }
            else if constexpr(strategy == strategy_t::not_last)
            {
                return std::as_const(self.data) | child<layout>;
            }
            else if constexpr(strategy == strategy_t::relayout_last)
            {
                return relayout_view<decltype(FWD(self, data)), layout>
                {
                   FWD(self, data)
                };
            }
            else if constexpr(strategy == strategy_t::relayout_not_last)
            {
                return relayout_view<decltype(as_const(self.data)), layout>
                {
                    as_const(self.data)
                };
            }
            else
            {
                static_assert(strategy == strategy_t::relayout_not_last, "Should not reach.");
            }
        }
    };

    namespace detail 
    {
        template<auto Seq, size_t I = 0uz, auto Cur = tuple{}, auto CurLayouts = tuple{}>
        RUZHOUXIE_CONSTEVAL auto get_unique_seq_and_set_map(auto& map)noexcept
        {
            if constexpr(I >= child_count<decltype(Seq)>)
            {
                return CurLayouts;
            }
            else if constexpr(tuple_contain(CurLayouts, Seq | child<I>))
            {
                return get_unique_seq_and_set_map<Seq, I + 1uz, Cur, CurLayouts>(map);
            }
            else
            {
                map[I] = child_count<decltype(Cur)>;
                return get_unique_seq_and_set_map<
                    Seq, I + 1uz,
                    concat_to_tuple(Cur, tuple{ I }),
                    concat_to_tuple(CurLayouts, tuple<purified<decltype(Seq | child<I>)>>{ Seq | child<I> })
                >(map);
            }
        }

        template<auto Seq>
        RUZHOUXIE_CONSTEVAL auto get_unique_seq_and_map()
        {
            array<size_t, child_count<decltype(Seq)>> map{};
            auto seq = detail::get_unique_seq_and_set_map<Seq>(map);
            struct result_t
            {
                decltype(seq) sequence;
                array<size_t, child_count<decltype(Seq)>> map;
            };
            return result_t{ seq, map };
        }
    }

    template<auto Seq>
    RUZHOUXIE_INLINE constexpr auto make_tape(auto&& data)noexcept
    {
        return tape_t<decltype(data), Seq>{ FWD(data) };
    }
    
    template<typename...T>
    struct tape_data_tie
    {
        tuple<T...> tapes;

        template<size_t I>
        RUZHOUXIE_INLINE constexpr auto&& get(this auto&& self)noexcept
        {
            auto&& tape = FWD(self, tapes).template get<I>();
            return FWD(tape, data);
        }
    };

    template<typename...T>
    tape_data_tie(T...) -> tape_data_tie<std::decay_t<T>...>;
}

template<typename...T>
struct std::tuple_size<ruzhouxie::tape_data_tie<T...>>
    : std::tuple_size<std::tuple<T...>>{};

template<size_t I, typename...T>
struct std::tuple_element<I, ruzhouxie::tape_data_tie<T...>> 
    : std::tuple_element<I, std::tuple<typename T::data_type...>>{};

namespace ruzhouxie
{
    namespace detail::get_tape_t_ns
    {
        template<auto Sequence>
        void tag_invoke();

        template<auto Sequence>
        struct get_tape_t;

        enum class strategy_t
        {
            none,
            tag_invoke,
            branched,
            terminal
        };
    }

    template<auto Sequence>
    constexpr inline tree_adaptor_closure<detail::get_tape_t_ns::get_tape_t<Sequence>> get_tape{};
}

namespace ruzhouxie::detail
{
    template<typename TSeq>
    static RUZHOUXIE_CONSTEVAL auto init_children_tapes_map()
    {
        if constexpr(indicesoid<TSeq>)
        {
            return size_t{};
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(init_children_tapes_map<child_type<TSeq, I>>()...);
        }(std::make_index_sequence<child_count<TSeq>>{});
    }

    template<size_t I, auto Seq>
    RUZHOUXIE_CONSTEVAL auto get_child_sequence_and_set_map(size_t& count, auto& map)
    {
        if constexpr(indicesoid<decltype(Seq)>)
        {
            if constexpr(Seq.size() == 0uz)
            {
                return make_tuple(Seq);
            }
            else if constexpr(Seq[0] == I)
            {
                map = count++;
                return make_tuple(detail::array_drop<1uz>(Seq));
            }
            else
            {
                return tuple{};
            }
        }
        else return[&]<size_t...J>(std::index_sequence<J...>)
        {
            static_assert(sizeof...(J) > 0, "invalid sequence.");
            //To ensure the order of evaluation.
            auto args = tuple<purified<decltype(get_child_sequence_and_set_map<I, Seq | child<J>>(count, map | child<J>))>...>
            {
                get_child_sequence_and_set_map<I, Seq | child<J>>(count, map | child<J>)... 
            };
            return concat_to_tuple(args | child<J>...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }

    template<size_t ChildCount, auto Seq>
    static RUZHOUXIE_CONSTEVAL auto get_children_sequnces_and_map()
    {
        auto map = init_children_tapes_map<decltype(Seq)>();
        auto counts = array<size_t, ChildCount>{};
        auto children_sequence = [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(get_child_sequence_and_set_map<I, Seq>(counts[I], map)...);
        }(std::make_index_sequence<ChildCount>{});

        struct result_t
        {
            decltype(children_sequence) sequences;
            decltype(map) map;
        };
        return result_t{ children_sequence, map };
    }

    template<auto Seqs, typename V, size_t...I>
    RUZHOUXIE_INLINE static constexpr auto get_children_tapes_impl(V&& view, std::index_sequence<I...>)
        AS_EXPRESSION(tape_data_tie{ FWD(view) | child<I> | get_tape<Seqs | child<I>>... })

    template<auto Seq, typename V>
    RUZHOUXIE_INLINE static constexpr auto get_children_tapes(V&& view)
        AS_EXPRESSION(get_children_tapes_impl<get_children_sequnces_and_map<child_count<V>, Seq>().sequences>(FWD(view), std::make_index_sequence<child_count<V>>{}))

    template<auto Seqs, typename V>
    RUZHOUXIE_CONSTEVAL auto get_children_tapes_seq()
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(decltype(std::declval<V>() | child<I> | get_tape<Seqs | child<I>>)::sequence...);
        }(std::make_index_sequence<child_count<V>>{});
    }

    template<auto Seq, auto ChildTapeSeqs, auto Map>
    RUZHOUXIE_CONSTEVAL auto mapped_children_tapes_seq()
    {
        if constexpr(indicesoid<decltype(Seq)>)
        {
            if constexpr(Seq.size() == 0uz)
            {
                return Seq;
            }
            else
            {
                return detail::concat_array(array{ Seq[0] }, ChildTapeSeqs | child<Seq[0], Map>); 
            }
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(mapped_children_tapes_seq<Seq | child<I>, ChildTapeSeqs, Map | child<I>>()...);
        }(std::make_index_sequence<child_count<decltype(Seq)>>{});
    }
    
    template<auto Seq, typename V>
    RUZHOUXIE_INLINE static constexpr auto get_tuple_tape(V&& view)
        noexcept(noexcept(get_children_tapes<Seq>(FWD(view))))
        requires(requires{get_children_tapes<Seq>(FWD(view));})
    {
        constexpr auto children_sequence_map = get_children_sequnces_and_map<child_count<V>, Seq>();
        constexpr auto children_sequences = children_sequence_map.sequences;
        constexpr auto children_map = children_sequence_map.map;
        constexpr auto children_tapes_seq = get_children_tapes_seq<children_sequences, V>();
        constexpr auto result_tape_seq = mapped_children_tapes_seq<Seq, children_tapes_seq, children_map>();

        return tape_t<decltype(get_children_tapes<Seq>(FWD(view))), result_tape_seq>
        {
            get_children_tapes<Seq>(FWD(view))
        };
    }
}

namespace ruzhouxie
{
    template<auto Sequence>
    struct detail::get_tape_t_ns::get_tape_t
    {
    private:
        template<typename T>
        static RUZHOUXIE_CONSTEVAL choice_t<strategy_t> choose()
        {
            constexpr auto seq = normalize_layout<Sequence, T>();
            //This requires con not use "seq" in clang.
            if constexpr (requires{ tag_invoke<normalize_layout<Sequence, T>()>(get_tape<normalize_layout<Sequence, T>()>, std::declval<T>()); })
            {
                return { strategy_t::tag_invoke, noexcept(tag_invoke<seq>(get_tape<seq>, std::declval<T>())) };
            }
            else if constexpr(branched<T>)
            {
                return { strategy_t::branched, noexcept(detail::get_tuple_tape<seq>(std::declval<T>())) };
            }
            else
            {
                return { strategy_t::terminal, true };
            }
        }
        
    public:
        template<typename T>
        RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t)const
            noexcept(choose<T>().nothrow)
            requires(choose<T>().strategy != strategy_t::none)
        {
            constexpr strategy_t strategy = choose<T>().strategy;
            constexpr auto seq = normalize_layout<Sequence, T>();
            if constexpr (strategy == strategy_t::tag_invoke)
            {
                return tag_invoke<seq>(get_tape<seq>, FWD(t));
            }
            else if constexpr(strategy == strategy_t::branched)
            {
                return detail::get_tuple_tape<seq>(FWD(t));
            }
            else if constexpr(strategy == strategy_t::terminal)
            {
                return tape_t<T&&, seq>{ FWD(t) };
            }
            else
            {
                static_assert(strategy == strategy_t::tag_invoke, "Should not reach.");
            }
        }
    };
}

#include "macro_undef.h"
#endif