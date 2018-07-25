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
 * AsterModel.cpp
 *
 *  Created on: Aug 30, 2013
 *      Author: devel
 */

#include "AsterModel.h"
#include "../Abstract/Model.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

namespace vega {
namespace aster {

namespace fs = boost::filesystem;
using namespace std;

const map<NamedValue::ParaName, string> AsterModel::NomParaByParaName = {
        { NamedValue::NO_PARA_NAME, "'NO_PARA_NAME'" },
        { NamedValue::FREQ, "'FREQ'" },
        { NamedValue::AMOR, "'AMOR'" },
        { NamedValue::PARAX, "'X'" }
};

const map<FunctionTable::Interpolation, string> AsterModel::InterpolationByInterpolation = {
        { FunctionTable::LINEAR, "'LIN'" },
        { FunctionTable::LOGARITHMIC, "'LOG'" },
        { FunctionTable::CONSTANT, "''" },
        { FunctionTable::NONE, "'NON'" },
};

const map<FunctionTable::Interpolation, string> AsterModel::ProlongementByInterpolation = {
        { FunctionTable::LINEAR, "'LINEAIRE'" },
        { FunctionTable::LOGARITHMIC, "''" },
        { FunctionTable::CONSTANT, "'CONSTANT'" },
        { FunctionTable::NONE, "'EXCLU'" },
};

const vector<string> AsterModel::DofByPosition = {
        "DX",
        "DY",
        "DZ",
        "DRX",
        "DRY",
        "DRZ"
};

AsterModel::AsterModel(const vega::Model& model, const vega::ConfigurationParameters &configuration) :
        model(model), configuration(configuration) {
    this->phenomene = "MECANIQUE";
}

AsterModel::~AsterModel() {

}

const string AsterModel::getOutputFileName(string extension, bool absolute) const {

    string outputFileName;
    if (model.name.empty()) {
        outputFileName = "code_aster";
    } else {
        outputFileName = model.name;
        const size_t period_idx = outputFileName.rfind('.');
        if (string::npos != period_idx) {
            outputFileName = outputFileName.substr(0, period_idx);
        }
    }

    string result = outputFileName + extension;
    if (absolute){
        string path = this->configuration.outputPath;
        result = (fs::absolute(path) / result).string();
    }
    return result;
}

const string AsterModel::getAsterVersion() const {
    string result = "STABLE";
    if (!this->configuration.solverVersion.empty()) {
        result = this->configuration.solverVersion;
    }
    return result;
}

double AsterModel::getMemjeveux() const {
    double mem = 2048.0* model.mesh->countNodes() / 300000.0;
    mem = max<double>(128., mem);
    mem = min<double>(12000.0, mem);
    return mem;
}
double AsterModel::getTpmax() const {
    double time = 3600.0 * model.mesh->countNodes() / 300000.0;
    time = max<double>(360.,time) * max(1,model.analyses.size());
    return time;
}

const string AsterModel::getModelisations(const shared_ptr<ElementSet> elementSet) const {
    ModelType modelType = elementSet->getModelType();
    string result;
    switch (elementSet->type) {
    case ElementSet::CONTINUUM:
        if (modelType == ModelType::TRIDIMENSIONAL_SI) {
            result = "('3D', '3D_SI')";
        } else if (modelType == ModelType::PLANE_STRESS) {
            result = "('C_PLAN',)";
        } else if (modelType == ModelType::PLANE_STRAIN) {
            result = "('D_PLAN',)";
        } else {
            result = "('3D',)";
        }
        break;
    case ElementSet::SHELL:
    case ElementSet::COMPOSITE:
        result = "('DKT',)";
        /*??
         coque_3D.modelisations = ("COQUE_3D",)
         */
        break;
    case ElementSet::CIRCULAR_SECTION_BEAM:
    case ElementSet::RECTANGULAR_SECTION_BEAM:
    case ElementSet::I_SECTION_BEAM:
    case ElementSet::GENERIC_SECTION_BEAM: {
        Beam::BeamModel beamModel = (static_pointer_cast<Beam>(elementSet))->beamModel;
        double largeDisp = 0;
        auto it = model.parameters.find(Model::LARGE_DISPLACEMENTS);
        if (it != model.parameters.end()) {
            largeDisp = it->second;
        }
        if (is_zero(largeDisp)) {
            if (beamModel == Beam::EULER) {
                result = "('POU_D_E',)";
            } else {
                result = "('POU_D_T',)";
            }
        } else {
            result = "('POU_D_T_GD',)";
        }
    }
        break;
    case ElementSet::DISCRETE_0D:
    case ElementSet::DISCRETE_1D: {
        shared_ptr<DiscretePoint> discret = (static_pointer_cast<DiscretePoint>(elementSet));
        if (discret->hasRotations()) {
            result = "('DIS_TR',)";
        } else {
            result = "('DIS_T',)";
        }
        break;
    }
    case ElementSet::NODAL_MASS: {
        result = "('DIS_TR',)";
        break;
    }
    default:
        throw logic_error("AFFE_MODELE unsupported" + to_str(*elementSet));
    }

    return result;
}

}
} //namespace vega
