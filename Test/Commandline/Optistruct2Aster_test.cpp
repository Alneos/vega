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

BOOST_AUTO_TEST_CASE( beam ) {
	CommandLineUtils::optistructStudy2Aster("/irt/beam/beam.fem", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( nas102prob6reuse ) {
	CommandLineUtils::optistructStudy2Aster("/irt/nas102prob6reuse/nas102prob6reuse.fem", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( shell ) {
	CommandLineUtils::optistructStudy2Aster("/irt/shell/shell.fem", RUN_ASTER, true, 0.05);
}

//BOOST_AUTO_TEST_CASE( bigtest ) {
//	CommandLineUtils::optistructStudy2Aster("/../../../../local/bigtest/bigtest.fem", false, true, 0.03);
//}

} /* namespace test */
} /* namespace vega */
