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
#include "../../Systus/SystusWriter.h"
#include "../../Systus/SystusRunner.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
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
    string solverVersion = "";
    fs::path inputFpath = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern";
    map<SolverName, unique_ptr<Writer>> writerBySolverName;
    writerBySolverName[SolverName::CODE_ASTER] = make_unique<aster::AsterWriter>();
    writerBySolverName[SolverName::SYSTUS] = make_unique<systus::SystusWriter>();
    writerBySolverName[SolverName::NASTRAN] = make_unique<nastran::NastranWriter>();
    map<SolverName, unique_ptr<Runner>> runnerBySolverName;
    runnerBySolverName[SolverName::CODE_ASTER] = make_unique<aster::AsterRunner>();
    runnerBySolverName[SolverName::SYSTUS] = make_unique<systus::SystusRunner>();
    map<SolverName, bool> canrunBySolverName = {
        {SolverName::CODE_ASTER, RUN_ASTER},
        {SolverName::SYSTUS, RUN_SYSTUS},
        {SolverName::NASTRAN, false}, // cosmic cannot handle PLOAD4 on volume cells
    };
    const map<CellType, string>& meshByCellType = {
        {CellType::TETRA4, "MeshTetraLin"},
        {CellType::PYRA5, "MeshPyraLin"},
        {CellType::PENTA6, "MeshPentaLin"},
        {CellType::HEXA8, "MeshHexaLin"},
        {CellType::TETRA10, "MeshTetraQuad"},
        {CellType::PYRA13, "MeshPyraQuad"},
        {CellType::PENTA15, "MeshPentaQuad"},
        {CellType::HEXA20, "MeshHexaQuad"},
    };
    const set<Loading::Type> loadingTypes = {
        Loading::Type::FORCE_SURFACE,
        Loading::Type::NORMAL_PRESSION_FACE,
        Loading::Type::NODAL_FORCE,
    };
	try {
	    for (const auto& writerEntry : writerBySolverName) {
            const SolverName& solverName = writerEntry.first;
            const Solver& solver{solverName};
            const auto& writer = writerEntry.second;
            for (const auto& cellMeshEntry : meshByCellType) {
                const CellType& cellType = cellMeshEntry.first;
                cout << "Using mesh of " + cellType.description << endl;
                fs::path outputPath = (testOutputBase / boost::to_lower_copy(solver.to_str()) / ("test_3d_cantilever_" + cellType.description)).make_preferred();
                fs::create_directories(outputPath);
                fs::path inputFname = (inputFpath / (cellMeshEntry.second + ".bdf")).make_preferred();
                fs::path testFname = (inputFpath / (cellMeshEntry.second + ".f06")).make_preferred();

                ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), solver,
                    solverVersion, "test_3d_cantilever_" + cellType.description, outputPath.string(), LogLevel::DEBUG, translationMode, testFname.string(),
                    0.02, false, false, "", "", "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
                    nastranOutputSyntax);
                nastran::NastranParser parser;
                unique_ptr<Model> model = parser.parse(configuration);

                if (model->mesh.getCellGroups().empty()) {
                    throw logic_error("No cell group has been found in mesh, maybe BEGIN_BULK is missing?");
                }
                const auto& x0group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(6));
                if (x0group == nullptr) {
                    throw logic_error("missing constraint group in mesh");
                }
                const auto& volgroup = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(9));
                if (volgroup == nullptr) {
                    throw logic_error("missing volume group in mesh");
                }
                const auto& x300group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(8));
                if (x300group == nullptr) {
                    throw logic_error("missing loading group in mesh");
                }

                // Add constraintset
                int spcSetId = 1;
                const auto& constraintSet = make_shared<ConstraintSet>(*model, ConstraintSet::Type::SPC, spcSetId);
                model->add(constraintSet);

                // Add output
                const auto& nodalOutput = make_shared<NodalDisplacementOutput>(*model);
                for (const int nodePosition : x300group->nodePositions()) {
                    nodalOutput->addNodePosition(nodePosition);
                }
                model->add(nodalOutput);

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

                // Add Analyses
                int analysisId = 1;
                int loadSetId = 1;
                //double f = 100.0;
                double p = 0.5;
                for (const auto& loadingType : loadingTypes) {
                    switch (loadingType) {
                        case Loading::Type::FORCE_SURFACE : {
                            for (const DOF& loadDirection : DOFS::TRANSLATIONS) {
                                DOFCoefs loadingCoefs(DOFS::ALL_DOFS, 0.0);
                                loadingCoefs.setValue(loadDirection, -p);
                                const VectorialValue& force{loadingCoefs.getValue(DOF::DX), loadingCoefs.getValue(DOF::DY), loadingCoefs.getValue(DOF::DZ)};
                                const auto& analysis = make_shared<LinearMecaStat>(*model, "PLOAD4", analysisId);
                                const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                                model->add(loadSet);

                                analysis->add(*loadSet);
                                analysis->add(*constraintSet);
                                analysis->add(*nodalOutput);
                                model->add(analysis);
                                for (const Cell& surfCell : x300group->getCells()) {
                                    const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
                                    const Cell& volCell = volCellAndFacenum.first;
                                    const int faceNum = volCellAndFacenum.second;
                                    const pair<int, int> applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
                                    shared_ptr<ForceSurface> forceSurfaceTwoNodes = nullptr;
                                    if (applicationNodeIds.second == Globals::UNAVAILABLE_INT) {
                                        forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, applicationNodeIds.first,
                                            force, VectorialValue(0.0, 0.0, 0.0));
                                    } else {
                                        forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, applicationNodeIds.first, applicationNodeIds.second,
                                            force, VectorialValue(0.0, 0.0, 0.0));
                                    }
                                    forceSurfaceTwoNodes->add(volCell);
                                    model->add(forceSurfaceTwoNodes);
                                    model->addLoadingIntoLoadSet(*forceSurfaceTwoNodes, *loadSet);
                                }
                                loadSetId++;
                                analysisId++;
                            }
                            break;
                        }
                        case Loading::Type::NORMAL_PRESSION_FACE : {
                            const auto& analysis = make_shared<LinearMecaStat>(*model, "PLOAD2", analysisId);
                            const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                            model->add(loadSet);

                            analysis->add(*loadSet);
                            analysis->add(*constraintSet);
                            analysis->add(*nodalOutput);
                            model->add(analysis);
                            for (const Cell& surfCell : x300group->getCells()) {
                                const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
                                const Cell& volCell = volCellAndFacenum.first;
                                const int faceNum = volCellAndFacenum.second;
                                const pair<int, int> applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
                                shared_ptr<NormalPressionFace> pressionFace = nullptr;
                                if (applicationNodeIds.second == Globals::UNAVAILABLE_INT) {
                                    pressionFace = make_shared<NormalPressionFaceTwoNodes>(*model, applicationNodeIds.first, p);
                                } else {
                                    pressionFace = make_shared<NormalPressionFaceTwoNodes>(*model, applicationNodeIds.first, applicationNodeIds.second, p);
                                }
                                pressionFace->add(volCell);
                                model->add(pressionFace);
                                model->addLoadingIntoLoadSet(*pressionFace, *loadSet);
                            }
                            loadSetId++;
                            analysisId++;
                            break;
                        }
                        case Loading::Type::NODAL_FORCE : {
                            const auto& analysis = make_shared<LinearMecaStat>(*model, "coverage", analysisId);
                            const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                            model->add(loadSet);

                            analysis->add(*loadSet);
                            analysis->add(*constraintSet);
                            analysis->add(*nodalOutput);
                            model->add(analysis);

                            for (const Cell& surfCell : x300group->getCells()) {
                                const auto& cornerNodeIds = surfCell.cornerNodeIds();
                                shared_ptr<Loading> staticPressure = nullptr;
                                switch (cornerNodeIds.size()) {
                                    case 3: {
                                        staticPressure = make_shared<StaticPressure>(*model, cornerNodeIds[0], cornerNodeIds[1], cornerNodeIds[2], Globals::UNAVAILABLE_INT, p);
                                        break;
                                    }
                                    case 4: {
                                        staticPressure = make_shared<StaticPressure>(*model, cornerNodeIds[0], cornerNodeIds[1], cornerNodeIds[2], cornerNodeIds[3], p);
                                        break;
                                    }
                                    default: {
                                        throw logic_error("Cannot apply NODAL_FORCE with given node ids");
                                    }
                                }

                                model->add(staticPressure);
                                model->addLoadingIntoLoadSet(*staticPressure, *loadSet);
                            }
                            loadSetId++;
                            analysisId++;
                            break;
                        }
                        default:
                            throw logic_error("Loading type not yet implemented");
                    }

                }

                model->finish();
                fs::path modelFile = fs::path(writer->writeModel(*model, configuration));
                const auto& runnerIt = runnerBySolverName.find(solverName);
                if (canrunBySolverName[solverName] and runnerIt != runnerBySolverName.end()) {
                    Runner::ExitCode exitCode = runnerIt->second->execSolver(configuration, modelFile.string());
                    BOOST_CHECK_EQUAL(static_cast<int>(exitCode), static_cast<int>(Runner::ExitCode::OK));
                }
            }
	    }
	} catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
