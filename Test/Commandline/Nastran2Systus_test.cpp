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
//don't run systus on windows for now
#ifdef WIN32
#define RUN_SYSTUS false
#else
#define RUN_SYSTUS true
#endif
//____________________________________________________________________________//

namespace vega {
namespace tests {

using namespace std;

BOOST_AUTO_TEST_CASE( maxim ) {
	CommandLineUtils::nastranStudy2Systus("/linstat/maxim/maxim.dat", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( rod1 ) {
	CommandLineUtils::nastranStudy2Systus("/linstat/rod1/rod1.dat", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( rod2 ) {
	CommandLineUtils::nastranStudy2Systus("/linstat/rod2/rod2.dat", RUN_SYSTUS, true, 0.000001);
}

BOOST_AUTO_TEST_CASE( bd03bar1 ) {
	CommandLineUtils::nastranStudy2Systus("/lindyna/bd03bar1/bd03bar1.dat", true, true, 0.03);
}

} /* namespace test */
} /* namespace vega */
