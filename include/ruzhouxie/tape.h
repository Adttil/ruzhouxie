#ifndef RUZHOUXIE_TAPE_H
#define RUZHOUXIE_TAPE_H

#include "general.h"
#include "tree_adaptor.h"
#include "tuple.h"
#include "array.h"
#include "get.h"

#include "macro_define.h"

namespace ruzhouxie::detail
{
    template<typename T, auto Layout>
    struct relayout_view;
    
    template<typename T, auto...ReservedLayouts>
    struct reserved_view;

    enum class layout_relation
    {
        independent,
        intersectant,
        included
    };

    consteval layout_relation operator|(layout_relation l, layout_relation r)
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

    consteval layout_relation operator&(layout_relation l, layout_relation r)
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
    consteval layout_relation indices_relation_to(const array<size_t, N1>& index_pack, const array<size_t, N2>& other)
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

    consteval layout_relation indices_relation_to_layout(const indicesoid auto& index_pack, const auto& layout)
    {
        if constexpr(indicesoid<decltype(layout)>)
        {
            return indices_relation_to(index_pack, layout);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... | indices_relation_to_layout(index_pack, layout | child<I>));
        }(std::make_index_sequence<child_count<decltype(layout)>>{});
    }

    consteval layout_relation layout_relation_to(const auto& layout, const auto& other)
    {
        if constexpr(indicesoid<decltype(layout)>)
        {
            return indices_relation_to_layout(layout, other);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... & layout_relation_to(layout | child<I>, other));
        }(std::make_index_sequence<child_count<decltype(layout)>>{});
    }
}

namespace ruzhouxie
{
    enum class access_mode
    {
        unknown,
        pass,
        once
    };

    template<typename Data, auto Sequence>
    struct tape_t
    {
        using data_type = Data;
        static constexpr auto sequence = Sequence;
        //Here, it is necessary to ensure that each child of data is independent.
        //Each child of data is also necessary to meet the above requirements.
        Data data;

        // template<size_t I, specified<tape_t> Self>
        // RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
        //     AS_EXPRESSION(FWD(self, data) | child<I>)

        template<size_t I, specified<tape_t> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) access_once(Self&& self)
        {
            if constexpr(I >= child_count<decltype(Sequence)>)
            {
                return;
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
                        return FWD(self, data) | child<layout>;
                    }
                    else
                    {
                        return std::as_const(self.data) | child<layout>;
                    }
                }
                else if constexpr(relation_to_rest == detail::layout_relation::independent)
                {
                    return detail::relayout_view<decltype(FWD(self, data)), layout>
                    {
                        {}, FWD(self, data)
                    };
                }
                // else if constexpr(relation_to_rest == layout_relation::intersectant)
                // {
                //     constexpr auto other = ...;
                //     return detail::reserved_view<decltype(FWD(self, data)), other>
                //     {
                //         {}, FWD(self, data)
                //     };
                // }
                else
                {
                    return detail::relayout_view<decltype(as_const(self.data)), layout>
                    {
                        {}, as_const(self.data)
                    };
                }
            }
        }

        template<size_t I, specified<tape_t> Self>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) access_pass(Self&& self)
        {
            if constexpr(I >= child_count<decltype(Sequence)>)
            {
                return;
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
                        return FWD(self, data) | child<layout>;
                    }
                    else
                    {
                        return std::as_const(self.data) | child<layout>;
                    }
                }
                else if constexpr(relation_to_rest == detail::layout_relation::independent)
                {
                    return detail::relayout_view<decltype(FWD(self, data)), layout>
                    {
                        {}, FWD(self, data)
                    };
                }
                // else if constexpr(relation_to_rest == layout_relation::included)
                // {
                //     return relayout_view<decltype(as_const(self.data)), layout>
                //     {
                //         as_const(self.data)
                //     };
                // }
                else
                {
                    return detail::relayout_view<decltype(as_const(self.data)), layout>
                    {
                        {}, as_const(self.data)
                    };
                }
            }
        }

        template<size_t I, access_mode Mode = access_mode::unknown, specified<tape_t> Self = void>
        RUZHOUXIE_INLINE friend constexpr decltype(auto) access(Self&& self)
        {
            if constexpr(Mode == access_mode::once)
            {
                return access_once<I>(FWD(self));
            }
            else
            {
                return access_pass<I>(FWD(self));
            }
        }

        template<auto InputLayoutsZip, size_t I = 0uz, auto Cur = tuple{}, auto CurLayouts = tuple{}>
        static constexpr auto get_unique_tape_seq_and_set_map(auto& map)noexcept
        {
            if constexpr(I >= child_count<decltype(InputLayoutsZip)>)
            {
                return CurLayouts;
            }
            else if constexpr(tuple_contain(CurLayouts, InputLayoutsZip | child<I>))
            {
                return get_unique_tape_seq_and_set_map<InputLayoutsZip, I + 1uz, Cur, CurLayouts>(map);
            }
            else
            {
                map[I] = child_count<decltype(Cur)>;
                return get_unique_tape_seq_and_set_map<
                    InputLayoutsZip, I + 1uz,
                    tuple_cat(Cur, tuple{ I }),
                    tuple_cat(CurLayouts, tuple<purified<decltype(InputLayoutsZip | child<I>)>>{ InputLayoutsZip | child<I> })
                >(map);
            }
        }

        static consteval auto get_unique_seq_and_map()
        {
            array<size_t, child_count<decltype(Sequence)>> map{};
            auto seq = get_unique_tape_seq_and_set_map<Sequence>(map);
            struct result_t
            {
                decltype(seq) sequence;
                array<size_t, child_count<decltype(Sequence)>> map;
            };
            return result_t{ seq, map };
        }
    };

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
    }

    template<auto Sequence>
    constexpr inline tree_adaptor_closure<detail::get_tape_t_ns::get_tape_t<Sequence>> get_tape{};

    template<auto Sequence>
    struct detail::get_tape_t_ns::get_tape_t
    {
        template<typename T>
        RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr (requires{ tag_invoke<Sequence>(get_tape<Sequence>, FWD(t)); })
            {
                return tag_invoke<Sequence>(get_tape<Sequence>, FWD(t));
            }
            else if constexpr(branched<T>)
            {
                return get_tuple_tape(FWD(t));
            }
            else
            {
                return tape_t<T&&, Sequence>{ FWD(t) };
            }
        }

    private:
        struct child_sequence_location
        {
            size_t child = invalid_index;
            size_t index = invalid_index;
        };

        template<size_t I, auto Seq>
        static constexpr auto child_sequence(size_t& count, auto& map)
        {
            if constexpr(indicesoid<decltype(Seq)>)
            {
                if constexpr(Seq.size() == 0uz)
                {
                    //map.child = I;
                    //map.index = count++; 
                    return make_tuple(Seq);
                }
                else if constexpr(Seq[0] == I)
                {
                    map.child = I;
                    map.index = count++;
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
                auto args = tuple<purified<decltype(child_sequence<I, Seq | child<J>>(count, map | child<J>))>...>
                {
                    child_sequence<I, Seq | child<J>>(count, map | child<J>)... 
                };
                return tuple_cat(args | child<J>...);
            }(std::make_index_sequence<child_count<decltype(Seq)>>{});
        }

        template<auto Seq, size_t ChildCount>
        static constexpr auto children_sequnce()
        {
            constexpr size_t n = child_count<decltype(Seq)>;
            auto map = init_children_tape_map<purified<decltype(Seq)>>();
            auto counts = array<size_t, ChildCount>{};
            auto children_sequence = [&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(child_sequence<I, Seq>(counts[I], map)...);
            }(std::make_index_sequence<ChildCount>{});

            struct result_t
            {
                decltype(children_sequence) sequences;
                decltype(map) map;
            };

            return result_t{ children_sequence, map };
        }

        template<typename V>
        RUZHOUXIE_INLINE static constexpr auto get_tuple_tape(V&& view)
        {
            constexpr auto children_sequence_map = children_sequnce<Sequence, child_count<V>>();
            constexpr auto children_sequences = children_sequence_map.sequences;
            constexpr auto children_map = children_sequence_map.map;

            constexpr auto children_tapes_seq = get_children_tapes_seq<children_sequences, V>();

            constexpr auto result_tape_seq = mapped_seq<Sequence, children_tapes_seq, children_map>();

            return tape_t<decltype(get_children_tapes<children_sequences>(FWD(view))), result_tape_seq>
            {
                get_children_tapes<children_sequences>(FWD(view))
            };
        }

        template<typename TSeq>
        RUZHOUXIE_INLINE static consteval auto init_children_tape_map()
        {
            if constexpr(indicesoid<TSeq>)
            {
                return child_sequence_location{};
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(init_children_tape_map<child_type<TSeq, I>>()...);
            }(std::make_index_sequence<child_count<TSeq>>{});
        }

        template<auto Seqs, typename V>
        RUZHOUXIE_INLINE static constexpr auto get_children_tapes(V&& view)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tape_data_tie{ FWD(view) | child<I> | get_tape<Seqs | child<I>>... };
            }(std::make_index_sequence<child_count<V>>{});
        }

        template<auto Seqs, typename V>
        RUZHOUXIE_INLINE static constexpr auto get_children_tapes_seq()
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(decltype(std::declval<V>() | child<I> | get_tape<Seqs | child<I>>)::sequence...);
            }(std::make_index_sequence<child_count<V>>{});
        }

        template<auto Seq, auto ChildTapeSeqs, auto Map>
        RUZHOUXIE_INLINE static constexpr auto mapped_seq()
        {
            if constexpr(indicesoid<decltype(Seq)>)
            {
                if constexpr(Seq.size() == 0uz)
                {
                    return Seq;
                }
                else
                {
                    return detail::concat_array(array{ Map.child }, ChildTapeSeqs | child<Map.child, Map.index>); 
                }
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(mapped_seq<Seq | child<I>, ChildTapeSeqs, Map | child<I>>()...);
            }(std::make_index_sequence<child_count<decltype(Seq)>>{});
        }
    };
}

#include "macro_undef.h"
#endif