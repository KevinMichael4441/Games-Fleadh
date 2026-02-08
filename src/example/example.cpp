#include "./example/example.hpp"

const std::string GetMessage()
{
    static std::string message = "Hello from C++ and Raylib";
    return message;
}