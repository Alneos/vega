/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Utility.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: devel
 */

#include "Utility.h"
#include "Value.h"
#include <math.h>       /* pow */
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <cmath>

namespace ublas = boost::numeric::ublas;

namespace vega {

using namespace std;

bool InvertMatrix(const ublas::matrix<double>& input, ublas::matrix<double>& inverse)
{
   typedef ublas::permutation_matrix<std::size_t> pmatrix;

   // create a working copy of the input
   ublas::matrix<double> A(input);

   // create a permutation matrix for the LU-factorization
   pmatrix pm(A.size1());

   // perform LU-factorization
   int res = boost::numeric_cast<int>(lu_factorize(A, pm));
   if (res != 0)
       return false;

   // create identity matrix of "inverse"
   inverse.assign(ublas::identity_matrix<double> (A.size1()));

   // backsubstitute to get the inverse
   ublas::lu_substitute(A, pm, inverse);
   return true;
}


//__________ ValueOrReference

} /* namespace vega */
