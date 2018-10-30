/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran2systus_test
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE

#include <boost/test/unit_test.hpp>
#include <string>
#include "build_properties.h"
#include "CommandLineUtils.h"
//____________________________________________________________________________//

namespace vega {
namespace tests {

using namespace std;

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/test4a/test4a.dat", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/rbar1mod/rbar1mod.dat", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Systus("/alneos/rod1freeforce/rod1freeforce.dat", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( prob19 ) {
	CommandLineUtils::nastranStudy2Systus("/caw/prob19/prob19.dat", RUN_SYSTUS, true, 0.001);
}

BOOST_AUTO_TEST_CASE( fixedcircularplate ) {
	CommandLineUtils::nastranStudy2Systus("/irt/fixed_circular_plate/fixed_circular_plate.dat", RUN_SYSTUS, true, 0.1);
}

BOOST_AUTO_TEST_CASE( prob2 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/prob2/prob2.dat", RUN_SYSTUS, true, 0.15);
}


} /* namespace test */
} /* namespace vega */
