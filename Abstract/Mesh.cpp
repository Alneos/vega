/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Mesh.cpp
 *
 *  Created on: 26 févr. 2013
 *      Author: dallolio
 */

#include "Mesh.h"

#if defined VDEBUG && defined __GNUC__  && !defined(_WIN32)
#include <valgrind/memcheck.h>
#endif
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include "Model.h"
#include <med.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <new>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace vega {

using namespace std;
using boost::lexical_cast;
namespace fs = boost::filesystem;

const double NodeStorage::RESERVED_POSITION = -DBL_MAX;

NodeData::NodeData(int id, const DOFS& dofs, double x, double y, double z, int cpPos, int cdPos) :
    id(id), dofs(dofs), x(x), y(y), z(z), cpPos(cpPos), cdPos(cdPos) {
}

/**
 * Node Container class
 */
NodeStorage::NodeStorage(Mesh& mesh, LogLevel logLevel) :
		logLevel(logLevel), mesh(mesh) {
	nodeDatas.reserve(4096);
}

NodeIterator NodeStorage::begin() const {
	return NodeIterator(this, 0);
}

NodeIterator NodeStorage::end() const {
	return NodeIterator(this, static_cast<int>(nodeDatas.size()));
}

int NodeStorage::reserveNodePosition(int nodeId) {
	int nodePosition = mesh.addNode(nodeId, RESERVED_POSITION, RESERVED_POSITION,
			RESERVED_POSITION);
	nodepositionById[nodeId] = nodePosition;
	if (this->logLevel >= LogLevel::TRACE) {
		cout << "Reserve node id:" << nodeId << " position:" << nodePosition << endl;
	}
	return nodePosition;
}

CellData::CellData(int id, const CellType& type, bool isvirtual, int elementId, int cellTypePosition) :
		id(id), typeCode(type.code), isvirtual(isvirtual), elementId(
				elementId), cellTypePosition(cellTypePosition) {
}

int Mesh::addNode(int id, double x, double y, double z, int cpPos, int cdPos) {
	int nodePosition;

	// In auto mode, we assign the first free node, starting from the biggest possible number
	if (id == Node::AUTO_ID){
		id = Node::auto_node_id--;
		while (findNodePosition(id)!= Node::UNAVAILABLE_NODE){
			id = Node::auto_node_id--;
		}
	}
	auto positionIterator = nodes.nodepositionById.find(id);
	if (positionIterator == nodes.nodepositionById.end()) {
		nodePosition = static_cast<int>(nodes.nodeDatas.size());
		NodeData nodeData(id, DOFS::NO_DOFS, x, y, z, cpPos, cdPos);
		nodes.nodeDatas.push_back(nodeData);
		nodes.nodepositionById[id] = nodePosition;
	} else {
		nodePosition = positionIterator->second;
		NodeData& nodeData = nodes.nodeDatas[nodePosition];
		nodeData.x = x;
		nodeData.y = y;
		nodeData.z = z;
        nodeData.cpPos = cpPos;
        nodeData.cdPos = cdPos;
	}

	return nodePosition;
}

int Mesh::countNodes() const {
	return static_cast<int>(nodes.nodeDatas.size());
}

const Node Mesh::findNode(const int nodePosition, const bool buildGlobalXYZ, const Model* model) const {
	if (nodePosition == Node::UNAVAILABLE_NODE) {
		throw invalid_argument(
				string("Node position ") + lexical_cast<string>(nodePosition) + " not found.");
	}
	const NodeData &nodeData = nodes.nodeDatas[nodePosition];
	Node node1 = Node(nodeData.id, nodeData.x, nodeData.y, nodeData.z, nodePosition, nodeData.dofs,
			nodeData.cpPos, nodeData.cdPos);

	// If asked, we compute the position of the Node in the Global Referentiel System
	// TODO LD: change this with something more explicit (like globalx etc.)
	if (buildGlobalXYZ){
	    node1.buildGlobalXYZ(model);
	}
	/*
	 * #if defined VDEBUG && defined __GNUC__
	 *	cerr << "node:" << node1 << endl;
	 *	VALGRIND_CHECK_VALUE_IS_DEFINED(node1);
	 *	cerr << "valid" << endl;
	 * #endif
	 */
	return node1;
}

int Mesh::findOrReserveNode(int nodeId) {

	int nodePosition = findNodePosition(nodeId);
	return nodePosition != Node::UNAVAILABLE_NODE ? nodePosition : nodes.reserveNodePosition(nodeId);

}

set<int> Mesh::findOrReserveNodes(const set<int>& nodeIds) {
	set<int> nodePositions;

	for (int nodeId : nodeIds) {
		nodePositions.insert(this->findOrReserveNode(nodeId));
	}
	return nodePositions;
}

int Mesh::findNodePosition(const int nodeId) const {
	auto positionIterator = this->nodes.nodepositionById.find(nodeId);
	if (positionIterator == this->nodes.nodepositionById.end()) {
		return Node::UNAVAILABLE_NODE;
	}
	return positionIterator->second;
}

void Mesh::allowDOFS(int nodePosition, const DOFS& allowed) {
	nodes.nodeDatas[nodePosition].dofs = static_cast<char>(nodes.nodeDatas[nodePosition].dofs
			| allowed);
}

bool NodeStorage::validate() const {
	bool validNodes = true;
	for (size_t i = 0; i < nodeDatas.size(); ++i) {
		const NodeData &nodeData = nodeDatas[i];
		if (nodeData.id == Node::UNAVAILABLE_NODE) {
			validNodes = false;
			cerr << "Node in position " << i << " has been reserved, but never defined" << endl;
		}
	}
	if (validNodes && this->logLevel >= LogLevel::DEBUG) {
		cout << "All the reserved nodes have been defined." << endl;
	}
	return validNodes;
}

/******************************************************************************
 * Mesh class
 ******************************************************************************/

Mesh::Mesh(LogLevel logLevel, const string& modelName) :
		logLevel(logLevel), name(modelName), //
		nodes(NodeStorage(*this, this->logLevel)),
				cells(CellStorage(*this, this->logLevel)) {

	finished = false;
	for (auto cellTypePair : CellType::typeByCode) {
		cellPositionsByType[*(cellTypePair.second)] = vector<int>();
	}
}

int Mesh::addCell(int id, const CellType &cellType, const std::vector<int> &nodeIds,
		bool virtualCell, const int cpos, int elementId) {
	int cellId;
	const int cellPosition = static_cast<int>(cells.cellDatas.size());

	// In "auto" mode, we choose the first available Id, starting from the maximum authorized number
	if (id == Cell::AUTO_ID) {
		cellId = Cell::auto_cell_id--;
		while (findCellPosition(cellId)!= Cell::UNAVAILABLE_CELL){
			cellId = Cell::auto_cell_id--;
		}
	} else {
		cellId = id;
	}
	if (cellType.numNodes == 0) {
		cerr << "Unsupported cell type" << cellType << endl;
	}
	if (this->logLevel >= LogLevel::TRACE) {
		//check connectivity, it causes strange errors in med
		set<int> coord_set(nodeIds.begin(), nodeIds.end());
		if (coord_set.size() != nodeIds.size()) {
			cerr << "Cell ID:" << cellId << " has duplicate nodes in connectivity. ";
			copy(nodeIds.begin(), nodeIds.end(),
					ostream_iterator<int>(cerr, " "));
			cerr << endl;
			throw logic_error(
					string("Duplicate node in connectivity cellId:")
							+ lexical_cast<string>(cellId));
		}
		if (cells.cellpositionById.find(cellId) != cells.cellpositionById.end()) {
			throw logic_error(
					string("CellId: ") + lexical_cast<string>(cellId) + " Already used.");
		}
	}
	if ((cellType.specificSize) && (cellType.numNodes != nodeIds.size())) {
		cerr << "Cell " << cellId << " not added because connectivity array differs from expected "
				"length";
		throw logic_error("Invalid cell");
	}

	cells.cellpositionById[cellId] = cellPosition;
	const int cellTypePosition = static_cast<int>(cellPositionsByType.find(cellType)->second.size());
	cellPositionsByType.find(cellType)->second.push_back(cellPosition);
	CellData cellData(cellId, cellType, virtualCell, elementId, cellTypePosition);

	if (cells.nodepositionsByCelltype.find(cellType) == cells.nodepositionsByCelltype.end()) {
		cells.nodepositionsByCelltype[cellType] = make_shared<deque<int>>(deque<int>());
	}
	shared_ptr<deque<int>> nodePositionsPtr = cells.nodepositionsByCelltype[cellType];
	for (unsigned int i = 0; i < nodeIds.size(); i++) {
		int nodePosition = findOrReserveNode(nodeIds[i]);
		nodePositionsPtr->push_back(nodePosition);
	}
	if (cpos != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
		std::shared_ptr<CellGroup> coordinateSystemCellGroup = this->getOrCreateCellGroupForCS(cpos);
		coordinateSystemCellGroup->addCellId(cellId);
		cellData.csPos = cpos;
    }
	cells.cellDatas.push_back(cellData);
	return cellPosition;
}

int Mesh::updateCell(int id, const CellType &cellType, const std::vector<int> &nodeIds,
        bool virtualCell, const int cpos, int elementId) {

    if (id == Cell::AUTO_ID) {
        throw invalid_argument("Can't update a cell with AUTO_ID.");
    }
    if (findCellPosition(id)== Cell::UNAVAILABLE_CELL){
        throw invalid_argument("Can't update a cell which does not exist yet.");
    }
    if (cellType.numNodes == 0) {
        cerr << "Unsupported cell type" << cellType << endl;
    }

    // Copy/paste from addCell : don't know why it's needed ?
    if (this->logLevel >= LogLevel::TRACE) {
        //check connectivity, it causes strange errors in med
        set<int> coord_set(nodeIds.begin(), nodeIds.end());
        if (coord_set.size() != nodeIds.size()) {
            cerr << "Cell ID:" << id << " has duplicate nodes in connectivity. ";
            copy(nodeIds.begin(), nodeIds.end(), ostream_iterator<int>(cerr, " "));
            cerr << endl;
            throw logic_error(
                    string("Duplicate node in connectivity cellId:")
                            + lexical_cast<string>(id));
        }
    }

    if ((cellType.specificSize) && (cellType.numNodes != nodeIds.size())) {
        cerr << "Cell " << id << " not updated because connectivity array differs from expected "
                "length";
        throw logic_error("Invalid cell");
    }

    // We don't update the old data, which is too complicated
    // We build another CellData, with an other cellPosition, and hope
    // for the best
    const int cellPosition = static_cast<int>(cells.cellDatas.size());
    cells.cellpositionById[id] = cellPosition;

    const int cellTypePosition = static_cast<int>(cellPositionsByType.find(cellType)->second.size());
    cellPositionsByType.find(cellType)->second.push_back(cellPosition);
    CellData cellData(id, cellType, virtualCell, elementId, cellTypePosition);

    if (cells.nodepositionsByCelltype.find(cellType) == cells.nodepositionsByCelltype.end()) {
        cells.nodepositionsByCelltype[cellType] = make_shared<deque<int>>(deque<int>());
    }
    shared_ptr<deque<int>> nodePositionsPtr = cells.nodepositionsByCelltype[cellType];
    for (unsigned int i = 0; i < nodeIds.size(); i++) {
        int nodePosition = findOrReserveNode(nodeIds[i]);
        nodePositionsPtr->push_back(nodePosition);
    }
    if (cpos != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        std::shared_ptr<CellGroup> coordinateSystemCellGroup = this->getOrCreateCellGroupForCS(cpos);
        coordinateSystemCellGroup->addCellId(id);
        cellData.csPos = cpos;
        }
    cells.cellDatas.push_back(cellData);

    return cellPosition;
}


const Cell Mesh::findCell(int cellPosition) const {
	if (cellPosition == Cell::UNAVAILABLE_CELL) {
		throw logic_error("Unavailable cell requested.");
	}
	const CellData& cellData = cells.cellDatas[cellPosition];
	const CellType* type = CellType::findByCode(cellData.typeCode);
	const unsigned int numNodes = type->numNodes;
	vector<int> nodeIds;
	nodeIds.resize(numNodes);
	auto it = cells.nodepositionsByCelltype.find(*type);
	deque<int>& globalNodePositions = *(it->second);
	const int start = cellData.cellTypePosition * numNodes;
	vector<int> nodePositions(globalNodePositions.begin() + start,
			globalNodePositions.begin() + start + numNodes);
	for (unsigned int i = 0; i < numNodes; i++) {
		const NodeData &nodeData = nodes.nodeDatas[nodePositions[i]];
		nodeIds[i] = nodeData.id;
	}
	Cell cell(cellData.id, *type, nodeIds, nodePositions, false, cellData.csPos, cellData.elementId, cellData.cellTypePosition);
	return cell;
}


void Mesh::createFamilies(med_idt fid, const char meshname[MED_NAME_SIZE + 1],
		const vector<Family>& families) {
	for (auto& family : families) {
		const unsigned int ngroups = static_cast<unsigned int>(family.groups.size());
		char* groupname = new char[ngroups * MED_LNAME_SIZE + 1]();
		for (unsigned int i = 0; i < ngroups; i++) {
			strncpy(groupname + (i * MED_LNAME_SIZE), family.groups[i]->getName().c_str(),
			MED_LNAME_SIZE);
		}

		if (MEDfamilyCr(fid, meshname, family.name.c_str(), family.num, ngroups, groupname) < 0) {
			throw logic_error("ERROR : creating family ...");
		}
		delete[] (groupname);
	}
}

void Mesh::writeMED(const Model& model, const char* medFileName) {
	if (!finished) {
		this->finish();
	}
	const char meshname[MED_NAME_SIZE + 1] = "3D unstructured mesh";
	const med_int spacedim = 3;
	const med_int meshdim = 3;
	const char axisname[3 * MED_SNAME_SIZE + 1] = "x               y               z               ";
	const char unitname[3 * MED_SNAME_SIZE + 1] = "m               m               m               ";
	const int nnodes = this->countNodes();
	if (this->logLevel >= LogLevel::DEBUG) {
		med_int v[3];
		MEDlibraryNumVersion(&v[0], &v[1], &v[2]);
		cout << "Med Version : " << v[0] << "." << v[1] << "." << v[2] << endl;
		cout << "Num nodes : " << nnodes << endl;
		cout << "Num cells : " << this->countCells() << endl;
	}

	/* open MED file */
	med_idt fid = MEDfileOpen(medFileName, MED_ACC_CREAT);
	if (fid < 0) {
		throw logic_error("ERROR : MED file creation ...");
	}
	/* mesh creation : a 2D unstructured mesh */

	if (MEDmeshCr(fid, meshname, spacedim, meshdim, MED_UNSTRUCTURED_MESH, this->name.c_str(), "",
			MED_SORT_DTIT, MED_CARTESIAN, axisname, unitname) < 0) {
		throw logic_error("ERROR : Mesh creation ...");
	}
	vector<med_float> coordinates;
	coordinates.reserve(nnodes);
	for (NodeData& nodeData : nodes.nodeDatas) {
	    if (nodeData.cpPos == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
            coordinates.push_back(nodeData.x);
            coordinates.push_back(nodeData.y);
            coordinates.push_back(nodeData.z);
	    } else {
	        shared_ptr<CoordinateSystem> coordSystem = model.getCoordinateSystemByPosition(nodeData.cpPos);
            VectorialValue gCoord = coordSystem->positionToGlobal(VectorialValue(nodeData.x,nodeData.y,nodeData.z));
            coordinates.push_back(gCoord.x());
            coordinates.push_back(gCoord.y());
            coordinates.push_back(gCoord.z());
	    }
	}
	if (MEDmeshNodeCoordinateWr(fid, meshname, MED_NO_DT, MED_NO_IT, 0.0, MED_FULL_INTERLACE,
			nnodes, coordinates.data()) < 0) {
		throw logic_error("ERROR : writing nodes ...");
	}

	/*char* nodeNames = new char[nodes.countNodes()*MED_SNAME_SIZE+1]();

	 for(int i=0; i<nodes.countNodes();i++){
	 string medName=string("N") +lexical_cast<string>(i+1);
	 strncpy(nodeNames+(i*MED_SNAME_SIZE),medName.c_str(),MED_SNAME_SIZE);
	 }
	 MEDmeshEntityNameWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE,
	 nodes.countNodes(), nodeNames);
	 delete[](nodeNames);*/

	for (auto kv : cellPositionsByType) {
		CellType type = kv.first;
		vector<int> cellPositions = kv.second;
		size_t numCells = cellPositions.size();
		if (type.numNodes == 0 || numCells == 0) {
			continue;
		}
		vector<med_int> connectivity;
		connectivity.reserve(numCells * type.numNodes);
		for (int cellPosition : cellPositions) {
			const Cell& cell = findCell(cellPosition);
			for (int nodePosition : cell.nodePositions) {
				// med nodes starts at node number 1.
				connectivity.push_back(nodePosition + 1);
			}
		}
		int result = MEDmeshElementConnectivityWr(fid, meshname, MED_NO_DT,
		MED_NO_IT, 0.0, MED_CELL, type.code, MED_NODAL, MED_FULL_INTERLACE,
				static_cast<med_int>(numCells),
				connectivity.data());
		if (result < 0) {
			throw logic_error("ERROR : writing cells ...");
		}

		/*		 char* cellNames = new char[numCells*MED_SNAME_SIZE+1]();
		 //med_int* cellNum=new med_int[numCells];
		 CellPositionInfo cpInfo(0,code);
		 for(int i=0; i<numCells;i++){
		 cpInfo.cellPosition = i;
		 //FIXME:Utterly slow
		 const vega::Cell cell = this->findCell(&cpInfo);
		 strncpy(cellNames+(i*MED_SNAME_SIZE),cell.getMedName().c_str(),MED_SNAME_SIZE);
		 }
		 result = MEDmeshEntityNameWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL, code,
		 numCells, cellNames);
		 result = MEDmeshEntityNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL, code,
		 numCells, cellNum);

		 delete[](cellNames);

		 if (result < 0) {
		 throw logic_error("ERROR : writing cell names ...");
		 }*/
	}

	if (MEDfamilyCr(fid, meshname, MED_NO_NAME, 0, 0, MED_NO_GROUP) < 0) {
		throw logic_error("ERROR : writing family 0 ...");
	}

	vector<shared_ptr<NodeGroup>> nodeGroups = getNodeGroups();
	if (nodeGroups.size() > 0) {
		NodeGroup2Families ng2fam(countNodes(), nodeGroups);
		//WARN: if writing to file is delayed to MEDfileClose may be necessary
		//to move the allocation outside the if
		auto& families = ng2fam.getFamilies();
		createFamilies(fid, meshname, families);
		//write family number for nodes
		if (MEDmeshEntityFamilyNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE,
				nnodes, ng2fam.getFamilyOnNodes().data()) < 0) {
			throw logic_error("ERROR : writing family on nodes ...");
		}
	}
	vector<shared_ptr<CellGroup>> cellGroups = this->getCellGroups();
	if (cellGroups.size() > 0) {
		unordered_map<CellType::Code, int, hash<int>> cellCountByType;
		for (auto typeAndCodePair : CellType::typeByCode) {
			int cellNum = this->countCells(*typeAndCodePair.second);
			if (cellNum > 0) {
				cellCountByType[typeAndCodePair.first] = cellNum;
			}
		}
		CellGroup2Families cellGroup2Family = CellGroup2Families(*this, cellCountByType, cellGroups);
		createFamilies(fid, meshname, cellGroup2Family.getFamilies());
		for (auto cellCodeFamilyVectorPair : cellGroup2Family.getFamilyOnCells()) {
			int ncells = static_cast<int>(cellCodeFamilyVectorPair.second->size());
			if (MEDmeshEntityFamilyNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL,
					cellCodeFamilyVectorPair.first, ncells, cellCodeFamilyVectorPair.second->data())
					< 0) {
				throw logic_error("ERROR : writing family on cells ...");
			}
		}
	}
	/*
	 if (MEDmeshElementWr(fid, meshname, MED_NO_DT, MED_NO_IT, 0.0, MED_CELL, MED_TRIA3,
	 MED_NODAL, MED_FULL_INTERLACE, 30, conn3,MED_FALSE,nullptr,MED_FALSE,nullptr,MED_FALSE,nullptr) < 0) {
	 MESSAGE("ERROR : triangular cells connectivity ...");
	 goto ERROR;
	 }
	 */
	/* close MED file */
	if (MEDfileClose(fid) < 0) {
		throw logic_error("ERROR : closing med file ...");
	}
	if (this->logLevel >= LogLevel::DEBUG) {
		cout << "File created : " << fs::absolute(medFileName) << endl;
	}
}

shared_ptr<CellGroup> Mesh::getOrCreateCellGroupForCS(int cid){
	shared_ptr<CellGroup> result;
	auto cellGroupNameIter = cellGroupNameByCID.find(cid);
	if (cellGroupNameIter != cellGroupNameByCID.end()) {
		string cellGroupName = cellGroupNameIter->second;
		result = dynamic_pointer_cast<CellGroup>(findGroup(cellGroupName));
	} else {
		string gmaName;
		string id = lexical_cast<string>(cellGroupNameByCID.size() + 1);
		if (id.length() > 7) {
			gmaName = string("C") + id.substr(id.length() - 7, 7);
		} else {
			gmaName = string("C") + id;
		}
		cellGroupNameByCID[cid] = gmaName;
		result = createCellGroup(gmaName, CellGroup::NO_ORIGINAL_ID, string("Orientation"));
	}
	return result;
}

CellStorage::CellStorage(Mesh& mesh, LogLevel logLevel) :
		logLevel(logLevel), mesh(mesh) {
}

CellIterator CellStorage::cells_begin(const CellType &type) const {
	if (type.numNodes == 0) {
		throw logic_error(
				string("Iteration on ") + lexical_cast<string>(type) + " not implemented");
	}

	return CellIterator(this, type, CellIterator::POSITION_BEGIN);
}

CellIterator CellStorage::cells_end(const CellType &type) const {
	if (type.numNodes == 0) {
		throw logic_error(
				string("Iteration on ") + lexical_cast<string>(type) + " not implemented");
	}
	return CellIterator(this, type, CellIterator::POSITION_END);
}

int Mesh::countCells() const {
	return static_cast<int>(cells.cellpositionById.size());
}

void Mesh::finish() {
	finished = true;

}

shared_ptr<NodeGroup> Mesh::createNodeGroup(const string& name, int group_id, const string & comment) {
	if (name.empty()) {
		throw invalid_argument("Can't create a nodeGroup with empty name ");
	}
	if (this->groupByName.find(name) != this->groupByName.end()) {
		throw invalid_argument("Another group exists with same name : " + name);
	}
	if (group_id != NodeGroup::NO_ORIGINAL_ID
			&& this->groupById.find(group_id) != this->groupById.end()) {
		string errorMessage = "Another group exists with same id : " + to_string(group_id);
        if (logLevel >= LogLevel::DEBUG) {
            stacktrace();
        }
		throw invalid_argument(errorMessage);
	}
	shared_ptr<NodeGroup> group = shared_ptr<NodeGroup>(new NodeGroup(*this, name, group_id, comment));
	this->groupByName[name] = group;
	if (group_id != NodeGroup::NO_ORIGINAL_ID) {
		this->groupById[group_id] = group;
	}

	if (this->logLevel >= LogLevel::DEBUG) {
		cout << "Created Node Group:" << name <<" with comment: "<<comment<< endl;
	}
	return group;
}

shared_ptr<NodeGroup> Mesh::findOrCreateNodeGroup(const string& name, int group_id, const string & comment) {
	if (name.empty()) {
		throw invalid_argument("Can't find or create a nodeGroup with empty name ");
	}
	shared_ptr<NodeGroup> group = dynamic_pointer_cast<NodeGroup>(this->findGroup(name));
	if (group==nullptr){
		return this->createNodeGroup(name, group_id, comment);
	}else{
		if (group->type != Group::NODEGROUP) {
			throw invalid_argument("Group " + name + " is not a nodeGroup.");
		}
		return group;
	}
}

shared_ptr<CellGroup> Mesh::createCellGroup(const string& name, int group_id, const string & comment) {
	if (name.empty()) {
		throw invalid_argument("Can't create a cellGroup with empty name.");
	}
	if (this->groupByName.find(name) != this->groupByName.end()) {
		throw invalid_argument("Another group exists with same name: " + name);
	}
	if (group_id != CellGroup::NO_ORIGINAL_ID
			&& this->groupById.find(group_id) != this->groupById.end()) {
		string errorMessage = "Another group exists with same id: " + to_string(group_id);
		if (logLevel >= LogLevel::DEBUG) {
            stacktrace();
        }
		throw invalid_argument(errorMessage);
	}
	shared_ptr<CellGroup> group= shared_ptr<CellGroup>(new CellGroup(*this, name, group_id, comment));
	this->groupByName[name] = group;
	if (group_id != CellGroup::NO_ORIGINAL_ID) {
		this->groupById[group_id] = group;
	}

	if (this->logLevel >= LogLevel::DEBUG) {
		cout << "Created Cell Group: " << name <<" with comment: "<<comment<< endl;
	}
	return group;
}

void Mesh::renameGroup(const string& oldname, const string& newname, const string& comment) {
    if (oldname.empty()) {
        throw invalid_argument("Can't rename a group with empty oldname.");
    }
    if (newname.empty()) {
        throw invalid_argument("Can't rename a group with empty newname.");
    }
    const auto & it = this->groupByName.find(oldname);
    if (it == this->groupByName.end()) {
        throw invalid_argument("No group exists with this name: " + oldname);
    }

    shared_ptr<Group> group = it->second;
    group->name = newname;
    group->comment = comment;
    this->groupByName.erase(it);
    this->groupByName[newname] = group;


    if (this->logLevel >= LogLevel::DEBUG) {
        cout << "Renamed Cell Group: " << newname << endl;
    }
}

void Mesh::removeGroup(const string& name) {

    auto it = this->groupByName.find(name);
    if (it != this->groupByName.end()) {
        std::shared_ptr<Group> group = it->second;
        this->groupByName.erase(name);
        const int gId= group->getId();
        if (gId!= Group::NO_ORIGINAL_ID){
            this->groupById.erase(gId);
        }
        if (this->logLevel >= LogLevel::DEBUG) {
            cout << "Remove group: " << name << endl;
        }
    }
}

vector<shared_ptr<NodeGroup>> Mesh::getNodeGroups() const {
	vector<shared_ptr<NodeGroup>> groups;
	for (const auto& it : groupByName) {
		shared_ptr<Group> group = it.second;
		if (group->type != Group::NODEGROUP) {
			continue;
		}
		groups.push_back(dynamic_pointer_cast<NodeGroup>(group));
	}
	return groups;
}

vector<shared_ptr<CellGroup>> Mesh::getCellGroups() const {
	vector<shared_ptr<CellGroup>> groups;
	for (const auto& it : groupByName) {
		shared_ptr<Group> group = it.second;
		if (group->type != Group::CELLGROUP) {
			continue;
		}
		groups.push_back(dynamic_pointer_cast<CellGroup>(group));
	}
	return groups;
}

void Mesh::assignElementId(const CellContainer& cellContainer, int elementId) {
	for (int cellId : cellContainer.getCellIds(true)) {
		int cellPosition = findCellPosition(cellId);
		CellData& cellData = cells.cellDatas[cellPosition];
		cellData.elementId = elementId;
	}
}

shared_ptr<Group> Mesh::findGroup(string groupName) const {
	auto groupIterator = groupByName.find(groupName);
	if (groupIterator == groupByName.end())
		return nullptr;
	return groupIterator->second;
}

shared_ptr<Group> Mesh::findGroup(int originalId) const {
	auto groupIterator = groupById.find(originalId);
	if (groupIterator == groupById.end())
		return nullptr;
	return groupIterator->second;
}

int Mesh::countCells(const CellType& type) const {

	//if (type.code == CellType::POLYL.code) {
	//	//FIXME polylines not handled
	//	return 0;
	//}
	const vector<int>& positions = cellPositionsByType.find(type)->second;
	return static_cast<int>(positions.size());
}

bool Mesh::hasCell(int cellId) const {
	auto positionIterator =
			cells.cellpositionById.find(cellId);
	return positionIterator != cells.cellpositionById.end();
}

int Mesh::findCellPosition(int cellId) const {
	auto it = this->cells.cellpositionById.find(cellId);
	if (it == this->cells.cellpositionById.end()) {
		return Cell::UNAVAILABLE_CELL;
	}
	return it->second;
}

bool Mesh::validate() const {
	return nodes.validate();
}

} /* namespace vega */

