/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranParser_test.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#define BOOST_TEST_MODULE nastran_coverage_tests
#include "../../Nastran/NastranParser.h"
#include "../../Nastran/NastranWriter.h"
#include "../../Nastran/NastranRunner.h"
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

BOOST_AUTO_TEST_CASE( test_seg2 ) {
    Solver solver(SolverName::NASTRAN);
    string solverVersion = "";
    fs::path inputFname = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage/MeshSegLin.bdf");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
    fs::path outputPath = (testOutputBase / "test_seg2").make_preferred();
    fs::create_directories(outputPath);
    string testLocation = inputFname.make_preferred().string();
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
	nastran::NastranParser parser;
	nastran::NastranWriter writer;
	nastran::NastranRunner runner;
	try {
        ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), solver,
            solverVersion, "test_seg2", outputPath.string(), LogLevel::DEBUG, translationMode);
		const unique_ptr<Model> model = parser.parse(configuration);
        model->finish();
        fs::path modelFile = outputPath / fs::path(writer.writeModel(*model, configuration));
        runner.execSolver(configuration, modelFile.string());
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE( test_3d_cantilever_hexa ) {
    Solver solver(SolverName::NASTRAN);
    string solverVersion = "";
    fs::path inputFname = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage/MeshHexaLin.bdf");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
    fs::path outputPath = (testOutputBase / "test_3d_cantilever_hexa").make_preferred();
    fs::create_directories(outputPath);
    string testLocation = inputFname.make_preferred().string();
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern";
	nastran::NastranParser parser;
	nastran::NastranWriter writer;
	try {
        ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), solver,
            solverVersion, "test_3d_cantilever_hexa", outputPath.string(), LogLevel::DEBUG, translationMode, "",
            0.02, false, false, "", "", "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
            nastranOutputSyntax);
		const unique_ptr<Model> model = parser.parse(configuration);
        model->finish();
        fs::path modelFile = outputPath / fs::path(writer.writeModel(*model, configuration));
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE( test_3d_cantilever_tetra ) {
    Solver solver(SolverName::NASTRAN);
    string solverVersion = "";
    fs::path inputFname = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage/MeshTetraLin.bdf");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
    fs::path outputPath = (testOutputBase / "test_3d_cantilever_tetra").make_preferred();
    fs::create_directories(outputPath);
    string testLocation = inputFname.make_preferred().string();
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern";
	nastran::NastranParser parser;
	nastran::NastranWriter writer;
	try {
        ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), solver,
            solverVersion, "test_3d_cantilever_tetra", outputPath.string(), LogLevel::DEBUG, translationMode, "",
            0.02, false, false, "", "", "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
            nastranOutputSyntax);
		const unique_ptr<Model> model = parser.parse(configuration);
        model->finish();
        fs::path modelFile = outputPath / fs::path(writer.writeModel(*model, configuration));
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}


//____________________________________________________________________________//
