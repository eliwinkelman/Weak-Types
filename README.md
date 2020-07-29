# Weak-Types
An implementation of weak typing for C++11 and Arduino.

Thanks to [Foo Nathan](https://github.com/foonathan) for inspiration on variant implementation from his typesafe library. The type tracking internals of this library are largely based on his implementation, with some modifications to make things run in C++11/Arduino and to properly override operators to create the operator behavior you would expect from the underlying type.

## Usage

``` c++
  using var = weak<int, float, double, std::string>;
  
  var myInt = 1;
  var myDouble = 14.2;
  var myFloat = 1.3f;
  var myString = std::string("Hello World");
  
  var a = myInt + myDouble * myFloat;
  
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

#### bool isType\<Type\>() 
Input: a Type template parameter.

Returns: Bool

Behavior: Returns true if the underlying value has is of type Type, returns false otherwise.

  
#### void run<Functor\<typename T>>(Args... args)
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

#### simple_optional\<Type\> retrieve\<Type>() 
Inputs: A type.

Returns: A simple_optional object.

Behavior: 
If the underlying value is of type T, returns a simple_optional object that holds the underlying value. Otherwise returns a simple_optional object that holds nothing.

Example usage:

``` c++
    using var = weak<int, float, double, std::string>;
    var a = 10;
    a.retrieve<int>().value_or(0); // 10
    a.retrieve<float>().value_or(0); // 0;
    a.retrieve<int>().value(); // 10
    a.retrieve<float>().value(); // Error
    
    if (a.retrieve<float>()) {
      float val = a.retrieve<float>().value();
      
      // do stuff with your float
    }
    else {
      // didn't get a float :(
    }
    
```
