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
#include "SystusModel.h"

namespace vega {

class SystusWriter: public Writer {
	int systusOption = 0;
	int maxNumNodes = 0;
	int nbNodes = 0;
	
	map<int, map<int, int>> lists;
	map<int, vector<double>> vectors;
	map<int, vector<int>> RbarPositions;     /** <material, <cells>> for all RBar (1002) elements. **/
	map<int, vector<int>> RBE2rbarPositions; /** <material, <cells>> for all RBE2 (1902/1903/1904) elements. **/
	map<int, vector<int>> RBE3rbarPositions; /** <material, <cells>> for all RBE3 elements. **/
	map<int, vector<DOFS>> RBE3Dofs;         /** <material, <master DOFS, slaves DOFS>> for all RBE3 elements. **/
	map<int, double> RBE3Coefs;              /** <material, coeff>  for all RBE3 elements. **/
	map<int, int> localLoadingListIdByLoadingListId;
	map<int, int> localVectorIdByLoadingListId;
	map<int, int> localVectorIdByConstraintListId;
	map<int, int> loadingListIdByNodePosition;
	map<int, int> constraintListIdByNodePosition;
	map<int, char> constraintByNodePosition;
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
	 *           2                            1
	 *         ,/ `\                        ,/ `\
	 *       ,/     `\                    ,/     `\
	 *     ,/         `\                ,/         `\
	 *    0-------------1              0-------------2\endcode
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
	 *   3-----------2                   1-----------2
	 *   |           |                   |           |
	 *   |           |                   |           |
	 *   |           |                   |           |
	 *   |           |                   |           |
	 *   |           |                   |           |
	 *   0-----------1                   0-----------3\endcode
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
	map<int, int> numberOfDofBySystusOption = boost::assign::map_list_of(3, 6)(4, 3);
	/** Converts a vega node Id in its ASC counterpart (i.e add one!) **/
	int getAscNodeId(const int vega_id) const;
	 /** Converts a vega DOFS to its ASC material counterpart.
	  *  Return the number of material field filled.
	  **/
	int DOFSToAsc(const DOFS dofs, ostream& out) const;
	/** Converts a vega DOFS to its integer Systus couterpart **/
	int DOFSToInt(const DOFS dofs) const;
	static const std::unordered_map<CellType::Code, vector<int>, hash<int>> systus2medNodeConnectByCellType;
	void writeAsc(const SystusModel&, const ConfigurationParameters&, const Analysis&, std::ostream&);
	void getSystusInformations(const SystusModel&, const ConfigurationParameters&);
	void fillLoads(const SystusModel&, const Analysis&);
	void fillVectors(const SystusModel&, const Analysis&);
	void fillConstraintLists(const std::shared_ptr<ConstraintSet> & , std::map<int, std::map<int, int>> &);
	void fillLists(const SystusModel&, const Analysis&);
	void generateRBEs(const SystusModel&, const ConfigurationParameters&);
	void writeHeader(const SystusModel&, std::ostream&);
	void writeInformations(const SystusModel&, std::ostream&);
	void writeNodes(const SystusModel&, std::ostream&);
	void writeElements(const SystusModel&, std::ostream&);
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
	void writeMaterials(const SystusModel&, const ConfigurationParameters&, std::ostream&);
	void writeLoads(const SystusModel&, const Analysis&, std::ostream&);
	void writeLists(const SystusModel&, std::ostream&);
	void writeVectors(const SystusModel&, const Analysis&, std::ostream&);
	void writeDat(const SystusModel&, const Analysis&, std::ostream&);
	void writeConstraint(const SystusModel& systusModel, const ConstraintSet&, std::ostream&);
	void writeLoad(const LoadSet&, std::ostream&);
	void writeLoad(const ConstraintSet&, std::ostream&);
	void writeNodalDisplacementAssertion(Assertion& assertion, ostream& out);
	void writeNodalComplexDisplacementAssertion(Assertion& assertion, ostream& out);
	void writeFrequencyAssertion(Assertion& assertion, ostream& out);
	virtual string toString() {
		return string("SystusWriter");
	}
	void writeMasses(const SystusModel&, std::ostream& out); /** Write Nodal Masses on outstream. **/

public:
	SystusWriter();
	virtual ~SystusWriter();

	string writeModel(const std::shared_ptr<Model> model, const ConfigurationParameters&)
		 override;
};

}
#endif /* SYSTUSBUILDER_H_ */
