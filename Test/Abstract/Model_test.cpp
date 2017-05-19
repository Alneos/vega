/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Model_test.cpp
 *
 *  Created on: Jan 11, 2013
 *      Author: dallolio
 */

#include "build_properties.h"
#include "../../Abstract/ConfigurationParameters.h"
#include "../../Abstract/Model.h"
#include <cstddef>
#include <new>
#include <string>
#include <vector>
#if defined VDEBUG && defined __GNUC__
#include <valgrind/memcheck.h>
#endif

#define BOOST_TEST_MODULE model_test
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace vega;

// BOOST_AUTO_TEST_CASE( test_node_pass_valgrind ) {
// //valgrind bug http://stackoverflow.com/questions/18776380
// Node node2(1, 0.0, 0.0, 0.0, 10, false);
// VALGRIND_CHECK_VALUE_IS_DEFINED(node2);
// Cell cell(28, CellType::SEG2, { 1, 2 });
// VALGRIND_CHECK_VALUE_IS_DEFINED(cell);
// }

BOOST_AUTO_TEST_CASE( test_model_spc ) {
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
	vega::NodeGroup* gn1 = model.mesh->findOrCreateNodeGroup("GN1");
	gn1->addNode(0);
	gn1->addNode(6);
	vega::CellGroup* gm1 = model.mesh->createCellGroup("GM1");
	gm1->addCell(31);
	vega::CellGroup* gm2 = model.mesh->createCellGroup("GM2");
	gm2->addCell(28);
	gm2->addCell(30);
	//SinglePointConstraint spc1 = SinglePointConstraint(model, true, true, true, false, false, false,
	//		0.0, gn1);
	SinglePointConstraint spc1 = SinglePointConstraint(model,std::array<ValueOrReference, 3>{{ 0, 0, 0 }}, gn1);
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
	model.mesh->writeMED(outfname.c_str());
	BOOST_CHECK(fs::exists(outfname.c_str()));
}

BOOST_AUTO_TEST_CASE( test_cells_iterator ) {
	Model model("inputfile", "10.3", SolverName::NASTRAN);
	BOOST_CHECKPOINT("Mesh start");
	model.mesh->addCell(27, CellType::SEG2, {0, 1});
	model.mesh->addCell(28, CellType::SEG2, {1, 2});
	model.mesh->addCell(29, CellType::SEG2, {2, 3});
	model.mesh->addCell(30, CellType::TRI3, {3, 4, 5});
	model.mesh->addCell(31, CellType::POINT1, {1});
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 1.0 };
	int i;
	for (i = 0; i < 18; i += 3) {
		model.mesh->addNode(i / 3, coords[i], coords[i + 1], coords[i + 2]);
	}
	BOOST_CHECKPOINT("Mesh finish");
	model.mesh->finish();

	CellIterator cellIterator = model.mesh->cells.cells_begin(CellType::TRI3);
	BOOST_CHECKPOINT("Iteration begin1");
//C++ style
	for (i = 0; cellIterator != model.mesh->cells.cells_end(CellType::TRI3); cellIterator++) {
		i++;
		Cell cell(*(cellIterator));
		cout << cell << endl;
	}
	BOOST_CHECK_EQUAL(1, i);
	BOOST_CHECKPOINT("Iteration new begin2");
	CellIterator cellIterator2 = model.mesh->cells.cells_begin(CellType::SEG2);
	for (i = 0; cellIterator2.hasNext(); i++) {
		Cell cell = cellIterator2.next();
		cout << cell << endl;
	}
	BOOST_CHECK_EQUAL(3, i);
	BOOST_CHECK_EQUAL(3, model.mesh->countCells(CellType::SEG2));
	BOOST_CHECK_EQUAL(1, model.mesh->countCells(CellType::POINT1));
	//BOOST_CHECK_EQUAL(0, model.mesh->countCells(CellType::POLYL));

}

BOOST_AUTO_TEST_CASE( test_Elements ) {
	Model model("inputfile", "10.3", SolverName::NASTRAN,
			ModelConfiguration(true, LogLevel::INFO, true, true, false, false));
	double coords[12] = { -433., 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
	int j = 1;
	for (int i = 0; i < 12; i += 3) {
		model.mesh->addNode(j, coords[i], coords[i + 1], coords[i + 2]);
		j++;
	}
	model.mesh->addCell(1, CellType::SEG2, {1, 4});
	model.mesh->addCell(2, CellType::SEG2, {2, 4});
	model.mesh->addCell(3, CellType::SEG2, {3, 4});
	vega::CellGroup* cn1 = model.mesh->createCellGroup("GM1");
	cn1->addCell(1);
	cn1->addCell(2);
	BOOST_CHECKPOINT("after addcells");
	RectangularSectionBeam rectangularSectionBeam(model, 100.0, 110.0, Beam::EULER, 1);
	rectangularSectionBeam.assignCellGroup(cn1);
	rectangularSectionBeam.assignMaterial(1);
	model.add(rectangularSectionBeam);
	model.getOrCreateMaterial(1)->addNature(ElasticNature(model, 1, 0));
	cout << "NODES:" << model.mesh->countNodes() << endl;
	model.finish();
	BOOST_CHECK(model.validate());
	const vector<shared_ptr<ElementSet>> beams = model.filterElements(ElementSet::RECTANGULAR_SECTION_BEAM);
	BOOST_CHECK_EQUAL((size_t )1, beams.size());

//no virtual elements

	const vector<shared_ptr<ElementSet>> discrets = model.filterElements(ElementSet::DISCRETE_0D);
	BOOST_CHECK_EQUAL((size_t )0, discrets.size());
	CellContainer assignment = model.getOrCreateMaterial(1)->getAssignment();

	BOOST_CHECK(assignment.hasCellGroups());
	BOOST_CHECK_EQUAL(assignment.getCellGroups()[0]->getName(), "GM1");

}

shared_ptr<Model> createModelWith1HEXA8() {
	shared_ptr<Model> model(
			new Model("inputfile", "10.3", SolverName::NASTRAN,
					ModelConfiguration(true, LogLevel::INFO, true)));
	double coords[24] = {
			0., 0., 0.,
			1., 0., 0.,
			1., 1., 0.,
			0., 1., 0.,
			0., 0., 1.,
			1., 0., 1.,
			1., 1., 1.,
			0., 1., 1.,
	};

	vector<int> nodeIds = { 50, 51, 52, 53, 54, 55, 56, 57 };
	for (int i = 0; i < 8; i ++) {
		model->mesh->addNode(nodeIds[i], coords[i*3], coords[i*3 + 1], coords[i*3 + 2]);
	}
	int cellPosition = model->mesh->addCell(1, CellType::HEXA8, nodeIds);
	Cell hexa = model->mesh->findCell(cellPosition);
	BOOST_CHECK_EQUAL_COLLECTIONS(hexa.nodeIds.begin(), hexa.nodeIds.end(), nodeIds.begin(),
			nodeIds.end());
	BOOST_CHECK_EQUAL(hexa.id, 1);
	vector<int> face1NodeIds = hexa.faceids_from_two_nodes(50, 52);
	vector<int> expectedFace1NodeIds = { 50, 51, 52, 53 };
	BOOST_CHECK_EQUAL_COLLECTIONS(face1NodeIds.begin(), face1NodeIds.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
	vega::CellGroup* cn1 = model->mesh->createCellGroup("GM1");
	cn1->addCell(1);
	BOOST_CHECKPOINT("after addcells");
	Continuum continuum(*model, &ModelType::TRIDIMENSIONAL_SI, 1);
	continuum.assignCellGroup(cn1);
	continuum.assignMaterial(1);
	model->add(continuum);
	model->getOrCreateMaterial(1)->addNature(ElasticNature(*model, 1, 0));
	return model;
}

// BOOST_AUTO_TEST_CASE( test_VirtualElements ) {
// Model model = createModelWith1HEXA8();
// //moment on x axis on node 51
// model.add(NodalForceComponents(model, 51, 0, 0, 0, 1.0, 0, 0));
// //force on x axis on node 52
// model.add(NodalForceComponents(model, 52, 1.0, 0, 0, 0.0, 0, 0));
// model.add(NodalForceComponents(model, 53, 0.0, 0.0, 0.0, 1.0, 0, 1.0));
// model.finish();
// BOOST_CHECK(model.validate());
//
// //two virtual elements
// vector<Element *> discrets = model.filterElements(Element::DISCRET);
// BOOST_CHECK_EQUAL(1, discrets.size());
//
// Node node51 = model.mesh->nodes.findNode(model.mesh->nodes.findNodePosition(51));
// BOOST_CHECK(node51.dofs == DOFS::ALL_DOFS);
// Node node52 = model.mesh->nodes.findNode(model.mesh->nodes.findNodePosition(52));
// BOOST_CHECK(node52.dofs == DOFS::TRANSLATIONS);
// Node node53 = model.mesh->nodes.findNode(model.mesh->nodes.findNodePosition(53));
// BOOST_CHECK(node53.dofs == DOFS::ALL_DOFS);
//
// map<int, ConstraintSet> constraints = model.getConstraintBySetId();
// BOOST_CHECK_EQUAL(1, constraints.size());
// ConstraintSet set = constraints.find(0)->second;
// vector<shared_ptr<Constraint> > spcs = set.getConstraintByType(
// Constraint::SPC);
// BOOST_CHECK_EQUAL(5, spcs.size());
// }


BOOST_AUTO_TEST_CASE( test_create_skin2d ) {
	shared_ptr<Model> model = createModelWith1HEXA8();
	vega::PressionFaceTwoNodes pressionFaceTwoNodes = PressionFaceTwoNodes(*model, 50, 52,
			VectorialValue(0, 0, 1.0), VectorialValue(0, 0, 0));

	pressionFaceTwoNodes.addCell(1);
	vector<Cell> cells = pressionFaceTwoNodes.getCells();
	BOOST_CHECK_EQUAL(cells.size(), (size_t )1);
	Cell hexa = cells[0];
	BOOST_CHECK_EQUAL(hexa.id, 1);
	//BOOST_CHECK_EQUAL(hexa.cellType, CellType::HEXA8);
	BOOST_CHECK_EQUAL(hexa.nodeIds[0], 50);
	model->add(pressionFaceTwoNodes);
	vector<int> expectedFace1NodeIds = { 50, 51, 52, 53 };
	vector<int> applicationFace = pressionFaceTwoNodes.getApplicationFace();
	BOOST_CHECK_EQUAL_COLLECTIONS(applicationFace.begin(), applicationFace.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
	model->finish();
	BOOST_CHECK_EQUAL(model->materials.size(), 1);
	BOOST_CHECK(model->validate());
	BOOST_REQUIRE_EQUAL(1, model->mesh->countCells(CellType::QUAD4));
	Cell cell = model->mesh->cells.cells_begin(CellType::QUAD4).next();
	BOOST_CHECK_EQUAL_COLLECTIONS(cell.nodeIds.begin(), cell.nodeIds.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
}

BOOST_AUTO_TEST_CASE(test_Analysis) {
	ModelConfiguration configuration(false, LogLevel::DEBUG, false, false, false, false, false);
	Model model("inputfile", "10.3", SolverName::NASTRAN, configuration);
	double coords[12] = { -433., 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
	int j = 1;
	for (int i = 0; i < 12; i += 3) {
		model.mesh->addNode(j, coords[i], coords[i + 1], coords[i + 2]);
		j++;
	}

	LinearMecaStat analysis(model);

	LoadSet loadSet1(model, LoadSet::LOAD, 1);
	model.add(loadSet1);
	Reference<LoadSet> loadSetRef = Reference<LoadSet>(loadSet1);
	analysis.add(loadSetRef);
	BOOST_CHECK(analysis.contains(loadSetRef));
	//CHECK getReference constructor and == operator
	BOOST_CHECK(loadSetRef == loadSet1.getReference());

	LoadSet loadSet2(model, LoadSet::LOAD, 2);
	//model.add(loadSet2);
	analysis.add(loadSet2);

	model.add(analysis);

	NodalForce force1(model, 1, 0.0, 1.0);
	model.add(force1);
	model.addLoadingIntoLoadSet(force1, loadSet1);

	NodalForce force2(model, 2, 0.0, 1.0);
	model.add(force2);
	model.addLoadingIntoLoadSet(force2, loadSet1);

	NodalForce force3(model, 2, 0.0, 1.0);
	model.add(force3);
	BOOST_CHECKPOINT("before finish");
	model.finish();
	BOOST_CHECKPOINT("after finish");
// LoadSet2 is missing in the model
	BOOST_CHECK_EQUAL(1, model.loadSets.size());
	BOOST_CHECK_EQUAL((size_t ) 2, analysis.getLoadSets().size());
	for (shared_ptr<LoadSet> ls : analysis.getLoadSets()) {
		if (ls)
			cout << "!!!!!!!!" << *ls << endl;
	}
	BOOST_CHECK(!analysis.validate());
	BOOST_CHECK(!model.validate());

	set<shared_ptr<Loading>> loadings = model.getLoadingsByLoadSet(loadSet1);
	BOOST_CHECK_EQUAL((size_t )2, loadings.size());
	for (shared_ptr<Loading> loading : loadings) {
		BOOST_CHECK(loading->getId() == force1.getId() || loading->getId() == force2.getId());
	}
}

BOOST_AUTO_TEST_CASE( test_find_methods ) {
	string outFile(PROJECT_BINARY_DIR "/bin/testMed.med");
	Model model("inputfile", "10.3", SolverName::NASTRAN);
	model.mesh->addCell(27, CellType::SEG2, {0, 1});
	model.mesh->addCell(28, CellType::SEG2, {1, 2});
	model.mesh->addCell(29, CellType::SEG2, {2, 3});
	model.mesh->addCell(30, CellType::SEG2, {3, 4});
	model.mesh->addCell(31, CellType::POINT1, {1});
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 0.0 };
	int i;
	for (i = 0; i < 18; i += 3) {
		model.mesh->addNode(i / 3, coords[i], coords[i + 1], coords[i + 2]);
	}
	vega::NodeGroup* gn1 = model.mesh->findOrCreateNodeGroup("GN1");
	BOOST_CHECKPOINT("find_methods: before add Node 0");
	gn1->addNode(0);
	BOOST_CHECKPOINT("find_methods: before add Node 5");
	gn1->addNode(5);
	vega::CellGroup* gm1 = model.mesh->createCellGroup("GM1");
	gm1->addCell(31);
	vega::CellGroup* gm2 = model.mesh->createCellGroup("GM2");
	gm2->addCell(28);
	gm2->addCell(30);
	vega::CellGroup* gm3 = model.mesh->createCellGroup("GM3");
	gm3->addCell(28);
	gm3->addCell(30);
	model.finish();
	BOOST_CHECKPOINT("find_methods: model completed");
	BOOST_CHECK_EQUAL(gn1, model.mesh->findGroup("GN1"));
	BOOST_CHECK(model.mesh->findGroup("DONT-EXIST") == nullptr);
	model.mesh->writeMED(outFile.c_str());
	BOOST_CHECK(boost::filesystem::exists(outFile));
}

BOOST_AUTO_TEST_CASE( combined_loadset1 ) {
	Model model("inputfile", "10.3", SolverName::NASTRAN);
	model.configuration.logLevel = LogLevel::DEBUG;
	LoadSet loadSet1(model, LoadSet::LOAD, 1);
	LoadSet loadSet3(model, LoadSet::LOAD, 3);
	model.mesh->addNode(1, 0.0, 0.0, 0.0);
	NodalForce force1 = NodalForce(model, 1, 1.0);
	model.add(force1);
	model.addLoadingIntoLoadSet(force1, loadSet1);
	NodalForce force3 = NodalForce(model, 1, 3.0);
	model.add(force3);
	model.addLoadingIntoLoadSet(force3, loadSet1);
	NodalForce force2 = NodalForce(model, 1, 2.0);
	model.add(force2);
	model.addLoadingIntoLoadSet(force2, loadSet3);
	LoadSet combination(model, LoadSet::LOAD, 10);
	combination.embedded_loadsets.push_back(
			pair<Reference<LoadSet>, double>(loadSet1.getReference(), 5.0));
	combination.embedded_loadsets.push_back(
			pair<Reference<LoadSet>, double>(loadSet3.getReference(), 7.0));
	BOOST_CHECK_EQUAL(combination.embedded_loadsets.size(), (size_t ) 2);
	model.add(combination);
	model.finish();
	BOOST_CHECK_EQUAL(model.getLoadingsByLoadSet(combination).size(), (size_t ) 3);
}

BOOST_AUTO_TEST_CASE( reference_compare ) {
	Reference<LoadSet> rauto1(LoadSet::LOAD, Reference<LoadSet>::NO_ID, 1);
	BOOST_CHECK(rauto1 == rauto1);
	BOOST_CHECK(!(rauto1 < rauto1));
	BOOST_CHECK(rauto1 == *rauto1.clone());
	Reference<LoadSet> rauto2(LoadSet::LOAD, Reference<LoadSet>::NO_ID, 2);
	Reference<LoadSet> r1(LoadSet::LOAD, 1);
	BOOST_CHECK(r1 == r1);
	BOOST_CHECK(!(r1 < r1));
	BOOST_CHECK(r1 == *r1.clone());
	Reference<LoadSet> r1bis(LoadSet::LOAD, 1);
	BOOST_CHECK(r1 == r1bis);
	BOOST_CHECK(!(r1 < r1bis));
	Reference<LoadSet> r2(LoadSet::LOAD, 2);
	BOOST_CHECK(!(r1 == r2));
	BOOST_CHECK(r1 < r2);
	Reference<LoadSet> rd1(LoadSet::DLOAD, 1);
	BOOST_CHECK(!(r1 == rd1));
	BOOST_CHECK(r1 < rauto1);
}

BOOST_AUTO_TEST_CASE( reference_hash ) {
	Reference<LoadSet> rauto1(LoadSet::LOAD, Reference<LoadSet>::NO_ID, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(rauto1), hash<Reference<LoadSet>>()(rauto1));
	Reference<LoadSet> rauto2(LoadSet::LOAD, Reference<LoadSet>::NO_ID, 2);
	BOOST_CHECK_NE(rauto1.id, rauto2.id);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(rauto1), hash<Reference<LoadSet>>()(rauto2));
	Reference<LoadSet> r1(LoadSet::LOAD, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r1));
	Reference<LoadSet> r1bis(LoadSet::LOAD, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r1bis));
	Reference<LoadSet> r2(LoadSet::LOAD, 2);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r2));
	Reference<LoadSet> rd1(LoadSet::DLOAD, 1);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(rd1));
	Reference<ConstraintSet> rc1(ConstraintSet::SPC, 1);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<ConstraintSet>>()(rc1));
}

BOOST_AUTO_TEST_CASE(test_ineffective_assertions_removed) {
	// Two NodalDisplacementAssertion are added to a model.
	// The first one is on a node DX but the node don't have that degree of freedom. It must be
	// removed by the finish.
	shared_ptr<Model> model = createModelWith1HEXA8();
	LinearMecaStat analysis(*model);
	NodalDisplacementAssertion nda(*model, 0.0001, 50, DOF::DZ, 1., 1);
	analysis.add(nda);
	model->add(nda);

	NodalDisplacementAssertion nda2(*model, 0.0001, 51, DOF::RX, 1., 1);
	analysis.add(nda2);
	model->add(nda2);
	model->add(analysis);
	BOOST_CHECK_EQUAL(2, model->objectives.size());
	model->finish();
	BOOST_CHECK(analysis.validate());
	BOOST_CHECK(model->validate());
	BOOST_CHECK_EQUAL(model->analyses.size(), 1);
	BOOST_CHECK_EQUAL(model->objectives.size(), 1);
	vector<shared_ptr<Assertion>> assertions = (*model->analyses.begin())->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), (size_t )1);
}

BOOST_AUTO_TEST_CASE(test_rbe3_assertions_not_removed) {
	Model model("fakemodelfortest", "10.3", SolverName::NASTRAN,
					ModelConfiguration(false, LogLevel::INFO, false, false, false));
	model.mesh->addNode(100, 0.0, 0.0, 0.0);
	model.mesh->addNode(101, 1.0, 1.0, 1.0);
	RBE3 rbe3(model, 100, DOFS::ALL_DOFS);
	rbe3.addSlave(101, DOFS::ALL_DOFS, 42.0);
	model.add(rbe3);
	model.addConstraintIntoConstraintSet(rbe3, model.commonConstraintSet);
	LinearMecaStat analysis(model);
	NodalDisplacementAssertion nda(model, 0.0001, 100, DOF::DZ, 1., 1);
	analysis.add(nda);
	model.add(nda);

	NodalDisplacementAssertion nda2(model, 0.0001, 101, DOF::RX, 1., 1);
	analysis.add(nda2);
	model.add(nda2);
	model.add(analysis);
	BOOST_CHECK_EQUAL(2, model.objectives.size());
	model.finish();
	BOOST_CHECK(analysis.validate());
	BOOST_CHECK(model.validate());
	BOOST_CHECK_EQUAL(model.analyses.size(), 1);
	BOOST_CHECK_EQUAL(model.objectives.size(), 2);
	vector<shared_ptr<Assertion>> assertions = (*model.analyses.begin())->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), (size_t )2);
}

BOOST_AUTO_TEST_CASE(test_spc_dof_remove) {
	shared_ptr<Model> model = createModelWith1HEXA8();
	LinearMecaStat analysis1(*model);
	SinglePointConstraint spc(*model, DOFS::ALL_DOFS, 0.0);
	spc.addNodeId(50);
	model->add(spc);
	model->addConstraintIntoConstraintSet(spc, model->commonConstraintSet);
	model->add(analysis1);
	LinearMecaStat analysis2(*model);
	model->add(analysis2);
	BOOST_CHECK_EQUAL(model->commonConstraintSet.getConstraints().size(), 1);
	int nodePosition = model->mesh->findNodePosition(50);
	BOOST_CHECK_EQUAL(spc.nodePositions().size(), 1);
	BOOST_CHECK_EQUAL(spc.getDOFSForNode(nodePosition), DOFS::ALL_DOFS);
	BOOST_CHECK_EQUAL(model->getConstraintSetsByConstraint(spc).size(), 1);
	BOOST_CHECK_EQUAL(model->constraintSets.size(), 1);
	BOOST_CHECKPOINT("model filled");
	analysis1.removeSPCNodeDofs(spc, nodePosition, DOF::DZ);
	BOOST_CHECK_EQUAL(spc.getDOFSForNode(nodePosition), DOFS::ALL_DOFS);
	BOOST_CHECK_EQUAL(model->commonConstraintSet.getConstraints().size(), 2);
	BOOST_CHECK_EQUAL(spc.nodePositions().size(), 0);
	for(auto& constraint : model->commonConstraintSet.getConstraints()) {
		if (*constraint == spc) {
			continue;
		}
		cout << *constraint;
		BOOST_CHECK_EQUAL(constraint->getDOFSForNode(nodePosition), DOFS::ALL_DOFS - DOF::DZ);
		BOOST_CHECK_EQUAL(constraint->nodePositions().size(), 1);
	}
	BOOST_CHECK_EQUAL(model->constraintSets.size(), 2);
	BOOST_CHECK_EQUAL(model->find(analysis1.getReference())->getConstraintSets().size(), 1);
	auto constraintSets2 = model->find(analysis2.getReference())->getConstraintSets();
	BOOST_CHECK_EQUAL(constraintSets2.size(), 2);
	for(auto& constraintSet : constraintSets2) {
		if (*constraintSet == model->commonConstraintSet) {
			continue;
		}
		BOOST_CHECK_EQUAL(constraintSet->getConstraints().size(), 1);
		for(auto& constraint : constraintSet->getConstraints()) {
			cout << *constraint;
			BOOST_CHECK_EQUAL(constraint->getDOFSForNode(nodePosition), DOF::DZ);
			BOOST_CHECK_EQUAL(constraint->nodePositions().size(), 1);
		}
	}
}
//____________________________________________________________________________//

