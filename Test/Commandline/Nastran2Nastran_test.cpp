/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * VegaCommandLine_test.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran2nastran_test
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

BOOST_AUTO_TEST_CASE( gpstress ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/gpstress/gpstress.nas", false, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( test4a_rbe2 ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/test4a_rbe2/test4a_rbe2.nas", RUN_NASTRAN, true, 0.05);
}

BOOST_AUTO_TEST_CASE( rbe2_z ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/rbe2_z/rbe2_z.nas", RUN_NASTRAN, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_y ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/rbe2_y/rbe2_y.nas", RUN_NASTRAN, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_yz ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/rbe2_yz/rbe2_yz.nas", RUN_NASTRAN, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_3nodes ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/rbe2_3nodes/rbe2_3nodes.nas", RUN_NASTRAN, true, 1.5);
}

BOOST_AUTO_TEST_CASE( rbe2_4nodes ) {
	CommandLineUtils::nastranStudy2Nastran("/irt/rbe2_4nodes/rbe2_4nodes.nas", RUN_NASTRAN, true, 1.5);
}

} /* namespace test */
} /* namespace vega */
