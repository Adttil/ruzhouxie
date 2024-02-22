#ifndef RUZHOUXIE_MACRO_DEFINE_H
#define RUZHOUXIE_MACRO_DEFINE_H

#ifdef __clang__

#define RUZHOUXIE_MAYBE_EMPTY [[msvc::no_unique_address]]
#define RUZHOUXIE_INLINE [[clang::always_inline]]
#define RUZHOUXIE_INLINE_CALLS
#define RUZHOUXIE_INTRINSIC

#else

#ifdef _MSC_VER


#define RUZHOUXIE_MAYBE_EMPTY [[msvc::no_unique_address]]
#define RUZHOUXIE_INLINE [[msvc::forceinline]]
#define RUZHOUXIE_INLINE_CALLS [[msvc::forceinline_calls]]
#define RUZHOUXIE_INTRINSIC [[msvc::intrinsic]]

#else

#define RUZHOUXIE_MAYBE_EMPTY [[no_unique_address]]
#define RUZHOUXIE_INLINE
#define RUZHOUXIE_INLINE_CALLS
#define RUZHOUXIE_INTRINSIC

#endif

#endif

#define CONCAT(A, B) CONCAT_IMPL(A, B)
#define CONCAT_IMPL(A, B) A##B

#define GET_N(N, ...) CONCAT(GET_N_, N)(__VA_ARGS__)

#define GET_N_0(_0, ...) _0
#define GET_N_1(_0, _1, ...) _1
#define GET_N_2(_0, _1, _2, ...) _2
#define GET_N_3(_0, _1, _2, _3, ...) _3
#define GET_N_4(_0, _1, _2, _3, _4, ...) _4
#define GET_LENGTH(...) GET_N(4 __VA_OPT__(,) __VA_ARGS__, 4, 3, 2, 1, 0)

#define GET_LAST(_0, ...) GET_N(GET_LENGTH(__VA_ARGS__), _0 __VA_OPT__(,) __VA_ARGS__)

#define FOLD(fn, ...) CONCAT(FOLD_, GET_LENGTH(__VA_ARGS__))(fn __VA_OPT__(,) __VA_ARGS__)
#define FOLD_0(f)
#define FOLD_1(f, _0) _0
#define FOLD_2(f, _0, _1) f(_0, _1)
#define FOLD_3(f, _0, _1, _2) f(f(_0, _1), _2)
#define FOLD_4(f, _0, _1, _2, _3) f(f(f(_0, _1), _2), _3)

#define FOREACH(fn, ...) CONCAT(FOREACH_, GET_LENGTH(__VA_ARGS__))(fn __VA_OPT__(,) __VA_ARGS__)
#define FOREACH_0(f)
#define FOREACH_1(f, _0) f(_0)
#define FOREACH_2(f, _0, _1) f(_0), f(_1)
#define FOREACH_3(f, _0, _1, _2) f(_0), f(_1), f(_2)
#define FOREACH_4(f, _0, _1, _2, _3) f(_0), f(_1), f(_2), f(_3)

#define PREFIXES(fn, _0, ...) CONCAT(PREFIXES_, GET_LENGTH(_0 __VA_OPT__(,) __VA_ARGS__))(fn, _0 __VA_OPT__(,) __VA_ARGS__)
#define PREFIXES_0(f)
#define PREFIXES_1(f, _0) _0
#define PREFIXES_2(f, _0, _1) PREFIXES_1(f, _0), f(_0, _1)
#define PREFIXES_3(f, _0, _1, _2) PREFIXES_2(f, _0, _1), f(_0, _1, _2)
#define PREFIXES_4(f, _0, _1, _2, _3) PREFIXES_3(f, _0, _1, _2), f(_0, _1, _2, _3)

#define ACCESS(o, e) o.e
#define CONCAT_BY_DOT(...) FOLD(ACCESS, __VA_ARGS__)

#define FWD(_0, ...) FWD_IMPL(_0 __VA_OPT__(,) __VA_ARGS__)
#define FWD_IMPL(...) ::ruzhouxie::fwd<FOREACH(decltype, PREFIXES(CONCAT_BY_DOT, __VA_ARGS__))>(CONCAT_BY_DOT(__VA_ARGS__))

#define FWDLIKE(_0, ...) FWDLIKE_IMPL(_0 __VA_OPT__(,) __VA_ARGS__)
#define FWDLIKE_IMPL(...) ::ruzhouxie::fwd<FOREACH(decltype, __VA_ARGS__)>(GET_LAST(__VA_ARGS__))

#define AS_EXPRESSION(...) noexcept(noexcept(__VA_ARGS__))\
-> decltype(auto)\
requires requires{ __VA_ARGS__; }\
{\
	return __VA_ARGS__;\
}

#define AS_EXPRESSION2(...) noexcept(noexcept(__VA_ARGS__))\
requires requires{ __VA_ARGS__; }\
{\
	return __VA_ARGS__;\
}

//#define TRY_EVALUATE(Ids, ...) [&]()->decltype(auto)\
//{\
//	if constexpr (unevaluated<decltype(__VA_ARGS__), Ids>)\
//	{\
//		return __VA_ARGS__ | evaluate<Ids>;\
//	}\
//	else\
//	{\
//		return __VA_ARGS__;\
//	}\
//}()

#endif