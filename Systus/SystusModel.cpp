/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * This file is part of Vega.
 *
 *   Vega is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Vega is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Vega.  If not, see <http://www.gnu.org/licenses/>.
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

    systusVersion= atof(configuration.solverVersion.c_str());
    if (systusVersion<2016){
        cerr << "Warning: chosen SYSTUS version ("<<systusVersion<<") is older than 2016. Upgraded to 2016.0"<<endl;
        systusVersion=2016.0;
    }
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

double SystusModel::getSystusVersion() const {
    return this->systusVersion;
}

const string SystusModel::getSystusVersionString() const {
    ostringstream os;
    os.precision(1);
    os << fixed << this->systusVersion;
    return os.str();
}


}
