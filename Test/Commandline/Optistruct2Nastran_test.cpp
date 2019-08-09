/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran2aster_test
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE
//#define BOOST_TEST_IGNORE_SIGCHLD

//#define BOOST_TEST_LIMITED_SIGNAL_DETAILS

#include <boost/test/unit_test.hpp>
#include <string>
#include "build_properties.h"
#include "CommandLineUtils.h"

//____________________________________________________________________________//

namespace vega {
namespace tests {

using namespace std;

//BOOST_AUTO_TEST_CASE( prob6 ) {
//	CommandLineUtils::optistructStudy2Nastran("/caw/prob6/prob6.dat", RUN_NASTRAN, true, 0.02);
//}

BOOST_AUTO_TEST_CASE( beam ) {
    // multiple subcases
    // MAXIMUM LINE COUNT EXCEEDED IN SUBROUTINE PAGE     LINE COUNT EQUALS   20016
	CommandLineUtils::optistructStudy2Nastran("/irt/beam/beam.fem", false, true, 0.02);
}

} /* namespace test */
} /* namespace vega */
