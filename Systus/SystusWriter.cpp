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
 * SystusWriter.cpp
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
#include "SystusAsc.h"
#include "../Abstract/CoordinateSystem.h"
#include "build_properties.h"
#include "cmath" /* M_PI */
#include <ctime>

namespace fs = boost::filesystem;

namespace vega {


string SMFToString(SMF key){
    return SMFtoString.find(key)->second;
}


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
                6, 13, 5, 12 } },
        { CellType::POLY4_CODE, { 0, 1, 2, 3  } }, 
        { CellType::POLY5_CODE, { 0, 1, 2, 3, 4  } },
        { CellType::POLY6_CODE, { 0, 1, 2, 3, 4, 5 } },
        { CellType::POLY7_CODE, { 0, 1, 2, 3, 4, 5, 6 } },
        { CellType::POLY8_CODE, { 0, 1, 2, 3, 4, 5, 6, 7 } },
        { CellType::POLY9_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8 } },
        { CellType::POLY10_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
        { CellType::POLY11_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
        { CellType::POLY12_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } },
        { CellType::POLY13_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } },
        { CellType::POLY14_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
        { CellType::POLY15_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 } },
        { CellType::POLY16_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } },
        { CellType::POLY17_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } },
        { CellType::POLY18_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 } },
        { CellType::POLY19_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 } },
        { CellType::POLY20_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } }

};

const int SystusWriter::DampingAccessId=41;
const int SystusWriter::MassAccessId=42;
const int SystusWriter::StiffnessAccessId=43;

/** Converts a vega node Id in its ASC counterpart (i.e add one!) **/
int SystusWriter::getAscNodeId(const int vega_id) const{
    return vega_id+1;
}

int SystusWriter::DOFSToMaterial(const DOFS dofs, ostream& out) const{
    int nelem=0;
    if (dofs.contains(DOF::DX))
        writeMaterialField(SMF::KX, 1.0, nelem, out);
    if (dofs.contains(DOF::DY))
        writeMaterialField(SMF::KY, 1.0, nelem, out);
    if (dofs.contains(DOF::DZ))
        writeMaterialField(SMF::KZ, 1.0, nelem, out);
    if (dofs.contains(DOF::RX))
        writeMaterialField(SMF::IX, 1.0, nelem, out);
    if (dofs.contains(DOF::RY))
        writeMaterialField(SMF::IY, 1.0, nelem, out);
    if (dofs.contains(DOF::RZ))
        writeMaterialField(SMF::IZ, 1.0, nelem, out);
    return nelem;
}

int SystusWriter::DOFSToInt(const DOFS dofs) const{
    int iout=0;
    if (dofs.contains(DOF::DX))
        iout=iout+1;
    if (dofs.contains(DOF::DY))
        iout=iout+2;
    if (dofs.contains(DOF::DZ))
        iout=iout+4;
    if (dofs.contains(DOF::RX))
        iout=iout+8;
    if (dofs.contains(DOF::RY))
        iout=iout+16;
    if (dofs.contains(DOF::RZ))
        iout=iout+32;
    return iout;
}

int SystusWriter::DOFToInt(const DOF dof) const{
    if (dof == DOF::DX)
        return 1;
    if (dof == DOF::DY)
        return 2;
    if (dof == DOF::DZ)
        return 3;
    if (dof == DOF::RX)
        return 4;
    if (dof == DOF::RY)
        return 5;
    if (dof == DOF::RZ)
        return 6;
    return -1;
}



// TODO: That should not be in the Writer.
CartesianCoordinateSystem SystusWriter::buildElementDefaultReferentiel(const SystusModel& systusModel, const vector<int> nodes, const int dim,
        const int celltype){

    VectorialValue O,x,y;

    switch (dim){
    case 0:
    case 3:{
        x = VectorialValue::X;
        y = VectorialValue::Y;
        O = VectorialValue(0,0,0);
        break;
    }

    case 1:{
        // Element 16XX use the global system as a default
        if (celltype==6){
            x = VectorialValue::X;
            y = VectorialValue::Y;
            O = VectorialValue(0,0,0);
        // Other 1D elements are computed from the orientation of the "bar"
        }else{
            shared_ptr<Mesh> mesh = systusModel.model->mesh;
            Node nO = mesh->findNode(mesh->findNodePosition(nodes[0]), true, systusModel.model);
            Node nX = mesh->findNode(mesh->findNodePosition(nodes[1]), true, systusModel.model);
            O = VectorialValue(nO.x, nO.y, nO.z);
            x = VectorialValue(nX.x-nO.x, nX.y-nO.y, nX.z-nO.z);
            // If the beam is parallel to Oz, we have a special treatment.
            if (is_zero(x.x()) && is_zero(x.y())){
                // If x is null, we use the global referentiel
                if (is_zero(x.z())){
                    handleWritingWarning("X-axis of element is undefined. Global referentiel chosen.", "Element Default Referentiel");
                    x = VectorialValue::X;
                }
                y = VectorialValue::Y;
            }else{
                y = VectorialValue::Z.cross(x);
            }
        }
        break;
    }

    case 2:{
        shared_ptr<Mesh> mesh = systusModel.model->mesh;

        // The origin of the referentiel is the center of the element.
        O = VectorialValue(0,0,0);
        int nbnodes=0;
        for (auto n : nodes){
            Node node = mesh->findNode(mesh->findNodePosition(n), true, systusModel.model);
            O = O + VectorialValue(node.x, node.y, node.z);
            nbnodes++;
        }
        O = O/nbnodes;

        // Shell must have at least 3 nodes.
        if (nbnodes<3){
            handleWritingWarning("2D element must have at least 3 nodes. Global referentiel chosen.", "Element Default Referentiel");
            x = VectorialValue::X;
            y = VectorialValue::Y;
            break;
        }

        if (celltype==4){

            //  See Systus Reference Manual, Section 16.2.2, subsection "Shell element with three nodes" page 1114
            if (nbnodes==3){
                Node nA0 = mesh->findNode(mesh->findNodePosition(nodes[0]), true, systusModel.model);
                Node nA1 = mesh->findNode(mesh->findNodePosition(nodes[1]), true, systusModel.model);
                Node nA2 = mesh->findNode(mesh->findNodePosition(nodes[2]), true, systusModel.model);

                VectorialValue a0a1 = VectorialValue(nA1.x-nA0.x, nA1.y-nA0.y, nA1.z-nA0.z);
                VectorialValue a0a2 = VectorialValue(nA2.x-nA0.x, nA2.y-nA0.y, nA2.z-nA0.z);
                VectorialValue z = a0a1.cross(a0a2);

                x = VectorialValue::Z.cross(z);
                y = z.cross(x);

            }else{

                // We don't treat this case yet.
                handleWritingWarning("Default referentiel of element 2"+std::to_string(celltype)+std::to_string(nbnodes)+" not supported. Global referentiel chosen.", "Element Default Referentiel");
                x = VectorialValue::X;
                y = VectorialValue::Y;
            }

            // If x or y are null, we use the global referentiel
            if (x.iszero() or y.iszero()){
                handleWritingWarning("Local axis of 2D element are undefined. Global referentiel chosen.", "Element Default Referentiel");
                x = VectorialValue::X;
                y = VectorialValue::Y;
            }

        }else{
            // We don't treat this case yet.
            handleWritingWarning("Default referentiel of element 2"+std::to_string(celltype)+"XX not supported. Global referentiel chosen.", "Element Default Referentiel");
            x = VectorialValue::X;
            y = VectorialValue::Y;
        }
        break;

    }
    default:{
        handleWritingError("Invalid dimension: "+ to_string(dim), "Element Default Referentiel");
        x = VectorialValue::X;
        y = VectorialValue::Y;
        O = VectorialValue(0,0,0);
    }
    }

    CartesianCoordinateSystem rcs = CartesianCoordinateSystem(*(systusModel.model), O, x, y);
    rcs.build();
    return rcs;

}

int SystusWriter::auto_part_id = 99999999;

int SystusWriter::getPartId(const string partName, set<int> & usedPartId) {

    int partId;

    // First, we try to cast an integer from the suffix of the PartName
    std::size_t pos=partName.rfind("_");
    if (pos!=std::string::npos){
        try{
            partId = std::stoi(partName.substr(pos+1));
        }catch(...){
            partId = auto_part_id--;
        }
    }else{
        partId = auto_part_id--;
    }

    // If the Part Id is unavailable, we find another one.
    while (usedPartId.find(partId)!= usedPartId.end()){
        partId = auto_part_id--;
    }

    usedPartId.insert(partId);
    return partId;
}

void SystusWriter::writeMaterialField(const SMF key, const double value, int& nbfields, ostream& out) const{
    switch(key){
    case(RHO):
    case(E):
    case(NU):{
        if (value>0.0){
            out << " " << key << " "<<value;
            nbfields++;
        }
        break;
    }
    case(S):
    case(IX):
    case(IY):
    case(IZ):
    case(H):
    case(KX):
    case(KY):
    case(KZ):
    case(COEF):{
        out << " " << key << " "<<value;
        nbfields++;
        break;
    }
    case(ALPHA):
    case(AY):
    case(AZ):{
        if (!is_equal(value, vega::Globals::UNAVAILABLE_DOUBLE)){
            out << " " << key << " " << value;
            nbfields++;
        }
        break;
    }
    default:{
        cerr << "Warning : Unrecognized material field (double value) "<< SMFToString(key) <<endl;
    }
    }
}

void SystusWriter::writeMaterialField(const SMF key, const int value, int& nbfields, ostream& out) const{

    switch(key){
    case(ID):{  //ID is the very beginning of the line. Its format is "N 0"
        out << value<<" 0";
        break;
    }
    case(TABLE):
    case(SHAPE):
    case(MID):
    case(DEPEND):
    case(LEVEL):
    case(TYPE):{
        out << " " << key << " "<<value;
        nbfields++;
        break;
    }
    default:{
        cerr << "Warning : Unrecognized material field (integer value) "<< SMFToString(key) <<endl;
    }
    }
}



string SystusWriter::writeModel(const shared_ptr<Model> model,
        const vega::ConfigurationParameters &configuration) {
    SystusModel systusModel = SystusModel(&(*model), configuration);
    this->translationMode= configuration.translationMode;
    cout << "Systus writer" << endl;

    string path = systusModel.configuration.outputPath;
    if (!fs::exists(path)) {
        throw iostream::failure("Directory " + path + " don't exist.");
    }

    // On Systus output, we build a "general" solver file
    ofstream dat_file_ofs;
    string dat_path = systusModel.getOutputFileName("_ALL.DAT");
    if (configuration.systusOutputProduct=="systus"){
        dat_file_ofs.open(dat_path.c_str(), ios::trunc);
        if (!dat_file_ofs.is_open()) {
            string message = string("Can't open file ") + dat_path + " for writing.";
            throw ios::failure(message);
        }
    }

    /* Work to Do Only once */
    getSystusInformations(systusModel, configuration);
    generateRBEs(systusModel, configuration);
    generateSubcases(systusModel, configuration);

    for (unsigned idSubcase = 0; idSubcase< systusSubcases.size(); idSubcase++){

        /* Translation and filling of a lots of things */
        this->translate(systusModel, idSubcase);

        /* ASCI file */
        string asc_path = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1)+ "_DATA1.ASC");
        ofstream asc_file_ofs;
        asc_file_ofs.precision(DBL_DIG);
        asc_file_ofs.open(asc_path.c_str(), ios::trunc | ios::out);
        if (!asc_file_ofs.is_open()) {
            string message = string("Can't open file ") + asc_path + " for writing.";
            throw ios::failure(message);
        }
        this->writeAsc(systusModel, configuration, idSubcase, asc_file_ofs);
        asc_file_ofs.close();

        /* Write some matrix files, if needed */
        this->writeMatrixFiles(systusModel, idSubcase);

        /* Analysis file */
        ofstream analyse_file_ofs;
        analyse_file_ofs.precision(DBL_DIG);
        string analyse_path = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + ".DAT");
        analyse_file_ofs.open(analyse_path.c_str(), ios::trunc);

        if (!analyse_file_ofs.is_open()) {
            string message = string("Can't open file ") + analyse_path + " for writing.";
            throw ios::failure(message);
        }
        this->writeDat(systusModel, configuration, idSubcase, analyse_file_ofs);
        analyse_file_ofs.close();

        if (configuration.systusOutputProduct=="systus"){
            dat_file_ofs << "READ " << systusModel.getName() << "_SC" << to_string(idSubcase+1) << ".DAT" << endl;
        }
    }

    if (configuration.systusOutputProduct=="systus"){
        dat_file_ofs.close();
    }
    return dat_path;
}

void SystusWriter::getSystusInformations(const SystusModel& systusModel, const ConfigurationParameters& configurationParameters) {

    CellType cellType[21] = { CellType::POINT1, CellType::SEG2, CellType::SEG3, CellType::SEG4, CellType::SEG5,
            CellType::TRI3, CellType::QUAD4, CellType::TRI6, CellType::TRI7, CellType::QUAD8,
            CellType::QUAD9, CellType::TETRA4, CellType::PYRA5, CellType::PENTA6, CellType::HEXA8,
            CellType::TETRA10, CellType::HEXGP12, CellType::PYRA13, CellType::PENTA15,
            CellType::HEXA20, CellType::HEXA27 };

    bool hasElement[21];
    int numNodes[21];
    for (int i = 0; i < 21; i++) {
        hasElement[i] = !!systusModel.model->mesh->countCells(cellType[i]);
        numNodes[i] = hasElement[i] ? cellType[i].numNodes : 0;
    }

    bool has1DOr2DElements = false;
    for (int i = 0; i < 11; i++)
        has1DOr2DElements = hasElement[i] || has1DOr2DElements;
    bool has3DElements = false;
    for (int i = 11; i < 21; i++)
        has3DElements = hasElement[i] || has3DElements;

    if (configurationParameters.systusOptionAnalysis =="auto"){
        if (has1DOr2DElements){
            systusOption = 3;
            systusSubOption = has3DElements ? 3 : 0;
        }else{
            systusOption = 4;
            systusSubOption = 0;
        }
    }else{
        if (configurationParameters.systusOptionAnalysis =="shell"){
            systusOption = 3;
            systusSubOption = 0;
        }else if(configurationParameters.systusOptionAnalysis =="shell-multi"){
            systusOption = 3;
            systusSubOption = 3;
        }else{
            systusOption = 4;
            systusSubOption = 0;
        }
    }
    maxNumNodes = *max_element(numNodes, numNodes + 21);
    nbNodes = systusModel.model->mesh->countNodes();

}

void SystusWriter::generateRBEs(const SystusModel& systusModel,
        const vega::ConfigurationParameters &configuration) {

    shared_ptr<Mesh> mesh = systusModel.model->mesh;
    vector<shared_ptr<ConstraintSet>> commonConstraintSets = systusModel.model->getCommonConstraintSets();
    RbarPositions.clear();
    RBE2rbarPositions.clear();
    RBE3rbarPositions.clear();
    RBE3Dofs.clear();
    RBE3Coefs.clear();
    rotationNodeIdByTranslationNodeId.clear();

    // Material Id are usually computed from the corresponding ElementSet Id
    // TODO: It should be the material...
    vector<int> v= systusModel.model->getElementSetsId();
    int idMaterial=0;
    if (v.size()!=0){
        idMaterial=*std::max_element(v.begin(), v.end());
    }

    for (const auto& constraintSet : commonConstraintSets) {

        // Translation of RBAR and RBE2 (RBE2 are viewed as an assembly of RBAR)
        // See Systus Reference Analysis Manual: RIGID BoDY Element (page 498)
        set<shared_ptr<Constraint>> constraints = constraintSet->getConstraintsByType(Constraint::RIGID);
        for (const auto& constraint : constraints) {
            std::shared_ptr<RigidConstraint> rbe2 = std::static_pointer_cast<RigidConstraint>(constraint);


            CellGroup* group = mesh->createCellGroup("RBE2_"+std::to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBE2");
            idMaterial++;

            Node master = mesh->findNode(rbe2->getMaster());
            int master_rot_id = 0;
            if (systusOption == 4){
                int master_rot_position = mesh->addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
                master_rot_id = mesh->findNode(master_rot_position).id;
                rotationNodeIdByTranslationNodeId[master.id]=master_rot_id;
            }

            for (int position : rbe2->getSlaves()){
                Node slave = mesh->findNode(position);
                vector<int> nodes = {master.id, slave.id};
                if (systusOption == 4)
                    nodes.push_back(master_rot_id);

                // With a Lagrangian formulation, we add a Lagrange node.
                if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
                    int slave_lagr_position = mesh->addNode(Node::AUTO_ID, slave.lx, slave.ly, slave.lz, slave.positionCS, slave.displacementCS);
                    int slave_lagr_id = mesh->findNode(slave_lagr_position).id;
                    nodes.push_back(slave_lagr_id);
                }

                int cellPosition;
                switch (nodes.size()){
                case (2):{
                    cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                    break;
                }
                case (3):{
                    cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG3, nodes, true);
                    break;
                }
                case(4):{
                    cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG4, nodes, true);
                    break;
                }
                }
                RBE2rbarPositions[idMaterial].push_back(cellPosition);
                group->addCell(mesh->findCell(cellPosition).id);
            }
        }

        constraints = constraintSet->getConstraintsByType(Constraint::QUASI_RIGID);
        for (const auto& constraint : constraints) {
            std::shared_ptr<QuasiRigidConstraint> rbar = std::static_pointer_cast<QuasiRigidConstraint>(constraint);

            if (!(rbar->isCompletelyRigid())){
                cerr << "QUASI_RIDID constraint not available yet. Constraint "
                        << constraint->getOriginalId()<< "translated as rigid constraint."<<endl;
            }

            CellGroup* group = mesh->createCellGroup("RBAR_"+std::to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBAR");
            idMaterial++;

            if (rbar->getSlaves().size()!=2){
                handleWritingError(string("QUASI_RIDID constraint must have exactly two slaves."));
            }

            // Master Node : first one
            Node masterNode = mesh->findNode(*rbar->getSlaves().begin());
            vector<int> nodes = {masterNode.id};

            // Slave Node : second and last one
            Node slaveNode = mesh->findNode(*rbar->getSlaves().rbegin());
            nodes.push_back(slaveNode.id);

            // Master Rotation Node (if needed)
            if (systusOption == 4){
                int master_rot_position = mesh->addNode(Node::AUTO_ID, masterNode.lx, masterNode.ly, masterNode.lz, masterNode.positionCS, masterNode.displacementCS);
                int master_rot_id = mesh->findNode(master_rot_position).id;
                nodes.push_back(master_rot_id);
                rotationNodeIdByTranslationNodeId[masterNode.id]=master_rot_id;
            }

            // With a Lagrangian formulation, we add a Lagrange node.
            if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
                int slave_lagr_position = mesh->addNode(Node::AUTO_ID, slaveNode.lx, slaveNode.ly, slaveNode.lz, slaveNode.positionCS, slaveNode.displacementCS);
                int slave_lagr_id = mesh->findNode(slave_lagr_position).id;
                nodes.push_back(slave_lagr_id);
            }

            int cellPosition;
            switch (nodes.size()){
            case (2):{
                cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG2, nodes, true);
                break;
            }
            case (3):{
                cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG3, nodes, true);
                break;
            }
            case(4):{
                cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG4, nodes, true);
                break;
            }
            }
            RbarPositions[idMaterial].push_back(cellPosition);
            group->addCell(mesh->findCell(cellPosition).id);
        }



        /* See Systus Reference Analysis Manual, Section 8.8 "Special Elements",
         * Subsection "Use of Averaging Type Solid Elements", p500.
         */
        constraints = constraintSet->getConstraintsByType(Constraint::RBE3);
        for (const auto& constraint : constraints) {

            std::map<DOFS, std::map<double, int>> materialByCoefByDOFS;

            std::shared_ptr<RBE3> rbe3 = std::static_pointer_cast<RBE3>(constraint);
            CellGroup* group = mesh->createCellGroup("RBE3_"+std::to_string(constraint->getOriginalId()), CellGroup::NO_ORIGINAL_ID, "RBE3");
            Node master = mesh->findNode(rbe3->getMaster());
            const DOFS mDOFS = rbe3->getDOFS();

            // Creating a Lagrange node
            int master_lagr_position = mesh->addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
            int master_lagr_id = mesh->findNode(master_lagr_position).id;


            // Creating rotation nodes if needed
            int master_rot_id=0;
            int master_lagr_rot_id=0;
            if (systusOption == 4){
                int master_rot_position = mesh->addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
                master_rot_id = mesh->findNode(master_rot_position).id;
                rotationNodeIdByTranslationNodeId[master.id]=master_rot_id;
                int master_lagr_rot_position = mesh->addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
                master_lagr_rot_id = mesh->findNode(master_lagr_rot_position).id;
            }

            for (int position : rbe3->getSlaves()){
                Node slave = mesh->findNode(position);
                vector<int> nodes = {master.id, slave.id};
                if (systusOption == 4){
                    nodes.push_back(master_rot_id);
                }
                nodes.push_back(master_lagr_id);
                if (systusOption == 4){
                    nodes.push_back(master_lagr_rot_id);
                }
                int cellPosition;
                switch (nodes.size()){
                case (3):{
                    cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG3, nodes, true);
                    break;
                }
                case (5):{
                    cellPosition = mesh->addCell(Cell::AUTO_ID, CellType::SEG5, nodes, true);
                    break;
                }
                }

                /* We build a material for each value of "Slave DOFS" and "Slave Coeff" */
                const DOFS dDOFS = rbe3->getDOFSForNode(position);
                const double dCoef = rbe3->getCoefForNode(position);
                if (materialByCoefByDOFS[dDOFS][dCoef]==0){
                    idMaterial++;
                    materialByCoefByDOFS[dDOFS][dCoef] =idMaterial;
                    RBE3Dofs[idMaterial]= {mDOFS, dDOFS};
                    RBE3Coefs[idMaterial]= dCoef;
                }
                int idCMaterial = materialByCoefByDOFS[dDOFS][dCoef];
                RBE3rbarPositions[idCMaterial].push_back(cellPosition);

                group->addCell(mesh->findCell(cellPosition).id);
            }
        }
    }

}


void SystusWriter::generateSubcases(const SystusModel& systusModel,
        const vega::ConfigurationParameters &configuration) {

    systusSubcases.clear();

    vector< vector<long unsigned int> > characteristicAnalysis;
    // Default is "Automatic merge of static subcases".
    if (configuration.systusSubcases.empty()){
        for (const auto& it : systusModel.model->analyses) {
            const Analysis& analysis = *it;

            // We create a vector with the characteristic of the Analysis
            vector<long unsigned int> cAna;
            cAna.push_back(analysis.type); //Analysis type

            const vector<shared_ptr<ConstraintSet>> constraintSets = analysis.getConstraintSets();
            cAna.push_back(constraintSets.size());
            for (auto constraintSet : constraintSets){
                cAna.push_back(constraintSet->getId());
            }
            //TODO Maybe we need to test that to?
            //const vector<shared_ptr<BoundaryCondition>> boundaryConditions= analysis.getBoundaryConditions();
            //cAna.push_back(boundaryConditions.size());
            //for (auto boundaryCondition : boundaryConditions){
            //  cAna.push_back(boundaryCondition->getId());
            //}


            // For mechanical static problem, we search the already defined subcases for the same characteristic
            long unsigned int idSubcase = systusSubcases.size();
            if (analysis.type == Analysis::Type::LINEAR_MECA_STAT){
                for (long unsigned int i=0; i<characteristicAnalysis.size(); i++){
                    if (cAna== characteristicAnalysis[i]){
                        idSubcase= i;
                        break;
                    }
                }
            }

            // New Subcase
            if (idSubcase == systusSubcases.size()){
                characteristicAnalysis.push_back(cAna);
                systusSubcases.push_back({analysis.getId()});
            }else{
                // Already existing subcases
                systusSubcases[idSubcase].push_back(analysis.getId());
            }

        }
    }else{

        // On "single mode", each analysis is in its own subcase.
        if ((configuration.systusSubcases.size()==1) &&
                (configuration.systusSubcases[0][0]==-1)){
            for (const auto& it : systusModel.model->analyses) {
                const Analysis& analysis = *it;
                systusSubcases.push_back({analysis.getId()});
            }
        }else{
            // We use the user defined lists

            // We make a correspondence between the User defined Id (aka "Original Id")
            // and the internal Id.
            map<int, int> analysisIdByOriginalId;
            for (const auto& it : systusModel.model->analyses) {
                const Analysis& analysis = *it;
                const int originalId = analysis.getOriginalId();
                if (originalId!=Analysis::NO_ORIGINAL_ID){
                    if (analysisIdByOriginalId.find(originalId)!= analysisIdByOriginalId.end() ){
                        cerr << "Warning: Systus is building Subcases from a list of Analysis Original Ids,"<<endl;
                        cerr << "Warning: but the Original Id "<<originalId<<" corresponds to several analyzes. Only the first one will be kept."<<endl;
                    }else{
                        analysisIdByOriginalId[originalId]=analysis.getId();
                    }
                }else{
                    cerr << "Warning: Systus is building Subcases from a list of Analysis Original Ids,"<<endl;
                    cerr << "Warning: but the Analysis "<<analysis.getId()<< " has no original Id and will be dismissed."<<endl;
                }
            }

            // Building the vector of AnalysisIds By SubcaseIds
            for (const vector<int> subcase : configuration.systusSubcases){
                vector<int> sub;
                for (int oId : subcase){
                    if (analysisIdByOriginalId.find(oId)== analysisIdByOriginalId.end() ){
                        cerr << "Warning: Systus is building Subcases from a list of Analysis Original Ids,"<<endl;
                        cerr << "Warning: but we can't find the Analysis "<<oId<<endl;
                    }else{
                        sub.push_back(analysisIdByOriginalId[oId]);
                    }
                }
                if (!sub.empty())
                    systusSubcases.push_back(sub);
            }

            // We test if every analysis in a subcase is of the same type
            for (unsigned i = 0 ; i< systusSubcases.size(); i++){
                const vector<int> subcase = systusSubcases[i];
                const auto refAnalysis = systusModel.model->getAnalysis(subcase[0]);
                const Analysis::Type refType = refAnalysis->type;
                for (unsigned j = 1; j < subcase.size(); j++){
                    const auto analysis = systusModel.model->getAnalysis(subcase[j]);
                    if (analysis->type!= refType){
                        cerr << "Warning: The user-defined subcase "<<(i+1)<<" regroups analysis of different types: "
                                <<Analysis::stringByType.at(refType)<< " "<< Analysis::stringByType.at(analysis->type)<<endl;
                        cerr << "Warning: "<< Analysis::stringByType.at(refType) <<" will be used."<<endl;
                        break;
                    }
                }
            }
        }
    }

    // Log is life
    if (configuration.logLevel >= LogLevel::INFO){
        for (unsigned i = 0 ; i< systusSubcases.size(); i++){
            const vector<int> subcase = systusSubcases[i];
            cout << "Subcase "<< (i+1) <<" regroups Analyzes:";
            for (unsigned j = 0; j < subcase.size(); j++){
                const auto analysis = systusModel.model->getAnalysis(subcase[j]);
                cout << " \""<< analysis->getLabel()<<"\"";
            }
            cout <<endl;
        }
    }

    // If there is no analysis, we will only translate the mesh
    if (systusSubcases.size()==0){
        systusSubcases.push_back({});
    }

}

// Select the Loads of the current analysis and give them a local Systus number.
void SystusWriter::fillLoads(const SystusModel& systusModel, const int idSubcase){

    int idSystusLoad=1;

    // All analysis to do
    const vector<int> analysisId = systusSubcases[idSubcase];

    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const shared_ptr<Analysis> analysis = systusModel.model->getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            cerr << "Warning in Filling Loads : wrong analysis number ("<< analysisId[i]<<") Analysis dismissed"<<endl;
            break;
        }
        localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]= {};
        const vector<shared_ptr<LoadSet>> analysisLoadSets = analysis->getLoadSets();
        for (const auto& loadSet : analysisLoadSets) {
            localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()][loadSet->getId()]= idSystusLoad;

            // Title of Loadset is of the form AnalysisName_lLoadId.
            // It is limited to 80 characters
            string suffixe = "_LOAD"+to_string(loadSet->bestId());
            localLoadingListName[idSystusLoad]= analysis->getLabel().substr(0, 80 - suffixe.length())+ suffixe;
            idSystusLoad++;
        }
    }
}


void SystusWriter::fillLoadingsVectors(const SystusModel& systusModel, const int idSubcase){

    // First available vector
    long unsigned int vectorId= vectors.size()+1;

    // All analysis to do
    const vector<int> analysisId = systusSubcases[idSubcase];

    // Work, work
    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const shared_ptr<Analysis> analysis = systusModel.model->getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            cerr << "Warning in Building Vectors : wrong analysis number. Analysis dismissed"<<endl;
            break;
        }

        // Add loading Vectors
        // Note: here, if a loading is used in several loadset or analysis, we duplicate the vector.
        // It's not mandatory, providing you can match the loading to its set of (node, vector).
        // But, it's easier this way ;)
        for (const auto& loadset : analysis->getLoadSets()){
            const int idLoadCase = localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()][loadset->getId()];
            loadingVectorIdByLocalLoading[idLoadCase]=0;
            for (const auto& loading : loadset->getLoadings()) {
                vector<double> vec;
                double normvec = 0.0;

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
                    vec.push_back(force.x()); normvec=max(normvec, abs(force.x()));
                    vec.push_back(force.y()); normvec=max(normvec, abs(force.y()));
                    vec.push_back(force.z()); normvec=max(normvec, abs(force.z()));
                    if (systusOption == 3){
                        vec.push_back(moment.x()); normvec=max(normvec, abs(moment.x()));
                        vec.push_back(moment.y()); normvec=max(normvec, abs(moment.y()));
                        vec.push_back(moment.z()); normvec=max(normvec, abs(moment.z()));
                    }
                    int node = nodalForce->getNode().position;
                    if (!is_zero(normvec)){
                        vectors[vectorId]=vec;
                        loadingVectorsIdByLocalLoadingByNodePosition[node][idLoadCase].push_back(vectorId);
                        vectorId++;
                    }
                    // Rigid Body Element in option 3D.
                    // We report the force from the master node to the master rotational node.
                    if (systusOption == 4){
                        int nid = systusModel.model->mesh->findNode(node).id;
                        const auto & it = rotationNodeIdByTranslationNodeId.find(nid);
                        if (it != rotationNodeIdByTranslationNodeId.end()){
                            int rotNodePosition= systusModel.model->mesh->findNodePosition(it->second);
                            normvec = 0.0;
                            vec.clear();
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(moment.x()); normvec=max(normvec, abs(moment.x()));
                            vec.push_back(moment.y()); normvec=max(normvec, abs(moment.y()));
                            vec.push_back(moment.z()); normvec=max(normvec, abs(moment.z()));
                            if (!is_zero(normvec)){
                                vectors[vectorId]=vec;
                                loadingVectorsIdByLocalLoadingByNodePosition[rotNodePosition][idLoadCase].push_back(vectorId);
                                vectorId++;
                            }
                        }
                    }
                    break;
                }

                case Loading::GRAVITY: {
                    shared_ptr<Gravity> gravity = static_pointer_cast<Gravity>(loading);
                    VectorialValue acceleration = gravity->getAccelerationVector();
                    vec.push_back(6);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(acceleration.x()); normvec=max(normvec, abs(acceleration.x()));
                    vec.push_back(0);
                    vec.push_back(acceleration.y()); normvec=max(normvec, abs(acceleration.y()));
                    vec.push_back(0);
                    vec.push_back(acceleration.z()); normvec=max(normvec, abs(acceleration.z()));
                    if (!is_zero(normvec)){
                        if (loadingVectorIdByLocalLoading[idLoadCase]!=0){
                            handleWritingWarning("GRAVITY already defined for this loadcase. Dismissing load "+ to_string(gravity->bestId()) );
                        }else{
                            vectors[vectorId]=vec;
                            loadingVectorIdByLocalLoading[idLoadCase]= vectorId;
                            vectorId++;
                        }
                    }
                    break;
                }
                case Loading::DYNAMIC_EXCITATION:{
                    // Nothing to be done here
                    break;
                }
                default: {
                    //TODO : throw WriterException("Loading type not supported");
                    cout << "WARNING: " << *loading << " not supported" << endl;
                }
                }
            }
        }
    }
}


void SystusWriter::fillConstraintsVectors(const SystusModel& systusModel, const int idSubcase){

    // First available vector
    long unsigned int vectorId = vectors.size()+1;

    // All analysis to do
    const vector<int> analysisId = systusSubcases[idSubcase];

    // Work, work
    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const shared_ptr<Analysis> analysis = systusModel.model->getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            cerr << "Warning in Building Vectors : wrong analysis number. Analysis dismissed"<<endl;
            break;
        }

        // Add Loadcase Constraint Vectors
        // TODO: Add Subcase Constraint Vectors
        for (const auto& constraintset : analysis->getConstraintSets()){
            for (const auto& constraint : constraintset->getConstraints()) {
                vector<double> vec;
                double normvec=0.0;
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
                        if (systusOption == 3){
                            for (DOF dof : DOFS::ALL_DOFS){
                                double value = (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0);
                                normvec = max(normvec, abs(value));
                                vec.push_back(value);
                            }
                        }else if (systusOption == 4) {
                            for (DOF dof : DOFS::TRANSLATIONS){
                                double value = (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0);
                                normvec = max(normvec, abs(value));
                                vec.push_back(value);
                            }
                        }else
                            handleWritingError("systusOption not supported");

                        if (!is_zero(normvec)){
                            vectors[vectorId]=vec;
                            for (const auto& it : localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]){
                                for (int nodePosition : constraint->nodePositions()){
                                    constraintVectorsIdByLocalLoadingByNodePosition[nodePosition][it.second].push_back(vectorId);
                                }
                            }
                            vectorId++;
                        }
                        // Rigid Body Element in option 3D.
                        // We report the constraints from the master node to the master rotational node.
                        if (systusOption==4){

                            // Rotation vector
                            vec.clear();
                            normvec=0.0;
                            vec.push_back(4);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            vec.push_back(0);
                            for (DOF dof : DOFS::ROTATIONS){
                                double value = (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0);
                                normvec = max(normvec, abs(value));
                                vec.push_back(value);
                            }

                            if (!is_zero(normvec)){
                                bool firstTime=true;
                                for (int nodePosition : constraint->nodePositions()){
                                    int nid = systusModel.model->mesh->findNode(nodePosition,false).id;
                                    const auto & it = rotationNodeIdByTranslationNodeId.find(nid);
                                    if (it!=rotationNodeIdByTranslationNodeId.end()){
                                        int rotNodePosition= systusModel.model->mesh->findNodePosition(it->second);
                                        for (const auto & it2 : localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]){
                                            constraintVectorsIdByLocalLoadingByNodePosition[rotNodePosition][it2.second].push_back(vectorId);
                                        }
                                        if (firstTime){
                                            vectors[vectorId]=vec;
                                            vectorId++;
                                            firstTime = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                case Constraint::RIGID:
                case Constraint::RBE3:
                case Constraint::QUASI_RIGID:{
                    // Nothing to be done here
                    break;
                }
                default: {
                    //TODO : throw WriterException("Constraint type not supported");
                    cout << "WARNING: " << *constraint << " not supported" << endl;
                }
                }
            }
        }
    }
}


void SystusWriter::fillCoordinatesVectors(const SystusModel& systusModel, const int idSubcase){

    UNUSEDV(idSubcase);

    // First available vector
    long unsigned int vectorId = vectors.size()+1;
    map<int, long unsigned int> localVectorIdByCoordinateSystemPos;
    
    // Add vectors for Node Coordinate System
    // Remark: Element Coordinate System are not translated as vectors
    const shared_ptr<Mesh> mesh = systusModel.model->mesh;
    for (const auto& node : mesh->nodes) {
        if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){

            // Trick for not doing the Cartesian coordinate systems all over again;
            auto it = localVectorIdByCoordinateSystemPos.find(node.displacementCS);
            if (it != localVectorIdByCoordinateSystemPos.end()){
                localVectorIdByNodePosition[node.position] = it->second;
                continue;
            }

            auto cs = systusModel.model->getCoordinateSystemByPosition(node.displacementCS);
            vector<double> vec;
            switch (cs->type){
            case CoordinateSystem::CARTESIAN:{
                VectorialValue angles = cs->getEulerAnglesIntrinsicZYX(); // (PSI, THETA, PHI)
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(angles.x());
                vec.push_back(angles.y());
                vec.push_back(angles.z());
                localVectorIdByCoordinateSystemPos[node.displacementCS]=vectorId;
                break;
            }
            // Element orientation : it depends of the kind of elements.
            case CoordinateSystem::ORIENTATION:{
                handleWritingWarning("Local coordinate system are forbidden for nodes.");
                handleWritingWarning("We will fill a null vector for the node "+ to_string(mesh->findNode(node.position).id));
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0.0);
                vec.push_back(0.0);
                vec.push_back(0.0);
                break;
            }

            // Cylyndrical orientation : we create a vector by point
            case CoordinateSystem::CYLINDRICAL:{
                Node nNode = mesh->findNode(node.position, true, systusModel.model);
                shared_ptr<CylindricalCoordinateSystem> ccs = static_pointer_cast<CylindricalCoordinateSystem>(cs);
                ccs->updateLocalBase(VectorialValue(nNode.x, nNode.y, nNode.z));
                VectorialValue angles = ccs->getLocalEulerAnglesIntrinsicZYX(); // (PSI, THETA, PHI)
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(angles.x());
                vec.push_back(angles.y());
                vec.push_back(angles.z());
                break;
            }

            default: {
                ostringstream oerr;
                oerr << *cs << " is not supported. Referentiel dismissed.";
                handleWritingWarning(oerr.str());
                handleWritingWarning("We will fill a null vector for the node "+ to_string(mesh->findNode(node.position).id));
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0);
                vec.push_back(0.0);
                vec.push_back(0.0);
                vec.push_back(0.0);
            }
            }
            vectors[vectorId]=vec;
            localVectorIdByNodePosition[node.position]=vectorId;
            vectorId++;
        }
    }
}

// Fill the vectors field with Vectors relative to Loadings and Castings
//TODO: add all vectors in this function
void SystusWriter::fillVectors(const SystusModel& systusModel, const int idSubcase){

    // Work
    fillLoadingsVectors(systusModel, idSubcase);
    fillConstraintsVectors(systusModel, idSubcase);
    fillCoordinatesVectors(systusModel, idSubcase);
}


void SystusWriter::fillConstraintsNodes(const SystusModel& systusModel, const int idLoadcase){

    // Available degrees of freedom
    char dofCode;
    if (systusOption == 3)
        dofCode = (char) DOFS::ALL_DOFS;
    else if (systusOption == 4)
        dofCode = (char) DOFS::TRANSLATIONS;
    else
        handleWritingError("systusOption not supported");
    const shared_ptr<Mesh> mesh = systusModel.model->mesh;


    // All analysis of the subcase
    // We only work on the first one, as they have the same constraints (normally!)
    const vector<int> analysisId = systusSubcases[idLoadcase];
    if (analysisId.size()==0){//Mode: mesh_only
        return;
    }
    const shared_ptr<Analysis> analysis = systusModel.model->getAnalysis(analysisId[0]);
    if (analysis==nullptr){
        handleWritingError("Wrong analysis number in Constraint Node. Analysis dismissed.");
        return;
    }

    // Work
    for (const auto & constraintSet : analysis->getConstraintSets()){
        for (const auto& constraint : constraintSet->getConstraints()) {

            switch (constraint->type) {
            case Constraint::SPC: {
                std::shared_ptr<SinglePointConstraint> spc = std::static_pointer_cast<
                        SinglePointConstraint>(constraint);
                for (int nodePosition : constraint->nodePositions()) {

                    // We compute the Degree Of Freedom of the node (see ASC Manual)
                    DOFS constrained = constraint->getDOFSForNode(nodePosition);
                    if (constraintByNodePosition.find(nodePosition) == constraintByNodePosition.end()){
                        constraintByNodePosition[nodePosition] = char(constrained) & dofCode;
                    }else{
                        constraintByNodePosition[nodePosition] = (char(constrained) & dofCode)
                                            | constraintByNodePosition[nodePosition];
                    }

                    // Rigid Body Element in option 3D.
                    // We report the constraints from the master node to the master rotational node.
                    if (systusOption==4){
                        int nid =  mesh->findNode(nodePosition).id;
                        const auto & it = rotationNodeIdByTranslationNodeId.find(nid);
                        if (it != rotationNodeIdByTranslationNodeId.end()){
                            DOFS constrainedRot(constrained.contains(DOF::RX),constrained.contains(DOF::RY),constrained.contains(DOF::RZ));
                            int rotNodePosition= mesh->findNodePosition(it->second);
                            if (constraintByNodePosition.find(rotNodePosition) == constraintByNodePosition.end()){
                                constraintByNodePosition[rotNodePosition] = char(constrainedRot) & dofCode;
                            }else{
                                constraintByNodePosition[rotNodePosition] = (char(constrainedRot) & dofCode)
                                                    | constraintByNodePosition[rotNodePosition];
                            }
                        }
                    }
                }
                break;
            }
            case Constraint::RIGID:
            case Constraint::RBE3:
            case Constraint::QUASI_RIGID:{
                // Nothing to be done here
                break;
            }
            default: {
                handleWritingWarning("Constraint type "+constraint->to_str() +"is not supported");
            }
            }
        }
    }
}


void SystusWriter::fillLists(const SystusModel& systusModel, const int idSubcase) {

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that this function relies heavily on lists built before.
    // Lists that ARE dependent on the model and current subcase.
    UNUSEDV(systusModel);
    UNUSEDV(idSubcase);

    // Starting from 1
    int idSystusList=1;

    // Building lists for Loading on nodes
    for (const auto& it : loadingVectorsIdByLocalLoadingByNodePosition){
        loadingListIdByNodePosition[it.first] = idSystusList;
        vector<long unsigned int> sl;
        for (const auto & it2 : it.second){
            for (const long unsigned int vectorId : it2.second){
                sl.push_back(it2.first);
                sl.push_back(vectorId);
            }
        }
        lists[idSystusList]= sl;
        idSystusList++;
    }

    // Building lists for Constraints on nodes
    for (const auto& it : constraintVectorsIdByLocalLoadingByNodePosition){
        constraintListIdByNodePosition[it.first] = idSystusList;
        vector<long unsigned int> sl;
        for (const auto & it2 : it.second){
            for (const long unsigned int vectorId : it2.second){
                sl.push_back(it2.first);
                sl.push_back(vectorId);
            }
        }
        lists[idSystusList]= sl;
        idSystusList++;
    }
}


void SystusWriter::fillTables(const SystusModel& systusModel, const int idSubcase) {

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that only one kind of tables are translated yet,
    // and we don't know what the others will need.
    UNUSEDV(idSubcase);

    if (systusModel.configuration.systusOutputMatrix=="table"){

        // Fill tables for Stiffness, Mass and Damping elements
        for (const auto& elementSet : systusModel.model->elementSets) {


            switch (elementSet->type) {
            // None of these elements needs a table
            case ElementSet::CIRCULAR_SECTION_BEAM:
            case ElementSet::GENERIC_SECTION_BEAM:
            case ElementSet::I_SECTION_BEAM:
            case ElementSet::RECTANGULAR_SECTION_BEAM:
            case ElementSet::STRUCTURAL_SEGMENT:
            case ElementSet::SHELL:
            case ElementSet::CONTINUUM:
            case ElementSet::NODAL_MASS: {
                continue;
            }

            // Those elements are not supported yet
            case ElementSet::DISCRETE_0D:
            case ElementSet::DISCRETE_1D:{
                continue;
            }

            // Stiffness, Mass and Damping matrices are the same kind.
            // Only the tableByElementSet value differs:
            //   - Stiffness: 0000XX
            //   - Mass     : 00XX00
            //   - Damping  : XX0000
            case ElementSet::STIFFNESS_MATRIX:{
                shared_ptr<StiffnessMatrix> sm = static_pointer_cast<StiffnessMatrix>(elementSet);
                long unsigned int tId= tables.size()+1;
                SystusTable aTable = SystusTable(tId);

                //Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : sm->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the table
                for (const auto np : sm->nodePairs()){
                    int pairCode = positionToSytusNumber[np.first]*1000 + positionToSytusNumber[np.second]*100;
                    shared_ptr<DOFMatrix> dM = sm->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofCode = 10*DOFToInt(dof.first.first) + DOFToInt(dof.first.second);
                        aTable.add(pairCode+dofCode);
                        aTable.add(dof.second);
                    }
                }
                tables.push_back(aTable);
                tableByElementSet[elementSet->getId()]=tId;
                break;
            }
            case ElementSet::MASS_MATRIX:{
                shared_ptr<MassMatrix> mm = static_pointer_cast<MassMatrix>(elementSet);
                long unsigned int tId= tables.size()+1;
                SystusTable aTable = SystusTable(tId);

                //Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : mm->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the table
                for (const auto np : mm->nodePairs()){
                    int pairCode = positionToSytusNumber[np.first]*1000 + positionToSytusNumber[np.second]*100;
                    shared_ptr<DOFMatrix> dM = mm->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofCode = 10*DOFToInt(dof.first.first) + DOFToInt(dof.first.second);
                        aTable.add(pairCode+dofCode);
                        aTable.add(dof.second);
                    }
                }
                tables.push_back(aTable);
                tableByElementSet[elementSet->getId()]=tId*100;
                break;
            }
            case ElementSet::DAMPING_MATRIX:{
                shared_ptr<DampingMatrix> dm = static_pointer_cast<DampingMatrix>(elementSet);
                long unsigned int tId= tables.size()+1;
                SystusTable aTable = SystusTable(tId);

                //Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : dm->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the table
                for (const auto np : dm->nodePairs()){
                    int pairCode = positionToSytusNumber[np.first]*1000 + positionToSytusNumber[np.second]*100;
                    shared_ptr<DOFMatrix> dM = dm->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofCode = 10*DOFToInt(dof.first.first) + DOFToInt(dof.first.second);
                        aTable.add(pairCode+dofCode);
                        aTable.add(dof.second);
                    }
                }
                tables.push_back(aTable);
                tableByElementSet[elementSet->getId()]=tId*10000;
                break;
            }

            default: {
                //TODO : throw WriterException("ElementSet type not supported");
                cout << "Warning in FillTables: " << *elementSet << " not supported" << endl;
            }
            }
        }
    }
}



void SystusWriter::fillMatrices(const SystusModel& systusModel, const int idSubcase){

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that only one kind of matrix  are translated yet,
    // and we don't know what the others will need.
    UNUSEDV(idSubcase);

    int nbDOFS;
    if (systusOption==3){
        nbDOFS=6;
    }else{
        nbDOFS=3;
    }
    dampingMatrices.nbDOFS=nbDOFS;
    massMatrices.nbDOFS=nbDOFS;
    stiffnessMatrices.nbDOFS=nbDOFS;

    // Fill tables for Stiffness, Mass and Damping elements
    if (systusModel.configuration.systusOutputMatrix=="file"){
        for (const auto& elementSet : systusModel.model->elementSets) {

            switch (elementSet->type) {
            // None of these elements needs to output matrices
            case ElementSet::CIRCULAR_SECTION_BEAM:
            case ElementSet::GENERIC_SECTION_BEAM:
            case ElementSet::I_SECTION_BEAM:
            case ElementSet::RECTANGULAR_SECTION_BEAM:
            case ElementSet::STRUCTURAL_SEGMENT:
            case ElementSet::SHELL:
            case ElementSet::CONTINUUM:
            case ElementSet::NODAL_MASS: {
                continue;
            }

            // Those elements are not supported yet
            case ElementSet::DISCRETE_0D:
            case ElementSet::DISCRETE_1D:{
                continue;
            }

            // Stiffness, Mass and Damping matrices are the same kind.
            // Only the tableByElementSet value differs:
            //   - Stiffness: -0000XX
            //   - Mass     : -00XX00
            //   - Damping  : -XX0000
            case ElementSet::DAMPING_MATRIX:{
                shared_ptr<DampingMatrix> dam = static_pointer_cast<DampingMatrix>(elementSet);
                long unsigned int seId= dampingMatrices.size()+1;

                // Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : dam->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the Systus Matrix
                SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
                for (const auto np : dam->nodePairs()){
                    int nI = positionToSytusNumber[np.first];
                    int nJ = positionToSytusNumber[np.second];
                    shared_ptr<DOFMatrix> dM = dam->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofI = DOFToInt(dof.first.first);
                        int dofJ = DOFToInt(dof.first.second);
                        aMatrix.setValue(nI, nJ, dofI, dofJ, dof.second);
                        aMatrix.setValue(nJ, nI, dofJ, dofI, dof.second);
                    }
                }

                tableByElementSet[elementSet->getId()]=-SystusWriter::DampingAccessId*10000;
                seIdByElementSet[elementSet->getId()]= seId;
                dampingMatrices.add(aMatrix);
                break;
            }

            case ElementSet::MASS_MATRIX:{
                shared_ptr<MassMatrix> mm = static_pointer_cast<MassMatrix>(elementSet);
                long unsigned int seId= massMatrices.size()+1;

                // Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : mm->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the Systus Matrix
                SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
                for (const auto np : mm->nodePairs()){
                    int nI = positionToSytusNumber[np.first];
                    int nJ = positionToSytusNumber[np.second];
                    shared_ptr<DOFMatrix> dM = mm->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofI = DOFToInt(dof.first.first);
                        int dofJ = DOFToInt(dof.first.second);
                        aMatrix.setValue(nI, nJ, dofI, dofJ, dof.second);
                        aMatrix.setValue(nJ, nI, dofJ, dofI, dof.second);
                    }
                }

                tableByElementSet[elementSet->getId()]=-SystusWriter::MassAccessId*100;
                seIdByElementSet[elementSet->getId()]= seId;
                massMatrices.add(aMatrix);
                break;
            }

            case ElementSet::STIFFNESS_MATRIX:{
                shared_ptr<StiffnessMatrix> sm = static_pointer_cast<StiffnessMatrix>(elementSet);
                long unsigned int seId= stiffnessMatrices.size()+1;

                // Numbering the node internally to the element
                map<int, int> positionToSytusNumber;
                int iSystus= 1;
                for (const int pos : sm->nodePositions()){
                    positionToSytusNumber[pos]=iSystus;
                    iSystus++;
                }

                // Building the Systus Matrix
                SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
                for (const auto np : sm->nodePairs()){
                    int nI = positionToSytusNumber[np.first];
                    int nJ = positionToSytusNumber[np.second];
                    shared_ptr<DOFMatrix> dM = sm->findSubmatrix(np.first, np.second);
                    for (const auto dof: dM->componentByDofs){
                        int dofI = DOFToInt(dof.first.first);
                        int dofJ = DOFToInt(dof.first.second);
                        aMatrix.setValue(nI, nJ, dofI, dofJ, dof.second);
                        aMatrix.setValue(nJ, nI, dofJ, dofI, dof.second);
                    }
                }

                tableByElementSet[elementSet->getId()]=-SystusWriter::StiffnessAccessId;
                seIdByElementSet[elementSet->getId()]= seId;
                stiffnessMatrices.add(aMatrix);
                break;
            }


            default: {
                //TODO : throw WriterException("ElementSet type not supported");
                cout << "Warning in FillTables: " << *elementSet << " not supported" << endl;
            }
            }
        }
    }
}



// Cleaning from previous analysis
void SystusWriter::clear(){

    // Clear loads
    localLoadingIdByLoadsetIdByAnalysisId.clear();
    localLoadingListName.clear();

    // Clear constraints nodes
    constraintByNodePosition.clear();

    // Clear vectors
    vectors.clear();
    localVectorIdByNodePosition.clear();
    loadingVectorIdByLocalLoading.clear();
    loadingVectorsIdByLocalLoadingByNodePosition.clear();
    constraintVectorsIdByLocalLoadingByNodePosition.clear();

    // Clear lists
    lists.clear();
    loadingListIdByNodePosition.clear();
    constraintListIdByNodePosition.clear();

    // Clear tables
    tables.clear();
    tableByElementSet.clear();

    // Clear matrices
    seIdByElementSet.clear();
    dampingMatrices.clear();
    massMatrices.clear();
    stiffnessMatrices.clear();

}

void SystusWriter::translate(const SystusModel &systusModel, const int idSubcase){

    this->clear();

    fillMatrices(systusModel, idSubcase);

    fillLoads(systusModel, idSubcase);

    fillConstraintsNodes(systusModel, idSubcase);

    fillVectors(systusModel, idSubcase);

    fillLists(systusModel, idSubcase);

    fillTables(systusModel, idSubcase);
}



void SystusWriter::writeAsc(const SystusModel &systusModel, const vega::ConfigurationParameters &configuration,
        const int idSubcase, ostream& out) {

    writeHeader(systusModel, out);

    writeInformations(configuration, idSubcase, out);

    writeNodes(systusModel, out);

    writeElements(systusModel, out);

    writeGroups(systusModel, out);

    writeMaterials(systusModel, configuration, out);

    out << "BEGIN_MEDIA 0" << endl;
    out << "END_MEDIA" << endl;

    writeLoads(out);

    writeLists(out);

    writeVectors(out);

    out << "BEGIN_RELEASES 0" << endl;
    out << "END_RELEASES" << endl;

    writeTables(out);

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

void SystusWriter::writeInformations(const ConfigurationParameters &configuration, int idSubcase , ostream& out) {
    out << "BEGIN_INFORMATIONS" << endl;

    //Subcase
    string ssubcase = " SC"+ to_string(idSubcase+1) +" ";

    // Logiciel version
    ostringstream otmp;
    otmp << "Built by VEGA "<<VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << "."
            << VEGA_VERSION_PATCH << " " << VEGA_VERSION_EXTRA<< " from ";
    std::string slogiciel = otmp.str();

    // Date
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[11];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,11,"%F",timeinfo);
    string sdate(buffer);

    // We have 80 characters
    long unsigned sizeleft =  80 - ssubcase.length() - sdate.length() - slogiciel.length();
    out << slogiciel << configuration.inputFile.substr(0, sizeleft) << ssubcase << sdate<< endl;


    out << " " << systusOption << " 0 0 1 0 0 0 0 0 0 0 1 0 "<< systusSubOption<<" 0 0 0 0 0 0" << endl;
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

    for (const auto& node : mesh->nodes) {
        Node nNode = mesh->findNode(node.position, true, systusModel.model);
        int nid = nNode.id;
        int iconst = 0;
        auto it = constraintByNodePosition.find(node.position);
        if (it != constraintByNodePosition.end())
            iconst = int(it->second);
        int imeca = 0;
        long unsigned int iangl = 0;
        if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
            iangl = localVectorIdByNodePosition[node.position];
        }
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
        out << nNode.x << " " << nNode.y << " " << nNode.z << endl;

        // Small warning against "infinite" node.
        if (nNode.x < -1.0e+300){
            handleWritingWarning("Infinite node with Id: " + std::to_string(nid),"Nodes");
        }
    }

    out << "END_NODES" << endl;
}


void SystusWriter::writeElementLocalReferentiel(const SystusModel& systusModel,
        const int dim, const int celltype, const vector<int> nodes, const int cpos, ostream& out){

    shared_ptr<CoordinateSystem> cs = systusModel.model->getCoordinateSystemByPosition(cpos);
    if (cs== nullptr){
        out << " 0";
        handleWritingWarning("Unknown coordinate system of position " +to_string(cpos), "Angle Elements");
        return;
    }

    if ((cs->type!=CoordinateSystem::CARTESIAN) and (cs->type!=CoordinateSystem::ORIENTATION)){
        out << " 0";
        handleWritingWarning("Coordinate System "+ to_string(cs->type) + " is not supported. Referentiel dismissed.", "Angle Elements");
        return;
    }
    
    CartesianCoordinateSystem rcs = buildElementDefaultReferentiel(systusModel, nodes, dim, celltype);
    VectorialValue angles = cs->getEulerAnglesIntrinsicZYX(&rcs); // (PSI, THETA, PHI)
    
    switch (dim) {

    // Orientation are not allowed for 0d elements.
    case 0: {
        if (cs->type==CoordinateSystem::ORIENTATION){
            out << " 0";
            handleWritingWarning("Local orientation are not defined for 0D elements.",  "Angle Elements");
        }else{
            out << " 3 "<< angles.x() <<" " << angles.y()<< " "<< angles.z();
        }
        break;
    }

    // For 1D elements, only PHI should be defined
    case 1: {
        // Except 16XX elements which need a full coordinate system
        if (celltype==6){
            out << " 3 "<< angles.x() <<" " << angles.y()<< " "<< angles.z();
        }else{
            if (!is_zero(angles.x()) or !is_zero(angles.y())){
                handleWritingWarning("Orientation of 1D Elements must be defined through the PHI angle only. PSI and THETA angles are dismissed.", "Angle Elements");
            }
            out << " 3 0.0 0.0 " << angles.z();
        }
        break;
    }

    // For 2D elements, only PSI should be defined
    case 2: {
        if (!is_zero(angles.y()) or !is_zero(angles.z())){
            handleWritingWarning("Orientation of 2D Elements must be defined through the PSI angle only. THETA and PHI angles are dismissed.", "Angle Elements");
        }
        out << " 1 " << angles.x();
        break;
    }

    // For 3D elements, three angles can be defined.
    case 3: {
        out << " 3 "<< angles.x() <<" " << angles.y()<< " "<< angles.z();
        break;

    }

    // Should never happen
    default: {
        out << " 0";
        handleWritingWarning("Dimension should be between 0 and 3", "Angle Elements");
    }
    }


}




void SystusWriter::writeElements(const SystusModel& systusModel, ostream& out) {
    shared_ptr<Mesh> mesh = systusModel.model->mesh;
    out << "BEGIN_ELEMENTS " << mesh->countCells() << endl;
    for (const auto& elementSet : systusModel.model->elementSets) {

        CellGroup* cellGroup = elementSet->cellGroup;
        int dim = 0;
        int typecell=0;


        switch (elementSet->type) {
        case ElementSet::CIRCULAR_SECTION_BEAM:
        case ElementSet::GENERIC_SECTION_BEAM:
        case ElementSet::I_SECTION_BEAM:
        case ElementSet::RECTANGULAR_SECTION_BEAM: {
            dim = 1;
            break;
        }
        case ElementSet::STRUCTURAL_SEGMENT:{
            typecell=6;
            // We treat only Stiffness matrixes for now, with 1602 and 0601 Elements
            shared_ptr<StructuralSegment> se = static_pointer_cast<StructuralSegment>(elementSet);
            if ((se->hasMass()) or (se->hasDamping())){
                cerr << "WARNING in Structural Elements: mass and damping are not supported and will be dismissed."<<endl;
            }
            break;
        }
        case ElementSet::SHELL: {
            dim = 2;
            typecell=4;
            break;
        }
        case ElementSet::CONTINUUM: {
            dim = 3;
            break;
        }
        case ElementSet::NODAL_MASS:{
            continue;
        }
        case ElementSet::DISCRETE_0D:
        case ElementSet::DISCRETE_1D:{
            continue;
        }    
        case ElementSet::STIFFNESS_MATRIX:
        case ElementSet::MASS_MATRIX:
        case ElementSet::DAMPING_MATRIX:{
            dim= 3;
            typecell = 9;
            break;
        }
        default: {
            //TODO : throw WriterException("ElementSet type not supported");
            cout << "Warning in Elements: " << *elementSet << " not supported" << endl;
            dim= 3;
            typecell = 0;
        }
        }
        for (const Cell& cell : cellGroup->getCells()) {
            auto systus2med_it = systus2medNodeConnectByCellType.find(cell.type.code);
            if (systus2med_it == systus2medNodeConnectByCellType.end()) {
                cout << "Warning in Elements: " << cell << " not supported in Systus" << endl;
                continue;
            }

            // Putting all nodes in the Systus order
            vector<int> systus2medNodeConnect = systus2med_it->second;
            vector<int> medConnect = cell.nodeIds;
            vector<int> systusConnect;
            for (unsigned int i = 0; i < cell.type.numNodes; i++)
                systusConnect.push_back(medConnect[systus2medNodeConnect[i]]);

            if (elementSet->type==ElementSet::STRUCTURAL_SEGMENT){
                dim = (cell.nodeIds.size()==2) ? 1 : 0 ;
            }

            out << cell.id << " " << dim << typecell;              // Dimension and type of cell;
            out << setfill('0') << setw(2) << cell.nodeIds.size(); // Number of nodes in two caracters: 01, 02, 05, 10, etc.

            if (cell.nodeIds.size()>20){
                cerr<< "Warning in Elements: " << cell << " has " << cell.nodeIds.size() << " but SYSTUS only support up to 20 nodes by element."<<endl;
            }

            //TODO: We should write here the Material Id: we use the elementSet id which SHOULD be the same
            out << " " << elementSet->getId(); // Material Id (it's an ugly fix)
            out << " 0"; // Loading List:  index that describes solicitation list (not supported yet)

            // Local Orientation
            if (cell.hasOrientation){
                writeElementLocalReferentiel(systusModel, dim, typecell, systusConnect, cell.cid, out);
            }else{
                out << " 0";
            } 

            // Writing Nodes
            for (int node : systusConnect) {
                out << " " << node;
            }
            out << endl;
        }
    }

    // Adding rbars elements corresponding to rbe2 and rbe3
    for (const auto& rbe2 : RBE2rbarPositions){
        for (int position : rbe2.second){
            Cell cell = mesh->findCell(position);
            out << cell.id << " 190" << cell.nodeIds.size() << " " << rbe2.first << " 0 0";
            for (int nodeId : cell.nodeIds)
                out << " " << nodeId;
            out << endl;
        }
    }

    for (const auto& rbar : RbarPositions){
        for (int position : rbar.second){
            Cell cell = mesh->findCell(position);
            out << cell.id << " 190" << cell.nodeIds.size() << " " << rbar.first << " 0 0";
            for (int nodeId : cell.nodeIds)
                out << " " << nodeId;
            out << endl;
        }
    }

    for (const auto& rbe3 : RBE3rbarPositions){
        for (int position : rbe3.second){
            Cell cell = mesh->findCell(position);
            out << cell.id << " 190" << cell.nodeIds.size() << " " << rbe3.first << " 0 0";
            for (int nodeId : cell.nodeIds)
                out << " " << nodeId;
            out << endl;
        }
    }

    out << "END_ELEMENTS" << endl;
}

// TODO: Add an option to only write the User groups, and not all vega-created groups.
void SystusWriter::writeGroups(const SystusModel& systusModel, ostream& out) {
    vector<NodeGroup*> nodeGroups = systusModel.model->mesh->getNodeGroups();
    vector<CellGroup*> cellGroups = systusModel.model->mesh->getCellGroups();

    ostringstream osgr;
    int nbGroups=0;

    // Write CellGroups
    set<int> pids= {};
    for (const auto& cellGroup : cellGroups) {
        // We don't write the groups of Nodal Mass, as they are not cells in Systus
        // We don't write the Orientation groups, they are not parts.
        // It IS an Ugly Fix. I know it is.
        // TODO: DO better
        if ((cellGroup->getComment().substr(0,10)!="NODAL MASS")&&
                (cellGroup->getComment()!="Orientation")){
            nbGroups++;
            osgr << nbGroups << " " << cellGroup->getName() << " 2 0 ";
            osgr << "\"PART_ID "<< getPartId(cellGroup->getName(), pids) << "\"  \"\"  ";
            osgr << "\"PART built in VEGA from "<< cellGroup->getComment() << "\"";

            for (const auto& cell : cellGroup->getCells())
                osgr << " " << cell.id;
            osgr << endl;
        }
    }
    //cout << pids << endl;
    // Write NodeGroups
    for (const auto& nodeGroup : nodeGroups) {
        nbGroups++;
        osgr << nbGroups << " " << nodeGroup->getName() << " 1 0 ";
        osgr << "\"No method\"  \"\"  ";
        osgr << "\"Group built in VEGA from "<< nodeGroup->getComment() << "\"";
        for (int id : nodeGroup->getNodeIds())
            osgr << " " << id;
        osgr << endl;
    }

    // Stream to output
    out << "BEGIN_GROUPS " << nbGroups << endl;
    out <<  osgr.str();
    out << "END_GROUPS" << endl;
}


void SystusWriter::writeMaterials(const SystusModel& systusModel,
        const vega::ConfigurationParameters &configuration, ostream& out) {

    ostringstream ogmat;
    ogmat.precision(DBL_DIG);
    int nbmaterials= 0;
    int nbelements = 0;

    for (const auto& elementSet : systusModel.model->elementSets) {
        const auto& material = elementSet->material;
        if (elementSet->cellGroup != nullptr){

            ostringstream omat;
            omat.precision(DBL_DIG);
            int nbElementsMaterial=0;
            bool isValid=true;
            // Elements with VEGA material
            if (material != nullptr){
                const shared_ptr<Nature> nature = material->findNature(Nature::NATURE_ELASTIC);
                if (nature) {
                    const ElasticNature& elasticNature = dynamic_cast<ElasticNature&>(*nature);

                    writeMaterialField(SMF::ID, elementSet->getId(), nbElementsMaterial, omat);
                    writeMaterialField(SMF::MID, elementSet->getId(), nbElementsMaterial, omat);
                    writeMaterialField(SMF::RHO, elasticNature.getRho(), nbElementsMaterial, omat);
                    writeMaterialField(SMF::E, elasticNature.getE(), nbElementsMaterial, omat);
                    writeMaterialField(SMF::NU, elasticNature.getNu(), nbElementsMaterial, omat);
                    writeMaterialField(SMF::ALPHA, 2*elasticNature.getGE(), nbElementsMaterial, omat);

                    switch (elementSet->type) {
                    case (ElementSet::GENERIC_SECTION_BEAM): {
                        shared_ptr<const GenericSectionBeam> genericBeam = static_pointer_cast<
                                const GenericSectionBeam>(elementSet);
                        const double S= genericBeam->getAreaCrossSection();
                        writeMaterialField(SMF::S, S, nbElementsMaterial, omat);

                        // VEGA stocks the inverse of our needed Shear Area Factors.
                        writeMaterialField(SMF::AY, S*genericBeam->getShearAreaFactorY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::AZ, S*genericBeam->getShearAreaFactorZ(), nbElementsMaterial, omat);

                        writeMaterialField(SMF::IX, genericBeam->getTorsionalConstant(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IY, genericBeam->getMomentOfInertiaY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IZ, genericBeam->getMomentOfInertiaZ(), nbElementsMaterial, omat);
                        break;
                    }
                    case (ElementSet::CIRCULAR_SECTION_BEAM): {
                        shared_ptr<const CircularSectionBeam> circularBeam = static_pointer_cast<
                                const CircularSectionBeam>(elementSet);
                        const double S= circularBeam->getAreaCrossSection();

                        writeMaterialField(SMF::S, S, nbElementsMaterial, omat);
                        writeMaterialField(SMF::AY, S*circularBeam->getShearAreaFactorY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::AZ, S*circularBeam->getShearAreaFactorZ(), nbElementsMaterial, omat);

                        writeMaterialField(SMF::IX, circularBeam->getTorsionalConstant(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IY, circularBeam->getMomentOfInertiaY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IZ, circularBeam->getMomentOfInertiaZ(), nbElementsMaterial, omat);
                        break;
                    }
                    case (ElementSet::RECTANGULAR_SECTION_BEAM): {
                        shared_ptr<const RectangularSectionBeam> rectangularBeam = static_pointer_cast<
                                const RectangularSectionBeam>(elementSet);
                        const double S= rectangularBeam->getAreaCrossSection();

                        writeMaterialField(SMF::S, S, nbElementsMaterial, omat);
                        writeMaterialField(SMF::AY, S*rectangularBeam->getShearAreaFactorY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::AZ, S*rectangularBeam->getShearAreaFactorZ(), nbElementsMaterial, omat);

                        writeMaterialField(SMF::IX, rectangularBeam->getTorsionalConstant(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IY, rectangularBeam->getMomentOfInertiaY(), nbElementsMaterial, omat);
                        writeMaterialField(SMF::IZ, rectangularBeam->getMomentOfInertiaZ(), nbElementsMaterial, omat);
                        break;
                    }

                    case (ElementSet::SHELL): {
                        shared_ptr<const Shell> shell = static_pointer_cast<const Shell>(elementSet);
                        writeMaterialField(SMF::H, shell->thickness, nbElementsMaterial, omat);
                        break;
                    }
                    case (ElementSet::CONTINUUM): {
                        break;
                    }
                    // Nodal Masses are not material in Systus
                    case (ElementSet::NODAL_MASS): {
                        continue;
                    }
                    // Default : we only print the default fields: E, NU, etc.
                    default:
                        cout << "Warning in Elastic Materials: " << *elementSet << " not supported." << endl;
                    }
                }else{
                    isValid=false;
                    cout << "Warning in Materials: " << *elementSet << " has not an Elastic nature." << endl;
                }
            }else{
                // Element without VEGA Material
                writeMaterialField(SMF::ID, elementSet->getId(), nbElementsMaterial, omat);
                writeMaterialField(SMF::MID, elementSet->getId(), nbElementsMaterial, omat);
                switch (elementSet->type){
                case (ElementSet::STRUCTURAL_SEGMENT):{


                    shared_ptr<const StructuralSegment> sS = static_pointer_cast<const StructuralSegment>(elementSet);
                    // K Matrix
                    writeMaterialField(SMF::IX, sS->findStiffness(DOF::RX, DOF::RX), nbElementsMaterial, omat);
                    writeMaterialField(SMF::IY, sS->findStiffness(DOF::RY, DOF::RY), nbElementsMaterial, omat);
                    writeMaterialField(SMF::IZ, sS->findStiffness(DOF::RZ, DOF::RZ), nbElementsMaterial, omat);
                    writeMaterialField(SMF::KX, sS->findStiffness(DOF::DX, DOF::DX), nbElementsMaterial, omat);
                    writeMaterialField(SMF::KY, sS->findStiffness(DOF::DY, DOF::DY), nbElementsMaterial, omat);
                    writeMaterialField(SMF::KZ, sS->findStiffness(DOF::DZ, DOF::DZ), nbElementsMaterial, omat);

                    // A few warning
                    if (sS->hasMass()){
                        cout << "Warning in Materials: mass in " << *elementSet << " is not supported and will be dismissed." << endl;
                    }
                    if (sS->hasDamping()){
                        cout << "Warning in Materials: damping in " << *elementSet << " is not supported and will be dismissed." << endl;
                    }

                    break;
                }

                // For matrix elements, material is only a "pointer" to a table.
                case ElementSet::STIFFNESS_MATRIX:
                case ElementSet::MASS_MATRIX:
                case ElementSet::DAMPING_MATRIX:{
                    auto it = tableByElementSet.find(elementSet->getId());
                    if (it == tableByElementSet.end()){
                        cout << "Warning in Materials: "<< *elementSet << " has no table."<<endl;
                        break;
                    }
                    writeMaterialField(SMF::TABLE, int(it->second), nbElementsMaterial, omat);
                    if (systusModel.configuration.systusOutputMatrix=="file"){
                        auto it2 = seIdByElementSet.find(elementSet->getId());
                        if (it2 == seIdByElementSet.end()){
                            cout << "Warning in Materials: "<< *elementSet << " has no reduction number."<<endl;
                            break;
                        }
                        writeMaterialField(SMF::E, double(it2->second), nbElementsMaterial, omat);
                    }
                    break;
                }
                default:{
                    isValid=false;
                    cout << "Warning in Materials: " << *elementSet << " not supported" << endl;
                }
                }
            }
            if (isValid){
                nbelements=nbelements+nbElementsMaterial;
                nbmaterials++;
                omat << endl;
                ogmat << omat.str();
            }
        }
    }


    // Adding rbars materials for rbe2s and rbe3s
    for (const auto& rbe2 : RBE2rbarPositions){
        nbmaterials++;
        int nbElementsMaterial=0;
        writeMaterialField(SMF::ID, rbe2.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::MID, rbe2.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::LEVEL, 1, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::TYPE, 9, nbElementsMaterial, ogmat);
        if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
            writeMaterialField(SMF::SHAPE, 19, nbElementsMaterial, ogmat);
            writeMaterialField(SMF::E, 1.0, nbElementsMaterial, ogmat);
        }else{
            writeMaterialField(SMF::SHAPE, 9, nbElementsMaterial, ogmat);
            writeMaterialField(SMF::E, configuration.systusRBE2Rigidity, nbElementsMaterial, ogmat);
        }
        ogmat << endl;
        nbelements=nbelements+nbElementsMaterial;
    }
    for (const auto& rbar : RbarPositions){
        nbmaterials++;
        int nbElementsMaterial=0;
        writeMaterialField(SMF::ID, rbar.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::MID, rbar.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::LEVEL, 1, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::TYPE, 9, nbElementsMaterial, ogmat);
        if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
            writeMaterialField(SMF::SHAPE, 19, nbElementsMaterial, ogmat);
            writeMaterialField(SMF::E, 1.0, nbElementsMaterial, ogmat);
        }else{
            writeMaterialField(SMF::SHAPE, 9, nbElementsMaterial, ogmat);
            writeMaterialField(SMF::E, configuration.systusRBE2Rigidity, nbElementsMaterial, ogmat);
        }
        ogmat << endl;
        nbelements=nbelements+nbElementsMaterial;
    }


    for (const auto& rbe3 : RBE3rbarPositions){
        nbmaterials++;
        int nbElementsMaterial=0;
        writeMaterialField(SMF::ID, rbe3.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::MID, rbe3.first, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::SHAPE, 19, nbElementsMaterial, ogmat);
        writeMaterialField(SMF::LEVEL, 3, nbElementsMaterial, ogmat);
        int nbFieldDOFS = DOFSToMaterial(RBE3Dofs[rbe3.first].front(), ogmat); // KX KY KZ IX IY IZ
        writeMaterialField(SMF::COEF, RBE3Coefs[rbe3.first], nbElementsMaterial, ogmat);
        writeMaterialField(SMF::DEPEND, DOFSToInt(RBE3Dofs[rbe3.first].back()), nbElementsMaterial, ogmat);
        nbelements=nbelements+nbElementsMaterial+nbFieldDOFS;
        ogmat << endl;
    }

    // Stream to output
    out << "BEGIN_MATERIALS " << nbmaterials << " " << nbelements << endl;
    out << ogmat.str();
    out << "END_MATERIALS" << endl;
}

void SystusWriter::writeLoads(ostream& out) {
    out << "BEGIN_LOADS ";
    // Number of written loads
    out << localLoadingListName.size() << endl;
    // Writing Loads
    for (const auto& load : localLoadingListName) {
        out << load.first << " \""<<load.second<< "\"";
        out << " 0 ";
        out << loadingVectorIdByLocalLoading[load.first];
        out << " 0 0 0 0 0 7" << endl;
    }
    out << "END_LOADS" << endl;

}

void SystusWriter::writeLists(ostream& out) {

    ostringstream olist;
    olist.precision(DBL_DIG);
    long unsigned int nbElements=0;
    for (const auto& list : lists) {
        olist << list.first;
        for (const long unsigned int d : list.second)
            olist << " " << d;
        olist << endl;
        nbElements = nbElements + (list.second.size()/2);
    }

    out << "BEGIN_LISTS ";
    out << lists.size() << " " << nbElements << endl;
    out << olist.str();
    out << "END_LISTS" << endl;
}

void SystusWriter::writeVectors(ostream& out) {
    out << "BEGIN_VECTORS " << vectors.size() << endl;
    for (const auto& vector : vectors) {
        out << vector.first;
        for (const auto& d : vector.second)
            out << " " << d;
        out << endl;
    }
    out << "END_VECTORS" << endl;
}

void SystusWriter::writeMasses(const SystusModel &systusModel, ostream& out) {
    // TODO : attention, la doc parle de dynamique. Prise en compte dans le poids en statique ???
    vector<shared_ptr<ElementSet>> masses = systusModel.model->filterElements(
            ElementSet::NODAL_MASS);
    out << "BEGIN_MASSES " << masses.size() << endl;
    if (masses.size() > 0) {
        for (const auto& mass : masses) {
            if (mass->cellGroup != nullptr) {
                shared_ptr<NodalMass> nodalMass = static_pointer_cast<NodalMass>(mass);
                //  VALUES NBR VAL(NBR) NODEi
                //  NBR:            Number of values [INTEGER]
                //  VAL(NBR):   Masses values [DOUBLE[NBR]]
                //  NODEi:      List of NODES index [INTEGER[*]]
                if (!is_zero(nodalMass->ixy) || !is_zero(nodalMass->iyz) || !is_zero(nodalMass->ixz)){
                    handleWritingError(
                            string("Asymetric masses are not (yet) implemented."));
                }
                if (!is_zero(nodalMass->ex) || !is_zero(nodalMass->ey) || !is_zero(nodalMass->ez)){
                    handleWritingWarning("Offset not implemented and dismissed.");
                }
                if (!is_zero(nodalMass->ixx) || !is_zero(nodalMass->iyy) || !is_zero(nodalMass->izz)){
                    out << "VALUES 6 " << nodalMass->getMass() << " " << nodalMass->getMass() << " "
                            << nodalMass->getMass() << " " << nodalMass->ixx << " " << nodalMass->iyy
                            << " " << nodalMass->izz;
                }else{
                    out << "VALUES 3 " << nodalMass->getMass() << " " << nodalMass->getMass() << " "
                            << nodalMass->getMass();
                }
                for (const auto& cell : mass->cellGroup->getCells()) {
                    // NODEi
                    out << " " << cell.nodeIds[0];
                }
                out << endl;
            }
        }
    }
    out << "END_MASSES" << endl;
}


void SystusWriter::writeTables(std::ostream& out){

    out << "BEGIN_TABLES " << tables.size()<<endl;
    for (const auto& table : tables){
        out << table;
    }
    out << "END_TABLES" << endl;
}



void SystusWriter::writeDat(const SystusModel& systusModel, const vega::ConfigurationParameters &configuration,
        const int idSubcase, ostream& out) {

    // For LEVELSHAPE, we comment a few lines.
    string comment="";
    if (configuration.systusOutputProduct=="levelshape"){
        comment="###LEVELSHAPE###";
    }

    // Same start for everyone
    out << comment<<"NAME " << systusModel.getName() << "_SC" << (idSubcase+1) << "_" << endl;
    out << endl;
    out << comment<<"SEARCH DATA 1 ASCII" << endl;
    out << endl;

    // Special case : if the subcase is void, it means that we are only translating a mesh
    // No analysis lines.
    if (systusSubcases[idSubcase].size()==0){
        return;
    }

    // If some elementary matrix are saved in files, we need to convert and load them
    if (configuration.systusOutputMatrix=="file"){
        out << "# ACCESS TO ELEMENTARY MATRIX FILES" << endl;
        for (const auto& it : filebyAccessId){
            out <<  "!filematrix ASC2BIN "<< it.second <<".ASC "<< it.second <<".TIT"<<endl;
            out << "ASSIGN "<< it.first << " "<< it.second <<".TIT BINARY"<<endl;
        }

    }


    // We find the first Analysis of the Subcase, which will be our reference
    const int idAnalysis = systusSubcases[idSubcase][0];
    const shared_ptr<Analysis> analysis = systusModel.model->getAnalysis(idAnalysis);
    if (analysis== nullptr){
        handleWritingError(string("Analysis " + to_string(idAnalysis) + " not found."));
    }

    switch (analysis->type) {
    case Analysis::LINEAR_MECA_STAT: {

        out << "SOLVE METHOD OPTIMISED" << endl;
        //out << "EXTRACT STIFFNESS ELEMENTS " <<endl;
        break;
    }
    case Analysis::LINEAR_MODAL:{
        out << "# SOLVE FILE TO USE THE DYNAMIC SOLVER" << endl;
        out << "# USE FOR EIGEN FREQUENCY CRITERION" << endl;
        out << endl;
        out << "# COMPUTING MASS MATRIX" << endl;
        out << "# AS THE COMMAND DYNAMIC COMPUTE THEM, IT SHOULD BE USELESS." << endl;
        out << "# BUT THERE SEEM TO BE BUGS ON THE COMMAND, SO WE USE EXPLICITLY THE COMMAND" << endl;
        out << "CLOSE STIFFNESS MASS" << endl;
        out << endl;
        out << "# COMPUTE MODES" << endl;
        out << "DYNAMIC" << endl;
        out << endl;

        const LinearModal& linearModal = static_cast<const LinearModal&>(*analysis);
        FrequencyBand& frequencyBand = *(linearModal.getFrequencyBand());
        string smodes= (frequencyBand.num_max == vega::Globals::UNAVAILABLE_INT ? "12":to_string(frequencyBand.num_max));
        string siters= (frequencyBand.num_max == vega::Globals::UNAVAILABLE_INT ? "24":to_string(2*frequencyBand.num_max));
        if (!is_equal(frequencyBand.upper, vega::Globals::UNAVAILABLE_DOUBLE)){
            cout << "WARNING: Modal analysis with upper bound frequency not supported. Will search for "<<smodes<<" Eigenmodes instead."<<endl;
            // Mechanical commands for Systus 2017
            out << "###SYSTUS2017#### THE NEXT COMMANDS ARE NOT AVAILABLE YET IN SYSTUS." << endl;
            out << "###SYSTUS2017#### WE KEEP THEM FOR THE SAKE OF COMPLETION." << endl;
            out << "###SYSTUS2017#### WE COMPUTE ALL MODES (max "<< smodes << ") under "<<frequencyBand.upper<<" Hz." << endl;
            out << "###SYSTUS2017###MODE SUBSPACE BAND" << endl;
            out << "###SYSTUS2017###METHOD OPTIMIZED" << endl;
            out << "###SYSTUS2017###VECTOR "<< smodes << "PRECISION 1*-5 STURM FREQ " << frequencyBand.upper;
            out << "###SYSTUS2017###RETURN" << endl;
        }
        if (!is_equal(frequencyBand.lower, vega::Globals::UNAVAILABLE_DOUBLE) && !is_equal(frequencyBand.lower, 0)){
            cout << "WARNING: Modal analysis with lower bound frequency not supported. Will search for "<<smodes<<" Eigenmodes instead."<<endl;
        }

        out << "# WE COMPUTE "<< smodes << " MODES." << endl;
        out << "# IT'S AN ITERATIVE MEHOD, WITH A MAXIMUM OF "<< siters <<" ITERATIONS" << endl;
        out << "MODE SUBSPACE BLOCK 6" << endl;
        out << "METHOD OPTIMIZED" << endl;
        out << "VECTOR "<< smodes <<" ITER "<< siters <<" PRECISION 1*-5 NORM MASS"<< endl;
        out << "RETURN" << endl;

        out << endl;
        out << "# COMPUTE THE STRESS TENSORS." << endl;
        out << "# MANDATORY TO COMPUTE THE GRADIENTS OF THE FREQUENCY CRITERIONS." << endl;
        out << "SOLVE FORCE" << endl;
        break;
    }
    // Not Done Yet
    case Analysis::LINEAR_DYNA_MODAL_FREQ: {
        cout << "WARNING: Dynamic modal analysis is not supported."<<endl;

        out << "# DYNAMIC MODAL ANALYSIS IS NOT SUPPORTED." << endl;
        out << "# NO SYSTUS COMMANDS GENERATED." << endl;
        out << endl;

        //TODO: Disable for now. Is it useful?
        //      if (analysis.type == Analysis::LINEAR_DYNA_MODAL_FREQ){
        //
        //          const LinearDynaModalFreq& linearDynaModalFreq = static_cast<const LinearDynaModalFreq&>(analysis);
        //
        //          out << "DYNAMIC" << endl;
        //          out << "PARTICIPATION DOUBLE FORCE DISPL" << endl;
        //          out << "RETURN" << endl;
        //          out << "SOLVE FORCE MODAL" << endl << endl;
        //
        //          out << "DYNAMIC" << endl;
        //          out << "HARMONIC RESPONSE MODAL FORCE DISPL" << endl;
        //
        //          shared_ptr<StepRange> freqValueSteps = linearDynaModalFreq.getFrequencyValues()->getStepRange();
        //          out << "FREQUENCY INITIAL " << freqValueSteps->start - freqValueSteps->step << endl;
        //          out << " " << freqValueSteps->end << " STEP " << freqValueSteps->step << endl;
        //
        //          shared_ptr<FunctionTable> modalDampingTable = linearDynaModalFreq.getModalDamping()->getFunctionTable();
        //          out << "DAMPING MODAL" << endl;
        //          for (auto it = modalDampingTable->getBeginValuesXY(); it != modalDampingTable->getEndValuesXY(); it++){
        //              out << " " << it->first << " / (GAMMA) " << it->second << endl;
        //              cout << "modal damping must be defined by {mode num}/value and not frequence/value " << endl;
        //          }
        //          out << endl;
        //
        //          for (auto it : DynamicExcitationByLoadId){
        //              cout << "phase and amplitude not taken into account for load " << it.first << endl;
        //              it.second->getDynaPhase();
        //              it.second->getFunctionTableB();
        //          }
        //
        //          out << "TRANSFER STATIONARY" << endl;
        //          out << "DISPLACEMENT" << endl;
        //          out << "RETURN" << endl << endl;
        //
        //          string("Analysis " + Analysis::stringByType.at(analysis.type) + " not (finish) implemented");
        //      }
        //      out << "SAVE DATA RESU " << analysis.getId() << endl;
        //      out << "CONVERT RESU" << endl;
        //      out << "POST " << analysis.getId() << endl;
        //      out << "RETURN" << endl << endl;
        break;
    }
    default:
        handleWritingError(
                string("Analysis " + Analysis::stringByType.at(analysis->type) + " not (yet) implemented"));
    }

    // We Save Results
    out << endl;
    out << "# SAVING RESULT" << endl;
    out << comment<<"SAVE DATA RESU 1" << endl;
    out << endl;

    // We Post-Treat Results
    out << endl;
    out << "# CONVERSION OF RESULTS FOR POST-PROCESSING" << endl;
    out << comment<<"CONVERT RESU" << endl;
    out << comment<<"POST 1" << endl;
    out << comment<<"RETURN" << endl;
    out << endl;


    vector<shared_ptr<Assertion>> assertions = analysis->getAssertions();
    if (!assertions.empty()) {
        out << "LANGAGE" << endl;
        out << "variable displacement[" << numberOfDofBySystusOption[systusOption] << "],"
                "frequency, phase[" << numberOfDofBySystusOption[systusOption] << "];" << endl;
        out << "iResu=open_file(\"" << systusModel.getName() << "_" << analysis->getId()
                                        << ".RESU\", \"write\");" << endl << endl;

        for (const auto& assertion : assertions) {
            switch (assertion->type) {
            case Assertion::NODAL_DISPLACEMENT_ASSERTION:
                writeNodalDisplacementAssertion(*assertion, out);
                break;
            case Assertion::FREQUENCY_ASSERTION:
                if (analysis->type == Analysis::LINEAR_DYNA_MODAL_FREQ)
                    break;
                writeFrequencyAssertion(*assertion, out);
                break;
            case Assertion::NODAL_COMPLEX_DISPLACEMENT_ASSERTION:
                writeNodalComplexDisplacementAssertion(*assertion, out);
                break;
            default:
                handleWritingError(string("Not implemented"));
            }
            out << endl;
        }

        out << "close_file(iResu)" << endl;
        out << "end;" << endl;
    }
}





void SystusWriter::writeNodalDisplacementAssertion(Assertion& assertion, ostream& out) {
    NodalDisplacementAssertion& nda = dynamic_cast<NodalDisplacementAssertion&>(assertion);

    if (!is_equal(nda.instant, -1))
        handleWritingError("Instant in NodalDisplacementAssertion not supported");
    int nodePos = getAscNodeId(nda.nodePosition);
    int dofPos = getAscNodeId(nda.dof.position);

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

    int nodePos = getAscNodeId(ncda.nodePosition);
    int dofPos = getAscNodeId(ncda.dof.position);
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




void SystusWriter::writeMatrixFiles(const SystusModel& systusModel, const int idSubcase){

    /* Writing Damping Matrices */
    if (dampingMatrices.size()>0){
        ofstream ofsMatrixFile;
        ofsMatrixFile.precision(DBL_DIG);
        string matrixFile = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + "_DAMGEN.ASC");
        ofsMatrixFile.open(matrixFile.c_str(), ios::trunc);

        if (!ofsMatrixFile.is_open()) {
            string message = string("Can't open file ") + matrixFile + " for writing.";
            throw ios::failure(message);
        }
        ofsMatrixFile << dampingMatrices<<endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::DampingAccessId]= matrixFile;
    }

    /* Writing Mass Matrices */
    if (massMatrices.size()>0){
        ofstream ofsMatrixFile;
        ofsMatrixFile.precision(DBL_DIG);
        string baseFile = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + "_MASGEN");
        string matrixFile = baseFile+".ASC";
        ofsMatrixFile.open(matrixFile.c_str(), ios::trunc);

        if (!ofsMatrixFile.is_open()) {
            string message = string("Can't open file ") + matrixFile + " for writing.";
            throw ios::failure(message);
        }
        ofsMatrixFile << massMatrices << endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::MassAccessId]= baseFile;
    }

    /* Writing Stiffness Matrices */
    if (stiffnessMatrices.size()>0){
        ofstream ofsMatrixFile;
        ofsMatrixFile.precision(DBL_DIG);
        string baseFile = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + "_STIGEN");
        string matrixFile = baseFile+".ASC";
        ofsMatrixFile.open(matrixFile.c_str(), ios::trunc);

        if (!ofsMatrixFile.is_open()) {
            string message = string("Can't open file ") + matrixFile + " for writing.";
            throw ios::failure(message);
        }
        ofsMatrixFile << stiffnessMatrices <<endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::StiffnessAccessId]= baseFile;
    }
}


} //namespace Vega
