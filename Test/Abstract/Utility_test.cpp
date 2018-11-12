/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */

#define BOOST_TEST_MODULE utility_tests
#include "build_properties.h"
#include "../../Abstract/Utility.h"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_backtrace ) {
	stacktrace(); // Only to check if this works
	BOOST_CHECK(true);
}
