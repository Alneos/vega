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

SpaceDimension::SpaceDimension(Code code, int medcouplingRelativeMeshDimension) :
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

bool SpaceDimension::operator<(const SpaceDimension &other) const {
	return this->code < other.code;
}

bool SpaceDimension::operator==(const SpaceDimension &other) const {
	return this->code == other.code;
}

bool SpaceDimension::operator!=(const SpaceDimension &other) const {
	return this->code != other.code;
}

unordered_map<CellType::Code, CellType*, EnumClassHash> CellType::typeByCode;

CellType::CellType(CellType::Code code, int numNodes, SpaceDimension dimension,
		const string& description) :
		code(code), numNodes(numNodes), dimension(dimension), description(description), specificSize{numNodes>0} {
	typeByCode.insert({code, this});
}

bool CellType::operator==(const CellType &other) const {
	return this->code == other.code;
}

bool CellType::operator<(const CellType& other) const {
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
ostream &operator<<(ostream &out, const CellType& cellType) {
	out << "CellType[" << cellType.description << "]";
	return out;
}

string CellType::to_str() const{
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


const CellType* CellType::findByCode(CellType::Code code) {
	return CellType::typeByCode[code];
}

const CellType CellType::polyType(unsigned int nbNodes) {
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

Group::Group(Mesh& mesh, const string& name, Type type, int _id, const string& comment) :
                Identifiable(_id), mesh(mesh), name(name), type(type), comment(comment), isUseful(false) {
}

const string& Group::getName() const {
	return this->name;
}

const string& Group::getComment() const {
	return this->comment;
}

/*******************
 * NodeGroup
 */
NodeGroup::NodeGroup(Mesh& mesh, const string& name, int groupId, const string& comment) :
		Group(mesh, name, Group::Type::NODEGROUP, groupId, comment) {
}

void NodeGroup::addNodeId(int nodeId) {
	int nodePosition = this->mesh.findOrReserveNode(nodeId);
	_nodePositions.insert(nodePosition);
}

void NodeGroup::addNode(const Node& node) {
	_nodePositions.insert(node.position);
}

void NodeGroup::addNodeByPosition(int nodePosition) {
	_nodePositions.insert(nodePosition);
}

void NodeGroup::removeNodeByPosition(int nodePosition) {
	if (_nodePositions.find(nodePosition) == _nodePositions.end()) {
		throw logic_error("Node position not present : " + to_string(nodePosition));
	}
	for (auto it = _nodePositions.begin(); it != _nodePositions.end();) {
		if (*it == nodePosition) {
			_nodePositions.erase(it);
			return;
		} else {
			it++;
		}
	}
}

bool NodeGroup::containsNodePosition(int nodePosition) const {
    return _nodePositions.find(nodePosition) != _nodePositions.end();
}

const std::set<int> NodeGroup::nodePositions() const {
	return _nodePositions;
}

bool NodeGroup::empty() const {
	return _nodePositions.size() == 0;
}

const set<int> NodeGroup::getNodeIds() const {
	set<int> nodeIds;
	for (int position : _nodePositions) {
		nodeIds.insert(mesh.nodes.nodeDatas[position].id);
	}
	return nodeIds;
}

const vector<Node> NodeGroup::getNodes() const {
	vector<Node> result;
	for (int nodePosition : _nodePositions) {
		result.push_back(mesh.findNode(nodePosition));
	}
	return result;
}

CellGroup::CellGroup(Mesh& mesh, const string& name, int groupId, const string& comment ) :
		Group(mesh, name, Group::Type::CELLGROUP, groupId, comment) {

}

void CellGroup::addCellId(int cellId) {
	_cellPositions.insert(mesh.findCellPosition(cellId));
}

void CellGroup::addCellPosition(int cellPosition) {
	_cellPositions.insert(cellPosition);
}

bool CellGroup::containsCellPosition(int cellPosition) const {
	return _cellPositions.find(cellPosition) != _cellPositions.end();
}

void CellGroup::removeCellPosition(int cellPosition) {
	_cellPositions.erase(cellPosition);
}

const vector<Cell> CellGroup::getCells() {
	vector<Cell> result;
	for (int cellPosition : _cellPositions) {
		result.push_back(mesh.findCell(cellPosition));
	}
	return result;
}

const vector<int> CellGroup::cellPositions() {
	vector<int> result{_cellPositions.begin(), _cellPositions.end()};
	return result;
}

const vector<int> CellGroup::cellIds() {
	vector<int> result;
    for (int cellPosition : _cellPositions) {
		result.push_back(mesh.findCellId(cellPosition));
	}
	return result;
}

const set<int> CellGroup::nodePositions() const {
	set<int> result;
	for (int cellPosition : _cellPositions) {
		const Cell& cell = mesh.findCell(cellPosition);
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

bool CellGroup::empty() const {
	return _cellPositions.size() == 0;
}

///////////////////////////////////////////////////////////////////////////////
/*                  Node                                                     */
///////////////////////////////////////////////////////////////////////////////
int Node::auto_node_id = 9999999;

Node::Node(int id, double lx, double ly, double lz, int position1, DOFS inElement1, double gx, double gy, double gz, int _positionCS, int _displacementCS, int _nodepartId) :
		id(id), nodepartId(_nodepartId), position(position1), lx(lx), ly(ly), lz(lz), dofs(inElement1), x(gx), y(gy), z(gz),
		positionCS(_positionCS), displacementCS(_displacementCS) {
}

double Node::distance(const Node& other) const {
    return boost::geometry::distance(*this, other);
    //return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2));
}

double Node::square_distance(const Node& other) const {
    return boost::geometry::comparable_distance(*this, other);
    //return pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2);
}

ostream &operator<<(ostream &out, const Node& node) {
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
unordered_map<CellType::Code, vector<vector<int>>, EnumClassHash > Cell::init_faceByCelltype() {
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
    vector<vector<int> > tetra10list = { //
        {1,5,2,6,3,7}, //
        {1,8,4,9,2,5}, //
        {1,7,3,10,4,8}, //
        {2,9,4,10,3,6} //
    };
    vector<vector<int> > pyra13list = { //
        {1,6,2,7,3,8,4,9}, //
        {1,10,5,11,2,6}, //
        {1,9,4,13,5,10}, //
        {4,8,3,12,5,13}, //
        {2,11,5,12,3,7}, //
    };
    vector<vector<int> > penta15list = { //
        {1,7,2,8,3,9}, //
        {4,12,6,11,5,10}, //
        {1,13,4,10,5,14,2,7}, //
        {1,9,3,15,6,12,4,13}, //
        {2,14,5,11,6,15,3,8} //
    };
    vector<vector<int> > hexa20list = { //
        {1,9,2,10,3,11,4,12}, //
        {5,13,8,14,7,15,6,16}, //
        {1,17,5,13,6,18,2,9}, //
        {2,18,6,14,7,19,3,10}, //
        {3,16,7,15,8,20,4,11}, //
        {1,12,4,20,8,16,5,17} //
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

Cell::Cell(int id, const CellType &type, const std::vector<int> &nodeIds, int position,
		const std::vector<int> &nodePositions, bool isvirtual,
		int cspos, int element_id, int cellTypePosition,
		std::shared_ptr<OrientationCoordinateSystem> orientation) :
		id(id), position(position), hasOrientation(cspos!=CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), type(type),
				nodeIds(nodeIds), nodePositions(nodePositions), isvirtual(isvirtual), elementId(
						element_id), cellTypePosition(cellTypePosition), cspos(cspos), orientation(orientation) {
}

int Cell::findNodeIdPosition(int node_id2) const {
	//|| cellType == CellType::TETRA10
	size_t node2connectivityPos = 0;
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
    const vector<vector<int> >& nodeConnectivityPosByFace = it->second;
    int faceNum = 1;
    map<int, vector<int>> result;
    for (vector<int> nodeConnectivityPos : nodeConnectivityPosByFace) {
        vector<int> faceConnectivity;
        faceConnectivity.reserve(nodeConnectivityPos.size());
        for (int nodenum : nodeConnectivityPos) {
            faceConnectivity.push_back(nodeIds[nodenum - 1]);
        }
        result[faceNum] = faceConnectivity;
        faceNum++;
    }
    return result;
}

vector<int> Cell::faceids_from_two_nodes(int nodeId1, int nodeId2) const {
	int node1connectivityPos = findNodeIdPosition(nodeId1);
    int node2connectivityPos = Globals::UNAVAILABLE_INT;
	if (nodeId2 != Globals::UNAVAILABLE_INT) {
        node2connectivityPos = findNodeIdPosition(nodeId2);
	}

	if (type.dimension == SpaceDimension::DIMENSION_2D) {
		return vector<int>(nodeIds.begin(), nodeIds.end());
	}
	const vector<vector<int>>& faceids = FACE_BY_CELLTYPE.find(type.code)->second;
	vector<int> nodePositions;
	switch(type.code) {
    case CellType::Code::TETRA4_CODE:
    case CellType::Code::TETRA10_CODE: {
        //node2 is on the opposite face
        if (node2connectivityPos == Globals::UNAVAILABLE_INT) {
            throw logic_error("Need two nodes to find a face on " + type.to_str() + " element type");
        }
		for (vector<int> faceid : faceids) {
			//0 based
			if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) == faceid.end()) {
				nodePositions.assign(faceid.begin(), faceid.end());
			}
		}
        break;
    }
    case CellType::Code::HEXA8_CODE:
    case CellType::Code::HEXA20_CODE: {
        if (node2connectivityPos == Globals::UNAVAILABLE_INT) {
            throw logic_error("Need two nodes to find a face on " + type.to_str() + " element type");
        }
		for (vector<int> faceid : faceids) {
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

        for (vector<int> faceid : faceids) {
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
	for (int nodenum : nodePositions) {
		faceConnectivity.push_back(nodeIds[nodenum - 1]);
	}
	return faceConnectivity;
}

vector<int> Cell::cornerNodeIds() const {
    const auto& it = CORNERNODEIDS_BY_CELLTYPE.find(type.code);
    if (it == CORNERNODEIDS_BY_CELLTYPE.end())
        throw logic_error("Missing CORNERNODEIDS_BY_CELLTYPE configuration for cell type :" + type.description);
    const vector<int>& nodeConnectivityPositions = it->second;
    vector<int> result;
    result.reserve(nodeConnectivityPositions.size());
    for (int nodenum : nodeConnectivityPositions) {
        result.push_back(nodeIds[nodenum]);
    }
    return result;
}

pair<int, int> Cell::two_nodeids_from_facenum(int faceNum) const {
    auto nodeIds_By_FaceNum = nodeIdsByFaceNum();
    const vector<int>& faceNodeIds = nodeIds_By_FaceNum[faceNum];
	switch(type.code) {
    case CellType::Code::TETRA4_CODE:
    case CellType::Code::TETRA10_CODE: {
        //node2 is on the opposite face
        set<int> cellOrderedNodeIds{nodeIds.begin(), nodeIds.end()};
        const set<int> faceOrderedNodeIds(faceNodeIds.begin(), faceNodeIds.end());
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

ostream &operator<<(ostream &out, const Cell& cell) {
	out << "Cell[id:" << cell.id;
	out << ",type:" << static_cast<int>(cell.type.code);
	out << ",nodeIds:[";
	for (const int nodeId : cell.nodeIds) {
		cout << "," << nodeId;
	}
	out << "]";
	return out;
}

CellIterator::CellIterator(const CellStorage* cellStorage, const CellType &cellType, bool begin) :
		cellStorage(cellStorage), endPosition(cellStorage->mesh.countCells(cellType)), cellType(cellType), position(
				begin ? 0 : endPosition) {
}

CellIterator::~CellIterator() {
}

const Cell CellIterator::next() {
	Cell result = dereference();
	increment(1);
	return result;
}

void CellIterator::increment(int i) {
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
	increment(1);
	return *this;
}

bool CellIterator::operator ==(const CellIterator& other) const {
	//cout << "this p " << this->position << "other p:" << other.position << endl;
	return (this->position == other.position) && (cellType.code == other.cellType.code)
			&& (this->endPosition == other.endPosition);
}

bool CellIterator::operator !=(const CellIterator& rhs) const {
	return !(*this == rhs);
}

const Cell CellIterator::dereference() const {
	return cellStorage->mesh.findCell(cellStorage->mesh.cellPositionsByType.find(cellType)->second[position]);
}

const Cell CellIterator::operator *() const {
	return dereference();
}

bool CellIterator::hasNext() const {
	return (position < endPosition);
}

bool CellIterator::equal(CellIterator const &other) const {
	//too slow! (this->cellIds->isEqual(*other.cellIds)
	return (this->position == other.position) && (cellType.code == other.cellType.code)
			&& (this->endPosition == other.endPosition);
}

/*******************
 * Cell container;
 */

CellContainer::CellContainer(const Mesh& mesh) :
		mesh(mesh) {
}

void CellContainer::addCellPosition(int cellPosition) {
	cellPositions.insert(cellPosition);
}

void CellContainer::addCellId(int cellId) {
	cellPositions.insert(mesh.findCellPosition(cellId));
}

void CellContainer::addCellIds(const vector<int>& otherIds) {
    for(const int cellId : otherIds) {
        cellPositions.insert(mesh.findCellPosition(cellId));
    }
}

void CellContainer::addCellIds(const set<int>& otherIds) {
    for(const int cellId : otherIds) {
        cellPositions.insert(mesh.findCellPosition(cellId));
    }
}

void CellContainer::addCellPositions(const vector<int>& otherPositions) {
    cellPositions.insert(otherPositions.begin(), otherPositions.end());
}

void CellContainer::addCellPositions(const set<int>& otherPositions) {
    cellPositions.insert(otherPositions.begin(), otherPositions.end());
}

void CellContainer::addCellGroup(const string& groupName) {
	shared_ptr<Group> group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: " + groupName + "not found.");
	}
	this->cellGroupNames.insert(groupName);
}

void CellContainer::add(const Cell& cell) {
	cellPositions.insert(cell.position);
}

void CellContainer::add(const CellGroup& cellGroup) {
	this->cellGroupNames.insert(cellGroup.getName());
}

void CellContainer::add(const CellContainer& cellContainer) {
    set<int> otherCellPositions = cellContainer.getCellPositionsExcludingGroups();
	if (not otherCellPositions.empty()) {
		cellPositions.insert(otherCellPositions.begin(), otherCellPositions.end());
	}

	if (not cellContainer.cellGroupNames.empty()) {
		this->cellGroupNames.insert(cellContainer.cellGroupNames.begin(), cellContainer.cellGroupNames.end());
	}
}

set<Cell> CellContainer::getCellsExcludingGroups() const {
	set<Cell> cells;
	for (int cellPosition : cellPositions) {
		cells.insert(mesh.findCell(cellPosition));
	}
	return cells;
}

set<Cell> CellContainer::getCellsIncludingGroups() const {
	set<Cell>&& cells = getCellsExcludingGroups();
    for (string groupName : cellGroupNames) {
        shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& cellsInGroup = group->getCells();
            cells.insert(cellsInGroup.begin(), cellsInGroup.end());
        }
    }
	return cells;
}

void CellContainer::removeCellsNotInAGroup() {
    cellPositions.clear();
}

set<int> CellContainer::getCellIdsIncludingGroups() const {
	set<int> result;
	for (const int cellPosition : cellPositions) {
	    result.insert(mesh.findCellId(cellPosition));
	}
    for (string groupName : cellGroupNames) {
        shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& groupCellIds = group->cellIds();
            result.insert(groupCellIds.begin(), groupCellIds.end());
        }
    }
	return result;
}

set<int> CellContainer::getCellPositionsIncludingGroups() const {
	set<int> result(cellPositions.begin(), cellPositions.end());
    for (string groupName : cellGroupNames) {
        shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
        if (group != nullptr) {
            const auto& groupCellPositions = group->cellPositions();
            result.insert(groupCellPositions.begin(), groupCellPositions.end());
        }
    }
	return result;
}

set<int> CellContainer::getCellPositionsExcludingGroups() const {
	return cellPositions;
}

set<int> CellContainer::getNodePositionsIncludingGroups() const {
	set<int> result;
	for (Cell cell : getCellsIncludingGroups()) {
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

set<int> CellContainer::getNodePositionsExcludingGroups() const {
	set<int> result;
	for (Cell cell : getCellsExcludingGroups()) {
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

bool CellContainer::empty() const {
	return cellGroupNames.empty() and cellPositions.empty();
}

void CellContainer::clear() {
	cellGroupNames.clear();
	cellPositions.clear();
}

bool CellContainer::hasCells() const {
	return not cellPositions.empty();
}

vector<shared_ptr<CellGroup>> CellContainer::getCellGroups() const {
	vector<shared_ptr<CellGroup>> cellGroups;
	cellGroups.reserve(cellGroupNames.size());
	for (string groupName : cellGroupNames) {
		shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		cellGroups.push_back(group);
	}
	return cellGroups;
}

bool CellContainer::hasCellGroups() const {
	return not cellGroupNames.empty();
}

/*******************
 * Node container;
 */

NodeContainer::NodeContainer(Mesh& mesh) :
		CellContainer(mesh), mesh(mesh) {
}

void NodeContainer::addNodeId(int nodeId) {
	nodePositions.insert(mesh.findOrReserveNode(nodeId));
}

void NodeContainer::addNodeIds(const set<int>& range) {
    for(const int nodePosition : range) {
        nodePositions.insert(mesh.findOrReserveNode(nodePosition));
    }
}

void NodeContainer::addNodeIds(const vector<int>& range) {
    for(const int nodePosition : range) {
        nodePositions.insert(mesh.findOrReserveNode(nodePosition));
    }
}

void NodeContainer::addNodePosition(int nodePosition) {
	nodePositions.insert(nodePosition);
}

void NodeContainer::addNodeGroup(const string& groupName) {
	shared_ptr<Group> group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: [" + groupName + "] not found.");
	}
	nodeGroupNames.insert(groupName);
}

void NodeContainer::removeNodePosition(int nodePosition) {
	nodePositions.erase(nodePositions.find(nodePosition));
}

void NodeContainer::add(const Node& node) {
	nodePositions.insert(node.position);
}

void NodeContainer::add(const NodeGroup& nodeGroup) {
	nodeGroupNames.insert(nodeGroup.getName());
}

void NodeContainer::add(const NodeContainer& nodeContainer) {
    const auto& otherNodePositions = nodeContainer.getNodePositionsExcludingGroups();
	if (otherNodePositions.size() > 0) {
		nodePositions.insert(otherNodePositions.begin(), otherNodePositions.end());
	}

	if (nodeContainer.nodeGroupNames.size() > 0) {
		nodeGroupNames.insert(nodeContainer.nodeGroupNames.begin(), nodeContainer.nodeGroupNames.end());
	}
}

set<int> NodeContainer::getNodePositionsExcludingGroups() const {
    if (CellContainer::empty()) {
        return nodePositions;
    } else {
        set<int> result(nodePositions);
        const set<int>& cellNodePositions = CellContainer::getNodePositionsExcludingGroups();
        result.insert(cellNodePositions.begin(), cellNodePositions.end());
        return result;
    }
}

set<int> NodeContainer::getNodePositionsIncludingGroups() const {
    set<int> result(nodePositions);
	for (string groupName : nodeGroupNames) {
		shared_ptr<NodeGroup> group = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
		set<int> groupNodePositions =  group->nodePositions();
		result.insert(groupNodePositions.begin(), groupNodePositions.end());
	}
    const set<int>& cellNodePositions = CellContainer::getNodePositionsIncludingGroups();
    result.insert(cellNodePositions.begin(), cellNodePositions.end());
	return result;
}

set<int> NodeContainer::getNodeIdsIncludingGroups() const {
    set<int> result;
	for (const int nodePosition : getNodePositionsIncludingGroups()) {
	    result.insert(mesh.findNodeId(nodePosition));
	}
	return result;
}

bool NodeContainer::empty() const {
	return nodeGroupNames.empty() and nodePositions.empty() and CellContainer::empty();
}

void NodeContainer::clear() {
	nodeGroupNames.clear();
	nodePositions.clear();
	CellContainer::clear();
}

bool NodeContainer::hasNodes() const {
	return not nodePositions.empty();
}

bool NodeContainer::hasNodeGroups() const {
	return not nodeGroupNames.empty();
}

vector<shared_ptr<NodeGroup>> NodeContainer::getNodeGroups() const {
	vector<shared_ptr<NodeGroup>> nodeGroups;
	for (string groupName : nodeGroupNames) {
		shared_ptr<NodeGroup> group = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		nodeGroups.push_back(group);
	}
	return nodeGroups;
}

} /* namespace vega */

