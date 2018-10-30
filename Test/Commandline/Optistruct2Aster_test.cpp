/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE optistruct2aster_test
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

BOOST_AUTO_TEST_CASE( prob6 ) {
	CommandLineUtils::optistructStudy2Aster("/caw/prob6/prob6.dat", RUN_ASTER, true, 0.02);
}

/*BOOST_AUTO_TEST_CASE( ploads ) {
	CommandLineUtils::nastranStudy2Aster("/irt/t01331/t01331a.inp", RUN_ASTER, true, 0.00001);
}*/

} /* namespace test */
} /* namespace vega */
