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

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/test4a/test4a.dat", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/rbar1mod/rbar1mod.dat", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/rod1freeforce/rod1freeforce.dat", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( n4w101 ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/n4w101/n4w101.nas", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( sdld27a ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/sdld27a/sdld27a.bdf", false, true, 0.00001);
}

} /* namespace test */
} /* namespace vega */
