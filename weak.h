//
// Created by Eli Winkelman on 8/20/19.
//

#ifndef WEAK_H
#define WEAK_H

#include "stdlib.h"
#include "type_traits"
#include <utility>
#include "strong_typedef.h"
#include "has_operator.h"


template <class T>
class simple_optional {

    // no assignments
    simple_optional& operator=(const simple_optional&) = delete;

    T* ptr;

public:

    simple_optional() : ptr(nullptr){


    }

    simple_optional(const T& val) : ptr(new T(val)){}

    simple_optional(const T&& val) : ptr(new T(val)){}

    simple_optional(const simple_optional& other) {
        simple_optional(std::move(other));
    }

    simple_optional(const simple_optional&& other) {
        if (other) {
            ptr = new T(other.value());
        }
        else {
            ptr = nullptr;
        }
    }

    ~simple_optional() {
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
    }

    void emplace(const T&& val) {

        if (ptr != nullptr) {
            delete ptr;
        }

        ptr = new T(val);
    }

    void emplace(const T& val) {
        emplace(std::move(val));
    }

    constexpr T& value_or(T&& orVal) const & {
        return ptr != nullptr ? *ptr : orVal;
    }

    constexpr T& value_or(T& orVal) const & {
        return ptr != nullptr ? *ptr : orVal;
    }

    constexpr T& value() const &{
        return *ptr;
    }

    constexpr operator bool() const {
        return ptr != nullptr;
    }
};

//=== get_type_index ==//
template <typename T, typename... Ts>
struct get_type_index_impl;

// base case
// type not found
template <typename T>
struct get_type_index_impl<T> : std::integral_constant<std::size_t, 0>
{};

// second case
// type is at the beginning
template <typename T, typename ... Tail>
struct get_type_index_impl<T, T, Tail...> : std::integral_constant<std::size_t, 1>
{};

// induction step
// type not found at the beginning
// the idea here is that if the type is at the beginning ^ will get called (No Head)
template <typename T, typename Head, typename... Tail>
struct get_type_index_impl<T, Head, Tail...> : std::integral_constant<std::size_t,
        get_type_index_impl<T, Tail...>::value == 0u ?
        0u :
        1 + get_type_index_impl<T, Tail...>::value>
{};


template <std::size_t N, typename ... Ts>
struct get_type_from_index;

template <std::size_t N>
struct get_type_from_index<N> {
    using type = void;
};

template <typename T, typename ... Ts>
struct get_type_from_index<0, T, Ts...> {
    using type = T;
};

template <std::size_t N, typename T, typename ... Ts>
struct get_type_from_index<N, T, Ts...> {
    using type = typename get_type_from_index<N-1, Ts...>::type;
};

// Wrapper struct to allow running functions on a specific Var for it's underlying type
// Function is a function, Var is a weak class, Types is a weak_types object, and args is a list of argument types to call the function with
// a struct to store a list of weak types

template <typename ... Types>
struct weak_types { };

template <typename T>
struct weak_type {
    // initialize
    constexpr weak_type(){}
};

template <typename ... Types>
class weak;

template <typename ... Types>
class weak {

    class type_id : public strong_typedef<type_id, std::size_t>, comparison<type_id> {

    public:

        // make sure that the type is valid
        template <typename T>
        static constexpr bool valid(weak_type<T>) {
            return get_type_index_impl<T, Types...>::value != 0u;
        }

        //empty type_id
        constexpr type_id() noexcept : strong_typedef<type_id, std::size_t>(0u){};

        //type_id initializer
        template <typename T>
        constexpr type_id(weak_type<T>) noexcept : type_id(get_type_index_impl<T, Types...>::value){

        };

        //initializer with a value for the other initializers to use
        explicit constexpr type_id(std::size_t value) : strong_typedef<type_id, std::size_t>(value)
        {};

    };

    type_id current_type;

    void* storage;

    template < template<typename Type, typename ... Ts> class Functor, typename ... Ts>
    class using_weak {

        template <typename T, typename ... Args>
        static auto call(weak<Types...>&& ptr, Args&&... args) -> decltype(Functor<T, Ts...>()(std::forward<weak<Types...>>(ptr).template value<T>(), std::forward<Args>(args)...)) {
            return Functor<T, Ts...>()(std::forward<weak<Types...>>(ptr).template value<T>(), std::forward<Args>(args)...);
        };

        template <typename T, typename ... Args>
        static auto constCall(const weak<Types...>&& ptr, Args&&... args) -> decltype(Functor<T, Ts...>()(std::forward<const weak<Types...>>(ptr).template value<T>(), std::forward<Args>(args)...)) {
            return Functor<T, Ts...>()(std::forward<const weak<Types...>>(ptr).template value<T>(), std::forward<Args>(args)...);
        };

    public:
        // out of types, do nothing
        template <typename ... Args>
        static void with(weak_types<>, weak<Types...>&&, Args&&...) {};

        template <typename ... Args>
        static void with(weak_types<>, const weak<Types...>&&, Args&&...) {};

        template <typename Head, typename ... Tail, typename ... Args>
        static void with(weak_types<Head, Tail...>, weak<Types...>&& ptr, Args&&... args){
            if (ptr.current_type == type_id(weak_type<Head>{})) {
                //the type is Head, execute function with value as head
                call<Head>(std::forward<weak<Types...>>(ptr), std::forward<Args>(args)...);
            }
            else {
                // keep going down the type list.
                with(weak_types<Tail...>{}, std::forward<weak<Types...>>(ptr), std::forward<Args>(args)...);
            }
        };

        template <typename Head, typename ... Tail, typename ... Args>
        static void with(weak_types<Head, Tail...>, const weak<Types...>&& ptr, Args&&... args){
            if (ptr.current_type == type_id(weak_type<Head>{})) {
                //the type is Head, execute function with value as head
                constCall<Head>(std::forward<const weak<Types...>>(ptr), std::forward<Args>(args)...);
            }
            else {
                // keep going down the type list.
                with(weak_types<Tail...>{}, std::forward<const weak<Types...>>(ptr), std::forward<Args>(args)...);
            }
        };
    };

public:

    weak(){
        // initialize with invalid type
        current_type = type_id();
        storage = nullptr;
    };

    //// Constructor, Deconstructor, Copy, Move ////
    template <typename T>
    weak(T val) : weak() {

        storage = nullptr;

        emplace(val);
    }

    /// Destructor
    ~weak(){
        // run functor on object
        if (storage != nullptr) {
            run<destroy>(&storage);
        }
    }

    /// Copy Constructor
    weak(const weak<Types...>& ptr) {
        // run the copier
        storage = nullptr;
        ptr.template run<copy>(this);
    }

    weak(const weak<Types...>&& val) {
        val.template run<copy>(this);
    }

    weak<Types...>& operator=(const weak<Types...>&& ptr) {

        // assign with underlying value
        // check for self assignment
        if (this == &ptr) {
            return *this;
        }

        ptr.template run<copy>(this);

        return *this;

    }

    weak<Types...>& operator=(const weak<Types...>& ptr) {

        // assign with underlying value
        // check for self assignment

        if (this == &ptr) {
            return *this;
        }

        ptr.template run<copy>(this);

        return *this;
    }

    template <typename T>
    weak<Types...>& operator=(const T&& val){

        // otherwise assign normally
        emplace(val);

        return *this;
    }

    template <typename T>
    weak<Types...>& operator=(const T& ptr) {

        emplace(ptr);

        return *this;
    }

    template <typename T>
    void emplace(const T val) {
        using t = typename std::decay<T>::type;
        /// ensure that the type is valid at compile time
        static_assert(type_id::valid(weak_type<t>{}), "Cannot store with non-weak type.");

        current_type = type_id(weak_type<t>());

        if (storage != nullptr) {
            run<destroy>(&storage);
        }

        storage = new t(val);
    }

    const type_id& type() const noexcept
    {
        return current_type;
    }

    bool isValid() {
        return !(current_type == type_id());
    }

    template <typename Type>
    bool isType() {
        return current_type == type_id(weak_type<Type>{});
    }

    template <template<typename Type, typename ... Ts> class Functor, typename ... Ts, typename ... Args>
    void run(Args&&... args) {
        using_weak<Functor, Ts...>::with(weak_types<Types...>{}, std::move(*this), std::forward<Args>(args)...);
    };

    template <template<typename Type, typename ... Ts> class Functor, typename ... Ts, typename ... Args>
    void run(Args&&... args) const {
        using_weak<Functor, Ts...>::with(weak_types<Types...>{}, std::move(*this), std::forward<Args>(args)...);
    };

    template <typename T>
    simple_optional<T> retrieve() const {
        if (check(weak_type<T>{})) {
            return simple_optional<T>(value<T>());
        }
        else return simple_optional<T>();
    };


    template <typename T>
    simple_optional<T> as() const {

        simple_optional<T> castedMaybe = simple_optional<T>();
        run<cast, T>(castedMaybe);

        return castedMaybe;
    }

    template <typename T>
    T&& value() const {
        return std::move(*reinterpret_cast<T*>(storage));
    };

private:

    ///// returns the underlying pointer if the type is correct. Otherwise returns a nullptr;
    template <typename Type>
    bool check(weak_type<Type> check_type) const {
        return current_type == check_type;
    }

    //// Functors to destroy, copy, move, assign without knowing the underlying value
    template <typename T>
    struct destroy {
        void operator() (T val, void** storage) {
            delete (T*)*storage;
            *storage = nullptr;
        }
    };

    template <typename T>
    struct copy {
        void operator() (T val, weak<Types...>* thisWeak) const {
            // copy the underlying value
            if (thisWeak -> storage == nullptr) {
                thisWeak -> emplace(val);
            }


        }
    };

    template <typename T, typename V, typename enable=void>
    struct cast;

    template <typename T, typename V>
    struct cast<T, V, typename std::enable_if<std::is_convertible<T, V>::value>::type> {
        void operator() (T val, simple_optional<V>& returnVal) {
                returnVal.emplace(V(val));
        };
    };

    template <typename T, typename V>
    struct cast<T, V, typename std::enable_if<!std::is_convertible<T, V>::value>::type> {
        void operator() (T val, simple_optional<V>& returnVal) {

        };
    };

public:
    ///// Arithmetic Operators /////

    /// Addition
    /// Weak types

    /// rvalue
    weak<Types...> operator+ (const weak<Types...>&& other) const {

        weak<Types...> added;

        run<addTop>(&other, &added);

        return added;
    }

    /// lvalue
    weak<Types...> operator+ (const weak<Types...>& other) const {


        weak<Types...> added;

        run<addTop>(&other, &added);

        return added;
    }

    /// Subtraction
    /// Weak Types
    weak<Types...> operator- (const weak<Types...>& other) const {
        weak<Types...> subtracted;

        other.run<subtractTop>(this, &subtracted);

        return subtracted;
    }

    /// Subtraction
    /// Weak Types
    weak<Types...> operator- (const weak<Types...>&& other) const {
        weak<Types...> subtracted;

        other.run<subtractTop>(this, &subtracted);

        return subtracted;
    }

    /// Multiplication
    /// Weak Types

    weak<Types...> operator* (const weak<Types...>& other) const {

        weak<Types...> multiplied;

        run<multTop>(&other, &multiplied);

        return multiplied;

    }

    weak<Types...> operator* (const weak<Types...>&& other) const {

        weak<Types...> multiplied;

        run<multTop>(&other, &multiplied);

        return multiplied;
    }

    /// Division
    /// Weak Types
    weak<Types...> operator / (const weak<Types...>& other) const {
        weak<Types...> divided;
        other.run<divideTop>(this, &divided);

        return divided;
    }

    weak<Types...> operator / (const weak<Types...>&& other) const {
        weak<Types...> divided;
        other.run<divideTop>(this, &divided);

        return divided;
    }

    // comparison operators
    bool operator==(const weak<Types...>& other) const {
        bool result = false;

        run<equalTop>(&other, &result);

        return result;
    }

    bool operator !=(const weak<Types...>& other) const {
        bool result = false;

        run<notEqualTop>(&other, &result);

        return result;
    }

    bool operator < (const weak<Types...>& other) const {
        bool result = false;

        other.run<lessTop>(this, &result);

        return result;
    }

    bool operator < (const weak<Types...>&& other) const {
        bool result = false;

        other.run<lessTop>(this, &result);

        return result;
    }

    bool operator > (const weak<Types...>& other) const {
        bool result = false;

        other.run<greaterTop>(this, &result);

        return result;
    }

    bool operator > (const weak<Types...>&& other) const {
        bool result = false;

        other.run<greaterTop>(this, &result);

        return result;
    }

    bool operator <= (const weak<Types...>& other) const {
        bool result = false;

        other.run<lessThanEqualToTop>(this, &result);

        return result;
    }

    bool operator <= (const weak<Types...>&& other) const {
        bool result = false;

        other.run<lessThanEqualToTop>(this, &result);

        return result;
    }

    bool operator >= (const weak<Types...>& other) const {
        bool result = false;

        other.run<greaterThanEqualToTop>(this, &result);
        return result;
    }

    bool operator >= (const weak<Types...>&& other) const {
        bool result = false;

        other.run<greaterThanEqualToTop>(this, &result);
        return result;
    }

    weak<Types...>& operator +=(const weak<Types...>& other) {
        *this = *this+other;
        return *this;
    };

    weak<Types...>& operator -=(const weak<Types...>& other) {
        *this = *this-other;
        return *this;
    };

    weak<Types...>& operator *=(const weak<Types...>& other) {
        *this = *this * other;
        return *this;
    }

    weak<Types...>& operator /=(const weak<Types...>& other) {
        *this = *this / other;
        return *this;
    }

private:


    template <typename T>
    struct equalTop {
        void operator() (T val, const weak<Types...>* other, bool* result) {
            other -> run<equalBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct equalBottom;

    template <typename T, typename V>
    struct equalBottom<T, V, typename std::enable_if<has_equal<T, V>::value>::type> {
        void operator() (T val, V val1, bool* result) {
            *result = val == val1;
        }
    };

    template <typename T, typename V>
    struct equalBottom<T, V, typename std::enable_if<!has_equal<T, V>::value>::type>{
        void operator() (T val, V val1, bool* result){

        };
    };


    template <typename T>
    struct notEqualTop {
        void operator() (T val, const weak<Types...>* other, bool* result) {
            other -> run<notEqualBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct notEqualBottom;

    template <typename T, typename V>
    struct notEqualBottom<T, V, typename std::enable_if<has_not_equal<T, V>::value>::type> {
        void operator() (T val, V val1, bool* result) {
            *result = val != val1;
        }
    };

    template <typename T, typename V>
    struct notEqualBottom<T, V, typename std::enable_if<!has_not_equal<T, V>::value>::type> {
        void operator() (T val, V val1, bool* result){

        };
    };

    /// Addition implementation
    template <typename T>
    struct addTop {
        void operator() (T val, const weak<Types...>* other, weak<Types...>* adding) {
            other -> run<addBottom, T>(val, adding);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct addBottom;

    template <typename T, typename V>
    struct addBottom<T, V, typename std::enable_if<has_addition<T, V>::value>::type> {
        void operator() (T val, V val1, weak<Types...>* adding) {
            adding -> emplace(val + val1);
        }
    };


    template <typename T, typename V>
    struct addBottom<T, V, typename std::enable_if<!has_addition<T, V>::value>::type> {
        void operator() (T val, V val1, weak<Types...>* adding){};
    };

    /// Addition implementation
    template <typename T>
    struct subtractTop {
        void operator() (T val, const weak<Types...>* other, weak<Types...>* adding) {

            other -> run<subtractBottom, T>(val, adding);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct subtractBottom;

    template <typename T, typename V>
    struct subtractBottom<T, V, typename std::enable_if<has_subtraction<T, V>::value>::type> {

        void operator() (T val, V val1, weak<Types...>* adding) {
            adding -> emplace(val-val1);
        }
    };


    template <typename T, typename V>
    struct subtractBottom<T, V, typename std::enable_if<!has_subtraction<T, V>::value>::type> {
        void operator() (T val, V val1, weak<Types...>* adding){};
    };

    /// Multiplication implementation
    template <typename T>
    struct multTop {
        void operator() (T val, const weak<Types...>* other, weak<Types...>* multiplying) {
            other -> run<multBottom, T>(val, multiplying);
        }
    };

    template <typename T, typename V, typename enable=void>
    struct multBottom;

    template <typename T, typename V>
    struct multBottom<T, V, typename std::enable_if<has_multiplication<T, V>::value>::type> {
        void operator() (T val, V val1, weak<Types...>* multiplying) {
            multiplying -> emplace(val * val1);
        }
    };

    template <typename T, typename V>
    struct multBottom<T, V, typename std::enable_if<!has_multiplication<T, V>::value>::type> {
        void operator() (T val, V val1, weak<Types...>* multiplying){

        }
    };

    template <typename T>
    struct divideTop {
        void operator() (T val, const weak<Types...>* numerator, weak<Types...>* dividing ) {
            numerator -> run<divideBottom, T>(val, dividing);
        }
    };

    template <typename T, typename V, typename enable=void>
    struct divideBottom;

    template< typename  T, typename V>
    struct divideBottom<T, V, typename std::enable_if<has_division<T, V>::value>::type>  {
        void operator() (T val, V val2, weak<Types...>* dividing) {
            dividing -> emplace(val/val2);
        }
    };

    template< typename  T, typename V>
    struct divideBottom<T, V, typename std::enable_if<!has_division<T, V>::value>::type>  {
        void operator() (T val, V val2, weak<Types...>* dividing) {

        }
    };

    template <typename T>
    struct lessTop {
        void operator() (T val, const weak<Types...>* _this, bool* result) {
            _this->run<lessBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct lessBottom;

    template <typename T, typename V>
    struct lessBottom<T, V, typename std::enable_if<has_less_than<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {
            *result = val2 > val;
        }
    };

    template <typename T, typename V>
    struct lessBottom<T, V, typename std::enable_if<!has_less_than<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {

        }
    };

    template <typename T>
    struct greaterTop {
        void operator() (T val, const weak<Types...>* _this, bool* result) {
            _this->run<greaterBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct greaterBottom;

    template <typename T, typename V>
    struct greaterBottom<T, V, typename std::enable_if<has_greater_than<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {
            *result = val > val2;
        }
    };

    template <typename T, typename V>
    struct greaterBottom<T, V, typename std::enable_if<!has_greater_than<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {

        }
    };

    template <typename T>
    struct lessThanEqualToTop {
        void operator() (T val, const weak<Types...>* _this, bool* result) {
            _this->run<lessThanEqualToBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct lessThanEqualToBottom;

    template <typename T, typename V>
    struct lessThanEqualToBottom<T, V, typename std::enable_if<has_less_than_equal_to<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {
            *result = val <= val2;
        }
    };

    template <typename T, typename V>
    struct lessThanEqualToBottom<T, V, typename std::enable_if<!has_less_than_equal_to<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {

        }
    };

    template <typename T>
    struct greaterThanEqualToTop {
        void operator() (T val, const weak<Types...>* _this, bool* result) {
            _this->run<greaterThanEqualToBottom, T>(val, result);
        }
    };

    template <typename T, typename V, typename enable = void>
    struct greaterThanEqualToBottom;

    template <typename T, typename V>
    struct greaterThanEqualToBottom<T, V, typename std::enable_if<has_greater_than_equal_to<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {
            *result = val >= val;
        }
    };

    template <typename T, typename V>
    struct greaterThanEqualToBottom<T, V, typename std::enable_if<!has_greater_than_equal_to<T, V>::value>::type> {
        void operator() (T val, V val2, bool* result) {

        }
    };
};

#endif //WEAK_H
