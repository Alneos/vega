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
#include "BoundaryCondition.h"
#include "MeshComponents.h"
#include "ConfigurationParameters.h"
#if defined(__GNUC__)
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif

namespace vega {

class Mesh;

class NodeData final {
public:
    NodeData(int id, const DOFS& dofs, double x, double y, double z, int cpPos, int cdPos, int nodePart);
	const int id;
	char dofs;
	double x;
	double y;
	double z;
	int cpPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID; /**< Vega Position Number of the CS used for location (x,y,z) **/;
	int cdPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID; /**< Vega Position Number of the CS used for displacements, forces, constraints **/;
	int nodePart = 0; /**< Node grouping by element part */
};

class NodeStorage final {
private:
	friend Mesh;
	friend NodeGroup;
	const LogLevel logLevel;
	std::vector<NodeData> nodeDatas;
	typedef std::map<int, int>::const_iterator mapid_iterator;
	std::map<int, int> nodepositionById;
	static const double RESERVED_POSITION;
	static int lastNodePart;
public:
	Mesh& mesh;
	std::map<int, int> mainNodePartByCellPart;
	std::map<int, std::set<int>> cellPartsByNodePart;
	std::map<std::set<int>, int> interfaceNodePartByCellParts;
	std::set<int> reservedButUnusedNodePositions;

    class NodeIterator final: public std::iterator<std::input_iterator_tag, const Node> {
        friend NodeStorage;
        void increment();
        bool equal(NodeIterator const& other) const;
        NodeIterator(const NodeStorage& nodeStorage, mapid_iterator currentIdIterator);
        const NodeStorage& nodeStorage;
        mapid_iterator currentIdIterator;
    public:
        //java style iteration
        bool hasNext() const;
        const Node next();
        NodeIterator& operator++();
        NodeIterator operator++(int);
        bool operator==(const NodeIterator& rhs) const;
        bool operator!=(const NodeIterator& rhs) const;
        const Node operator*();
    };

	NodeStorage(Mesh& mesh, LogLevel logLevel);
	NodeIterator begin() const;
	NodeIterator end() const;
	const std::vector<NodeData>& getNodeDatas() const {
	    return nodeDatas;
	}
	int getMinNodeId() const {
	    return nodepositionById.begin()->first;
	}
	int getMaxNodeId() const {
	    return nodepositionById.rbegin()->first;
	}
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
	const std::vector<CellType> cellTypes() const;

	bool validate() const;
};

class MeshStatistics final {
public:
    double minLength;
    double minNonzeroLength;
    double maxLength;
    double quadraticMeanLength;
};

class Mesh final {

private:
	friend CellIterator;
	//access flag debug on model
	friend CellGroup;
	friend CoordinateSystemStorage;
	const LogLevel logLevel;
	const std::string name;
	bool finished = false;

	std::map<std::string, std::shared_ptr<Group>> groupByName;

	/**
	 * Groups ordered by the id provided by the input solver. Since inputSolver may not provide
	 * this id this map may not contain all the groups.
	 */
	std::map<int, std::shared_ptr<Group>> groupById;

	std::shared_ptr<CellGroup> getOrCreateCellGroupForCS(const int cspos);

	std::unique_ptr<MeshStatistics> stats = nullptr;
public:
	std::map<CellType, std::vector<int>> cellPositionsByType;
	std::map<int, std::string> cellGroupNameByCspos; /**< mapping position->group name **/
	std::map<int, std::string> cellGroupNameByMaterialOrientationTimes100;
	Mesh(LogLevel logLevel, const std::string& name);
	NodeStorage nodes;
	CellStorage cells;
	CoordinateSystemStorage coordinateSystemStorage; /**< Container for Coordinate System numerotations. **/

    std::shared_ptr<NodeGroup> createNodeGroup(const std::string& name, const int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	/**
	 * Find the NodeGroup named "name".
	 * If it does not exists, create and return a NodeGroup with specified name, groupId and comment.
	 **/
	std::shared_ptr<NodeGroup> findOrCreateNodeGroup(const std::string& name, const int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	bool hasGroup(const int groupId) const noexcept;
	bool hasGroup(const std::string& name) const noexcept;
	std::vector<std::shared_ptr<NodeGroup>> getNodeGroups() const noexcept;
	std::shared_ptr<CellGroup> createCellGroup(const std::string& name, int groupId = Group::NO_ORIGINAL_ID, const std::string& comment="");
	void renameGroup(const std::string& oldname, const std::string& newname, const std::string& comment);
    /**
     * Remove the Group named "name". Do nothing if the group does not exist.
     */
    void removeGroup(const std::string& name) noexcept;
	std::vector<std::shared_ptr<CellGroup>> getCellGroups() const noexcept;
	std::shared_ptr<Group> findGroup(std::string) const noexcept;
	/**
	 * Find a group by its "original" id: the id provided by the input solver. If not found
	 * returns nullptr
	 */
	std::shared_ptr<Group> findGroup(const int originalId) const noexcept;
    /**
     * Find or Reserve a Coordinate System in the model by Input Id.
     * Return the VEGA Id (position) of the Coordinate System.
     */
    int findOrReserveCoordinateSystem(const Reference<CoordinateSystem>);
    void add(const CoordinateSystem& coordinateSystem);
	std::shared_ptr<CoordinateSystem> findCoordinateSystem(const Reference<CoordinateSystem> csref) const;
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
	int addNode(const int id, const double x, const double y, const double z = 0,
	        const int cpPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
	        const int cdPos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
	        const int nodePartId = 0) noexcept;
    const std::string getName() const noexcept;
	size_t countNodes() const noexcept;
	void allowDOFS(const int nodePosition, const DOFS& allowed) noexcept;
	/**
	 * Find a node from its Vega position.
	 * throws invalid_argument if node not found
	 */
	const Node findNode(const int nodePosition) const;

  /**
	 * given an internal node position returns the Id from the model
	 * @return Node::UNAVAILABLE_NODE if not found
	 */
	int findNodeId(const int nodePosition) const noexcept;

	/**
	 * given an Id from the model returns an internal node position
	 * use together with findNode.
	 * @return Node::UNAVAILABLE_NODE if not found
	 */
	int findNodePosition(const int nodeId) const noexcept;

    /**
	 * given an internal node position returns the nodepartId
	 * @return Node::UNAVAILABLE_NODE if not found
	 */
	int findNodePartId(const int nodePosition) const noexcept;
	int findOrReserveNode(const int nodeId, const int cellPartId = Globals::UNAVAILABLE_INT) noexcept;
	//returns a set of nodePositions
	std::set<int> findOrReserveNodes(const std::set<int>& nodeIds) noexcept;

	int countCells() const noexcept;
	int countCells(const CellType &type) const noexcept;
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
    int findCellPosition(int cellId) const noexcept;
    inline int findCellId(int cellPosition) const noexcept {
        return cells.cellDatas[cellPosition].id;
    };
	const Cell findCell(int cellPosition) const;
	int generateSkinCell(const std::vector<int>& faceIds, const SpaceDimension& dimension);
    std::pair<Cell, int> volcellAndFaceNum_from_skincell(const Cell& skinCell) const;
	bool hasCell(int cellId) const noexcept;

	/**
	 * Assign an elementId (an integer) to a group of cells.
	 * Cells must have been previously defined.
	 */
	inline void assignElementId(int cellPosition, int elementId) noexcept {
	    cells.cellDatas[cellPosition].elementId = elementId;
    }

	const MeshStatistics calcStats();

	void finish() noexcept;
	bool validate() const;
	Mesh(const Mesh& that) = delete;
};

} /* namespace vega */

#endif /* MESH_H_ */
