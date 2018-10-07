/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * ConfigurationParameters.cpp
 *
 *  Created on: Aug 30, 2013
 *      Author: devel
 */

#include <string>
#include "ConfigurationParameters.h"
#include <boost/algorithm/string.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>

using namespace std;
namespace assign = boost::assign;

namespace vega {

ConfigurationParameters::ConfigurationParameters(string inputFile, Solver outputSolver,
        string solverVersion, string outputFile, string outputPath, LogLevel logLevel,
        TranslationMode translationMode, fs::path resultFile, double tolerance, bool runSolver,
        string solverServer, string solverCommand,
        string systusRBE2TranslationMode, double systusRBE2Rigidity, double systusRBELagrangian,
        string systusOptionAnalysis, string systusOutputProduct, vector<vector<int> > systusSubcases,
        string systusOutputMatrix, int systusSizeMatrix, string systusDynamicMethod) :
                inputFile(inputFile), outputSolver(outputSolver), solverVersion(solverVersion), outputFile(
                outputFile), outputPath(outputPath), logLevel(logLevel), translationMode(
                translationMode), resultFile(resultFile), testTolerance(tolerance), runSolver(
                runSolver), solverServer(solverServer), solverCommand(solverCommand),
                systusRBE2TranslationMode(systusRBE2TranslationMode), systusRBE2Rigidity(systusRBE2Rigidity),
                systusRBELagrangian(systusRBELagrangian), systusOptionAnalysis(systusOptionAnalysis),
                systusOutputProduct(systusOutputProduct), systusSubcases(systusSubcases),
                systusOutputMatrix(systusOutputMatrix), systusSizeMatrix(systusSizeMatrix), systusDynamicMethod(systusDynamicMethod)
{

}

const ModelConfiguration ConfigurationParameters::getModelConfiguration() const {
    ModelConfiguration configuration;
    configuration.logLevel = this->logLevel;
    if (this->outputSolver.getSolverName() == CODE_ASTER) {
        configuration.virtualDiscrets = true;
        configuration.createSkin = true;
        configuration.emulateLocalDisplacement = true;
        configuration.emulateAdditionalMass = true;
        configuration.replaceCombinedLoadSets = true;
        configuration.removeIneffectives = true;
        configuration.replaceDirectMatrices = true;
        configuration.removeRedundantSpcs = true;
        configuration.addVirtualMaterial = true;
    } else if (this->outputSolver.getSolverName() == SYSTUS) {
        configuration.createSkin = true;
        configuration.emulateAdditionalMass = true;
        configuration.replaceCombinedLoadSets = true;
        configuration.removeIneffectives = true;
        configuration.removeRedundantSpcs = true;
        configuration.splitDirectMatrices = true;
        configuration.sizeDirectMatrices = this->systusSizeMatrix;
        configuration.makeCellsFromDirectMatrices = true;
        configuration.makeCellsFromLMPC = true;
        configuration.makeCellsFromRBE = true;
        configuration.splitElementsByDOFS = true;
    } else if (this->outputSolver.getSolverName() == NASTRAN) {
        // default should be always false
    } else {
        throw logic_error(" solver not implemented");
    }
    return configuration;
}

ConfigurationParameters::~ConfigurationParameters() {
}

const boost::bimap<SolverName, string> Solver::SOLVERNAME_BY_SOLVER = assign::list_of<
        boost::bimap<SolverName, string>::relation>(NASTRAN, string("NASTRAN"))(CODE_ASTER,
        string("ASTER"))(SYSTUS, string("SYSTUS"))(OPTISTRUCT, string("OPTISTRUCT"));

ostream &operator<<(ostream &out, const Solver& solver) {
    out << Solver::SOLVERNAME_BY_SOLVER.left.find(solver.solverName)->second;
    return out;
}

Solver::Solver(SolverName name) :
        solverName(name) {
}

SolverName Solver::getSolverName() const {
    return solverName;
}

Solver Solver::fromString(string name) {
    string normalizedSolverName = boost::to_upper_copy(boost::trim_copy(name));
    if (normalizedSolverName.empty()) {
        throw invalid_argument("SolverTypeName empty");
    }
    auto solverIterator = Solver::SOLVERNAME_BY_SOLVER.right.find(normalizedSolverName);
    if (solverIterator == Solver::SOLVERNAME_BY_SOLVER.right.end()) {
        throw invalid_argument(string("Solver name : ") + name + " not recognized.");
    }
    return Solver(solverIterator->second);
}

ModelConfiguration::ModelConfiguration() {
}

}

