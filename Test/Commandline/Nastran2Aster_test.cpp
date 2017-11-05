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

// TEST_EXEC_SOLVER is "false" on development pc and "true" on node0
// Code Aster launched only on node0, to launch it in the local machine
// either replace "TESTS_EXEC_SOLVER" with true or run
// cmake -TESTS_EXEC_SOLVER=1 ../..

namespace vega {
namespace tests {

using namespace std;

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/test4a/test4a.dat", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/rbar1mod/rbar1mod.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/rod1freeforce/rod1freeforce.dat", true, true, 0.00001);
}

} /* namespace test */
} /* namespace vega */
