/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * SystusBuilder.h
 *
 *  Created on: 2 octobre 2013
 *      Author: devel
 */

#ifndef SYSTUSBUILDER_H_
#define SYSTUSBUILDER_H_

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "../Abstract/ConfigurationParameters.h"
#include "../Abstract/CoordinateSystem.h"
#include "SystusModel.h"
#include "SystusAsc.h"

namespace vega {
namespace systus {

/**
 *  This enum class regroups all physical caracteristics stocked in Systus Materials.
 *  The order is important, as it follows the Systus order, thus allowing us
 *  to write materials with good keys. (See SNOMAT.FMT in Systus Code Source)
 */
enum class SMF {
    ID,
    PSI, THETA, PHI, RHO, E, NU, G, LX, LY, LZ,
    S, AY, AZ, IX, IY, IZ, LOCAL, XX, YY, ZZ,
    H, SURF, INTEG, PRESS, TEMP, TABLE, VARI, DIAME, DIAH, UNIT,
    KX, KY, KZ, C, ALPHA, BETA, R, IG, FT, SECT,
    VY, VPY, VZ, VPZ, PHASE, DYNA, MODEL, CREEP, SLOPE, YIELD,
    FLEXI, NODE, VX, OMEGA, ENTHAL, COEF, COEG, CRIT ,PLAT, TREF,
    SHAPE, PCA, PCB, PNA, PCC, PNC, FSB, FSN, CURVE, COMP,
    LAYER, FCURE, OOGC, ZC, YC, MATE, AUST, MART, LAW, PERX,
    PERY, PERZ, SIGX, SIGY, SIGZ, SATU, HCX, HCY, HCZ, DUCT,
    GROUP, EMIS, TF, FLUID, INTG, PERV, EX, EY, EZ, CARACT,
    XCEN, YCEN, ZCEN, XINT, YINT, ZINT, XEXT, YEXT, ZEXT, XCOEF,
    YCOEF, ZCOEF, TRANSF, SURF2, HYPER, MRCB, DMASS, REAC, OFFSET, PISOTR,
    DISSIP, AUXI, EVOL, DILA, PREC, GAMA, CYIE, CSTR, GRAI, SELE,
    REDU, GAUS, DEFO, STAT, IDENT, RHOMF, RHOF, EZREG, EZSUP, TECS,
    SHEETS, TRIA, VSIG, VPER, VEPS, PROPOR, VCURV, METAL, SARGIN, PRICE,
    SATX, STAY, SATZ, TSAT, TPIEZO, EPSX, EPSY, EPSZ, TEPS, TMAGNE,
    PCX, PCY, PCZ, TSIG, PENALI, PEIX, PEIY, PEIZ, TPER, TRAJ,
    EPIX, EPIY, EPIZ, INFOS, MATRIX, SERI, CRES, TLIQ, TSOL, CRACK,
    PID, MID, FSOL, DEPEND, OAHE, OAHF, OAHG, OAHH, OAHI, SWELL,
    OAIA, OAIB, OAIC, OAID, EPSI, TOLE, LEVEL, GAP, EXTRAC, TYPE
};

static const std::map<SMF, std::string> SMFtoString = {
        {SMF::ID, "ID"},
        {SMF::PSI, "PSI"}, {SMF::THETA, "THETA"}, {SMF::PHI, "PHI"}, {SMF::RHO, "RHO"}, {SMF::E, "E"}, {SMF::NU, "NU"}, {SMF::G, "G"}, {SMF::LX, "LX"}, {SMF::LY, "LY"}, {SMF::LZ, "LZ"},
        {SMF::S, "S"}, {SMF::AY, "AY"}, {SMF::AZ, "AZ"}, {SMF::IX, "IX"}, {SMF::IY, "IY"}, {SMF::IZ, "IZ"}, {SMF::LOCAL, "LOCAL"}, {SMF::XX, "XX"}, {SMF::YY, "YY"}, {SMF::ZZ, "ZZ"},
        {SMF::H, "H"}, {SMF::SURF, "SURF"}, {SMF::INTEG, "INTEG"}, {SMF::PRESS, "PRESS"}, {SMF::TEMP, "TEMP"}, {SMF::TABLE, "TABLE"}, {SMF::VARI, "VARI"}, {SMF::DIAME, "DIAME"}, {SMF::DIAH, "DIAH"}, {SMF::UNIT, "UNIT"},
        {SMF::KX, "KX"}, {SMF::KY, "KY"}, {SMF::KZ, "KZ"}, {SMF::C, "C"}, {SMF::ALPHA, "ALPHA"}, {SMF::BETA, "BETA"}, {SMF::R, "R"}, {SMF::IG, "IG"}, {SMF::FT, "FT"}, {SMF::SECT, "SECT"},
        {SMF::VY, "VY"}, {SMF::VPY, "VPY"}, {SMF::VZ, "VZ"}, {SMF::VPZ, "VPZ"}, {SMF::PHASE, "PHASE"}, {SMF::DYNA, "DYNA"}, {SMF::MODEL, "MODEL"}, {SMF::CREEP, "CREEP"}, {SMF::SLOPE, "SLOPE"}, {SMF::YIELD, "YIELD"},
        {SMF::FLEXI, "FLEXI"}, {SMF::NODE, "NODE"}, {SMF::VX, "VX"}, {SMF::OMEGA, "OMEGA"}, {SMF::ENTHAL, "ENTHAL"}, {SMF::COEF, "COEF"}, {SMF::COEG, "COEG"}, {SMF::CRIT, "CRIT"}, {SMF::PLAT, "PLAT"}, {SMF::TREF, "TREF"},
        {SMF::SHAPE, "SHAPE"}, {SMF::PCA, "PCA"}, {SMF::PCB, "PCB"}, {SMF::PNA, "PNA"}, {SMF::PCC, "PCC"}, {SMF::PNC, "PNC"}, {SMF::FSB, "FSB"}, {SMF::FSN, "FSN"}, {SMF::CURVE, "CURVE"}, {SMF::COMP, "COMP"},
        {SMF::LAYER, "LAYER"}, {SMF::FCURE, "FCURE"}, {SMF::OOGC, "OOGC"}, {SMF::ZC, "ZC"}, {SMF::YC, "YC"}, {SMF::MATE, "MATE"}, {SMF::AUST, "AUST"}, {SMF::MART, "MART"}, {SMF::LAW, "LAW"}, {SMF::PERX, "PERX"},
        {SMF::PERY, "PERY"}, {SMF::PERZ, "PERZ"}, {SMF::SIGX, "SIGX"}, {SMF::SIGY, "SIGY"}, {SMF::SIGZ, "SIGZ"}, {SMF::SATU, "SATU"}, {SMF::HCX, "HCX"}, {SMF::HCY, "HCY"}, {SMF::HCZ, "HCZ"}, {SMF::DUCT, "DUCT"},
        {SMF::GROUP, "GROUP"}, {SMF::EMIS, "EMIS"}, {SMF::TF, "TF"}, {SMF::FLUID, "FLUID"}, {SMF::INTG, "INTG"}, {SMF::PERV, "PERV"}, {SMF::EX, "EX"}, {SMF::EY, "EY"}, {SMF::EZ, "EZ"}, {SMF::CARACT, "CARACT"},
        {SMF::XCEN, "XCEN"}, {SMF::YCEN, "YCEN"}, {SMF::ZCEN, "ZCEN"}, {SMF::XINT, "XINT"}, {SMF::YINT, "YINT"}, {SMF::ZINT, "ZINT"}, {SMF::XEXT, "XEXT"}, {SMF::YEXT, "YEXT"}, {SMF::ZEXT, "ZEXT"}, {SMF::XCOEF, "XCOEF"},
        {SMF::YCOEF, "YCOEF"}, {SMF::ZCOEF, "ZCOEF"}, {SMF::TRANSF, "TRANSF"}, {SMF::SURF2, "SURF2"}, {SMF::HYPER, "HYPER"}, {SMF::MRCB, "MRCB"}, {SMF::DMASS, "DMASS"}, {SMF::REAC, "REAC"}, {SMF::OFFSET, "OFFSET"}, {SMF::PISOTR, "PISOTR"},
        {SMF::DISSIP, "DISSIP"}, {SMF::AUXI, "AUXI"}, {SMF::EVOL, "EVOL"}, {SMF::DILA, "DILA"}, {SMF::PREC, "PREC"}, {SMF::GAMA, "GAMA"}, {SMF::CYIE, "CYIE"}, {SMF::CSTR, "CSTR"}, {SMF::GRAI, "GRAI"}, {SMF::SELE, "SELE"},
        {SMF::REDU, "REDU"}, {SMF::GAUS, "GAUS"}, {SMF::DEFO, "DEFO"}, {SMF::STAT, "STAT"}, {SMF::IDENT, "IDENT"}, {SMF::RHOMF, "RHOMF"}, {SMF::RHOF, "RHOF"}, {SMF::EZREG, "EZREG"}, {SMF::EZSUP, "EZSUP"}, {SMF::TECS, "TECS"},
        {SMF::SHEETS, "SHEETS"}, {SMF::TRIA, "TRIA"}, {SMF::VSIG, "VSIG"}, {SMF::VPER, "VPER"}, {SMF::VEPS, "VEPS"}, {SMF::PROPOR, "PROPOR"}, {SMF::VCURV, "VCURV"}, {SMF::METAL, "METAL"}, {SMF::SARGIN, "SARGIN"}, {SMF::PRICE, "PRICE"},
        {SMF::SATX, "SATX"}, {SMF::STAY, "STAY"}, {SMF::SATZ, "SATZ"}, {SMF::TSAT, "TSAT"}, {SMF::TPIEZO, "TPIEZO"}, {SMF::EPSX, "EPSX"}, {SMF::EPSY, "EPSY"}, {SMF::EPSZ, "EPSZ"}, {SMF::TEPS, "TEPS"}, {SMF::TMAGNE, "TMAGNE"},
        {SMF::PCX, "PCX"}, {SMF::PCY, "PCY"}, {SMF::PCZ, "PCZ"}, {SMF::TSIG, "TSIG"}, {SMF::PENALI, "PENALI"}, {SMF::PEIX, "PEIX"}, {SMF::PEIY, "PEIY"}, {SMF::PEIZ, "PEIZ"}, {SMF::TPER, "TPER"}, {SMF::TRAJ, "TRAJ"},
        {SMF::EPIX, "EPIX"}, {SMF::EPIY, "EPIY"}, {SMF::EPIZ, "EPIZ"}, {SMF::INFOS, "INFOS"}, {SMF::MATRIX, "MATRIX"}, {SMF::SERI, "SERI"}, {SMF::CRES, "CRES"}, {SMF::TLIQ, "TLIQ"}, {SMF::TSOL, "TSOL"}, {SMF::CRACK, "CRACK"},
        {SMF::PID, "PID"}, {SMF::MID, "MID"}, {SMF::FSOL, "FSOL"}, {SMF::DEPEND, "DEPEND"}, {SMF::OAHE, "OAHE"}, {SMF::OAHF, "OAHF"}, {SMF::OAHG, "OAHG"}, {SMF::OAHH, "OAHH"}, {SMF::OAHI, "OAHI"}, {SMF::SWELL, "SWELL"},
        {SMF::OAIA, "OAIA"}, {SMF::OAIB, "OAIB"}, {SMF::OAIC, "OAIC"}, {SMF::OAID, "OAID"}, {SMF::EPSI, "EPSI"}, {SMF::TOLE, "TOLE"}, {SMF::LEVEL, "LEVEL"}, {SMF::GAP, "GAP"}, {SMF::EXTRAC, "EXTRAC"}, {SMF::TYPE, "TYPE"},
};

std::string SMFToString(SMF key);

SMF DOFtoSMF(DOF dof);

static const int defaultNbDesiredRoots=100; /**< Default number of desired roots for a static analysis (chosen from experiment)**/

class SystusWriter final: public Writer {

private:
    SystusOption systusOption;
    SystusSubOption systusSubOption;
    char dofCode;
    DOFS availableDOFS;
    int  nbDOFS;
    double maxYoungModulus = Globals::UNAVAILABLE_DOUBLE;
    static int auto_part_id;                 /**< Next available number for Systus Part ID **/
    static const int DampingAccessId;        /**< Access Id for the Damping Matrices file (Element X9XX type 0)**/
    static const int MassAccessId;           /**< Access Id for the Mass Matrices file (Element X9XX type 0)**/
    static const int StiffnessAccessId;      /**< Access Id for the Stiffness Matrices file (Element X9XX type 0)**/

    std::map<int, std::vector<systus_ascid_t> > lists;
    std::map<systus_ascid_t, std::vector<double>> vectors;
    std::map<int, int> rotationNodeIdByTranslationNodeId; /**< nodeId, nodeId > :  map between the reference node and the reference rotation for 190X elements in 3D mode.**/
    std::map<int, std::map<int, int>> localLoadingIdByLoadsetIdByAnalysisId;
    std::map<int, systus_ascid_t> loadingVectorIdByLocalLoading;
    std::map<int, std::map<int, std::vector<systus_ascid_t>>> loadingVectorsIdByLocalLoadingByNodePosition;
    std::map<int, std::map<int, std::vector<systus_ascid_t>>> loadingVectorsIdByLocalLoadingByCellId;
    std::map<int, std::map<int, std::vector<systus_ascid_t>>> constraintVectorsIdByLocalLoadingByNodePosition;
    std::map<int, systus_ascid_t> localVectorIdByNodePosition;  /**< nodePosition, vectorId> for all Coordinate Systems Vectors. **/
    std::map<int, int> loadingListIdByNodePosition;
    std::map<int, int> loadingListIdByCellId;
    std::map<int, std::string> localLoadingListName;
    std::map<int, int> constraintListIdByNodePosition;
    std::map<int, char> constraintByNodePosition;
    std::vector< std::vector<int> > systusSubcases;   /**< Ids of loadcases composing the subcase **/
    std::vector<SystusTable> tables;
    SystusMatrices dampingMatrices;         /**< All needed damping matrices (element X9XX type 0). **/
    SystusMatrices massMatrices ;           /**< All needed mass matrices (element X9XX type 0). **/
    SystusMatrices stiffnessMatrices;       /**< All needed rigidity matrices (element X9XX type 0). **/
    std::map<int, systus_ascid_t> tableByElementSet;
    std::map<int, systus_ascid_t> tableByLoadcase;
    std::map<int, systus_ascid_t> seIdByElementSet; /**< Number of the matrix associated to SE (element X9XX type 0). **/
    std::map<int, std::string > filebyAccessId;        /**< Names of matrix files **/
    /**
     * Renumbers the nodes
     * see Systus ref manual chapter 15 or chapter 13 2.7
     *
     *\code{.cpp}
     *         Systus                       Med\endcode
     *
     *
     *   SEG2
     *\code{.cpp}
     *     0-----------1               0-----------1\endcode
     *
     *
     *   SEG3
     *\code{.cpp}
     *     0-----1-----2               0-----2-----1\endcode
     *
     *
     *   TRI3
     *\code{.cpp}
     *           2                            2
     *         ,/ `\                        ,/ `\
     *       ,/     `\                    ,/     `\
     *     ,/         `\                ,/         `\
     *    0-------------1              0-------------1\endcode
     *
     *
     *   TRI6
     *\code{.cpp}
     *           4                            1
     *         ,/ `\                        ,/ `\
     *       ,5     `3                    ,3     `4
     *     ,/         `\                ,/         `\
     *    0------1------2              0------5------2\endcode
     *
     *
     *   QUAD4
     *\code{.cpp}
     *   3-----------2                   3-----------2
     *   |           |                   |           |
     *   |           |                   |           |
     *   |           |                   |           |
     *   |           |                   |           |
     *   |           |                   |           |
     *   0-----------1                   0-----------1\endcode
     *
     *
     *   QUAD8
     *\code{.cpp}
     *   6-----5-----4                   1-----5-----2
     *   |           |                   |           |
     *   |           |                   |           |
     *   7           3                   4           6
     *   |           |                   |           |
     *   |           |                   |           |
     *   0-----1-----2                   0-----7-----3\endcode
     *
     *
     *   TETRA4
     *\code{.cpp}
     *            3                             3
     *          ,/|`\                         ,/|`\
     *        ,/  |  `\                     ,/  |  `\
     *      ,/    '.   `\                 ,/    '.   `\
     *    ,/       |     `\             ,/       |     `\
     *  ,/         |       `\         ,/         |       `\
     * 0-----------'.--------2       0-----------'.--------1
     *  `\.         |      ,/         `\.         |      ,/
     *     `\.      |    ,/              `\.      |    ,/
     *        `\.   '. ,/                   `\.   '. ,/
     *           `\. |/                        `\. |/
     *              `1                            `2\endcode
     *
     *
     *   TETRA10
     *\code{.cpp}
     *            9                             3
     *          ,/|`\                         ,/|`\
     *        ,/  |  `\                     ,/  |  `\
     *      ,6    '.   `8                 ,7    '.   `8
     *    ,/       7     `\             ,/       9     `\
     *  ,/         |       `\         ,/         |       `\
     * 0--------5--'.--------4       0--------4--'.--------1
     *  `\.         |      ,/         `\.         |      ,/
     *     `\.      |    ,3              `\.      |    ,5
     *        `1.   '. ,/                   `6.   '. ,/
     *           `\. |/                        `\. |/
     *              `2                            `2\endcode
     *
     *
     *    PRISM6
     *\code{.cpp}
     *              5                             4
     *            ,/|`\                         ,/|`\
     *          ,/  |  `\                     ,/  |  `\
     *        ,/    |    `\                 ,/    |    `\
     *       3------+------4               3------+------5
     *       |      |      |               |      |      |
     *       |      |      |               |      |      |
     *       |      |      |               |      |      |
     *       |      2      |               |      1      |
     *       |    ,/ `\    |               |    ,/ `\    |
     *       |  ,/     `\  |               |  ,/     `\  |
     *       |,/         `\|               |,/         `\|
     *       0-------------1               0-------------2\endcode
     *
     *
     *    PRISM15
     *\code{.cpp}
     *              13                            4
     *            ,/|`\                         ,/|`\
     *          14  |  12                      9  |  10
     *        ,/    |    `\                 ,/    |    `\
     *       9------10-----11              3------11-----5
     *       |      |      |               |      |      |
     *       |      8      |               |     13      |
     *       |      |      |               |      |      |
     *       6      4      7               12     1      14
     *       |    ,/ `\    |               |    ,/ `\    |
     *       |  ,5     `3  |               |  ,6     `7  |
     *       |,/         `\|               |,/         `\|
     *       0------1------2               0------8------2\endcode
     *
     *
     *    HEXA8
     *\code{.cpp}
     *    6-----------5                  6-----------7
     *    |\          |\                 |\          |\
     *    | \         | \                | \         | \
     *    |  \        |  \               |  \        |  \
     *    |   7-------+---4              |   5-------+---4
     *    |   |       |   |              |   |       |   |
     *    2---+-------1   |              2---+-------3   |
     *     \  |        \  |               \  |        \  |
     *      \ |         \ |                \ |         \ |
     *       \|          \|                 \|          \|
     *        3-----------0                  1-----------0\endcode
     *
     *
     *    HEXA20
     *\code{.cpp}
     *   16-----15----14                 6-----14----7
     *    |\          |\                 |\          |\
     *    | 17        | 13               | 13        | 15
     *   10  \        9  \              18  \        19 \
     *    |  18----19-+---12             |   5----12-+---4
     *    |   |       |   |              |   |       |   |
     *    4---+--3----2   |              2---+--10---3   |
     *     \  11       \  8               \  17       \  16
     *      5 |         1 |                9 |        11 |
     *       \|          \|                 \|          \|
     *        6-----7-----0                  1-----8-----0\endcode
     *
     **/
    // vs 2013 compiler bug	in initializer list {{3,6}, {4,3}} not supported
    std::map<int, int> numberOfDofBySystusOption = boost::assign::map_list_of(3, 6)(4, 3);

    /** Converts a vega DOFS to its ASC material counterpart.
     *  Return the number of material field filled.
     **/
    int DOFSToMaterial(const DOFS dofs, std::ostream& out) const;
    /** Converts a vega DOFS to its integer Systus counterpart **/
    int DOFSToInt(const DOFS dofs) const;
    /** Converts a vega DOF to its integer Systus counterpart **/
    int DOFToInt(const DOF dof) const;

    /** Find an available Part Id for a Cell Group.
     * If possible, try to use the suffix (_NN) of the Group Name. **/
    int getPartId(const std::string partName, std::set<int> & usedPartId);
    static const std::unordered_map<CellType::Code, std::vector<int>, EnumClassHash> systus2medNodeConnectByCellType;
    void writeAsc(const SystusModel&, const ConfigurationParameters&, const int idSubcase, std::ostream&);
    void getSystusInformations(const SystusModel&, const ConfigurationParameters&);

    /**
     * Determines the best Systus Option fitting the model.
     */
    void getSystusAutomaticOption(const SystusModel&, SystusOption & autoSystusOption, SystusSubOption & autoSystusSubOption);

    /**
     * Clear all maps, vectors, lists filled during a previous translation.
     */
    void clear();

    /**
     * Translate the model into a Systus compatible format.
     * It fills all needed tables, vectors, lists, and so on.
     */
    void translate(const SystusModel &systusModel, const int idSubcase);

    void fillLoads(const SystusModel&, const int idSubcase);
    void fillConstraintsNodes(const SystusModel& systusModel, const int idLoadcase);
    void fillConstraintsVectors(const SystusModel& systusModel, const int idSubcase);
    void fillCoordinatesVectors(const SystusModel& systusModel, const int idSubcase);
    void fillLoadingsVectors(const SystusModel& systusModel, const int idSubcase);
    void fillMatrices(const SystusModel& systusModel, const int idSubcase);
    void fillTables(const SystusModel&, const int idSubcase);
    void fillVectors(const SystusModel&, const int idSubcase);
    void fillLists(const SystusModel&, const int idSubcase);

    /**
     *  Generate a rigidity for a a Rbar Element Set. The formulation we use is
     *      K = max_i(E_i)*max_j(L_j)
     *    where
     *       E_i is the Young modulus of the part i,
     *       L_j the length of the j-th segment of the current Rbar.
     *
     */
    double generateRbarStiffness(const SystusModel&);
    //double generateLmpcRigidity(const SystusModel&, const std::shared_ptr<Lmpc>);

    /**
     *  Generate RBE from Rbars and Rbe3 ElementSet:
     *    - adding the lagrangian/rotation node
     *    - computing the Rigidity.
     */
    void generateRBEs(SystusModel&, const ConfigurationParameters&);
    /** Following a user-defined list, we regroup analyzes in order to build
     *  multi-loadcases Subcases.
     *  Default result is "each analysis on its own subcase".
     */
    void generateSubcases(const SystusModel&, const ConfigurationParameters&);
    void writeHeader(const SystusModel&, std::ostream&);

    /**
     *  Write the informations field of the ASC file, including the long title
     *  and the codes of the model (See NCODE(20) in Systus code for more details).
     **/
    void writeInformations(const SystusModel&, const int idSubcase, std::ostream&);

    /**
     * Write the Nodes in ASC format.
     * If possible, nodes numbers copy the numbers of the input model.
     **/
    void writeNodes(const SystusModel&, std::ostream&);

    /**
     *  Compute the default referentiel for an element, as described in the
     *  Systus Reference Manual secion "16.2 Local axes (X,Y,Z)"
     *  TODO: Should not be a part of the writer.
     **/
    CartesianCoordinateSystem buildElementDefaultReferentiel(const SystusModel& systusModel, const std::vector<int> nodes, const int dim, const int celltype);

    /**
     *  Write the Euler Angles corresponding to an element with local referentiel cpos.
     *  Depending of the type of element, some angles may be dismissed.
     **/
    void writeElementLocalReferentiel(const SystusModel& systusModel, const int dim, const int celltype, const std::vector<int> nodes, const int cpos, std::ostream& out);
    void writeElements(const SystusModel&, const int idSubcase, std::ostream&);
    /**
     * Write the Cells and Nodes groups in ASC format.
     *
     * Cells groups follow the format:
     *   id NAME 2 0 "Subcommand"  ""  "Comment" elem1 elem2 ... elemN
     *
     *   Visual Enviromnent will use the Cells Groups where "Subcommand" is of the form "PART_ID k" (k integer positive) to build a partition of the domain.
     *   For this to work, these groups must verify:
     *       - Each group has a different part number k.
     *       - Every cell of the mesh must belong to one "Part" Cells Group and one only.
     *
     *   VEGA normally ensures the second condition (if not, there a bug in the Reader), allowing us to write Cells groups as PARTs here.
     *
     * Nodes Groups follow the format:
     *  id NAME 1 0 "No methods"  ""  "Comment" node1 node2 ... nodeM
     */
    void writeGroups(const SystusModel&, std::ostream&);
    /**
     * Writes a Material Field in the ASC Material lines.
     * Output verifies the syntax " number value" where the number is
     * the integer conversion of the SMF::key.
     * Value here is a double.
     */
    void writeMaterialField(const SMF key, const double value, int& nbfields, std::ostream& out) const;
    /**
     * Writes a Material Field in the ASC Material lines.
     * Output verifies the syntax " number value" where the number is
     * the integer conversion of the SMF::key.
     * Value here is an integer.
     */
    void writeMaterialField(const SMF key, const int value, int& nbfields, std::ostream& out) const ;
    void writeMaterials(const SystusModel&, const ConfigurationParameters&, const int, std::ostream&);
    void writeLoads(std::ostream&);
    void writeLists(std::ostream&);
    /**
     * Writes the tables to the TABLE part of the ASC file.
     */
    void writeTables(std::ostream&);
    void writeVectors(std::ostream&);
    void writeDat(const SystusModel&, const ConfigurationParameters &, const int idSubcase, std::ostream&);

    void writeNodalDisplacementAssertion(Assertion& assertion, std::ostream& out);
    void writeNodalComplexDisplacementAssertion(Assertion& assertion, std::ostream& out);
    void writeFrequencyAssertion(Assertion& assertion, std::ostream& out);
    void writeNodalForceVector(const SystusModel& systusModel, std::shared_ptr<NodalForce> nodalForce, const int idLoadCase, systus_ascid_t& vectorId);

    std::string toString() const override {
        return "SystusWriter";
    }
    void writeMasses(const SystusModel&, std::ostream& out); /** Write Nodal Masses on outstream. **/

    /**
     * Write all matrix files to an ASC format. To be used by SYSTUS, these files must be converted to
     * a BINARY format (tool filematrix of the ESI Systus Package)
     */
    void writeMatrixFiles(const SystusModel& systusModel, const int idSubcase);


public:
    SystusWriter() = default;
    SystusWriter(const SystusWriter& that) = delete;
    virtual ~SystusWriter();

    std::string writeModel(Model& model, const ConfigurationParameters&) override;
};

} // namespace systus
} // namespace vega
#endif /* SYSTUSBUILDER_H_ */
