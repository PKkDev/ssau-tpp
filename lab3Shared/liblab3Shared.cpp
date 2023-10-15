#include <iostream>

static unsigned index_;               // Current seq. position
static unsigned long long current_;   // Current sequence value
static unsigned long long previous_;  // Previous value, if any

// Initialize a Fibonacci relation sequence
// such that F(0) = a, F(1) = b.
// This function must be called before any other function.
void fibonacci_init2( const unsigned long long a, const unsigned long long b)
{
    index_ = 0;
    current_ = a;
    previous_ = b; // see special case when initialized
    std::cout << "Lib: Старт" << std::endl;
}
