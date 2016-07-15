/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
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
	static const std::unordered_map<string, parseElementFPtr> PARSE_FUNCTION_BY_KEYWORD;

	void addAnalysis(std::shared_ptr<Model> model, std::map<std::string, std::string>& context, int analysis_id =
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
	void parseCBAR(NastranTokenizer& tok, std::shared_ptr<Model> model); //in NastranParser_geometry.cpp
	void parseCBEAM(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseElem(NastranTokenizer& tok, std::shared_ptr<Model> model, vector<CellType>);//in NastranParser_geometry.cpp
	void parseCELAS2(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCELAS4(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCGAP(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCHEXA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCMASS2(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCONM2(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCORD1R(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCORD2R(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCPENTA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCPYRAM(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCQUAD(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCROD(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseCTETRA(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseFORCE(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseFORCE1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseGRDSET(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseGRID(NastranTokenizer& tok, std::shared_ptr<Model> model);//in NastranParser_geometry.cpp
	void parseGRAV(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseInclude(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseMAT1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseMATS1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseMOMENT(NastranTokenizer& tok, std::shared_ptr<Model> model);
	/**
	 * Parse the PBAR keyword. Neither Stress coefficients nor i12 are supported.
	 * */
	void parsePBAR(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePBARL(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePBEAM(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePBEAML(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePGAP(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePLOAD4(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePROD(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePSHELL(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePSOLID(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRBAR(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRBAR1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRBE2(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRFORCE(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseDAREA(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseEIGR(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseEIGRL(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseFREQ1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseDLOAD(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseDMIG(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRLOAD2(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseDPHASE(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseTABDMP1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseTABLED1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseNLPARM(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parsePARAM(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseSPCADD(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseLOAD(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseCORD2C(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseRBE3(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseMPC(NastranTokenizer& tok, std::shared_ptr<Model> model);

	/**
	 * Parse cells in standard form: CTRIA3,CTRIAR,QUAD4
	 */
	void parseShellElem(NastranTokenizer& tok, std::shared_ptr<Model> model, CellType cellType); //in NastranParser_geometry.cpp
	void parseSPC(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseSPC1(NastranTokenizer& tok, std::shared_ptr<Model> model);
	void parseSPCD(NastranTokenizer& tok, std::shared_ptr<Model> model);
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

	void handleParseException(vega::ParsingException &e, std::shared_ptr<Model> model, string message = "");
	void handleParsingError(const string& message, NastranTokenizer& tok, std::shared_ptr<Model> model);
	void handleParsingWarning(const string& message, NastranTokenizer& tok, std::shared_ptr<Model> model);
	int parseOrientation(int point1, int point2, NastranTokenizer& tok,
			std::shared_ptr<Model> model);

	/**
	 * This variable is not reentrant.
	 */
	ConfigurationParameters::TranslationMode translationMode;
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
