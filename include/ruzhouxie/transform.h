#ifndef RUZHOUXIE_TRANSFORM_H
#define RUZHOUXIE_TRANSFORM_H

#include "general.h"
#include "pipe_closure.h"
#include "ruzhouxie/macro_define.h"
#include "ruzhouxie/tuple.h"
#include "tree_view.h"
#include "relayout.h"

#include "macro_define.h"

namespace ruzhouxie
{
    namespace detail
	{
		template<typename Fn, typename...T>
		struct zip_transform_view;
	}

	template<typename Fn, typename...Views>
	struct detail::zip_transform_view : view_base<zip_transform_view<Fn, Views...>>
	{
		RUZHOUXIE_MAYBE_EMPTY Fn fn;
		RUZHOUXIE_MAYBE_EMPTY tuple<Views...> views;

		static constexpr size_t view_count = sizeof...(Views);
		static constexpr size_t size = std::min({ child_count<Views>... });

		template<size_t I, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
			//todo...noexcept
		{
			if constexpr (I >= size)
			{
				return;
			}
			else return[&]<size_t...J>(std::index_sequence<J...>) -> decltype(auto)
			{
				return FWD(self, fn)(FWD(self, views) | child<J, I>...);
			}(std::make_index_sequence<sizeof...(Views)>{});
		}

		template<auto Seq, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			//todo...noexcept
		{
			//constexpr auto input_seq_map = get_input_seq_and_map<Seq>();
			constexpr auto input_seq_map = get_view_seq_and_map<Seq>();

			auto input_tapes = [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ FWD(self, views) | child<I> | get_tape<input_seq_map.seq>... };
			}(std::index_sequence_for<Views...>{});
			using input_tapes_type = decltype(input_tapes);
			
			constexpr auto input_tape_layouts_zip = get_input_tape_layouts_zip<input_tapes_type>(
				std::make_index_sequence<child_count<decltype(input_seq_map.seq)>>{});
			constexpr auto unique_input_indices_map = get_unique_input_indices_and_map<input_tape_layouts_zip>();
			constexpr size_t n_unique_input_indices = child_count<decltype(unique_input_indices_map.indices)>;

			const auto result_data_at = [&]<size_t I, size_t...J>(std::index_sequence<J...>) -> decltype(auto)
			{
				return FWD(self, fn)(FWD(input_tapes) | child<J, I> ...);
			};
			const auto result_data = [&]<size_t...I>(std::index_sequence<I...>)
			{
				constexpr auto s = std::index_sequence_for<Views...>{};
				return tuple<decltype(result_data_at.template operator()<I>(s))...>
				{
					result_data_at.template operator()<I>(s)...
				};
			};

			constexpr auto result_layouts = get_result_layouts<Seq, input_seq_map.map, unique_input_indices_map.map>(
				std::make_index_sequence<child_count<decltype(Seq)>>{});
			return tape_t<decltype(result_data(std::make_index_sequence<n_unique_input_indices>{})), result_layouts>
			{
				result_data(std::make_index_sequence<n_unique_input_indices>{})
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
				if constexpr(indices<decltype(IndexTree)>)
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
		static consteval auto get_view_seq_and_map()
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
			else if constexpr(not indices<decltype(Seq | child<I>)>)
			{
				static_assert(indices<decltype(Seq | child<I>)>, "Invalid sequence.");
			}
			else if constexpr((Seq | child<I>).size() == 0uz)
			{
				return [&]<size_t...I>(std::index_sequence<I...>)
				{
					map = array{ I... };
					return tuple{ array{I}... };
				}(std::make_index_sequence<child_count<zip_transform_view>>{});
			}
			else if constexpr(tuple_contain(Cur, array{ (Seq | child<I>)[0] } ))
			{
				return get_input_seq_and_map_impl<Seq, I + 1uz, Cur>(map);
			}
			else
			{
				constexpr size_t index = (Seq | child<I>)[0];
				map[index] = child_count<decltype(Cur)>;
				return get_input_seq_and_map_impl<Seq, I + 1uz, tuple_cat(Cur, tuple{ array{ index } })>(map);
			}
		}

		template<auto Seq>
		static consteval auto get_input_seq_and_map()
		{
			array<size_t, child_count<zip_transform_view>> map{};
			auto seq = get_input_seq_and_map_impl<Seq>(map);
			struct result_t
			{
				decltype(seq) seq;
				array<size_t, child_count<zip_transform_view>> map;
			};
			return result_t{ seq, map };
		}

		template<typename TInputTapes, size_t...I>
		static constexpr auto get_input_tape_layouts_zip(std::index_sequence<I...>)
		{
			constexpr auto input_tape_layouts_zip_at = []<size_t J, size_t...K>(std::index_sequence<K...>)
			{
				return tuple<purified<decltype(purified<child_type<TInputTapes, K>>::sequence | child<J>)>...>
				{
					purified<child_type<TInputTapes, K>>::sequence | child<J> ... 
				};
			};
			constexpr auto s = std::index_sequence_for<Views...>{};
			return tuple<decltype(input_tape_layouts_zip_at.template operator()<I>(s))...>
			{
				input_tape_layouts_zip_at.template operator()<I>(s)...
			};
		}//(std::make_index_sequence<child_count<decltype(input_seq_map.seq)>>{});

		template<auto InputLayoutsZip, size_t I = 0uz, auto Cur = tuple{}, auto CurLayouts = tuple{}>
		static constexpr auto get_unique_input_indices_and_map_impl(auto& map)noexcept
		{
			if constexpr(I >= child_count<decltype(InputLayoutsZip)>)
			{
				return Cur;
			}
			else if constexpr(tuple_contain(CurLayouts, InputLayoutsZip | child<I>))
			{
				return get_unique_input_indices_and_map_impl<InputLayoutsZip, I + 1uz, Cur, CurLayouts>(map);
			}
			else
			{
				map[I] = child_count<decltype(Cur)>;
				return get_unique_input_indices_and_map_impl<
					InputLayoutsZip, I + 1uz,
					tuple_cat(Cur, tuple{ I }),
					tuple_cat(CurLayouts, tuple<purified<decltype(InputLayoutsZip | child<I>)>>{ InputLayoutsZip | child<I> })
				>(map);
			}
		}

		template<auto InputLayoutsZip>
		static consteval auto get_unique_input_indices_and_map()
		{
			array<size_t, child_count<decltype(InputLayoutsZip)>> map{};
			auto indices = get_unique_input_indices_and_map_impl<InputLayoutsZip>(map);
			struct result_t
			{
				decltype(indices) indices;
				array<size_t, child_count<decltype(InputLayoutsZip)>> map;
			};
			return result_t{ indices, map };
		}

		static constexpr auto set_result_seq(auto& seq, const auto& view_map, const auto& unique_view_tape_map)noexcept
		{
			if constexpr(indices<decltype(seq)>)
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
		static consteval auto get_result_layouts(std::index_sequence<I...>)
		{
			auto seq = Seq;
			set_result_seq(seq, InputMap, UniqueInputMap);
			return seq;
			// constexpr auto result_layouts_at = []<size_t J>()
			// {
			// 	auto indices = Seq | child<J>;
			// 	if constexpr(indices.size() == 0)
			// 	{
			// 		return indices;
			// 	}
			// 	else
			// 	{
			// 		indices[0] = UniqueInputMap[InputMap[indices[0]]];
			// 	}
			// 	return indices;
			// };
			// return tuple{ result_layouts_at.template operator()<I>()... };
		}//(std::make_index_sequence<child_count<decltype(Seq)>>{});
	};

	namespace detail
	{
		struct zip_transform_t
		{
			template<typename Fn, typename...T>
			RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn, T&&...trees)const
			{
				return detail::zip_transform_view<std::decay_t<Fn>, T...>
				{
					{}, FWD(fn), tuple<T...>{ FWD(trees)... }
				};
			}
		};

        struct zip_t
		{
            static constexpr auto zip_fn = [](auto&&...args)
            {
                return fwd_as_tuple(FWD(args)...);
            };

			template<typename...T>
			RUZHOUXIE_INLINE constexpr auto operator()(T&&...trees)const
			{
				return detail::zip_transform_view<tag_t<zip_fn>, T...>
				{
					{}, zip_fn, tuple<T...>{ FWD(trees)... }
				};
			}
		};

		struct transform_t
		{
			template<typename Fn>
			RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn)const
			{
				return tree_adaptor_closure
				{
					[fn = FWD(fn)]<typename View>(this auto&& self, View&& view)
					{
						return zip_transform_view<std::decay_t<Fn>, View>
						{
							{}, FWDLIKE(self, fn), tuple<View>{ FWD(view) }
						};
					}
				};
			}
		};
	}

	inline constexpr detail::zip_transform_t zip_transform{};
    inline constexpr detail::zip_t zip{};
	inline constexpr tree_adaptor<detail::transform_t> transform{};
}

#include "macro_undef.h"
#endif