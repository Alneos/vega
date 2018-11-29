/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranParser_test.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#define BOOST_TEST_MODULE medwriter_tests
#include "../../Nastran/NastranParser.h"
#include "../../Aster/MedWriter.h"
#include "build_properties.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#if defined VDEBUG && defined __GNUC_ && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

using namespace std;
using namespace vega;
using namespace aster;

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_medwriter_spc ) {
	Model model("inputfile", "10.3", SolverName::NASTRAN);
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 0.0 };
	for (int i = 0; i < 18; i += 3) {
		model.mesh->addNode(i, coords[i], coords[i + 1], coords[i + 2]);
	}
	model.mesh->addCell(27, CellType::SEG2, {0, 1});
	model.mesh->addCell(28, CellType::SEG2, {1, 2});
	model.mesh->addCell(29, CellType::SEG2, {2, 3});
	model.mesh->addCell(30, CellType::SEG2, {3, 4});
	model.mesh->addCell(31, CellType::POINT1, {1});
	shared_ptr<vega::NodeGroup> gn1 = model.mesh->findOrCreateNodeGroup("GN1");
	gn1->addNodeId(0);
	gn1->addNodeId(6);
	shared_ptr<vega::CellGroup> gm1 = model.mesh->createCellGroup("GM1");
	gm1->addCellId(31);
	shared_ptr<vega::CellGroup> gm2 = model.mesh->createCellGroup("GM2");
	gm2->addCellId(28);
	gm2->addCellId(30);
	//SinglePointConstraint spc1 = SinglePointConstraint(model, true, true, true, false, false, false,
	//		0.0, gn1);
	SinglePointConstraint spc1 = SinglePointConstraint(model,array<ValueOrReference, 3>{{ 0, 0, 0 }}, gn1);
	model.add(spc1);
	model.finish();

	shared_ptr<SinglePointConstraint> spc1_ptr = dynamic_pointer_cast<SinglePointConstraint>(
			model.find(Reference<Constraint>(spc1)));
	BOOST_CHECK(spc1_ptr);
	DOFS spc_dofs = spc1_ptr->getDOFSForNode(0);
	BOOST_CHECK(spc1_ptr->hasReferences() == false);
	BOOST_CHECK(spc_dofs == DOFS::TRANSLATIONS);
	BOOST_CHECK(spc1_ptr->nodePositions().size() == 2);
	string outfname = PROJECT_BINARY_DIR "/Testing/file1.med";
	MedWriter medWriter;
	medWriter.writeMED(model, outfname.c_str());
	BOOST_CHECK(fs::exists(outfname.c_str()));
}

BOOST_AUTO_TEST_CASE( nastran_med_write ) {
	string testLocation = fs::path(
	PROJECT_BASE_DIR "/testdata/unitTest/nastranparser/test1.nas").make_preferred().string();
	string outFile = fs::path(PROJECT_BINARY_DIR "/bin/test1.med").make_preferred().string();
	nastran::NastranParser parser;
    try {
        const shared_ptr<Model> model = parser.parse(
                ConfigurationParameters(testLocation, SolverName::CODE_ASTER, string("1")));
        MedWriter medWriter;
        medWriter.writeMED(*model, outFile.c_str());

        BOOST_CHECK(boost::filesystem::exists(outFile));
    } catch (exception& e) {
		cout << e.what() << endl;
		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

//____________________________________________________________________________//
