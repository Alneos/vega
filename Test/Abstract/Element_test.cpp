/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */

#define BOOST_TEST_MODULE Element_test
#include <boost/test/unit_test.hpp>
#include "../../Abstract/Model.h"
#include "../../Abstract/Element.h"

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_differentnodes ) {
	Model model("fakemodel");
	DiscreteSegment discrete(model);
	double expected = -25000;
	discrete.addStiffness(0,1,DOF::RY, DOF::DX, expected);
	double found = discrete.findStiffness(0,1,DOF::RY, DOF::DX);
	BOOST_CHECK(is_equal(found, expected));
}
