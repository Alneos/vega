/*
 * Copyright (C) IRT Systemx (luca.dallolio@ext.irt-systemx.fr)
 * This file is part of Vega.
 *
 *   Vega is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Vega is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Vega.  If not, see <http://www.gnu.org/licenses/>.
 *
 * NastranParser.cpp
 *
 *  Created on: Jul 5, 2018
 *      Author: Luca Dall'Olio
 */

#define BOOST_TEST_MODULE optistruct_parser_tests
#include "../../Optistruct/OptistructParser.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#if VALGRIND_FOUND && defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
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
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        BOOST_CHECK_EQUAL(model->targets.size(), 2);
        BOOST_CHECK_EQUAL(model->constraints.size(), 1);
        //auto& master = model->find(Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, 1));
        //BOOST_CHECK_NE(master, nullptr);
        //auto& slave = model->find(Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, 2));
        //BOOST_CHECK_NE(slave, nullptr);
        model->finish();
        //BOOST_CHECK((model->analyses.first())->type == Analysis::Type::NONLINEAR_MECA_STAT);
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
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        BOOST_CHECK_EQUAL(model->targets.size(), 2);
        BOOST_CHECK_EQUAL(model->constraints.size(), 1);
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        model->finish();
        //BOOST_CHECK((*model.analyses.begin())->type == Analysis::Type::NONLINEAR_MECA_STAT);
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
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        const auto& analysis = model->analyses.first();
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
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 1);
        const auto& analysis = model->analyses.first();
        BOOST_CHECK(analysis->isLinear());
        BOOST_CHECK_EQUAL(analysis->getConstraintSets().size(), 1);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_pbushrigid) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/pbushrigid.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        BOOST_CHECK_EQUAL(model->elementSets.size(), 1);
        BOOST_CHECK_EQUAL(model->constraintSets.size(), 0);
        model->finish();
        BOOST_CHECK_EQUAL(model->elementSets.size(), 1);
        BOOST_CHECK_EQUAL(model->constraintSets.size(), 1);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_twosubcases) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/twosubcases.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        BOOST_CHECK_EQUAL(model->constraintSets.size(), 3);
        model->finish();
        BOOST_CHECK_EQUAL(model->constraintSets.size(), 3);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(optistruct_multilineset) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/optistructparser/multilineset.nas").make_preferred().string();
	optistruct::OptistructParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        const auto& setGroup = dynamic_pointer_cast<NodeGroup>(model->mesh.findGroup("SET_5"));
        const auto& nodeIds = setGroup->getNodeIds();
        BOOST_CHECK_EQUAL(nodeIds.size(), 7);
        model->finish();
        BOOST_CHECK_EQUAL(nodeIds.size(), 7);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
