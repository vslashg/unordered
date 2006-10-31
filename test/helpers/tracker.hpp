
// Copyright 2006 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This header contains metafunctions/functions to get the equivalent
// associative container for an unordered container, and compare the contents.

#if !defined(BOOST_UNORDERED_TEST_HELPERS_TRACKER_HEADER)
#define BOOST_UNORDERED_TEST_HELPERS_TRACKER_HEADER

#include <set>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>
#include "../objects/fwd.hpp"
#include "./metafunctions.hpp"
#include "./helpers.hpp"
#include "./equivalent.hpp"

namespace test
{
    template <class X>
    struct equals_to_compare2
        : public boost::mpl::identity<std::less<typename X::first_argument_type> >
    {
    };

    template <class X>
    struct equals_to_compare
        : public boost::mpl::eval_if<
            boost::is_same<X, test::equal_to>,
            boost::mpl::identity<test::less>,
            equals_to_compare2<X>
        >
    {
    };

    template <class X1, class X2>
    void compare_range(X1 const& x1, X2 const& x2)
    {
        typedef typename non_const_value_type<X1>::type value_type;
        std::vector<value_type> values1, values2;
        values1.reserve(x1.size());
        values2.reserve(x2.size());
        std::copy(x1.begin(), x1.end(), std::back_inserter(values1));
        std::copy(x2.begin(), x2.end(), std::back_inserter(values2));
        std::sort(values1.begin(), values1.end());
        std::sort(values2.begin(), values2.end());
        BOOST_TEST(values1.size() == values2.size() &&
                std::equal(values1.begin(), values1.end(), values2.begin(), test::equivalent));
    }

    template <class X1, class X2, class T>
    void compare_pairs(X1 const& x1, X2 const& x2, T*)
    {
        std::vector<T> values1, values2;
        values1.reserve(std::distance(x1.first, x1.second));
        values2.reserve(std::distance(x2.first, x2.second));
        std::copy(x1.first, x1.second, std::back_inserter(values1));
        std::copy(x2.first, x2.second, std::back_inserter(values2));
        std::sort(values1.begin(), values1.end());
        std::sort(values2.begin(), values2.end());
        BOOST_TEST(values1.size() == values2.size() &&
                std::equal(values1.begin(), values1.end(), values2.begin(), test::equivalent));
    }

    template <class X>
    struct ordered_set
        : public boost::mpl::if_<
            test::has_unique_keys<X>,
            std::set<typename X::value_type,
                typename equals_to_compare<typename X::key_equal>::type>,
            std::multiset<typename X::value_type,
                typename equals_to_compare<typename X::key_equal>::type>
            > {};

    template <class X>
    struct ordered_map
        : public boost::mpl::if_<
            test::has_unique_keys<X>,
            std::map<typename X::key_type, typename X::mapped_type,
                typename equals_to_compare<typename X::key_equal>::type>,
            std::multimap<typename X::key_type, typename X::mapped_type,
                typename equals_to_compare<typename X::key_equal>::type>
            > {};

    template <class X>
    struct ordered_base
        : public boost::mpl::eval_if<
            test::is_set<X>,
            test::ordered_set<X>,
            test::ordered_map<X> >
    {
    };

    template <class X>
    class ordered : public ordered_base<X>::type
    {
        typedef typename ordered_base<X>::type base;
    public:
        typedef typename base::key_compare key_compare;

        ordered()
            : base()
        {}

        explicit ordered(key_compare const& compare)
            : base(compare)
        {}

        void compare(X const& x)
        {
            compare_range(x, *this);
        }

        void compare_key(X const& x, typename X::value_type const& val)
        {
            compare_pairs(
                x.equal_range(get_key<X>(val)),
                this->equal_range(get_key<X>(val)),
                (typename non_const_value_type<X>::type*) 0
                );
        }

        template <class It>
        void insert_range(It begin, It end) {
            while(begin != end) {
                this->insert(*begin);
                ++begin;
            }
        }
    };

    template <class Equals>
    typename equals_to_compare<Equals>::type create_compare(Equals const&)
    {
        typename equals_to_compare<Equals>::type x;
        return x;
    }

    template <class X>
    ordered<X> create_ordered(X const& container)
    {
        return ordered<X>(create_compare(container.key_eq()));
    }

    template <class X1, class X2>
    void check_container(X1 const& container, X2 const& values)
    {
        ordered<X1> tracker = create_ordered(container);
        tracker.insert_range(values.begin(), values.end());
        tracker.compare(container);
    }
}

#endif

