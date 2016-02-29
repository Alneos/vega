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
#include "../../Nastran/NastranFacade.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#if defined VDEBUG && defined __GNUC_
#include <valgrind/memcheck.h>
#endif

using namespace std;
using namespace vega;

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( nastran_med_write ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/test1.nas").make_preferred().string();
	string outFile = fs::path(PROJECT_BINARY_DIR "/bin/test1.med").make_preferred().string();
	nastran::NastranParser parser;

	const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, CODE_ASTER, string("1")));
	model->mesh->writeMED(outFile.c_str());

	BOOST_CHECK(boost::filesystem::exists(outFile));
}

BOOST_AUTO_TEST_CASE( test_model_read ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/nastran/linstat/bar1n/bar1n.dat").make_preferred().string();
	nastran::NastranParser parser;
	try {
		const shared_ptr<Model> model = parser.parse(
				ConfigurationParameters(testLocation, CODE_ASTER, "", ""));
		int num_materials = model->materials.size();
		BOOST_CHECK_EQUAL(1, num_materials);
#if defined VDEBUG && defined __GNUC_
		VALGRIND_CHECK_VALUE_IS_DEFINED(model);
#endif
		//expected 1 material elastic
		if (num_materials == 1) {
			shared_ptr<Material> mat = *(model->materials.begin());
			BOOST_CHECK(mat->findNature(Nature::NATURE_ELASTIC));
		}
		BOOST_CHECK_EQUAL(1, model->analyses.size());
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
		const shared_ptr<Model> model = parser.parse(
				ConfigurationParameters(testLocation, CODE_ASTER, "", ""));
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
		const shared_ptr<Model> model = parser.parse(
			ConfigurationParameters(testLocation, CODE_ASTER, "", ""));
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
	//expected 1 material elastic
}
//____________________________________________________________________________//
