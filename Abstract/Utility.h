/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Utility.h
 *
 *  Created on: Nov 10, 2013
 *      Author: devel
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#define UNUSEDV(x) (void)(x)

#include <climits>
#include <string>
#include <cmath>
#include <stdio.h>
#include <cfloat>
#include "prettyprint.hpp"

#if defined(__GNUC__)
// Avoid tons of warnings with the following code
#pragma GCC system_header
#endif

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/variant.hpp>
#include "build_properties.h"
#if Backtrace_FOUND
#include <signal.h>
#elif LIBUNWIND_FOUND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

// https://stackoverflow.com/a/54293978
#define CHECK_ENUM_CLASS_EQUAL(L, R) BOOST_CHECK_EQUAL(static_cast<int>(L), static_cast<int>(R))

namespace vega {

class Globals {
public:
	static constexpr int UNAVAILABLE_INT = INT_MIN;
	static constexpr unsigned char UNAVAILABLE_UCHAR = UCHAR_MAX;
	static constexpr double UNAVAILABLE_DOUBLE = -DBL_MAX;
	static constexpr double DOUBLE_COMPARE_TOLERANCE = std::numeric_limits<double>::epsilon() * 5;
};

enum class MatrixType {
    SYMMETRIC,
    FULL,
    DIAGONAL
};

/**
 * Tolerance used in comparing double.
 */


inline bool is_zero(double x, double tolerance = Globals::DOUBLE_COMPARE_TOLERANCE) noexcept {
	return std::abs(x) <= tolerance;
}

inline bool is_equal(double x, double y, double tolerance = Globals::DOUBLE_COMPARE_TOLERANCE) noexcept {
	return std::abs(x - y) <= tolerance * std::max(1.0, std::max(std::abs(x), std::abs(y)));
}

namespace ublas = boost::numeric::ublas;

/**
 * The following code inverts the matrix input using LU-decomposition with backsubstitution of unit vectors. Reference: Numerical Recipies in C, 2nd ed., by Press, Teukolsky, Vetterling & Flannery.
 * From https://gist.github.com/lilac/2464434
 */
bool InvertMatrix(const ublas::matrix<double>& input, ublas::matrix<double>& inverse);

void stacktrace();
void handler(int sig);

/**
 * https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
 */
struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(const T t) const noexcept
    {
        return static_cast<std::size_t>(t); // you don't need to provide a specialization of std::hash, the template argument deduction does the job
    }
};

} /* namespace vega */

// https://isocpp.org/files/papers/N3656.txt
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {
    template<class T> struct _Unique_if {
        using _Single_object = unique_ptr<T>;
    };

    template<class T> struct _Unique_if<T[]> {
        using _Unknown_bound = unique_ptr<T[]>;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        using _Known_bound = void;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            using U = typename remove_extent<T>::type;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

#endif /* UTILITY_H_ */
