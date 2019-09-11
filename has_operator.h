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
struct has_not_equal_impl : std::false_type {};

template <class T, class U> struct has_not_equal_impl<T, U, decltype(std::declval<T>() != std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_not_equal : has_not_equal_impl<T, U, void> {};

template <class T, class U, class>
struct has_addition_impl : std::false_type {};

template <class T, class U> struct has_addition_impl<T, U, decltype(std::declval<T>() + std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_addition : has_addition_impl<T, U, void> {};

template <class T, class U, class>
struct has_subtraction_impl : std::false_type {};

template <class T, class U> struct has_subtraction_impl<T, U, decltype(std::declval<T>() - std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_subtraction : has_subtraction_impl<T, U, void> {};

template <class T, class U, class>
struct has_multiplication_impl : std::false_type {};

template <class T, class U> struct has_multiplication_impl<T, U, decltype(std::declval<T>() * std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_multiplication : has_multiplication_impl<T, U, void> {};

template <class T, class U, class>
struct has_division_impl : std::false_type {};

template <class T, class U> struct has_division_impl<T, U, decltype(std::declval<T>() / std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_division : has_division_impl<T, U, void> {};

template <class T, class U, class>
struct has_less_than_impl : std::false_type {};

template <class T, class U> struct has_less_than_impl<T, U, decltype(std::declval<T>() < std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_less_than : has_less_than_impl<T, U, void> {};

template <class T, class U, class>
struct has_greater_than_impl : std::false_type {};

template <class T, class U> struct has_greater_than_impl<T, U, decltype(std::declval<T>() > std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_greater_than : has_greater_than_impl<T, U, void> {};

template <class T, class U, class>
struct has_greater_than_equal_to_impl : std::false_type {};

template <class T, class U> struct has_greater_than_equal_to_impl<T, U, decltype(std::declval<T>() >= std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_greater_than_equal_to : has_greater_than_equal_to_impl<T, U, void> {};

template <class T, class U, class>
struct has_less_than_equal_to_impl : std::false_type {};

template <class T, class U> struct has_less_than_equal_to_impl<T, U, decltype(std::declval<T>() <= std::declval<U>(), void())> : std::true_type {};

template <class T, class U> struct has_less_than_equal_to : has_less_than_equal_to_impl<T, U, void> {};


#endif //WEAK_TYPES_HAS_OPERATOR_H
