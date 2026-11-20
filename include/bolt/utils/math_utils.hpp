
#ifndef BOLT_UTILS_MATH_UTILS_HPP_
#define BOLT_UTILS_MATH_UTILS_HPP_

#include <cmath>

class MathUtils {
 public:
    static double squareRoot(double num) { return std::sqrt(num); }

    static double power(double base, double exponent) { return std::pow(base, exponent); }

    static unsigned long long factorial(unsigned int num) {  // NOLINT(runtime/int)
        if (num <= 1)
            return 1;
        return num * factorial(num - 1);
    }
};

#endif  // BOLT_UTILS_MATH_UTILS_HPP_
