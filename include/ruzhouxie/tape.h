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

	enum class access_mode
	{
		unknown,
		pass,
		once
	};

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

	template<typename Data, auto Sequence>
	struct tape_t
	{
		using data_type = Data;
		static constexpr auto sequence = Sequence;
		//Here, it is necessary to ensure that each child of data is independent.
		//Each child of data is also necessary to meet the above requirements.
		Data data;

		template<size_t I, specified<tape_t> Self>
		friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
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
					return (layout_relation::independent | ... | layout_relation_to(layout, Sequence | child<J + I + 1uz>));
				}(std::make_index_sequence<child_count<decltype(Sequence)> - I - 1uz>{});

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

		template<size_t I, specified<tape_t> Self>
		friend constexpr decltype(auto) access_once(Self&& self)
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
					return (layout_relation::independent | ... | layout_relation_to(layout, Sequence | child<J + I + 1uz>));
				}(std::make_index_sequence<child_count<decltype(Sequence)> - I - 1uz>{});

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

		template<size_t I, specified<tape_t> Self>
		friend constexpr decltype(auto) access_pass(Self&& self)
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
					constexpr auto front = (layout_relation::independent | ... | layout_relation_to(layout, Sequence | child<J>));
					constexpr auto behind = (layout_relation::independent | ... | layout_relation_to(layout, Sequence | child<K + I + 1uz>));
					return front | behind;
				}(std::make_index_sequence<I>{}, std::make_index_sequence<child_count<decltype(Sequence)> - I - 1uz>{});

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

		template<size_t I, access_mode Mode = access_mode::unknown, specified<tape_t> Self = void>
		friend constexpr decltype(auto) access(Self&& self)
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
	};

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