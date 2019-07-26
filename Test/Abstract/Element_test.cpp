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
	DiscreteSegment discrete(model, MatrixType::SYMMETRIC);
	double expected = -25000;
	discrete.addStiffness(0,1,DOF::RY, DOF::DX, expected);
	double found = discrete.findStiffness(0,1,DOF::RY, DOF::DX);
	BOOST_CHECK(is_equal(found, expected));
}

BOOST_AUTO_TEST_CASE( test_diagonal ) {
	Model model("fakemodel");
	const auto& virtualDiscretTR = make_shared<DiscretePoint>(model, MatrixType::DIAGONAL);
    for (DOF dof: DOFS::ALL_DOFS) {
        virtualDiscretTR->addStiffness(dof, dof, 0.0);
    }
	BOOST_CHECK(virtualDiscretTR->isDiagonal());
}

BOOST_AUTO_TEST_CASE( test_structuralsegment ) {
	Model model("fakemodel");
    const auto& structuralElement = make_shared<StructuralSegment>(model, MatrixType::DIAGONAL, 55);
    structuralElement->addStiffness(DOF::DX, DOF::DX, 42.0);
    structuralElement->addStiffness(DOF::DY, DOF::DY, 43.0);
    structuralElement->addStiffness(DOF::DZ, DOF::DZ, 44.0);
    structuralElement->addStiffness(DOF::RX, DOF::RX, 45.0);
    structuralElement->addStiffness(DOF::RY, DOF::RY, 46.0);
    structuralElement->addStiffness(DOF::RZ, DOF::RZ, 47.0);
	BOOST_CHECK(structuralElement->isDiagonal());
	BOOST_CHECK(structuralElement->hasRotations());
}
