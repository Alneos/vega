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
        string solverServer, string solverCommand) :
        inputFile(inputFile), outputSolver(outputSolver), solverVersion(solverVersion), outputFile(
                outputFile), outputPath(outputPath), logLevel(logLevel), translationMode(
                translationMode), resultFile(resultFile), testTolerance(tolerance), runSolver(
                runSolver), solverServer(solverServer), solverCommand(solverCommand) {

}

const ModelConfiguration ConfigurationParameters::getModelConfiguration() const {
    if (this->outputSolver.getSolverName() == CODE_ASTER) {
        return ModelConfiguration(true, this->logLevel, true);
    } else if (this->outputSolver.getSolverName() == SYSTUS) {
        return ModelConfiguration(false, this->logLevel, true, false);
    } else if (this->outputSolver.getSolverName() == NASTRAN) {
        return ModelConfiguration(false, this->logLevel, false, false, false, false, false, false,
                false, false, false);
    } else {
        throw logic_error(" solver not implemented");
    }
}

ConfigurationParameters::~ConfigurationParameters() {
}

const boost::bimap<SolverName, string> Solver::SOLVERNAME_BY_SOLVER = assign::list_of<
        boost::bimap<SolverName, string>::relation>(NASTRAN, string("NASTRAN"))(CODE_ASTER,
        string("ASTER"))(SYSTUS, string("SYSTUS"));

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

Solver::~Solver() {
}

ModelConfiguration::ModelConfiguration(bool virtualDiscrets, LogLevel logLevel, bool createSkin,
        bool emulateLocalDisplacement, bool displayHomogeneousConstraint,
        bool emulateAdditionalMass, bool replaceCombinedLoadSets, bool removeIneffectives,
        bool partitionModel, bool replaceDirectMatrices, bool removeRedundantSpcs) :
        virtualDiscrets(virtualDiscrets), logLevel(logLevel), createSkin(createSkin), emulateLocalDisplacement(
                emulateLocalDisplacement), displayHomogeneousConstraint(
                displayHomogeneousConstraint), emulateAdditionalMass(emulateAdditionalMass), replaceCombinedLoadSets(
                replaceCombinedLoadSets), removeIneffectives(removeIneffectives), partitionModel(
                partitionModel), replaceDirectMatrices(replaceDirectMatrices), removeRedundantSpcs(
                removeRedundantSpcs) {
}

}

