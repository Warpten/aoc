#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <tuple>
#include <chrono>
#include <iostream>

using size_t = std::size_t;

// Fwd-decl lock
template <size_t... Ls> struct seq;

// Rules
namespace rules {
    namespace _1 { // Part 1
        namespace details {
            template <typename> struct is_increasing_seq { };

            // Tests if F <= S and if S <= Ts..., recurses
            template <size_t F, size_t S, size_t... Ls>
            struct is_increasing_seq<seq<F, S, Ls...>> {
                constexpr const static bool value =
                    is_increasing_seq<seq<F, S>>::value
                        && is_increasing_seq<seq<S, Ls...>>::value;
            };

            // Tests if F <= S, recursion breaker
            template <size_t F, size_t S>
            struct is_increasing_seq<seq<F, S>> {
                constexpr const static bool value = F <= S;
            };

            // ----

            template <typename> struct has_same_seq { };

            // Tests if F == S or recurses down one level (S == top(Ls...))
            template <size_t F, size_t S, size_t... Ls>
            struct has_same_seq<seq<F, S, Ls...>> {
                constexpr const static bool value =
                    has_same_seq<seq<F, S>>::value
                        || has_same_seq<seq<S, Ls...>>::value;
            };

            // Tests if F == S, recursion breaker
            template <size_t F, size_t S>
            struct has_same_seq<seq<F, S>> {
                constexpr const static bool value = F == S;
            };
        }

        // Tests if a lock is valid.
        template <typename Lock>
        struct is_valid_lock {
            constexpr const static bool value =
                details::is_increasing_seq<Lock>::value
                    && details::has_same_seq<Lock>::value;
        };

        template <typename Lock>
        constexpr const static bool is_valid_lock_v = is_valid_lock<Lock>::value;
    }

    namespace _2 { // Part 2

    }
}

// Implementation of a lock
template <size_t... Ls>
struct seq {
    constexpr const static size_t component_count = sizeof...(Ls);

    constexpr const static bool is_valid_1 = rules::_1::is_valid_lock<seq<Ls...>>::value;
};

namespace helpers {
    namespace _std {
        template <size_t ... Is>
        constexpr auto reverse_index_sequence(::std::index_sequence<Is...> const &)
            -> decltype(::std::index_sequence<sizeof...(Is) - 1U - Is...>{});
        
        template <size_t N>
        using make_reverse_index_sequence
            = decltype(reverse_index_sequence(::std::make_index_sequence<N>{}));

        template <class T, T... Ints>
        constexpr T nth(std::integer_sequence<T, Ints...>, size_t i) {
            constexpr T arr[] = {Ints...};
            return arr[i];
        }
    }
}

template <
    template <typename> class RuleChecker // is_valid_lock from rules::_1 or rules::_2
>
struct generator {
    // Extract value from lock
    template <size_t... Ix>
    struct get_value {
        template <size_t I, typename> struct value_t;

        template <size_t Itr, size_t... Is>
        struct value_t<Itr, std::index_sequence<Is...>> {
            constexpr const static size_t value =
                helpers::_std::nth(std::index_sequence<Is...>{}, Itr)
                    + value_t<Itr - 1, std::index_sequence<Is...>>::value * 10;
        };

        template <size_t... Is>
        struct value_t<0, std::index_sequence<Is...>> {
            constexpr const static size_t value =
                helpers::_std::nth(std::index_sequence<Is...>{}, 0);
        };

        constexpr const static size_t value = value_t<
            sizeof...(Ix) - 1,
            std::index_sequence<Ix...>
        >::value;
    };

    template <size_t... Ix>
    constexpr const static size_t value_v = get_value<Ix...>::value;

    template <typename> struct value_t;

    template <template <size_t...> class Seq, size_t... Is>
    struct value_t<Seq<Is...>> {
        constexpr const static size_t value = value_v<Is...>;
    };

    template <typename Seq>
    constexpr const static size_t value_t_v = value_t<Seq>::value;

    // Construct lock from value
    template <size_t V>
    struct make_lock {
        constexpr static size_t count_digits(size_t value, size_t prev) {
            return value < 10 ? prev : count_digits(value / 10, prev + 1);
        }

        constexpr const static size_t digit_count = count_digits(V, 0) + 1;

        constexpr static size_t nth_digit(size_t value, size_t idx) {
            return idx == digit_count - 1
                ? (value - (value / 10) * 10)
                : nth_digit(value / 10, idx + 1);
        }

        template <size_t... Ix>
        constexpr static auto make_lock_fn(std::index_sequence<Ix...>) -> seq<nth_digit(V, Ix)...>;

        using lock = decltype(make_lock_fn(std::make_index_sequence<digit_count>{}));
    };

    // Increment lock
    template <typename Seq>
    using incr_t = typename make_lock<value_t_v<Seq> + 1>::lock;

    // obtain next valid lock from any lock
    template <bool, typename> struct next_valid_lock;

    template <typename Seq>
    struct next_valid_lock<true, Seq> {
        using lock = Seq;
    };

    template <typename Seq>
    struct next_valid_lock<false, Seq> {
        using lock = typename next_valid_lock<
            RuleChecker<incr_t<Seq>>::value, incr_t<Seq>
        >::lock;
    };

    template <typename Seq>
    using next_valid_lock_t = typename next_valid_lock<RuleChecker<Seq>::value, Seq>::lock;
};

// Validate rules for part 1
static_assert(seq<1, 2, 2, 3, 5, 6>::is_valid_1, "Lock should be valid");
static_assert(!seq<1, 0, 2, 3, 4, 5>::is_valid_1, "Lock should be invalid");
static_assert(!seq<1, 2, 3, 4, 5, 6>::is_valid_1, "Lock should be invalid");

using generator_rules_1 = generator<rules::_1::is_valid_lock>;

using start_seq = seq<3, 4, 7, 3, 1, 2>;
using incr_lock = generator_rules_1::next_valid_lock_t<start_seq>;

static_assert(std::is_same_v<incr_lock, seq<3, 4, 7, 7, 7, 7>>, "invalid lock generated");
