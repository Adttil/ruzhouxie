#ifndef RUZHOUXIE_GET_H
#define RUZHOUXIE_GET_H

#include "pipe_closure.h"
#include "array.h"
#include "macro_define.h"

namespace ruzhouxie
{
	template<size_t N = 0uz, typename Tag = void>
	struct id_set
	{
		using tag_type = Tag;
		array<size_t, N> ids;

		/*constexpr explicit id_set(const array<size_t, N>& ids) : ids(ids) {}
		template<std::same_as<size_t>...T>
		constexpr explicit id_set(T...ids) : ids{ ids... } {}*/

		friend constexpr bool operator==(const id_set&, const id_set&) = default;

		static constexpr bool empty() noexcept
		{
			return N == 0;
		}

		constexpr bool coutain(size_t target_id)const noexcept
		{
			for (size_t id : ids)
			{
				if (id == target_id)
				{
					return true;
				}
			}
			return false;
		}

		static constexpr bool untagged() noexcept
		{
			return std::same_as<Tag, void>;
		}

		constexpr bool invalid()const noexcept
		{
			if constexpr (N == 0)
			{
				return false;
			}
			else
			{
				return ids[N - 1] == invalid_index;
			}
		}

		template<size_t M>
		constexpr bool independent_to(const id_set<M, Tag>& reserved_ids)const noexcept
		{
			if (reserved_ids.invalid())
			{
				return false;
			}
			else if (invalid())
			{
				return reserved_ids.empty();
			}
			for (auto id : ids)
			{
				if (reserved_ids.coutain(id)) return false;
			}
			return true;
		}

		template<size_t M, typename OTag>
		constexpr bool independent_to(const id_set<M, OTag>& reserved_ids)const noexcept
		{
			return reserved_ids.empty();
		}
	};

	template<size_t N>
	id_set(array<size_t, N>) -> id_set<N>;

	template<std::same_as<size_t>...T>
	id_set(T...) -> id_set<sizeof...(T)>;

	template<typename Tag = void, size_t N>
	constexpr id_set<N, Tag> make_id_set(array<size_t, N> arr)
	{
		return { arr };
	}

	template<id_set S1, id_set S2>
	constexpr auto merge_2_id_set()
	{
		using tag_type1 = purified<decltype(S1)>::tag_type;
		using tag_type2 = purified<decltype(S2)>::tag_type;

		if constexpr (S1.empty())
		{
			return S2;
		}
		else if constexpr (S2.empty())
		{
			return S1;
		}
		else if constexpr (S1.invalid())
		{
			return S1;
		}
		else if constexpr (S2.invalid())
		{
			return S2;
		}
		else if constexpr (S1.untagged() || S2.untagged())
		{
			return id_set{ invalid_index };
		}
		else if constexpr (not std::same_as<tag_type1, tag_type2>)
		{
			return id_set<1, tag_type1>{ invalid_index };
		}
		else
		{
			return make_id_set<tag_type1>(merge_2_array<S1.ids, S2.ids>());
		}
	}

	template<id_set...Set>
	constexpr auto merge_id_set()
	{
		if constexpr (sizeof...(Set) == 0)
		{
			return id_set{};
		}
		else if constexpr (sizeof...(Set) == 1)
		{
			return (..., Set);
		}
		else return[]<size_t...I>(std::index_sequence<I...>)
		{
			return merge_2_id_set<merge_id_set<arg_at<I>(Set...)...>(), last_arg(Set...)>();
		}(std::make_index_sequence<sizeof...(Set) - 1>{});
	}

	inline constexpr id_set<0, void> empty_id_set{};
	inline constexpr id_set<1, void> invalid_id_set{ invalid_index };
}

namespace ruzhouxie
{
	namespace detail
	{
		template<auto...I>
		struct child_t;
	}

	inline namespace functors
	{
		template<auto...I>
		inline constexpr pipe_closure<detail::child_t<I...>> child{};
	}

	template<typename T>
	struct getter_trait;

	template<typename T>
	using getter = getter_trait<T>::type;

	template<>
	struct detail::child_t<>
	{
		template<typename T>
		RUZHOUXIE_INLINE constexpr T&& operator()(T&& t)const noexcept
		{
			return FWD(t);
		}
	};

	//must has this for child return a pure-rvalue.
	template<size_t I>
	struct detail::child_t<I>
	{
		template<typename T>
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I>(FWD(t)))

		template<typename T> 
			requires (not requires{ getter<purified<T>>{}.template get<I>(declval<T>()); })
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I, invalid_id_set>(FWD(t)))
	};

	template<size_t I, auto Second>
	struct detail::child_t<I, Second>
	{
		static constexpr bool is_ids = not std::integral<purified<decltype(Second)>>;

		template<typename T> requires (is_ids)
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I, Second>(FWD(t)))

		template<typename T> requires (is_ids)
			&& (not requires{ getter<purified<T>>{}.template get<I, Second>(declval<T>()); })
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I>(FWD(t)))

		template<typename T> requires (not is_ids)
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I>(FWD(t)) | child<size_t{Second}>)
	};

	template<size_t I, auto...Rest>
	struct detail::child_t<I, Rest...>
	{
		/*template<typename T>
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I, Rest...>(FWD(t)))*/
		
		template<typename T> 
			//requires (not requires{ getter<purified<T>>{}.template get<I, Rest...>(declval<T>()); })
		RUZHOUXIE_INLINE constexpr auto operator()(T&& t)const
			AS_EXPRESSION(getter<purified<T>>{}.template get<I>(FWD(t)) | child<Rest...>)
	};

	template<typename T>
	inline constexpr size_t child_count = []<size_t N = 0uz>(this auto && self)
	{
		if constexpr (requires{ { declval<T&&>() | child<N> } -> concrete; })
		{
			return self.template operator() < N + 1uz > ();
		}
		else
		{
			return N;
		}
	}();

	template<typename T, size_t...I>
	using child_type = decltype(declval<T>() | child<I...>);

	template<typename T>
	concept terminal = child_count<T> == 0;

	template<typename T>
	concept branched = not terminal<T>;

	template<typename T>
	inline constexpr size_t leaf_count = []
		{
			if constexpr (terminal<T>)
			{
				return 1uz;
			}
			else
			{
				return[]<size_t...I>(std::index_sequence<I...>)
				{
					return (0uz + ... + leaf_count<decltype(declval<T&&>() | child<I>())>);
				}(std::make_index_sequence<child_count<T>>{});
			}
		}();

	template<typename T>
	inline constexpr auto tree_shape = []
		{
			if constexpr (terminal<T>)
			{
				return array{ '{', '}' };
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				return concat_array < array{ '{' }, tree_shape<decltype(child<I>(declval<T>()))>..., array{ '}' } > ();
			}(std::make_index_sequence<child_count<T>>{});
		}();

	template<typename T>
	inline constexpr size_t tensor_rank = []
		{
			if constexpr (terminal<T>)
			{
				return 0uz;
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				auto child_ranks = std::array{ tensor_rank<child_type<T, I>>... };
				size_t child_rank_min = std::numeric_limits<size_t>::max();
				for (size_t child_rank : child_ranks)
				{
					if (child_rank < child_rank_min)
					{
						child_rank_min = child_rank;
					}
				}
				return 1uz + child_rank_min;
			}(std::make_index_sequence<child_count<T>>{});
		}();

	template<typename T>
	inline constexpr auto tensor_shape = []
		{
			if constexpr (terminal<T>)
			{
				return array<size_t, 0>{};
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				constexpr size_t rank = tensor_rank<T>;
				array<size_t, rank> result{ child_count<T> };

				auto child_shapes = std::array{ array_take<rank - 1>(tensor_shape<child_type<T, I>>)... };

				for (size_t i = 0uz; i < rank - 1uz; ++i)
				{
					size_t child_axis_length_min = std::numeric_limits<size_t>::max();
					for (const auto& child_shape : child_shapes)
					{
						if (child_shape[i] < child_axis_length_min)
						{
							child_axis_length_min = child_shape[i];
						}
					}
					result[i + 1uz] = child_axis_length_min;
				}

				return result;
			}(std::make_index_sequence<child_count<T>>{});
		}();
}

namespace ruzhouxie
{
	struct invalid_getter {	};

	namespace detail::tag_invoke_getter_ns
	{
		//for adl find.
		template<auto...I>
		void tag_invoke();

		struct tag_invoke_getter
		{
			template<auto...I, typename T>
			RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(tag_invoke<I...>(child<I...>, FWD(t)))
		};
	}
	using detail::tag_invoke_getter_ns::tag_invoke_getter;

	struct array_getter
	{
		template<size_t I, typename T> requires (I < std::extent_v<purified<T>>)
			RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(FWD(t)[I])
	};

	struct member_getter
	{
		template<size_t I, typename T>
		RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(FWD(t).template get<I>())
	};

	struct adl_getter
	{
		template<size_t I, typename T>
		RUZHOUXIE_INLINE constexpr auto get(T&& t)const AS_EXPRESSION(get<I>(FWD(t)))
	};

	struct tuple_like_getter
	{
		enum strategy_t
		{
			none,
			member,
			adl
		};

		template<size_t I, typename T>
		static constexpr choice_t<strategy_t> choose()
		{
			using std::get;//for some std::get wich can not find by adl.
			if constexpr (not requires{ requires (I < size_t{ std::tuple_size<purified<T>>::value }); })
			{
				return { strategy_t::none };
			}
			else if constexpr (requires{ declval<T>().template get<I>(); })
			{
				return { strategy_t::member, noexcept(declval<T>().template get<I>()) };
			}
			else if constexpr (requires{ get<I>(declval<T>()); })
			{
				return { strategy_t::adl, noexcept(get<I>(declval<T>())) };
			}
			else
			{
				return { strategy_t::none };
			}
		}

		template<size_t I, typename T>
		RUZHOUXIE_INLINE constexpr decltype(auto) get(T&& t)const
			noexcept(choose<I, T&&>().nothrow)
			requires(choose<I, T&&>().strategy != none)
		{
			constexpr strategy_t strategy = choose<I, T&&>().strategy;
			if constexpr (strategy == strategy_t::member)
			{
				return FWD(t).template get<I>();
			}
			else if constexpr (strategy == strategy_t::adl)
			{
				using std::get;
				return get<I>(FWD(t));
			}
		}
	};

	struct aggregate_getter
	{
		struct universal_type
		{
			/*template<typename T>
				requires std::is_copy_constructible_v<T>
			operator T& ();

			template<typename T>
				requires std::is_move_constructible_v<T>
			operator T && ();

			template<typename T>
				requires(!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
			operator T();*/

			template<typename T>
			operator T& ();

			template<typename T>
			operator T && ();
		};

		static constexpr size_t aggregate_supported_to_get_max_size = 64uz;

		template <aggregated T>
		static constexpr size_t aggregate_member_count = []<bool had_success = false>(this auto && self, auto...args)
		{
			using type = purified<T>;
			if constexpr (sizeof...(args) > aggregate_supported_to_get_max_size)
			{
				return 0uz;
			}
			else if constexpr (requires{ type{ universal_type{ args }...,  {universal_type{}} }; })
				//else if constexpr (requires{ type{ {universal_type{ args }}...,  {universal_type{}} }; })
			{
				return self.template operator() < true > (args..., universal_type{});
			}
			else if constexpr (had_success)
			{
				return sizeof...(args);
			}
			else
			{
				return self.template operator() < false > (args..., universal_type{});
			}
		}();

		template<size_t I, aggregated T>
			requires (I < aggregate_member_count<T>)
		constexpr decltype(auto) get(T&& t)const noexcept
		{
			constexpr size_t n = aggregate_member_count<T>;

#include "generate/aggregate_getter_invoker.code"

		};
	};



	template<typename T>
	struct getter_trait
	{
		static consteval auto choose_default_getter() noexcept
		{
			if constexpr (requires{ tag_invoke_getter{}.get<0uz>(declval<T>()); }
			|| requires{ tag_invoke_getter{}.get<0uz, empty_id_set>(declval<T>()); })
			{
				return tag_invoke_getter{};
			}
			else if constexpr (std::is_bounded_array_v<purified<T>>)
			{
				return array_getter{};
			}
			else if constexpr (requires{ std::tuple_size<purified<T>>::value; })
			{
				return tuple_like_getter{};
			}
			else if constexpr (aggregated<T>)
			{
				return aggregate_getter{};
			}
			else
			{
				return invalid_getter{};
			}
		}

		using type = decltype(choose_default_getter());
	};
}

#include "macro_undef.h"
#endif