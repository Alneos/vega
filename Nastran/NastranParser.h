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

namespace vega {

namespace nastran {

namespace fs = boost::filesystem;

class NastranParserImpl: public vega::Parser {
private:
    class GrdSet {
    public:
        GrdSet(const int cp = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
                const int cd = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
                const int ps = 0, const int seid = 0) :
                    cp(cp), cd(cd), ps(ps), seid(seid) {
        }
        int cp;
        int cd;
        int ps;
        int seid;
    };
    GrdSet grdSet;

    std::unordered_map<string, shared_ptr<Reference<ElementSet>>> directMatrixByName;
    typedef void (NastranParserImpl::*parseElementFPtr)(NastranTokenizer& tok, std::shared_ptr<Model> model);
    static const std::set<string> IGNORED_KEYWORDS;
    static const std::set<string> IGNORED_PARAMS;
    static const std::unordered_map<string, parseElementFPtr> PARSE_FUNCTION_BY_KEYWORD;

    void addAnalysis(NastranTokenizer& tok, std::shared_ptr<Model> model, std::map<std::string, std::string>& context, int analysis_id =
            Analysis::NO_ORIGINAL_ID);
    void addCellIds(ElementLoading& loading, int eid1, int eid2);

    fs::path findModelFile(const string& filename);
    void parseBULKSection(NastranTokenizer &tok, std::shared_ptr<Model> model1);

    void parseExecutiveSection(NastranTokenizer& tok, std::shared_ptr<Model> model, map<string, string>& context);
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
    static const std::unordered_map<CellType::Code, vector<int>, std::hash<int>> nastran2medNodeConnectByCellType;
    /**
     * Parse the CBAR keyword (page 1154 of MDN Nastran 2006 Quick Reference Guide.)
     * Pin flags (PA, PB) and offset vectors (OFFT, WA, WB) are not supported.
     */
    void parseCBAR(NastranTokenizer& tok, std::shared_ptr<Model> model); //in NastranParser_geometry.cpp

    /**
     * Parse the CBEAM keyword (page 1161 of MDN Nastran 2006 Quick Reference Guide.)
     * Pin flags (PA, PB), offset vectors (OFFT, WiA, WiB) and grid point identification (SA, SB)
     * are not supported.
     */
    void parseCBEAM(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CBUSH keyword (page 1174 of MDN Nastran 2006 Quick Reference Guide.)
     * The location of spring damper (S), coordinate system  of spring damper (OCID) and
     * component of spring-damper-offset (S1, S2, S3) are not supported.
     */
    void parseCBUSH(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS1 keyword (page 1199 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseCELAS1(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS2 keyword (page 1202 of MDN Nastran 2006 Quick Reference Guide.)
     * The stress coefficient (S) is ignored, as it's only used for post-treatment.
     */
    void parseCELAS2(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CELAS4 keyword (page 1207 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseCELAS4(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CGAP keyword (page 1216 of MDN Nastran 2006 Quick Reference Guide.)
     * Only support Two nodes gap.
     */
    void parseCGAP(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCHEXA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CMASS2 keyword (page 1243 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseCMASS2(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CONM2 keyword (page 1251 of MDN Nastran 2006 Quick Reference Guide.)
     * CID is not supported.
     */
    void parseCONM2(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the CORD1R keyword (page 1264 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseCORD1R(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the CORD2C keyword (page 1271 of MDN Nastran 2006 Quick Reference Guide.)
     * Reference coordinate system (RID) not supported.
     */
    void parseCORD2C(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the CORD2R keyword (page 1273 of MDN Nastran 2006 Quick Reference Guide.)
     * Reference coordinate system (RID) not supported.
     */
    void parseCORD2R(NastranTokenizer& tok, std::shared_ptr<Model> model);
    void parseCPENTA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCPYRAM(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCQUAD(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCQUAD4(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCQUAD8(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCQUADR(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the CROD keyword (page 1310 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseCROD(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCTETRA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCTRIA3(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCTRIA6(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseCTRIAR(NastranTokenizer& tok, shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the keyword DAREA (page 1377 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseDAREA(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword DELAY (page 1386 of MDN Nastran 2006 Quick Reference Guide.)
     * Point identification (P), Component number (C) and a second phase are
     * ignored.
     */
    void parseDELAY(NastranTokenizer& tok, shared_ptr<Model> model);

    /**
     * Parse the keyword DLOAD (page 1398 of MDN Nastran 2006 Quick Reference Guide.)
     * Scale factors (S and Si) must be equal to 1.
     */
    void parseDLOAD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword DMIG (page 1046 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseDMIG(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword DPHASE (page 1433 of MDN Nastran 2006 Quick Reference Guide.)
     * Point identification (P), Component number (C) and a second phase are
     * ignored.
     */
    void parseDPHASE(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword EIGR (page 1517 of MDN Nastran 2006 Quick Reference Guide.)
     * Only Lanczos Method is supported.
     */
    void parseEIGR(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword EIGRL (page 1522 of MDN Nastran 2006 Quick Reference Guide.)
     * Options are not supported.
     */
    void parseEIGRL(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     *  Generic function for parsing Element keywords.
     */
    void parseElem(NastranTokenizer& tok, std::shared_ptr<Model> model, vector<CellType>);//in NastranParser_geometry.cpp

    /**
     * Parse the FORCE keyword (page 1549 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the FORCE1 keyword (page 1550 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the FORCE2 keyword (page 1552 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseFORCE2(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword FREQ1 (page 1556 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseFREQ1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword FREQ4
     */
    void parseFREQ4(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the GRAV keyword (page 1651 of MDN Nastran 2006 Quick Reference Guide.)
     * CID and MB are not supported.
     */
    void parseGRAV(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the GRDSET keyword (page 1623 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseGRDSET(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the GRID keyword (page 1624 of MDN Nastran 2006 Quick Reference Guide.)
     * CP coordinate system not supported, and taken as blank.
     */
    void parseGRID(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    void parseInclude(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the LSEQ keyword
     * Fully supported.
     */
    void parseLSEQ(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the LOAD keyword (page 1646 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseLOAD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the MAT1 keyword (page 1664 of MDN Nastran 2006 Quick Reference Guide.)
     * Structural element damping coefficient (GE) are not supported.
     */
    void parseMAT1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword MATS1 (page 1928 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseMATS1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword MOMENT (page 1984 of MDN Nastran 2006 Quick Reference Guide.)
     * Coordinate System (CID) not supported.
     */
    void parseMOMENT(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword MPC (page 1997 of MDN Nastran 2006 Quick Reference Guide.)
     * Unknown reliability.
     */
    void parseMPC(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword NLPARM (page 2014 of MDN Nastran 2006 Quick Reference Guide.)
     * All parameters are ignored except NINC.
     */
    void parseNLPARM(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword PARAM (page 2088 of MDN Nastran 2006 Quick Reference Guide.)
     * Partial support, with unknown reliability.
     */
    void parsePARAM(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PBAR keyword (page 2092 of MDN Nastran 2006 Quick Reference Guide.)
     * Neither Stress coefficients nor i12 are supported.
     */
    void parsePBAR(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PBARL keyword (page 2094 of MDN Nastran 2006 Quick Reference Guide.)
     * Only BAR and ROD type are supported.
     */
    void parsePBARL(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PBEAM keyword (page 2109 of MDN Nastran 2006 Quick Reference Guide.)
     * Shear center, sections, area of product are not implemented.
     */
    void parsePBEAM(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PBEAML keyword (page 2145 of MDN Nastran 2006 Quick Reference Guide.)
     * Only BAR, ROD and I types are supported.
     */
    void parsePBEAML(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PBUSH keyword (page 2165 of MDN Nastran 2006 Quick Reference Guide.)
     * Only Stiffness (K) is supported.
     */
    void parsePBUSH(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp

    /**
     * Parse the PELAS keyword (page 2202 of MDN Nastran 2006 Quick Reference Guide.)
     * The stress coefficient (S) is ignored, as it's only used for post-treatment.
     */
    void parsePELAS(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PGAP keyword (page 2210 of MDN Nastran 2006 Quick Reference Guide.)
     * Only U0 is supported.
     */
    void parsePGAP(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PLOAD1 keyword
     */
    void parsePLOAD1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PLOAD2 keyword
     */
    void parsePLOAD2(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PLOAD4 keyword (page 2227 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform pressure are supported.
     */
    void parsePLOAD4(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PROD keyword (page 2245 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform pressure are supported.
     */
    void parsePROD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PSHELL keyword (page 2250 of MDN Nastran 2006 Quick Reference Guide.)
     * Only uniform PSHELLs are supported, meaning MID2, MID3, MID4, etc are dismissed.
     */
    void parsePSHELL(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the PSOLID keyword (page 2264 of MDN Nastran 2006 Quick Reference Guide.)
     * Partial support.
     */
    void parsePSOLID(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the RBAR keyword (page 2312 of MDN Nastran 2006 Quick Reference Guide.)
     * CMA, CMB and the thermal expansion (ALPHA) are not supported.
     */
    void parseRBAR(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the RBAR1 keyword (page 2314 of MDN Nastran 2006 Quick Reference Guide.)
     * Thermal expansion (ALPGA) is not supported.
     */
    void parseRBAR1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the RBE2 keyword (page 2319 of MDN Nastran 2006 Quick Reference Guide.)
     * Only rigid constraint (dofs=123456) is supported.
     */
    void parseRBE2(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the RBE3 keyword (page 2334 of MDN Nastran 2006 Quick Reference Guide.)
     * No support of ALPHA nor UM.
     */
    void parseRBE3(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the RFORCE keyword (page 2373 of MDN Nastran 2006 Quick Reference Guide.)
     * No support of METHOD, RACC and MB.
     */
    void parseRFORCE(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword RLOAD2 (page 2387 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support, albeit with constant DELAY and DPHASE (see theses commands)
     */
    void parseRLOAD2(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the SLOAD keyword (page 2464 of MDN Nastran 2006 Quick Reference Guide.)
     * Fully supported.
     */
    void parseSLOAD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse shell cells in standard form: CTRIA3, CTRIAR, CQUAD4
     */
    void parseShellElem(NastranTokenizer& tok, std::shared_ptr<Model> model, CellType cellType); //in NastranParser_geometry.cpp

    /**
     * Parse the keyword SPC (page 2467 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPC(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword SPC1 (page 2468 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPC1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword SPCADD (page 2470 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPCADD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword SPCD (page 2472 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPCD(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword SPOINT (page 2498 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseSPOINT(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword TABDMP1 (page 2512 of MDN Nastran 2006 Quick Reference Guide.)
     * Only CRIT type is supported, with an unknown reliability.
     */
    void parseTABDMP1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword TABLED1 (page 2518 of MDN Nastran 2006 Quick Reference Guide.)
     * Full support.
     */
    void parseTABLED1(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the keyword TEMP (page 2572 of MDN Nastran 2006 Quick Reference Guide.)
     */
    void parseTEMP(NastranTokenizer& tok, std::shared_ptr<Model> model);

    string parseSubcase(NastranTokenizer& tok, std::shared_ptr<Model> model, map<string, string> context);

    /**
     *  Add the Cell to the CellGroup corresponding to the property_id (use getOrCreateCellGroup)
     */
    void addProperty(int property_id, int cell_id, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
    /**
     *  Get the Cellgroup corresponding to the property_id. If this group does not exist, it is created.
     *  The name of the group is then "COMMAND_property_id".
     */
    CellGroup* getOrCreateCellGroup(int property_id, std::shared_ptr<Model> model, const std::string & command="CGVEGA");//in NastranParser_geometry.cpp

    /**
     * Parse and build an Orientation referentiel.
     * P1 is the origin O
     * P1P2 make the Ox axis
     * We then parse either vectorr V or a point P3 (v=P1P3) which defines Oy and Oz.
     * See, for example, the help on the CBAR keyword (page 1154 of MDN Nastran 2006 Quick Reference Guide.)
     */
    int parseOrientation(int point1, int point2, NastranTokenizer& tok,
            std::shared_ptr<Model> model);

    /**
     * Parse a DOF Nastran field to return a VEGA DOF
     *   - Scalar point have a "0" Nastran DOF, translated as a "0" (DX) Vega DOF
     *   - Classic Nastran dofs go from 1 to 6, VEGA from 0 to 5.
     */
    int parseDOF(NastranTokenizer& tok, std::shared_ptr<Model> model);

    LogLevel logLevel = LogLevel::INFO;
public:
    NastranParserImpl();
    virtual ~NastranParserImpl();
    std::shared_ptr<Model> parse(const ConfigurationParameters& configuration) override;
};

}
/* namespace nastran */
} /* namespace vega */
#endif /* NASTRANPARSER_H_ */
