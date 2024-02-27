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
	

	namespace detail::get_tape_t_ns
	{
		template<auto Sequence, indices auto SubIndices>
		void tag_invoke();

		template<auto Sequence, indices auto SubIndices>
		struct get_tape_t;
	}

	template<auto Sequence, indices auto SubIndices = 
		[]<size_t...I>(std::index_sequence<I...>)
		{
			 return array<size_t, sizeof...(I)>{ I... };
		}(std::make_index_sequence<child_count<decltype(Sequence)>>{})>
	constexpr inline pipe_closure<detail::get_tape_t_ns::get_tape_t<Sequence, SubIndices>> get_tape{};

	template<typename T, auto Sequence, indices auto SubIndices>
	struct terminal_tape
	{
		T value;
		template<size_t I, specified<terminal_tape> Self>
		friend constexpr decltype(auto) tag_invoke(tag_t<child<I>>, Self&& self)
		{
			if constexpr(I >= SubIndices.size())
			{
				return;
			}
			else if constexpr(SubIndices[I] == child_count<decltype(Sequence)> - 1uz)
			{
				return FWD(self, value);
			}
			else
			{
				return (self.value);
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

		template<size_t I, auto Sequence, indices auto SubIndices, size_t J = 0uz, auto Current = tuple{}>
		constexpr auto child_sequence(auto& map)
		{
			if constexpr(J >= SubIndices.size())
			{
				return Current;
			}
			else
			{
				constexpr auto index = SubIndices[J];
				constexpr auto access = Sequence | child<index>;
				if constexpr(access.size() == 0uz)
				{
					return child_sequence<I, Sequence, SubIndices, J + 1uz, tuple_cat(Current, tuple{ access })>(map);
				}
				else if constexpr(access[0] != I)
				{
					return child_sequence<I, Sequence, SubIndices, J + 1uz, Current>(map);
				}
				else
				{
					map[J].child = I;
					map[J].index = child_count<decltype(Current)>;
					return child_sequence<I, Sequence, SubIndices, J + 1uz, tuple_cat(Current, tuple{ array_drop<1>(access) })>(map);
				}
			}
		}

		template<auto Sequence, indices auto SubIndices, size_t ChildCount>
		constexpr auto children_sequnce_and_map()
		{
			constexpr size_t n = SubIndices.size();
			std::array<child_sequence_location, n> map{};
			auto children_sequence = [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ child_sequence<I, Sequence, SubIndices>(map)... };
			}(std::make_index_sequence<ChildCount>{});

			struct result_t
			{
				 decltype(children_sequence) sequences;
				 std::array<child_sequence_location, n> map;
			};

			return result_t{ children_sequence, map };
		}
	}

	template<typename T, auto Sequence, indices auto SubIndices>
	struct tuple_tape
	{
		static constexpr auto children_sequnce_map = detail::children_sequnce_and_map<Sequence, SubIndices, child_count<T>>();
		static constexpr const auto& children_sequences = children_sequnce_map.sequences;
		static constexpr const auto& map = children_sequnce_map.map;

		static constexpr auto init_children_tape(T&& t)
		{
			return [&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple//<decltype(FWD(t) | child<I> | get_tape<children_sequences | child<I>>)...>
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
			if constexpr(I >= SubIndices.size())
			{
				return;
			}
			else if constexpr((Sequence | child<SubIndices[I]>).size() == 0uz)
			{
				if constexpr(SubIndices[I] == child_count<decltype(Sequence)> - 1uz)
				{
					return FWD(self, tpl);
				}
				else
				{
					return (self.tpl);
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

	template<size_t Offset, typename Tape>
	constexpr auto sub_tape(Tape&& tape) noexcept
	{
		return sub_tape_t<Offset, Tape&&>{ FWD(tape) };
	}

	template<auto Sequence, indices auto SubIndices>
	struct detail::get_tape_t_ns::get_tape_t
	{
		template<typename T>
		constexpr decltype(auto) operator()(T&& t)const
		{
			if constexpr (requires{ tag_invoke<Sequence, SubIndices>(get_tape<Sequence, SubIndices>, FWD(t)); })
			{
				return tag_invoke<Sequence, SubIndices>(get_tape<Sequence, SubIndices>, FWD(t));
			}
			else if constexpr(terminal<T>)
			{
				return terminal_tape<T&&, Sequence, SubIndices>{ FWD(t) };
			}
			else
			{
				return tuple_tape<T&&, Sequence, SubIndices>{ FWD(t) };
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

namespace ruzhouxie::detail
{
	template<typename T>
	concept principled = requires
	{
		requires (not std::same_as<decltype(getter_trait<T>::choose_default_getter()), tag_invoke_getter>);
	};
}

namespace ruzhouxie
{
	namespace detail::local_id_tree_t_ns
	{
		template<typename T>
		struct local_id_tree_t;

		template<typename T>
		void tag_invoke();
	};

	template<typename T>
	constexpr inline detail::local_id_tree_t_ns::local_id_tree_t<T> local_id_tree{};

	namespace detail
	{
		template<typename T>
		constexpr size_t local_id_tree_find_max(const T& id_tree)
		{
			if constexpr (!terminal<T>)
			{
				return[=]<size_t...I>(std::index_sequence<I...>)
				{
					auto max_ids = array{ local_id_tree_find_max(id_tree | child<I>)... };
					auto max_id = 0uz;
					for (auto id : max_ids)
					{
						if (id > max_id) max_id = id;
					}
					return max_id;
				}(std::make_index_sequence<child_count<T>>{});
			}
			else if constexpr (std::same_as<T, size_t>)
			{
				return id_tree;
			}
			else
			{
				return 0uz - 1uz;
			}
		}

		template<typename T>
		constexpr auto local_id_tree_each_plus(const T& id_tree, size_t value)
		{
			if constexpr (not terminal<T>)
			{
				return[&]<size_t...I>(std::index_sequence<I...>)
				{
					return tuple{ local_id_tree_each_plus(id_tree | child<I>, value)... };
				}(std::make_index_sequence<child_count<T>>{});
			}
			else if (id_tree == invalid_index)
			{
				return invalid_index;
			}
			else
			{
				return id_tree + value;
			}
		}

		template<auto result, auto tree, auto...rest>
		constexpr auto concat_local_id_tree()
		{
			if constexpr (requires{ requires (tree == invalid_index); })
			{
				return invalid_index;
			}
			else
			{
				constexpr size_t offset = local_id_tree_find_max(result) + 1uz;
				constexpr auto shift_tree = local_id_tree_each_plus(tree, offset);
				constexpr auto new_result = tuple_cat(result, tuple<std::decay_t<decltype(shift_tree)>>{ shift_tree });

				if constexpr (sizeof...(rest) == 0uz)
				{
					return new_result;
				}
				else
				{
					return concat_local_id_tree<new_result, rest...>();
				}
			}
		}
	}

	template<typename T>
	struct detail::local_id_tree_t_ns::local_id_tree_t
	{
		constexpr auto operator()()const
		{
			if constexpr (requires{ tag_invoke<T>(local_id_tree<T>); })
			{
				return tag_invoke<T>(local_id_tree<T>);
			}
			else if constexpr (std::is_fundamental_v<T>)
			{
				return 0uz;
			}
			else if constexpr (not principled<T>)
			{
				return invalid_index;
			}
			else if constexpr (terminal<T>)
			{
				return 0uz;
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				constexpr auto empty_tree = tuple{};
				return concat_local_id_tree<empty_tree, local_id_tree<child_type<T, I>>()...>();
			}(std::make_index_sequence<child_count<T>>{});
		}
	};

	template<typename T>
	concept independent = (not std::same_as<decltype(local_id_tree<T>()), size_t>) || (local_id_tree<T>() != invalid_index);

	template<auto tree, typename Tag = void>
	constexpr auto local_id_tree_to_set()
	{
		using tree_type = purified<decltype(tree)>;
		if constexpr (branched<tree_type>)
		{
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return merge_id_set<local_id_tree_to_set<tree | child<I>, Tag>()...>();
			}(std::make_index_sequence<child_count<tree_type>>{});
		}
		/*else 
		{
			return id_set<1, Tag>{ tree };
		}*/
		else if constexpr (std::same_as<tree_type, size_t>)
		{
			return id_set<1, Tag>{ tree };
		}
		else
		{
			static_assert(std::same_as<tree_type, size_t>, "invalid id-tree type.");
		}
	}

	
}

namespace ruzhouxie
{
	template<typename Tag = void>
	struct tagged_id
	{
		using tag_type = Tag;
		size_t value;

		constexpr explicit tagged_id(size_t value) : value(value) {}

		friend constexpr std::strong_ordering operator<=>(const tagged_id&, const tagged_id&) = default;
	};

	tagged_id(size_t) -> tagged_id<void>;

	template<typename Tag>
	constexpr auto tagged_local_id_tree(const auto& tree)
	{
		using tree_type = purified<decltype(tree)>;
		if constexpr (branched<tree_type>)
		{
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ tagged_local_id_tree<Tag>(tree | child<I>)... };
			}(std::make_index_sequence<child_count<tree_type>>{});
		}
		else if constexpr (std::same_as<tree_type, size_t>)
		{
			return tagged_id<Tag>{ tree };
		}
		else
		{
			static_assert(std::same_as<tree_type, size_t>, "invalid local id-tree type.");
		}
	}

	template<size_t I, size_t...Rest, typename Tree = void>
	constexpr auto id_tree_get(const Tree& tree) 
	{
		if constexpr(terminal<Tree>) 
		{
			return tree;
		}
		else if constexpr(sizeof...(Rest) == 0uz)
		{
			return tree | child<I>;
		}
		else
		{
			return id_tree_get<Rest...>(tree | child<I>);
		}
	}

	template<auto tree>
	constexpr auto id_tree_to_set()
	{
		using tree_type = purified<decltype(tree)>;
		if constexpr (branched<tree_type>)
		{
			return[&]<size_t...I>(std::index_sequence<I...>)
			{
				return merge_id_set<id_tree_to_set<tree | child<I>>()...>();
			}(std::make_index_sequence<child_count<tree_type>>{});
		}
		else 
		{
			//static_assert(std::same_as<tree_type, >);
			return id_set<1, typename tree_type::tag_type>{ tree.value };
		}
		/*else if constexpr (requires { requires std::same_as<tree_type, tagged_id<typename tree_type::tag_type>>; })
		{
			return id_set<1, typename tree_type::tag_type>{ tree.value };
		}
		else
		{
			static_assert(std::same_as<tree_type, void>, "invalid id-tree type.");
		}*/
	}

	

	namespace detail::id_tree_t_ns
	{
		template<typename T>
		struct id_tree_t;

		template<typename T>
		void tag_invoke();
	}

	template<typename T>
	inline constexpr detail::id_tree_t_ns::id_tree_t<T> id_tree{};

	template<typename T>
	struct detail::id_tree_t_ns::id_tree_t
	{
		constexpr auto operator()()const
		{
			if constexpr (requires{ tag_invoke<T>(id_tree<T>); })
			{
				return tag_invoke<T>(id_tree<T>);
			}
			else if constexpr (terminal<T>)
			{
				return tagged_id{ invalid_index };
			}
			else return[]<size_t...I>(std::index_sequence<I...>)
			{
				return tuple{ id_tree<child_type<T, I>>()... };
			}(std::make_index_sequence<child_count<T>>{});
		}
	};

	template<typename T>
	concept tagged = (not id_tree_to_set<id_tree<T>()>().untagged());

	template<typename T>
	concept taggedable = independent<T> && (not tagged<T>);

	//===============================================================================================================
	//===============================================================================================================

	//namespace detail::evaluate_t_ns
	//{
	//	template<id_set ReservedIds>
	//	struct evaluate_t;
	//
	//	template<id_set ReservedIds>
	//	void tag_invoke();
	//};
	//
	//template<id_set ReservedIds = id_set{} >
	//inline constexpr pipe_closure<detail::evaluate_t_ns::evaluate_t<ReservedIds>> evaluate{};
	//
	//template<id_set ReservedIds>
	//struct detail::evaluate_t_ns::evaluate_t
	//{
	//	using tag_t = pipe_closure<evaluate_t>;
	//	template<typename T>
	//	RUZHOUXIE_INLINE constexpr decltype(auto) operator()(T&& t)const
	//		//todo...noexcept
	//	{
	//		if constexpr (requires{ tag_invoke<ReservedIds>(tag_t{}, FWD(t)); })
	//		{
	//			return tag_invoke<ReservedIds>(tag_t{}, FWD(t));
	//		}
	//		else 
	//		{
	//			return FWD(t);
	//		}
	//	}
	//};
	//
	//namespace detail::evaluate_t_ns
	//{
	//	template<typename T, id_set Ids = empty_id_set>
	//	concept unevaluated = requires{ tag_invoke<Ids>(typename evaluate_t<Ids>::tag_t{}, declval<T>()); };
	//}
	//
	//using detail::evaluate_t_ns::unevaluated;
}
// //unreserve
// namespace ruzhouxie
// {
// 	namespace detail
// 	{
// 		template<typename T>
// 		struct unreserved_view;

// 		struct as_ref_t
// 		{
// 			template<typename T>
// 			RUZHOUXIE_INLINE constexpr auto&& operator()(T& t) const noexcept
// 			{
// 				return std::as_const(t);
// 			}

// 			template<typename T>
// 			RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const noexcept
// 			{
// 				return unreserved_view<T>{ t };
// 			}
// 		};
// 	}

// 	inline constexpr pipe_closure<detail::as_ref_t> as_ref{};

// 	template<typename T>
// 	struct detail::unreserved_view
// 	{
// 		T& ref;

// 	private:
// 		// actually, this view's behavior is similar as r-value refrence.
// 		// so its constructor should be protected to avoid misuse.
// 		friend as_ref_t;
// 		constexpr explicit unreserved_view(T& ref) noexcept : ref(ref) {}
// 	public:
// 		template<size_t I, id_set Ids, specified<unreserved_view> Self> requires (I < child_count<T>)
// 		RUZHOUXIE_INLINE friend constexpr decltype(auto) tag_invoke(tag_t<child<I, Ids>>, Self&& self)
// 			//todo...noexcept
// 		{
// 				constexpr auto dependency_ids = id_tree_to_set<id_tree_get<I>(id_tree<T>())>();
// 				constexpr bool independent = dependency_ids.independent_to(Ids);
// 				using result_type = decltype(move_if<independent>(self.ref) | child<I>);
// 				if constexpr (branched<result_type> && std::is_reference_v<result_type>)
// 				{
// 					return move_if<independent>(self.ref) | child<I> | as_ref;
// 				}
// 				else
// 				{
// 					return move_if<independent>(self.ref) | child<I>;
// 				}
// 		}
		
// 		template<specified<unreserved_view> Self>
// 		friend constexpr auto tag_invoke(tag_t<local_id_tree<Self>>)
// 		{
// 			return local_id_tree<T>();
// 		}

// 		template<specified<unreserved_view> Self>
// 		friend constexpr auto tag_invoke(tag_t<id_tree<Self>>)
// 		{
// 			return id_tree<T>();
// 		}
// 	};

// 	template<typename T>
// 	RUZHOUXIE_INLINE constexpr decltype(auto) reserve(T&& t) noexcept
// 	{
// 		return std::as_const(t);
// 	}

// 	template<typename T>
// 	RUZHOUXIE_INLINE constexpr decltype(auto) reserve(detail::unreserved_view<T> t) noexcept
// 	{
// 		return std::as_const(t.ref);
// 	}
// }

// //reserve
// namespace ruzhouxie
// {
// 	namespace detail
// 	{
// 		template<typename T, id_set ReservedIds>
// 		struct reserved_view;
// 	}

// 	template<typename T, id_set ReservedIds>
// 	struct detail::reserved_view
// 	{
// 		T value;

// 		template<size_t I, id_set Ids, specified<reserved_view> Self>
// 		friend constexpr decltype(auto) tag_invoke(tag_t<child<I, Ids>>, Self&& self)
// 			//todo...noexcept
// 		{
// 			constexpr auto ids = merge_id_set<ReservedIds, Ids>();
// 			if constexpr (I >= child_count<T>)
// 			{
// 				return;
// 			}
// 			else if constexpr(terminal<child_type<T, I>>) 
// 			{
// 				return FWD(self, value) | child<I, ids>;
// 			}
// 			else
// 			{
// 				return reserved_view<decltype(FWD(self, value) | child<I>), ids>{ FWD(self, value) | child<I> };
// 			}
// 		}

// 		template<specified<reserved_view> Self>
// 		friend constexpr auto tag_invoke(tag_t<local_id_tree<Self>>) noexcept
// 		{
// 			return local_id_tree<T>();
// 		}

// 		template<specified<reserved_view> Self>
// 		friend constexpr auto tag_invoke(tag_t<id_tree<Self>>) noexcept
// 		{
// 			return id_tree<T>();
// 		}
// 	};

// 	namespace detail
// 	{
// 		template<id_set ReservedIds>
// 		struct reserved_t
// 		{
// 			template<typename T>
// 			RUZHOUXIE_INLINE constexpr auto operator()(T&& t) const noexcept
// 			{
// 				return reserved_view<T, ReservedIds>{ FWD(t) };
// 			}
// 		};
// 	}

// 	template<id_set ReservedIds>
// 	inline constexpr pipe_closure<detail::reserved_t<ReservedIds>> reserved{};
// }

// //tagged
// namespace ruzhouxie
// {
// 	namespace detail
// 	{
// 		template<typename T, auto untagged_id_tree, typename Tag>
// 		struct tagged_view
// 		{
// 			using id_tree_type = purified<decltype(untagged_id_tree)>;

// 			T value;

// 			template<size_t I, id_set Ids, specified<tagged_view> Self>
// 			friend constexpr decltype(auto) tag_invoke(tag_t<child<I, Ids>>, Self&& self)
// 				//todo...noexcept
// 			{
				
// 				if constexpr (I >= child_count<T>)
// 				{
// 					return;
// 				}
// 				else
// 				{
// 					constexpr auto child_id_tree = id_tree_get<I>(untagged_id_tree);
// 					constexpr auto child_id_set = local_id_tree_to_set<child_id_tree, Tag>();
// 					if constexpr (terminal<child_type<T, I>>)
// 					{
// 						if constexpr (child_id_set.independent_to(Ids))
// 						{
// 							return FWD(self, value) | child<I, empty_id_set>;
// 						}
// 						else
// 						{
// 							return self.value | child<I>;
// 						}
// 					}
// 					else
// 					{
// 						return tagged_view<decltype(FWD(self, value) | child<I> | reserved<Ids>), child_id_tree, Tag>
// 						{
// 							FWD(self, value) | child<I> | reserved<Ids>
// 						};
// 					}
// 				}
// 			}

// 			template<specified<tagged_view> Self>
// 			friend constexpr auto tag_invoke(tag_t<ruzhouxie::id_tree<Self>>) noexcept
// 			{
// 				return tagged_local_id_tree<Tag>(untagged_id_tree);
// 			}
// 		};

// 		struct try_tagged_t
// 		{
// 			constexpr try_tagged_t operator()()const
// 			{
// 				return {};
// 			}

// 			template<typename Tag = decltype([] {}), typename T = void >
// 				requires taggedable<T>
// 			constexpr auto operator()(T&& t)const
// 			{
// 				return tagged_view<T, local_id_tree<T>(), Tag>{ FWD(t) };
// 			}

// 			template<typename T>
// 				requires (not taggedable<T>)
// 			constexpr T operator()(T&& t)const
// 			{
// 				return FWD(t);
// 			}

// 			template<typename Tag = decltype([] {}), typename T = void >
// 				requires taggedable<T>
// 			friend constexpr auto operator|(T&& t, try_tagged_t)
// 			{
// 				return tagged_view<T, local_id_tree<T>(), Tag>{ FWD(t) };
// 			}

// 			template<typename T>
// 				requires (not taggedable<T>)
// 			friend constexpr T operator|(T&& t, try_tagged_t)
// 			{
// 				return FWD(t);
// 			}
// 		};
// 	}

// 	template<typename Tag = decltype([] {}), taggedable T = void>
// 	constexpr auto make_tagged(T&& t) 
// 	{
// 		return detail::tagged_view<T, id_tree<T>(), Tag>{ FWD(t) };
// 	}
// 	inline constexpr detail::try_tagged_t try_tagged{};

// 	namespace detail::auto_tagged_pipe_closure_ns
// 	{
// 		template<typename Fn, size_t NArgsMin = 1uz> struct auto_tagged_pipe_closure;
// 		template<typename Fn> auto_tagged_pipe_closure(Fn) -> auto_tagged_pipe_closure<std::decay_t<Fn>>;
// 	}
// 	using detail::auto_tagged_pipe_closure_ns::auto_tagged_pipe_closure;

// 	template<typename Fn, size_t NArgsMin>
// 	struct detail::auto_tagged_pipe_closure_ns::auto_tagged_pipe_closure : Fn
// 	{
// 		using fn_type = Fn;
// 		static constexpr size_t n_args_min = NArgsMin;
// 		using Fn::operator();

// 		template<typename T, specified<auto_tagged_pipe_closure> Self> requires (not taggedable<T>)
// 		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self)
// 			AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(t)))

// 		template<taggedable T, specified<auto_tagged_pipe_closure> Self, typename Tag = decltype([] {})>
// 		RUZHOUXIE_INLINE friend constexpr auto operator|(T&& t, Self&& self)
// 			AS_EXPRESSION(as_base<Fn>(FWD(self))(make_tagged<Tag>(FWD(t))))

// 		template<typename Pipe, specified<auto_tagged_pipe_closure> Self>
// 		RUZHOUXIE_INLINE friend constexpr auto operator|(Pipe&& t, Self&& self)
// 			requires std::same_as<Pipe, auto_tagged_pipe_closure<typename Pipe::fn_type, Pipe::n_args_min>>
// 					|| std::same_as<Pipe, pipe_closure<typename Pipe::fn_type, Pipe::n_args_min>>
// 		{
// 			return auto_tagged_pipe_closure_ns::auto_tagged_pipe_closure
// 			{
// 				[&](auto&& arg) AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(arg) | FWD(t)))
// 			};
// 		}

// 		template<specified<auto_tagged_pipe_closure> Self, typename Pipe>
// 		RUZHOUXIE_INLINE friend constexpr auto operator|(Self&& self, Pipe&& t)
// 			requires std::same_as<Pipe, pipe_closure<typename Pipe::fn_type, Pipe::n_args_min>>
// 		{
// 			return auto_tagged_pipe_closure_ns::auto_tagged_pipe_closure
// 			{
// 				[&] (auto&& arg) AS_EXPRESSION(as_base<Fn>(FWD(t))(FWD(arg) | FWD(self)))
// 			};
// 		}

// 		RUZHOUXIE_INLINE constexpr decltype(auto) operator()(this auto&& self, auto&&...args)noexcept
// 			requires (not requires{ as_base<Fn>(FWD(self))(FWD(args)...); }) && (NArgsMin > sizeof...(args))
// 		{
// 			return auto_tagged_pipe_closure_ns::auto_tagged_pipe_closure
// 			{
// 				[&] (auto&&...appended_args)
// 					AS_EXPRESSION(as_base<Fn>(FWD(self))(FWD(args)..., FWD(appended_args)...))
// 				/*[&] (this auto&& new_fn, auto&&...appended_args)
// 					AS_EXPRESSION(as_base<Fn>(FWDLIKE(new_fn, self))(FWDLIKE(new_fn, args)..., FWD(appended_args)...))*/
// 			};
// 		}
// 	};
	
// 	template<typename Fn, size_t NArgsMin = 1uz>
// 	using adaptor_closure = auto_tagged_pipe_closure<Fn, NArgsMin>;
// 	//using adaptor_closure = pipe_closure<Fn, NArgsMin>;
// }





#include "macro_undef.h"
#endif