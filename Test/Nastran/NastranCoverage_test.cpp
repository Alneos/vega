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

#include "../../Aster/AsterWriter.h"
#include "../../Aster/AsterRunner.h"
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

/*BOOST_AUTO_TEST_CASE( test_seg2 ) {
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
}*/

BOOST_AUTO_TEST_CASE( test_3d_cantilever ) {
    Solver solver(SolverName::NASTRAN);
    string solverVersion = "";
    fs::path inputFpath = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern";
    const map<CellType, string>& meshByCellType = {{CellType::HEXA8, "MeshHexaLin"}, {CellType::TETRA4, "MeshTetraLin"}};
	try {
	    for (const auto& cellMeshEntry : meshByCellType) {
            const CellType& cellType = cellMeshEntry.first;
            fs::path outputPath = (testOutputBase / ("test_3d_cantilever_" + cellType.description)).make_preferred();
            fs::create_directories(outputPath);
            fs::path inputFname = (inputFpath / (cellMeshEntry.second + ".bdf")).make_preferred();
            fs::path testFname = (inputFpath / (cellMeshEntry.second + ".f06")).make_preferred();

            ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), solver,
                solverVersion, "test_3d_cantilever_" + cellType.description, outputPath.string(), LogLevel::DEBUG, translationMode, testFname.string(),
                0.02, false, false, "", "", "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
                nastranOutputSyntax);
            nastran::NastranParser parser;
            unique_ptr<Model> model = parser.parse(configuration);

            const auto& x0group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(6));
            const auto& volgroup = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(9));
            const auto& x300group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(8));

            // Add constraintset
            int spcSetId = 1;
            const auto& constraintSet = make_shared<ConstraintSet>(*model, ConstraintSet::Type::SPC, spcSetId);
            model->add(constraintSet);

            // Add loadset
            int loadSetId = 1;
            const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
            model->add(loadSet);

            // Add output
            const auto& nodalOutput = make_shared<NodalDisplacementOutput>(*model);
            for (const int nodePosition : x300group->nodePositions()) {
                nodalOutput->addNodePosition(nodePosition);
            }
            model->add(nodalOutput);

            // Add Analysis
            int analysisId = 1;
            const auto& analysis = make_shared<LinearMecaStat>(*model, "coverage", analysisId);
            analysis->add(*loadSet);
            analysis->add(*constraintSet);
            analysis->add(*nodalOutput);
            model->add(analysis);

            // Define material
            int matId = 1;
            double youngModulus = 200000;
            double poissonNumber = 0.3;
            shared_ptr<Material> material = model->getOrCreateMaterial(matId);
            material->addNature(make_shared<ElasticNature>(*model, youngModulus, poissonNumber));

            // Assign material and model
            int partId = 1;
            const auto& continuum = make_shared<Continuum>(*model, ModelType::TRIDIMENSIONAL, partId);
            continuum->assignMaterial(matId);
            continuum->add(*volgroup);
            model->add(continuum);

            // Add constraint
            const auto& spc = make_shared<SinglePointConstraint>(*model, DOFS::TRANSLATIONS, 0.0, x0group);
            model->add(spc);
            model->addConstraintIntoConstraintSet(*spc, *constraintSet);

            // Add load
            for (const Cell& surfCell : x300group->getCells()) {
                const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
                const Cell& volCell = volCellAndFacenum.first;
                const int faceNum = volCellAndFacenum.second;
                const pair<int, int> applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);

                shared_ptr<ForceSurface> forceSurfaceTwoNodes = nullptr;
                if (applicationNodeIds.second == Globals::UNAVAILABLE_INT) {
                    forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, applicationNodeIds.first,
                        VectorialValue(0.0, 0.0, -0.5), VectorialValue(0.0, 0.0, 0.0));
                } else {
                    forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, applicationNodeIds.first, applicationNodeIds.second,
                        VectorialValue(0.0, 0.0, -0.5), VectorialValue(0.0, 0.0, 0.0));
                }
                forceSurfaceTwoNodes->add(volCell);
                model->add(forceSurfaceTwoNodes);
                model->addLoadingIntoLoadSet(*forceSurfaceTwoNodes, *loadSet);

            }
            model->finish();
            aster::AsterWriter asterWriter;
            fs::path modelFile = fs::path(asterWriter.writeModel(*model, configuration));
            aster::AsterRunner asterRunner;
            //Runner::ExitCode exitCode = asterRunner.execSolver(configuration, modelFile.string());
            //BOOST_CHECK_EQUAL(static_cast<int>(exitCode), static_cast<int>(Runner::ExitCode::OK));
	    }

	}
	catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
