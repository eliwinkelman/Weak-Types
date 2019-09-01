# Weak-Types
An implementation of weak typing for C++11 and Arduino.

Thanks to [Foo Nathan](https://github.com/foonathan) for inspiration on variant implementation from his typesafe library.

Should work for any types as long as they all have default constructors.

## Usage

``` c++
  using var = weak<int, float, double, std::string>;
  
  var myInt = 1;
  var myDouble = 14.2;
  var myFloat = 1.3f;
  var myString = std::string("Hello World");
  
```

## Undefined Behavior

* Operations on objects when the underlying type doesn't have the operation defined. (Currently implementing defined behavior and will document.)

## weak<Types...> Methods

#### void emplace(T val)
Input: T val

Returns: Void

Behavior: 
If T is a type in weak<Types...>, destroys the previous stored value, stores val, and updates the type to T.
If T is not a type in weak<Types...> triggers a static assert and causes a failure at compile time.

#### bool isValid()
Input: None

Returns: bool

Behavior: Returns true if the weak<Types...> is in a valid state (i.e. current_type refers to a type in Types...). Returns false otherwise.

#### bool isType<Type>() 
Input: a Type template parameter.

Returns: Bool

Behavior: Returns true if the underlying value has is of type Type, returns false otherwise.

  
#### void run<Functor<typename T>>(Args... args)
Input: 
  
  * Functor: a struct with at least one templated type T and a defined () operator which accepts a value of type T as the first argument.
  * args: a list of arguments expected by Functor(T val, Args... args)

Returns: void

Behavior: Calls Functor() with the underlying value as the first argument and the rest of the arguments.

Example:

```c++
  #include <iostream>
  #include "weak.h"
  // a print Functor.
  template <typename T>
  struct printVal {
      void operator ()(T val) {
          std::cout << val << std::endl;
      }
  };
  
  int main() {
    weak<float, int, char*> myPrintableInt = 12;
    
    myPrintableInt.run<printVal>(); // 12
  }
  // 
```

#### T&& value<Type>(ErrorHandlerFunc errorHandlerFunc) 
Inputs: A type and an error handling function that accepts a string as it's only argument.

Output: The underlying value of type T or an instance of T created by T's default constructor.

Behavior: 
If the underlying value is of type T, returns the underlying value. Otherwise it calls errorHandlerFunc with the error message "\[ERROR\] Weak Types: Attempting to access underlying value with incorrect type." and returns an object created with T's default constructor.


