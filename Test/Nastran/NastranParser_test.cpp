/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranParser_test.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#define BOOST_TEST_MODULE nastran_parser_tests
#include "../../Nastran/NastranParser.h"
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

BOOST_AUTO_TEST_CASE( test_model_read ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/nastran/alneos/test4a/test4a.dat").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
				ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
		int num_materials = model->materials.size();
		BOOST_CHECK_EQUAL(1, num_materials);
#if defined VDEBUG && defined __GNUC_
		VALGRIND_CHECK_VALUE_IS_DEFINED(model);
#endif
		//expected 1 material elastic
		if (num_materials == 1) {
			shared_ptr<Material> mat = model->materials.first();
			BOOST_CHECK(mat->findNature(Nature::NatureType::NATURE_ELASTIC) != nullptr);
		}
		//BOOST_CHECK_EQUAL(1, model.analyses.size());
	} catch (exception& e) {
		cout << e.what() << endl;
		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE( test_include ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/include.dat").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
				ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
		int num_materials = model->materials.size();
		BOOST_CHECK_EQUAL(2, num_materials);
	} catch (exception& e) {
		cout << e.what() << endl;
		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
	//expected 1 material elastic
}

BOOST_AUTO_TEST_CASE(test_comments_in_the_end) {
	//a short version of Optistruct test, that fails in windows
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/comments.dat").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
	//expected 1 material elastic
}

BOOST_AUTO_TEST_CASE(nastran_github_issue15) {
	// https://github.com/Alneos/vega/issues/15
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/github_issue15.nas").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
    BOOST_CHECK_EQUAL(model->analyses.size(), 0);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
	//expected 1 material elastic
}

BOOST_AUTO_TEST_CASE(nastran_pload4d) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/pload4d.nas").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 2);
        const auto& analysis = model->analyses.first();
        BOOST_CHECK_EQUAL(analysis->getConstraintSets().size(), 1);
        BOOST_CHECK_EQUAL(analysis->getLoadSets().size(), 1);
        BOOST_CHECK_EQUAL(model->loadings.size(), 2);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE( test_issue24_multilineset ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/multilineset.nas").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
				ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        BOOST_CHECK_EQUAL(model->mesh.getNodeGroups().size(), 1);
        BOOST_CHECK_EQUAL(model->mesh.getNodeGroups()[0]->nodePositions().size(), 16);
	} catch (exception& e) {
		cout << e.what() << endl;
		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE(nastran_issue23_doubleload) {
	string testLocation = fs::path(
		PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/doubleload.nas").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const unique_ptr<Model> model = parser.parse(
			ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "", ""});
        model->finish();
        BOOST_CHECK_EQUAL(model->analyses.size(), 2);
        const auto& analysis = model->analyses.first();
        BOOST_CHECK_EQUAL(analysis->getConstraintSets().size(), 1);
        BOOST_CHECK_EQUAL(analysis->getLoadSets().size(), 1);
        BOOST_CHECK_EQUAL(model->loadings.size(), 2);
        const auto& analysis2 = model->analyses.last();
        BOOST_CHECK_EQUAL(analysis2->getConstraintSets().size(), 1);
        BOOST_CHECK_EQUAL(analysis2->getLoadSets().size(), 1);
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
