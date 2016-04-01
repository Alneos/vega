/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * SystusBuilder.cpp
 *
 *  Created on: 2 octobre 2013
 *      Author: devel
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "SystusWriter.h"
#include "build_properties.h"
#include "cmath" /* M_PI */

namespace fs = boost::filesystem;

namespace vega {
SystusWriter::SystusWriter() {
}

SystusWriter::~SystusWriter() {
}

const unordered_map<CellType::Code, vector<int>, hash<int>> SystusWriter::systus2medNodeConnectByCellType =
		{
				{ CellType::POINT1_CODE, { 0 } },
				{ CellType::SEG2_CODE, { 0, 1 } },
				{ CellType::SEG3_CODE, { 0, 2, 1 } },
				{ CellType::TRI3_CODE, { 0, 2, 1 } },
				{ CellType::TRI6_CODE, { 0, 5, 2, 4, 1, 3 } },
				{ CellType::QUAD4_CODE, { 0, 3, 2, 1 } },
				{ CellType::QUAD8_CODE, { 0, 7, 3, 6, 2, 5, 1, 4 } },
				{ CellType::TETRA4_CODE, { 0, 2, 1, 3 } },
				{ CellType::TETRA10_CODE, { 0, 6, 2, 5, 1, 4, 7, 9, 8, 3 } },
				{ CellType::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
				{ CellType::PENTA15_CODE, { 0, 8, 2, 7, 1, 6, 12, 14, 13, 3, 11, 5, 10, 4, 9 } },
				{ CellType::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
				{ CellType::HEXA20_CODE, { 0, 11, 3, 10, 2, 9, 1, 8, 16, 19, 18, 17, 4, 15, 7, 14,
						6, 13, 5, 12 } }
		};

string SystusWriter::writeModel(const shared_ptr<Model> model,
		const vega::ConfigurationParameters &configuration) {
	SystusModel systusModel = SystusModel(&(*model), configuration);
	//string currentOutFile = asterModel.getOutputFileName();
	cout << "Systus writer" << endl;

	string path = systusModel.configuration.outputPath;
	if (!fs::exists(path)) {
		throw iostream::failure("Directory " + path + " don't exist.");
	}

	ofstream dat_file_ofs;
	string dat_path = systusModel.getOutputFileName("_ALL.DAT");
	dat_file_ofs.open(dat_path.c_str(), ios::trunc);
	if (!dat_file_ofs.is_open()) {
		string message = string("Can't open file ") + dat_path + " for writing.";
		throw ios::failure(message);
	}

	/* Work to Do Only once */
	getSystusInformations(systusModel);
	generateRBEs(systusModel);


	for (auto it : systusModel.model->analyses) {
		const Analysis& analysis = *it;

		/* ASCI file */
		string asc_path = systusModel.getOutputFileName("_SC" + to_string(analysis.getId())+ "_DATA1.ASC");
		ofstream asc_file_ofs;
		asc_file_ofs.precision(DBL_DIG);
		asc_file_ofs.open(asc_path.c_str(), ios::trunc | ios::out);
		if (!asc_file_ofs.is_open()) {
			string message = string("Can't open file ") + asc_path + " for writing.";
			throw ios::failure(message);
		}
		this->writeAsc(systusModel, analysis, asc_file_ofs);
		asc_file_ofs.close();

		/* Analysis file */
		ofstream analyse_file_ofs;
		analyse_file_ofs.precision(DBL_DIG);
		string analyse_path = systusModel.getOutputFileName("_SC" + to_string(analysis.getId()) + ".DAT");
		analyse_file_ofs.open(analyse_path.c_str(), ios::trunc);

		if (!analyse_file_ofs.is_open()) {
			string message = string("Can't open file ") + analyse_path + " for writing.";
			throw ios::failure(message);
		}
		this->writeDat(systusModel, analysis, analyse_file_ofs);
		analyse_file_ofs.close();

		dat_file_ofs << "READ " << systusModel.getName() << "_SC" << analysis.getId() << ".DAT" << endl;
	}
	dat_file_ofs.close();

	return dat_path;
}

void SystusWriter::getSystusInformations(const SystusModel& systusModel) {

	CellType cellType[20] = { CellType::POINT1, CellType::SEG2, CellType::SEG3, CellType::SEG4,
			CellType::TRI3, CellType::QUAD4, CellType::TRI6, CellType::TRI7, CellType::QUAD8,
			CellType::QUAD9, CellType::TETRA4, CellType::PYRA5, CellType::PENTA6, CellType::HEXA8,
			CellType::TETRA10, CellType::HEXGP12, CellType::PYRA13, CellType::PENTA15,
			CellType::HEXA20, CellType::HEXA27 };

	bool hasElement[20];
	int numNodes[20];
	for (int i = 0; i < 20; i++) {
		hasElement[i] = !!systusModel.model->mesh->countCells(cellType[i]);
		numNodes[i] = hasElement[i] ? cellType[i].numNodes : 0;
	}

	bool has1DOr2DElements = false;
	for (int i = 0; i < 10; i++)
		has1DOr2DElements = hasElement[i] || has1DOr2DElements;

	if (has1DOr2DElements)
		systusOption = 3;
	else
		systusOption = 4;

	maxNumNodes = *max_element(numNodes, numNodes + 20);

	nbNodes = systusModel.model->mesh->countNodes();

}

void SystusWriter::generateRBEs(const SystusModel& systusModel) {

	shared_ptr<Mesh> mesh = systusModel.model->mesh;
	vector<shared_ptr<ConstraintSet>> commonConstraintSets = systusModel.model->getCommonConstraintSets();
	for (auto constraintSet : commonConstraintSets) {
		set<shared_ptr<Constraint>> constraints = constraintSet->getConstraintsByType(Constraint::RIGID);
		for (auto constraint : constraints) {
			std::shared_ptr<RigidConstraint> rbe2 = std::static_pointer_cast<RigidConstraint>(constraint);

			CellGroup* group = mesh->createCellGroup("RBE2_"+std::to_string(constraint->getId()));

			Node master = mesh->findNode(rbe2->getMaster());
			int master_rot_id = 0;
			if (systusOption == 4){
				int master_rot_position = mesh->addNode(Node::AUTO_ID, master.x, master.y, master.z, master.displacementCS);
				master_rot_id = mesh->findNode(master_rot_position).id;
			}

			for (int position : rbe2->getSlaves()){
				Node slave = mesh->findNode(position);
				int slave_lagr_position = mesh->addNode(Node::AUTO_ID, slave.x, slave.y, slave.z, slave.displacementCS);
				int slave_lagr_id = mesh->findNode(slave_lagr_position).id;
				vector<int> nodes = {master.id, slave.id};
				if (systusOption == 4)
					nodes.push_back(master_rot_id);
				nodes.push_back(slave_lagr_id);
				int cellPosition;
				if (systusOption == 3)
					cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG3, nodes, true);
				else if (systusOption == 4)
					cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG4, nodes, true);
				RBE2rbarPositions.push_back(cellPosition);
				group->addCell(mesh->findCell(cellPosition).id);
			}
		}
		constraints = constraintSet->getConstraintsByType(Constraint::RBE3);
		for (auto constraint : constraints) {
			std::shared_ptr<RBE3> rbe3 = std::static_pointer_cast<RBE3>(constraint);

			CellGroup* group = mesh->createCellGroup("RBE3_"+std::to_string(constraint->getId()));

			Node master = mesh->findNode(rbe3->getMaster());
			for (int position : rbe3->getSlaves()){
				Node slave = mesh->findNode(position);
				vector<int> nodes = {master.id, slave.id};
				int cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
				RBE3rbarPositions.push_back(cellPosition);
				group->addCell(mesh->findCell(cellPosition).id);
			}
		}
	}

}

// Select the Loads of the current analysis and give them a local Systus number.
void SystusWriter::fillLoads(const SystusModel& systusModel, const Analysis& analysis){

	int idSystusLoad=1;

	localLoadingListIdByLoadingListId.clear();

	vector<shared_ptr<LoadSet>> analysisLoadSets = analysis.getLoadSets();
	for (auto loadSet : analysisLoadSets) {
		localLoadingListIdByLoadingListId[loadSet->getId()]= idSystusLoad;
		idSystusLoad++;
	}


	// We discarded this part, as it creates a useless loadcase
	// It's a "wrong" translation but it does not create problems
	/*for (auto constraintSet : commonConstraintSets) {
			out << idSystusLoad << " \"CONSTRAINTSET_"<< constraintSet->getId() << "\" ";
			out << "0 0 0 0 0 0 0 7" << endl;
			idSystusLoad++;
		}*/
	// TODO : Check if all loadings are taken into account
	// TODO : What are GRAVITY Loading
	/*
		 for (auto loading : systusModel.model->loadings){
		 switch (loading->type) {
		 case Loading::GRAVITY:{
		 shared_ptr<Gravity> gravity = static_pointer_cast<Gravity>(loading);
		 out << systusModel.model->loadings.size() + loading->id << " \"LOADING_" <<  loading->id << "\" ";
		 out << "0 " << loading->id << " 0 0 0 0 0 7" << endl;
		 break;
		 }
		 default:{
		 // Nothing to do for other cases
		 }
		 }
		 }
	 */
}

// Fill the vectors field with Vectors relative to Loadings and Castings
//TODO: add all vectors in this function
void SystusWriter::fillVectors(const SystusModel& systusModel, const Analysis& analysis){

	int vectorId=1;

	// Cleaning from previous analysis
	vectors.clear();
	localVectorIdByLoadingListId.clear();
	localVectorIdByConstraintListId.clear();

	// Add Loadcase Loading Vectors
	for (auto loadset : analysis.getLoadSets()){
		for (auto loading : loadset->getLoadings()) {
			vector<double> vec;

			switch (loading->type) {
			case Loading::NODAL_FORCE: {
				shared_ptr<NodalForce> nodalForce = static_pointer_cast<NodalForce>(loading);
				VectorialValue force = nodalForce->getForce();
				VectorialValue moment = nodalForce->getMoment();

				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(force.x());
				vec.push_back(force.y());
				vec.push_back(force.z());
				vec.push_back(moment.x());
				vec.push_back(moment.y());
				vec.push_back(moment.z());

				vectors[vectorId]=vec;
				localVectorIdByLoadingListId[loading->getId()]=vectorId;
				vectorId++;
				break;
			}
			case Loading::GRAVITY: {
				shared_ptr<Gravity> gravity = static_pointer_cast<Gravity>(loading);
				VectorialValue acceleration = gravity->getAccelerationVector();

				vec.push_back(1);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(0);
				vec.push_back(acceleration.x());
				vec.push_back(acceleration.y());
				vec.push_back(acceleration.z());

				vectors[vectorId]=vec;
				localVectorIdByLoadingListId[loading->getId()]=vectorId;
				vectorId++;
				break;
			}
			case Loading::DYNAMIC_EXCITATION:{
				// Nothing to be done here
				break;
			}
			default: {
				//TODO : throw WriterException("Loading type not supported");
				cout << "Warning : " << *loading << " not supported" << endl;
			}
			}
		}
	}


	// Add Loadcase Constraint Vectors
	// TODO: Add Subcase Constraint Vectors
	for (auto constraintset : analysis.getConstraintSets()){
		for (auto constraint : constraintset->getConstraints()) {
			vector<double> vec;

			switch (constraint->type) {
			case Constraint::SPC: {
				std::shared_ptr<SinglePointConstraint> spc = std::static_pointer_cast<SinglePointConstraint>(constraint);
				//TODO: Null vectors should be overlooked
				if (spc->hasReferences()) {
					cout << "Warning : " << *constraint << " SPC with references to functions not supported" << endl;
				} else {
					vec.push_back(4);
					vec.push_back(0);
					vec.push_back(0);
					vec.push_back(0);
					vec.push_back(0);
					vec.push_back(0);
					DOFS spcDOFS = spc->getDOFSForNode(0);
					for (DOF dof : DOFS::ALL_DOFS) {
						vec.push_back( (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0) );
					}
					vectors[vectorId]=vec;
					localVectorIdByConstraintListId[constraint->getId()]=vectorId;
					vectorId++;
				}
				break;
			}
			case Constraint::RIGID:
			case Constraint::RBE3:{
				// Nothing to be done here
				break;
			}
			default: {
				//TODO : throw WriterException("Constraint type not supported");
				cout << "Warning : " << *constraint << " not supported" << endl;
			}
			}
		}
	}

}


void SystusWriter::fillConstraintLists(const std::shared_ptr<ConstraintSet> & constraintSet, std::map<int, std::map<int, int>> & localVectorsByLocalLoadingByNodePosition){

	char dofCode;
	if (systusOption == 3)
		dofCode = (char) DOFS::ALL_DOFS;
	else if (systusOption == 4)
		dofCode = (char) DOFS::TRANSLATIONS;
	else
		throw WriterException("systusOption not supported");


	for (auto constraint : constraintSet->getConstraints()) {

		switch (constraint->type) {
		case Constraint::SPC: {
			std::shared_ptr<SinglePointConstraint> spc = std::static_pointer_cast<
					SinglePointConstraint>(constraint);
			for (int nodePosition : constraint->nodePositions()) {

				// TODO: We suppose here that the ConstrainSet have the same numbering as its corresponding LoadCase. Seems dangerous.
				int localLoadingId = localLoadingListIdByLoadingListId[constraintSet->getId()];
				int localVectorId  = localVectorIdByConstraintListId[constraint->getId()];

				localVectorsByLocalLoadingByNodePosition[nodePosition][localLoadingId]=localVectorId;

				// We compute the Degree Of Freedom of the node (see ASC Manual)
				DOFS constrained = constraint->getDOFSForNode(nodePosition);
				if (constraintByNodePosition.find(nodePosition) == constraintByNodePosition.end())
					constraintByNodePosition[nodePosition] = char(constrained) & dofCode;
				else
					constraintByNodePosition[nodePosition] = (char(constrained) & dofCode)
					| constraintByNodePosition[nodePosition];
			}
			break;
		}
		case Constraint::RIGID:
		case Constraint::RBE3:{
			// Nothing to be done here
			break;
		}
		default: {
			//cout << typeid(*constraint).name() << endl;
			//TODO : throw WriterException("Constraint type not supported");
			cout << "Warning : " << *constraint << " not supported" << endl;
		}
		}
	}
}



void SystusWriter::fillLists(const SystusModel& systusModel, const Analysis& analysis) {

	// Cleaning from previous analysis
	lists.clear();
	loadingListIdByNodePosition.clear();
	constraintListIdByNodePosition.clear();
	constraintByNodePosition.clear();

	// Filling lists for Loadings
	// Format is (indice of list, local loading number, local vector)
	//TODO: here we take here the indice of the node, but it may not be a good idea
	map<int, map<int, int>> localVectorsByLocalLoadingByNodePosition;
	for (auto loadSet : analysis.getLoadSets()) {
		set<shared_ptr<Loading>> loadings = loadSet->getLoadings();
		for (auto loading : loadings) {
			switch (loading->type) {
			case Loading::NODAL_FORCE: {
				shared_ptr<NodalForce> nodalForce = static_pointer_cast<NodalForce>(loading);
				int node = nodalForce->getNode().position;
				int localLoadingId = localLoadingListIdByLoadingListId[loadSet->getId()];
				int localVectorId  = localVectorIdByLoadingListId[loading->getId()];

				localVectorsByLocalLoadingByNodePosition[node][localLoadingId]=localVectorId;
				break;
			}
			case Loading::GRAVITY: {
				// Nothing to be done here
				break;
			}
			case Loading::ROTATION: {
				// Nothing to be done here
				break;
			}
			default:
				throw WriterException("Loading type not supported");
			}
		}
	}

	for (auto it : localVectorsByLocalLoadingByNodePosition) {
		List list(it.second);
		lists.push_back(list);
		loadingListIdByNodePosition[it.first] = list.getId();
	}




	// Filling lists for Common (Subcase?) Constraints
	//TODO: This is still buggy, as I don't understand what VEGA put in "common" constraints
	localVectorsByLocalLoadingByNodePosition.clear();
    //vector<shared_ptr<ConstraintSet>> commonConstraintSets = systusModel.model->getCommonConstraintSets();
    //for (auto constraintSet : commonConstraintSets){
	//		cout << "Filling List for Common ConstraintSet "<<constraintSet->getId()<<" of size "<< constraintSet->size()<< endl;
	//		fillConstraintLists(constraintSet, localVectorsByLocalLoadingByNodePosition);
	//	}

	// Filling lists for Loadcase Constraints
	for (auto constraintSet : analysis.getConstraintSets()){
		//cout << "Filling List for ConstraintSet "<<constraintSet->getId()<<" of size "<< constraintSet->size()<< endl;
		fillConstraintLists(constraintSet, localVectorsByLocalLoadingByNodePosition);
	}

	constraintListIdByNodePosition.clear();
	for (auto it : localVectorsByLocalLoadingByNodePosition) {
		List list(it.second);
		lists.push_back(list);
		constraintListIdByNodePosition[it.first] = list.getId();
	}
}


void SystusWriter::writeAsc(const SystusModel &systusModel, const Analysis& analysis, ostream& out) {

	fillLoads(systusModel, analysis);

	fillVectors(systusModel, analysis);

	fillLists(systusModel, analysis);

	writeHeader(systusModel, out);

	writeInformations(systusModel, out);

	writeNodes(systusModel, out);

	writeElements(systusModel, out);

	writeGroups(systusModel, out);

	writeMaterials(systusModel, out);

	out << "BEGIN_MEDIA 0" << endl;
	out << "END_MEDIA" << endl;

	writeLoads(systusModel, analysis, out);

	writeLists(systusModel, out);

	writeVectors(systusModel, analysis, out);

	out << "BEGIN_RELEASES 0" << endl;
	out << "END_RELEASES" << endl;

	out << "BEGIN_TABLES 0" << endl;
	out << "END_TABLES" << endl;

	out << "BEGIN_TEMPERATURES 0 11" << endl;
	out << "END_TEMPERATURES" << endl;

	out << "BEGIN_VELOCITIES 0 11" << endl;
	out << "END_VELOCITIES" << endl;

	writeMasses(systusModel, out);

	out << "BEGIN_DAMPINGS 0" << endl;
	out << "END_DAMPINGS" << endl;

	out << "BEGIN_RELATIONS 0" << endl;
	out << "END_RELATIONS" << endl;

	out << "BEGIN_PULSATIONS 0" << endl;
	out << "END_PULSATIONS" << endl;

	out << "BEGIN_SECTIONS 0" << endl;
	out << "END_SECTIONS" << endl;

	out << "BEGIN_COMPOSITES 0" << endl;
	out << "END_COMPOSITES" << endl;

	out << "BEGIN_AFFECTATIONS 0" << endl;
	out << "END_AFFECTATIONS" << endl;
}

void SystusWriter::writeHeader(const SystusModel& systusModel, ostream& out) {
	out << "1VSD 0 121126 133214 121126 133214 " << endl;
	out << systusModel.getName().substr(0, 20) << endl; //should be less than 24
	out << " 100000 " << systusOption << " " << systusModel.model->mesh->countNodes() << " ";
	out << systusModel.model->mesh->countCells() << " " << systusModel.model->loadSets.size()
			<< " ";

	// TODO : wrong if a model possess bar and volume elements only, maybe check the model
	int numberOfDof = numberOfDofBySystusOption[systusOption];
	out << numberOfDof << " " << numberOfDof * maxNumNodes << " 0 0" << endl;

}

void SystusWriter::writeInformations(const SystusModel& systusModel, ostream& out) {
	out << "BEGIN_INFORMATIONS" << endl;
	out << systusModel.getName().substr(0, 80) << endl; //should be less than 80
	out << " " << systusOption << " 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0" << endl;
	out << " 0 0 0 0 0 0 0 0 0 0 ";

	int numberOfDof = numberOfDofBySystusOption[systusOption];
	out << numberOfDof << " " << numberOfDof << " " << numberOfDof * numberOfDof << " 0 0 0 0 "
			<< numberOfDof * numberOfDof;
	out << " 0 0 0 0 12 0 0 0 0 0 3 0 0 0 0 0 0 0 2 2 0 0" << endl;
	out << "END_INFORMATIONS" << endl;
}

void SystusWriter::writeNodes(const SystusModel& systusModel, ostream& out) {
	const shared_ptr<Mesh> mesh = systusModel.model->mesh;

	out << "BEGIN_NODES ";
	out << mesh->countNodes();
	out << " 3" << endl; // number of coordinates

	for (auto node : mesh->nodes) {
		int nid = node.position + 1;
		int iconst = 0;
		auto it = constraintByNodePosition.find(node.position);
		if (it != constraintByNodePosition.end())
			iconst = int(it->second);
		int imeca = 0;
		int iangl = 0;
		if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID)
			iangl = Constraint::lastAutoId() + Loading::lastAutoId()
				+systusModel.model->find(Reference<CoordinateSystem>(CoordinateSystem::UNKNOWN, node.displacementCS))->getId();
		int isol = 0;
		auto it2 = loadingListIdByNodePosition.find(node.position);
		if (it2 != loadingListIdByNodePosition.end())
			isol = it2->second;
		int idisp = 0;
		it2 = constraintListIdByNodePosition.find(node.position);
		if (it2 != constraintListIdByNodePosition.end())
			idisp = it2->second;
		out << nid << " " << iconst << " " << imeca << " " << iangl << " " << isol << " " << idisp
				<< " ";
		out << node.x << " " << node.y << " " << node.z << endl;
	}

	out << "END_NODES" << endl;
}

void SystusWriter::writeElements(const SystusModel& systusModel, ostream& out) {
	shared_ptr<Mesh> mesh = systusModel.model->mesh;
	out << "BEGIN_ELEMENTS " << mesh->countCells() << endl;
	for (auto elementSet : systusModel.model->elementSets) {
		//if (elementSet->getElementType() == ElementSet::ELEMENT_UNDEFINED || elementSet->cellGroup == nullptr)
		//	continue;
		CellGroup* cellGroup = elementSet->cellGroup;
		//const Material* material = element->material;
		int dim = 0;
		switch (elementSet->type) {
		case ElementSet::CIRCULAR_SECTION_BEAM:
			case ElementSet::GENERIC_SECTION_BEAM:
			case ElementSet::I_SECTION_BEAM:
			case ElementSet::RECTANGULAR_BEAM: {
			dim = 1;
			break;
		}
		case ElementSet::SHELL: {
			dim = 2;
			break;
		}
		case ElementSet::CONTINUUM: {
			dim = 3;
			break;
		}
		case ElementSet::NODAL_MASS:
			case ElementSet::DISCRETE_0D:
			case ElementSet::DISCRETE_1D: {
			continue;
		}
		default: {
			//TODO : throw WriterException("ElementSet type not supported");
			cout << "Warning : " << *elementSet << " not supported" << endl;
		}
		}
		for (const Cell& cell : cellGroup->getCells()) {
			auto systus2med_it = systus2medNodeConnectByCellType.find(cell.type.code);
			if (systus2med_it == systus2medNodeConnectByCellType.end()) {
				cout << "Warning : " << cell << " not supported in Systus" << endl;
				continue;
			}
			out << cell.id << " " << dim << 0 << setfill('0') << setw(2)
					<< cell.nodeIds.size();
			//out << " " << material->getId() << " " << 0 << " " << 0 << " ";
			out << " " << elementSet->getId() << " 0 0";
			vector<int> systus2medNodeConnect = systus2med_it->second;
			vector<int> medConnect = cell.nodeIds;
			vector<int> systusConnect;
			for (unsigned int i = 0; i < cell.type.numNodes; i++)
				systusConnect.push_back(medConnect[systus2medNodeConnect[i]]);
			for (int node : systusConnect) {
				out << " " << mesh->findNodePosition(node) + 1;
			}
			out << endl;
		}
	}
	// adding rbars elements corresponding to rbe2 and rbe3
	if (RBE2rbarPositions.size()){
		RBE2rbarsElementId = ElementSet::lastAutoId() + 1;
		for (int position : RBE2rbarPositions){
			Cell cell = mesh->findCell(position);
			out << cell.id << " 190" << cell.nodeIds.size() << " " << RBE2rbarsElementId << " 0 0";
			for (int nodeId : cell.nodeIds)
				out << " " << mesh->findNodePosition(nodeId) + 1;
			out << endl;
		}
	}
	if (RBE3rbarPositions.size()){
		RBE3rbarsElementId = ElementSet::lastAutoId() + 2;
		for (int position : RBE3rbarPositions){
			Cell cell = mesh->findCell(position);
			out << cell.id << " 100" << cell.nodeIds.size() << " " << RBE3rbarsElementId << " 0 0";
			for (int nodeId : cell.nodeIds)
				out << " " << mesh->findNodePosition(nodeId) + 1;
			out << endl;
		}
	}

	out << "END_ELEMENTS" << endl;
}

void SystusWriter::writeGroups(const SystusModel& systusModel, ostream& out) {
	vector<NodeGroup*> nodeGroups = systusModel.model->mesh->getNodeGroups();
	vector<CellGroup*> cellGroups = systusModel.model->mesh->getCellGroups();

	out << "BEGIN_GROUPS ";
	out << cellGroups.size() + nodeGroups.size() << endl;
	for (auto cellGroup : cellGroups) {
		//1 E2D 2 0 "SYST DIMENSION 2" "" "Comments of the group" 101 102 103 201 202 203 301 302 303
		out << cellGroup->getId() << " " << cellGroup->getName()
				<< " 2 0 \"No method\" \"\" \"No Comments\"";
		for (auto cell : cellGroup->getCells())
			out << " " << cell.id;
		out << endl;
	}

	for (auto nodeGroup : nodeGroups) {
		out << nodeGroup->getId() << " " << nodeGroup->getName()
				<< " 1 0 \"No method\" \"\" \"No Comments\"";
		for (int id : nodeGroup->nodePositions())
			out << " " << id;
		out << endl;
	}
	out << "END_GROUPS" << endl;
}

void SystusWriter::writeMaterials(const SystusModel& systusModel, ostream& out) {

	ostringstream omat;
	omat.precision(DBL_DIG);
	int nbmaterials= 0;
	int nbelements = 0;

    for (auto elementSet : systusModel.model->elementSets) {
		auto material = elementSet->material;
		if (material != nullptr && elementSet->cellGroup != nullptr) {
			const shared_ptr<Nature> nature = material->findNature(Nature::NATURE_ELASTIC);
			if (nature) {
				const ElasticNature& elasticNature = dynamic_cast<ElasticNature&>(*nature);
				omat << elementSet->getId() << " 0 ";
				if (elasticNature.getRho()>0.0){
					omat << "4 " << elasticNature.getRho() << " ";
					nbelements++;
				}
				if (elasticNature.getE()>0.0){
					omat << "5 " << elasticNature.getE() << " ";
					nbelements++;
				}
				if (elasticNature.getNu()>0.0){
					omat << "6 " << elasticNature.getNu() << " ";
					nbelements++;
				}
				switch (elementSet->type) {
				case (ElementSet::GENERIC_SECTION_BEAM): {
					shared_ptr<const GenericSectionBeam> genericBeam = static_pointer_cast<
							const GenericSectionBeam>(elementSet);
					omat << "11 " << genericBeam->getAreaCrossSection() << " ";
					omat << "12 " << genericBeam->getShearAreaFactorY() << " ";
					omat << "13 " << genericBeam->getShearAreaFactorZ() << " ";
					omat << "14 " << genericBeam->getTorsionalConstant() << " ";
					omat << "15 " << genericBeam->getMomentOfInertiaY() << " ";
					omat << "16 " << genericBeam->getMomentOfInertiaZ() << " ";
					nbelements=nbelements+6;
					break;
				}
				case (ElementSet::CIRCULAR_SECTION_BEAM): {
					shared_ptr<const CircularSectionBeam> circularBeam = static_pointer_cast<
							const CircularSectionBeam>(elementSet);
					omat << "11 " << circularBeam->getAreaCrossSection() << " ";
					nbelements++;
					break;
				}

				case (ElementSet::SHELL): {
					shared_ptr<const Shell> shell = static_pointer_cast<const Shell>(elementSet);
					omat << "21 " << shell->thickness << " ";
					nbelements++;
					break;
				}
				case (ElementSet::CONTINUUM): {
					break;
				}
				default:
					cout << "Warning : " << *elementSet << " not supported" << endl;
				}
				nbmaterials++;
				omat << endl;
			}
		}
	}
	// adding rbars materials for rbe2s and rbe3s
	if (RBE2rbarPositions.size()){
		omat << RBE2rbarsElementId << " 0 200 9 61 19 197 1 5 1" << endl;
		nbmaterials++;
		nbelements=nbelements+4;
	}
	if (RBE3rbarPositions.size()){
		cout << "Warning : RBE3 material emulated by beam with low rigidity" << endl;
		omat << RBE3rbarsElementId << " 0 4 0 5 1e-12 6 0 11 3.14159265358979e-06" << endl;
		nbmaterials++;
		nbelements=nbelements+4;
	}

	// Stream to output
	out << "BEGIN_MATERIALS " << nbmaterials << " " << nbelements << endl;
	out << omat.str();
	out << "END_MATERIALS" << endl;
}

void SystusWriter::writeLoads(const SystusModel& systusModel, const Analysis & analysis, ostream& out) {

	out << "BEGIN_LOADS ";

	// Number of written loads
	out << localLoadingListIdByLoadingListId.size() << endl;

	// Writing Loads
	for (auto load : localLoadingListIdByLoadingListId) {
		out << load.second << " \"LOADSET_" << load.first << "\" ";
		out << "0 0 0 0 0 0 0 7" << endl;
	}

	out << "END_LOADS" << endl;

}

void SystusWriter::writeLists(const SystusModel& systusModel, ostream& out) {
	UNUSEDV(systusModel);
	out << "BEGIN_LISTS ";
	out << lists.size() << " " <<2*lists.size() << endl;
	for (auto list : lists) {
		list.write(out);
	}
	out << "END_LISTS" << endl;
}

void SystusWriter::writeVectors(const SystusModel& systusModel, const Analysis & analysis, ostream& out) {

	out << "BEGIN_VECTORS "
			<< vectors.size() + systusModel.model->coordinateSystems.size() << endl;

	// Writing vectors from Loads
	for (auto vector : vectors) {
		out << vector.first;
		for (auto d : vector.second)
			out << " " << d;
		out << endl;
	}

	//TODO: transfer the construction of these vectors to the fillVectors function
	for (auto coordinateSystem : systusModel.model->coordinateSystems) {
		VectorialValue angles = coordinateSystem->getEulerAnglesIntrinsicZYX();
		out << vectors.size() + Constraint::lastAutoId() + coordinateSystem->getId()
				<< " 0 0 0 0 0 0 ";
		out << angles.x() << " " << angles.y() << " " << angles.z() << endl;
	}

	out << "END_VECTORS" << endl;
}

void SystusWriter::writeMasses(const SystusModel &systusModel, ostream& out) {
	// TODO : attention, la doc parle de dynamique. Prise en compte dans le poids en statique ???
	vector<shared_ptr<ElementSet>> masses = systusModel.model->filterElements(
			ElementSet::NODAL_MASS);
	out << "BEGIN_MASSES " << masses.size() << endl;
	if (masses.size() > 0) {
		for (auto mass : masses) {
			if (mass->cellGroup != nullptr) {
				shared_ptr<NodalMass> nodalMass = static_pointer_cast<NodalMass>(mass);
				//	VALUES NBR VAL(NBR) NODEi
				//	NBR:			Number of values [INTEGER]
				//  VAL(NBR):	Masses values [DOUBLE[NBR]]
				//	NODEi:		List of NODES index [INTEGER[*]]
				out << "VALUES 6 " << nodalMass->getMass() << " " << nodalMass->getMass() << " "
						<< nodalMass->getMass() << " " << nodalMass->ixx << " " << nodalMass->iyy
						<< " " << nodalMass->izz;
				for (auto cell : mass->cellGroup->getCells()) {
					// NODEi
					out << " " << cell.nodeIds[0];
				}
				out << endl;
			}
		}
	}
	out << "END_MASSES" << endl;
}

void SystusWriter::writeDat(const SystusModel& systusModel, const Analysis& analysis,
		ostream& out) {

	out << "NAME " << systusModel.getName() << "_SC" << analysis.getId() << "_" << endl;
	out << endl;
	out << "SEARCH DATA 1 ASCII" << endl;
	out << endl;

	switch (analysis.type) {
	case Analysis::LINEAR_MECA_STAT: {

		out << "SOLVE METHOD OPTIMISED" << endl;
		break;
	}
	case Analysis::LINEAR_MODAL:
	case Analysis::LINEAR_DYNA_MODAL_FREQ: {
		out << "# SOLVE FILE TO USE THE DYNAMIC SOLVER" << endl;
		out << "# USE FOR EIGEN FREQUENCY CRITERION" << endl;
		out << endl;
		out << "# COMPUTING MASS MATRIX" << endl;
		out << "# AS THE COMMAND DYNAMIC COMPUTE THEM, IT SHOULD BE USELESS." << endl;
		out << "# BUT THERE SEEM TO BE BUGS ON THE COMMAND, SO WE USE EXPLICITLY THE COMMAND" << endl;
		out << "SOLVE STIFFNESS MASS" << endl;
		out << endl;
		out << "# COMPUTE MODES" << endl;
		out << "DYNAMIC" << endl;
		out << endl;

		const LinearModal& linearModal = static_cast<const LinearModal&>(analysis);
		FrequencyBand& frequencyBand = *(linearModal.getFrequencyBand());
		string smodes= (frequencyBand.num_max == vega::Globals::UNAVAILABLE_INT ? "":to_string(frequencyBand.num_max));
		string siters= (frequencyBand.num_max == vega::Globals::UNAVAILABLE_INT ? "":to_string(2*frequencyBand.num_max));
		out << "# WE COMPUTE "<< smodes << " MODES." << endl;
		out << "# IT'S AN ITERATIVE MEHOD, WITH A MAXIMUM OF "<< siters <<" ITERATIONS" << endl;
		out << "MODE SUBSPACE " << (is_equal(frequencyBand.upper, vega::Globals::UNAVAILABLE_DOUBLE) ? "BLOCK 6":"BAND") << endl;
		out << "METHOD OPTIMIZED" << endl;
		out << "VECTOR "<< smodes <<" ITER "<< siters <<" EPSILON 1*-5";
		if (!is_equal(frequencyBand.upper, vega::Globals::UNAVAILABLE_DOUBLE))
			out << " STURM FREQ " << frequencyBand.upper;
		else
			out << " NORM MASS";
		out << endl;
		if (!is_equal(frequencyBand.lower, vega::Globals::UNAVAILABLE_DOUBLE) && !is_equal(frequencyBand.lower, 0))
			cout << "Warning : lower frequencyBand not supported" << endl;
		out << "RETURN" << endl;

		out << endl;
		out << "# COMPUTE THE 'TENSEUR DE DÉFORMATION ET DE CONTRAINTES' (IN FRENCH IN THE TEXT ;)" << endl;
		out << "# MANDATORY TO COMPUTE THE GRADIENTS OF THE FREQUENCY CRITERIONS." << endl;
		out << "SOLVE FORCE" << endl;

		//TODO: Disable for now. Is it useful?
		//		if (analysis.type == Analysis::LINEAR_DYNA_MODAL_FREQ){
		//
		//			const LinearDynaModalFreq& linearDynaModalFreq = static_cast<const LinearDynaModalFreq&>(analysis);
		//
		//			out << "DYNAMIC" << endl;
		//			out << "PARTICIPATION DOUBLE FORCE DISPL" << endl;
		//			out << "RETURN" << endl;
		//			out << "SOLVE FORCE MODAL" << endl << endl;
		//
		//			out << "DYNAMIC" << endl;
		//			out << "HARMONIC RESPONSE MODAL FORCE DISPL" << endl;
		//
		//			shared_ptr<StepRange> freqValueSteps = linearDynaModalFreq.getFrequencyValues()->getStepRange();
		//			out << "FREQUENCY INITIAL " << freqValueSteps->start - freqValueSteps->step << endl;
		//			out << " " << freqValueSteps->end << " STEP " << freqValueSteps->step << endl;
		//
		//			shared_ptr<FunctionTable> modalDampingTable = linearDynaModalFreq.getModalDamping()->getFunctionTable();
		//			out << "DAMPING MODAL" << endl;
		//			for (auto it = modalDampingTable->getBeginValuesXY(); it != modalDampingTable->getEndValuesXY(); it++){
		//				out << " " << it->first << " / (GAMMA) " << it->second << endl;
		//				cout << "modal damping must be defined by {mode num}/value and not frequence/value " << endl;
		//			}
		//			out << endl;
		//
		//			for (auto it : DynamicExcitationByLoadId){
		//				cout << "phase and amplitude not taken into account for load " << it.first << endl;
		//				it.second->getDynaPhase();
		//				it.second->getFunctionTableB();
		//			}
		//
		//			out << "TRANSFER STATIONARY" << endl;
		//			out << "DISPLACEMENT" << endl;
		//			out << "RETURN" << endl << endl;
		//
		//			string("Analysis " + Analysis::stringByType.at(analysis.type) + " not (finish) implemented");
		//		}
		//		out << "SAVE DATA RESU " << analysis.getId() << endl;
		//		out << "CONVERT RESU" << endl;
		//		out << "POST " << analysis.getId() << endl;
		//		out << "RETURN" << endl << endl;
		break;
	}
	default:
		throw WriterException(
				string("Analysis " + Analysis::stringByType.at(analysis.type) + " not (yet) implemented"));
	}

    // We Save Result
	out << endl;
	out << "# Saving result" << endl;
	out << "SAVE DATA RESU 1" << endl;
	out << endl;

	vector<shared_ptr<Assertion>> assertions = analysis.getAssertions();
	if (!assertions.empty()) {
		out << "LANGAGE" << endl;
		out << "variable displacement[" << numberOfDofBySystusOption[systusOption] << "],"
				"frequency, phase[" << numberOfDofBySystusOption[systusOption] << "];" << endl;
		out << "iResu=open_file(\"" << systusModel.getName() << "_" << analysis.getId()
				<< ".RESU\", \"write\");" << endl << endl;

		for (auto assertion : assertions) {
			switch (assertion->type) {
			case Assertion::NODAL_DISPLACEMENT_ASSERTION:
				writeNodalDisplacementAssertion(*assertion, out);
				break;
			case Assertion::FREQUENCY_ASSERTION:
				if (analysis.type == Analysis::LINEAR_DYNA_MODAL_FREQ)
					break;
				writeFrequencyAssertion(*assertion, out);
				break;
			case Assertion::NODAL_COMPLEX_DISPLACEMENT_ASSERTION:
				writeNodalComplexDisplacementAssertion(*assertion, out);
				break;
			default:
				throw WriterException(string("Not implemented"));
			}
			out << endl;
		}

		out << "close_file(iResu)" << endl;
		out << "end;" << endl;
	}
}



void SystusWriter::writeConstraint(const SystusModel& systusModel,
		const ConstraintSet& constraintSet, ostream& out) {
	for (auto constraint : constraintSet.getConstraints()) {
		switch (constraint->type) {
		case Constraint::SPC: {
			std::shared_ptr<SinglePointConstraint> spc = std::static_pointer_cast<
					SinglePointConstraint>(constraint);
			for (int nodePosition : constraint->nodePositions()) {
				Node node = systusModel.model->mesh->findNode(nodePosition);
				out << " NODE " << node.id << " /";
				DOFS spcDOFS = spc->getDOFSForNode(nodePosition);
				if (spcDOFS.contains(DOF::DX))
					out << " UX";
				if (spcDOFS.contains(DOF::DY))
					out << " UY";
				if (spcDOFS.contains(DOF::DZ))
					out << " UZ";
				if (spcDOFS.contains(DOF::RX))
					out << " RX";
				if (spcDOFS.contains(DOF::RY))
					out << " RY";
				if (spcDOFS.contains(DOF::RZ))
					out << " RZ";
				if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
					std::shared_ptr<CoordinateSystem> coordinateSystem = systusModel.model->find(
							Reference<CoordinateSystem>(CoordinateSystem::UNKNOWN,
									node.displacementCS));
					VectorialValue angles = coordinateSystem->getEulerAnglesIntrinsicZYX();
					out << " PSI " << angles.x() << " THETA " << angles.y() << " PHI "
							<< angles.z();
				}
				out << endl;
			}
			break;
		}
		default: {
			//TODO : throw WriterException("Constraint type not supported");
			cout << "Warning : " << *constraint << " not supported" << endl;
		}
		}
	}
}

void SystusWriter::writeLoad(const LoadSet& loadSet, ostream& out) {
	for (auto loading : loadSet.getLoadings()) {
		switch (loading->type) {
		case Loading::NODAL_FORCE: {
			shared_ptr<NodalForce> nodalForce = static_pointer_cast<NodalForce>(loading);
			int node = nodalForce->getNode().position;
			VectorialValue force = nodalForce->getForce();
			VectorialValue moment = nodalForce->getMoment();
			out << " NODE " << node + 1 << " /";
			if (!is_zero(force.x()))
				out << " FX " << force.x();
			if (!is_zero(force.y()))
				out << " FY " << force.y();
			if (!is_zero(force.z()))
				out << " FZ " << force.z();
			if (!is_zero(moment.x()))
				out << " CX " << moment.x();
			if (!is_zero(moment.y()))
				out << " CY " << moment.y();
			if (!is_zero(moment.z()))
				out << " CZ " << moment.z();
			out << endl;
			break;
		}
		case Loading::GRAVITY: {
			// TODO : sans doute faux, à vérifier
			shared_ptr<Gravity> gravity = static_pointer_cast<Gravity>(loading);
			VectorialValue acceleration = gravity->getAccelerationVector();
			out << "  /";
			if (!is_zero(acceleration.x()))
				out << " GX " << acceleration.x();
			if (!is_zero(acceleration.y()))
				out << " GY " << acceleration.y();
			if (!is_zero(acceleration.z()))
				out << " GZ " << acceleration.z();
			out << endl;
			break;
		}
		case Loading::ROTATION: {
			// TODO : sans doute faux, à vérifier
			shared_ptr<Rotation> rotation = static_pointer_cast<Rotation>(loading);
			double omega = rotation->getSpeed();
			VectorialValue pnt1 = rotation->getCenter();
			VectorialValue pnt2 = pnt1 + rotation->getAxis();
			out << "  / CENT " << omega * omega;
			out << " PNT1 " << pnt1.x() << " " << pnt1.y() << " " << pnt1.z();
			out << " PNT2 " << pnt2.x() << " " << pnt2.y() << " " << pnt2.z();
			out << endl;
			break;
		}

		default:
			throw WriterException("Loading type not supported");
		}
	}
}

void SystusWriter::writeLoad(const ConstraintSet& constraintSet, std::ostream& out) {
	for (auto constraint : constraintSet.getConstraints()) {
		switch (constraint->type) {
		case Constraint::SPC: {
			std::shared_ptr<SinglePointConstraint> spc = std::static_pointer_cast<
					SinglePointConstraint>(constraint);
			if (spc->hasReferences()) {
				cerr << "SPC " << spc << " contains references. Not supported" << endl;
				throw logic_error("SPC with references not supported.");
			} else {
				bool outSPC = false;
				DOFS dofs;
				for (DOF dof : spc->getDOFSForNode(0)) {
					if (!is_zero(spc->getDoubleForDOF(dof))) {
						dofs = dofs + dof;
						outSPC = true;
					}
				}
				if (outSPC) {
					for (int nodePosition : constraint->nodePositions()) {
						out << " NODE " << nodePosition + 1 << " /";
						if (dofs.contains(DOF::DX))
							out << " UX " << spc->getDoubleForDOF(DOF::DX);
						if (dofs.contains(DOF::DY))
							out << " UY " << spc->getDoubleForDOF(DOF::DY);
						if (dofs.contains(DOF::DZ))
							out << " UZ " << spc->getDoubleForDOF(DOF::DZ);
						if (dofs.contains(DOF::RX))
							out << " RX " << spc->getDoubleForDOF(DOF::RX);
						if (dofs.contains(DOF::RY))
							out << " RY " << spc->getDoubleForDOF(DOF::RY);
						if (dofs.contains(DOF::RZ))
							out << " RZ " << spc->getDoubleForDOF(DOF::RZ);
						out << endl;
					}
				}

			}
			break;
		}
		default: {
			//TODO : throw WriterException("Constraint type not supported");
			cout << "Warning : " << *constraint << " not supported" << endl;
		}
		}
	}
}

void SystusWriter::writeNodalDisplacementAssertion(Assertion& assertion, ostream& out) {
	NodalDisplacementAssertion& nda = dynamic_cast<NodalDisplacementAssertion&>(assertion);

	if (!is_equal(nda.instant, -1))
		throw WriterException("Instant in NodalDisplacementAssertion not supported");
	int nodePos = nda.nodePosition + 1;
	int dofPos = nda.dof.position + 1;

	out << scientific;
	out << "displacement = node_displacement(1" << "," << nodePos << ");" << endl;
	out << "diff = abs((displacement[" << dofPos << "]-(" << nda.value << "))/("
			<< (abs(nda.value) >= 1e-9 ? nda.value : 1.) << "));" << endl;

	out << "fprintf(iResu,\" ------------------------ TEST_RESU DISPLACEMENT ASSERTION ------------------------\\n\")"
			<< endl;
	out
			<< "fprintf(iResu,\"      NOEUD        NUM_CMP      VALE_REFE             VALE_CALC    ERREUR       TOLE\\n\");"
			<< endl;
	out << "if (diff > abs(" << nda.tolerance
			<< ")) fprintf(iResu,\" NOOK \"); else fprintf(iResu,\" OK   \");" << endl;
	out << "fprintf(iResu,\"" << setw(8) << nodePos << "     " << setw(8) << dofPos << "     "
			<< nda.value
			<< " %e %e " << nda.tolerance << " \\n\\n\", displacement[" << dofPos << "], diff);"
			<< endl;
	out.unsetf(ios::scientific);
}

void SystusWriter::writeNodalComplexDisplacementAssertion(Assertion& assertion, ostream& out) {
	NodalComplexDisplacementAssertion& ncda = dynamic_cast<NodalComplexDisplacementAssertion&>(assertion);

	int nodePos = ncda.nodePosition + 1;
	int dofPos = ncda.dof.position + 1;
	double puls = ncda.frequency*2*M_PI;
	out << scientific;
	out << "nb_map = number_of_tran_maps(1);" << endl;
	out << "nume_ordre = 1;" << endl;
	out << "puls = time_map(nume_ordre);" << endl;
	out << "while (nume_ordre<nb_map-1 && abs(puls - " << puls << ")/" << max(puls, 1.) << "> 1e-5){nume_ordre=nume_ordre+1; puls = time_map(nume_ordre);}" << endl;
	out << "displacement = trans_node_displacement(nume_ordre," << nodePos << ");" << endl;
	out << "phase = trans_node_displacement(nume_ordre+1," << nodePos << ");" << endl;
	out << "displacement_real = displacement[" << dofPos << "]*cos(phase["<< dofPos <<"]);" << endl;
	out << "displacement_imag = displacement[" << dofPos << "]*sin(phase["<< dofPos <<"]);" << endl;
	out << "diff = (abs(displacement_real-(" << ncda.value.real() << ")) + abs(displacement_imag-(" << ncda.value.imag() << ")))"
			<< "/(" << (abs(ncda.value) >= 1e-9 ? abs(ncda.value) : 1.) << ");" << endl;

	out << "fprintf(iResu,\" ------------------------ TEST_RESU COMPLEX DISPLACEMENT ASSERTION ----------------\\n\")"
			<< endl;
	out
			<< "fprintf(iResu,\"      NOEUD        NUM_CMP      FREQUENCE             VALE_REFE                                     "
			<< "VALE_CALC                     ERREUR       TOLE\\n\");"
			<< endl;
	out << "if (diff > abs(" << ncda.tolerance << ")) fprintf(iResu,\" NOOK \"); else fprintf(iResu,\" OK   \");" << endl;
	out << "fprintf(iResu,\"" << setw(8) << nodePos << "     " << setw(8) << dofPos << "     " << ncda.frequency << " "
			<< ncda.value << " (%e,%e) %e " << ncda.tolerance << " \\n\\n\", displacement_real, displacement_imag, diff);"
			<< endl;
	out.unsetf(ios::scientific);
}

void SystusWriter::writeFrequencyAssertion(Assertion& assertion, ostream& out) {
	FrequencyAssertion& frequencyAssertion = dynamic_cast<FrequencyAssertion&>(assertion);

	out << scientific;
	out << "frequency = frequency_number(" << frequencyAssertion.number << ");" << endl;
	out << "diff = abs((frequency-(" << frequencyAssertion.value << "))/("
			<< (abs(frequencyAssertion.value) >= 1e-9 ? frequencyAssertion.value : 1.) << "));" << endl;

	out << "fprintf(iResu,\" ------------------------ TEST_RESU FREQUENCY ASSERTION ------------------------\\n\")"
			<< endl;
	out << "fprintf(iResu,\"      FREQUENCY    VALE_REFE             VALE_CALC    ERREUR       TOLE\\n\");"
			<< endl;
	out << "if (diff > abs(" << frequencyAssertion.tolerance
			<< ")) fprintf(iResu,\" NOOK \"); else fprintf(iResu,\" OK   \");" << endl;
	out << "fprintf(iResu,\"" << setw(8) << frequencyAssertion.number << "     " << frequencyAssertion.value
			<< " %e %e " << frequencyAssertion.tolerance << " \\n\\n\", frequency, diff);"
			<< endl;
	out.unsetf(ios::scientific);
}

} //namespace Vega
