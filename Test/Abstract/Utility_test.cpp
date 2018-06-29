/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */

#define BOOST_TEST_MODULE utility_tests
#include "build_properties.h"
#include "../../Abstract/Value.h"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_vectorial_value ) {
	VectorialValue v = VectorialValue(1., 2., 3.);
	VectorialValue v2 = VectorialValue(2., 4., 0.);
	BOOST_CHECK_EQUAL(v.norm(), sqrt(14.));
	BOOST_CHECK_EQUAL(v2.norm(), sqrt(20.));
	BOOST_CHECK_EQUAL(v.dot(v2), 10);
	BOOST_CHECK_EQUAL(v.normalized().x(), 1. / sqrt(14.));
	BOOST_CHECK_EQUAL(v.scaled(42.0).y(), 84.0);
	// test with a precision
	VectorialValue v3 = v.orthonormalized(v2);
	BOOST_CHECK_EQUAL(v2.dot(v3), 0);
	//BOOST_CHECK_CLOSE(v.orthonormalized(v2).x(), (1.-sqrt(20.))/sqrt(114.-10*sqrt(20.)), 10e-12);

	BOOST_CHECK_EQUAL(v.cross(v2).x(), -12.);

	v.scale(5.0);
	BOOST_CHECK_CLOSE(v.x(), 5.0, 10e-12);
	BOOST_CHECK_CLOSE(v.y(), 10.0, 10e-12);
	BOOST_CHECK_CLOSE(v.z(), 15.0, 10e-12);
	BOOST_CHECK(!v.iszero());
	VectorialValue vzero = VectorialValue(0., 0., 0.);
	BOOST_CHECK(vzero.iszero());

}

BOOST_AUTO_TEST_CASE( valueOrReference_val ) {
	ValueOrReference val1(2.3);
	ValueOrReference val2(2);
	ValueOrReference val2c(2);
	BOOST_CHECK_LT(val2, val1);
	BOOST_CHECK(val1 != val2);
	BOOST_CHECK_EQUAL(val2, val2c);
	ValueOrReference val2d = val2;
	BOOST_CHECK_EQUAL(val2, val2d);
}

BOOST_AUTO_TEST_CASE( valueOrReference_ref ) {
	ValueOrReference ref1(Reference<NamedValue>(Value::FUNCTION_TABLE, 1, 5));
	ValueOrReference ref2(Reference<NamedValue>(Value::FUNCTION_TABLE, 2, 6));
	ValueOrReference val2(2);
	BOOST_CHECK_LT(ref1, ref2);
	BOOST_CHECK(ref1 != val2);
	ValueOrReference ref2a = ref2;
	BOOST_CHECK_EQUAL(ref2a, ref2);
}
