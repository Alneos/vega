/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */
#define BOOST_TEST_MODULE CSVResultReaderTest

#include "build_properties.h"
#include "../../ResultReaders/CSVResultReader.h"
#include "../../Abstract/ConfigurationParameters.h"
#include "../../Abstract/Model.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <string>

using namespace std;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_CASE(read_tut_01_csv) {
	string testLocation(PROJECT_BASE_DIR "/testdata/unitTest/resultReaders/tut01.csv");
	fs::path resultPath = fs::path(testLocation).make_preferred();

	vega::ConfigurationParameters params(string(""), vega::CODE_ASTER, string(""), string(""),
			string("."), vega::LogLevel::INFO,
			vega::ConfigurationParameters::TranslationMode::BEST_EFFORT, resultPath);
	vega::result::CSVResultReader reader;
	shared_ptr<vega::Model> model = make_shared<vega::Model>("tut_01", "", vega::SolverName::CODE_ASTER,
					params.getModelConfiguration());
	reader.add_assertions(params, model);
	BOOST_CHECK_EQUAL(model->objectives.size(), 126);
}

