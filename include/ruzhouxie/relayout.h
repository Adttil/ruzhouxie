#ifndef RUZHOUXIE_RELAYOUT_H
#define RUZHOUXIE_RELAYOUT_H

#include "general.h"
#include "tree_view.h"

#include "macro_define.h"

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

			template<auto Indices, typename TLayout>
			static constexpr auto mapped_indices(const TLayout& layout)
			{
				if constexpr(indices<TLayout>)
				{
					return concat_array(layout, Indices);
				}
				else if constexpr(Indices.size() == 0uz)
				{
					return layout;
				}
				else
				{
					return mapped_indices<array_drop<1uz>(Indices)>(layout | child<Indices[0uz]>);
				}
			}

			template<auto Layout, typename Trans>
			static constexpr auto mapped_layout(const Trans& trans)
			{
				if constexpr(indices<decltype(Layout)>)
				{
					return mapped_indices<Layout>(trans);
				}
				else return[&]<size_t...I>(std::index_sequence<I...>)
				{
					return tuple<decltype(mapped_layout<Layout | child<I>>(trans))...>
					{
						mapped_layout<Layout | child<I>>(trans)...
					};
				}(std::make_index_sequence<child_count<decltype(Layout)>>{});
			}

			template<auto Seq, specified<relayout_view> Self>
			RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<get_tape<Seq>>, Self&& self)
			{
				// constexpr auto transformed_sequence = []<size_t...I>(std::index_sequence<I...>)
				// {
				// 	return tuple{ Layout | child<Seq | child<I>> ... };
				// }(std::make_index_sequence<child_count<decltype(Seq)>>{});

				constexpr auto transformed_sequence = mapped_layout<Seq>(Layout);
				return FWD(self, raw_tree) | get_tape<transformed_sequence>;
			}
		};
	}

	namespace detail
	{
		template<auto Layout>
		struct relayout_t
		{
			using layout_type = purified<decltype(Layout)>;

			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (indices<layout_type>)
				{
					return FWD(t) | child<Layout>;
				}
				else
				{
					return relayout_view<T, Layout>{ {}, FWD(t) };
				}
			}
		};

		template<size_t N>
		struct repeat_t
		{
			static constexpr auto layout = []<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ array<size_t, I - I>{}... };
			}(std::make_index_sequence<N>{});

			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (N == 1)
				{
					return T{ FWD(t) };
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
		template<auto Layout>
		inline constexpr pipe_closure<detail::relayout_t<Layout>> relayout{};

		template<size_t N>
		inline constexpr pipe_closure<detail::repeat_t<N>> repeat{};
	}
}

#include "macro_undef.h"
#endif