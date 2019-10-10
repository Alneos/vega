/*
 * Copyright (C) IRT Systemx (luca.dallolio@ext.irt-systemx.fr)
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
 * MedWriter.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: Luca Dall'Olio
 */

#include "MedWriter.h"
#include "../Abstract/Model.h"
#include <med.h>
#define MESGERR 1
#include <boost/filesystem.hpp>

namespace vega {
namespace aster {

using namespace std;

// Declaration to avoid Wmissing-declarations error
void createFamilies(med_idt fid, const char meshname[], const std::vector<Family>& families);

NodeGroup2Families::NodeGroup2Families(int nnodes, const vector<shared_ptr<NodeGroup>> nodeGroups) {
	int currentFamilyId = 0;
	unordered_map<int, int> newFamilyByOldfamily;
	unordered_map<int, Family> familyByFamilyId;
	if (nnodes > 0 && nodeGroups.size() > 0) {
		this->nodes.resize(nnodes, 0);
		for (const auto& nodeGroup : nodeGroups) {
			newFamilyByOldfamily.clear();
			for (int nodePosition : nodeGroup->nodePositions()) {
				int oldFamilyId = nodes[nodePosition];
				auto newFamilyPair = newFamilyByOldfamily.find(oldFamilyId);
				int newFamilyId;
				if (newFamilyPair == newFamilyByOldfamily.end()) {
					//family not found, create one
					currentFamilyId++;
                    newFamilyId = currentFamilyId;
					Family fam;
					fam.num = currentFamilyId;
					auto oldFamilyPair = familyByFamilyId.find(oldFamilyId);
					if (oldFamilyPair != familyByFamilyId.end()) {
						Family& oldFamily = oldFamilyPair->second;
						fam.groups.insert(fam.groups.begin(), oldFamily.groups.begin(),
								oldFamily.groups.end());
						fam.name = oldFamily.name + "_" + nodeGroup->getName();
						if (fam.name.length() >= MED_LNAME_SIZE) {
							fam.name = "Family" + to_string(currentFamilyId);
						}
					} else {
						fam.name = nodeGroup->getName();
					}
					fam.groups.push_back(nodeGroup);
					newFamilyByOldfamily.insert({oldFamilyId, currentFamilyId});
					familyByFamilyId.insert({currentFamilyId, fam});
				} else {
					newFamilyId = newFamilyPair->second;
				}
				nodes[nodePosition] = newFamilyId;
			}
		}
	}
	set<int> familiesInUse(nodes.begin(), nodes.end());
	for (int fam_id : familiesInUse) {
		if (fam_id != 0) {
			families.push_back(familyByFamilyId[fam_id]);
		}
	}
}

vector<Family> NodeGroup2Families::getFamilies() const {
	return this->families;
}

vector<int> NodeGroup2Families::getFamilyOnNodes() const {
	return this->nodes;
}

CellGroup2Families::CellGroup2Families(
		const Mesh& mesh, unordered_map<CellType::Code, int, EnumClassHash> cellCountByType,
		const vector<shared_ptr<CellGroup>>& cellGroups) : mesh(mesh) {
	int currentFamilyId = 0;
	unordered_map<int, int> newFamilyByOldfamily;
	unordered_map<int, Family> familyByFamilyId;
	for (const auto& cellCountByTypePair : cellCountByType) {
		shared_ptr<vector<int>> cells = make_shared<vector<int>>();
		cells->resize(cellCountByTypePair.second, 0);
		cellFamiliesByType[cellCountByTypePair.first] = cells;
	}

	for (const auto& cellGroup : cellGroups) {
		newFamilyByOldfamily.clear();
		for (const auto& cellPosition : cellGroup->cellPositions()) {
			const Cell&& cell = mesh.findCell(cellPosition);
			shared_ptr<vector<int>> currentCellFamilies = cellFamiliesByType[cell.type.code];
			int oldFamilyId = currentCellFamilies->at(cell.cellTypePosition);
			auto newFamilyPair = newFamilyByOldfamily.find(oldFamilyId);
			int newFamilyId;
			if (newFamilyPair == newFamilyByOldfamily.end()) {
				//family not found, create one
				currentFamilyId--;
                newFamilyId = currentFamilyId;
                Family fam;
                fam.num = currentFamilyId;
				auto oldFamilyPair = familyByFamilyId.find(oldFamilyId);
				if (oldFamilyPair != familyByFamilyId.end()) {
					Family& oldFamily = oldFamilyPair->second;
					fam.groups.insert(fam.groups.begin(), oldFamily.groups.begin(),
							oldFamily.groups.end());
					fam.name = oldFamily.name + "_" + cellGroup->getName();
					if (fam.name.length() >= MED_LNAME_SIZE) {
						fam.name = "CELLFamily" + to_string(-currentFamilyId);
					}
				} else {
					fam.name = cellGroup->getName();
				}
				fam.groups.push_back(cellGroup);
				newFamilyByOldfamily[oldFamilyId] = currentFamilyId;
				familyByFamilyId[currentFamilyId] = fam;
			} else {
				newFamilyId = newFamilyPair->second;
			}
			currentCellFamilies->at(cell.cellTypePosition) = newFamilyId;
		}
	}

	set<int> familiesInUse;
	for (const auto& cellFamilyAndTypePair : cellFamiliesByType) {
		familiesInUse.insert(cellFamilyAndTypePair.second->begin(),
				cellFamilyAndTypePair.second->end());
	}
	for (int fam_id : familiesInUse) {
		if (fam_id != 0) {
			families.push_back(familyByFamilyId[fam_id]);
		}
	}
}

vector<Family> CellGroup2Families::getFamilies() const {
	return this->families;
}

unordered_map<CellType::Code, shared_ptr<vector<int>>, EnumClassHash> CellGroup2Families::getFamilyOnCells() const {
	return this->cellFamiliesByType;
}

void createFamilies(med_idt fid, const char meshname[],
		const vector<Family>& families) {
	for (const auto& family : families) {
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

void MedWriter::writeMED(const Model& model, const string& medFileName) {
	//if (!finished) {
	//	this->finish();
	//}
	const char meshname[MED_NAME_SIZE + 1] = "3D unstructured mesh";
	const med_int spacedim = 3;
	const med_int meshdim = 3;
	const char axisname[3 * MED_SNAME_SIZE + 1] = "x               y               z               ";
	const char unitname[3 * MED_SNAME_SIZE + 1] = "m               m               m               ";
	const med_int nnodes = static_cast<med_int>(model.mesh.countNodes());
	if (model.configuration.logLevel >= LogLevel::DEBUG) {
		med_int v[3];
		MEDlibraryNumVersion(&v[0], &v[1], &v[2]);
		cout << "Med Version : " << v[0] << "." << v[1] << "." << v[2] << endl;
		cout << "Num nodes : " << nnodes << endl;
		cout << "Num cells : " << model.mesh.countCells() << endl;
	}

	/* open MED file */
	med_idt fid = MEDfileOpen(medFileName.c_str(), MED_ACC_CREAT);
	//cout << "FID : " << fid << endl;
	if (fid < 0) {
		throw logic_error("ERROR : MED file creation ... fid=" + to_string(fid));
	}

	if (MEDmeshCr(fid, meshname, spacedim, meshdim, MED_UNSTRUCTURED_MESH, model.mesh.getName().c_str(), "",
			MED_SORT_DTIT, MED_CARTESIAN, axisname, unitname) < 0) {
		throw logic_error("ERROR : Mesh creation ...");
	}
	vector<med_float> coordinates;
	coordinates.reserve(nnodes);
	for (const NodeData& nodeData : model.mesh.nodes.getNodeDatas()) {
	    if (nodeData.cpPos == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
            coordinates.push_back(nodeData.x);
            coordinates.push_back(nodeData.y);
            coordinates.push_back(nodeData.z);
	    } else {
	        shared_ptr<CoordinateSystem> coordSystem = model.mesh.getCoordinateSystemByPosition(nodeData.cpPos);
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

/*	char* nodeNames = new char[nnodes*MED_SNAME_SIZE]();

    for(int i=0; i<nnodes;i++){
        strncpy(nodeNames+(i*MED_SNAME_SIZE),Node::MedName(i).c_str(),MED_SNAME_SIZE);
    }
    // Problem: Le nom  existe déjà dans le répertoire de noms MAIL    .NOMNOE.
    MEDmeshEntityNameWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE,
    nnodes, nodeNames);
    delete[](nodeNames);*/

    vector<med_int> numnoe;
    numnoe.reserve(nnodes);
    for(med_int i=0; i<nnodes;i++){
        numnoe.push_back(i+1);
    }
    MEDmeshEntityNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE,nnodes, numnoe.data());
    numnoe.clear();

	for (const auto& kv : model.mesh.cellPositionsByType) {
		CellType type = kv.first;
		med_int code = static_cast<med_int>(type.code);
		vector<med_int> cellPositions = kv.second;
		med_int numCells = static_cast<med_int>(cellPositions.size());
		if (type.numNodes == 0 || numCells == 0) {
			continue;
		}
		vector<med_int> connectivity;
		connectivity.reserve(numCells * type.numNodes);
		for (med_int cellPosition : cellPositions) {
			const Cell&& cell = model.mesh.findCell(cellPosition);
			for (med_int nodePosition : cell.nodePositions) {
				// med nodes starts at node number 1.
				connectivity.push_back(nodePosition + 1);
			}
		}
		int result = MEDmeshElementConnectivityWr(fid, meshname, MED_NO_DT,
		MED_NO_IT, 0.0, MED_CELL, code, MED_NODAL, MED_FULL_INTERLACE,
				numCells,
				connectivity.data());
		if (result < 0) {
			throw logic_error("ERROR : writing cells ...");
		}
		connectivity.clear();

/*		char* cellNames = new char[numCells*MED_SNAME_SIZE]();
		//med_int* cellNum=new med_int[numCells];
		int i = 0;
        for (int cellPosition : cellPositions) {
            //int cellId = findCellId(cellPosition);
            //cellNum[i] = cellId;
            //cellNum[i] = cellPosition;
            strncpy(cellNames+(i*MED_SNAME_SIZE),Cell::MedName(cellPosition).c_str(),MED_SNAME_SIZE);
            cout << cellNames+(i*MED_SNAME_SIZE) << endl;
            ++i;
        }
        result = MEDmeshEntityNameWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL, code,
        numCells, cellNames);
        cout << "result:" << to_string(result) << endl;
        delete[](cellNames);
        //result = MEDmeshEntityNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL, code, numCells, cellNum);
        //delete[](cellNum);*/

        vector<med_int> cellnums;
        cellnums.reserve(numCells);
        for (int cellPosition : cellPositions) {
            cellnums.push_back(cellPosition+1);
        }
        MEDmeshEntityNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL, code, numCells, cellnums.data());
        cellnums.clear();

        if (result < 0) {
        throw logic_error("ERROR : writing cell names ...");
        }
	}

	if (MEDfamilyCr(fid, meshname, MED_NO_NAME, 0, 0, MED_NO_GROUP) < 0) {
		throw logic_error("ERROR : writing family 0 ...");
	}

	vector<shared_ptr<NodeGroup>> nodeGroups = model.mesh.getNodeGroups();
    // To avoid useless cast to type ‘int’ in some installations
	if (not nodeGroups.empty()) {
		NodeGroup2Families ng2fam(nnodes, nodeGroups);
		//WARN: if writing to file is delayed to MEDfileClose may be necessary
		//to move the allocation outside the if
		const auto& families = ng2fam.getFamilies();
		createFamilies(fid, meshname, families);
		//write family number for nodes
		if (MEDmeshEntityFamilyNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE,
				nnodes, ng2fam.getFamilyOnNodes().data()) < 0) {
			throw logic_error("ERROR : writing family on nodes ...");
		}
	}
	vector<shared_ptr<CellGroup>> cellGroups = model.mesh.getCellGroups();
	if (not cellGroups.empty()) {
		unordered_map<CellType::Code, int, EnumClassHash> cellCountByType;
		for (const auto& typeAndCodePair : CellType::typeByCode) {
			int cellNum = model.mesh.countCells(*typeAndCodePair.second);
			if (cellNum > 0) {
				cellCountByType[typeAndCodePair.first] = cellNum;
			}
		}
		CellGroup2Families cellGroup2Family = CellGroup2Families(model.mesh, cellCountByType, cellGroups);
		createFamilies(fid, meshname, cellGroup2Family.getFamilies());
		for (const auto& cellCodeFamilyVectorPair : cellGroup2Family.getFamilyOnCells()) {
			int ncells = static_cast<int>(cellCodeFamilyVectorPair.second->size());
			if (MEDmeshEntityFamilyNumberWr(fid, meshname, MED_NO_DT, MED_NO_IT, MED_CELL,
					static_cast<int>(cellCodeFamilyVectorPair.first), ncells, cellCodeFamilyVectorPair.second->data())
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
	if (model.configuration.logLevel >= LogLevel::DEBUG) {
		cout << "File created : " << fs::absolute(medFileName) << endl;
	}
}

} // namespace aster
} //namespace vega
