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

BOOST_AUTO_TEST_CASE( prob6 ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob6/prob6.dat", RUN_ASTER, true, 0.02);
}

BOOST_AUTO_TEST_CASE( prob19 ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob19/prob19.dat", RUN_ASTER, true, 0.001);
}

BOOST_AUTO_TEST_CASE( prob30c ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob30c/prob30c.dat", RUN_ASTER, true, 0.9);
}

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/test4a/test4a.dat", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/rbar1mod/rbar1mod.dat", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Aster("/alneos/rod1freeforce/rod1freeforce.dat", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( fixedcircularplate ) {
	CommandLineUtils::nastranStudy2Aster("/irt/fixed_circular_plate/fixed_circular_plate.dat", RUN_ASTER, true, 0.1);
}

BOOST_AUTO_TEST_CASE( prob2 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/prob2/prob2.dat", RUN_ASTER, true, 0.15);
}

BOOST_AUTO_TEST_CASE( t01331a ) {
    // TODO : LD test not yet complete, but running
	CommandLineUtils::nastranStudy2Aster("/irt/t01331/t01331a.inp", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( cbush ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush/cbush.inp", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( probA ) {
  // TODO : LD RESU NOOK, maybe mesh refine ?
	CommandLineUtils::nastranStudy2Aster("/irt/probA/probA.bdf", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE( cbush1 ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush1/cbush1.bdf", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( cbush1b ) {
	CommandLineUtils::nastranStudy2Aster("/irt/cbush1b/cbush1b.bdf", RUN_ASTER, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( prob5 ) {
    // TODO : LD missing G param, tests not found, otherwise ok
	CommandLineUtils::nastranStudy2Aster("/irt/prob5/prob5.bdf", RUN_ASTER, true, 0.05);
}

BOOST_AUTO_TEST_CASE( prob9 ) {
	CommandLineUtils::nastranStudy2Aster("/caw/prob9/prob9.dat", false, false, 0.02);
}

BOOST_AUTO_TEST_CASE( ssnv129a ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv129a/ssnv129a.bdf", RUN_ASTER, true, 0.01);
}

BOOST_AUTO_TEST_CASE( ssnv104i ) {
	CommandLineUtils::nastranStudy2Aster("/irt/ssnv104i/ssnv104i.bdf", RUN_ASTER, true, 0.03);
}

//BOOST_AUTO_TEST_CASE( prob24 ) {
//	CommandLineUtils::nastranStudy2Aster("/caw/prob24/prob24.dat", RUN_ASTER, true, 0.05);
//}

//BOOST_AUTO_TEST_CASE( test_2dof108 ) {
//	CommandLineUtils::nastranStudy2Aster("/irt/2dof108/2dof108.bdf", false, false, 0.05);
//}



} /* namespace test */
} /* namespace vega */
