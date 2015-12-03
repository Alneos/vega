/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * SystusModel.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: devel
 */

#include "SystusModel.h"
#include <boost/filesystem.hpp>

namespace vega {
namespace fs = boost::filesystem;

SystusModel::SystusModel(const vega::Model* model,
		const vega::ConfigurationParameters &configuration) :
		model(model), configuration(configuration) {
	/*this->phenomene = "MECANIQUE";*/
}

SystusModel::~SystusModel() {

}

const string SystusModel::getOutputFileName(string extension) const {

	string path = this->configuration.outputPath;
	fs::path dir(fs::absolute(path));

	string outputFileName = getName();
	fs::path result = dir / (outputFileName + extension);

	return result.string();
}

const string SystusModel::getName() const {
	string name;

	if (model->name.empty()) {
		name = "systus";
	} else {
		name = model->name;
		const size_t period_idx = name.rfind('.');
		if (string::npos != period_idx) {
			name = name.substr(0, period_idx);
		}
	}
	return name;
}

}
