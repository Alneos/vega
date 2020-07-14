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

BOOST_AUTO_TEST_CASE( test_2dof108 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/2dof108/2dof108.bdf", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE(cyl_sol105) {
    // slow test, no reference results yet
	CommandLineUtils::nastranStudy2Aster("/cnes/cyl_sol105/cyl_sol105.bdf", false, true, 0.01);
}

BOOST_AUTO_TEST_CASE( ssnv104i ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv104i/ssnv104i.bdf", RUN_ASTER, true, 0.04);
}

BOOST_AUTO_TEST_CASE( ssnv129a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv129a/ssnv129a.bdf", RUN_ASTER, true, 0.01);
}

BOOST_AUTO_TEST_CASE( t01331a ) {
    // Results NOOK, still missing THETA CTRIA3 orientation (should become a ANGL_VRIL)
	CommandLineUtils::nastranStudy2Aster("/irt/t01331/t01331a.inp", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE(sdlv302a) {
	CommandLineUtils::nastranStudy2Aster("/irt/sdlv302a/sdlv302a.bdf", RUN_ASTER, true, 0.05);
}

} /* namespace test */
} /* namespace vega */
