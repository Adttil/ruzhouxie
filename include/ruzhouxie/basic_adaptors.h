#ifndef RUZHOUXIE_BASIC_ADAPTORS_H
#define RUZHOUXIE_BASIC_ADAPTORS_H

#include "array.h"
#include "general.h"
#include "ruzhouxie/array.h"
#include "tape.h"
#include "get.h"
#include "ruzhouxie/macro_define.h"
#include "tuple.h"
#include "processer.h"
#include "macro_define.h"
#include <array>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>

//view_base
namespace ruzhouxie
{
	template<typename T>
	struct view;

	namespace detail
	{
		template<typename View>
		struct universal_view
		{
			View raw_view;

			template<typename U>
			constexpr operator U(this auto&& self) 
				requires requires{ FWD(self, raw_view) | make_tree<U>; }
			{
				return FWD(self, raw_view) | make_tree<U>;
			}
		};

		template<typename View>
		struct view_base
		{
			template<typename Self>
			constexpr auto operator+(this Self&& self)
			{
				return universal_view<Self&&>{ FWD(self) };
			}
		};
	}
}

//view
namespace ruzhouxie
{
	template<typename T>
	struct view : wrapper<T>, detail::view_base<view<T>>
	{
		template<size_t I, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | child<I>)

		template<auto Seq, specified<view> Self>
		RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			AS_EXPRESSION(as_base<wrapper<T>>(FWD(self)).value() | get_tape<Seq>)
		// template<specified<view> Self>
		// RUZHOUXIE_INLINE friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
		// 	AS_EXPRESSION(id_tree<T>())
	};

	template<typename T>
	view(T&&) -> view<T>;

	template<typename T>
	struct tree_maker_trait<view<T>>
	{
		struct type1// : processer<type1>
		{
			static constexpr tree_maker<T> maker{};

			template<typename U>
			static consteval auto get_sequence()
			{
				return maker.template get_sequence<U>();
			}

			template<typename U, typename Tape>
			constexpr auto process_tape(Tape&& tape)const
			{
				return view<T>{ maker.template process_tape<U>(FWD(tape)) };
			}

			template<typename Tree>
			RUZHOUXIE_INLINE constexpr auto operator()(Tree&& tree)const
			{
				return process_tape<Tree&&>(FWD(tree) | get_tape<get_sequence<Tree&&>()>);
			}
			//AS_EXPRESSION(process_tape<Tree&&>(FWD(tree) | get_tape<get_sequence<Tree&&>()>))
		};

		using type = type1;
	};

	namespace detail
	{
		struct as_ref_t
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (std::is_rvalue_reference_v<T&&>)
				{
					return view<T&&>{ FWD(t) };
				}
				else
				{
					return t;
				}
			}
		};
	};

	inline namespace functors
	{
		inline constexpr pipe_closure<detail::as_ref_t> as_ref{};
	}
}

//relayout
namespace ruzhouxie
{
	namespace detail
	{
		// template<auto layout>
		// constexpr auto relayout_id_tree(const auto& tree) 
		// {
		// 	using layout_type = purified<decltype(layout)>;
		// 	return[&]<size_t...I>(std::index_sequence<I...>)
		// 	{
		// 		if constexpr (tensor_rank<layout_type> >= 2uz)
		// 		{
		// 			return tuple{ relayout_id_tree<layout | child<I>>(tree)... };
		// 		}
		// 		else
		// 		{
		// 			return id_tree_get<layout | child<I>...>(tree);
		// 		}
		// 	}(std::make_index_sequence<child_count<layout_type>>{});
		// }

		template<typename T, auto Layout>
		struct relayout_view : view_base<relayout_view<T, Layout>>
		{
			static constexpr auto layout = Layout;
			using layout_type = purified<decltype(Layout)>;

			RUZHOUXIE_MAYBE_EMPTY T raw_tree;

			template<size_t I, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
				//todo... noexcept)
			{
				if constexpr (I >= child_count<layout_type>)
				{
					return;
				}
				else if constexpr(indices<child_type<layout_type, I>>)
				{
					constexpr auto index_pack = Layout | child<I>;
					return FWD(self, raw_tree) | child<index_pack>;
				}
				else
				{
					return relayout_view<decltype(FWD(self, raw_tree)), Layout | child<I>>
					{
						{}, FWD(self, raw_tree)
					};
				}
			}

			template<auto Seq, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			{
				constexpr auto transformed_sequence = []<size_t...I>(std::index_sequence<I...>)
				{
					return tuple{ Layout | child<Seq | child<I>> ... };
				}(std::make_index_sequence<child_count<decltype(Seq)>>{});
				return FWD(self, raw_tree) | get_tape<transformed_sequence>;
			}
		};
	}

	namespace detail
	{
		template<auto layout>
		struct relayout_t
		{
			using layout_type = purified<decltype(layout)>;

			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (indices<layout_type>)
				{
					return FWD(t) | child<layout>;
				}
				else
				{
					return relayout_view<T, layout>{ {}, FWD(t) };
				}
			}
		};
	};

	inline namespace functors
	{
		template<auto layout>
		inline constexpr pipe_closure<detail::relayout_t<layout>> relayout{};
	}
}

//zip transform
namespace ruzhouxie
{
	namespace detail
	{
		template<typename Fn, typename...T>
		struct zip_transform_view;
	}

	template<typename Fn, typename...T>
	struct detail::zip_transform_view : view_base<zip_transform_view<Fn, T...>>
	{
		RUZHOUXIE_MAYBE_EMPTY Fn fn;
		RUZHOUXIE_MAYBE_EMPTY tuple<T...> trees;

		template<size_t I, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
			//todo...noexcept
		{
			if constexpr ((... || (I >= child_count<T>)))
			{
				return;
			}
			else return[&]<size_t...J>(std::index_sequence<J...>) -> decltype(auto)
			{
				return FWD(self, fn)(FWD(self, trees) | child<J, I>...);
			}(std::make_index_sequence<sizeof...(T)>{});
		}

		template<auto Seq, specified<zip_transform_view> View>
		struct tape_type
		{
			template<size_t...I>
			static constexpr auto init_tapes(View&& view, std::index_sequence<I...>)
			{
				return tuple<decltype(FWD(view, trees) |  child<I> | get_tape<Seq>)...>
				{
					FWD(view, trees) |  child<I> | get_tape<Seq>... 
				};
			}
			
			View view;
			decltype(init_tapes(declval<View&&>(), std::index_sequence_for<T...>{})) tapes;

			explicit constexpr tape_type(View&& view) 
				: view(FWD(view))
				, tapes(init_tapes(FWD(view), std::index_sequence_for<T...>{}))
			{}

			template<size_t I, specified<tape_type> Self>
			friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
			{
				if constexpr(I >= child_count<decltype(Seq)>)
				{
					return;
				}
				else 
				{
					return [&]<size_t...J>(std::index_sequence<J...>) -> decltype(auto)
					{
						return self.view.fn(FWD(self, tapes) | child<J, I> ...);
					}(std::index_sequence_for<T...>{});
				}
			}
		};

		template<auto Seq, specified<zip_transform_view> Self>
		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			//todo...noexcept
		{
			constexpr auto input_seq_map = get_input_seq_and_map<Seq>();

			auto input_tapes = [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple<decltype(FWD(self, trees) | child<I> | get_tape<input_seq_map.seq>)...>
				{
					FWD(self, trees) | child<I> | get_tape<input_seq_map.seq>... 
				};
			}(std::index_sequence_for<T...>{});

			using input_tapes_type = decltype(input_tapes);

			constexpr auto input_tape_layouts_zip = get_input_tape_layouts_zip<input_tapes_type>(
				std::make_index_sequence<child_count<decltype(input_seq_map.seq)>>{});

			constexpr auto unique_input_indices_map = get_unique_input_indices_and_map<input_tape_layouts_zip>();

			const auto result_data_at = [&]<size_t I, size_t...J>(std::index_sequence<J...>) -> decltype(auto)
			{
				return FWD(self, fn)(FWD(input_tapes) | child<J, I> ...);
			};

			const auto result_data = [&]<size_t...I>(std::index_sequence<I...>)
			{
				constexpr auto s = std::index_sequence_for<T...>{};
				return tuple<decltype(result_data_at.template operator()<I>(s))...>
				{
					result_data_at.template operator()<I>(s)...
				};
			};

			constexpr auto result_layouts = get_result_layouts<Seq, input_seq_map.map, unique_input_indices_map.map>
				(std::make_index_sequence<child_count<decltype(Seq)>>{});

			return tape_t<
					decltype(result_data(std::make_index_sequence<child_count<decltype(unique_input_indices_map.indices)>>{})),
					result_layouts
				>
				{
					result_data(std::make_index_sequence<child_count<decltype(unique_input_indices_map.indices)>>{})
				};
			
			//...........................................................................................................
			
			// constexpr auto sequence = []<size_t...I>(std::index_sequence<I...>)
			// {
			// 	return tuple{ array{ I }... };
			// }(std::make_index_sequence<child_count<Self>>{});

			// auto args_tape = [&]<size_t...I>(std::index_sequence<I...>)
			// {
			// 	return tuple<decltype(FWD(self, trees) | child<I> | get_tape<sequence>)...>
			// 	{
			// 		FWD(self, trees) | child<I> | get_tape<sequence> ... 
			// 	};
			// }(std::index_sequence_for<T...>{});

			// auto data_get = [&]<size_t I, size_t...J>(std::index_sequence<J...>)
			// {
			// 	return FWD(self, fn)(FWD(args_tape) | child<J, I>...);
			// };

			// auto data = [&]<size_t...I>(std::index_sequence<I...>)
			// {
			// 	constexpr auto s = std::index_sequence_for<T...>{};
			// 	return tuple<decltype(data_get.template operator()<I>(s))...>
			// 	{
			// 		data_get.template operator()<I>(s)... 
			// 	};
			// };

			// return tape_t<decltype(data(std::make_index_sequence<child_count<Self>>{})), Seq>
			// {
			// 	data(std::make_index_sequence<child_count<Self>>{}) 
			// };
		}

	private:
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
				return tuple<purified<decltype(purified<child_type<TInputTapes, K>>::layouts | child<J>)>...>
				{
					purified<child_type<TInputTapes, K>>::layouts | child<J> ... 
				};
			};
			constexpr auto s = std::index_sequence_for<T...>{};
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

		

		template<auto Seq, auto InputMap, auto UniqueInputMap, size_t...I>
		static consteval auto get_result_layouts(std::index_sequence<I...>)
		{
			constexpr auto result_layouts_at = []<size_t J>()
			{
				auto indices = Seq | child<J>;
				if constexpr(indices.size() == 0)
				{
					return indices;
				}
				else
				{
					indices[0] = UniqueInputMap[InputMap[indices[0]]];
				}
				return indices;
			};
			return tuple{ result_layouts_at.template operator()<I>()... };
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

		struct transform_t
		{
			template<typename Fn, typename T>
			RUZHOUXIE_INLINE constexpr auto operator()(Fn&& fn, T&& tree)const
			{
				return zip_transform_view<std::decay_t<Fn>, T>
				{
					{}, FWD(fn), tuple<T>{ FWD(tree) }
				};
			}
		};
	}

	inline constexpr detail::zip_transform_t zip_transform{};
	inline constexpr pipe_closure<detail::transform_t, 2> transform{};
}

#include "macro_undef.h"
#endif