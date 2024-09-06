#include "../include/commonexcept.hpp"

InitializedException::InitializedException() : std::runtime_error("Instance already initialized") {}
UninitializedException::UninitializedException() : std::runtime_error("Instance not initialized") {}
CountLimitException::CountLimitException() : std::runtime_error("Maximum count reached") {}

void checkInitialized(bool initialized) {
    if (initialized)
        throw InitializedException();
}

void checkUninitialized(bool initialized) {
    if (!initialized)
        throw UninitializedException();
}