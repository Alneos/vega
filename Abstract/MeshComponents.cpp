/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * MeshComponents.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: devel
 */

#include "MeshComponents.h"
#include "Mesh.h"
#include "Model.h"
#include <string>
#include <initializer_list>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#if defined VDEBUG && defined __GNUC__  && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif

namespace vega {
using namespace std;
unordered_map<SpaceDimension::Code, SpaceDimension*, EnumClassHash> SpaceDimension::dimensionByCode =
		init_map();

SpaceDimension::SpaceDimension(Code code, int medcouplingRelativeMeshDimension) noexcept :
		code(code), relativeMeshDimension(medcouplingRelativeMeshDimension) {
#if defined VDEBUG && defined __GNUC__  && !defined(_WIN32)
	VALGRIND_CHECK_VALUE_IS_DEFINED(code);
	VALGRIND_CHECK_VALUE_IS_DEFINED(*this);
	VALGRIND_CHECK_VALUE_IS_DEFINED(SpaceDimension::dimensionByCode);
#endif
	SpaceDimension::dimensionByCode[code] = this;
}

const SpaceDimension SpaceDimension::DIMENSION_0D = SpaceDimension(SpaceDimension::Code::DIMENSION0D_CODE, -3);
const SpaceDimension SpaceDimension::DIMENSION_1D = SpaceDimension(SpaceDimension::Code::DIMENSION1D_CODE, -2);
const SpaceDimension SpaceDimension::DIMENSION_2D = SpaceDimension(SpaceDimension::Code::DIMENSION2D_CODE, -1);
const SpaceDimension SpaceDimension::DIMENSION_3D = SpaceDimension(SpaceDimension::Code::DIMENSION3D_CODE, 0);

bool SpaceDimension::operator<(const SpaceDimension &other) const noexcept {
	return this->code < other.code;
}

bool SpaceDimension::operator==(const SpaceDimension &other) const noexcept {
	return this->code == other.code;
}

bool SpaceDimension::operator!=(const SpaceDimension &other) const noexcept {
	return this->code != other.code;
}

unordered_map<CellType::Code, CellType*, EnumClassHash> CellType::typeByCode;

CellType::CellType(CellType::Code code, int numNodes, SpaceDimension dimension,
		const string& description) noexcept :
		code(code), numNodes(numNodes), dimension(dimension), description(description), specificSize{numNodes>0} {
	typeByCode.insert({code, this});
}

bool CellType::operator==(const CellType &other) const noexcept {
	return this->code == other.code;
}

bool CellType::operator<(const CellType& other) const noexcept {
	return this->code < other.code;
}
/*
 const CellType& CellType::operator=(const CellType other) {
 this->code = other.code;
 this->dimension = other.dimension;
 this->numNodes = other.numNodes;
 return *this;
 }
 */
ostream &operator<<(ostream &out, const CellType& cellType) noexcept {
	out << "CellType[" << cellType.description << "]";
	return out;
}

string CellType::to_str() const noexcept {
	return "CellType[" +this->description + "]";
}

const CellType CellType::POINT1 = CellType(CellType::Code::POINT1_CODE, 1, SpaceDimension::DIMENSION_0D, "POINT1");
const CellType CellType::SEG2 = CellType(CellType::Code::SEG2_CODE, 2, SpaceDimension::DIMENSION_1D, "SEG2");
const CellType CellType::SEG3 = CellType(CellType::Code::SEG3_CODE, 3, SpaceDimension::DIMENSION_1D, "SEG3");
const CellType CellType::SEG4 = CellType(CellType::Code::SEG4_CODE, 4, SpaceDimension::DIMENSION_1D, "SEG4");
const CellType CellType::SEG5 = CellType(CellType::Code::SEG5_CODE, 5, SpaceDimension::DIMENSION_1D, "SEG5");
//const CellType CellType::POLYL = CellType(CellType::Code::POLYL_CODE, -1, SpaceDimension::DIMENSION_1D, "POLYL");
const CellType CellType::TRI3 = CellType(CellType::Code::TRI3_CODE, 3, SpaceDimension::DIMENSION_2D, "TRI3");
const CellType CellType::QUAD4 = CellType(CellType::Code::QUAD4_CODE, 4, SpaceDimension::DIMENSION_2D, "QUAD4");
//const CellType CellType::POLYGON = CellType(CellType::Code::POLYGON_CODE, -1, SpaceDimension::DIMENSION_2D,
//		"POLYGON");
const CellType CellType::TRI6 = CellType(CellType::Code::TRI6_CODE, 6, SpaceDimension::DIMENSION_2D, "TRI6");
const CellType CellType::TRI7 = CellType(CellType::Code::TRI7_CODE, 7, SpaceDimension::DIMENSION_2D, "TRI7");
const CellType CellType::QUAD8 = CellType(CellType::Code::QUAD8_CODE, 8, SpaceDimension::DIMENSION_2D, "QUAD8");
const CellType CellType::QUAD9 = CellType(CellType::Code::QUAD9_CODE, 9, SpaceDimension::DIMENSION_2D, "QUAD9");
//const CellType CellType::QPOLYG = CellType(CellType::Code::QPOLYG_CODE, -1,
//		SpaceDimension::DIMENSION_2D, "QPOLYG");
const CellType CellType::TETRA4 = CellType(CellType::Code::TETRA4_CODE, 4, SpaceDimension::DIMENSION_3D, "TETRA4");
const CellType CellType::PYRA5 = CellType(CellType::Code::PYRA5_CODE, 5, SpaceDimension::DIMENSION_3D, "PYRA5");
const CellType CellType::PENTA6 = CellType(CellType::Code::PENTA6_CODE, 6, SpaceDimension::DIMENSION_3D, "PENTA6");
const CellType CellType::HEXA8 = CellType(CellType::Code::HEXA8_CODE, 8, SpaceDimension::DIMENSION_3D, "HEXA8");
const CellType CellType::TETRA10 = CellType(CellType::Code::TETRA10_CODE, 10, SpaceDimension::DIMENSION_3D,
		"TETRA10");
const CellType CellType::HEXGP12 = CellType(CellType::Code::HEXGP12_CODE, 12, SpaceDimension::DIMENSION_3D,
		"HEXGP12");
const CellType CellType::PYRA13 = CellType(CellType::Code::PYRA13_CODE, 13, SpaceDimension::DIMENSION_3D, "PYRA13");
const CellType CellType::PENTA15 = CellType(CellType::Code::PENTA15_CODE, 15, SpaceDimension::DIMENSION_3D,
		"PENTA15");
const CellType CellType::HEXA20 = CellType(CellType::Code::HEXA20_CODE, 20, SpaceDimension::DIMENSION_3D, "HEXA20");
const CellType CellType::HEXA27 = CellType(CellType::Code::HEXA27_CODE, 27, SpaceDimension::DIMENSION_3D,
		"DIMENSION_3D");
//const CellType CellType::POLYHED = CellType(CellType::Code::POLYHED_CODE, -1, SpaceDimension::DIMENSION_3D,
//		"POLYHED");

//TODO: Ugly fix because POLYHED and co are not working yet. We need an element with undefined number of nodes. :/
const CellType CellType::POLY3  = CellType(CellType::Code::POLY3_CODE, 3, SpaceDimension::DIMENSION_3D, "POLY3");
const CellType CellType::POLY4  = CellType(CellType::Code::POLY4_CODE, 4, SpaceDimension::DIMENSION_3D, "POLY4");
const CellType CellType::POLY5  = CellType(CellType::Code::POLY5_CODE, 5, SpaceDimension::DIMENSION_3D, "POLY5");
const CellType CellType::POLY6  = CellType(CellType::Code::POLY6_CODE, 6, SpaceDimension::DIMENSION_3D, "POLY6");
const CellType CellType::POLY7  = CellType(CellType::Code::POLY7_CODE, 7, SpaceDimension::DIMENSION_3D, "POLY7");
const CellType CellType::POLY8  = CellType(CellType::Code::POLY8_CODE, 8, SpaceDimension::DIMENSION_3D, "POLY8");
const CellType CellType::POLY9  = CellType(CellType::Code::POLY9_CODE, 9, SpaceDimension::DIMENSION_3D, "POLY9");
const CellType CellType::POLY10 = CellType(CellType::Code::POLY10_CODE, 10, SpaceDimension::DIMENSION_3D, "POLY10");
const CellType CellType::POLY11 = CellType(CellType::Code::POLY11_CODE, 11, SpaceDimension::DIMENSION_3D, "POLY11");
const CellType CellType::POLY12 = CellType(CellType::Code::POLY12_CODE, 12, SpaceDimension::DIMENSION_3D, "POLY12");
const CellType CellType::POLY13 = CellType(CellType::Code::POLY13_CODE, 13, SpaceDimension::DIMENSION_3D, "POLY13");
const CellType CellType::POLY14 = CellType(CellType::Code::POLY14_CODE, 14, SpaceDimension::DIMENSION_3D, "POLY14");
const CellType CellType::POLY15 = CellType(CellType::Code::POLY15_CODE, 15, SpaceDimension::DIMENSION_3D, "POLY15");
const CellType CellType::POLY16 = CellType(CellType::Code::POLY16_CODE, 16, SpaceDimension::DIMENSION_3D, "POLY16");
const CellType CellType::POLY17 = CellType(CellType::Code::POLY17_CODE, 17, SpaceDimension::DIMENSION_3D, "POLY17");
const CellType CellType::POLY18 = CellType(CellType::Code::POLY18_CODE, 18, SpaceDimension::DIMENSION_3D, "POLY18");
const CellType CellType::POLY19 = CellType(CellType::Code::POLY19_CODE, 19, SpaceDimension::DIMENSION_3D, "POLY19");
const CellType CellType::POLY20 = CellType(CellType::Code::POLY20_CODE, 20, SpaceDimension::DIMENSION_3D, "POLY20");


CellType* CellType::findByCode(CellType::Code code) noexcept {
	return CellType::typeByCode[code];
}

CellType CellType::polyType(unsigned int nbNodes) {
    //TODO: Add a version with "variable sized" cells.
    switch (nbNodes){
    case 1:{
        return CellType::POINT1;
        break;
    }
    case 2:{
        return CellType::SEG2;
        break;
    }
    case 3:{
        return CellType::POLY3;
        break;
    }
    case 4:{
        return CellType::POLY4;
        break;
    }
    case 5:{
        return CellType::POLY5;
        break;
    }
    case 6:{
        return CellType::POLY6;
        break;
    }
    case 7:{
        return CellType::POLY7;
        break;
    }
    case 8:{
        return CellType::POLY8;
        break;
    }
    case 9:{
        return CellType::POLY9;
        break;
    }
    case 10:{
        return CellType::POLY10;
        break;
    }
    case 11:{
        return CellType::POLY11;
        break;
    }
    case 12:{
        return CellType::POLY12;
        break;
    }
    case 13:{
        return CellType::POLY13;
        break;
    }
    case 14:{
        return CellType::POLY14;
        break;
    }
    case 15:{
        return CellType::POLY15;
        break;
    }
    case 16:{
        return CellType::POLY16;
        break;
    }
    case 17:{
        return CellType::POLY17;
        break;
    }
    case 18:{
        return CellType::POLY18;
        break;
    }
    case 19:{
        return CellType::POLY19;
        break;
    }
    case 20:{
        return CellType::POLY20;
        break;
    }
    default:{
        //TODO: Don't work for now, because elements can't be of a variable size, for now :/
        //cellType = CellType::POLYHED;
        throw logic_error("Element size exceed the maximum size : 20.");
    }
    }
    return CellType::POLY20;
}

Group::Group(Mesh& mesh, const string& name, Type type, int _id, const string& comment) noexcept :
                Identifiable(_id), mesh(mesh), name(name), type(type), comment(comment), isUseful(false) {
}

/*******************
 * NodeGroup
 */
NodeGroup::NodeGroup(Mesh& mesh, const string& name, int groupId, const string& comment) noexcept :
		Group(mesh, name, Group::Type::NODEGROUP, groupId, comment), NodeContainer(mesh) {
}

void NodeGroup::addNode(const Node& node) noexcept {
	NodeContainer::add(node);
}

void NodeGroup::addNodeByPosition(const pos_t nodePosition) noexcept {
	NodeContainer::addNodePosition(nodePosition);
}

void NodeGroup::removeNodeByPosition(const pos_t nodePosition) noexcept {
	NodeContainer::removeNodePositionExcludingGroups(nodePosition);
}

bool NodeGroup::containsNodePosition(const pos_t nodePosition) const noexcept {
    return NodeContainer::containsNodePositionExcludingGroups(nodePosition);
}

std::set<pos_t> NodeGroup::nodePositions() const noexcept {
	return NodeContainer::getNodePositionsExcludingGroups();
}

set<int> NodeGroup::getNodeIds() const noexcept {
	return NodeContainer::getNodeIdsExcludingGroups();
}

set<Node> NodeGroup::getNodes() const {
	return NodeContainer::getNodesExcludingGroups();
}

CellGroup::CellGroup(Mesh& mesh, const string& name, int groupId, const string& comment ) noexcept :
		Group(mesh, name, Group::Type::CELLGROUP, groupId, comment), CellContainer(mesh) {
}

set<Cell> CellGroup::getCells() {
	return CellContainer::getCellsIncludingGroups();
}

set<pos_t> CellGroup::cellPositions() noexcept {
	return CellContainer::getCellPositionsIncludingGroups();
}

set<int> CellGroup::cellIds() noexcept {
	return CellContainer::getCellIdsIncludingGroups();
}

set<pos_t> CellGroup::nodePositions() const noexcept {
	return CellContainer::getNodePositionsIncludingGroups();
}

///////////////////////////////////////////////////////////////////////////////
/*                  Node                                                     */
///////////////////////////////////////////////////////////////////////////////
int Node::auto_node_id = 9999999;

Node::Node(int id, double lx, double ly, double lz, pos_t position1, DOFS inElement1, double gx, double gy, double gz, pos_t _positionCS, pos_t _displacementCS, int _nodepartId) noexcept :
		id(id), nodepartId(_nodepartId), position(position1), lx(lx), ly(ly), lz(lz), dofs(inElement1), x(gx), y(gy), z(gz),
		positionCS(_positionCS), displacementCS(_displacementCS) {
}

double Node::distance(const Node& other) const noexcept {
    return boost::geometry::distance(*this, other);
    //return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2));
}

double Node::square_distance(const Node& other) const noexcept {
    return boost::geometry::comparable_distance(*this, other);
    //return pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2);
}

ostream &operator<<(ostream &out, const Node& node) noexcept {
	out << "Node[id:" << node.id << ",x:" << node.x << ",y:" << node.y << ",z:" << node.z << " pos:"
			<< node.position << "]";
	return out;
}

///////////////////////////////////////////////////////////////////////////////
/*                             Cells                                         */
///////////////////////////////////////////////////////////////////////////////
int Cell::auto_cell_id = 9999999;

const unordered_map<CellType::Code, vector<vector<int>>, EnumClassHash > Cell::FACE_BY_CELLTYPE =
		init_faceByCelltype();

// http://www.code-aster.org/outils/med/html/connectivites.html
// https://hammi.extra.cea.fr/static/MED/web_med/logiciels/medV2.1.4_doc_html/html/modele_de_donnees.html
unordered_map<CellType::Code, vector<vector<int>>, EnumClassHash > Cell::init_faceByCelltype() noexcept {
	vector<vector<int> > hexa8list = { //
        {1,2,3,4}, //
        {5,8,7,6}, //
        {1,5,6,2}, //
        {2,6,7,3}, //
        {3,7,8,4}, //
        {1,4,8,5} //
    };
	vector<vector<int> > tetra4list = { //
        {1,2,3}, //
        {1,4,2}, //
        {1,3,4}, //
        {2,4,3} //
    };
    vector<vector<int> > pyra5list = { //
        {1,2,3,4}, //
        {1,5,2}, //
        {1,4,5}, //
        {4,3,5}, //
        {2,5,3}, //
    };
	vector<vector<int> > penta6list = { //
        {1,2,3}, //
        {4,6,5}, //
        {1,4,5,2}, //
        {1,3,6,4}, //
        {2,5,6,3} //
    };

    // Nodal order is used to create 2D surface with sorting normal (example: TRIA6 over TETRA10 face)
    vector<vector<int> > tetra10list = { //
        {1,2,3,5,6,7}, //
        {1,4,2,8,9,5}, //
        {1,3,4,7,10,8}, //
        {2,4,3,9,10,6} //
    };
    vector<vector<int> > pyra13list = { //
        {1,2,3,4,6,7,8,9}, //
        {1,5,2,10,11,6}, //
        {1,4,5,9,13,10}, //
        {4,3,5,8,12,13}, //
        {2,5,3,11,12,7}, //
    };
    vector<vector<int> > penta15list = { //
        {1,2,3,7,8,9}, //
        {4,6,5,12,11,10}, //
        {1,4,5,2,13,10,14,7}, //
        {1,3,6,4,9,15,12,13}, //
        {2,5,6,3,14,11,15,8} //
    };
    vector<vector<int> > hexa20list = { //
        {1,2,3,4,9,10,11,12}, //
        {5,8,7,6,16,15,14,16}, //
        {1,2,6,5,9,18,13,17}, //
        {2,6,7,3,18,14,19,10}, //
        {3,7,8,4,19,15,20,11}, //
        {1,4,8,5,12,20,16,17} //
    };

	unordered_map<CellType::Code, vector<vector<int>>, EnumClassHash > result = {
        {CellType::HEXA8.code, hexa8list}, //
        {CellType::TETRA4.code, tetra4list}, //
        {CellType::PYRA5.code, pyra5list}, //
        {CellType::PENTA6.code, penta6list}, //
        {CellType::TETRA10.code, tetra10list}, //
        {CellType::PYRA13.code, pyra13list}, //
        {CellType::PENTA15.code, penta15list}, //
        {CellType::HEXA20.code, hexa20list}, //
    };
	return result;
}

// http://www.code-aster.org/outils/med/html/connectivites.html
// https://hammi.extra.cea.fr/static/MED/web_med/logiciels/medV2.1.4_doc_html/html/modele_de_donnees.html
const unordered_map<CellType::Code, vector<int>, EnumClassHash > Cell::CORNERNODEIDS_BY_CELLTYPE = {
    {CellType::SEG2.code, {0, 1}},
    {CellType::SEG3.code, {0, 1}},
    {CellType::TRI3.code, {0, 1, 2}},
    {CellType::TRI6.code, {0, 1, 2}},
    {CellType::QUAD4.code, {0, 1, 2, 3}},
    {CellType::QUAD8.code, {0, 1, 2, 3}},
    {CellType::TETRA4.code, {0, 1, 2, 3}},
    {CellType::TETRA10.code, {0, 1, 2, 3}},
    {CellType::PYRA5.code, {0, 1, 2, 3, 4}},
    {CellType::PYRA13.code, {0, 1, 2, 3, 4}},
    {CellType::PENTA6.code, {0, 1, 2, 3, 4, 5}},
    {CellType::PENTA15.code, {0, 1, 2, 3, 4, 5}},
    {CellType::HEXA8.code, {0, 1, 2, 3, 4, 5, 6, 7}},
    {CellType::HEXA20.code, {0, 1, 2, 3, 4, 5, 6, 7}},
};

Cell::Cell(const int id, const CellType &type, const std::vector<int> &nodeIds, const pos_t position,
		const std::vector<pos_t>& nodePositions, const bool isvirtual,
		const pos_t cspos, const int element_id, const pos_t cellTypePosition,
		const std::shared_ptr<OrientationCoordinateSystem> orientation, const double offset) noexcept :
		id(id), position(position), hasOrientation(cspos!=CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), type(type),
				nodeIds(nodeIds), nodePositions(nodePositions), isvirtual(isvirtual),
				elementId(element_id), cellTypePosition(cellTypePosition), cspos(cspos), orientation(orientation),
                offset(offset) {
}

pos_t Cell::findNodeIdPosition(int node_id2) const {
	//|| cellType == CellType::TETRA10
	pos_t node2connectivityPos = 0;
	for (; node2connectivityPos < nodeIds.size(); node2connectivityPos++) {
		if (nodeIds[node2connectivityPos] == node_id2) {
			break;
		}
	}
	if (node2connectivityPos == nodeIds.size()) {
		throw logic_error("node id " + to_string(node_id2) + " not in cell");
	}
	return static_cast<int>(node2connectivityPos);
}

map<int, vector<int>> Cell::nodeIdsByFaceNum() const {
    const auto& it = FACE_BY_CELLTYPE.find(type.code);
    if (it == FACE_BY_CELLTYPE.end())
        throw logic_error("Missing FACE_BY_CELLTYPE configuration for cell type :" + type.description);
    const auto& nodeConnectivityPosByFace = it->second;
    int faceNum = 1;
    map<int, vector<int>> result;
    for (const auto& nodeConnectivityPos : nodeConnectivityPosByFace) {
        vector<int> faceConnectivity;
        faceConnectivity.reserve(nodeConnectivityPos.size());
        for (const auto nodenum : nodeConnectivityPos) {
            faceConnectivity.push_back(nodeIds[nodenum - 1]);
        }
        result[faceNum] = faceConnectivity;
        faceNum++;
    }
    return result;
}

vector<int> Cell::faceids_from_two_nodes(int nodeId1, int nodeId2) const {
	const pos_t node1connectivityPos = findNodeIdPosition(nodeId1);
    pos_t node2connectivityPos = Globals::UNAVAILABLE_POS;
	if (nodeId2 != Globals::UNAVAILABLE_INT) {
        node2connectivityPos = findNodeIdPosition(nodeId2);
	}

	if (type.dimension == SpaceDimension::DIMENSION_2D) {
		return {nodeIds.begin(), nodeIds.end()};
	}
	const auto& faceids = FACE_BY_CELLTYPE.find(type.code)->second;
	vector<pos_t> nodePositions;
	switch(type.code) {
    case CellType::Code::TETRA4_CODE:
    case CellType::Code::TETRA10_CODE: {
        //node2 is on the opposite face
        if (node2connectivityPos == Globals::UNAVAILABLE_POS) {
            throw logic_error("Need two nodes to find a face on " + type.to_str() + " element type");
        }
		for (const auto& faceid : faceids) {
			//0 based
			if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) == faceid.end()) {
				nodePositions.assign(faceid.begin(), faceid.end());
			}
		}
        break;
    }
    case CellType::Code::HEXA8_CODE:
    case CellType::Code::HEXA20_CODE: {
        if (node2connectivityPos == Globals::UNAVAILABLE_POS) {
            throw logic_error("Need two nodes to find a face on " + type.to_str() + " element type");
        }
		for (const auto& faceid : faceids) {
			//0 based
			if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) != faceid.end()
					&& find(faceid.begin(), faceid.end(), node1connectivityPos + 1)
							!= faceid.end()) {
				nodePositions.assign(faceid.begin(), faceid.end());
				break;
			}
		}
        break;
    }
    case CellType::Code::PYRA5_CODE:
    case CellType::Code::PYRA13_CODE:
    case CellType::Code::PENTA6_CODE:
    case CellType::Code::PENTA15_CODE: {

        for (const auto& faceid : faceids) {
            if (find(faceid.begin(), faceid.end(), node1connectivityPos + 1) == faceid.end()) {
                continue;
            }
			if (nodeId2 == Globals::UNAVAILABLE_INT) {
                // nodePosition2 must be omitted for a **triangular** surface on a
                // CPENTA element. 6 nodes for quadratic
                if (faceid.size() == 3 or faceid.size() == 6) {
                    nodePositions.assign(faceid.begin(), faceid.end());
                    break;
                }
			} else {
                if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) != faceid.end()) {
                    nodePositions.assign(faceid.begin(), faceid.end());
                    break;
                }
			}
		}
        break;
    }
    default: {
		throw logic_error("FaceidfromtwoNodes not implemented for " + type.to_str() + " element type");
    }
	}

	vector<int> faceConnectivity;
	faceConnectivity.reserve(nodePositions.size());
	for (const auto nodenum : nodePositions) {
		faceConnectivity.push_back(nodeIds[nodenum - 1]);
	}
	return faceConnectivity;
}

vector<int> Cell::cornerNodeIds() const {
    const auto& it = CORNERNODEIDS_BY_CELLTYPE.find(type.code);
    if (it == CORNERNODEIDS_BY_CELLTYPE.end())
        throw logic_error("Missing CORNERNODEIDS_BY_CELLTYPE configuration for cell type :" + type.description);
    const auto& nodeConnectivityPositions = it->second;
    vector<int> result;
    result.reserve(nodeConnectivityPositions.size());
    for (int nodenum : nodeConnectivityPositions) {
        result.push_back(nodeIds[nodenum]);
    }
    return result;
}

pair<int, int> Cell::two_nodeids_from_facenum(int faceNum) const {
    auto nodeIds_By_FaceNum = nodeIdsByFaceNum();
    const auto& faceNodeIds = nodeIds_By_FaceNum[faceNum];
	switch(type.code) {
    case CellType::Code::TETRA4_CODE:
    case CellType::Code::TETRA10_CODE: {
        //node2 is on the opposite face
        const set<int>& cellOrderedNodeIds{nodeIds.begin(), nodeIds.end()};
        const set<int>& faceOrderedNodeIds{faceNodeIds.begin(), faceNodeIds.end()};
        vector<int> v_difference;

        set_difference(cellOrderedNodeIds.begin(), cellOrderedNodeIds.end(),
                              faceOrderedNodeIds.begin(), faceOrderedNodeIds.end(),
                              back_inserter(v_difference));
        return {faceNodeIds[0], v_difference[0]};
    }
    case CellType::Code::PYRA5_CODE:
    case CellType::Code::PENTA6_CODE:
    case CellType::Code::PYRA13_CODE:
    case CellType::Code::PENTA15_CODE: {
        if (faceNodeIds.size() == 4 or faceNodeIds.size() == 8) {
            return {faceNodeIds[0], faceNodeIds[1]};
        } else {
            int nodeId2 = Globals::UNAVAILABLE_INT;
            return {faceNodeIds[0], nodeId2};
        }
    }
    case CellType::Code::HEXA8_CODE:
    case CellType::Code::HEXA20_CODE: {
        return {faceNodeIds[0], faceNodeIds[2]};
    }
    default: {
		throw logic_error("two_nodeids_from_facenum not implemented for " + type.to_str() + " element type");
    }
	}
}

ostream &operator<<(ostream &out, const Cell& cell) noexcept {
	out << "Cell[id:" << cell.id;
	out << ",type:" << static_cast<int>(cell.type.code);
	out << ",nodeIds:[";
	for (const int nodeId : cell.nodeIds) {
		cout << "," << nodeId;
	}
	out << "]";
	return out;
}

CellIterator::CellIterator(const CellStorage* cellStorage, const CellType &cellType, bool begin) noexcept :
		cellStorage(cellStorage), endPosition(static_cast<pos_t>(cellStorage->mesh.countCells(cellType))), cellType(cellType), position(
				begin ? 0 : endPosition) {
}

Cell CellIterator::next() {
	Cell result = dereference();
	increment(1);
	return result;
}

void CellIterator::increment(pos_t i) {
	position += i;
	//cout << "currentPos " << position << "end " << endPosition << endl;
	if (position > endPosition) {
		throw out_of_range(
				"Iterator on cells in position " + to_string(position)
						+ " after end.");
	}
}

CellIterator& CellIterator::operator ++() {
	increment(1);
	return *this;
}

CellIterator CellIterator::operator ++(int) {
	increment(1); // argument must be ignored https://en.cppreference.com/w/cpp/language/operators#Increment_and_decrement
	return *this;
}

bool CellIterator::operator ==(const CellIterator& other) const noexcept {
	//cout << "this p " << this->position << "other p:" << other.position << endl;
	return (this->position == other.position) && (cellType.code == other.cellType.code)
			&& (this->endPosition == other.endPosition);
}

bool CellIterator::operator !=(const CellIterator& rhs) const noexcept {
	return !(*this == rhs);
}

Cell CellIterator::dereference() const {
	return cellStorage->mesh.findCell(cellStorage->mesh.cellPositionsByType.find(cellType)->second[position]);
}

Cell CellIterator::operator *() const noexcept {
	return dereference();
}

bool CellIterator::hasNext() const noexcept {
	return (position < endPosition);
}

bool CellIterator::equal(CellIterator const &other) const noexcept {
	//too slow! (this->cellIds->isEqual(*other.cellIds)
	return (this->position == other.position) && (cellType.code == other.cellType.code)
			&& (this->endPosition == other.endPosition);
}

/*******************
 * Cell container;
 */

CellContainer::CellContainer(const Mesh& mesh) noexcept :
		mesh(mesh) {
}

void CellContainer::addCellPosition(pos_t cellPosition) noexcept {
	cellPositions.insert(cellPosition);
}

void CellContainer::addCellId(int cellId) noexcept {
	cellPositions.insert(mesh.findCellPosition(cellId));
}

void CellContainer::addCellIds(const vector<int>& otherIds) noexcept {
	transform(otherIds.begin(),
              otherIds.end(),
              std::inserter(cellPositions, cellPositions.begin()),
              [&](const int cellId) {
                  return mesh.findCellPosition(cellId);
              });
}

void CellContainer::addCellIds(const set<int>& otherIds) noexcept {
	transform(otherIds.begin(),
              otherIds.end(),
              std::inserter(cellPositions, cellPositions.begin()),
              [&](const int cellId) {
                  return mesh.findCellPosition(cellId);
              });
}

void CellContainer::addCellIds(const list<int>& otherIds) noexcept {
	transform(otherIds.begin(),
              otherIds.end(),
              std::inserter(cellPositions, cellPositions.begin()),
              [&](const int cellId) {
                  return mesh.findCellPosition(cellId);
              });
}


void CellContainer::addCellPositions(const vector<pos_t>& otherPositions) noexcept {
    cellPositions.insert(otherPositions.begin(), otherPositions.end());
}

void CellContainer::addCellPositions(const set<pos_t>& otherPositions) noexcept {
    cellPositions.insert(otherPositions.begin(), otherPositions.end());
}

void CellContainer::addCellPositions(const list<pos_t>& otherPositions) noexcept {
    cellPositions.insert(otherPositions.begin(), otherPositions.end());
}

void CellContainer::addCellGroup(const string& groupName) {
	const auto& group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: " + groupName + "not found.");
	}
	this->cellGroupNames.insert(groupName);
}

void CellContainer::add(const Cell& cell) noexcept {
	cellPositions.insert(cell.position);
}

void CellContainer::add(const CellGroup& cellGroup) noexcept {
	this->cellGroupNames.insert(cellGroup.getName());
}

void CellContainer::add(const CellContainer& cellContainer) noexcept {
    const auto& otherCellPositions = cellContainer.getCellPositionsExcludingGroups();
	if (not otherCellPositions.empty()) {
		cellPositions.insert(otherCellPositions.begin(), otherCellPositions.end());
	}

	if (not cellContainer.cellGroupNames.empty()) {
		this->cellGroupNames.insert(cellContainer.cellGroupNames.begin(), cellContainer.cellGroupNames.end());
	}
}

bool CellContainer::containsCellPosition(pos_t cellPosition) const noexcept {
	return cellPositions.find(cellPosition) != cellPositions.end();
}

void CellContainer::removeCellPositionExcludingGroups(pos_t cellPosition) noexcept {
	cellPositions.erase(cellPosition);
}


set<Cell> CellContainer::getCellsExcludingGroups() const noexcept {
	set<Cell> cells;
	transform(cellPositions.begin(),
              cellPositions.end(),
              std::inserter(cells, cells.begin()),
              [&](const pos_t cellPosition) {
                  return mesh.findCell(cellPosition);
              });
	return cells;
}

set<Cell> CellContainer::getCellsIncludingGroups() const noexcept {
	set<Cell>&& cells = getCellsExcludingGroups();
    for (const auto& groupName : cellGroupNames) {
        const auto& group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& cellsInGroup = group->getCells();
            cells.insert(cellsInGroup.begin(), cellsInGroup.end());
        }
    }
	return cells;
}

void CellContainer::removeAllCellsExcludingGroups() noexcept {
    cellPositions.clear();
}

set<int> CellContainer::getCellIdsExcludingGroups() const noexcept {
	set<int> result;
	transform(cellPositions.begin(),
              cellPositions.end(),
              std::inserter(result, result.begin()),
              [&](const pos_t cellPosition) {
                  return mesh.findCellId(cellPosition);
              });
	return result;
}

set<int> CellContainer::getCellIdsIncludingGroups() const noexcept {
	auto&& result = getCellIdsExcludingGroups();
    for (const auto& groupName : cellGroupNames) {
        const auto& group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& groupCellIds = group->cellIds();
            result.insert(groupCellIds.begin(), groupCellIds.end());
        }
    }
	return result;
}

set<pos_t> CellContainer::getCellPositionsIncludingGroups() const noexcept {
	set<pos_t> result(cellPositions.begin(), cellPositions.end());
    for (const auto& groupName : cellGroupNames) {
        const auto& group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& groupCellPositions = group->cellPositions();
            result.insert(groupCellPositions.begin(), groupCellPositions.end());
        }
    }
	return result;
}

set<pos_t> CellContainer::getCellPositionsExcludingGroups() const noexcept {
	return cellPositions;
}

set<pos_t> CellContainer::getNodePositionsIncludingGroups() const noexcept {
	set<pos_t> result;
	for (const auto& cell : getCellsIncludingGroups()) {
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

set<pos_t> CellContainer::getNodePositionsExcludingGroups() const noexcept {
	set<pos_t> result;
	for (const auto& cell : getCellsExcludingGroups()) {
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

bool CellContainer::empty() const noexcept {
	return not hasCellsIncludingGroups();
}

void CellContainer::clear() noexcept {
	cellGroupNames.clear();
	cellPositions.clear();
}

bool CellContainer::hasCellsExcludingGroups() const noexcept {
	return not cellPositions.empty();
}

bool CellContainer::hasCellsIncludingGroups() const noexcept {
	if (not cellPositions.empty())
        return true;
    for (const auto& groupName : cellGroupNames) {
        const auto& group = static_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (not group->empty())
            return true;
    }
    return false;
}

vector<shared_ptr<CellGroup>> CellContainer::getCellGroups() const {
	vector<shared_ptr<CellGroup>> cellGroups;
	cellGroups.reserve(cellGroupNames.size());
	for (const auto& groupName : cellGroupNames) {
		const auto& group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		cellGroups.push_back(group);
	}
	return cellGroups;
}

bool CellContainer::hasCellGroups() const noexcept {
	return not cellGroupNames.empty();
}

string CellContainer::to_str() const {
    ostringstream out;
	out << "CellContainer[";
	if (not cellPositions.empty()) {
        out << "cell ids:[" << cellPositions << "],";
	}
	if (not cellGroupNames.empty()) {
        out << "cell groups:[" << cellGroupNames << "],";
	}
	out << "]";
	return out.str();
}

void CellGroup::removeCellPosition(pos_t cellPosition) noexcept {
    CellContainer::removeCellPositionExcludingGroups(cellPosition);
}

/*******************
 * Node container;
 */

NodeContainer::NodeContainer(Mesh& mesh) noexcept :
		CellContainer(mesh), mesh(mesh) {
}

void NodeContainer::addNodeId(int nodeId) noexcept {
	nodePositions.insert(mesh.findOrReserveNode(nodeId));
}

void NodeContainer::addNodeIds(const set<int>& range) noexcept {
    for(const auto nodeId : range) {
        nodePositions.insert(mesh.findOrReserveNode(nodeId));
    }
}

void NodeContainer::addNodeIds(const vector<int>& range) noexcept {
    for(const auto nodeId : range) {
        nodePositions.insert(mesh.findOrReserveNode(nodeId));
    }
}

void NodeContainer::addNodeIds(const list<int>& range) noexcept {
    for(const auto nodeId : range) {
        nodePositions.insert(mesh.findOrReserveNode(nodeId));
    }
}

void NodeContainer::addNodePositions(const set<pos_t>& range) noexcept {
    nodePositions.insert(range.begin(), range.end());
}

void NodeContainer::addNodePositions(const vector<pos_t>& range) noexcept {
    nodePositions.insert(range.begin(), range.end());
}

void NodeContainer::addNodePositions(const list<pos_t>& range) noexcept {
    nodePositions.insert(range.begin(), range.end());
}

void NodeContainer::addNodePosition(const pos_t nodePosition) noexcept {
	nodePositions.insert(nodePosition);
}

void NodeContainer::addNodeGroup(const string& groupName) {
	const auto& group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: [" + groupName + "] not found.");
	}
	nodeGroupNames.insert(groupName);
}

void NodeContainer::removeNodePositionExcludingGroups(const pos_t nodePosition) noexcept {
	nodePositions.erase(nodePositions.find(nodePosition));
}

void NodeContainer::add(const Node& node) noexcept {
	nodePositions.insert(node.position);
}

void NodeContainer::add(const NodeGroup& nodeGroup) noexcept {
	nodeGroupNames.insert(nodeGroup.getName());
}

void NodeContainer::add(const NodeContainer& nodeContainer) noexcept {
    const auto& otherNodePositions = nodeContainer.getNodePositionsExcludingGroups();
	if (not otherNodePositions.empty()) {
		nodePositions.insert(otherNodePositions.begin(), otherNodePositions.end());
	}

	if (not nodeContainer.nodeGroupNames.empty()) {
		nodeGroupNames.insert(nodeContainer.nodeGroupNames.begin(), nodeContainer.nodeGroupNames.end());
	}
}

set<pos_t> NodeContainer::getNodePositionsExcludingGroups() const noexcept {
    if (CellContainer::empty()) {
        return nodePositions;
    } else {
        set<pos_t> result(nodePositions);
        const auto& cellNodePositions = CellContainer::getNodePositionsExcludingGroups();
        result.insert(cellNodePositions.begin(), cellNodePositions.end());
        return result;
    }
}

set<pos_t> NodeContainer::getNodePositionsIncludingGroups() const noexcept {
    set<pos_t> result(nodePositions);
	for (const auto& groupName : nodeGroupNames) {
		const auto& group = static_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
		const auto& groupNodePositions = group->nodePositions();
		result.insert(groupNodePositions.begin(), groupNodePositions.end());
	}
    const auto& cellNodePositions = CellContainer::getNodePositionsIncludingGroups();
    result.insert(cellNodePositions.begin(), cellNodePositions.end());
	return result;
}

set<int> NodeContainer::getNodeIdsIncludingGroups() const noexcept {
    set<int> result;
	for (const pos_t nodePosition : getNodePositionsIncludingGroups()) {
	    result.insert(mesh.findNodeId(nodePosition));
	}
	return result;
}

set<int> NodeContainer::getNodeIdsExcludingGroups() const noexcept {
    set<int> result;
	for (const pos_t nodePosition : getNodePositionsExcludingGroups()) {
	    result.insert(mesh.findNodeId(nodePosition));
	}
	return result;
}

set<Node> NodeContainer::getNodesExcludingGroups() const {
    set<Node> result;
	for (const pos_t nodePosition : getNodePositionsExcludingGroups()) {
	    result.insert(mesh.findNode(nodePosition));
	}
	return result;
}

bool NodeContainer::containsNodePositionExcludingGroups(const pos_t nodePosition) const {
	return nodePositions.find(nodePosition) != nodePositions.end();
}

bool NodeContainer::empty() const noexcept {
	return nodeGroupNames.empty() and nodePositions.empty()  and CellContainer::empty() and (not hasNodesIncludingGroups());
}

void NodeContainer::clear() noexcept {
	nodeGroupNames.clear();
	nodePositions.clear();
	CellContainer::clear();
}

bool NodeContainer::hasNodesExcludingGroups() const noexcept {
	return not nodePositions.empty();
}

bool NodeContainer::hasNodesIncludingGroups() const noexcept {
	if (not nodePositions.empty())
        return true;
	if (not CellContainer::empty())
        return true;
	for (string groupName : nodeGroupNames) {
        if (not mesh.findGroup(groupName)->empty())
            return true;
    }
    return false;
}

bool NodeContainer::hasNodeGroups() const noexcept {
	return not nodeGroupNames.empty();
}

vector<shared_ptr<NodeGroup>> NodeContainer::getNodeGroups() const {
	vector<shared_ptr<NodeGroup>> nodeGroups;
	for (const auto& groupName : nodeGroupNames) {
		const auto& group = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		nodeGroups.push_back(group);
	}
	return nodeGroups;
}

string NodeContainer::to_str() const {
    ostringstream out;
	out << "NodeContainer[";
	if (hasCellsExcludingGroups()) {
        out << "cell ids:[" << getCellIdsExcludingGroups() << "],";
	}
	if (hasCellGroups()) {
        out << "cell groups:[" << getCellGroups() << "],";
	}
	if (not nodePositions.empty()) {
        out << "node ids:[" << getNodeIdsExcludingGroups() << "],";
	}
	if (not nodeGroupNames.empty()) {
        out << "node groups:[" << nodeGroupNames << "],";
	}
	out << "]";
	return out.str();
}

} /* namespace vega */

