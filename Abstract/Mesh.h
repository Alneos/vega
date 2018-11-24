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
#if defined(__GNUC__)
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif
#include <med.h>
#define MESGERR 1

namespace vega {

class Mesh;

class NodeData final {
public:
    NodeData(int id, const DOFS& dofs, double x, double y, double z, int cpPos, int cdPos);
	const int id;
	char dofs;
	double x;
	double y;
	double z;
	int cpPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID; /**< Vega Position Number of the CS used for location (x,y,z) **/;
	int cdPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID; /**< Vega Position Number of the CS used for displacements, forces, constraints **/;
};

class NodeStorage final {
private:
	friend Mesh;
	friend NodeGroup;

	const LogLevel logLevel;
	std::vector<NodeData> nodeDatas;
	std::map<int, int> nodepositionById;
	/**
	 * Reserve a node position (VEGA Id) given a node id (input model id).
	 * WARNING! Reserving an already created node will erase the previous value
	 * of NodeData ! Use Mesh::findOrReserveNode to avoid this problem.
	 **/
	int reserveNodePosition(int nodeId);
	static const double RESERVED_POSITION;
public:
	Mesh& mesh;

	NodeStorage(Mesh& mesh, LogLevel logLevel);
	NodeIterator begin() const;
	NodeIterator end() const;

	bool validate() const;
};

class CellData final {
public:
	CellData(int id, const CellType& type, bool isvirtual, int elementId, int cellTypePosition);
	const int id;
	const CellType::Code typeCode;
	const bool isvirtual;
	int csPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID; /**< Vega Position Number for the CS **/
	int elementId;
	const int cellTypePosition;
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
	Mesh& mesh;
	CellStorage(Mesh& mesh, LogLevel logLevel);
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
	friend CoordinateSystemStorage;
	const LogLevel logLevel;
	const std::string name;
	bool finished;

	//mapping position->external id
	std::map<CellType, std::vector<int>> cellPositionsByType;

	std::map<std::string, std::shared_ptr<Group>> groupByName;

	/**
	 * Groups ordered by the id provided by the input solver. Since inputSolver may not provide
	 * this id this map may not contain all the groups.
	 */
	std::map<int, std::shared_ptr<Group>> groupById;

	std::shared_ptr<CellGroup> getOrCreateCellGroupForCS(const int cid);
	void createFamilies(med_idt fid, const char meshname[MED_NAME_SIZE + 1],
			const std::vector<Family>& families);
public:

	std::map<int, std::string> cellGroupNameByCID;
	std::map<int, std::string> cellGroupNameByMaterialOrientationTimes100;
	Mesh(LogLevel logLevel, const std::string& name);
	NodeStorage nodes;
	CellStorage cells;
	CoordinateSystemStorage coordinateSystemStorage; /**< Container for Coordinate System numerotations. **/

    std::shared_ptr<NodeGroup> createNodeGroup(const std::string& name, int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	/**
	 * Find the NodeGroup named "name".
	 * If it does not exists, create and return a NodeGroup with specified name, groupId and comment.
	 **/
	std::shared_ptr<NodeGroup> findOrCreateNodeGroup(const std::string& name, int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	std::vector<std::shared_ptr<NodeGroup>> getNodeGroups() const;
	std::shared_ptr<CellGroup> createCellGroup(const std::string& name, int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	void renameGroup(const std::string& oldname, const std::string& newname, const std::string& comment);
    /**
     * Remove the Group named "name". Do nothing if the group does not exist.
     */
    void removeGroup(const std::string& name);
	std::vector<std::shared_ptr<CellGroup>> getCellGroups() const;
	std::shared_ptr<Group> findGroup(std::string) const;
	/**
	 * Find a group by its "original" id: the id provided by the input solver. If not found
	 * returns nullptr
	 */
	std::shared_ptr<Group> findGroup(int originalId) const;
    /**
     * Find or Reserve a Coordinate System in the model by Input Id.
     * Return the VEGA Id (position) of the Coordinate System.
     */
    int findOrReserveCoordinateSystem(int cid);
    void add(const CoordinateSystem& coordinateSystem);
	std::shared_ptr<CoordinateSystem> getCoordinateSystem(int cid) const; /**< Return a CoordinateSystem by its USER Id **/
    /**
     * Add or Find an Orientation Coordinate System in the model.
     * Return the Position of the Orientation Coordinate System.
     */
    int addOrFindOrientation(const OrientationCoordinateSystem & ocs);
    /**
     * Find an Orientation Coordinate System in the model, by checking its axis.
     * Return 0 if nothing has been found.
     */
    int findOrientation(const OrientationCoordinateSystem & ocs) const;
    /**
     * Get a Coordinate System in the model from its VEGA Position.
     * Return nullptr if nothing has been found.
     */
    std::shared_ptr<vega::CoordinateSystem> getCoordinateSystemByPosition(const int pos) const;
	int addNode(int id, double x, double y, double z = 0,
	        int cpPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
	        int cdPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	int countNodes() const;
	void allowDOFS(int nodePosition, const DOFS& allowed);
	/**
	 * Find a node from its Vega position.
	 * throws invalid_argument if node not found
	 */
	const Node findNode(const int nodePosition) const;

  /**
	 * given an internal node position returns the Id from the model
	 * @return Node::UNAVAILABLE_NODE if not found
	 */
	int findNodeId(const int nodePosition) const;

	/**
	 * given an Id from the model returns an internal node position
	 * use together with findNode.
	 * @return Node::UNAVAILABLE_NODE if not found
	 */
	int findNodePosition(const int nodeId) const;
	int findOrReserveNode(int nodeId);
	//returns a set of nodePositions
	std::set<int> findOrReserveNodes(const std::set<int>& nodeIds);

	int countCells() const;
	int countCells(const CellType &type) const;
	/** Add a cell to the mesh.
	 *  The vector nodesIds regroups the nodes use to build the cell. Nodes Ids are expressed as "input node number"
	 *  and will be added to the model if not already defined.
	 **/
    int addCell(int id, const CellType &type, const std::vector<int> &nodesIds,
            bool virtualCell = false, const int cpos=CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int elementId = Cell::UNAVAILABLE_CELL);
    /**
     *  Update a cell to the mesh.
	 *  The vector nodesIds regroups the nodes use to build the cell. Nodes Ids are expressed as "input node number"
	 *  and will be added to the model if not already defined.
	 *  TODO: The previous CellData is NOT erased, has it complexify a LOT the update. Instead
	 *  an other CellData is created, and "branched over".
	 **/
    int updateCell(int id, const CellType &type, const std::vector<int> &nodesIds,
            bool virtualCell = false, const int cpos=CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int elementId = Cell::UNAVAILABLE_CELL);
    int findCellPosition(int cellId) const;
    inline int findCellId(int cellPosition) const {
        return cells.cellDatas[cellPosition].id;
    };
	const Cell findCell(int cellPosition) const;
	bool hasCell(int cellId) const;

	/**
	 * Assign an elementId (an integer) to a group of cells.
	 * Cells must have been previously defined.
	 */
	void assignElementId(const CellContainer&, int elementId);

	void writeMED(const Model& model, const char* medFileName);
	void finish();
	bool validate() const;
	Mesh(const Mesh& that) = delete;
};

} /* namespace vega */

#endif /* MESH_H_ */
