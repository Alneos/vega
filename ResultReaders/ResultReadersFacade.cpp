/*
 * ResultReadersFacade.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: devel
 */

#include "ResultReadersFacade.h"
#include "CSVResultReader.h"
#include "F06Parser.h"
#include "../Abstract/ConfigurationParameters.h"
#include <ciso646>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <stdexcept>

namespace vega {
namespace result {

namespace fs = boost::filesystem;
using namespace std;

ResultReadersFacade::ResultReadersFacade() {

}

shared_ptr<ResultReader> ResultReadersFacade::getResultReader(
		const ConfigurationParameters& configuration) {
	shared_ptr<ResultReader> result;
	if (!configuration.resultFile.empty()) {
		fs::path ext = configuration.resultFile.extension();
		if (!ext.empty()) {
			string ext_str = boost::algorithm::to_lower_copy(ext.string());
			if (ext_str == ".f06") {
				result = shared_ptr<ResultReader>(new F06Parser());
			} else if (ext_str == ".csv") {
				result = shared_ptr<ResultReader>(new CSVResultReader());
			} else {
				cerr << "Can't determine the type of the result file. " << endl;
				cerr << "allowed types are .f06, .csv" << endl;
				throw invalid_argument("Invalid result file type");
			}

		}
	}

	return result;
}

}
} /* namespace vega */

