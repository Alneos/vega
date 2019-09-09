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
 *  Created on: Aug, 2019
 *      Author: Luca Dall'Olio
 */

#define BOOST_TEST_MODULE nastran_coverage_tests
#include "../../Nastran/NastranParser.h"
#include "../../Nastran/NastranWriter.h"

#include "build_properties.h"
#include "../Commandline/CommandLineUtils.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
//#include <thread>         // std::this_thread::sleep_for
//#include <chrono>         // std::chrono::seconds
#if defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

using namespace std;
namespace vega {
namespace tests {

//____________________________________________________________________________//


BOOST_AUTO_TEST_CASE( test_3d_cantilever ) {
    string solverVersion = "";
    fs::path inputFpath = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/coverage");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/nastrancoverage");
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern"; // cosmic cannot handle PLOAD4 on volume cells
    map<SolverName, bool> canrunBySolverName = {
        {SolverName::CODE_ASTER, RUN_ASTER},
//        {SolverName::SYSTUS, RUN_SYSTUS},
        {SolverName::NASTRAN, false}, // cosmic cannot handle PLOAD4 on volume cells
    };
    const map<CellType, string>& meshByCellType = {
        {CellType::SEG2, "MeshSegLin"},
        {CellType::TRI3, "MeshTriaLin"},
        {CellType::QUAD4, "MeshQuadLin"},
        {CellType::TETRA4, "MeshTetraLin"},
        {CellType::PYRA5, "MeshPyraLin"},
        {CellType::PENTA6, "MeshPentaLin"},
        {CellType::HEXA8, "MeshHexaLin"},
        {CellType::TETRA10, "MeshTetraQuad"},
        {CellType::PYRA13, "MeshPyraQuad"},
        {CellType::PENTA15, "MeshPentaQuad"},
        {CellType::HEXA20, "MeshHexaQuad"},
    };
    enum class LoadingTest {
        FORCE_NODALE,
        FORCE_BEAM,
        FORCE_SURFACE,
        NORMAL_PRESSION_SHELL,
        NORMAL_PRESSION_FACE,
        STATIC_PRESSURE
    };
    const map<SpaceDimension, vector<LoadingTest>> loadingTestsBySpaceDimension = {
        {SpaceDimension::DIMENSION_1D, {
                                        LoadingTest::FORCE_NODALE,
                                        LoadingTest::FORCE_BEAM,
                                       }},
        {SpaceDimension::DIMENSION_2D, {
                                        //LoadingTest::FORCE_NODALE, // NOOK 32% on Quad FY ?
                                        LoadingTest::NORMAL_PRESSION_SHELL,
                                       }},
        {SpaceDimension::DIMENSION_3D, {
                                        LoadingTest::FORCE_SURFACE,
                                        LoadingTest::NORMAL_PRESSION_FACE,
                                        LoadingTest::STATIC_PRESSURE,
                                        LoadingTest::FORCE_NODALE,
                                       }},
    };
	try {
        const Solver& nastranSolver{SolverName::NASTRAN};
        const auto& nastranWriter = make_unique<nastran::NastranWriter>();
        for (const auto& cellMeshEntry : meshByCellType) {
            const CellType& cellType = cellMeshEntry.first;
            cout << "Using mesh of " + cellType.description << endl;
            fs::path outputPath = (testOutputBase / boost::to_lower_copy(nastranSolver.to_str()) / ("test_3d_cantilever_" + cellType.description)).make_preferred();
            fs::create_directories(outputPath);
            fs::path inputFname = (inputFpath / (cellMeshEntry.second + ".bdf")).make_preferred();
            fs::path testFname = (inputFpath / (cellMeshEntry.second + ".f06")).make_preferred();

            ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), nastranSolver,
                solverVersion, "test_3d_cantilever_" + cellType.description, outputPath.string(), LogLevel::DEBUG, translationMode, testFname.string(),
                0.02, false, false, "", "", "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
                nastranOutputSyntax);
            nastran::NastranParser parser;
            unique_ptr<Model> model = parser.parse(configuration);

            if (model->mesh.getCellGroups().empty()) {
                throw logic_error("No cell group has been found in mesh, maybe BEGIN_BULK is missing?");
            }
            const int constraintGroupId = 6;
            shared_ptr<Group> x0group = nullptr;
            if (cellType.dimension == SpaceDimension::DIMENSION_1D) {
                x0group = dynamic_pointer_cast<NodeGroup>(model->mesh.findGroup(constraintGroupId));
            } else {
                x0group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(constraintGroupId));
            }
            if (x0group == nullptr) {
                throw logic_error("missing constraint group in mesh");
            }

            const int bodyGroupId = 9;
            const auto& volgroup = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(bodyGroupId));
            if (volgroup == nullptr) {
                throw logic_error("missing volume group in mesh");
            }

            const int loadGroupId = 8;
            shared_ptr<Group> x300group = nullptr;
            if (cellType.dimension == SpaceDimension::DIMENSION_1D) {
                x300group = dynamic_pointer_cast<NodeGroup>(model->mesh.findGroup(loadGroupId));
            } else {
                x300group = dynamic_pointer_cast<CellGroup>(model->mesh.findGroup(loadGroupId));
            }
            if (x300group == nullptr) {
                throw logic_error("missing loading group in mesh");
            }

            // Add constraintset
            int spcSetId = 1;
            const auto& constraintSet = make_shared<ConstraintSet>(*model, ConstraintSet::Type::SPC, spcSetId);
            model->add(constraintSet);

            // Add output
            const auto& nodalOutput = make_shared<NodalDisplacementOutput>(*model);
            nodalOutput->add(*x300group);
            model->add(nodalOutput);

            if (cellType.dimension > SpaceDimension::DIMENSION_1D) {
                const auto& vmisOutput = make_shared<VonMisesStressOutput>(*model);
                vmisOutput->add(*x0group);
                model->add(vmisOutput);
            }

            // Define material
            int matId = 1;
            double youngModulus = 200000;
            double poissonNumber = 0.3;
            shared_ptr<Material> material = model->getOrCreateMaterial(matId);
            material->addNature(make_shared<ElasticNature>(*model, youngModulus, poissonNumber));

            // Assign material and model
            double beamWidth = 10;
            double beamHeight = 20;
            shared_ptr<ElementSet> elementSet = nullptr;
            switch(cellType.dimension.code) {
            case SpaceDimension::Code::DIMENSION1D_CODE: {
                elementSet = make_shared<RectangularSectionBeam>(*model, beamWidth, beamHeight, Beam::BeamModel::TIMOSHENKO, 0.0, volgroup->bestId());
                break;
            }
            case SpaceDimension::Code::DIMENSION2D_CODE: {
                elementSet = make_shared<Shell>(*model, beamWidth, 0.0, volgroup->bestId());
                break;
            }
            case SpaceDimension::Code::DIMENSION3D_CODE: {
                elementSet = make_shared<Continuum>(*model, ModelType::TRIDIMENSIONAL, volgroup->bestId());
                break;
            }
            default:
                throw invalid_argument("Dimension coverage not yet implemented");
            }
            elementSet->assignMaterial(matId);
            elementSet->add(*volgroup);
            model->add(elementSet);

            // Add constraint
            const auto& spc = make_shared<SinglePointConstraint>(*model, DOFS::ALL_DOFS, 0.0);
            spc->add(*x0group);
            model->add(spc);
            model->addConstraintIntoConstraintSet(*spc, *constraintSet);

            // Add Analyses
            int analysisId = 1;
            int loadSetId = 1;
            double f = 100.0;
            double p = f / (beamHeight*beamWidth);
            for (const auto& loadingTest : loadingTestsBySpaceDimension.at(cellType.dimension)) {
                switch (loadingTest) {
                    case LoadingTest::FORCE_NODALE : {
                        for (const DOF& loadDirection : DOFS::TRANSLATIONS) {
                            DOFCoefs loadingCoefs(DOFS::ALL_DOFS, 0.0);
                            double intensity;
                            switch (cellType.dimension.code) {
                            case SpaceDimension::Code::DIMENSION1D_CODE: {
                                intensity = f;
                                break;
                            }
                            case SpaceDimension::Code::DIMENSION2D_CODE: {
                                intensity = f / beamHeight;
                                break;
                            }
                            case SpaceDimension::Code::DIMENSION3D_CODE: {
                                intensity = p;
                                break;
                            }
                            default:
                                throw logic_error("Nodal force coverage on this cell type not yet implemented");
                            }
                            loadingCoefs.setValue(loadDirection, -intensity);
                            const VectorialValue& force{loadingCoefs.getValue(DOF::DX), loadingCoefs.getValue(DOF::DY), loadingCoefs.getValue(DOF::DZ)};
                            const auto& analysis = make_shared<LinearMecaStat>(*model, "FORCE", analysisId);
                            const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                            model->add(loadSet);
                            analysis->add(*loadSet);
                            analysis->add(*constraintSet);
                            analysis->add(*nodalOutput);
                            model->add(analysis);
                            const auto& forceLoading = make_shared<NodalForce>(*model, loadSet, force);
                            forceLoading->add(*x300group);
                            model->add(forceLoading);
                            loadSetId++;
                            analysisId++;
                        }
                        break;
                    }
                    case LoadingTest::FORCE_SURFACE : {
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
                            for (const Cell& surfCell : dynamic_pointer_cast<CellGroup>(x300group)->getCells()) {
                                const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
                                const Cell& volCell = volCellAndFacenum.first;
                                const int faceNum = volCellAndFacenum.second;
                                const pair<int, int> applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
                                shared_ptr<ForceSurface> forceSurfaceTwoNodes = nullptr;
                                if (applicationNodeIds.second == Globals::UNAVAILABLE_INT) {
                                    forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, loadSet, applicationNodeIds.first,
                                        force);
                                } else {
                                    forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, loadSet, applicationNodeIds.first, applicationNodeIds.second,
                                        force);
                                }
                                forceSurfaceTwoNodes->add(volCell);
                                model->add(forceSurfaceTwoNodes);
                            }
                            loadSetId++;
                            analysisId++;
                        }
                        break;
                    }
                    case LoadingTest::FORCE_BEAM : {
                        const auto& analysis = make_shared<LinearMecaStat>(*model, "PLOAD1", analysisId);
                        const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                        for (const DOF& loadDirection : DOFS::ALL_DOFS) {
                            const auto& force = make_shared<FunctionTable>(*model, FunctionTable::Interpolation::LINEAR, FunctionTable::Interpolation::LINEAR, FunctionTable::Interpolation::NONE, FunctionTable::Interpolation::NONE);
                            force->setParaX(FunctionTable::ParaName::PARAX);
                            force->setXY(0.0, -f);
                            force->setXY(1.0, -f);
                            const auto& forceLine = make_shared<ForceLine>(*model, loadSet, force, loadDirection);
                            forceLine->add(*volgroup);
                            model->add(forceLine);
                        }
                        analysis->add(*loadSet);
                        analysis->add(*constraintSet);
                        analysis->add(*nodalOutput);
                        model->add(analysis);
                        loadSetId++;
                        analysisId++;
                        break;
                    }
                    case LoadingTest::NORMAL_PRESSION_SHELL : {
                        const auto& analysis = make_shared<LinearMecaStat>(*model, "PLOAD2", analysisId);
                        const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                        model->add(loadSet);

                        analysis->add(*loadSet);
                        analysis->add(*constraintSet);
                        analysis->add(*nodalOutput);
                        model->add(analysis);
                        shared_ptr<NormalPressionFace> pressionFace = make_shared<NormalPressionFace>(*model, loadSet, p);
                        pressionFace->add(*volgroup);
                        model->add(pressionFace);
                        loadSetId++;
                        analysisId++;
                        break;
                    }
                    case LoadingTest::NORMAL_PRESSION_FACE : {
                        const auto& analysis = make_shared<LinearMecaStat>(*model, "PLOAD4", analysisId);
                        const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                        model->add(loadSet);

                        analysis->add(*loadSet);
                        analysis->add(*constraintSet);
                        analysis->add(*nodalOutput);
                        model->add(analysis);
                        for (const Cell& surfCell : dynamic_pointer_cast<CellGroup>(x300group)->getCells()) {
                            const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
                            const Cell& volCell = volCellAndFacenum.first;
                            const int faceNum = volCellAndFacenum.second;
                            const pair<int, int> applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
                            shared_ptr<NormalPressionFace> pressionFace = nullptr;
                            if (applicationNodeIds.second == Globals::UNAVAILABLE_INT) {
                                pressionFace = make_shared<NormalPressionFaceTwoNodes>(*model, loadSet, applicationNodeIds.first, p);
                            } else {
                                pressionFace = make_shared<NormalPressionFaceTwoNodes>(*model, loadSet, applicationNodeIds.first, applicationNodeIds.second, p);
                            }
                            pressionFace->add(volCell);
                            model->add(pressionFace);
                        }
                        loadSetId++;
                        analysisId++;
                        break;
                    }
                    case LoadingTest::STATIC_PRESSURE : {
                        const auto& analysis = make_shared<LinearMecaStat>(*model, "coverage", analysisId);
                        const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, loadSetId);
                        model->add(loadSet);

                        analysis->add(*loadSet);
                        analysis->add(*constraintSet);
                        analysis->add(*nodalOutput);
                        model->add(analysis);

                        for (const Cell& surfCell : dynamic_pointer_cast<CellGroup>(x300group)->getCells()) {
                            const auto& cornerNodeIds = surfCell.cornerNodeIds();
                            shared_ptr<Loading> staticPressure = nullptr;
                            // LD careful here: PLOAD2 only works on shells and only on linear cells!
                            switch (cornerNodeIds.size()) {
                                case 3: {
                                    staticPressure = make_shared<StaticPressure>(*model, loadSet, cornerNodeIds[0], cornerNodeIds[1], cornerNodeIds[2], Globals::UNAVAILABLE_INT, p);
                                    break;
                                }
                                case 4: {
                                    staticPressure = make_shared<StaticPressure>(*model, loadSet, cornerNodeIds[0], cornerNodeIds[1], cornerNodeIds[2], cornerNodeIds[3], p);
                                    break;
                                }
                                default: {
                                    throw logic_error("Cannot apply NODAL_FORCE with given node ids");
                                }
                            }

                            model->add(staticPressure);
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
            fs::path modelFile = fs::path(nastranWriter->writeModel(*model, configuration));
            if (fs::exists(testFname)) {
                fs::copy_file(testFname, modelFile.parent_path() / (modelFile.stem().string() + ".f06"), fs::copy_option::overwrite_if_exists);
            }
            for (const auto& canRunEntry : canrunBySolverName) {
                const SolverName& solverName = canRunEntry.first;
                if (cellType == CellType::SEG2 and solverName == SolverName::SYSTUS) {
                    continue; // FORCE_BEAM not yet translated
                }
                CommandLineUtils::run(modelFile.string(), nastranSolver.getSolverName(), solverName, canRunEntry.second, true, 0.005, nastranOutputSyntax=="cosmic95");
                //std::this_thread::sleep_for (std::chrono::seconds(1));
            }
        }
	} catch (exception& e) {
		cerr << e.what() << endl;
		BOOST_TEST_MESSAGE(string("Application exception") + e.what());

		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}


} /* namespace test */
} /* namespace vega */
//____________________________________________________________________________//
