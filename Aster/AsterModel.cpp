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

namespace vega {
namespace aster {

namespace fs = boost::filesystem;
using namespace std;

const map<Function::ParaName, string> AsterModel::NomParaByParaName = {
        { Function::ParaName::NO_PARA_NAME, "NO_PARA_NAME" },
        { Function::ParaName::FREQ, "FREQ" },
        { Function::ParaName::AMOR, "AMOR" },
        { Function::ParaName::STRESS, "SIGM" },
        { Function::ParaName::STRAIN, "EPSI" },
        { Function::ParaName::PARAX, "X" },
        { Function::ParaName::ABSC, "ABSC" },
};

const map<FunctionTable::Interpolation, string> AsterModel::InterpolationByInterpolation = {
        { FunctionTable::Interpolation::LINEAR, "LIN" },
        { FunctionTable::Interpolation::LOGARITHMIC, "LOG" },
        { FunctionTable::Interpolation::CONSTANT, "" },
        { FunctionTable::Interpolation::NONE, "NON" },
};

const map<FunctionTable::Interpolation, string> AsterModel::ProlongementByInterpolation = {
        { FunctionTable::Interpolation::LINEAR, "LINEAIRE" },
        { FunctionTable::Interpolation::LOGARITHMIC, "" },
        { FunctionTable::Interpolation::CONSTANT, "CONSTANT" },
        { FunctionTable::Interpolation::NONE, "EXCLU" },
};

const vector<string> AsterModel::DofByPosition = {
        "DX",
        "DY",
        "DZ",
        "DRX",
        "DRY",
        "DRZ"
};

AsterModel::AsterModel(vega::Model& model, const vega::ConfigurationParameters &configuration) :
        model(model), configuration(configuration), phenomene{"MECANIQUE"} {
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
    string result = "stable";
    if (!this->configuration.solverVersion.empty()) {
        result = this->configuration.solverVersion;
    }
    return result;
}

double AsterModel::getMemjeveux() const {
    double mem = 2048.0* static_cast<int>(model.mesh.countNodes()) / 300000.0;
    mem = max<double>(512., mem);
    mem = min<double>(12000.0, mem);
    return mem;
}
double AsterModel::getTpmax() const {
    double time = 3600.0 * static_cast<int>(model.mesh.countNodes()) / 300000.0;
    time = max<double>(360.,time) * max(1, static_cast<int>(model.analyses.size()));
    return time;
}

const string AsterModel::getModelisations(const shared_ptr<ElementSet> elementSet) const {
    ModelType modelType = elementSet->getModelType();
    string result;
    switch (elementSet->type) {
    case ElementSet::Type::SKIN:
    case ElementSet::Type::CONTINUUM: {
        if (modelType == ModelType::TRIDIMENSIONAL_SI and \
            not model.analyses.contains(Analysis::Type::NONLINEAR_MECA_STAT)) {
            result = "('3D', '3D_SI')";
        } else if (modelType == ModelType::TRIDIMENSIONAL_SI and \
                   model.analyses.contains(Analysis::Type::NONLINEAR_MECA_STAT)) {
            // Workaround for problem ELEMENTS4_73
            // Les comportements écrits en configuration de référence ne sont pas disponibles     !
            // sur les éléments linéaires pour la modélisation 3D_SI.                             !
            // Pour contourner le problème et passer à un comportement en configuration actuelle, !
            // ajoutez un état initial nul au calcul.
            result = "('3D',)";
        } else if (modelType == ModelType::PLANE_STRESS) {
            result = "('C_PLAN',)";
        } else if (modelType == ModelType::PLANE_STRAIN) {
            result = "('D_PLAN',)";
        } else {
            result = "('3D',)";
        }
        break;
    }
    case ElementSet::Type::SHELL:
    case ElementSet::Type::COMPOSITE: {
        if (model.analyses.contains(Analysis::Type::LINEAR_MECA_STAT)) {
            // Some tests are better with DST "shell", other are worst "nastrancoverage cantilever tria"
            // Also Q4G only gives similar results to nastran in linear static (but non corresponding ones in modal)
            result = "('DKT',)"; // DST // Q4G // Q4GG // DKTG
        } else {
            // Workaround for MECANONLINE_3
            // Erreur utilisateur :                                                                                           !
            //   Vous essayez de faire un calcul non-linéaire mécanique ou un post-traitement sur un modèle dont les éléments !
            //   ne sont pas programmés pour cela.                                                                            !
            //   On arrête le calcul.                                                                                         !
            // Risques & conseils :                                                                                           !
            //   Vous devriez changer de modélisation.

            // Workaround for CALCUL_37
            // Le TYPE_ELEMENT MEQ4QU4  ne sait pas encore calculer l'option:  RIGI_MECA_GE.
            result = "('DKT',)";
        }
        /*??
         coque_3D.modelisations = ("COQUE_3D",)
         */
        break;
    }
    case ElementSet::Type::CIRCULAR_SECTION_BEAM:
    case ElementSet::Type::TUBE_SECTION_BEAM:
    case ElementSet::Type::RECTANGULAR_SECTION_BEAM:
    case ElementSet::Type::I_SECTION_BEAM:
    case ElementSet::Type::GENERIC_SECTION_BEAM: {
        const auto& beam = static_pointer_cast<Beam>(elementSet);
        if (not model.needsLargeDisplacements()) {
            switch (beam->beamModel) {
            case Beam::BeamModel::EULER: {
                result = "('POU_D_E',)";
                break;
            }
            case Beam::BeamModel::TIMOSHENKO: {
                result = "('POU_D_T',)";
                break;
            }
            case Beam::BeamModel::TRUSS: {
                result = "('BARRE',)";
                break;
            }
            default:
                throw logic_error("Beam model not yet implemented");
            }
        } else {
            result = "('POU_D_T_GD',)";
        }
        break;
    }
    case ElementSet::Type::STRUCTURAL_SEGMENT:
    case ElementSet::Type::SCALAR_SPRING:
    case ElementSet::Type::DISCRETE_0D:
    case ElementSet::Type::DISCRETE_1D: {
        const auto& discret = static_pointer_cast<Discrete>(elementSet);
        if (discret->hasRotations()) {
            result = "('DIS_TR',)";
        } else {
            result = "('DIS_T',)";
        }
        break;
    }
    case ElementSet::Type::NODAL_MASS: {
        const auto& mass = static_pointer_cast<NodalMass>(elementSet);
        if (mass->hasRotations()) {
            result = "('DIS_TR',)";
        } else {
            result = "('DIS_T',)";
        }
        break;
    }
    default:
        throw logic_error("AFFE_MODELE unsupported " + to_str(*elementSet));
    }

    return result;
}

} // namespace aster
} //namespace vega
