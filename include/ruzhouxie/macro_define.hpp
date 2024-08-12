#ifndef RUZHOUXIE_MACRO_DEFINE_HPP
#define RUZHOUXIE_MACRO_DEFINE_HPP

#define CONCAT_IMPL(A, B) A##B
#define CONCAT(A, B) CONCAT_IMPL(A, B)

#define RUZHOUXIE(x) RUZHOUXIE_##x

#ifdef _MSC_VER

#define RUZHOUXIE_no_unique_address [[msvc::no_unique_address]]

#else

#define RUZHOUXIE_no_unique_address [[no_unique_address]]

#endif

#define GET_N(N, ...) CONCAT(GET_N_, N)(__VA_ARGS__)
#define GET_N_0(_0, ...) _0
#define GET_N_1(_0, _1, ...) _1
#define GET_N_2(_0, _1, _2, ...) _2
#define GET_N_3(_0, _1, _2, _3, ...) _3
#define GET_N_4(_0, _1, _2, _3, _4, ...) _4

#define GET_LENGTH(...) GET_N(4 __VA_OPT__(,) __VA_ARGS__, 4, 3, 2, 1, 0)

#define FWD(...) CONCAT(FWD_, GET_LENGTH(__VA_ARGS__))(__VA_ARGS__)
#define FWD_0()
#define FWD_1(_0) static_cast<decltype(_0)&&>(_0)
#define FWD_2(_0, _1) static_cast<::rzx::fwd_type<decltype((_0._1)), decltype(_0), decltype(_0._1)>>(_0._1)
#define FWD_3(_0, _1, _2) static_cast<::rzx::fwd_type<decltype((_0._1._2)), decltype(_0), decltype(_0._1), decltype(_0._1._2)>>(_0._1._2)
#define FWD_4(_0, _1, _2, _3) static_cast<::rzx::fwd_type<decltype((_0._1._2._3)), decltype(_0), decltype(_0._1), decltype(_0._1._2), decltype(_0._1._2._3)>>(_0._1._2._3)

#define FWDLIKE(...) CONCAT(FWDLIKE_, GET_LENGTH(__VA_ARGS__))(__VA_ARGS__)
#define FWDLIKE_0()
#define FWDLIKE_1(_0) static_cast<decltype(_0)&&>(_0)
#define FWDLIKE_2(_0, _1) static_cast<::rzx::fwd_type<decltype((_1)), decltype(_0), decltype(_1)>>(_1)
#define FWDLIKE_3(_0, _1, _2) static_cast<::rzx::fwd_type<decltype((_2)), decltype(_0), decltype(_1), decltype(_2)>(_2)
#define FWDLIKE_4(_0, _1, _2, _3) static_cast<::rzx::fwd_type<decltype((_3)), decltype(_0), decltype(_1), decltype(_2), decltype(_3)>>(_3)

#define AS_EXPRESSION(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(auto)\
    requires requires{ __VA_ARGS__; }\
{\
    return __VA_ARGS__;\
}

#endif