#pragma once

#include <functional>
#include <string>

// this is what is returned when you "allocate" a function
// it provides the functions to compile code and execute the code after being compiled

template < typename __FunctionType >
class KnifeFunction : public std::function< __FunctionType >
{

    // stores the string data for the last attempted compile
    std::wstring stringdata;

    // information from the C# compiler (errors, notes, warnings, info)
    std::wstring compileinfo;

    bool ready = false;

public:
    // Doesnt initialise anything
    KnifeFunction() : std::function( nullptr )
    {
    }

    // Initialises the string data and attempts to compile
    KnifeFunction( std::wstring &stringdata ) : std::function( nullptr ), stringdata( stringdata )
    {
    }

    // attempts to compile the string data
    void Compile();

	std::wstring &GetCompileInfo();
};

// This class manages & provides C# functions for other classes to use

// Other classes need to be able to request a function to be "allocated"
// Other classes can then provide code that:
//		1. includes the function definition and the arguments
//		2. does not include any imports or other functions
// If compilation fails - it should provide the reasons to the providee
// After a function has been successfully "allocated" and compiled, the providee should be able to call it

// This class is backed in C# with the KnifeProvider class that handles the compilation and appdomains

class KnifeProvider
{
public:
};