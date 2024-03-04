#ifndef RUZHOUXIE_RELAYOUT_H
#define RUZHOUXIE_RELAYOUT_H

#include "general.h"
#include "get.h"
#include "pipe_closure.h"
#include "tree_view.h"
#include "constant.h"

#include "macro_define.h"
#include <utility>

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
		struct relayout_t : pipe_closure<relayout_t<Layout>>
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
		struct repeat_t : pipe_closure<repeat_t<N>>
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
		inline constexpr detail::relayout_t<Layout> relayout{};

		template<size_t N>
		inline constexpr detail::repeat_t<N> repeat{};
	}
}

namespace ruzhouxie
{
	namespace detail
	{
		template<size_t M, size_t N>
		constexpr auto tensor_layout_add_prefix(const array<size_t, M>& layout, const array<size_t, N>& prefix)
		{
			return concat_array(prefix, layout);
		}

		template<typename T, size_t N> requires (tensor_rank<T> >= 2uz)
			constexpr auto tensor_layout_add_prefix(const T& layout, const array<size_t, N>& prefix)
		{
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return array{ tensor_layout_add_prefix(layout | child<I>, prefix)... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	}

	template<typename T>
	constexpr auto default_tensor_layout
	{
		[]()
		{
			if constexpr (terminal<T>)
			{
				return array<size_t, 0uz>{};
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				return array{ detail::tensor_layout_add_prefix(default_tensor_layout<child_type<T, I>>, array{I})... };
			}(std::make_index_sequence<child_count<T>>{});
		}()
	};
}

namespace ruzhouxie
{
	template<size_t I, size_t Axis = 0uz, typename T = void>
	constexpr auto component_copy(const T& t)
	{
		if constexpr (Axis == 0uz)
		{
			static_assert(I < child_count<T>, "Component index out of range.");
			return t | child<I>;
		}
		else
		{
			static_assert(branched<T>, "Axis index out of range.");
			return[&]<size_t...J>(std::index_sequence<J...>)
			{
				return tuple{ component_copy<I, Axis - 1uz>(t | child<J>)... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	}

	template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz, typename T = void>
	constexpr auto transpose_copy(const T& t)
	{
		if constexpr (Axis1 == 0uz)
		{
			constexpr size_t N = tensor_shape<T>[Axis2];
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ component_copy<I, Axis2>(t)... };
			}(std::make_index_sequence<N>{});
		}
		else return[&]<size_t...I>(std::index_sequence<I...>)
		{
			return tuple{ transpose_copy<Axis1 - 1uz, Axis2 - 1uz>(t | child<I>)... };
		}(std::make_index_sequence<child_count<T>>{});
	}

    
}

//component
namespace ruzhouxie
{
	namespace detail
	{
		template<size_t I, size_t Axis>
		struct component_t : pipe_closure<component_t<I, Axis>>
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				if constexpr (Axis == 0)
				{
					return t | child<I>;
				}
				else
				{
					constexpr auto tensor_layout = default_tensor_layout<T>;
					return relayout_view<T, component_copy<I, Axis>(tensor_layout)>
					{
						{}, FWD(t)
					};
				}
			}
		};
	};

	inline namespace functors
	{
		template<size_t J, size_t Axis>
		inline constexpr detail::component_t<J, Axis> component{};
	}
}

//transpose
namespace ruzhouxie
{
	namespace detail
	{
		template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
		struct transpose_t : pipe_closure<transpose_t<Axis1, Axis2>>
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				constexpr auto tensor_layout = default_tensor_layout<T>;
				return relayout_view<T, transpose_copy<Axis1, Axis2>(tensor_layout)>
				{
					{}, FWD(t)
				};
			}
		};
	};

	template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
	inline constexpr detail::transpose_t<Axis1, Axis2> transpose{};
}

//transpose
namespace ruzhouxie
{
    constexpr size_t mod(size_t I, size_t N)
    {
        return I % N;
    }

    //todo...add Aixs
    template<size_t Begin, size_t Count/*, size_t Axis = 0uz*/, typename T = void>
    constexpr auto range_copy(const T& t)
    {
        return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple<purified<decltype(t | child<mod(Begin + I, child_count<T>)>)>...>
            {
                t | child<mod(Begin + I, child_count<T>)>...
            };
        }(std::make_index_sequence<Count>{});
    }

	namespace detail
	{
		template<size_t Begin, size_t Count>
		struct range_t : pipe_closure<range_t<Begin, Count>>
		{
			template<typename T>
			RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t) const
			{
				constexpr auto tensor_layout = default_tensor_layout<T>;
				return relayout_view<T, range_copy<Begin, Count>(tensor_layout)>
				{
					{}, FWD(t)
				};
			}

            template<size_t I, specified<range_t> T> requires(I < Count)
	        friend constexpr auto tag_invoke(tag_t<child<I>>, T&&)noexcept
	        {
                return constant_t<Begin + I>{};
            }
		};

        
	};

	template<size_t Begin, size_t Count>
	inline constexpr detail::range_t<Begin, Count> range{};
    
    
}

#include "macro_undef.h"
#endif