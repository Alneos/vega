/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * F06Parser_tests.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: devel
 */

#define BOOST_TEST_MODULE nastran_f06_tests
#include "build_properties.h"
#include "../../Abstract/Model.h"
#include "../../ResultReaders/F06Parser.h"
#include <boost/test/unit_test.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/assign.hpp>
#include <string>

using namespace std;
using namespace vega;
using vega::result::F06Parser;

BOOST_AUTO_TEST_CASE(nastran_f06_parsing) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/nastran/alneos/rbar1mod/rbar1mod.f06");
	ConfigurationParameters confParams("inputFile", SolverName::CODE_ASTER, "..", "vega", ".",
			LogLevel::DEBUG, ConfigurationParameters::TranslationMode::BEST_EFFORT, testLocation, 0.0003);
	F06Parser f06parser;

	unique_ptr<Model> model = make_unique<Model>("mname", "unknown", SolverName::NASTRAN);
	model->mesh.addNode(1, 2, 3, 4);
	model->mesh.addNode(2, 2, 3, 4);
	model->mesh.addNode(3, 2, 3, 4);
	model->mesh.addNode(4, 2, 3, 4);
	model->mesh.addNode(5, 2, 3, 4);
	model->mesh.addNode(6, 2, 3, 4);
	model->mesh.addNode(10, 2, 3, 4);
	model->mesh.addCell(1, CellType::QUAD4, {1, 2, 3, 4});
	model->mesh.addCell(2, CellType::QUAD4, {1, 4, 5, 6});
	shared_ptr<CellGroup> cn1 = model->mesh.createCellGroup("GM1");
	cn1->addCellId(1);
	cn1->addCellId(2);
	const auto& shell = make_shared<Shell>(*model, 1.1);
	shell->assignCellGroup(cn1);
	model->add(shell);
	//node 10 outside mesh, number of assertion = 6 nodes * 6 dofs
	model->add(make_shared<LinearMecaStat>(*model, "", 1));
	model->add(make_shared<LinearMecaStat>(*model, "", 2));

	BOOST_TEST_CHECKPOINT("Before Parse");
	f06parser.add_assertions(confParams, *model);
	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::Type::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(24));

	model->finish();
	//ineffective assertions are removed by model->finish()
	vector<shared_ptr<Assertion>> assertions2 = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions2.size(), static_cast<size_t>(24));
	bool found = false;
	for (const auto& assertion : assertions) {

		BOOST_CHECK(assertion->type == Objective::Type::NODAL_DISPLACEMENT_ASSERTION);
		NodalDisplacementAssertion & nodalDispAssertion =
				dynamic_cast<NodalDisplacementAssertion&>(*assertion);
		int nodeId = model->mesh.findNodeId(nodalDispAssertion.nodePosition);
		if (nodeId == 3 && nodalDispAssertion.dof == DOF::DX) {
			found = true;
			BOOST_CHECK_EQUAL(nodalDispAssertion.value, 4.901961E-01);
		}
	}
	BOOST_CHECK_EQUAL(found, true);
}

BOOST_AUTO_TEST_CASE(node_not_in_elements) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/nastran/caw/prob6/prob6.f06");
	ConfigurationParameters confParams{"inputFile", SolverName::CODE_ASTER, "..", "vega", ".",
			LogLevel::INFO, ConfigurationParameters::TranslationMode::BEST_EFFORT, testLocation, 0.0003};
	F06Parser f06parser;

	unique_ptr<Model> model = make_unique<Model>("mname", "unknown", SolverName::NASTRAN);
	model->mesh.addNode(1, 2, 3, 4);
	model->mesh.addNode(2, 2.2, 3.2, 4.2);
	model->mesh.addNode(3, 2.3, 3.3, 4.3);
	model->mesh.addNode(4, 2, 3, 4);
	model->mesh.addNode(5, 2, 6.6, 4);
	model->mesh.addNode(6, 2, 3, 4);
	model->mesh.addNode(10, 2.1, 3.1, 4.1);

	model->add(make_shared<LinearMecaStat>(*model, "", 1));
	model->add(make_shared<LinearMecaStat>(*model, "", 2));

	BOOST_TEST_CHECKPOINT("Before Parse");
	f06parser.add_assertions(confParams, *model);
	BOOST_TEST_CHECKPOINT("Before Finish");
	model->finish();

	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::Type::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);

	//assertion skipped because nodes are not in elements
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(0));

}

BOOST_AUTO_TEST_CASE(test_4a) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/nastran/alneos/test4a/test4a.f06");
	ConfigurationParameters confParams{"inputFile", SolverName::CODE_ASTER, "..", "vega", ".",
			LogLevel::INFO, ConfigurationParameters::TranslationMode::BEST_EFFORT, testLocation, 0.0003};

	unique_ptr<Model> model = make_unique<Model>("unittest4a", "unknown", SolverName::NASTRAN);
	model->mesh.addNode(1, 2, 3, 4);
	model->mesh.addNode(2, 2.2, 3.2, 4.2);
	model->mesh.addNode(3, 2.3, 3.3, 4.3);
	model->mesh.addNode(4, 2, 3, 4);
	model->mesh.addNode(5, 2, 6.6, 4);
	model->mesh.addNode(6, 2, 3, 4);
	model->mesh.addNode(7, 2.1, 3.1, 4.1);
	model->mesh.addNode(8, 2.2, 3.1, 4.1);
	model->mesh.addNode(9, 2.3, 3.1, 4.1);
	model->mesh.addCell(1, CellType::HEXA8, { 1, 2, 3, 4, 5, 6, 7, 8 });
	model->mesh.addCell(2, CellType::POINT1, { 9 });
	shared_ptr<CellGroup> cn1 = model->mesh.createCellGroup("GM1");
	cn1->addCellId(1);
	const auto& continuum = make_shared<Continuum>(*model, ModelType::TRIDIMENSIONAL_SI, 1);
	continuum->assignCellGroup(cn1);
	model->add(continuum);
	shared_ptr<CellGroup> cn2 = model->mesh.createCellGroup("GM2");
	cn2->addCellId(2);
	const auto& discrete = make_shared<DiscretePoint>(*model);
	discrete->assignCellGroup(cn2);
	model->add(discrete);
	model->add(make_shared<LinearMecaStat>(*model, "", 1));

	BOOST_TEST_CHECKPOINT("Before Parse");
	F06Parser f06parser;
	f06parser.add_assertions(confParams, *model);

	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::Type::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);
	//before finish() in the model are present all the assertion found in the f06 file
	vector<shared_ptr<Assertion>> all_assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(all_assertions.size(), static_cast<size_t>(54));

	model->finish();
	//assertion deleted because nodes are not in elements
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(30));

}

BOOST_AUTO_TEST_CASE(test_modal_disp) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/unitTest/resultReaders/modal_disp.f06");
	ConfigurationParameters confParams{"inputFile", SolverName::CODE_ASTER, "..", "vega", ".",
			LogLevel::INFO, ConfigurationParameters::TranslationMode::BEST_EFFORT, testLocation, 0.0003};

	unique_ptr<Model> model = make_unique<Model>("test_modal_disp", "unknown", SolverName::NASTRAN);
	model->mesh.addNode(1, 2, 3, 4);
	model->mesh.addNode(2, 2.2, 3.2, 4.2);
	model->mesh.addNode(3, 2.3, 3.3, 4.3);
	model->mesh.addNode(4, 2, 3, 4);
	model->mesh.addNode(5, 2, 6.6, 4);
	model->mesh.addNode(6, 2, 3, 4);
	model->mesh.addNode(7, 2.1, 3.1, 4.1);
	model->mesh.addNode(8, 2.2, 3.1, 4.1);
	model->mesh.addNode(9, 2.3, 3.1, 4.1);
	model->mesh.addCell(1, CellType::HEXA8, { 1, 2, 3, 4, 5, 6, 7, 8 });
	model->mesh.addCell(2, CellType::POINT1, { 9 });

	model->mesh.addNode(5814, 2, 3, 4);
	model->mesh.addNode(7651, 2.2, 3.2, 4.2);
	model->mesh.addNode(8456, 2.3, 3.3, 4.3);
	model->mesh.addNode(8487, 2, 3, 4);
	model->mesh.addNode(17554, 2, 6.6, 4);
	model->mesh.addNode(17575, 2, 3, 4);
	model->mesh.addNode(18046, 2.1, 3.1, 4.1);
	model->mesh.addCell(2, CellType::HEXA8, { 5814, 7961, 8469, 8483, 17624, 17672, 18049, 8 });
	shared_ptr<CellGroup> cn1 = model->mesh.createCellGroup("GM1");
	cn1->addCellId(1);
	const auto& continuum = make_shared<Continuum>(*model, ModelType::TRIDIMENSIONAL_SI, 1);
	continuum->assignCellGroup(cn1);
	model->add(continuum);
	shared_ptr<CellGroup> cn2 = model->mesh.createCellGroup("GM2");
	cn2->addCellId(2);
	const auto& discrete = make_shared<DiscretePoint>(*model);
	discrete->assignCellGroup(cn2);
	model->add(discrete);
    const auto& bandRange = make_shared<BandRange>(*model, 0.0, 10, 1000);
    const auto& frequencySearch = make_shared<FrequencySearch>(*model, FrequencySearch::FrequencyType::BAND, *bandRange);
    model->add(bandRange);
    model->add(frequencySearch);
	model->add(make_shared<LinearModal>(*model, *frequencySearch, "", 1));
	model->add(make_shared<LinearMecaStat>(*model, "", 2));

	BOOST_TEST_CHECKPOINT("Before Parse");
	F06Parser f06parser;
	f06parser.add_assertions(confParams, *model);

	shared_ptr<LinearModal> linearModal1 = dynamic_pointer_cast<LinearModal>(
			model->find(Reference<Analysis>(Analysis::Type::LINEAR_MODAL, 1)));
	BOOST_ASSERT(linearModal1!=nullptr);
	vector<shared_ptr<Assertion>> all_assertions1 = linearModal1->getAssertions();
	BOOST_CHECK_EQUAL(all_assertions1.size(), static_cast<size_t>(10));

	shared_ptr<LinearMecaStat> linearStatic2 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::Type::LINEAR_MECA_STAT, 2)));
	BOOST_ASSERT(linearStatic2!=nullptr);
	vector<shared_ptr<Assertion>> all_assertions2 = linearStatic2->getAssertions();
	BOOST_CHECK_EQUAL(all_assertions2.size(), static_cast<size_t>(42));
	//before finish() in the model are present all the assertion found in the f06 file


	model->finish();
	vector<shared_ptr<Assertion>> assertions = linearModal1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(10));

}

