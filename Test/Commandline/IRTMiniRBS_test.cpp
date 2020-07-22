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
#include "../../Optistruct/OptistructParser.h"
#include "../../Nastran/NastranWriter.h"

#include "build_properties.h"
#include "../Commandline/CommandLineUtils.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
//#include <thread>         // std::this_thread::sleep_for
//#include <chrono>         // std::chrono::seconds
#if VALGRIND_FOUND && defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

using namespace std;
namespace vega {
namespace tests {

//____________________________________________________________________________//


BOOST_AUTO_TEST_CASE( test_3d_cantilever ) {
    string solverVersion = "";
    fs::path inputFpath = fs::path(PROJECT_BASE_DIR "/testdata/nastran/irt/minirbs");
    fs::path testOutputBase = fs::path(PROJECT_BINARY_DIR "/Testing/minirbs");
	ConfigurationParameters::TranslationMode translationMode = ConfigurationParameters::TranslationMode::MODE_STRICT;
    string nastranOutputSyntax = "modern"; // cosmic cannot handle PLOAD4 on volume cells

	try {
        const auto& nastranWriter = make_unique<nastran::NastranWriter>();
        fs::path outputPath = (testOutputBase / "minirbs").make_preferred();
        fs::create_directories(outputPath);
        fs::path inputFname = (inputFpath / "minirbs.bdf").make_preferred();
        fs::path testFname = (inputFpath / ("minirbs.f06")).make_preferred();

        ConfigurationParameters configuration = ConfigurationParameters(inputFname.string(), SolverName::NASTRAN,
            solverVersion, "minirbs", outputPath.string(), LogLevel::DEBUG, translationMode, testFname.string(),
            0.02, false, false, "", "", false, "lagrangian", 0.0, 0.0, "auto", "systus", {}, "table", 9, "direct",
            "modern");
        optistruct::OptistructParser parser;
        unique_ptr<Model> model = parser.parse(configuration);
        BOOST_TEST_CHECKPOINT("input parsed");

        if (model->mesh.getCellGroups().empty()) {
            throw logic_error("No cell group has been found in mesh, maybe BEGIN_BULK is missing?");
        }

        int spcSetId = 1;
        const auto& spcSet = make_shared<ConstraintSet>(*model, ConstraintSet::Type::SPC, spcSetId);
        model->add(spcSet);

        const auto& group45 = model->mesh.findGroup(45);
        if (group45 == nullptr) {
            throw logic_error("missing constraint group in mesh");
        }

        const auto& spc = make_shared<SinglePointConstraint>(*model, DOFS::ALL_DOFS, 0.0);
        spc->addCellGroup(group45->getName());
        model->add(spc);
        model->addConstraintIntoConstraintSet(*spc, *spcSet);

        const auto& group48 = model->mesh.findGroup(48);
        if (group48 == nullptr) {
            throw logic_error("missing constraint group in mesh");
        }

        const auto& spcdx = make_shared<SinglePointConstraint>(*model, DOF::DX, 0.0);
        spcdx->addCellGroup(group48->getName());
        model->add(spcdx);
        model->addConstraintIntoConstraintSet(*spcdx, *spcSet);

        const int ploadGroupId = 3;
        const auto& ploadGroup = model->mesh.findGroup(ploadGroupId);
        if (ploadGroup == nullptr) {
            throw logic_error("missing pload group in mesh");
        }

        const VectorialValue& force{0.0, 0.0, 1.547};
        const auto& analysis = model->analyses.first();
        const auto& loadSet = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, 1);
        model->add(loadSet);

        analysis->add(*loadSet);
        BOOST_TEST_CHECKPOINT("loadset added");

        for (const Cell& surfCell : static_pointer_cast<CellGroup>(ploadGroup)->getCells()) {
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

        BOOST_TEST_CHECKPOINT("force surface created");

        const int contactSlaveGroupId = 4;
        const auto& contactSlaveGroup = static_pointer_cast<CellGroup>(model->mesh.findGroup(contactSlaveGroupId));
        if (contactSlaveGroup == nullptr) {
            throw logic_error("missing contact slave group in mesh");
        }

        const int contactMasterGroupId = 58;
        const auto& contactMasterGroup = static_pointer_cast<CellGroup>(model->mesh.findGroup(contactMasterGroupId));
        if (contactMasterGroup == nullptr) {
            throw logic_error("missing contact master group in mesh");
        }
        //cout << "Group: " << contactMasterGroupId << " found with : " << contactMasterGroup->cellPositions().size() << " cells" << endl;

        const auto& constraintSet = make_shared<ConstraintSet>(*model, ConstraintSet::Type::CONTACT, 1);
        model->add(constraintSet);
        analysis->add(*constraintSet);
        BOOST_TEST_CHECKPOINT("constraintset added");
        list<BoundaryElementFace::ElementFaceByTwoNodes> slaveFaceInfos;
        for (const Cell& surfCell : contactSlaveGroup->getCells()) {
            const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
            const Cell& volCell = volCellAndFacenum.first;
            const int faceNum = volCellAndFacenum.second;
            const pair<int, int>& applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
            BoundaryElementFace::ElementFaceByTwoNodes faceInfo(volCell.id, applicationNodeIds.first, applicationNodeIds.second);
            slaveFaceInfos.push_back(faceInfo);
        }
        const auto& befSlave = make_shared<BoundaryElementFace>(*model, slaveFaceInfos, 2);
        befSlave->surfaceCellGroup = contactSlaveGroup;
        model->add(befSlave);

        list<BoundaryElementFace::ElementFaceByTwoNodes> masterFaceInfos;
        for (const Cell& surfCell : contactMasterGroup->getCells()) {
            const auto volCellAndFacenum = model->mesh.volcellAndFaceNum_from_skincell(surfCell);
            const Cell& volCell = volCellAndFacenum.first;
            const int faceNum = volCellAndFacenum.second;
            const pair<int, int>& applicationNodeIds = volCell.two_nodeids_from_facenum(faceNum);
            BoundaryElementFace::ElementFaceByTwoNodes faceInfo(volCell.id, applicationNodeIds.first, applicationNodeIds.second);
            masterFaceInfos.push_back(faceInfo);
        }
        const auto& befMaster = make_shared<BoundaryElementFace>(*model, masterFaceInfos, 3);
        befMaster->surfaceCellGroup = contactMasterGroup;
        model->add(befMaster);

        const auto& surface = make_shared<SurfaceSlide>(*model, befSlave->getReference(), befMaster->getReference());
        model->add(surface);
        //model.addConstraintIntoConstraintSet(surface, constraintSetReference);
        model->addConstraintIntoConstraintSet(*surface, *constraintSet);

        BOOST_TEST_CHECKPOINT("surfaceslide added");

        model->finish();

        BOOST_TEST_CHECKPOINT("model finish() completed");

        fs::path modelFile = fs::path(nastranWriter->writeModel(*model, configuration));
        BOOST_TEST_CHECKPOINT("output written");
        if (fs::exists(testFname)) {
            fs::copy_file(testFname, modelFile.parent_path() / (modelFile.stem().string() + ".f06"), fs::copy_option::overwrite_if_exists);
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
