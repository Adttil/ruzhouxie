#ifndef RUZHOUXIE_TRANSFORM_H
#define RUZHOUXIE_TRANSFORM_H

#include "general.h"
#include "get.h"
#include "tree_adaptor.h"
#include "tape.h"
#include "tree_view.h"
#include "relayout.h"

#include "macro_define.h"

namespace ruzhouxie
{
    template<typename V, typename F>
    struct transform_view : detail::view_base<V>, wrapper<F>, view_interface<transform_view<V, F>>
    {
        // RUZHOUXIE_MAYBE_EMPTY V base;
        // RUZHOUXIE_MAYBE_EMPTY F fn;

        static constexpr size_t size = child_count<V>;

        template<specified<transform_view> Self>
        RUZHOUXIE_INLINE constexpr auto&& fn(this Self&& self)noexcept
        {
            return rzx::as_base<wrapper<F>>(FWD(self)).value();
        }

        template<size_t I, specified<transform_view> Self> requires (I >= child_count<V>)
        friend end_t tag_invoke(tag_t<child<I>>, Self&& self){ return end(); }

        template<size_t I, specified<transform_view> Self> requires (I < child_count<V>)
        RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
        AS_EXPRESSION(FWD(self).fn()(FWD(self).base() | child<I>))

        template<typename T>
        struct data_type
        {
            //constexpr auto input_seq_map = get_view_seq_and_map<Seq>();
            static constexpr auto unique_base_tape_seq_and_map = detail::get_unique_seq_and_map<T::sequence>();
            
            RUZHOUXIE_INLINE static constexpr auto get_data(T&& base_tape, F& fn)
            {
                auto uinque_base_tape = make_tape<unique_base_tape_seq_and_map.sequence>(FWD(base_tape, data));
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple<decltype(fn(pass<I>(FWD(uinque_base_tape))))...>
                    {
                        fn(pass<I>(FWD(uinque_base_tape)))...
                    };
                }(std::make_index_sequence<child_count<decltype(unique_base_tape_seq_and_map.sequence)>>{});
            }
            using result_data_type = decltype(get_data(std::declval<T>(), std::declval<F&>()));

            T base_tape;
            result_data_type data;

            RUZHOUXIE_INLINE constexpr data_type(auto&& base, auto&& get_tape_fn, F& fn)
                : base_tape(FWD(base) | get_tape_fn)
                , data(get_data(std::move(base_tape), fn))
            {}

            template<size_t I, specified<data_type> Self>
            RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
            AS_EXPRESSION(getter<result_data_type>{}.template get<I>(FWD(self, data)))
        };

        template<auto Seq, specified<transform_view> Self> requires(not std::same_as<decltype(Seq), size_t>)
        RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
            //todo...noexcept
        {
            constexpr auto input_seq_map = get_view_seq_and_map<Seq>();

            using base_tape_type = decltype(FWD(self).base() | get_tape<input_seq_map.seq>);
            
            constexpr auto unique_input_tape_seq_and_map = detail::get_unique_seq_and_map<base_tape_type::sequence>();
    
            constexpr auto uinque_input_map = unique_input_tape_seq_and_map.map;

            constexpr auto result_layouts = get_result_layouts<Seq, input_seq_map.map, uinque_input_map>(
                std::make_index_sequence<child_count<decltype(Seq)>>{});

            return tape_t<data_type<base_tape_type>, result_layouts>
            {
                data_type<base_tape_type>{ FWD(self).base(), get_tape<input_seq_map.seq>, self.fn() }
            };
        }

    private:
        struct view_seq_and_map_info
        {
            size_t n_seq = 0uz;
            array<size_t, size> seq_buff{};
            array<size_t, size> map = []<size_t...I>(std::index_sequence<I...>)
            {
                return array<size_t, size>{ (invalid_index + (I - I))... };
            }(std::make_index_sequence<size>{});

            template<auto IndexTree>
            constexpr view_seq_and_map_info& set()
            {
                if constexpr(indicesoid<decltype(IndexTree)>)
                {
                    if constexpr(IndexTree.size() == 0uz)
                    {
                        for(size_t i = 0uz; i < size; ++i)
                        {
                            seq_buff[i] = i;
                            map[i] = i;
                        }
                        n_seq = size;
                    }
                    else if(map[IndexTree[0]] == invalid_index)
                    {
                        map[IndexTree[0]] = n_seq;
                        seq_buff[n_seq++] = IndexTree[0];
                    }
                }
                else
                {
                    [&]<size_t...I>(std::index_sequence<I...>)
                    {
                        (..., set<IndexTree | child<I>>());
                    }(std::make_index_sequence<child_count<decltype(IndexTree)>>{});
                }
                return *this;
            }
        };

        template<auto Seq>
        static RUZHOUXIE_CONSTEVAL auto get_view_seq_and_map()
        {
            constexpr auto info = view_seq_and_map_info().template set<Seq>();

            auto seq = [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple{ array{ info.seq_buff[I] }... };
            }(std::make_index_sequence<info.n_seq>{});

            struct result_t
            {
                decltype(seq) seq;
                array<size_t, size> map;
            };
            return result_t{ seq, info.map };
        }

        template<auto Seq, size_t I = 0uz, auto Cur = tuple{}>
        static constexpr auto get_input_seq_and_map_impl(auto& map)noexcept
        {
            if constexpr(I >= child_count<decltype(Seq)>)
            {
                return Cur;
            }
            else if constexpr(not indicesoid<decltype(Seq | child<I>)>)
            {
                static_assert(indicesoid<decltype(Seq | child<I>)>, "Invalid sequence.");
            }
            else if constexpr((Seq | child<I>).size() == 0uz)
            {
                return [&]<size_t...J>(std::index_sequence<J...>)
                {
                    map = array{ J... };
                    return tuple{ array{J}... };
                }(std::make_index_sequence<size>{});
            }
            else if constexpr(tuple_contain(Cur, array{ (Seq | child<I>)[0] } ))
            {
                return get_input_seq_and_map_impl<Seq, I + 1uz, Cur>(map);
            }
            else
            {
                constexpr size_t index = (Seq | child<I>)[0];
                map[index] = child_count<decltype(Cur)>;
                return get_input_seq_and_map_impl<Seq, I + 1uz, detail::concat_to_tuple(Cur, tuple{ array{ index } })>(map);
            }
        }

        template<auto Seq>
        static RUZHOUXIE_CONSTEVAL auto get_input_seq_and_map()
        {
            array<size_t, child_count<V>> map{};
            auto seq = get_input_seq_and_map_impl<Seq>(map);
            struct result_t
            {
                decltype(seq) seq;
                array<size_t, child_count<V>> map;
            };
            return result_t{ seq, map };
        }

        static constexpr auto set_result_seq(auto& seq, const auto& view_map, const auto& unique_view_tape_map)noexcept
        {
            if constexpr(indicesoid<decltype(seq)>)
            {
                if (seq.size() != 0uz)
                {
                    seq[0] = unique_view_tape_map[view_map[seq[0]]];
                }
            }
            else
            {
                [&]<size_t...I>(std::index_sequence<I...>)
                {
                    (... , set_result_seq(seq | child<I>, view_map, unique_view_tape_map));
                }(std::make_index_sequence<child_count<decltype(seq)>>{});
            }
        }

        template<auto Seq, auto InputMap, auto UniqueInputMap, size_t...I>
        static RUZHOUXIE_CONSTEVAL auto get_result_layouts(std::index_sequence<I...>)
        {
            auto seq = Seq;
            set_result_seq(seq, InputMap, UniqueInputMap);
            return seq;
        }
    };

    template<typename V, typename F>
    transform_view(V&&, F&&) -> transform_view<V, std::decay_t<F>>;
}

namespace ruzhouxie
{
    namespace detail
    {
        struct transform_t
        {
            template<typename V, typename F>
            RUZHOUXIE_INLINE constexpr auto operator()(V&& view, F&& fn)const
            AS_EXPRESSION(transform_view{ FWD(view), FWD(fn) })
        };
    }

    inline constexpr tree_adaptor<detail::transform_t> transform{};

    namespace detail
    {
        struct zip_transform_t
        {
            template<typename Fn, typename...T>
            RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn, T&&...trees)const
            noexcept(noexcept(zip(FWD(trees)...)))
            {
                return zip(FWD(trees)...) | transform(
                    [fn = FWD(fn)](auto&& zip_args)->decltype(auto)
                    {
                        return [&]<size_t...I>(std::index_sequence<I...>)->decltype(auto)
                        {
                            return fn(FWD(zip_args) | child<I> ...);
                        }(std::index_sequence_for<T...>{});
                    }
                );
            }
        };
    }
    
    inline constexpr detail::zip_transform_t zip_transform{};
}

#include "macro_undef.h"
#endif