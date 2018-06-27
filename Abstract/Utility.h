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
#ifdef __GNUC__
// Avoid tons of warnings with the following code
#pragma GCC system_header
#endif
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/variant.hpp>

namespace vega {
/**
 * Tolerance used in comparing double.
 */
const double DOUBLE_COMPARE_TOLERANCE = std::numeric_limits<double>::epsilon() * 5;

inline bool is_zero(double x, double tolerance = DOUBLE_COMPARE_TOLERANCE) {
	return std::abs(x) <= tolerance;
}

inline bool is_equal(double x, double y, double tolerance = DOUBLE_COMPARE_TOLERANCE) {
	return std::abs(x - y) <= tolerance * std::max(1.0, std::max(std::abs(x), std::abs(y)));
}

namespace ublas = boost::numeric::ublas;

	/**
	 * The following code inverts the matrix input using LU-decomposition with backsubstitution of unit vectors. Reference: Numerical Recipies in C, 2nd ed., by Press, Teukolsky, Vetterling & Flannery.
	 * From https://gist.github.com/lilac/2464434
	 */
	bool InvertMatrix(const ublas::matrix<double>& input, ublas::matrix<double>& inverse);

		} /* namespace vega */
#endif /* UTILITY_H_ */
