/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Mesh.h
 *
 *  Created on: 26 févr. 2013
 *      Author: dallolio
 */

#ifndef MESH_H_
#define MESH_H_

#include <array>
#include <string>
#include <stdexcept>
#include <boost/range.hpp>
#include <boost/functional/hash.hpp>
#include "BoundaryCondition.h"
#include "MeshComponents.h"
#include "ConfigurationParameters.h"
#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif
#include <med.h>
#define MESGERR 1

namespace vega {

class Mesh;

class NodeData final {
public:
	int id;
	char dofs;
	double x;
	double y;
	double z;
	int displacementCS;
};

class NodeStorage final {
private:
	friend Mesh;
	friend NodeGroup;

	const LogLevel logLevel;
	std::vector<NodeData> nodeDatas;
	std::map<int, int> nodepositionById;
	/*
	 * Reserve a node position given an id
	 */
	int reserveNodePosition(int nodeId);
	static const double RESERVED_POSITION;
public:
	Mesh* mesh;

	NodeStorage(Mesh* mesh, LogLevel logLevel);
	NodeIterator begin() const;
	NodeIterator end() const;

	bool validate() const;
};

class CellData final {
public:
	CellData(int id, CellType type, bool isvirtual, int elementId, int cellTypePosition);
	int id;
	CellType::Code typeCode;
	bool isvirtual;
	int orientationId = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
	int elementId;
	int cellTypePosition;
};

class CellStorage final {
private:
	friend Mesh;
	friend NodeGroup;
	friend CellGroup;

	const LogLevel logLevel;
	std::vector<CellData> cellDatas;
	std::map<int, int> cellpositionById;
	std::map<CellType, std::shared_ptr<std::deque<int>>> nodepositionsByCelltype;
	/*
	 * Reserve a cell position given an id
	 */
	int reserveCellPosition(int nodeId);
public:
	Mesh* mesh;
	CellStorage(Mesh* mesh, LogLevel logLevel);
	CellIterator cells_begin(const CellType &type) const;
	CellIterator cells_end(const CellType &type) const;

	bool validate() const;
};

class Mesh final {

private:
	friend NodeIterator;
	friend CellIterator;
	//access flag debug on model
	friend CellGroup;
	const LogLevel logLevel;
	const string name;
	bool finished;

	//mapping position->external id
	std::unordered_map<CellType, vector<int>, std::hash<CellType>> cellPositionsByType;

	std::unordered_map<string, Group*> groupByName;
	/**
	 * Groups ordered by the id provided by the input solver. Since inputSolver may not provide
	 * this id this map may not contain all the groups.
	 */
	map<int, Group*> groupById;

	CellGroup * getOrCreateCellGroupForOrientation(const std::shared_ptr<Orientation> orientation);
	void createFamilies(med_idt fid, const char meshname[MED_NAME_SIZE + 1],
			vector<Family>& families);
public:
	static const int UNAVAILABLE_NODE = INT_MIN;
	static const int UNAVAILABLE_CELL = INT_MIN;
	static const int UNAVAILABLE_ELEM = INT_MIN;

	//TODO: maybe add a field into Orientation?, make it private?
	std::map<std::shared_ptr<Orientation>, string> cellGroupName_by_orientation;
	Mesh(LogLevel logLevel, const string& name);
	~Mesh();
	NodeStorage nodes;
	CellStorage cells;

	NodeGroup* createNodeGroup(const string& name, int groupId = Group::NO_ORIGINAL_ID);
	std::vector<NodeGroup*> getNodeGroups() const;
	CellGroup* createCellGroup(const string& name, int groupId = Group::NO_ORIGINAL_ID);
	std::vector<CellGroup*> getCellGroups() const;
	Group* findGroup(string) const;
	/**
	 * Find a group by its "original" id: the id provided by the input solver. If not found
	 * returns nullptr
	 */
	Group* findGroup(int originalId) const;

	int addNode(int id, double x, double y, double z = 0, int cd_id =
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	int countNodes() const;
	void allowDOFS(int nodePosition, const DOFS allowed);
	/**
	 * throws invalid_argument if node not found
	 */
	const Node findNode(int nodePosition) const;
	/**
	 * given an Id from the model returns an internal node position
	 * use together with findNode.
	 * @return UNAVAILABLE_NODE if not found
	 */
	int findNodePosition(const int nodeId) const;
	int findOrReserveNode(int nodeId);
	//returns a set of nodePositions
	set<int> findOrReserveNodes(const std::set<int>& nodeIds);

	void add_orientation(int cellId, const Orientation& orientation);
	int countCells() const;
	int countCells(const CellType &type) const;
	int addCell(int id, const CellType &type, const std::vector<int> &connectivity,
			bool virtualCell = false, const Orientation* = nullptr, int elementId = UNAVAILABLE_ELEM);
	int findCellPosition(int cellId) const;
	const Cell findCell(int cellPosition) const;
	bool hasCell(int cellId) const;

	/**
	 * Assign an elementId (an integer) to a group of cells.
	 * Cells must have been previously defined.
	 */
	void assignElementId(const CellContainer&, int elementId);

	void writeMED(const char* medFileName);
	void finish();
	bool validate() const;
};

} /* namespace vega */

#endif /* MESH_H_ */
