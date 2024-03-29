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
 * ConfigurationParameters.h
 *
 *  Created on: Aug 30, 2013
 *      Author: devel
 */

#ifndef CONFIGURATIONPARAMETERS_H_
#define CONFIGURATIONPARAMETERS_H_

#define BOOST_SYSTEM_NO_DEPRECATED 1
#include <string>
#if defined(__GNUC__)
// Avoid tons of warnings with the following code
#pragma GCC system_header
#endif
#include <boost/filesystem.hpp>
#include <boost/bimap.hpp>

namespace vega {

namespace fs = boost::filesystem;

enum class SolverName {
    CODE_ASTER = 2,
    NASTRAN = 3,
    OPTISTRUCT = 4,
    SYSTUS = 5
};

enum class LogLevel {
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

class Solver {
private:
    friend std::ostream &operator<<(std::ostream &out, const Solver& solver); //output
    SolverName solverName;
    const static boost::bimap<SolverName, std::string> SOLVERNAME_BY_SOLVER;
public:
    Solver(SolverName name);
    SolverName getSolverName() const;
    std::string to_str() const;
    static Solver fromString(const std::string solverTypeName);
};
/**
 * Contains configuration of model operations most of them done during finish()
 * method.
 */
class ModelConfiguration {
public:
    bool virtualDiscrets = false;
    LogLevel logLevel = LogLevel::INFO;
    bool createSkin = false;
    /**
     * Some codes (like Code_Aster) need to include "skin" elements to model
     * when the skin is used to apply pressures etc.
     * while most others do not need this.
     */
    bool addSkinToModel = false;
    bool emulateLocalDisplacement = false;
    bool displayMasterSlaveConstraint = false;
    bool emulateAdditionalMass = false;
    bool replaceCombinedLoadSets = false;
    bool removeIneffectives = false;
    /**
     * Create a partition of materials and elements in the model, so that
     * materials are assigned to elements.
     * This is necessary for output languages such as Nastran.
     * TODO: this is unimplemented at the moment.
     */
    bool partitionModel = false;
    bool replaceDirectMatrices = false;
    bool removeRedundantSpcs = false;
    /**
     *  Direct Matrices (DISCRETE_0D, DISCRETE_1D, STIFFNESS_MATRIX, MASS_MATRIX, DAMPING_MATRIX) may be
     *  too big to be handled by some solver (eg Systus).
     *  This bool commands the use of Model::splitDirectMatrices() in Model::finish(), which splits the matrices
     *  into smaller ones.
     */
    bool splitDirectMatrices = false;
    int sizeDirectMatrices = 999;
    /**
     *  ElementSets of Direct Matrices (DISCRETE_0D, DISCRETE_1D, STIFFNESS_MATRIX, MASS_MATRIX, DAMPING_MATRIX)
     *  generally don't have associated cells/cellgroup. This may causes problems in generic element writer,
     *  such as in SYSTUS.
     *  This bool commands the use of Model::makeCellsFromDirectMatrices() in Model::finish(), which creates the
     *  needed cells.
     */
    bool makeCellsFromDirectMatrices = false;
    /**
     *  Build the corresponding cells to the LMPC constraints with a group, and a Rigid material.
     *  This bool commands the use of Model::makeCellsFromLMPC() in Model::finish(), which creates the
     *  needed cells.
     */
    bool makeCellsFromLMPC = false;
    /**
     *  Build the corresponding cells of RigidSets (RBAR, RBE3) with a group, and a Rigid material.
     *  This bool commands the use of Model::makeCellsFromRBE() in Model::finish(), which creates the
     *  needed cells.
     */
    bool makeCellsFromRBE = false;
    /**
     *  Build the corresponding cells of Surface Slides (Optistruct SURF) with a group.
     *  This bool commands the use of Model::makeCellsFromSurfaceSlide() in Model::finish(), which creates the
     *  needed cells.
     */
    bool makeCellsFromSurfaceSlide = false;
    /**
     *  Some elementSet can hold very general elements, acting on various DOFs of the corresponding cells.
     *  For example, Nastran PELAS1 can regroup CELAS1 cells which "spring" in various DOFS 'DXtoDX, DYtoDZ, etc)
     *  whereas Systus can only have one spring direction by 1902 Part (the translation).
     *  This bool commands the use of Model::splitElementsByDOFS in Model::finish() which split the ElementSet
     */
    bool splitElementsByDOFS = false;
    /**
     * ElementSets of Direct Matrices usually do not have an associated material, this can create issues in Code_Aster,
     * so a virtual material should be added. This must not be done in Systus instead.
     */
    bool addVirtualMaterial = false;
    /**
     * Explicitely creating cell elements defined by nodes (see Nastran BLSEG)
     */
    bool makeBoundaryCells = false;
    /**
     * Select automatically the analysis (when missing) based on features in the model
     */
    bool autoDetectAnalysis = false;
    /**
     * Remove constraint+imposed displacements
     * see SPC1+SPCD, in Aster it would cause ASSEMBLA_26
     * le noeud:  N## composante:  DX  est bloqué plusieurs fois.
     */
    bool removeConstrainedImposed = false;

    /**
     * Replace rigid elements (for example PBUSH with RIGID keywords)
     * see http://blog.altair.co.kr/wp-content/uploads/2013/08/PBUSH.pdf
     * This flag asks to use a cinematic constraint to avoid conditionment
     * problems in stiffness matrix
     */
    bool replaceRigidSegments = false;

    /**
     * Replace parametric functions (ex: ABSC coordinates, like PLOAD1 with LR option in Nastran)
     * with absolute (X,Y,Z coordinates)
     */
    bool changeParametricForceLineToAbsolute = false;

    /**
     * Workaround for Aster problem : MODELISA6_96
     * les 1 mailles imprimées ci-dessus n'appartiennent pas au modèle et pourtant elles ont été affectées dans le mot-clé facteur : !
     */
    bool alwaysUseGroupsForCells = false;

    /**
     * Create 1D discrets in the place of 0Ds
     */
    bool convert0DDiscretsInto1D = false;

    /**
     * Use MPCs instead of completely rigid constraints
     */
    bool convertCompletelyRigidsIntoMPCs = false;


    /**
     * Split parts by cell offsets (see CQUAD4 ZOFFS)
     */
    bool splitElementsByCellOffsets = false;

    /**
     * Workaround for Aster problem : MODELISA8_71
     * matériau non valide
     */
    bool alwaysUseOrthotropicMaterialsInComposites = false;

};
// TODO: THe Configuration Parameters should be much more generalized. With this,
// it's a pain in the keyboard to add options!!
class ConfigurationParameters final {
public:
    enum class TranslationMode {
        BEST_EFFORT = 0, //
        MESH_AT_LEAST = 1, //
        MODE_STRICT = 2 //
    };

    ConfigurationParameters(std::string inputFile, Solver outputSolver, std::string solverVersion =
            "", std::string outputFile = "vega", std::string outputPath = ".", LogLevel logLevel =
            LogLevel::INFO, TranslationMode translationMode = TranslationMode::BEST_EFFORT, fs::path resultFile = "",
            double testTolerance = 0.02, bool runSolver = false, bool createGraph = false, std::string solverServer = "",
            std::string solverCommand = "", bool convertCompletelyRigidsIntoMPCs = false,
            std::string systusRBE2TranslationMode = "lagrangian", double systusRBEStiffness= 0.0,
            double systusRBECoefficient= 0.0,
            std::string systusOptionAnalysis="auto", std::string systusOutputProduct="systus",
            std::vector< std::vector<int> > systusSubcases = {},
            std::string systusOutputMatrix="table", int systusSizeMatrix=9,
            std::string systusDynamicMethod="auto", std::string nastranOutputDialect="cosmic95");
    ModelConfiguration getModelConfiguration() const;

    const std::string inputFile;
    const Solver outputSolver;
    const std::string solverVersion;
    std::string outputFile;
    std::string outputPath;
    const LogLevel logLevel;
    const TranslationMode translationMode;
    const fs::path resultFile;
    const double testTolerance;
    const bool runSolver;
    const bool createGraph;
    const std::string solverServer;
    const std::string solverCommand;
    bool convertCompletelyRigidsIntoMPCs;
    const std::string systusRBE2TranslationMode;
    const double systusRBEStiffness;
    const double systusRBECoefficient;
    const std::string systusOptionAnalysis;
    const std::string systusOutputProduct;
    const std::vector< std::vector<int> > systusSubcases;
    /**
     * Output of Matrix Elements (e.g Super Elements) to 'table' (default) or 'file'
     * (needs a Systus tool to convert the output to the correct format)
     */
    const std::string systusOutputMatrix;
    /**
     * Maximum size of Systus Matrix Elements: 9 for a 'table' output, '20' for a file output.
     * Oversized matrix are splitted.
     */
    const int systusSizeMatrix;
    /**
     * Choice of Dynamic method : either a direct or a modal one
     */
    const std::string systusDynamicMethod;
    /**
     * Nastran syntax that should be written: cosmic95 or modern
     */
    const std::string nastranOutputDialect;
};

}
#endif /* CONFIGURATIONPARAMETERS_H_ */
