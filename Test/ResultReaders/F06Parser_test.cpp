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
//#include <valgrind/memcheck.h>
#include <boost/pointer_cast.hpp>
#include <boost/assign.hpp>
#include <string>

using namespace std;
using namespace vega;
using boost::assign::list_of;
using vega::result::F06Parser;

BOOST_AUTO_TEST_CASE(nastran_f06_parsing) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/nastran/alneos/rbar1mod/rbar1mod.f06");
	ConfigurationParameters confParams("inputFile", vega::CODE_ASTER, "..", "vega", ".",
			LogLevel::DEBUG, ConfigurationParameters::BEST_EFFORT, testLocation, 0.0003);
	F06Parser f06parser;

	shared_ptr<Model> model = make_shared<Model>("mname", "unknown", NASTRAN, true);
	model->mesh->addNode(1, 2, 3, 4);
	model->mesh->addNode(2, 2, 3, 4);
	model->mesh->addNode(3, 2, 3, 4);
	model->mesh->addNode(4, 2, 3, 4);
	model->mesh->addNode(5, 2, 3, 4);
	model->mesh->addNode(6, 2, 3, 4);
	model->mesh->addNode(10, 2, 3, 4);
	model->mesh->addCell(1, CellType::QUAD4, list_of<int>(1)(2)(3)(4));
	model->mesh->addCell(2, CellType::QUAD4, list_of<int>(1)(4)(5)(6));
	shared_ptr<CellGroup> cn1 = model->mesh->createCellGroup("GM1");
	cn1->addCellId(1);
	cn1->addCellId(2);
	Shell shell(*model, 1.1);
	shell.assignCellGroup(cn1);
	model->add(shell);
	//node 10 outside mesh, number of assertion = 6 nodes * 6 dofs
	model->add(LinearMecaStat(*model, "", 1));
	model->add(LinearMecaStat(*model, "", 2));

	BOOST_TEST_CHECKPOINT("Before Parse");
	f06parser.add_assertions(confParams, model);
	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(24));

	model->finish();
	//ineffective assertions are removed by model.finish()
	vector<shared_ptr<Assertion>> assertions2 = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions2.size(), static_cast<size_t>(24));
	bool found = false;
	for (auto assertion : assertions) {

		BOOST_CHECK_EQUAL(assertion->type, Assertion::NODAL_DISPLACEMENT_ASSERTION);
		NodalDisplacementAssertion & nodalDispAssertion =
				dynamic_cast<NodalDisplacementAssertion&>(*assertion);
		int nodeId = model->mesh->findNode(nodalDispAssertion.nodePosition).id;
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
	ConfigurationParameters confParams("inputFile", vega::CODE_ASTER, "..", "vega", ".",
			LogLevel::INFO, ConfigurationParameters::BEST_EFFORT, testLocation, 0.0003);
	F06Parser f06parser;

	shared_ptr<Model> model = make_shared<Model>("mname", "unknown", NASTRAN, true);
	model->mesh->addNode(1, 2, 3, 4);
	model->mesh->addNode(2, 2.2, 3.2, 4.2);
	model->mesh->addNode(3, 2.3, 3.3, 4.3);
	model->mesh->addNode(4, 2, 3, 4);
	model->mesh->addNode(5, 2, 6.6, 4);
	model->mesh->addNode(6, 2, 3, 4);
	model->mesh->addNode(10, 2.1, 3.1, 4.1);

	model->add(LinearMecaStat(*model, "", 1));
	model->add(LinearMecaStat(*model, "", 2));

	BOOST_TEST_CHECKPOINT("Before Parse");
	f06parser.add_assertions(confParams, model);
	BOOST_TEST_CHECKPOINT("Before Finish");
	model->finish();

	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);

	//assertion skipped because nodes are not in elements
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(0));

}

BOOST_AUTO_TEST_CASE(test_4a) {

	string testLocation(
	PROJECT_BASE_DIR "/testdata/nastran/alneos/test4a/test4a.f06");
	ConfigurationParameters confParams("inputFile", vega::CODE_ASTER, "..", "vega", ".",
			LogLevel::INFO, ConfigurationParameters::BEST_EFFORT, testLocation, 0.0003);

	shared_ptr<Model> model = make_shared<Model>("mname", "unknown", NASTRAN, true);
	model->mesh->addNode(1, 2, 3, 4);
	model->mesh->addNode(2, 2.2, 3.2, 4.2);
	model->mesh->addNode(3, 2.3, 3.3, 4.3);
	model->mesh->addNode(4, 2, 3, 4);
	model->mesh->addNode(5, 2, 6.6, 4);
	model->mesh->addNode(6, 2, 3, 4);
	model->mesh->addNode(7, 2.1, 3.1, 4.1);
	model->mesh->addNode(8, 2.2, 3.1, 4.1);
	model->mesh->addNode(9, 2.3, 3.1, 4.1);
	model->mesh->addCell(1, CellType::HEXA8, { 1, 2, 3, 4, 5, 6, 7, 8 });
	model->mesh->addCell(2, CellType::POINT1, { 9 });
	shared_ptr<CellGroup> cn1 = model->mesh->createCellGroup("GM1");
	cn1->addCellId(1);
	Continuum continuum(*model, &ModelType::TRIDIMENSIONAL_SI, 1);
	continuum.assignCellGroup(cn1);
	model->add(continuum);
	shared_ptr<CellGroup> cn2 = model->mesh->createCellGroup("GM2");
	cn2->addCellId(2);
	DiscretePoint discrete(*model, { });
	discrete.assignCellGroup(cn2);
	model->add(discrete);
	model->add(LinearMecaStat(*model, "", 1));

	BOOST_TEST_CHECKPOINT("Before Parse");
	F06Parser f06parser;
	f06parser.add_assertions(confParams, model);

	shared_ptr<LinearMecaStat> linearMecaStat1 = dynamic_pointer_cast<LinearMecaStat>(
			model->find(Reference<Analysis>(Analysis::LINEAR_MECA_STAT, 1)));
	BOOST_ASSERT(linearMecaStat1!=nullptr);
	//before finish() in the model are present all the assertion found in the f06 file
	vector<shared_ptr<Assertion>> all_assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(all_assertions.size(), static_cast<size_t>(54));

	model->finish();
	//assertion deleted because nodes are not in elements
	vector<shared_ptr<Assertion>> assertions = linearMecaStat1->getAssertions();
	BOOST_CHECK_EQUAL(assertions.size(), static_cast<size_t>(30));

}

