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

BOOST_AUTO_TEST_CASE( anis1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/anis1/anis1.dat", true, true, 0.00001);
}

/*BOOST_AUTO_TEST_CASE( bar1b ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/bar1b/bar1b.dat", true, true, 0.01);
}

BOOST_AUTO_TEST_CASE( bar1n ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/bar1n/bar1n.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( bar2 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/bar2/bar2.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( beam1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/beam1/beam1.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( const1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/const/const1.dat", true, true, 0.5);
}

BOOST_AUTO_TEST_CASE( const2 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/const/const2.dat", true, true, 0.5);
}


BOOST_AUTO_TEST_CASE( const3 ) {
	// test fail
	CommandLineUtils::nastranStudy2Nastran("/linstat/const/const3.dat", false, true, 0.9);
}

BOOST_AUTO_TEST_CASE( const4 ) {
	// test fail
	CommandLineUtils::nastranStudy2Nastran("/linstat/const/const4.dat", false, true);
}

BOOST_AUTO_TEST_CASE( diag ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/diag/diag.dat", true, true, 0.02);
}

BOOST_AUTO_TEST_CASE( grav2 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/grav2/grav2.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( ibeam ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/ibeam/ibeam.dat", true, true, 0.01);
}

BOOST_AUTO_TEST_CASE( maxim ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/maxim/maxim.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( mpc1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/mpc1/mpc1.dat", true, true, 0.001);
}

BOOST_AUTO_TEST_CASE( plate1 ) {
	// test fail
	CommandLineUtils::nastranStudy2Nastran("/linstat/plate1/plate1.dat", true, true, 12.8);
}

BOOST_AUTO_TEST_CASE( plate1r ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/plate1r/plate1r.dat", true, true);
}

BOOST_AUTO_TEST_CASE( quad1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/quad1/quad1.dat", true, true, 0.01);
}

BOOST_AUTO_TEST_CASE( quadload ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/quadload/quadload.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/rbar1/rbar1.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rforce1 ) {
	// Le TYPE_ELEMENT MECA_DIS_TR_N  ne sait pas encore calculer l'option:  CHAR_MECA_ROTA_R
	CommandLineUtils::nastranStudy2Nastran("/linstat/rforce1/rforce1.dat", false, true);
}

BOOST_AUTO_TEST_CASE( rforce2 ) {
	// Le TYPE_ELEMENT MECA_DIS_TR_N  ne sait pas encore calculer l'option:  CHAR_MECA_ROTA_R
	CommandLineUtils::nastranStudy2Nastran("/linstat/rforce2/rforce2.dat", false, true);
}

BOOST_AUTO_TEST_CASE( rod1 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/rod1/rod1.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod2 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/rod2/rod2.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rotate ) {
	// Le TYPE_ELEMENT MECA_DIS_TR_N  ne sait pas encore calculer l'option:  CHAR_MECA_ROTA_R
	CommandLineUtils::nastranStudy2Nastran("/linstat/rotate/rotate.dat", false, true);
}

BOOST_AUTO_TEST_CASE( shear2 ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/shear2/shear2.dat", true, true, 1.1);
}

BOOST_AUTO_TEST_CASE( bd03bar1 ) {
	CommandLineUtils::nastranStudy2Nastran("/lindyna/bd03bar1/bd03bar1.dat", true, true, 0.03);
}

BOOST_AUTO_TEST_CASE( bd05barV4 ) {
	CommandLineUtils::nastranStudy2Nastran("/lindyna/bd05barV4/bd05barV4.dat", true, true, 0.081);
}

BOOST_AUTO_TEST_CASE( test4a ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/test4a/test4a.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rbar1mod ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/rbar1mod/rbar1mod.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( rod1freeforce ) {
	CommandLineUtils::nastranStudy2Nastran("/alneos/rod1freeforce/rod1freeforce.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( prob1 ) {
	CommandLineUtils::nastranStudy2Nastran("/workshop/prob1/prob1.dat", true, true, 0.00001);
}

BOOST_AUTO_TEST_CASE( prob6 ) {
	CommandLineUtils::nastranStudy2Nastran("/workshop/prob6/prob6.dat", true, true, 0.02);
}

BOOST_AUTO_TEST_CASE( prob30c ) {
	CommandLineUtils::nastranStudy2Nastran("/workshop/prob30c/prob30c.dat", true, true, 0.06);
}

BOOST_AUTO_TEST_CASE( dmigstfl ) {
	CommandLineUtils::nastranStudy2Nastran("/linstat/dmigstfl/dmigstfl.dat", true, true, 0.001);
}*/

} /* namespace test */
} /* namespace vega */
