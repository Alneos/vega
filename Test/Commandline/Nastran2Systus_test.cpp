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
//#include "build_properties.h"
#define RUN_SYSTUS false
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

BOOST_AUTO_TEST_CASE( cbush ) {
	CommandLineUtils::nastranStudy2Systus("/irt/cbush/cbush.inp", RUN_SYSTUS, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( truss1 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/truss1/truss1.nas", RUN_SYSTUS, true, 0.001);
}

//BOOST_AUTO_TEST_CASE( truss5 ) {
// COMBINATION
//	CommandLineUtils::nastranStudy2Systus("/irt/truss5/truss5.nas", RUN_SYSTUS, true, 0.001);
//}

BOOST_AUTO_TEST_CASE( pload4 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/pload4/pload4.nas", RUN_SYSTUS, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( VSLX0005 ) {
//	CommandLineUtils::nastranStudy2Systus("/../../../../tmp/tests/VSLX0005-19/VSLX0005-19.nas", RUN_SYSTUS, true, 0.00001);
//}
//
//BOOST_AUTO_TEST_CASE( VSLX0006 ) {
//	CommandLineUtils::nastranStudy2Systus("/../../../../tmp/tests/VSLX0006-19/VSLX0006-19.nas", RUN_SYSTUS, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( rbe3 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/rbe3/rbe3.nas", RUN_SYSTUS, true, 0.000001);
}

//BOOST_AUTO_TEST_CASE( cbush1 ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/cbush1/cbush1.bdf", RUN_SYSTUS, true, 0.00001);
//}

//BOOST_AUTO_TEST_CASE( cbush1b ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/cbush1b/cbush1b.bdf", RUN_SYSTUS, true, 0.00001);
//}

BOOST_AUTO_TEST_CASE( ssll11a ) {
	CommandLineUtils::nastranStudy2Systus("/irt/ssll11a/ssll11a.nas", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( ssll11c ) {
	CommandLineUtils::nastranStudy2Systus("/irt/ssll11c/ssll11c.nas", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( bugcoords ) {
	CommandLineUtils::nastranStudy2Systus("/irt/bug-coord/bug-coord.nas", false, true, 0.00001);
}

//BOOST_AUTO_TEST_CASE( nas101prob2 ) {
    // ForceLine
	//CommandLineUtils::nastranStudy2Systus("/irt/nas101prob2/nas101prob2.dat", RUN_SYSTUS, true, 0.15);
//}

BOOST_AUTO_TEST_CASE( nas101prob3 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob3/nas101prob3.nas", RUN_SYSTUS, true, 0.005);
}

BOOST_AUTO_TEST_CASE( nas101prob4 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob4/nas101prob4.nas", RUN_SYSTUS, false, 0.005);
}

//BOOST_AUTO_TEST_CASE( nas102prob5 ) {
//  Missing DIRECT solver
//	CommandLineUtils::nastranStudy2Systus("/irt/nas102prob5/nas102prob5.bdf", RUN_SYSTUS, true, 0.05);
//}

BOOST_AUTO_TEST_CASE( nas101prob6 ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101prob6/nas101prob6.nas", RUN_SYSTUS, true, 0.1);
}

BOOST_AUTO_TEST_CASE( nas101probA ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101probA/nas101probA.bdf", RUN_SYSTUS, true, 0.05);
}

BOOST_AUTO_TEST_CASE( nas101probB ) {
	CommandLineUtils::nastranStudy2Systus("/irt/nas101probB/nas101probB.nas", RUN_SYSTUS, true, 0.1);
}

} /* namespace test */
} /* namespace vega */
