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
	Model model{"inputfile", "10.3", SolverName::NASTRAN};
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 0.0 };
	for (int i = 0; i < 18; i += 3) {
		model.mesh.addNode(i, coords[i], coords[i + 1], coords[i + 2]);
	}
	model.mesh.addCell(27, CellType::SEG2, {0, 1});
	model.mesh.addCell(28, CellType::SEG2, {1, 2});
	model.mesh.addCell(29, CellType::SEG2, {2, 3});
	model.mesh.addCell(30, CellType::SEG2, {3, 4});
	model.mesh.addCell(31, CellType::POINT1, {1});
	shared_ptr<vega::NodeGroup> gn1 = model.mesh.findOrCreateNodeGroup("GN1");
	gn1->addNodeId(0);
	gn1->addNodeId(6);
	shared_ptr<vega::CellGroup> gm1 = model.mesh.createCellGroup("GM1");
	gm1->addCellId(31);
	shared_ptr<vega::CellGroup> gm2 = model.mesh.createCellGroup("GM2");
	gm2->addCellId(28);
	gm2->addCellId(30);
	const auto spc1 = make_shared<SinglePointConstraint>(model,array<ValueOrReference, 3>{{ 0, 0, 0 }}, gn1);
	model.add(spc1);
	model.finish();

	shared_ptr<SinglePointConstraint> spc1_ptr = dynamic_pointer_cast<SinglePointConstraint>(
			model.find(spc1->getReference()));
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
        const unique_ptr<Model> model = parser.parse(
                ConfigurationParameters{testLocation, SolverName::CODE_ASTER, "1"});
        MedWriter medWriter;
        medWriter.writeMED(*model, outFile.c_str());

        BOOST_CHECK(boost::filesystem::exists(outFile));
    } catch (exception& e) {
		cout << e.what() << endl;
		BOOST_FAIL(string("Parse threw exception ") + e.what());
	}
}

BOOST_AUTO_TEST_CASE( test_NodeGroup2Families )
{
    Mesh mesh(LogLevel::INFO, "test");
    vector<shared_ptr<NodeGroup>> nodeGroups;
    shared_ptr<NodeGroup> gn1 = mesh.findOrCreateNodeGroup("GN1");
    gn1->addNodeByPosition(0);
    gn1->addNodeByPosition(3);
    gn1->addNodeByPosition(4);
    nodeGroups.push_back(gn1);
    shared_ptr<NodeGroup> gn2 = mesh.findOrCreateNodeGroup("GN2");
    gn2->addNodeByPosition(0);
    gn2->addNodeByPosition(1);
    nodeGroups.push_back(gn2);
    NodeGroup2Families ng(5, nodeGroups);
    int expected[] = { 2, 3, 0, 1, 1, 0 };
    vector<int> result = ng.getFamilyOnNodes();
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected, expected + 5);
    vector<Family> families = ng.getFamilies();
    BOOST_CHECK_EQUAL(static_cast<size_t>(3), families.size());
    bool famGN1_GN2_found = false;
    for (Family fam : families)
    {
        famGN1_GN2_found |= fam.name == "GN1_GN2";
    }
    BOOST_CHECK(famGN1_GN2_found);
}

BOOST_AUTO_TEST_CASE( test_CellGroup2Families )
{
    Mesh mesh(LogLevel::INFO, "test");
    vector<shared_ptr<CellGroup>> cellGroups;
    shared_ptr<CellGroup> gn1 = mesh.createCellGroup("GMA1");
    mesh.addCell(1, CellType::TRI3, {1,2,3});
    mesh.addCell(2, CellType::SEG2, {1,3});
    mesh.addCell(3, CellType::TRI3, {3,4,5});
    gn1->addCellId(1);
    gn1->addCellId(2);
    gn1->addCellId(3);
    cellGroups.push_back(gn1);
    shared_ptr<CellGroup> gn2 = mesh.createCellGroup("GMA2");
    gn2->addCellId(1);
    cellGroups.push_back(gn2);
    unordered_map<CellType::Code, int, EnumClassHash> cellCountByType;
    cellCountByType[CellType::Code::SEG2_CODE] = 3;
    cellCountByType[CellType::Code::TRI3_CODE] = 3;
    CellGroup2Families cg2fam(mesh, cellCountByType, cellGroups);
    const auto& result = cg2fam.getFamilyOnCells();

    int expectedTri3[] = { -2, -1, 0 };
    const auto& tri3 = result.find(CellType::Code::TRI3_CODE)->second;
    BOOST_CHECK_EQUAL_COLLECTIONS(tri3->begin(), tri3->end(), expectedTri3,
                                  expectedTri3 + 3);

    int expectedSeg2[] = { -1, 0, 0 };
    const auto& seg2 = result.find(CellType::Code::SEG2_CODE)->second;
    BOOST_CHECK_EQUAL_COLLECTIONS(seg2->begin(), seg2->end(), expectedSeg2,
                                  expectedSeg2 + 3);

    vector<Family> families = cg2fam.getFamilies();
    BOOST_CHECK_EQUAL(2, families.size());
    bool famGMA1_GMA2_found = false;
    for (Family fam : families)
    {
        famGMA1_GMA2_found |= fam.name == "GMA1_GMA2";
    }
    BOOST_CHECK(famGMA1_GMA2_found);

}

//____________________________________________________________________________//
