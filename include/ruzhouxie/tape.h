#ifndef RUZHOUXIE_ID_H
#define RUZHOUXIE_ID_H

#include "general.h"
#include "pipe_closure.h"
#include "ruzhouxie/array.h"
#include "ruzhouxie/macro_define.h"
#include "tuple.h"
#include "array.h"
#include "get.h"
#include "macro_define.h"
#include <array>
#include <initializer_list>
#include <utility>

namespace ruzhouxie
{
	namespace detail
	{
		template<typename T, auto Layout>
		struct relayout_view;
	}

	template<typename T, auto...ReservedLayouts>
	struct reserved_view;

	enum class layout_relation
	{
		independent,
		intersectant,
		included
	};

	constexpr layout_relation operator|(layout_relation l, layout_relation r)noexcept
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

	constexpr layout_relation operator&(layout_relation l, layout_relation r)noexcept
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
	constexpr layout_relation indices_relation_to(const array<size_t, N1>& index_pack, const array<size_t, N2>& other)
	{
		if constexpr(N1 >= N2)
		{
			if(array_take<N2>(index_pack) == other)
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
			if(array_take<N1>(other) == index_pack)
			{
				return layout_relation::intersectant;
			}
			else
			{
				return layout_relation::independent;
			}
		}
	}

	constexpr layout_relation indices_relation_to_layout(const indices auto& index_pack, const auto& layout)
	{
		if constexpr(indices<decltype(layout)>)
		{
			return indices_relation_to(index_pack, layout);
		}
		else return [&]<size_t...I>(std::index_sequence<I...>)
		{
			return (... | indices_relation_to_layout(index_pack, layout | child<I>));
		}(std::make_index_sequence<child_count<decltype(layout)>>{});
	}

	constexpr layout_relation layout_relation_to(const auto& layout, const auto& other)
	{
		if constexpr(indices<decltype(layout)>)
		{
			return indices_relation_to_layout(layout, other);
		}
		else return [&]<size_t...I>(std::index_sequence<I...>)
		{
			return (... & layout_relation_to(layout | child<I>, other));
		}(std::make_index_sequence<child_count<decltype(layout)>>{});
	}

	template<typename Data, auto Layouts>
	struct tape_t
	{
		using data_type = Data;
		static constexpr auto layouts = Layouts;
		//Here, it is necessary to ensure that each child of data is independent.
		//Each child of data is also necessary to meet the above requirements.
		Data data;

		template<size_t I, specified<tape_t> Self>
		friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
		{
			if constexpr(I >= child_count<decltype(Layouts)>)
			{
				return;
			}
			else
			{
				constexpr auto layout = Layouts | child<I>;
				constexpr auto relation_to_rest = []<size_t...J>(std::index_sequence<J...>)
				{
					//for some compiler.
					constexpr auto layout = Layouts | child<I>;
					return (layout_relation::independent | ... | layout_relation_to(layout, Layouts | child<J + I + 1uz>));
				}(std::make_index_sequence<child_count<decltype(Layouts)> - I - 1uz>{});

				if constexpr(indices<decltype(layout)>)
				{
					if constexpr(relation_to_rest == layout_relation::independent)
					{
						return FWD(self, data) | child<layout>;
					}
					else
					{
						return std::as_const(self.data) | child<layout>;
					}
				}
				else if constexpr(relation_to_rest == layout_relation::independent)
				{
					return detail::relayout_view<decltype(FWD(self, data)), layout>
					{
						{}, FWD(self, data)
					};
				}
				// else if constexpr(relation_to_rest == layout_relation::included)
				// {
				// 	return relayout_view<decltype(as_const(self.data)), layout>
				// 	{
				// 		as_const(self.data)
				// 	};
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
	};

	namespace detail::get_tape_t_ns
	{
		template<auto Sequence>
		void tag_invoke();

		template<auto Sequence>
		struct get_tape_t;
	}

	template<auto Sequence>
	constexpr inline pipe_closure<detail::get_tape_t_ns::get_tape_t<Sequence>> get_tape{};

	template<typename T, auto Sequence>
	struct terminal_tape
	{
		T value;
		template<size_t I, specified<terminal_tape> Self>
		friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
		{
			if constexpr(I >= child_count<decltype(Sequence)>)
			{
				return;
			}
			else if constexpr(I == child_count<decltype(Sequence)> - 1uz)
			{
				return FWD(self, value);
			}
			else
			{
				return as_const(self.value);
			}
		}
	};

	namespace detail
	{
		struct child_sequence_location
		{
			size_t child = invalid_index;
			size_t index = invalid_index;
		};

		template<size_t I, auto Sequence, size_t J = 0uz, auto Current = tuple{}>
		constexpr auto child_sequence(auto& map)
		{
			if constexpr(J >= child_count<decltype(Sequence)>)
			{
				return Current;
			}
			else
			{
				constexpr auto index_pack = Sequence | child<I>;
				if constexpr(index_pack.size() == 0uz)
				{
					return child_sequence<I, Sequence, J + 1uz, tuple_cat(Current, tuple{ index_pack })>(map);
				}
				else if constexpr(index_pack[0] != I)
				{
					return child_sequence<I, Sequence, J + 1uz, Current>(map);
				}
				else
				{
					map[J].child = I;
					map[J].index = child_count<decltype(Current)>;
					return child_sequence<I, Sequence, J + 1uz, tuple_cat(Current, tuple{ array_drop<1>(index_pack) })>(map);
				}
			}
		}

		template<auto Sequence, size_t ChildCount>
		constexpr auto children_sequnce_and_map()
		{
			constexpr size_t n = child_count<decltype(Sequence)>;
			std::array<child_sequence_location, n> map{};
			auto children_sequence = [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ child_sequence<I, Sequence>(map)... };
			}(std::make_index_sequence<ChildCount>{});

			struct result_t
			{
				 decltype(children_sequence) sequences;
				 std::array<child_sequence_location, n> map;
			};

			return result_t{ children_sequence, map };
		}
	}

	template<typename T, auto Sequence>
	struct tuple_tape
	{
		static constexpr auto children_sequnce_map = detail::children_sequnce_and_map<Sequence, child_count<T>>();
		static constexpr const auto& children_sequences = children_sequnce_map.sequences;
		static constexpr const auto& map = children_sequnce_map.map;

		static constexpr auto init_children_tape(T&& t)
		{
			return [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple<decltype(FWD(t) | child<I> | get_tape<children_sequences | child<I>>)...>
				{
					FWD(t) | child<I> | get_tape<children_sequences | child<I>> ... 
				};
			}(std::make_index_sequence<child_count<T>>{});
		}
        
		T tpl;
		decltype(init_children_tape(declval<T&&>())) children_tapes;

		constexpr tuple_tape(T&& tpl)
			: tpl(FWD(tpl))
			, children_tapes(init_children_tape(FWD(tpl)))
		{}

		template<size_t I, specified<tuple_tape> Self>
		friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
		{
			if constexpr(I >= child_count<decltype(Sequence)>)
			{
				return;
			}
			else if constexpr((Sequence | child<I>).size() == 0uz)
			{
				if constexpr(I == child_count<decltype(Sequence)> - 1uz)
				{
					return FWD(self, tpl);
				}
				else
				{
					return as_const(self.tpl);
				}
			}
			else
			{
				decltype(auto) child_tape = FWD(self, children_tapes) | child<map[I].child>;
				return FWD(child_tape) | child<map[I].index>;//
			}
		}
	};

	template<size_t Offset, typename Tape>
	struct sub_tape_t
	{
        Tape tape;

		template<size_t I, specified<sub_tape_t> Self>
		friend constexpr auto tag_invoke(tag_t<child<I>>, Self&& self)
			AS_EXPRESSION(FWD(self, tape) | child<I + Offset>)
	};

	template<size_t N, typename Tape>
	constexpr auto tape_drop(Tape&& tape) noexcept
	{
		using tape_type = purified<Tape>;
		constexpr auto layouts = tuple_drop<N>(tape_type::layouts);
		return tape_t<decltype(FWD(tape, data)), layouts>{ FWD(tape, data) };
	}

	template<auto Sequence>
	struct detail::get_tape_t_ns::get_tape_t
	{
		template<typename T>
		constexpr decltype(auto) operator()(T&& t)const
		{
			if constexpr (requires{ tag_invoke<Sequence>(get_tape<Sequence>, FWD(t)); })
			{
				return tag_invoke<Sequence>(get_tape<Sequence>, FWD(t));
			}
			else
			{
				return tape_t<T&&, Sequence>{ FWD(t) };
			}
			// else return[&]<size_t...I>(std::index_sequence<I...>)
			// {
			// 	return tuple<decltype(FWD(t) | child<Sequence | child<I>>)...>
			// 	{
			// 		FWD(t) | child<Sequence | child<I>>...
			// 	};
			// }(std::make_index_sequence<child_count<decltype(Sequence)>>{});
		}
	};
}

#include "macro_undef.h"
#endif