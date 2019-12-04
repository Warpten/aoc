#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <tuple>

// C: coordinate
// S: step counter (used in phase 2)

template <int64_t X, int64_t Y>
struct coord {
    constexpr static int64_t const x = X;
    constexpr static int64_t const y = Y;

    constexpr static int64_t const manhattan = std::abs(x) + std::abs(y);
};

// Base wire
template <typename... /* coords */> struct wire { };

using start_t = wire<coord<0, 0>>; // origin

// Wire node
template <typename C>
struct wire<C> {
    // Steps it took to get here
    constexpr const static size_t step_counter = 1;
    constexpr const static size_t manhattan = C::manhattan;

    using head_coord = C;
    using coords_t = std::tuple<C>;

    // Do we go over given coordinates
    template <int64_t X, int64_t Y>
    constexpr const static bool has_coord_v = X == C::x && Y == C::y;

    // Rebind with an extra node
    template <typename N>
    using attach = wire<N, C>;
};

// Recursive wire impl
template <typename C, typename... Cs>
struct wire<C, Cs...> {
    constexpr const static size_t step_counter = sizeof...(Cs) + 1;
    constexpr const static size_t manhattan = C::manhattan;

    using head_coord = C;
    using coords_t = std::tuple<C, Cs...>;

    template <size_t I>
    using coord_at_step = typename std::tuple_element<sizeof...(Cs) - I, coords_t>::type;

    template <int64_t X, int64_t Y>
    struct has_coord {
        constexpr const static bool value = wire<C>::template has_coord_v<X, Y>
            || wire<Cs...>::template has_coord_v<X, Y>;
    };

    template <int64_t X, int64_t Y>
    constexpr const static bool has_coord_v = has_coord<X, Y>::value;

    template <typename N>
    using attach = wire<N, C, Cs...>;
};

template <int64_t XD /* x displacement */, int64_t YD /* y displacement */>
struct wire_weaver {
    template <size_t C /* amount of iterations */>
    struct weaver_impl {
        // recurse
        template <typename W>
        using woven_t = typename weaver_impl<C - 1>::template woven_t<
            typename W::template attach<
                coord<
                    XD + W::head_coord::x,
                    YD + W::head_coord::y
                >
            >
        >;
    };

    template <> // recursion tail
    struct weaver_impl<0> {
        template <typename W>
        using woven_t = W;
    };
};

namespace parser {
    // Parser token
    template <char... F> struct token { };

    template <> struct token<> {
        struct next_t_impl {
            template <typename W>
            using woven_t = W;
        };

        template <typename W>
        using woven_t = W;

        using next_t = next_t_impl;
    };

    // Last token from string, has to be int
    template <char C>
    struct token<C> {
        constexpr const static size_t value = C - '0';
    };

    template <char... Cs> struct token_int_part { };

    template <char H, char... F>
    struct token_int_part<H, F...> {
        constexpr const static size_t value = (H - '0') * 10 + token_int_part<F...>::value;

        using next_t = typename token_int_part<F...>::next_t;
    };

    template <char H, char... F>
    struct token_int_part<H, ',', F...> {
        constexpr const static size_t value = H - '0';

        using next_t = token<F...>;
    };

    template <char H>
    struct token_int_part<H> {
        constexpr const static size_t value = H - '0';
    };

#define MAKE_TOKEN(LIMITER, WEAVER_X, WEAVER_Y)                                         \
    template <char... Cs>                                                               \
    struct token<LIMITER, Cs...> {                                                      \
        constexpr const static size_t value = token_int_part<Cs...>::value;             \
                                                                                        \
        using next_t = typename token_int_part<Cs...>::next_t;                          \
                                                                                        \
        using weaver_t = wire_weaver<WEAVER_X, WEAVER_Y>;                               \
                                                                                        \
        template <typename W>                                                           \
        using woven_t = typename next_t::template woven_t<                              \
            typename weaver_t::weaver_impl<value>::template woven_t<W>                  \
        >;                                                                              \
    };

    MAKE_TOKEN('R', 1, 0)
    MAKE_TOKEN('L', -1, 0)
    MAKE_TOKEN('D', 0, -1)
    MAKE_TOKEN('U', 0, 1)
#undef MAKE_TOKEN
}

namespace tokenizer {
    constexpr unsigned c_strlen( char const* str, unsigned count = 0 ) { 
        return ('\0' == str[0]) ? count : c_strlen(str+1, count+1);
    }

    template <typename T, unsigned L, char... Cs>
    struct explode_impl {
        using result =
            typename explode_impl<T, L - 1, T::str()[L - 1], Cs...>::result;
    };

    template < typename T, char... Cs>
    struct explode_impl<T, 0, Cs... > {
         using result = parser::token<Cs...>;
    };

    template <typename T>
    using explode =
        typename explode_impl<T, c_strlen(T::str()), ','>::result;
}

struct wire_a_provider {
    constexpr static char const* str() {
        return "R75,D30,R83,U83,L12,D49,R71,U7,L72";
    }
};

struct wire_b_provider {
    constexpr static char const* str() {
        return "U62,R66,U55,R34,D71,R55,D58,R83";
    }
};

using wire_a = tokenizer::explode<wire_a_provider>::woven_t<start_t>;
using wire_b = tokenizer::explode<wire_b_provider>::woven_t<start_t>;

// Determines if wire W is colliding with wire O when at the coordinates pointed to by
// S on wire W
template <size_t S /* steps walked */, typename W /* wire */>
struct collision_tester {
    using wire_t = W;
    constexpr const static size_t step_index = S;

    template <typename O>
    struct collides {
        constexpr static const bool value = O::template has_coord<
            wire_t::template coord_at_step<S>::x,
            wire_t::template coord_at_step<S>::y
        >::value;
    };

    template <typename O>
    constexpr static const bool collides_v = collides<O>::value;
};

template <
    typename L /* prime wire */, 
    typename R /* secondary wire */
>
struct collision_finder {
    // Evaluates if prime and secondary collide at step I on prime
    template <size_t I>
    struct collides {
        constexpr const static bool value = 
            collision_tester<I, L>::template collides_v<R>;
    };

    template <size_t I>
    constexpr const static bool collides_v = collides<I>::value;

    template <bool, size_t CO> struct is_collision { };

    template <size_t CO>
    using is_collision_t = is_collision<collides_v<CO>, CO>;

    template <size_t CO>
    struct is_collision<true, CO> {
        constexpr static const size_t offset = CO;
        constexpr static const int64_t x = L::template coord_at_step<CO>::x;
        constexpr static const int64_t y = L::template coord_at_step<CO>::y;

        // This is still bugged
        using next = typename std::conditional_t<
            (CO < std::tuple_size_v<typename L::coords_t> - 1),
            typename collision_finder<L, R>::template is_collision_t<CO + 1>,
            std::nullptr_t
        >;
    };

    template <size_t CO>
    struct is_collision<false, CO> : public is_collision_t<CO + 1> {
        using next = typename is_collision_t<CO + 1>::next;
    };

    using collision = is_collision_t<1>;
};

constexpr const bool are_sharing_origin = collision_tester<0, wire_a>::collides_v<wire_b>;

using collision_finder_ab = collision_finder<wire_a, wire_b>;

int foo() {
    return collision_finder_ab::collision::offset;
}
