// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Assign_hpp
#define cf3_common_Assign_hpp

#include <coolfluid-config.hpp>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <deque>
#include <vector>
#include <utility>
#include <type_traits>

#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/decay.hpp>

namespace boost
{

namespace assign
{

template<typename T>
using uncvref = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename T>
using assign_decay = typename mpl::eval_if<
    ::boost::is_array<T>,
    ::boost::decay<const T>,
    ::boost::decay<T> >::type;

template<typename T>
struct generic_list
{
private:
    std::deque<T> values;

public:
    generic_list() = default;
    generic_list(generic_list &&) = default;
    generic_list(generic_list const &) = delete;
    generic_list &operator=(generic_list &&) = delete;
    generic_list &operator=(generic_list const &) = delete;

    template<typename U>
    generic_list & operator()(U && t)
    {
        values.push_back(static_cast<U &&>(t));
        return *this;
    }

    template<typename K, typename V>
    generic_list & operator()(K&& k, V&& v)
    {
        values.push_back(std::make_pair(static_cast<K&&>(k), static_cast<V&&>(v)));
        return *this;
    }

    template<typename Container, typename = decltype(Container(std::make_move_iterator(values.begin()), std::make_move_iterator(values.end())))>
    operator Container() noexcept(noexcept(Container(Container(std::make_move_iterator(values.begin()), std::make_move_iterator(values.end())))))
    {
        return Container(std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()));
    }
};

template<typename T>
inline generic_list<assign_decay<uncvref<T>>> list_of(T&& t)
{
    return std::move(generic_list<assign_decay<uncvref<T>>>()(static_cast<T &&>(t)));
}

template< class Key, class T >
inline generic_list< std::pair
    <
        assign_decay<uncvref<Key>>,
        assign_decay<uncvref<T>>
    > >
map_list_of(Key&& k, T&& t)
{
    return std::move(generic_list<std::pair<assign_decay<uncvref<Key> >, assign_decay<uncvref<T>>>>()( k, t ));
}

} // namespace assign

} // namespace boost

#endif // cf3_common_Assign_hpp
