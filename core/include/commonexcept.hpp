#ifndef COMMONEXCEPT_HPP_
#define COMMONEXCEPT_HPP_

/*
    This module is used to define exception classes that are used
    for cases common in various components of the library. Exceptions 
    specific to cases only within or by using individual modules are 
    included within their headers.
*/

#include <iostream>

/* class InitializedException
    This exception is thrown when an instance with an init()/uninit() method
    pair is attempted to be initialized while already initialized.
*/
class InitializedException : public std::runtime_error {
public:
    InitializedException();
};

/* class InitializedException
    This exception is thrown when an instance with an init()/uninit() method
    pair is attempted to have calls made on it while uninitialized.
*/
class UninitializedException : public std::runtime_error {
public:
    UninitializedException();
};

/* class CountLimitException
   This exception is thrown when an instance with a maximum count value has
   a method called that would result in the current count exceeding that value.
*/
class CountLimitException : public std::runtime_error {
public:
    CountLimitException();
};

/* Throws if initialized is true. */
void checkInitialized(bool initialized);

/* Throws if initialized is false. */
void checkUninitialized(bool initialized);

#endif