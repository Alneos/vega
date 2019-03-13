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

unordered_map<CellType::Code, CellType*, EnumClassHash> CellType::typeByCode;

CellType::CellType(CellType::Code code, int numNodes, SpaceDimension dimension,
		const string& description) :
		code(code), numNodes(numNodes), dimension(dimension), description(description), specificSize{numNodes>0} {
	typeByCode.insert(make_pair(code, this));
}

CellType::CellType(const CellType& other) :
		code(other.code), numNodes(other.numNodes), dimension(other.dimension), description(
				other.description), specificSize(other.specificSize) {
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

Group::~Group() {

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

NodeIterator::NodeIterator(const NodeStorage *nodeStorage, int position) :
		nodeStorage(nodeStorage), position(position), endPosition(nodeStorage->mesh.countNodes()) {
}

void NodeIterator::increment() {
	position++;
	if (position > endPosition) {
		throw out_of_range(
				"Iterator on nodes in position " + to_string(position)
						+ " after end.");
	}
}

bool NodeIterator::hasNext() const {
	return (position < endPosition);
}

bool NodeIterator::equal(NodeIterator const& other) const {
	//this.mesh == other.mesh
	return this->position == other.position;
}

NodeIterator& NodeIterator::operator ++() {
	increment();
	return *this;
}

NodeIterator NodeIterator::operator ++(int) {
	increment();
	return *this;
}

bool NodeIterator::operator ==(const NodeIterator& rhs) const {
	return this->position == rhs.position;
}

bool NodeIterator::operator !=(const NodeIterator& rhs) const {
	return this->position != rhs.position;
}

const Node NodeIterator::operator *() {
	return nodeStorage->mesh.findNode(position);
}

const Node NodeIterator::next() {
	const Node& result(nodeStorage->mesh.findNode(position));
	this->increment();
	return result;
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

	unordered_map<CellType::Code, vector<vector<int>>, EnumClassHash > result = {
        {CellType::HEXA8.code, hexa8list}, //
        {CellType::TETRA4.code, tetra4list}, //
        {CellType::PYRA5.code, pyra5list}, //
        {CellType::PENTA6.code, penta6list}, //
    };
	return result;
}

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

const vector<int> Cell::faceids_from_two_nodes(int nodeId1, int nodeId2) const {
	int node1connectivityPos = findNodeIdPosition(nodeId1);
    int node2connectivityPos = Globals::UNAVAILABLE_INT;
	if (nodeId2 != Globals::UNAVAILABLE_INT) {
        node2connectivityPos = findNodeIdPosition(nodeId2);
	}

	if (type.dimension == SpaceDimension::DIMENSION_2D) {
		return vector<int>(nodeIds.begin(), nodeIds.end());
	}
	vector<int> nodePositions;
	//node2 is on the opposite face
	if (type.code == CellType::TETRA4.code) { //|| cellType == CellType::TETRA10
		vector<vector<int> > faceids = FACE_BY_CELLTYPE.find(CellType::TETRA4.code)->second;
		for (vector<int> faceid : faceids) {
			//0 based
			if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) == faceid.end()) {
				nodePositions.assign(faceid.begin(), faceid.end());
			}
		}
	} else if (type.code == CellType::HEXA8.code) {
		vector<vector<int> > faceids = FACE_BY_CELLTYPE.find(CellType::HEXA8.code)->second;
		for (vector<int> faceid : faceids) {
			//0 based
			if (find(faceid.begin(), faceid.end(), node2connectivityPos + 1) != faceid.end()
					&& find(faceid.begin(), faceid.end(), node1connectivityPos + 1)
							!= faceid.end()) {
				nodePositions.assign(faceid.begin(), faceid.end());
				break;
			}
		}
    } else if (type.code == CellType::PENTA6.code) {
        vector<vector<int> > faceids = FACE_BY_CELLTYPE.find(CellType::PENTA6.code)->second;
        for (vector<int> faceid : faceids) {
            if (find(faceid.begin(), faceid.end(), node1connectivityPos + 1) == faceid.end()) {
                continue;
            }
			if (nodeId2 == Globals::UNAVAILABLE_INT) {
                // nodePosition2 must be omitted for a **triangular** surface on a
                // CPENTA element.
                if (faceid.size() == 3) {
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
	} else {
		throw logic_error("FaceidfromtwoNodes not implemented for " + type.to_str() + " element type");
	}
	vector<int> faceConnectivity;
	faceConnectivity.reserve(nodePositions.size());
	for (int nodenum : nodePositions) {
		faceConnectivity.push_back(nodeIds[nodenum - 1]);
	}
	return faceConnectivity;
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
 * Node container;
 */

NodeContainer::NodeContainer(const Mesh& mesh) :
		mesh(mesh) {
}

void NodeContainer::addNodeId(int nodeId) {
	nodeIds.insert(nodeId);
}

void NodeContainer::addNodeGroup(const string& groupName) {
	shared_ptr<Group> group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: " + groupName + "not found.");
	}
	this->groupNames.insert(groupName);
}

void NodeContainer::add(const Node& node) {
	addNodeId(node.id);
}

void NodeContainer::add(const NodeGroup& nodeGroup) {
	this->groupNames.insert(nodeGroup.getName());
}

void NodeContainer::add(const NodeContainer& nodeContainer) {
	if (nodeContainer.nodeIds.size() > 0) {
		this->nodeIds.insert(nodeContainer.nodeIds.begin(), nodeContainer.nodeIds.end());
	}

	if (nodeContainer.groupNames.size() > 0) {
		this->groupNames.insert(nodeContainer.groupNames.begin(), nodeContainer.groupNames.end());
	}
}

const vector<int> NodeContainer::getNodeIds(bool all) const {
	vector<int> nodes(nodeIds.begin(), nodeIds.end());
	if (all) {
		for (string groupName : groupNames) {
			shared_ptr<NodeGroup> group = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
			if (group != nullptr) {
				nodes.insert(nodes.end(), group->getNodeIds().begin(), group->getNodeIds().end());
			}
		}
	}
	return nodes;
}

const set<int> NodeContainer::nodePositions() const {
	set<int> result;
	for (int nodeId : nodeIds) {
		result.insert(mesh.findNodePosition(nodeId));
	}
	return result;
}

bool NodeContainer::empty() const {
	return groupNames.empty() && nodeIds.empty();
}

void NodeContainer::clear() {
	groupNames.clear();
	nodeIds.clear();
}

bool NodeContainer::hasNodes() const {
	return !nodeIds.empty();
}

bool NodeContainer::hasNodeGroups() const {
	return groupNames.size() > 0;
}

const vector<shared_ptr<NodeGroup>> NodeContainer::getNodeGroups() const {
	vector<shared_ptr<NodeGroup>> nodeGroups;
	nodeGroups.reserve(groupNames.size());
	for (string groupName : groupNames) {
		shared_ptr<NodeGroup> group = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		nodeGroups.push_back(group);
	}
	return nodeGroups;
}

/*******************
 * Cell container;
 */

CellContainer::CellContainer(const Mesh& mesh) :
		mesh(mesh) {
}

void CellContainer::addCellId(int cellId) {
	cellIds.insert(cellId);
}

void CellContainer::addCellGroup(const string& groupName) {
	shared_ptr<Group> group = mesh.findGroup(groupName);
	if (group == nullptr) {
		throw logic_error("Group name: " + groupName + "not found.");
	}
	this->groupNames.insert(groupName);
}

void CellContainer::add(const Cell& cell) {
	addCellId(cell.id);
}

void CellContainer::add(const CellGroup& cellGroup) {
	this->groupNames.insert(cellGroup.getName());
}

void CellContainer::add(const CellContainer& cellContainer) {
	if (cellContainer.cellIds.size() > 0) {
		this->cellIds.insert(cellContainer.cellIds.begin(), cellContainer.cellIds.end());
	}

	if (cellContainer.groupNames.size() > 0) {
		this->groupNames.insert(cellContainer.groupNames.begin(), cellContainer.groupNames.end());
	}
}

const vector<Cell> CellContainer::getCells(bool all) const {
	vector<Cell> cells;
	cells.reserve(cellIds.size());
	for (int cellId : cellIds) {
		int cellPosition = mesh.findCellPosition(cellId);
		cells.push_back(mesh.findCell(cellPosition));
	}
	if (all) {
		for (string groupName : groupNames) {
			shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
			if (group != nullptr) {
				vector<Cell> cellsInGroup = group->getCells();
				cells.insert(cells.end(), cellsInGroup.begin(), cellsInGroup.end());
			}
		}
	}
	return cells;
}

const vector<int> CellContainer::getCellIds(bool all) const {
	vector<int> cells(cellIds.begin(), cellIds.end());
	if (all) {
		for (string groupName : groupNames) {
			shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
			if (group != nullptr) {
                const auto& groupCellIds = group->cellIds();
				cells.insert(cells.end(), groupCellIds.begin(), groupCellIds.end());
			}
		}
	}
	return cells;
}

const vector<int> CellContainer::getCellPositions(bool all) const {
	vector<int> cellPositions;
	cellPositions.reserve(cellIds.size());
	for (int cellId : cellIds) {
        cellPositions.push_back(mesh.findCellPosition(cellId));
	}
	if (all) {
		for (string groupName : groupNames) {
			shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
			if (group != nullptr) {
				cellPositions.insert(cellPositions.end(), group->cellPositions().begin(), group->cellPositions().end());
			}
		}
	}
	return cellPositions;
}

const set<int> CellContainer::nodePositions() const {
	set<int> result;
	for (Cell cell : getCells(true)) {
		result.insert(cell.nodePositions.begin(), cell.nodePositions.end());
	}
	return result;
}

bool CellContainer::containsCells(const CellType& cellType, bool all) {
	vector<Cell> cells = getCells(all);
	bool result = false;
	for (Cell cell : cells) {
		if (cell.type.code == cellType.code) {
			result = true;
			break;
		}
	}
	return result;
}

bool CellContainer::empty() const {
	return groupNames.empty() && cellIds.empty();
}

void CellContainer::clear() {
	groupNames.clear();
	cellIds.clear();
}

bool CellContainer::hasCells() const {
	return !cellIds.empty();
}

const vector<shared_ptr<CellGroup>> CellContainer::getCellGroups() const {
	vector<shared_ptr<CellGroup>> cellGroups;
	cellGroups.reserve(groupNames.size());
	for (string groupName : groupNames) {
		shared_ptr<CellGroup> group = dynamic_pointer_cast<CellGroup>(mesh.findGroup(groupName));
		if (group == nullptr) {
			throw invalid_argument("Cannot find group with name:" + groupName);
		}
		cellGroups.push_back(group);
	}
	return cellGroups;
}

bool CellContainer::hasCellGroups() const {
	return groupNames.size() > 0;
}

} /* namespace vega */

