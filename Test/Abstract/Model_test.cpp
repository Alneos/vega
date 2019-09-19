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
#include "Model_test.h"
#include <cstddef>
#include <new>
#include <string>
#include <vector>
#if defined VDEBUG && defined __GNUC__  && !defined(_WIN32)
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
	const auto& spc1 = make_shared<SinglePointConstraint>(model,array<ValueOrReference, 3>{{ 0, 0, 0 }});
	spc1->add(*gn1);
	model.add(spc1);
	model.finish();

	shared_ptr<SinglePointConstraint> spc1_ptr = dynamic_pointer_cast<SinglePointConstraint>(
			model.find(spc1->getReference()));
	BOOST_CHECK(spc1_ptr);
	DOFS spc_dofs = spc1_ptr->getDOFSForNode(0);
	BOOST_CHECK(spc1_ptr->hasReferences() == false);
	BOOST_CHECK_EQUAL(spc_dofs, DOFS::TRANSLATIONS);
	BOOST_CHECK(spc1_ptr->nodePositions().size() == 2);
}

BOOST_AUTO_TEST_CASE( test_cells_iterator ) {
	Model model{"inputfile", "10.3", SolverName::NASTRAN};
	BOOST_TEST_CHECKPOINT("Mesh start");
	model.mesh.addCell(27, CellType::SEG2, {0, 1});
	model.mesh.addCell(28, CellType::SEG2, {1, 2});
	model.mesh.addCell(29, CellType::SEG2, {2, 3});
	model.mesh.addCell(30, CellType::TRI3, {3, 4, 5});
	model.mesh.addCell(31, CellType::POINT1, {1});
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 1.0 };
	int i;
	for (i = 0; i < 18; i += 3) {
		model.mesh.addNode(i / 3, coords[i], coords[i + 1], coords[i + 2]);
	}
	BOOST_TEST_CHECKPOINT("Mesh finish");
	model.mesh.finish();

	CellIterator cellIterator = model.mesh.cells.cells_begin(CellType::TRI3);
	BOOST_TEST_CHECKPOINT("Iteration begin1");
//C++ style
	for (i = 0; cellIterator != model.mesh.cells.cells_end(CellType::TRI3); cellIterator++) {
		i++;
		Cell cell(*(cellIterator));
		cout << cell << endl;
	}
	BOOST_CHECK_EQUAL(1, i);
	BOOST_TEST_CHECKPOINT("Iteration begin2");
	CellIterator cellIterator2 = model.mesh.cells.cells_begin(CellType::SEG2);
	for (i = 0; cellIterator2.hasNext(); i++) {
		Cell cell = cellIterator2.next();
		cout << cell << endl;
	}
	BOOST_CHECK_EQUAL(3, i);
	BOOST_CHECK_EQUAL(3, model.mesh.countCells(CellType::SEG2));
	BOOST_CHECK_EQUAL(1, model.mesh.countCells(CellType::POINT1));
	BOOST_CHECK_EQUAL(static_cast<size_t>(3), model.mesh.cells.cellTypes().size());
	//BOOST_CHECK_EQUAL(0, model.mesh.countCells(CellType::POLYL));

}

BOOST_AUTO_TEST_CASE( test_Elements ) {
	Model model{"inputfile", "10.3", SolverName::NASTRAN};
	double coords[12] = { -433., 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
	int j = 1;
	for (int i = 0; i < 12; i += 3) {
		model.mesh.addNode(j, coords[i], coords[i + 1], coords[i + 2]);
		j++;
	}
	model.mesh.addCell(1, CellType::SEG2, {1, 4});
	model.mesh.addCell(2, CellType::SEG2, {2, 4});
	model.mesh.addCell(3, CellType::SEG2, {3, 4});
	shared_ptr<vega::CellGroup> cn1 = model.mesh.createCellGroup("GM1");
	cn1->addCellId(1);
	cn1->addCellId(2);
	BOOST_TEST_CHECKPOINT("after addcells");
	const auto& rectangularSectionBeam = make_shared<RectangularSectionBeam>(model, 100.0, 110.0, Beam::BeamModel::EULER, 1);
	rectangularSectionBeam->add(*cn1);
	rectangularSectionBeam->assignMaterial(1);
	model.add(rectangularSectionBeam);
	model.getOrCreateMaterial(1)->addNature(make_shared<ElasticNature>(model, 1, 0));
	cout << "NODES:" << model.mesh.countNodes() << endl;
	model.finish();
	BOOST_CHECK(model.validate());
	const vector<shared_ptr<ElementSet>> beams = model.elementSets.filter(ElementSet::Type::RECTANGULAR_SECTION_BEAM);
	BOOST_CHECK_EQUAL(1, beams.size());
	BOOST_CHECK(model.elementSets.contains(ElementSet::Type::RECTANGULAR_SECTION_BEAM));

//no virtual elements

	const vector<shared_ptr<ElementSet>> discrets = model.elementSets.filter(ElementSet::Type::DISCRETE_0D);
	BOOST_CHECK_EQUAL(0, discrets.size());
	BOOST_CHECK(not model.elementSets.contains(ElementSet::Type::DISCRETE_0D));
	CellContainer assignment = model.getOrCreateMaterial(1)->getAssignment();

	BOOST_CHECK(assignment.hasCellGroups());
    BOOST_CHECK(not assignment.getCellGroups().empty());
	BOOST_CHECK_EQUAL(assignment.getCellGroups()[0]->getName(), "GM1");

}

unique_ptr<Model> createModelWith1HEXA8() {
    ModelConfiguration configuration;
    configuration.virtualDiscrets = true;
    configuration.createSkin = true;
	unique_ptr<Model> model = make_unique<Model>("inputfile", "10.3", SolverName::NASTRAN,
					configuration);
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
		model->mesh.addNode(nodeIds[i], coords[i*3], coords[i*3 + 1], coords[i*3 + 2]);
	}
	int cellPosition = model->mesh.addCell(1, CellType::HEXA8, nodeIds);
	const Cell& hexa = model->mesh.findCell(cellPosition);
	BOOST_CHECK_EQUAL_COLLECTIONS(hexa.nodeIds.begin(), hexa.nodeIds.end(), nodeIds.begin(),
			nodeIds.end());
	BOOST_CHECK_EQUAL(hexa.id, 1);
	vector<int> face1NodeIds = hexa.faceids_from_two_nodes(50, 52);
	vector<int> expectedFace1NodeIds = { 50, 51, 52, 53 };
	BOOST_CHECK_EQUAL_COLLECTIONS(face1NodeIds.begin(), face1NodeIds.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
	shared_ptr<vega::CellGroup> cn1 = model->mesh.createCellGroup("GM1");
	cn1->addCellId(1);
	BOOST_TEST_CHECKPOINT("after addcells");
	const auto& continuum = make_shared<Continuum>(*model, ModelType::TRIDIMENSIONAL_SI, 1);
	continuum->add(*cn1);
	continuum->assignMaterial(1);
	model->add(continuum);
	model->getOrCreateMaterial(1)->addNature(make_shared<ElasticNature>(*model, 1, 0));
	return model;
}

BOOST_AUTO_TEST_CASE( test_graph ) {
    unique_ptr<Model> model = createModelWith1HEXA8();
	const auto& spc = make_shared<SinglePointConstraint>(*model, DOFS::ALL_DOFS, 0.0);
	spc->addNodeId(50);
	model->add(spc);
	model->addConstraintIntoConstraintSet(spc->getReference(), model->commonConstraintSet->getReference());

    const auto& analysis = make_shared<LinearMecaStat>(*model);
    model->add(analysis);
    analysis->createGraph(cout);
}

 BOOST_AUTO_TEST_CASE( test_VirtualElements ) {
     unique_ptr<Model> model = createModelWith1HEXA8();
     const auto& loadSet1 = make_shared<LoadSet>(*model, LoadSet::Type::LOAD, 1);
     //model->add(loadSet1);
     //moment on x axis on node 51
     const auto& f1 = make_shared<NodalForce>(*model, loadSet1, 0, 0, 0, 1.0, 0, 0);
     f1->addNodeId(51);
     model->add(f1);
     //force on x axis on node 52
     const auto& f2 = make_shared<NodalForce>(*model, loadSet1, 1.0, 0, 0, 0.0, 0, 0);
     f2->addNodeId(52);
     model->add(f2);
     const auto& f3 = make_shared<NodalForce>(*model, loadSet1, 0.0, 0.0, 0.0, 1.0, 0, 1.0);
     f3->addNodeId(53);
     model->add(f3);
     const auto& analysis = make_shared<LinearMecaStat>(*model);
     analysis->add(loadSet1);
     model->add(analysis);
     model->finish(); // Should add constraints, discretes to add dofs
     BOOST_CHECK(model->validate());

     BOOST_CHECK(model->analyses.contains(Analysis::Type::LINEAR_MECA_STAT));

     //two virtual elements
     const auto& discrets = model->elementSets.filter(ElementSet::Type::DISCRETE_0D);
     BOOST_CHECK_EQUAL(1, discrets.size());
     BOOST_CHECK(model->elementSets.contains(ElementSet::Type::DISCRETE_0D));

     const Node& node51 = model->mesh.findNode(model->mesh.findNodePosition(51));
     BOOST_CHECK(node51.dofs == DOFS::ALL_DOFS);
     const Node& node52 = model->mesh.findNode(model->mesh.findNodePosition(52));
     BOOST_CHECK(node52.dofs == DOFS::TRANSLATIONS);
     const Node& node53 = model->mesh.findNode(model->mesh.findNodePosition(53));
     BOOST_CHECK(node53.dofs == DOFS::ALL_DOFS);

     BOOST_CHECK_EQUAL(2, model->constraintSets.size());
     //auto cset = model->constraintSets.next();
     //auto& spcs = cset->getConstraintsByType(Constraint::Type::SPC);
     //BOOST_CHECK_EQUAL(5, spcs.size());
 }


BOOST_AUTO_TEST_CASE( test_create_skin2d ) {
	unique_ptr<Model> model = createModelWith1HEXA8();
	const auto& forceSurfaceTwoNodes = make_shared<ForceSurfaceTwoNodes>(*model, nullptr, 50, 52,
			VectorialValue(0, 0, 1.0), VectorialValue(0, 0, 0));

	forceSurfaceTwoNodes->addCellId(1);
	const auto& cells = forceSurfaceTwoNodes->getCellsIncludingGroups();
	BOOST_CHECK_EQUAL(cells.size(), static_cast<size_t>(1));
	Cell hexa = *cells.begin();
	BOOST_CHECK_EQUAL(hexa.id, 1);
	//BOOST_CHECK_EQUAL(hexa.cellType, CellType::HEXA8);
	BOOST_CHECK_EQUAL(hexa.nodeIds[0], 50);
	model->add(forceSurfaceTwoNodes);
	vector<int> expectedFace1NodeIds = { 50, 51, 52, 53 };
	vector<int> applicationFace = forceSurfaceTwoNodes->getApplicationFaceNodeIds();
	BOOST_CHECK_EQUAL_COLLECTIONS(applicationFace.begin(), applicationFace.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());

    // Same check but using base class
    const auto& cellLoading = dynamic_pointer_cast<CellLoading>(forceSurfaceTwoNodes);
	vector<int> applicationFace2 = cellLoading->getApplicationFaceNodeIds();
	BOOST_CHECK_EQUAL_COLLECTIONS(applicationFace2.begin(), applicationFace2.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
	model->finish();
	//BOOST_CHECK_EQUAL(model->materials.size(), 2 ); // 2 because skin adds a virtual material
	BOOST_CHECK(model->validate());
	BOOST_REQUIRE_EQUAL(1, model->mesh.countCells(CellType::QUAD4));
	Cell cell = model->mesh.cells.cells_begin(CellType::QUAD4).next();
	BOOST_CHECK_EQUAL_COLLECTIONS(cell.nodeIds.begin(), cell.nodeIds.end(),
			expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
}

BOOST_AUTO_TEST_CASE(test_Analysis) {
	Model model{"inputfile", "10.3", SolverName::NASTRAN};
	double coords[12] = { -433., 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
	int j = 1;
	for (int i = 0; i < 12; i += 3) {
		model.mesh.addNode(j, coords[i], coords[i + 1], coords[i + 2]);
		j++;
	}

	const auto& analysis = make_shared<LinearMecaStat>(model);

	const auto& loadSet1 = make_shared<LoadSet>(model, LoadSet::Type::LOAD, 1);
	model.add(loadSet1);
	Reference<LoadSet> loadSetRef = Reference<LoadSet>(*loadSet1);
	cout << loadSetRef << endl;
	analysis->add(loadSetRef);
	BOOST_CHECK(analysis->contains(loadSetRef));
	//CHECK getReference constructor and == operator
	BOOST_CHECK(loadSetRef == loadSet1->getReference());

	LoadSet loadSet2(model, LoadSet::Type::LOAD, 2);
	//model.add(loadSet2);
	analysis->add(loadSet2);

	model.add(analysis);

	const auto& force1 = make_shared<NodalForce>(model, loadSet1, 1, 0.0, 1.0);
	model.add(force1);

	const auto& force2 = make_shared<NodalForce>(model, loadSet1, 2, 0.0, 1.0);
	model.add(force2);

	const auto& force3 = make_shared<NodalForce>(model, nullptr, 2, 0.0, 1.0);
	model.add(force3);
	BOOST_TEST_CHECKPOINT("before finish");
	model.finish();
	BOOST_TEST_CHECKPOINT("after finish");
// LoadSet2 is missing in the model
	BOOST_CHECK_EQUAL(1, model.loadSets.size());
	BOOST_CHECK_EQUAL(1, analysis->getLoadSets().size());
	for (const auto& ls : analysis->getLoadSets()) {
		if (ls != nullptr)
			cout << "Found loadset:" << *ls << endl;
	}
	BOOST_CHECK(not analysis->validate());
	BOOST_CHECK(not model.validate());

	auto& loadings = model.getLoadingsByLoadSet(loadSet1);
	BOOST_CHECK_EQUAL(2, loadings.size());
	for (const auto& loading : loadings) {
		BOOST_CHECK(loading->getId() == force1->getId() or loading->getId() == force2->getId());
	}
}

BOOST_AUTO_TEST_CASE( test_find_methods ) {
	Model model{"inputfile", "10.3", SolverName::NASTRAN};
	model.mesh.addCell(27, CellType::SEG2, {0, 1});
	model.mesh.addCell(28, CellType::SEG2, {1, 2});
	model.mesh.addCell(29, CellType::SEG2, {2, 3});
	model.mesh.addCell(30, CellType::SEG2, {3, 4});
	model.mesh.addCell(31, CellType::POINT1, {1});
	double coords[18] = { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.75, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4, 0.0,
			0.0, 1.3, 0.0, 0.0 };
	int i;
	for (i = 0; i < 18; i += 3) {
		model.mesh.addNode(i / 3, coords[i], coords[i + 1], coords[i + 2]);
	}
	shared_ptr<vega::NodeGroup> gn1 = model.mesh.findOrCreateNodeGroup("GN1");
	BOOST_TEST_CHECKPOINT("find_methods: before add Node 0");
	gn1->addNodeId(0);
	BOOST_TEST_CHECKPOINT("find_methods: before add Node 5");
	gn1->addNodeId(5);
	shared_ptr<vega::CellGroup> gm1 = model.mesh.createCellGroup("GM1");
	gm1->addCellId(31);
	shared_ptr<vega::CellGroup> gm2 = model.mesh.createCellGroup("GM2");
	gm2->addCellId(28);
	gm2->addCellId(30);
	shared_ptr<vega::CellGroup> gm3 = model.mesh.createCellGroup("GM3");
	gm3->addCellId(28);
	gm3->addCellId(30);
	model.finish();
	BOOST_TEST_CHECKPOINT("find_methods: model completed");
	BOOST_CHECK_EQUAL(gn1, model.mesh.findGroup("GN1"));
	BOOST_CHECK(model.mesh.findGroup("DONT-EXIST") == nullptr);
}

BOOST_AUTO_TEST_CASE( combined_loadset1 ) {
    ModelConfiguration configuration;
    configuration.replaceCombinedLoadSets = true;
    configuration.logLevel = LogLevel::DEBUG;
	Model model{"inputfile", "10.3", SolverName::NASTRAN, configuration};
	const auto& loadSet1 = make_shared<LoadSet>(model, LoadSet::Type::LOAD, 1);
	model.add(loadSet1);
	const auto& loadSet3 = make_shared<LoadSet>(model, LoadSet::Type::LOAD, 3);
	model.add(loadSet3);
	model.mesh.addNode(1, 0.0, 0.0, 0.0);
	const auto& force1 = make_shared<NodalForce>(model, loadSet1, 1, 1.0);
	model.add(force1);
	const auto& force3 = make_shared<NodalForce>(model, loadSet1, 1, 3.0);
	model.add(force3);
	const auto& force2 = make_shared<NodalForce>(model, loadSet3, 1, 2.0);
	model.add(force2);
	const auto& combination = make_shared<LoadSet>(model, LoadSet::Type::LOAD, 10);
	combination->embedded_loadsets.push_back({loadSet1, 5.0});
	combination->embedded_loadsets.push_back({loadSet3, 7.0});
	BOOST_CHECK_EQUAL(combination->embedded_loadsets.size(), 2);
	model.add(combination);
	model.finish();
	BOOST_CHECK_EQUAL(model.getLoadingsByLoadSet(combination).size(), 3);
}

BOOST_AUTO_TEST_CASE(auto_analysis_linst) {
    ModelConfiguration configuration;
    configuration.autoDetectAnalysis = true;
    configuration.logLevel = LogLevel::DEBUG;
	Model model{"inputfile", "10.3", SolverName::NASTRAN, configuration};
	model.finish();
	BOOST_CHECK(model.validate());
	BOOST_CHECK_EQUAL(model.analyses.size(), 1);
	const auto& analysis = model.analyses.first();
	BOOST_CHECK(analysis->type == Analysis::Type::LINEAR_MECA_STAT);
}

BOOST_AUTO_TEST_CASE(auto_analysis_nonlin) {
    ModelConfiguration configuration;
    configuration.autoDetectAnalysis = true;
    configuration.logLevel = LogLevel::DEBUG;
	Model model{"inputfile", "10.3", SolverName::NASTRAN, configuration};
    const auto& nls = make_shared<NonLinearStrategy>(model, 1);
	model.add(nls);
	model.finish();
	BOOST_CHECK(model.validate());
	BOOST_CHECK_EQUAL(model.analyses.size(), 1);
	const auto& analysis = model.analyses.first();
	BOOST_CHECK(analysis->type == Analysis::Type::NONLINEAR_MECA_STAT);
}

BOOST_AUTO_TEST_CASE( reference_compare ) {
	Reference<LoadSet> rauto1(LoadSet::Type::LOAD, Reference<LoadSet>::NO_ID, 1);
	BOOST_CHECK(rauto1 == rauto1);
	BOOST_CHECK(!(rauto1 < rauto1));
	Reference<LoadSet> rauto2(LoadSet::Type::LOAD, Reference<LoadSet>::NO_ID, 2);
	Reference<LoadSet> r1(LoadSet::Type::LOAD, 1);
	BOOST_CHECK(r1 == r1);
	BOOST_CHECK(!(r1 != r1));
	BOOST_CHECK(!(r1 < r1));
	Reference<LoadSet> r1bis(LoadSet::Type::LOAD, 1);
	BOOST_CHECK(r1 == r1bis);
	BOOST_CHECK(!(r1 < r1bis));
	Reference<LoadSet> r2(LoadSet::Type::LOAD, 2);
	BOOST_CHECK(!(r1 == r2));
	BOOST_CHECK(r1 != r2);
	BOOST_CHECK(r1 < r2);
	Reference<LoadSet> rd1(LoadSet::Type::DLOAD, 1);
	BOOST_CHECK(!(r1 == rd1));
	BOOST_CHECK(r1 != rd1);
	BOOST_CHECK(r1 < rauto1);
}

BOOST_AUTO_TEST_CASE( reference_hash ) {
	Reference<LoadSet> rauto1(LoadSet::Type::LOAD, Reference<LoadSet>::NO_ID, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(rauto1), hash<Reference<LoadSet>>()(rauto1));
	Reference<LoadSet> rauto2(LoadSet::Type::LOAD, Reference<LoadSet>::NO_ID, 2);
	BOOST_CHECK_NE(rauto1.id, rauto2.id);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(rauto1), hash<Reference<LoadSet>>()(rauto2));
	Reference<LoadSet> r1(LoadSet::Type::LOAD, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r1));
	Reference<LoadSet> r1bis(LoadSet::Type::LOAD, 1);
	BOOST_CHECK_EQUAL(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r1bis));
	Reference<LoadSet> r2(LoadSet::Type::LOAD, 2);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(r2));
	Reference<LoadSet> rd1(LoadSet::Type::DLOAD, 1);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<LoadSet>>()(rd1));
	Reference<ConstraintSet> rc1(ConstraintSet::Type::SPC, 1);
	BOOST_CHECK_NE(hash<Reference<LoadSet>>()(r1), hash<Reference<ConstraintSet>>()(rc1));
}

BOOST_AUTO_TEST_CASE( reference_str ) {
	Reference<LoadSet> rauto1(LoadSet::Type::LOAD, Reference<LoadSet>::NO_ID, 1);
	std::ostringstream oss;
    oss << rauto1;
    std::string refstr = oss.str();
    BOOST_CHECK(not refstr.empty());
    std::string refstr2 = to_str(rauto1);
    BOOST_CHECK(not refstr2.empty());
    BOOST_CHECK_EQUAL(refstr, refstr2);
    Reference<NamedValue> rauto2(Value::Type::LIST, 1);
    BOOST_CHECK(not to_str(rauto2).empty());
    Reference<Target> rtarget(Target::Type::CONTACT_BODY, 1);
    std::ostringstream oss2;
    oss2 << rtarget;
    std::string refstr3 = oss2.str();
    BOOST_CHECK(not refstr3.empty());
    std::string refstr4 = to_str(rtarget);
    BOOST_CHECK(not refstr4.empty());
    BOOST_CHECK_EQUAL(refstr3, refstr4);
}

BOOST_AUTO_TEST_CASE(test_ineffective_assertions_removed) {
	unique_ptr<Model> model = createModelWith1HEXA8();
	const auto& analysis = make_shared<LinearMecaStat>(*model);
	const auto& nda = make_shared<NodalDisplacementAssertion>(*model, 0.0001, 50, DOF::DZ, 1., 1);
	analysis->add(nda->getReference());
	model->add(nda);

	const auto& nda2 = make_shared<NodalDisplacementAssertion>(*model, 0.0001, 51, DOF::RX, 1., 1);
	analysis->add(nda2->getReference());
	model->add(nda2);
	model->add(analysis);
	BOOST_CHECK_EQUAL(2, model->objectives.size());
	model->finish();
	BOOST_CHECK(analysis->validate());
	BOOST_CHECK(model->validate());
	BOOST_CHECK_EQUAL(model->analyses.size(), 1);
	BOOST_CHECK_EQUAL(model->objectives.size(), 1);
	const auto& assertions = model->analyses.first()->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(1));
}

BOOST_AUTO_TEST_CASE(test_rbe3_assertions_not_removed) {
	Model model{"fakemodelfortest", "10.3", SolverName::NASTRAN};
	model.mesh.addNode(100, 0.0, 0.0, 0.0);
	model.mesh.addNode(101, 1.0, 1.0, 1.0);
	const auto& rbe3 = make_shared<RBE3>(model, 100, DOFS::ALL_DOFS);
	rbe3->addRBE3Slave(101, DOFS::ALL_DOFS, 42.0);
	model.add(rbe3);
	model.addConstraintIntoConstraintSet(rbe3->getReference(), model.commonConstraintSet->getReference());
	const auto& analysis = make_shared<LinearMecaStat>(model);
	const auto& nda = make_shared<NodalDisplacementAssertion>(model, 0.0001, 100, DOF::DZ, 1., 1);
	analysis->add(nda->getReference());
	model.add(nda);

	const auto& nda2 = make_shared<NodalDisplacementAssertion>(model, 0.0001, 101, DOF::RX, 1., 1);
	analysis->add(nda2->getReference());
	model.add(nda2);
	model.add(analysis);
	BOOST_CHECK_EQUAL(2, model.objectives.size());
	model.finish();
	BOOST_CHECK(analysis->validate());
	BOOST_CHECK(model.validate());
	BOOST_CHECK_EQUAL(model.analyses.size(), 1);
	BOOST_CHECK_EQUAL(model.objectives.size(), 2);
	const auto& assertions = model.analyses.first()->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_spc_dof_remove) {
	unique_ptr<Model> model = createModelWith1HEXA8();
	const auto& analysis1 = make_shared<LinearMecaStat>(*model);
	const auto& spc = make_shared<SinglePointConstraint>(*model, DOFS::ALL_DOFS, 0.0);
    BOOST_CHECK(spc->empty());
    BOOST_CHECK(spc->ineffective());
	spc->addNodeId(50);
    BOOST_CHECK(not spc->empty());
    BOOST_CHECK(not spc->ineffective());
	model->add(spc);
	model->addConstraintIntoConstraintSet(spc->getReference(), model->commonConstraintSet->getReference());
	model->add(analysis1);
	const auto& analysis2 = make_shared<LinearMecaStat>(*model);
	model->add(analysis2);
	BOOST_CHECK_EQUAL(model->commonConstraintSet->getConstraints().size(), 1);
	int nodePosition = model->mesh.findNodePosition(50);
	BOOST_CHECK_EQUAL(spc->nodePositions().size(), 1);
	BOOST_CHECK_EQUAL(spc->getDOFSForNode(nodePosition), DOFS::ALL_DOFS);
	BOOST_CHECK_EQUAL(model->getConstraintSetsByConstraint(spc->getReference()).size(), 1);
	BOOST_CHECK_EQUAL(model->constraintSets.size(), 1);
	BOOST_TEST_CHECKPOINT("model filled");
	analysis1->removeSPCNodeDofs(*spc, nodePosition, DOF::DZ);
	BOOST_CHECK_EQUAL(spc->getDOFSForNode(nodePosition), DOFS::ALL_DOFS);
	BOOST_CHECK_EQUAL(model->commonConstraintSet->getConstraints().size(), 2);
	BOOST_CHECK_EQUAL(spc->nodePositions().size(), 0);
	for(const auto& constraint : model->commonConstraintSet->getConstraints()) {
		if (*constraint == *spc) {
			continue;
		}
		cout << *constraint;
		BOOST_CHECK_EQUAL(constraint->getDOFSForNode(nodePosition), DOFS::ALL_DOFS - DOF::DZ);
		BOOST_CHECK_EQUAL(constraint->nodePositions().size(), 1);
	}
	BOOST_CHECK_EQUAL(model->constraintSets.size(), 2);
	BOOST_CHECK_EQUAL(model->find(analysis1->getReference())->getConstraintSets().size(), 1);
	auto constraintSets2 = model->find(analysis2->getReference())->getConstraintSets();
	BOOST_CHECK_EQUAL(constraintSets2.size(), 2);
	for(const auto& constraintSet : constraintSets2) {
		if (constraintSet == model->commonConstraintSet) {
			continue;
		}
		BOOST_CHECK_EQUAL(constraintSet->getConstraints().size(), 1);
		for(const auto& constraint : constraintSet->getConstraints()) {
			cout << *constraint;
			BOOST_CHECK_EQUAL(constraint->getDOFSForNode(nodePosition), DOF::DZ);
			BOOST_CHECK_EQUAL(constraint->nodePositions().size(), 1);
		}
	}
}


BOOST_AUTO_TEST_CASE( test_cdnoanalysis )
{
    // https://github.com/Alneos/vega/issues/15
    Model model{"github_issue_15", "10.3", SolverName::NASTRAN};
    CartesianCoordinateSystem coordinateSystem(model.mesh);
    model.mesh.add(coordinateSystem);
    model.mesh.addNode(7, 0.0, 0.0, 0.0, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, coordinateSystem.getOriginalId());
    BOOST_CHECK_EQUAL(model.analyses.size(), 0);

}

BOOST_AUTO_TEST_CASE( test_globalcs_force )
{
    // https://github.com/Alneos/vega/issues/15
    Model model{"cs test model", "10.3", SolverName::NASTRAN};
    NodalForce force1(model, nullptr, 42.0, 43.0, 44.0, 0., 0., 0., Loading::NO_ORIGINAL_ID,
            Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, 0));
    BOOST_CHECK_EQUAL(force1.csref, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM);
}

BOOST_AUTO_TEST_CASE( test_indentifiable_sort )
{
    // https://github.com/Alneos/vega/issues/15
    Model model{"cs test model", "10.3", SolverName::NASTRAN};
    const auto& spc2 = make_shared<SinglePointConstraint>(model, 2);
    BOOST_CHECK(spc2->empty());
    BOOST_CHECK(spc2->ineffective());
    const auto& spc1 = make_shared<SinglePointConstraint>(model, 1);
    BOOST_CHECK(*spc1 < *spc2);
    BOOST_CHECK(spc1->getReference() < spc2->getReference());
    BOOST_CHECK_EQUAL(*spc1, *spc1);
    BOOST_CHECK(! (*spc1 == *spc2));
    const auto& spc3 = make_shared<SinglePointConstraint>(model);
    BOOST_CHECK(*spc1 < *spc3);
    BOOST_CHECK(*spc2 < *spc3);
    BOOST_CHECK_EQUAL(*spc3, *spc3);
    const auto& spc4 = make_shared<SinglePointConstraint>(model);
    BOOST_CHECK(*spc1 < *spc4);
    BOOST_CHECK(*spc2 < *spc4);
    BOOST_CHECK(*spc3 < *spc4);
    BOOST_CHECK(! (*spc1 == *spc4));
    BOOST_CHECK(! (*spc3 == *spc4));
    set<shared_ptr<Constraint>, ptrLess<Constraint>> cset;
    cset.insert(spc4);
    cset.insert(spc2);
    BOOST_CHECK_EQUAL(*(*(cset.begin())), *spc2);
    BOOST_CHECK_EQUAL(*(*(cset.rbegin())), *spc4);
    cset.insert(spc1);
    cset.insert(spc3);
    cset.insert(spc1);
    shared_ptr<Constraint> spc1b = spc1;
    cset.insert(spc1b);
    BOOST_CHECK_EQUAL(cset.size(), 4);
    BOOST_CHECK_EQUAL(*(*(cset.begin())), *spc1);
    BOOST_CHECK_EQUAL(*(*(cset.rbegin())), *spc4);
}

//____________________________________________________________________________//

