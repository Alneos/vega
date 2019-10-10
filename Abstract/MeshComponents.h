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
 * MeshComponents.h
 *
 *  Created on: Nov 4, 2013
 *      Author: devel
 */

#ifndef MESHCOMPONENTS_H_
#define MESHCOMPONENTS_H_

#include "CoordinateSystem.h"
#include "Dof.h"
#include <boost/functional/hash.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <iterator>
#if defined(__GNUC__) || defined(__MINGW32__)
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif

// Add specific elements not already defined by MED
// EVERY NUMBER MUST BE UNIQUE !
using vega_med_geometrie_element = enum {MED_POINT1=1, //
              MED_SEG2 = 102, MED_SEG3 = 103, MED_SEG4 = 104, MED_SEG5=105, //
              MED_TRIA3 = 203, MED_QUAD4 = 204, MED_TRIA6 = 206, MED_TRIA7 = 207, MED_QUAD8 = 208, MED_QUAD9 = 209, //
              MED_TETRA4 = 304, MED_PYRA5 = 305, MED_PENTA6 = 306, MED_HEXA8 = 308, MED_TETRA10 = 310, MED_OCTA12 = 312, MED_PYRA13 = 313, MED_PENTA15 = 315, MED_HEXA20 = 320, MED_HEXA27 = 327, //
              MED_POLYGON = 400, MED_POLYGON2 = 420, MED_POLYHEDRON = 500,//
              MED_POLY3=1303, MED_POLY4=1304, MED_POLY5=1305, MED_POLY6=1306, MED_POLY7=1307, MED_POLY8=1308, MED_POLY9=1309,
              MED_POLY10=1310, MED_POLY11=1311, MED_POLY12=1312, MED_POLY13=1313, MED_POLY14=1314, MED_POLY15=1315, MED_POLY16=1316,
              MED_POLY17=1317, MED_POLY18=1318, MED_POLY19=1319, MED_POLY20=1320};

namespace vega {

class SpaceDimension final {

public:

    enum class Code {
        DIMENSION0D_CODE = 0,
        DIMENSION1D_CODE = 1,
        DIMENSION2D_CODE = 2,
        DIMENSION3D_CODE = 3
    };

    // enum class value DECLARATIONS - they are defined later
    static std::unordered_map<SpaceDimension::Code, SpaceDimension*, EnumClassHash> dimensionByCode;
    static const SpaceDimension DIMENSION_0D;
    static const SpaceDimension DIMENSION_1D;
    static const SpaceDimension DIMENSION_2D;
    static const SpaceDimension DIMENSION_3D;

private:

    SpaceDimension(Code code, int medcouplingRelativeMeshDimension) noexcept;

    static std::unordered_map<SpaceDimension::Code, SpaceDimension*, EnumClassHash> init_map() noexcept {
        return std::unordered_map<SpaceDimension::Code, SpaceDimension*, EnumClassHash>();
    }

public:
    Code code;
    int relativeMeshDimension;
    bool operator<(const SpaceDimension &other) const noexcept;
    inline bool operator>(const SpaceDimension &other) const noexcept {
        return this->code > other.code;
    }
    bool operator==(const SpaceDimension &other) const noexcept;
    bool operator!=(const SpaceDimension &other) const noexcept;
};

class CellType final {

public:

    enum class Code {
        //codes used here are the same in Med
        POINT1_CODE = MED_POINT1,
        SEG2_CODE = MED_SEG2,
        SEG3_CODE = MED_SEG3,
        SEG4_CODE = MED_SEG4,
        SEG5_CODE = MED_SEG5,
        POLYL_CODE = MED_POLYGON,
        TRI3_CODE = MED_TRIA3,
        QUAD4_CODE = MED_QUAD4,
        POLYGON_CODE = MED_POLYGON,
        TRI6_CODE = MED_TRIA6,
        TRI7_CODE = MED_TRIA7,
        QUAD8_CODE = MED_QUAD8,
        QUAD9_CODE = MED_QUAD9,
        //QPOLYG_CODE = MED_POLYGON2,
        //
        TETRA4_CODE = MED_TETRA4,
        PYRA5_CODE = MED_PYRA5,
        PENTA6_CODE = MED_PENTA6,
        HEXA8_CODE = MED_HEXA8,
        TETRA10_CODE = MED_TETRA10,
        HEXGP12_CODE = MED_OCTA12,
        PYRA13_CODE = MED_PYRA13,
        PENTA15_CODE = MED_PENTA15,
        HEXA20_CODE = MED_HEXA20,
        HEXA27_CODE = MED_HEXA27,
        POLYHED_CODE = MED_POLYHEDRON,

        POLY3_CODE = MED_POLY3,
        POLY4_CODE = MED_POLY4,
        POLY5_CODE = MED_POLY5,
        POLY6_CODE = MED_POLY6,
        POLY7_CODE = MED_POLY7,
        POLY8_CODE = MED_POLY8,
        POLY9_CODE = MED_POLY9,
        POLY10_CODE = MED_POLY10,
        POLY11_CODE = MED_POLY11,
        POLY12_CODE = MED_POLY12,
        POLY13_CODE = MED_POLY13,
        POLY14_CODE = MED_POLY14,
        POLY15_CODE = MED_POLY15,
        POLY16_CODE = MED_POLY16,
        POLY17_CODE = MED_POLY17,
        POLY18_CODE = MED_POLY18,
        POLY19_CODE = MED_POLY19,
        POLY20_CODE = MED_POLY20,

        //type for reserved (but not yet defined) cells
        RESERVED = -1
    };

    // enum class value DECLARATIONS - they are defined later
    static std::unordered_map<CellType::Code, CellType*, EnumClassHash> typeByCode;
    static const CellType POINT1;
    static const CellType SEG2;
    static const CellType SEG3;
    static const CellType SEG4;
    static const CellType SEG5;
    //static const CellType POLYL;
    static const CellType TRI3;
    static const CellType QUAD4;
    //static const CellType POLYGON;
    static const CellType TRI6;
    static const CellType TRI7;
    static const CellType QUAD8;
    static const CellType QUAD9;
    //static const CellType QPOLYG;
    static const CellType TETRA4;
    static const CellType PYRA5;
    static const CellType PENTA6;
    static const CellType HEXA8;
    static const CellType TETRA10;
    static const CellType HEXGP12;
    static const CellType PYRA13;
    static const CellType PENTA15;
    static const CellType HEXA20;
    static const CellType HEXA27;
    //static const CellType POLYHED;
    static const CellType POLY3;
    static const CellType POLY4;
    static const CellType POLY5;
    static const CellType POLY6;
    static const CellType POLY7;
    static const CellType POLY8;
    static const CellType POLY9;
    static const CellType POLY10;
    static const CellType POLY11;
    static const CellType POLY12;
    static const CellType POLY13;
    static const CellType POLY14;
    static const CellType POLY15;
    static const CellType POLY16;
    static const CellType POLY17;
    static const CellType POLY18;
    static const CellType POLY19;
    static const CellType POLY20;

private:

    CellType(Code code, int numNodes, SpaceDimension dimension, const std::string& description) noexcept;
    friend std::ostream &operator<<(std::ostream &out, const CellType& cellType) noexcept; //output

public:
    CellType(const CellType& other) = default;
    Code code;
    unsigned int numNodes;
    SpaceDimension dimension;
    std::string description;
    bool operator==(const CellType& other) const noexcept;
    bool operator<(const CellType& other) const noexcept;
    //const CellType& operator=(const CellType& other);
    static CellType* findByCode(Code code) noexcept;
    static CellType polyType(unsigned int); /**< Return the POLY type corresponding to a cell of n nodes.*/
    std::string to_str() const noexcept;
    bool specificSize; /**< True for all Type except the POLY ones, where the number of Nodes varies */
};

class Mesh;
class NodeStorage;
class CellStorage;

class Group: public Identifiable<Group> {
public:
    enum class Type {
        NODE = 0,
        CELL = 1,
        NODEGROUP = 2,
        CELLGROUP = 3
    };
protected:
    friend Mesh;
    Group(Mesh& mesh, const std::string& name, Type, int id = NO_ORIGINAL_ID, const std::string& comment=" ") noexcept;
    Mesh& mesh;
    std::string name;
public:
    //const SpaceDimension dimension;
    Type type;
    std::string comment; ///< A comment string, usually used to retain the command which created the group.
    bool isUseful; ///< A boolean that can be used by Writer to keep or discard group.
    inline std::string getName() const noexcept {
        return this->name;
    }
    inline std::string getComment() const noexcept {
        return this->comment;
    }
    virtual std::set<int> nodePositions() const noexcept = 0;
    virtual bool empty() const noexcept = 0;
    virtual ~Group() = default;
    Group(const Group& that) = delete;
};

class Node final {
private:
    friend std::ostream &operator<<(std::ostream &out, const Node& node) noexcept;    //output
    friend Mesh;
    static int auto_node_id;
    Node(int id, double lx, double ly, double lz, int position, DOFS dofs,
            double gx, double gy, double gz, int positionCS = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID,
            int displacementCS = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, int nodePartId = 0) noexcept;
public:
    static const int AUTO_ID = INT_MIN;
    static const int UNAVAILABLE_NODE = INT_MIN;

    /** Usually, the original id of the node, from the input mesh.
     *  Can also be an automatic generated id.
     *  Unique by mesh.
     **/
    const int id;
    const int nodepartId;
    /** Position, or VEGA id, of the node in the model.
     *  Unique by mesh.
     **/
    const int position;
    const double lx; /**< X coordinate of the Node in the Local Coordinate System positionCS **/
    const double ly; /**< Y coordinate of the Node in the Local Coordinate System positionCS **/
    const double lz; /**< Z coordinate of the Node in the Local Coordinate System positionCS **/
    //bool, but Valgrind isn't happy maybe gcc 2.7.2 bug?
    DOFS dofs;
    double x; /**< X coordinate of the Node in the Global Coordinate System **/
    double y; /**< Y coordinate of the Node in the Global Coordinate System **/
    double z; /**< Z coordinate of the Node in the Global Coordinate System **/
    const int positionCS;     /**< Vega Position of the CS used to compute the position of node. **/
    const int displacementCS; /**< Vega Position of the CS used to compute displacement, loadings, etc. **/
    inline static std::string MedName(const int nodePosition) noexcept {
//        if (nodePosition == Node::UNAVAILABLE_NODE) {
//          throw std::logic_error("Node position " + std::to_string(nodePosition) + " not found.");
//        }
        return nodePosition == Node::UNAVAILABLE_NODE ? "UNAVAIL" : "N" + std::to_string(nodePosition + 1);
    }
    double square_distance(const Node& other) const noexcept;
    double distance(const Node& other) const noexcept;

    inline bool operator<(const Node& other) const noexcept {
        return this->position < other.position;
    }
    inline bool operator>(const Node& other) const noexcept {
        return this->position > other.position;
    }
    inline bool operator==(const Node& other) const noexcept {
        return this->position == other.position;
    }
};

/**
 * A mesh cell
 */
class Cell final {
private:
    friend std::ostream &operator<<(std::ostream &out, const Cell & cell) noexcept;    //output
    friend Mesh;
    int findNodeIdPosition(int node_id2) const;
    /**
     * Every face is identified by the nodes that belongs to that face
     */
    static const std::unordered_map<CellType::Code, std::vector<std::vector<int>>, EnumClassHash > FACE_BY_CELLTYPE;
    /**
     * Corner node ids
     */
    static const std::unordered_map<CellType::Code, std::vector<int>, EnumClassHash > CORNERNODEIDS_BY_CELLTYPE;
    static std::unordered_map<CellType::Code, std::vector<std::vector<int>>, EnumClassHash > init_faceByCelltype() noexcept;
    static int auto_cell_id;
    Cell(int id, const CellType &type, const std::vector<int> &nodeIds, int position, const std::vector<int> &nodePositions, bool isvirtual,
            int cspos, int elementId, int cellTypePosition, std::shared_ptr<OrientationCoordinateSystem> orientation = nullptr) noexcept;
public:
    static const int AUTO_ID = INT_MIN;
    static const int UNAVAILABLE_CELL = INT_MIN;
    int id;
    int position;
    bool hasOrientation;
    CellType type;
    std::vector<int> nodeIds;
    std::vector<int> nodePositions;
    bool isvirtual;
    int elementId;
    int cellTypePosition;
    int cspos; /**< Id of local Coordinate System **/
    std::shared_ptr<OrientationCoordinateSystem> orientation;
    std::map<int, std::vector<int>> nodeIdsByFaceNum() const;
    std::vector<int> cornerNodeIds() const;

    /**
     * @param nodeId1: grid point connected to a corner of the face.
     * Required data for solid elements only.
     * @param nodeId2: grid point connected to a corner diagonally
     * opposite to nodePosition1 on the same face of a CHEXA or CPENTA element.
     * Required data for quadrilateral faces of CHEXA and CPENTA
     * elements only. nodePosition2 must be omitted for a triangular surface on a
     * CPENTA element.
     *
     * nodeId2 : CTETRA grid point located at the corner;
     * this grid point may not reside on the face being loaded. This is
     * required data and is used for CTETRA elements only.
     *
     * <p>For faces of CTETRA elements, nodePosition1 is a corner grid point
     * that is on the face being loaded and nodePosition2 is a corner grid point
     * that is not on the face being loaded. Since a CTETRA has only
     * four corner points, this point nodePosition2 will be unique and
     * different for each of the four faces of a CTETRA element. </p>
     */

    std::vector<int> faceids_from_two_nodes(int nodeId1, int nodeId2 = Globals::UNAVAILABLE_INT) const;
    std::pair<int, int> two_nodeids_from_facenum(int faceNum) const;

    /**
     * Returns the name used in med file for this cell
     */
    inline static const std::string MedName(const int position) noexcept {
        return "M" + std::to_string(position + 1);
    }

    inline bool operator<(const Cell& other) const noexcept {
        return this->position < other.position;
    }
    inline bool operator>(const Cell& other) const noexcept {
        return this->position > other.position;
    }
    inline bool operator==(const Cell& other) const noexcept {
        return this->position == other.position;
    }
};

class CellIterator final: public std::iterator<std::input_iterator_tag, const Cell> {
    friend CellStorage;
    const CellStorage* cellStorage;
    unsigned int endPosition;
    CellType cellType;
    unsigned int position;
    bool equal(CellIterator const& other) const noexcept;
    void increment(int i);
    Cell dereference() const;
    friend Mesh;
    CellIterator(const CellStorage* cellStorage, const CellType &cellType, bool begin) noexcept;
public:
    static const bool POSITION_BEGIN = true;
    static const bool POSITION_END = false;

    virtual ~CellIterator() = default;
    bool hasNext() const noexcept;
    Cell next();
    CellIterator& operator++();
    CellIterator operator++(int);
    bool operator==(const CellIterator& rhs) const noexcept;
    bool operator!=(const CellIterator& rhs) const noexcept;
    Cell operator*() const noexcept;
};

class CellGroup;

/**
 * This class represents a container of cells or groups of cells.
 * It can split the groups into single cells.
 *
 */
class CellContainer {
    std::set<int> cellPositions;
    std::set<std::string> cellGroupNames;
    const Mesh& mesh;
public:
    CellContainer(const Mesh& mesh) noexcept;
    virtual ~CellContainer() = default;

    /**
     * Adds a cellId to the current set
     */
    void addCellPosition(int cellPosition) noexcept;
    void addCellId(int cellId) noexcept;
    // These methods should be templated but this class is based on incomplete types (Mesh) so cannot move the function implementation inside header :..(
    void addCellIds(const std::vector<int>& otherIds) noexcept;
    void addCellPositions(const std::vector<int>& otherPositions) noexcept;
    void addCellIds(const std::set<int>& otherIds) noexcept;
    void addCellPositions(const std::set<int>& otherPositions) noexcept;
    void addCellGroup(const std::string& groupName);
    void add(const Cell& cell) noexcept;
    //virtual void add(const Group& group);
    virtual void add(const CellGroup& cellGroup) noexcept;
    void add(const CellContainer& cellContainer) noexcept;
    bool containsCellPosition(int cellPosition) const noexcept;
    std::set<Cell> getCellsIncludingGroups() const noexcept;
    std::set<Cell> getCellsExcludingGroups() const noexcept;
    void removeCellPositionExcludingGroups(int cellPosition) noexcept;
    void removeAllCellsExcludingGroups() noexcept;

    std::set<int> getCellIdsIncludingGroups() const noexcept;
    std::set<int> getCellPositionsIncludingGroups() const noexcept;
    std::set<int> getCellPositionsExcludingGroups() const noexcept;

    virtual std::set<int> getNodePositionsExcludingGroups() const noexcept;
    virtual std::set<int> getNodePositionsIncludingGroups() const noexcept;

    /**
     * True if the container contains some cellGroup
     */
    virtual bool hasCellGroups() const noexcept;
    virtual bool empty() const noexcept;
    virtual void clear() noexcept;
    /**
     * True if the cellContainer contains some spare cell, not inserted
     * in any group.
     */
    bool hasCellsExcludingGroups() const noexcept;
    bool hasCellsIncludingGroups() const noexcept;
    virtual std::vector<std::shared_ptr<CellGroup>> getCellGroups() const;
};

class CellGroup final: public Group, private CellContainer {
    friend Mesh;
    CellGroup(Mesh& mesh, const std::string & name, int id = NO_ORIGINAL_ID, const std::string & comment = "") noexcept;
    CellGroup(const CellGroup& that) = delete;
public:
    void addCellId(int cellId) noexcept;
    void addCellIds(const std::vector<int>& cellIds) noexcept;
    void addCellPosition(int cellPosition) noexcept;
    bool containsCellPosition(int cellPosition) const noexcept;
    void removeCellPosition(int cellPosition) noexcept;
    std::set<Cell> getCells();
    std::set<int> cellPositions() noexcept;
    std::set<int> cellIds() noexcept;
    std::set<int> nodePositions() const noexcept override;
    bool empty() const noexcept override;
};

class NodeGroup;

/**
 * This class represents a container of nodes or groups of nodes.
 * It can split the groups into single nodes.
 *
 */
class NodeContainer : public CellContainer {
    Mesh& mesh; // because CellContainer mesh needs to be const
    std::set<int> nodePositions;
    std::set<std::string> nodeGroupNames;
public:
    NodeContainer(Mesh& mesh) noexcept;
    virtual ~NodeContainer() = default;

    /*
     * Adds a nodeId to the current set
     */
    void addNodeId(int nodeId) noexcept;

    /** Should be template but uses incomplete type Mesh, so cannot put the implementation inside header */
    void addNodeIds(const std::set<int>&) noexcept;
    void addNodeIds(const std::vector<int>&) noexcept;
    void addNodePosition(int) noexcept;
    //void addNodeGroup(const std::string& groupName);
    void add(const Node&) noexcept;
    void addNodeGroup(const std::string& groupName);
    virtual void add(const NodeGroup& nodeGroup) noexcept final;
    void add(const NodeContainer& nodeContainer) noexcept;
    bool containsNodePositionExcludingGroups(int nodePosition) const;
    void removeNodePositionExcludingGroups(int nodePosition) noexcept;
    virtual std::set<int> getNodePositionsExcludingGroups() const noexcept override final;
    virtual std::set<int> getNodePositionsIncludingGroups() const noexcept override final;
    virtual std::set<int> getNodeIdsIncludingGroups() const noexcept final;
    virtual std::set<int> getNodeIdsExcludingGroups() const noexcept final;
    virtual std::set<Node> getNodesExcludingGroups() const final;
    virtual std::vector<std::shared_ptr<NodeGroup>> getNodeGroups() const;

    // True if the container contains some nodeGroup
    virtual bool hasNodeGroups() const noexcept;
    bool empty() const noexcept override;
    void clear() noexcept override final;
    bool hasNodesExcludingGroups() const noexcept; /**< True if the container contains some spare nodes, not inserted in any group. */
    bool hasNodesIncludingGroups() const noexcept;
};


class NodeGroup final : public Group, private NodeContainer {
private:
    friend Mesh;
    NodeGroup(Mesh& mesh, const std::string& name, int groupId, const std::string& comment="    ") noexcept;
public:
    // Add a node using its numerical id. If the node hasn't been yet defined it reserve position in the model.
    void addNodeId(int nodeId) noexcept;
    void addNode(const Node& node) noexcept;
    void addNodeByPosition(int nodePosition) noexcept;
    void removeNodeByPosition(int nodePosition) noexcept;
    bool containsNodePosition(int nodePosition) const noexcept;
    std::set<int> nodePositions() const noexcept override;
    bool empty() const noexcept override;
    std::set<int> getNodeIds() const noexcept;
    std::set<Node> getNodes() const;
    NodeGroup(const NodeGroup& that) = delete;
};

} /* namespace vega */

namespace std {

template<>
struct hash<vega::SpaceDimension> {
    size_t operator()(const vega::SpaceDimension& spaceDimension) const noexcept {
        return hash<std::size_t>()(static_cast<std::size_t>(spaceDimension.code));
    }
};

template<>
struct hash<vega::CellType> {
    size_t operator()(const vega::CellType& cellType) const noexcept {
        return hash<std::size_t>()(static_cast<std::size_t>(cellType.code));
    }
};

template<>
struct hash<vega::Node> {
    size_t operator()(const vega::Node& node) const noexcept {
        return hash<std::size_t>()(static_cast<std::size_t>(node.position));
    }
};

} /* namespace std */

namespace boost {
    namespace geometry {
        namespace traits {
            // Adapt Node to Boost.Geometry

            template<> struct tag<vega::Node> {
                using type = point_tag;
            };

            template<> struct coordinate_type<vega::Node> {
                using type = double;
            };

            template<> struct coordinate_system<vega::Node> {
                // using global coordinates
                using type = cs::cartesian;
            };

            template<> struct dimension<vega::Node> : boost::mpl::int_<3> {
            };

            template<>
            struct access<vega::Node, 0> {
                static double get(vega::Node const& p) {
                    return p.x;
                }

                static void set(vega::Node& p, double const& value) {
                    p.x = value;
                }
            };

            template<>
            struct access<vega::Node, 1> {
                static double get(vega::Node const& p) {
                    return p.y;
                }

                static void set(vega::Node& p, double const& value) {
                    p.y = value;
                }
            };

            template<>
            struct access<vega::Node, 2> {
                static double get(vega::Node const& p) {
                    return p.z;
                }

                static void set(vega::Node& p, double const& value) {
                    p.z = value;
                }
            };
        }
    }
} // namespace boost::geometry::traits

#endif /* MESHCOMPONENTS_H_ */
