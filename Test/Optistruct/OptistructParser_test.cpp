/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranParser_test.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#define BOOST_TEST_MODULE optistruct_parser_tests
#include "../../Optistruct/OptistructParser.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#if defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

using namespace std;
using namespace vega;

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE(optistruct_contact) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/contact.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, SolverName::CODE_ASTER, "", ""));
        BOOST_CHECK_EQUAL(model->targets.size(), 2);
        BOOST_CHECK_EQUAL(model->constraints.size(), 1);
        //auto& master = model->find(Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, 1));
        //BOOST_CHECK_NE(master, nullptr);
        //auto& slave = model->find(Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, 2));
        //BOOST_CHECK_NE(slave, nullptr);
        model->finish();
        //BOOST_CHECK((*model->analyses.begin())->type == Analysis::Type::NONLINEAR_MECA_STAT);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_contact_cpenta) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/contact2.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, SolverName::CODE_ASTER, "", ""));
        BOOST_CHECK_EQUAL(model->targets.size(), 2);
        BOOST_CHECK_EQUAL(model->constraints.size(), 1);
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        model->finish();
        //BOOST_CHECK((*model->analyses.begin())->type == Analysis::Type::NONLINEAR_MECA_STAT);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_pload4) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/pload4.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, SolverName::CODE_ASTER, "", ""));
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        auto analysis = model->analyses.first();
        //BOOST_CHECK(not analysis->isLinear());
        BOOST_CHECK_EQUAL(analysis->getConstraintSets().size(), 2);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_rbe2) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/rbe2.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, SolverName::CODE_ASTER, "", ""));
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        auto analysis = model->analyses.first();
        BOOST_CHECK(analysis->isLinear());
        BOOST_CHECK_EQUAL(analysis->getConstraintSets().size(), 1);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
