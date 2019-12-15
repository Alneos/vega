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
 * NastranParser_geometry.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

//#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "NastranParser.h"

using namespace std;
namespace alg = boost::algorithm;
using boost::to_upper;

namespace vega {

namespace nastran {

const unordered_map<CellType::Code, vector<int>, EnumClassHash> NastranParser::nastran2medNodeConnectByCellType =
        {
                { CellType::Code::TRI3_CODE, { 0, 1, 2 } },
                { CellType::Code::TRI6_CODE, { 0, 1, 2, 3, 4, 5 } },
                { CellType::Code::QUAD4_CODE, { 0, 1, 2, 3 } },
                { CellType::Code::QUAD8_CODE, { 0, 1, 2, 3, 4, 5, 6, 7 } },
                { CellType::Code::QUAD9_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8 } },
                { CellType::Code::TETRA4_CODE, { 0, 2, 1, 3 } },
                { CellType::Code::TETRA10_CODE, { 0, 2, 1, 3, 6, 5, 4, 7, 9, 8 } },
                { CellType::Code::PYRA5_CODE, { 0, 3, 2, 1, 4 } },
                { CellType::Code::PYRA13_CODE, { 0, 3, 2, 1, 4, 8, 7, 6, 5, 9, 12, 11, 10 } },
                { CellType::Code::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
                { CellType::Code::PENTA15_CODE, { 0, 2, 1, 3, 5, 4, 8, 7, 6, 12, 14, 13, 11, 10, 9 } },
                { CellType::Code::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
                { CellType::Code::HEXA20_CODE, { 0, 3, 2, 1, 4, 7, 6, 5, 11, 10, 9, 8, 16, 19, 18, 17, 15,
                        14, 13, 12 } }
        };

void NastranParser::parseGRDSET(NastranTokenizer& tok, Model& model) {
    tok.skip(1);
    grdSet.cp = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (grdSet.cp != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        grdSet.cp = model.mesh.findOrReserveCoordinateSystem(Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, grdSet.cp));
        }
    tok.skip(3);
    grdSet.cd = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (grdSet.cd != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        grdSet.cd = model.mesh.findOrReserveCoordinateSystem(Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, grdSet.cd));
        }
    grdSet.ps = tok.nextInt(true, 0);
    grdSet.seid = tok.nextInt(true, 0);

}

void NastranParser::parseGRID(NastranTokenizer& tok, Model& model) {
    int id = tok.nextInt();
    int cp = tok.nextInt(true, grdSet.cp);
    int cpos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
    string scp;
    if (cp != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        cpos = model.mesh.findOrReserveCoordinateSystem(Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, cp));
        scp=" in CS"+to_string(cp)+"_"+to_string(cpos);
    }

    double x1 = tok.nextDouble(true, 0.0);
    double x2 = tok.nextDouble(true, 0.0);
    double x3 = tok.nextDouble(true, 0.0);

    /* Coordinate System for Displacement */
    int cd = tok.nextInt(true, grdSet.cd);
    int cdos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
    string scd="";
    if (cd != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        cdos = model.mesh.findOrReserveCoordinateSystem(Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, cd));
        scd=", DISP in CS"+to_string(cd)+"_"+to_string(cdos);
    }
    model.mesh.addNode(id, x1, x2, x3, cpos, cdos);

    int ps = tok.nextInt(true, grdSet.ps);
    if (ps) {
        string spcName = "SPC" + to_string(id);
        const auto& spc = make_shared<SinglePointConstraint>(model, DOFS::nastranCodeToDOFS(ps));
        spc->addNodeId(id);
        model.add(spc);
        model.addConstraintIntoConstraintSet(*spc, *model.commonConstraintSet);
    }

    if (this->logLevel >= LogLevel::TRACE) {
        cout << fixed << "GRID " << id << ": (" << x1 << ";" << x2 << ";" << x3 <<")"<< scp<<scd<< endl;
    }
}

void NastranParser::addProperty(NastranTokenizer& tok, int property_id, int cell_id, Model& model) {
    auto cellGroup = dynamic_pointer_cast<CellGroup>(model.mesh.findGroup(property_id));
    if (cellGroup == nullptr) {
        string cellGroupName = "PROP_" + to_string(property_id);
        string comment = cellGroupName;
        auto commentEntry = tok.labelByCommentTypeAndId.find({NastranTokenizer::CommentType::COMP, property_id});
        if (commentEntry != tok.labelByCommentTypeAndId.end())
            comment = commentEntry->second;
        cellGroup = model.mesh.createCellGroup(cellGroupName, property_id, comment);
    }
    cellGroup->addCellId(cell_id);
}

shared_ptr<CellGroup> NastranParser::getOrCreateCellGroup(int group_id, Model& model, string comment) {
    auto cellGroup = dynamic_pointer_cast<CellGroup>(model.mesh.findGroup(group_id));

    if (cellGroup == nullptr) {
        cellGroup = model.mesh.createCellGroup("CGVEGA_"+to_string(group_id), group_id, comment);
    }
    return cellGroup;
}

int NastranParser::parseOrientation(int point1, int point2, NastranTokenizer& tok,
        Model& model) {

    const auto& line = tok.currentDataLine();
    bool alternateFormat = line.size() < 8 || line[6].empty() || line[7].empty();
    shared_ptr<OrientationCoordinateSystem> ocs;
    if (alternateFormat) {
        int g0 = tok.nextInt();
        tok.nextDouble(true);
        tok.nextDouble(true);
        ocs = make_shared<OrientationCoordinateSystem>(model.mesh, point1, point2, g0);
    } else {
        double x1, x2, x3;
        x1 = tok.nextDouble();
        x2 = tok.nextDouble();
        x3 = tok.nextDouble();
        if (is_zero(x1) and is_zero(x2) and is_zero(x3)) {
            x2 = 1; // LD Hack, should find a better solution for a ill-defined segment orientation
        }
        ocs = make_shared<OrientationCoordinateSystem>(model.mesh, point1, point2, VectorialValue(x1,x2,x3));
    }
    const int idOCS = model.mesh.addOrFindOrientation(*ocs);
    return idOCS;
}

void NastranParser::parseCBAR(NastranTokenizer& tok, Model& model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt();
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    int cpos = parseOrientation(point1, point2, tok, model);
    string offt = tok.nextString(true);
    if (!offt.empty() && offt != "GGG") {
        string message = "OFFT " + offt + " not supported and taken as GGG.";
        handleParsingWarning(message, tok, model);
    }

    // Pin flags : not supported.
    int pa = tok.nextInt(true);
    int pb = tok.nextInt(true);
    if ((pa != Globals::UNAVAILABLE_INT) || (pb != Globals::UNAVAILABLE_INT)) {
        handleParsingWarning("Pin flags (PA, PB) not supported and dismissed.", tok, model);
    }
    // Offset vectors : not supported
    double w1a = tok.nextDouble(true, 0.0);
    double w2a = tok.nextDouble(true, 0.0);
    double w3a = tok.nextDouble(true, 0.0);
    double w1b = tok.nextDouble(true, 0.0);
    double w2b = tok.nextDouble(true, 0.0);
    double w3b = tok.nextDouble(true, 0.0);
    if (! ( is_equal(w1a,0.0)&&is_equal(w2a,0.0)&&is_equal(w3a,0.0)
            &&is_equal(w1b,0.0)&&is_equal(w2b,0.0)&&is_equal(w3b,0.0))){
        handleParsingWarning("Offset vectors (WA, WB) not supported and taken as null.", tok, model);
    }
    model.mesh.addCell(cell_id, CellType::SEG2, {point1, point2}, false, cpos, property_id);
    addProperty(tok, property_id, cell_id, model);
}

void NastranParser::parseCBEAM(NastranTokenizer& tok, Model& model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt();
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    int cpos = parseOrientation(point1, point2, tok, model);
    string offt = tok.nextString(true);
    if (!offt.empty() && offt != "GGG") {
        handleParsingWarning("OFFT " + offt + " not supported and taken as GGG.", tok, model);
    }

    // Pin flags : not supported.
    int pa = tok.nextInt(true);
    int pb = tok.nextInt(true);
    if ((pa != Globals::UNAVAILABLE_INT) || (pb != Globals::UNAVAILABLE_INT)) {
        handleParsingWarning("Pin flags (PA, PB) not supported and dismissed.", tok, model);
    }

    // Offset vectors : not supported
    double w1A = tok.nextDouble(true, 0.0);
    double w2A = tok.nextDouble(true, 0.0);
    double w3A = tok.nextDouble(true, 0.0);
    double w1B = tok.nextDouble(true, 0.0);
    double w2B = tok.nextDouble(true, 0.0);
    double w3B = tok.nextDouble(true, 0.0);
    if (!is_zero(w1A) || !is_zero(w2A) || !is_zero(w3A) || !is_zero(w1B) || !is_zero(w2B)
            || !is_zero(w3B)) {
        handleParsingWarning("Offset vectors (WA, WB) not supported and taken as null.", tok, model);
    }

    // Scalar or grid point identification number: not supported.
    int sa = tok.nextInt(true);
    int sb = tok.nextInt(true);
    if ((sa != Globals::UNAVAILABLE_INT) || (sb != Globals::UNAVAILABLE_INT)) {
        handleParsingWarning("Grid point identification numbers (SA, SB) not supported and dismissed.", tok, model);
    }

    model.mesh.addCell(cell_id, CellType::SEG2, {point1, point2}, false, cpos, property_id);
    addProperty(tok, property_id, cell_id, model);
}


void NastranParser::parseCBUSH(NastranTokenizer& tok, Model& model) {
    int eid = tok.nextInt(); // Cell Id
    int pid = tok.nextInt(); // Property Id
    int ga = tok.nextInt();  // Node A
    int gb = tok.nextInt(true);  // Node B
    bool forbidOrientation = (gb == Globals::UNAVAILABLE_INT);

    // Local element coordinate system
    int cpos = 0;
    if (!tok.isEmptyUntilNextKeyword()) {
        vector<string> line = tok.currentDataLine();
        if ( (line.size()>8) && !(line[8].empty())){
            // A CID is provided by the user
            tok.skip(3);
            int cid = tok.nextInt();
            cpos = model.mesh.findOrReserveCoordinateSystem(Reference<CoordinateSystem>(CoordinateSystem::Type::ABSOLUTE, cid));
        }else{
            // Local definition of the element coordinate system
            if (forbidOrientation){
                handleParsingWarning("Single node CBUSH can't support local orientation. Orientation dismissed.", tok, model);
                tok.skip(4);
                cpos = 0;
            }else{
                cpos = parseOrientation(ga, gb, tok, model);
                tok.nextInt(true);
            }
        }

        // Spring damper location (S): not supported.
        double s=tok.nextDouble(true, 0.5);
        if (!is_equal(s,0.5)){
            handleParsingWarning("S keyword not supported. Default (0.5) used.", tok, model);
            s=0.5;
        }

        // Coordinate system identification of spring-damper offset (OCID): not supported.
        int ocid =tok.nextInt(true, -1);
        double s1=tok.nextDouble(true);
        double s2=tok.nextDouble(true);
        double s3=tok.nextDouble(true);
        if (ocid!=-1){
            handleParsingWarning("OCID keyword not supported. Default (-1) used.", tok, model);
            handleParsingWarning("S1 S2 S3 dismissed :"+to_string(s1)+ " "+to_string(s2)+ " "+to_string(s3), tok, model);
            ocid=-1;
        }
    }

    // Add cell
    if (gb == Globals::UNAVAILABLE_INT){
        model.mesh.addCell(eid, CellType::POINT1, {ga}, false, cpos, pid);
    }else{
        model.mesh.addCell(eid, CellType::SEG2, {ga, gb}, false, cpos, pid);
    }
    addProperty(tok, pid, eid, model);
}


void NastranParser::parseCDAMP1(NastranTokenizer& tok, Model& model) {
    // Defines a scalar mass element without reference to a property entry.
    int eid = tok.nextInt();
    int pid = tok.nextInt();
    int g1 = tok.nextInt();
    dof_int c1 = parseDOF(tok,model);
    int g2 = tok.nextInt();
    dof_int c2 = parseDOF(tok,model,true);

    // Creates cell
    // If G2 is undefined, it is considered a fictitious grounded point, and c2=c1
    if (g2 == Globals::UNAVAILABLE_INT or g2 == 0){
        c2= c1;
        const auto& spc = make_shared<SinglePointConstraint>(model, DOFS::ALL_DOFS, 0.0);
        //shared_ptr<NodeGroup> spcNodeGroup = model.mesh.findOrCreateNodeGroup("CELAS1_" + to_string(eid) + "_GND",NodeGroup::NO_ORIGINAL_ID,"CELAS1 GROUND");
        int nodePosition = model.mesh.addNode(Node::AUTO_ID, 0.0, 0.0, 0.0);
        g2 = model.mesh.findNodeId(nodePosition);
        spc->addNodeId(g2);
        model.add(spc);
    }

    int cellPosition= model.mesh.addCell(eid, CellType::SEG2, {g1, g2}, false, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, pid);
    const auto& cellGroup = getOrCreateCellGroup(pid, model, "CDAMP1");
    cellGroup->addCellPosition(cellPosition);

    // Creates or update the ElementSet defined by the PELAS key.
    const auto& elementSet = model.elementSets.find(pid);
    if (elementSet == nullptr){
        const auto& scalarSpring = make_shared<ScalarSpring>(model, pid);
        scalarSpring->add(*cellGroup);
        scalarSpring->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        model.add(scalarSpring);
    } else {
        if (elementSet->type == ElementSet::Type::SCALAR_SPRING){
            const auto& springElementSet = static_pointer_cast<ScalarSpring>(elementSet);
            springElementSet->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        } else {
            handleParsingError("The part of PID "+to_string(pid)+" already exists with the wrong NATURE.", tok, model);
        }
    }
}

void NastranParser::parseCELAS1(NastranTokenizer& tok, Model& model) {
    // Defines a scalar spring element.
    int eid = tok.nextInt();
    int pid = tok.nextInt();
    int g1 = tok.nextInt();
    dof_int c1 = parseDOF(tok,model);
    int g2 = tok.nextInt(true);
    dof_int c2 = parseDOF(tok,model,true);

    // Creates cell
    // If G2 is undefined, it is considered a fictitious grounded point, and c2=c1
    CellType cellType =  CellType::SEG2;
    if (g2 == Globals::UNAVAILABLE_INT or g2 == 0){
        c2= c1;
        const auto& spc = make_shared<SinglePointConstraint>(model, DOFS::ALL_DOFS, 0.0);
        //shared_ptr<NodeGroup> spcNodeGroup = model.mesh.findOrCreateNodeGroup("CELAS1_" + to_string(eid) + "_GND",NodeGroup::NO_ORIGINAL_ID,"CELAS1 GROUND");
        int nodePosition = model.mesh.addNode(Node::AUTO_ID, 0.0, 0.0, 0.0);
        g2 = model.mesh.findNodeId(nodePosition);
        spc->addNodeId(g2);
        model.add(spc);
    }
    int cellPosition= model.mesh.addCell(eid, cellType, {g1, g2}, false, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, pid);
    const auto& cellGroup = getOrCreateCellGroup(pid, model, "CELAS1");
    cellGroup->addCellPosition(cellPosition);

    // Creates or update the ElementSet defined by the PELAS key.
    const auto& elementSet = model.elementSets.find(pid);
    if (elementSet == nullptr){
        const auto& scalarSpring = make_shared<ScalarSpring>(model, pid);
        scalarSpring->add(*cellGroup);
        scalarSpring->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        model.add(scalarSpring);
    }else{
        if (elementSet->type == ElementSet::Type::SCALAR_SPRING){
            const auto& springElementSet = dynamic_pointer_cast<ScalarSpring>(elementSet);
            springElementSet->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        }else{
            handleParsingError("The part of PID "+to_string(pid)+" already exists with the wrong NATURE.", tok, model);
        }
    }
}

void NastranParser::parseCELAS2(NastranTokenizer& tok, Model& model) {
    // Defines a scalar spring element without reference to a property entry.
    int eid = tok.nextInt();
    double k = tok.nextDouble();
    int g1 = tok.nextInt();
    dof_int c1 = parseDOF(tok,model);
    int g2 = tok.nextInt();
    dof_int c2 = parseDOF(tok,model);
    double ge = tok.nextDouble(true, 0.0);

    // S is only used for post-treatment, and so discarded.
    double s = tok.nextDouble(true);
    if (!is_equal(s, Globals::UNAVAILABLE_DOUBLE)){
        if (this->logLevel >= LogLevel::DEBUG) {
            handleParsingWarning("Stress coefficient (S) is only used for post-treatment and dismissed.", tok, model);
        }
    }

    // Create a Cell and a cellgroup
    const auto& springGroup = model.mesh.createCellGroup("CELAS2_" + to_string(eid), Group::NO_ORIGINAL_ID, "CELAS2");
    int cellPosition= model.mesh.addCell(eid, CellType::SEG2, {g1, g2});
    springGroup->addCellPosition(cellPosition);

    // Create ElementSet
    const auto& scalarSpring = make_shared<ScalarSpring>(model, eid, k ,ge);
    scalarSpring->add(*springGroup);
    scalarSpring->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
    model.add(scalarSpring);
}

void NastranParser::parseCELAS4(NastranTokenizer& tok, Model& model) {
    // Defines a scalar spring element that is connected only to scalar points, without
    // reference to a property entry.
    int eid = tok.nextInt();
    double k = tok.nextDouble();
    int s1 = tok.nextInt();
    int s2 = tok.nextInt();

    // Create a Cell and a cellgroup
    const auto& springGroup = model.mesh.createCellGroup("CELAS4_" + to_string(eid), Group::NO_ORIGINAL_ID, "CELAS4");
    int cellPosition= model.mesh.addCell(eid, CellType::SEG2, {s1, s2});
    springGroup->addCellPosition(cellPosition);

    // Create ElementSet
    const auto& scalarSpring = make_shared<ScalarSpring>(model, eid, k);
    scalarSpring->add(*springGroup);
    scalarSpring->addSpring(cellPosition, DOF::DX, DOF::DX);
    model.add(scalarSpring);
}

void NastranParser::parseElem(NastranTokenizer& tok, Model& model,
                                  const vector<CellType>& cellTypes) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);
    auto it = cellTypes.begin();
    CellType cellType = *it;
    unsigned int i = 0;
    const auto& nodeIds = tok.nextInts();
    while (cellType.numNodes != nodeIds.size()) {
        if (it == cellTypes.end()) {
            cerr << "cellType.numNodes:" << cellType.numNodes << ", nodeIds.size():" << nodeIds.size() << endl;
            cerr << "cell_id:" << cell_id << ", property_id:" << property_id << ", i:" << i << endl;
            handleParsingError("Format element not supported:"+cellType.to_str() + " for line: " + tok.currentRawDataLine(), tok, model);
        }
        cellType = *it;
        it++;
    }
    vector<int> nastranConnect;
    nastranConnect.reserve(nodeIds.size());
    nastranConnect.insert( nastranConnect.end(), nodeIds.begin(), nodeIds.end() );
    vector<int> medConnect;
    auto nastran2med_it = nastran2medNodeConnectByCellType.find(cellType.code);
    if (nastran2med_it == nastran2medNodeConnectByCellType.end()) {
        medConnect = nastranConnect;
    } else {
        const auto& nastran2medNodeConnect = nastran2med_it->second;
        medConnect.resize(cellType.numNodes);
        for (unsigned int i2 = 0; i2 < cellType.numNodes; i2++) {
            //model.mesh.findOrReserveNode(nastranConnect[i2], property_id); // LD this is optional : preserving cell nodeid order for better output readeability
            medConnect[nastran2medNodeConnect[i2]] = nastranConnect[i2];
        }
    }
    model.mesh.addCell(cell_id, cellType, medConnect, false, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, property_id);
    addProperty(tok, property_id, cell_id, model);
}

void NastranParser::parseCGAP(NastranTokenizer& tok, Model& model) {
    int eid = tok.nextInt();
    int pid = tok.nextInt(true, eid);
    int ga = tok.nextInt();
    int gb = tok.nextInt();
    parseOrientation(ga, gb, tok, model);
    int cid = tok.nextInt(true,CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);

    if (cid != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        handleParsingWarning("CID not supported and dismissed.", tok, model);
    }

    const auto& gapPtr = model.find(Reference<Constraint>(Constraint::Type::GAP, pid));
    if (gapPtr == nullptr) {
        const auto& gapConstraint = make_shared<GapTwoNodes>(model, pid);
        gapConstraint->addGapNodes(ga, gb);
        model.add(gapConstraint);
        model.addConstraintIntoConstraintSet(*gapConstraint, *model.commonConstraintSet);
    } else {
        const auto& gapConstraint = static_pointer_cast<GapTwoNodes>(gapPtr);
        gapConstraint->addGapNodes(ga, gb);
    }
}

void NastranParser::parseCHEXA(NastranTokenizer& tok, Model& model) {
    parseElem(tok, model, { CellType::HEXA8, CellType::HEXA20 });
}

void NastranParser::parseCMASS2(NastranTokenizer& tok, Model& model) {
    // Defines a scalar mass element without reference to a property entry.
    int eid = tok.nextInt();
    double m = tok.nextDouble();
    int g1 = tok.nextInt();
    dof_int c1 = parseDOF(tok, model); // Nastran coordinate goes from 1 to 6, VEGA from 0 to 5.
    int g2 = tok.nextInt();
    dof_int c2 = parseDOF(tok, model); // Nastran coordinate goes from 1 to 6, VEGA from 0 to 5.
    const auto& matrix = make_shared<MassMatrix>(model, MatrixType::SYMMETRIC, eid);
    matrix->addComponent(g1, DOF::findByPosition(c1), g2, DOF::findByPosition(c2), m);
    model.add(matrix);
}

void NastranParser::parseCPENTA(NastranTokenizer& tok, Model& model) {
    parseElem(tok, model, { CellType::PENTA6, CellType::PENTA15 });
}

void NastranParser::parseCPYRAM(NastranTokenizer& tok, Model& model) {
    parseElem(tok, model, { CellType::PYRA5, CellType::PYRA13 });
}

void NastranParser::parseCQUAD(NastranTokenizer& tok, Model& model) {
    parseElem(tok, model, { CellType::QUAD4, CellType::QUAD8, CellType::QUAD9 });
}

void NastranParser::parseCQUAD4(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::QUAD4);
}

void NastranParser::parseCQUAD8(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::QUAD8);
}

void NastranParser::parseCQUADR(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::QUAD4);
}

void NastranParser::parseCROD(NastranTokenizer& tok, Model& model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    model.mesh.addCell(cell_id, CellType::SEG2, {point1, point2}, false, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, property_id);
    addProperty(tok, property_id, cell_id, model);
}

void NastranParser::parseCTETRA(NastranTokenizer& tok, Model& model) {
    parseElem(tok, model, { CellType::TETRA4, CellType::TETRA10 });
}

void NastranParser::parseCTRIA3(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::TRI3);
}

void NastranParser::parseCTRIA6(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::TRI6);
}

void NastranParser::parseCTRIAR(NastranTokenizer& tok, Model& model) {
    parseShellElem(tok, model, CellType::TRI3);
}

void NastranParser::parseShellElem(NastranTokenizer& tok, Model& model,
        CellType cellType) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);

    vector<int> nodeIds;
    vector<double> ti;
    double thetaOrMCID=0.0;
    double zoffs=0.0;
    int tflag=0;
    bool isThereT=false;

    CellType::Code code = cellType.code;
    switch (code) {
    case CellType::Code::TRI3_CODE:
        for (int i=0; i < 3; i++)
            nodeIds.push_back(tok.nextInt());
        thetaOrMCID = tok.nextDouble(true,0.0);
        zoffs = tok.nextDouble(true,0.0);
        tok.skip(2);

        tflag = tok.nextInt(true, 0);
        for (int i=0; i < 3; i++){
            double t = tok.nextDouble(true);
            ti.push_back(t);
            isThereT = isThereT or (!is_equal(t, Globals::UNAVAILABLE_DOUBLE));
        }
        break;

    case CellType::Code::QUAD4_CODE:
        for (int i=0; i < 4; i++)
            nodeIds.push_back(tok.nextInt());
        thetaOrMCID = tok.nextDouble(true,0.0);
        zoffs = tok.nextDouble(true,0.0);

        tok.skip(1);
        tflag = tok.nextInt(true,0);
        for (int i=0; i < 4; i++){
            double t = tok.nextDouble(true);
            ti.push_back(t);
            isThereT = isThereT or (!is_equal(t, Globals::UNAVAILABLE_DOUBLE));
        }
        break;

    case CellType::Code::TRI6_CODE:
        for (int i=0; i < 6; i++)
            nodeIds.push_back(tok.nextInt());
        thetaOrMCID = tok.nextDouble(true,0.0);
        zoffs = tok.nextDouble(true,0.0);
        for (int i=0; i < 3; i++){
            double t = tok.nextDouble(true);
            ti.push_back(t);
            isThereT = isThereT or (!is_equal(t, Globals::UNAVAILABLE_DOUBLE));
        }
        tflag = tok.nextInt(true,0);
        break;

    case CellType::Code::QUAD8_CODE:
        for (int i=0; i < 8; i++)
            nodeIds.push_back(tok.nextInt());
        for (int i=0; i < 4; i++){
            double t = tok.nextDouble(true);
            ti.push_back(t);
            isThereT = isThereT or (!is_equal(t, Globals::UNAVAILABLE_DOUBLE));
        }
        thetaOrMCID = tok.nextDouble(true,0.0);
        zoffs = tok.nextDouble(true,0.0);
        tflag = tok.nextInt(true,0);
        break;

    default:
        handleParsingError("Unrecognized shell element: "+to_string(static_cast<int>(code)), tok, model);
    }

    //double theta = 0.0;
    if (!is_zero(thetaOrMCID)){
        //if (tok.isNextDouble()) {
        //    theta = tok.nextDouble(true, 0.0);
        //} else {
            handleParsingWarning("THETA/MCID parameter ignored.", tok, model);
        //}
    }
    if (!is_zero(zoffs)){
        handleParsingWarning("non-null ZOFFS parameter ignored.", tok, model);
    }
    if (tflag!=0){
        handleParsingWarning("non-null TFLAG ("+ to_string(tflag)+") parameter ignored.", tok, model);
    }
    if (isThereT){
        handleParsingWarning("membrane thickness is ignored.", tok, model);
    }

    model.mesh.addCell(cell_id, cellType, nodeIds, false, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, property_id);
    addProperty(tok, property_id, cell_id, model);

}

/*
 * Defining DOFS::DX should suffice for VEGA to know, during the translation
 * that these GRID points have only one DOF. It's not the case, so we create SPC to
 * compensate.
 */
void NastranParser::parseSPOINT(NastranTokenizer& tok, Model& model) {

    int id1 = tok.nextInt();
    double x1 = 0.0;
    double x2 = 0.0;
    double x3 = 0.0;
    int cpos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;

    // Creating or finding the Node Group and Constraint
    const auto& spc = make_shared<SinglePointConstraint>(model, DOFS::ALL_DOFS-DOF::DX, 0.0);
    const auto& spcNodeGroup = model.mesh.findOrCreateNodeGroup("SPC_SPOINT",NodeGroup::NO_ORIGINAL_ID,"SPOINT");

    int nodePosition = model.mesh.addNode(id1, x1, x2, x3, cpos, cpos);
    model.mesh.allowDOFS(nodePosition, DOF::DX);
    spcNodeGroup->addNodeId(id1);
    spc->addNodeId(id1);
    if (this->logLevel >= LogLevel::TRACE) {
        cout << fixed << "SPOINT " << id1 << endl;
    }

    bool format1 = tok.isNextInt() || tok.isEmptyUntilNextKeyword();
    if (format1) {
        while (tok.isNextInt()){
            const int id2 = tok.nextInt();
            nodePosition = model.mesh.addNode(id2, x1, x2, x3, cpos, cpos);
            model.mesh.allowDOFS(nodePosition, DOF::DX);
            spcNodeGroup->addNodeId(id2);
            spc->addNodeId(id2);
            if (this->logLevel >= LogLevel::TRACE) {
                cout << fixed << "SPOINT " << id2 << endl;
            }
        }
    } else {
        string str2 = tok.nextString();
        to_upper(str2);
        if (str2 == "THRU") {
            //format2
            const int id2 = tok.nextInt();
            for (int id=id1+1; id<=id2; id++){
                nodePosition = model.mesh.addNode(id, x1, x2, x3, cpos, cpos);
                model.mesh.allowDOFS(nodePosition, DOF::DX);
                spcNodeGroup->addNodeId(id);
                spc->addNodeId(id);
                if (this->logLevel >= LogLevel::TRACE) {
                    cout << fixed << "SPOINT " << id << endl;
                }
            }
        } else {
            handleParsingError("Invalid format for SPOINT.", tok, model);
        }
    }

    // Adding the constraint to the model
    model.add(spc);
    model.addConstraintIntoConstraintSet(*spc, *model.commonConstraintSet);
}




} /* namespace nastran */

} /* namespace vega */
