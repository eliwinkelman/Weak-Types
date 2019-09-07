//
// Created by Eli Winkelman on 8/31/19.
//

#ifndef WEAK_TYPES_HAS_OPERATOR_H
#define WEAK_TYPES_HAS_OPERATOR_H

#include <type_traits>

template <class T, class U, class>
struct has_equal_impl : std::false_type {};

template <class T, class U> struct has_equal_impl<T, U, decltype(std::declval<T>() == std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_equal : has_equal_impl<T, U, void> {};


template <class T, class U, class>
struct has_addition_impl : std::false_type {};

template <class T, class U> struct has_addition_impl<T, U, decltype(std::declval<T>() + std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_addition : has_addition_impl<T, U, void> {};


#endif //WEAK_TYPES_HAS_OPERATOR_H
