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
 * NastranParser.h
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

#ifndef NASTRANPARSER_H_
#define NASTRANPARSER_H_

#include <boost/filesystem.hpp>
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "NastranTokenizer.h"
#include <type_traits>

namespace vega {

namespace nastran {

namespace fs = boost::filesystem;

class NastranParser: public vega::Parser {
protected:
    enum class NastranAnalysis {
        STATIC = 101,
        MODES = 103,
        BUCKL = 105,
        NLSTATIC = 106,
        DFREQ = 108,
        DTRAN = 109,
        MCEIG = 110,
        MFREQ = 111,
        MTRAN = 112,
        DESOPT = 200
    };
    using parseElementFPtr = void (NastranParser::*)(NastranTokenizer& tok, Model& model);
    virtual parseElementFPtr findCmdParser(const std::string) const;
    virtual parseElementFPtr findParamParser(const std::string) const;
    virtual std::string defaultAnalysis() const;
    virtual NastranAnalysis autoSubcaseAnalysis(std::map<std::string, std::string> &context) const;
private:
    class GrdSet {
    public:
        GrdSet(const pos_t cp = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
                const pos_t cd = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
                const int ps = 0, const int seid = 0) :
                    cp(cp), cd(cd), ps(ps), seid(seid) {
        }
        pos_t cp;
        pos_t cd;
        int ps;
        int seid;
    };
    GrdSet grdSet;

    std::unordered_map<std::string, Reference<ElementSet>> directMatrixByName;
    static const std::unordered_map<std::string, NastranAnalysis> ANALYSIS_BY_LABEL;
    static const std::unordered_map<std::string, parseElementFPtr> PARSE_FUNCTION_BY_KEYWORD;
    static const std::unordered_map<std::string, parseElementFPtr> PARSEPARAM_FUNCTION_BY_KEYWORD;

    void addSet(NastranTokenizer& tok, Model& model);
    void addAnalysis(NastranTokenizer& tok, Model& model, std::map<std::string, std::string>& context, int analysis_id =
            Analysis::NO_ORIGINAL_ID, const InputContext& = {});
    void addCombinationAnalysis(NastranTokenizer& tok, Model& model, std::map<std::string, std::string>& context, int analysis_id =
            Analysis::NO_ORIGINAL_ID);

    fs::path findModelFile(const std::string& filename);
    void parseBULKSection(NastranTokenizer &tok, Model& model1);

    void parseExecutiveSection(NastranTokenizer& tok, Model& model, std::map<std::string, std::string>& context);
    /**Renumbers the nodes
     * The map has no keys for CellTypes that have the same connectivity in Nastran and Med
     *
     * \code{.cpp}
     *         Nastran                       Med\endcode
     *
     *
     *   TRIA{3;6}
     *\code{.cpp}
     *           2                            1
     *         ,/ `\                        ,/ `\
     *       ,5     `4                    ,3     `4
     *     ,/         `\                ,/         `\
     *    0------3------1              0------5------2\endcode
     *
     *
     *   QUAD{4;8;9}
     *\code{.cpp}
     *   3-----6-----2                   1-----5-----2
     *   |           |                   |           |
     *   |           |                   |           |
     *   7     8     5                   4     8     6
     *   |           |                   |           |
     *   |           |                   |           |
     *   0-----4-----1                   0-----7-----3\endcode
     *
     *
     *   TETRA{4;10}
     *\code{.cpp}
     *            3                             3
     *          ,/|`\                         ,/|`\
     *        ,/  |  `\                     ,/  |  `\
     *      ,7    '.   `9                 ,7    '.   `8
     *    ,/       8     `\             ,/       9     `\
     *  ,/         |       `\         ,/         |       `\
     * 0--------6--'.--------2       0--------4--'.--------1
     *  `\.         |      ,/         `\.         |      ,/
     *     `\.      |    ,5              `\.      |    ,5
     *        `4.   '. ,/                   `6.   '. ,/
     *           `\. |/                        `\. |/
     *              `1                            `2\endcode
     *
     *
     *    PENTA{5;13}
     *\code{.cpp}
     *                4                           4
     *              ,/|\                        ,/|\
     *            ,/ .'|\                     ,/ .'|\
     *          ,/   | | \                  ,/   | | \
     *        12    .' 11 \               10    .' 11 \
     *      ,/      |   \  \            ,/      |   \  \
     *    ,/       9    |  10         ,/       9    |  12
     *  ,/         |    |    \      ,/         |    |    \
     *  3--------7-+-----2    `.    1--------6-+-----2    `.
     *   `\        |      `\    \    `\        |      `\    \
     *     `8     .'         6   \     `5     .'         7   \
     *       `\   |           `\  \      `\   |           `\  \
     *         `\.'             `\`        `\.'             `\`
     *            0--------5-------1          0--------8-------3\endcode
     *
     *
     *    PRISM{6;15}
     *\code{.cpp}
     *              5                             4
     *            ,/|`\                         ,/|`\
     *          14  |  13                      9  |  10
     *        ,/    |    `\                 ,/    |    `\
     *       3------12-----4               3------11-----5
     *       |      |      |               |      |      |
     *       |     11      |               |     13      |
     *       |      |      |               |      |      |
     *       9      2      10              12     1      14
     *       |    ,/ `\    |               |    ,/ `\    |
     *       |  ,8     `7  |               |  ,6     `7  |
     *       |,/         `\|               |,/         `\|
     *       0------6------1               0------8------2\endcode
     *
     *
     *    HEXA{8;20}
     *\code{.cpp}
     *    6-----17----5                  6-----14----7
     *    |\          |\                 |\          |\
     *    | 18        | 16               | 13        | 15
     *   14  \        13 \              18  \        19 \
     *    |   7----19-+---4              |   5----12-+---4
     *    |   |       |   |              |   |       |   |
     *    2---+--9----1   |              2---+--10---3   |
     *     \  15       \  12              \  17       \  16
     *     10 |         8 |                9 |        11 |
     *       \|          \|                 \|          \|
     *        3-----11----0                  1-----8-----0\endcode
     *
     */
    static const std::unordered_map<CellType::Code, std::vector<int>, EnumClassHash> nastran2medNodeConnectByCellType;

    /**
     * PARAM ALPHA1
     * Rayleigh damping, mass matrix factor
     */
    void parseParamALPHA1(NastranTokenizer& tok, Model& model);

    /**
     * PARAM ALPHA2
     * Rayleigh damping, stiffness matrix factor
     */
    void parseParamALPHA2(NastranTokenizer& tok, Model& model);

    /**
     * PARAM AUTOSPC
     */
    void parseParamAUTOSPC(NastranTokenizer& tok, Model& model);

    /**
     * BCBODY
     */
    void parseBCBODY(NastranTokenizer& tok, Model& model);

    /**
     * BCGRID
     */
    void parseBCGRID(NastranTokenizer& tok, Model& model);

    /**
     * BCONP
     */
    void parseBCONP(NastranTokenizer& tok, Model& model);

    /**
     * BCTABLE
     */
    void parseBCTABLE(NastranTokenizer& tok, Model& model);

    /**
     * BFRIC
     */
    void parseBFRIC(NastranTokenizer& tok, Model& model);

    /**
     * BLSEG
     */
    void parseBLSEG(NastranTokenizer& tok, Model& model);

    /**
     * BSCONP
     */
    void parseBSCONP(NastranTokenizer& tok, Model& model);

    /**
     * BSSEG
     */
    void parseBSSEG(NastranTokenizer& tok, Model& model);

    /**
     * BSURF
     */
    void parseBSURF(NastranTokenizer& tok, Model& model);

    /**
     * Parse the CBAR keyword (page 1154 of MDN Nastran 2006 Quick Reference Guide.)
     * Pin flags (PA, PB) and offset vectors (OFFT, WA, WB) are not supported.
     */
    void parseCBAR(NastranTokenizer& tok, Model& model); //in NastranParser_geometry.cpp

    /**
     * Parse the CBEAM keyword (page 1161 of MDN Nastran 2006 Quick Reference Guide.)
     * Pin flags (PA, PB), offset vectors (OFFT, WiA, WiB) and grid point identification (SA, SB)
     * are not supported.
     */
    void parseCBEAM(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CBUSH keyword (page 1174 of MDN Nastran 2006 Quick Reference Guide.)
     * The location of spring damper (S), coordinate system  of spring damper (OCID) and
     * component of spring-damper-offset (S1, S2, S3) are not supported.
     */
    void parseCBUSH(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CDAMP keyword (page 1187 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseCDAMP1(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS1 keyword (page 1199 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseCELAS1(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS2 keyword (page 1202 of MDN Nastran 2006 Quick Reference Guide.)
     * The stress coefficient (S) is ignored, as it's only used for post-treatment.
     */
    void parseCELAS2(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS4 keyword (page 1207 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseCELAS4(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CGAP keyword (page 1216 of MDN Nastran 2006 Quick Reference Guide.)
     * Only support Two nodes gap.
     */
    void parseCGAP(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCHEXA(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CMASS2 keyword (page 1243 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseCMASS2(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CONM1 keyword (page 1250 of MDN Nastran 2006 Quick Reference Guide.)
     * CID is not supported.
     */
    void parseCONM1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the CONM2 keyword (page 1251 of MDN Nastran 2006 Quick Reference Guide.)
     * CID is not supported.
     */
    void parseCONM2(NastranTokenizer& tok, Model& model);



    /**
     * Parse the CRIGD1 keyword (page 385 of THE NASTRAN USER'S MANUAL)
     *
     */
    void parseCRIGD1(NastranTokenizer& tok, Model& model);

    /**
     * Rod Element Property and Connection,
     * defines a tension-compression-torsion element without reference to a property entry
     */
    void parseCONROD(NastranTokenizer& tok, Model& model);
    /**
     * Parse the CORD1R keyword (page 1264 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseCORD1R(NastranTokenizer& tok, Model& model);

    /**
     * Parse the CORD2C keyword (page 1271 of MDN Nastran 2006 Quick Reference Guide.)
     * Reference coordinate system (RID) not supported.
     */
    void parseCORD2C(NastranTokenizer& tok, Model& model);
    /**
     * Parse the CORD2R keyword (page 1273 of MDN Nastran 2006 Quick Reference Guide.)
     * Reference coordinate system (RID) not supported.
     */
    void parseCORD2R(NastranTokenizer& tok, Model& model);
    /**
     * Parse the CORD2S keyword (page 1271 of MDN Nastran 2006 Quick Reference Guide.)
     * Reference coordinate system (RID) not supported.
     */
    void parseCORD2S(NastranTokenizer& tok, Model& model);
    void parseCPENTA(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCPYRAM(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCQUAD(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCQUAD4(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCQUAD8(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCQUADR(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the CROD keyword (page 1310 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseCROD(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCTETRA(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCTRIA3(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCTRIA6(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp
    void parseCTRIAR(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the keyword CVISC (page 1349 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseCVISC(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword DAREA (page 1377 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseDAREA(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword DELAY (page 1386 of MDN Nastran 2006 Quick Reference Guide.)
     * Point identification (P), Component number (C) and a second phase are
     * ignored.
     */
    void parseDELAY(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword DLOAD (page 1398 of MDN Nastran 2006 Quick Reference Guide.)
     * Scale factors (S and Si) must be equal to 1.
     */
    void parseDLOAD(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword DMIG (page 1046 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseDMIG(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword DPHASE (page 1433 of MDN Nastran 2006 Quick Reference Guide.)
     * Point identification (P), Component number (C) and a second phase are
     * ignored.
     */
    void parseDPHASE(NastranTokenizer& tok, Model& model);

    /**
     * EIGB Defines data needed to perform buckling analysis.
     */
    void parseEIGB(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword EIGC (page 1509 of MDN Nastran 2006 Quick Reference Guide.)
     * Options are not supported.
     */
    void parseEIGC(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword EIGR (page 1517 of MDN Nastran 2006 Quick Reference Guide.)
     * Only Lanczos Method is supported.
     */
    void parseEIGR(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword EIGRL (page 1522 of MDN Nastran 2006 Quick Reference Guide.)
     * Options are not supported.
     */
    void parseEIGRL(NastranTokenizer& tok, Model& model);

    /**
     *  Generic function for parsing Element keywords.
     */
    void parseElem(NastranTokenizer& tok, Model& model, const std::vector<CellType>&);//in NastranParser_geometry.cpp

    /**
     * Parse the FORCE keyword (page 1549 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE(NastranTokenizer& tok, Model& model);

    /**
     * Parse the FORCE1 keyword (page 1550 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the FORCE2 keyword (page 1552 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE2(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword FREQ (page 1555 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseFREQ(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword FREQ1 (page 1556 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseFREQ1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword FREQ1 (page 1560 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseFREQ3(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword FREQ4
     */
    void parseFREQ4(NastranTokenizer& tok, Model& model);

    /**
     * Parse the GRAV keyword (page 1651 of MDN Nastran 2006 Quick Reference Guide.)
     * CID and MB are not supported.
     */
    void parseGRAV(NastranTokenizer& tok, Model& model);

    /**
     * Parse the GRDSET keyword (page 1623 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseGRDSET(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the GRID keyword (page 1624 of MDN Nastran 2006 Quick Reference Guide.)
     * CP coordinate system not supported, and taken as blank.
     */
    void parseGRID(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * PARAM GRDPNT
     */
    void parseParamGRDPNT(NastranTokenizer& tok, Model& model);

    /**
     * PARAM HFREQ
     */
    void parseParamHFREQ(NastranTokenizer& tok, Model& model);

    /**
     * PARAM K6ROT
     */
    void parseParamK6ROT(NastranTokenizer& tok, Model& model);

    /**
     * PARAM INREL
     * INREL controls the calculation of inertia relief or enforced
     * acceleration in linear static analysis, buckling analysis, and
     * differential stiffness in dynamic analysis.
     */
    void parseParamINREL(NastranTokenizer& tok, Model& model);

    /**
     * PARAM G
     * Specifies the uniform structural damping coefficient
     * in the formulation of global damping matrix in direct transient solutions.
     * To obtain the value for the model parameter G, multiply the critical damping ratio,
     * C/C0, by 2.0. Note that PARAM, W3 must be greater than zero or PARAM, G will be ignored.
     */
    void parseParamG(NastranTokenizer& tok, Model& model);

    /**
     * PARAM W3
     * Frequency of interest for structural damping
     * The units of W3 are radians per unit time.
     */
    void parseParamW3(NastranTokenizer& tok, Model& model);

    void parseInclude(NastranTokenizer& tok, Model& model);

    /**
     * PARAM LFREQ
     * Default = 0.0
     * PARAM,LFREQ gives the lower limit on the frequency range of retained modes.
     */
    void parseParamLFREQ(NastranTokenizer& tok, Model& model);

    /**
     * PARAM LGDISP
     */
    void parseParamLGDISP(NastranTokenizer& tok, Model& model);

    /**
     * Parse the LSEQ keyword
     * Fully supported.
     */
    void parseLSEQ(NastranTokenizer& tok, Model& model);

    /**
     * Parse the LOAD keyword (page 1646 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseLOAD(NastranTokenizer& tok, Model& model);

    /**
     * Parse the MAT1 keyword (page 1664 of MDN Nastran 2006 Quick Reference Guide.)
     * Structural element damping coefficient (GE) are not supported.
     */
    void parseMAT1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the MAT8 keyword
     * Only mandatory part is supported
     */
    void parseMAT8(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword MATS1 (page 1928 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseMATS1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword MATHP (page 1917 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseMATHP(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword MOMENT (page 1984 of MDN Nastran 2006 Quick Reference Guide.)
     * Coordinate System (CID) not supported.
     */
    void parseMOMENT(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword MPC (page 1997 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseMPC(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword NLPARM (page 2014 of MDN Nastran 2006 Quick Reference Guide.)
     * All parameters are ignored except NINC.
     */
    void parseNLPARM(NastranTokenizer& tok, Model& model);

    /**
     * Parameters for Arc-Length Methods in Nonlinear Static Analysis
     * Defines a set of parameters for the arc-length incremental solution strategies in nonlinear static analysis
     */
    void parseNLPCI(NastranTokenizer& tok, Model& model);

    /**
     * PARAM CHECKEL
     */
    void parseParamCHECKEL(NastranTokenizer& tok, Model& model);

    /**
     * PARAM NOCOMPS
     */
    void parseParamNOCOMPS(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword PARAM (page 2088 of MDN Nastran 2006 Quick Reference Guide.)
     * Partial support, with unknown reliability.
     */
    virtual void parsePARAM(NastranTokenizer& tok, Model& model);

    /**
     * PARAM PATVER
     */
    void parseParamPATVER(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PBAR keyword (page 2092 of MDN Nastran 2006 Quick Reference Guide.)
     * Neither Stress coefficients nor i12 are supported.
     */
    void parsePBAR(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PBARL keyword (page 2094 of MDN Nastran 2006 Quick Reference Guide.)
     * Only BAR and ROD type are supported.
     */
    void parsePBARL(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PBEAM keyword (page 2109 of MDN Nastran 2006 Quick Reference Guide.)
     * Shear center, sections, area of product are not implemented.
     */
    void parsePBEAM(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PBEAML keyword (page 2145 of MDN Nastran 2006 Quick Reference Guide.)
     * Only BAR, ROD and I types are supported.
     */
    void parsePBEAML(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PBUSH keyword (page 2165 of MDN Nastran 2006 Quick Reference Guide.)
     * Only Stiffness (K) is supported.
     */
    void parsePBUSH(NastranTokenizer& tok, Model& model);//in NastranParser_geometry.cpp

    /**
     * Parse the PCOMP keyword
     */
    void parsePCOMP(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PDAMP keyword (page 2197 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parsePDAMP(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PELAS keyword (page 2202 of MDN Nastran 2006 Quick Reference Guide.)
     * The stress coefficient (S) is ignored, as it's only used for post-treatment.
     */
    void parsePELAS(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PGAP keyword (page 2210 of MDN Nastran 2006 Quick Reference Guide.)
     * Only U0 is supported.
     */
    void parsePGAP(NastranTokenizer& tok, Model& model);

/**
     * Parse the PLOAD keyword
     */
    void parsePLOAD(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PLOAD1 keyword
     */
    void parsePLOAD1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PLOAD2 keyword
     */
    void parsePLOAD2(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PLOAD4 keyword (page 2227 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform pressure are supported.
     */
    void parsePLOAD4(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PROD keyword (page 2245 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform pressure are supported.
     */
    void parsePROD(NastranTokenizer& tok, Model& model);

    /**
     * PARAM PRTMAXIM
     */
    void parseParamPRTMAXIM(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PLSOLID keyword (page 2237 of MDN Nastran 2006 Quick Reference Guide.)
     * Partial support.
     */
    void parsePLSOLID(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PSHELL keyword (page 2250 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform PSHELLs are supported, meaning MID2, MID3, MID4, etc are dismissed.
     */
    void parsePSHELL(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PSOLID keyword (page 2264 of MDN Nastran 2006 Quick Reference Guide.)
     * Partial support.
     */
    void parsePSOLID(NastranTokenizer& tok, Model& model);

    /**
     * Parse the PVISC keyword (page 2278 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parsePVISC(NastranTokenizer& tok, Model& model);

    /**
     * Parse the RBAR keyword (page 2312 of MDN Nastran 2006 Quick Reference Guide.)
     * CMA, CMB and the thermal expansion (ALPHA) are not supported.
     */
    void parseRBAR(NastranTokenizer& tok, Model& model);

    /**
     * Parse the RBAR1 keyword (page 2314 of MDN Nastran 2006 Quick Reference Guide.)
     * Thermal expansion (ALPGA) is not supported.
     */
    void parseRBAR1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the RBE2 keyword (page 2319 of MDN Nastran 2006 Quick Reference Guide.)
     * Only rigid constraint (dofs=123456) is supported.
     */
    void parseRBE2(NastranTokenizer& tok, Model& model);

    /**
     * Parse the RBE3 keyword (page 2334 of MDN Nastran 2006 Quick Reference Guide.)
     * No support of ALPHA nor UM.
     */
    void parseRBE3(NastranTokenizer& tok, Model& model);

    /**
     * Parse the RFORCE keyword (page 2373 of MDN Nastran 2006 Quick Reference Guide.)
     * No support of METHOD, RACC and MB.
     */
    void parseRFORCE(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword RLOAD1 (page 2384 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support, albeit with constant DELAY and DPHASE (see these commands)
     * and **without** imaginary D(f) term
     */
    void parseRLOAD1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword RLOAD2 (page 2387 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support, albeit with constant DELAY and DPHASE (see these commands)
     */
    void parseRLOAD2(NastranTokenizer& tok, Model& model);

    /**
     * Parse the SET1 keyword (page 2482 of MDN Nastran 2006 Quick Reference Guide.)
     * Set Definition Defines a list of structural grid points or element ID's.
     * Fully supported.
     */
    void parseSET1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the SET3 keyword (page 2457 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseSET3(NastranTokenizer& tok, Model& model);

    /**
     * Parse the SLOAD keyword (page 2464 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseSLOAD(NastranTokenizer& tok, Model& model);

    /**
     * Parse shell cells in standard form: CTRIA3, CTRIAR, CQUAD4
     */
    void parseShellElem(NastranTokenizer& tok, Model& model, CellType cellType); //in NastranParser_geometry.cpp

    /**
     * Parse the keyword SPC (page 2467 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPC(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword SPC1 (page 2468 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPC1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword SPCADD (page 2470 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPCADD(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword SPCD (page 2472 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPCD(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword SPOINT (page 2498 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPOINT(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword TABDMP1 (page 2512 of MDN Nastran 2006 Quick Reference Guide.)
     * Only CRIT type is supported, with an unknown reliability.
     */
    void parseTABDMP1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword TABLED1 (page 2518 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     * Defines a tabular function for use in generating frequency-dependent and
     * time-dependent dynamic loads.
     */
    void parseTABLED1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword TABLES1 (page 2518 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     * Defines a tabular function for stress-dependent material properties such as the
     * stress-strain curve (MATS1 entry), creep parameters (CREEP entry) and hyperelastic
     * material parameters (MATHP entry).
     */
    void parseTABLES1(NastranTokenizer& tok, Model& model);

    /**
     * Parse the keyword TEMP (page 2572 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseTEMP(NastranTokenizer& tok, Model& model);

    /**
     * PARAM WTMASS
     */
    void parseParamWTMASS(NastranTokenizer& tok, Model& model);

    std::string parseSubcase(NastranTokenizer& tok, Model& model, std::map<std::string, std::string> context);
    std::string parseSubcom(NastranTokenizer& tok, Model& model, std::map<std::string, std::string> context);

    /**
     *  Add the Cell to the CellGroup corresponding to the property_id (use getOrCreateCellGroup)
     */
    void addProperty(NastranTokenizer& tok, int property_id, int cell_id, Model& model);//in NastranParser_geometry.cpp
    /**
     *  Get the Cellgroup corresponding to the property_id. If this group does not exist, it is created.
     *  The name of the group is then "COMMAND_property_id".
     */
    std::shared_ptr<CellGroup> getOrCreateCellGroup(int group_id, Model& model, std::string comment);//in NastranParser_geometry.cpp

    /**
     * Parse and build an Orientation referentiel.
     * P1 is the origin O
     * P1P2 make the Ox axis
     * We then parse either vectorr V or a point P3 (v=P1P3) which defines Oy and Oz.
     * See, for example, the help on the CBAR keyword (page 1154 of MDN Nastran 2006 Quick Reference Guide.)
     */
    pos_t parseOrientation(int point1, int point2, NastranTokenizer& tok,
            Model& model);

    /**
     * Parse a DOF Nastran field to return a VEGA DOF
     *   - Scalar point have a "0" Nastran DOF, translated as a "0" (DX) Vega DOF
     *   - Classic Nastran dofs go from 1 to 6, VEGA from 0 to 5.
     */
    dof_int parseDOF(NastranTokenizer& tok, Model& model, bool returnDefaultIfNotFoundOrBlank = false, dof_int defaultValue = Globals::UNAVAILABLE_UCHAR);

    LogLevel logLevel = LogLevel::INFO;
    protected:
    // see also http://www.altairhyperworks.com/hwhelp/Altair/hw12.0/help/hm/hmbat.htm?design_variables.htm
    std::set<std::string> IGNORED_KEYWORDS = {
        "DCONSTR", "DCONADD", "DESVAR", "DLINK", //nastran optimization keywords
        "DOPTPRM", // Nastran optimization keyword
        "DEQATN", // Defines one or more equations for use in design sensitivity
        "DRAW", "DRESP1", "DRESP2", //ignored in Vega
        "EFFMAS", // Outputs modal participation factors and effective mass for normal modes analyses. Inutile de le traduire.
        "ENDDATA",
        "PLOTEL",  // Fictitious element for plotting
        "TOPVAR", //  Topological Design Variable
    };

    // See chapter 5 of the Nastran Quick Reference guide
    // Please keep alphabetical order for a better readibility
    std::set<std::string> IGNORED_PARAMS = {
        "BAILOUT", // Behavior when matrixes are almost singular. Useless for translation.
        "COUPMASS", // Generation of coupled rather than diagonal mass matrices for elements with coupled mass capability
        "DESPCH", "DESPCH1", // Amount of output data to write during an optimization process. Useless for translation.
        "NASPRT",  // Data recovery during optimization process. Useless for translation.
        "OUGCORD", // Choice of referentiel for printout
        "POST",    // Post-treatment parameter. Useless for translation.
        "PRGPST",  // Printout command
        "TINY"     // Printout command
    };

    struct fourCharsComparator {
        bool operator()(const std::string& a, const std::string& b) const {
            return a.compare(0, 4, b, 0, 4) < 0;
        }
    };

    // See 4.2 Case Control Command Descriptions of the Nastran Quick Reference guide page 192
    std::set<std::string, fourCharsComparator> ACCEPTED_CASE_CONTROL_COMMANDS = {
        "ANALYSIS", // Specifies the type of analysis being performed for the current subcase.
        "ASSIGN", // Assigns physical file names or other properties to DBset members or special FORTRAN files that are used by other FMS statements or DMAP modules.
        "AUTOSPC", // Requests that stiffness singularities and near singularities be automatically constrained via single or multipoint constraints.
        "B2GG", // Selects direct input damping matrices.
        "BCONTACT", // Requests line contact output
        "DESGLB", // IGNORED: Selects the design constraints to be applied at the global level in a design optimization task.
        "DESOBJ", // IGNORED: Selects the DRESP1 or DRESP2 entry to be used as the design objective.
        "DESSUB", // IGNORED: Selects the design constraints to be used in a design optimization task for the current subcase.
        "DESVAR", // IGNORED: Selects a set of DESVAR entries for the design set to be used.
        "DRSPAN", // IGNORED: Selects a set of DRESP1 entries for the current subcase that are to be used in a DRESP2 or DRESP3 response that spans subcase.
        "DISP", // Requests the form and type of displacement or pressure vector output.
        "DLOA", // Selects a dynamic load or an acoustic source to be applied in a transient or frequency response problem.
        "ECHO", // IGNORED: Controls echo (i.e., printout) of the Bulk Data.
        "ELCHECK",  // requires element quality checks
        "ELFORCE", // equivalent command to FORCE
        "ELSTRESS", // equivalent command to STRESS
        "ESE", // IGNORED: Requests the output of the strain energy in selected elements.
        "FORCE", // IGNORED: Requests the form and type of element force output or particle velocity output in coupled fluid-structural analysis.
        "FREQUENCY", // Selects the set of forcing frequencies to be solved in frequency response problems.
        "GPFORCE", // IGNORED: Requests grid point force balance at selected grid points.
        "GPSTRAIN", // IGNORED: Requests grid points strains for printing only.
        "GPSTRESS", // IGNORED: Requests grid point stresses for printing only.
        "K2GG", // Selects direct input stiffness matrices.
        "LABEL", // Defines a character string that will appear on the third heading line of each page of printer output.
        "LOAD", // Selects an external static loading set.
        "LOADSET", // Selects a sequence of static load sets to be applied to the structural model. The load sets may be referenced by dynamic load commands.
        "MAXIMUM DEFORM", // IGNORED: Defines the magnification of the maximum displacement. All other displacements are scaled accordingly.
        "MAXLINES", // IGNORED: Sets the maximum number of output lines.
        "M2GG", // Selects direct input mass matrices.
        "MEFFMASS", // IGNORED: Requests the output of the modal effective mass, participation factors, and modal effective mass fractions in normal modes analysis.
        "METHOD", // Selects the real eigenvalue extraction parameters.
        "MODTRAK", // IGNORED: Selects mode tracking options in design optimization (SOL 200).
        "MPC", // Selects a multipoint constraint set.
        "MPCFORCES", // IGNORED: Requests the form and type of multipoint force of constraint vector output.
        "NLPARM", // Selects the parameters used for nonlinear static analysis.
        "OFREQUENCY", // Selects the frequencies to output
        "OLOAD", // IGNORED: Requests the form and type of applied load vector output.
        "OUTPUT", // IGNORED: Delimits the various types of commands for the structure plotter, curve plotter, grid point stress, and MSGSTRESS.
        "PRESSURE", // IGNORED: Equivalent to DISPLACEMENT
        "RESVEC", // Specifies options for and calculation of residual vectors.
        "SDAMPING", // Requests modal damping as a function of natural frequency in modal solutions or viscoelastic materials as a function of frequency in direct frequency response analysis.
        "SET", // Defines a set of element identification numbers only for the SURFACE and VOLUME commands (grid point stress) or the OUTRCV Bulk Data entry (p-element data recovery).
        "SOL", //
        "SPC", // Selects a single-point constraint set to be applied.
        "SPCFORCES", // IGNORED: Requests the form and type of single-point force of constraint vector output.
        "STATSUB", // Selects static analysis or preload for buckling
        "STRAIN", // IGNORED: Requests the form and type of strain output.
        "STRESS", // Requests the form and type of element stress output.
        "STRFIELD", // IGNORED: Requests the computation of grid point stresses for graphical postprocessing and mesh stress discontinuities.
        "SUBSEQ", // Gives the coefficients for forming a linear combination of the previous subcases.
        "SUBTITLE", // Defines a subtitle that will appear on the second heading line of each page of printer output.
        "TITLE", // Defines a character string that will appear on the first heading line of each page of printer output.
        "VECTOR", // IGNORED: Equivalent to DISPLACEMENT,
        "XLOG", // IGNORED: Selects logarithmic or linear x-axis.
        "XBLOG", // IGNORED: Selects logarithmic or linear x-axis.
        "XTLOG", // IGNORED: Selects logarithmic or linear x-axis.
        "XTITLE", // IGNORED: Defines a character string that will appear along the x-axis.
        "XBTITLE", // IGNORED: Defines a character string that will appear along the x-axis.
        "XTTITLE", // IGNORED: Defines a character string that will appear along the x-axis.
        "XGRID LINES", // IGNORED: Controls the drawing of the grid lines
        "XBGRID LINES", // IGNORED: Controls the drawing of the grid lines
        "XTGRID LINES", // IGNORED: Controls the drawing of the grid lines
        "XYPLOT", // IGNORED: Generate X-Y plots for a plotter
        "YGRID LINES", // IGNORED: Controls the drawing of the grid lines
        "YLOG", // IGNORED: Selects logarithmic or linear y-axis.
        "YBLOG", // IGNORED: Selects logarithmic or linear y-axis.
        "YTLOG", // IGNORED: Selects logarithmic or linear y-axis.
        "YBGRID LINES", // IGNORED: Controls the drawing of the grid lines
        "YTITLE", // IGNORED: Defines a character string that will appear along the y-axis.
        "YBTITLE", // IGNORED: Defines a character string that will appear along the y-axis.
        "YTTITLE", // IGNORED: Defines a character string that will appear along the y-axis.
        "YTGRID LINES", // IGNORED: Controls the drawing of the grid lines
    };
public:
    NastranParser() = default;
    NastranParser(const NastranParser& that) = delete;
    virtual ~NastranParser() = default;
    std::unique_ptr<Model> parse(const ConfigurationParameters& configuration) override;
};

}
/* namespace nastran */
} /* namespace vega */
#endif /* NASTRANPARSER_H_ */
