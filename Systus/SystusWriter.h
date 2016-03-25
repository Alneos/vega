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
	class List: public Identifiable<List> {
		map<int, int> iVectByiLoad;
	public:
		List(const map<int, int>& iVectByiLoad) :
				Identifiable(), iVectByiLoad(iVectByiLoad) {
		}
		void write(std::ostream& out) {
			out << getId();
			for (auto it : iVectByiLoad)
				out << " " << it.first << " " << it.second;
			out << endl;
		}
	};
	vector<List> lists;
	map<int, vector<double>> vectors;
	vector<int> RBE2rbarPositions;
	vector<int> RBE3rbarPositions;
	int RBE2rbarsElementId = 0;
	int RBE3rbarsElementId = 0;
	map<int, int> localLoadingListIdByLoadingListId;
	map<int, int> localVectorIdByLoadingListId;
	map<int, int> loadingListIdByNodePosition;
	map<int, int> constraintListIdByNodePosition;
	map<int, char> constraintByNodePosition;
	// vs 2013 compiler bug	in initializer list {{3,6}, {4,3}} not supported
	map<int, int> numberOfDofBySystusOption = boost::assign::map_list_of(3, 6)(4, 3);
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
	 */
	static const std::unordered_map<CellType::Code, vector<int>, hash<int>> systus2medNodeConnectByCellType;
	void writeAsc(const SystusModel&, const Analysis&, std::ostream&);
	void getSystusInformations(const SystusModel&);
	void fillLoads(const SystusModel&, const Analysis&);
	void fillVectors(const SystusModel&, const Analysis&);
	void fillLists(const SystusModel&, const Analysis&);
	void generateRBEs(const SystusModel&);
	void writeHeader(const SystusModel&, std::ostream&);
	void writeInformations(const SystusModel&, std::ostream&);
	void writeNodes(const SystusModel&, std::ostream&);
	void writeElements(const SystusModel&, std::ostream&);
	void writeGroups(const SystusModel&, std::ostream&);
	void writeMaterials(const SystusModel&, std::ostream&);
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
	void writeMasses(const SystusModel&, std::ostream& out);

public:
	SystusWriter();
	virtual ~SystusWriter();

	string writeModel(const std::shared_ptr<Model> model, const ConfigurationParameters&)
		 override;
};

}
#endif /* SYSTUSBUILDER_H_ */
