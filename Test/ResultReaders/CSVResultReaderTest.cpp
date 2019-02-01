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
using namespace vega;

BOOST_AUTO_TEST_CASE(read_tut_01_csv) {
	string testLocation(PROJECT_BASE_DIR "/testdata/unitTest/resultReaders/tut01.csv");
	boost::filesystem::path resultPath = fs::path(testLocation).make_preferred();

	ConfigurationParameters params{string(""), SolverName::CODE_ASTER, string(""), string(""),
			string("."), LogLevel::INFO,
			ConfigurationParameters::TranslationMode::BEST_EFFORT, resultPath};
	result::CSVResultReader reader;
	unique_ptr<Model> model = make_unique<Model>("tut_01", "", SolverName::CODE_ASTER,
					params.getModelConfiguration());
	reader.add_assertions(params, *model);
	BOOST_CHECK_EQUAL(model->objectives.size(), 126);
}

