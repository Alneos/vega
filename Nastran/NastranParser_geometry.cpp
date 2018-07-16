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
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "NastranParser.h"

using namespace std;
namespace alg = boost::algorithm;
using boost::assign::list_of;
using boost::lexical_cast;
using boost::to_upper;
using namespace boost::assign;

namespace vega {

namespace nastran {

const unordered_map<CellType::Code, vector<int>, hash<int>> NastranParser::nastran2medNodeConnectByCellType =
        {
                { CellType::TRI3_CODE, { 0, 2, 1 } },
                { CellType::TRI6_CODE, { 0, 2, 1, 5, 4, 3 } },
                { CellType::QUAD4_CODE, { 0, 3, 2, 1 } },
                { CellType::QUAD8_CODE, { 0, 3, 2, 1, 7, 6, 5, 4 } },
                { CellType::QUAD9_CODE, { 0, 3, 2, 1, 7, 6, 5, 4, 8 } },
                { CellType::TETRA4_CODE, { 0, 2, 1, 3 } },
                { CellType::TETRA10_CODE, { 0, 2, 1, 3, 6, 5, 4, 7, 9, 8 } },
                { CellType::PYRA5_CODE, { 0, 3, 2, 1, 4 } },
                { CellType::PYRA13_CODE, { 0, 3, 2, 1, 4, 8, 7, 6, 5, 9, 12, 11, 10 } },
                { CellType::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
                { CellType::PENTA15_CODE, { 0, 2, 1, 3, 5, 4, 8, 7, 6, 12, 14, 13, 11, 10, 9 } },
                { CellType::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
                { CellType::HEXA20_CODE, { 0, 3, 2, 1, 4, 7, 6, 5, 11, 10, 9, 8, 16, 19, 18, 17, 15,
                        14, 13, 12 } }
        };

void NastranParser::parseGRDSET(NastranTokenizer& tok, shared_ptr<Model> model) {
    tok.skip(1);
    grdSet.cp = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (grdSet.cp != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        grdSet.cp = model->findOrReserveCoordinateSystem(grdSet.cp);
        }
    tok.skip(3);
    grdSet.cd = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (grdSet.cd != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        grdSet.cd = model->findOrReserveCoordinateSystem(grdSet.cd);
        }
    grdSet.ps = tok.nextInt(true, 0);
    grdSet.seid = tok.nextInt(true, 0);

}

void NastranParser::parseGRID(NastranTokenizer& tok, shared_ptr<Model> model) {
    int id = tok.nextInt();
    int cp = tok.nextInt(true, grdSet.cp);
    int cpos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
    string scp;
    if (cp != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        cpos = model->findOrReserveCoordinateSystem(cp);
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
        cdos = model->findOrReserveCoordinateSystem(cd);
        scd=", DISP in CS"+to_string(cd)+"_"+to_string(cdos);
    }
    model->mesh->addNode(id, x1, x2, x3, cpos, cdos);

    int ps = tok.nextInt(true, grdSet.ps);
    if (ps) {
        string spcName = string("SPC") + lexical_cast<string>(id);
        SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(ps));
        spc.addNodeId(id);
        model->add(spc);
        model->addConstraintIntoConstraintSet(spc, model->commonConstraintSet);
    }

    if (this->logLevel >= LogLevel::TRACE) {
        cout << fixed << "GRID " << id << ": (" << x1 << ";" << x2 << ";" << x3 <<")"<< scp<<scd<< endl;
    }
}

void NastranParser::addProperty(int property_id, int cell_id, shared_ptr<Model> model) {
    shared_ptr<CellGroup> cellGroup = getOrCreateCellGroup(property_id, model);
    cellGroup->addCellId(cell_id);
}

shared_ptr<CellGroup> NastranParser::getOrCreateCellGroup(int property_id, shared_ptr<Model> model, const string & command) {
    shared_ptr<CellGroup> cellGroup = dynamic_pointer_cast<CellGroup>(model->mesh->findGroup(property_id));

    string cellGroupName= command + string("_") + lexical_cast<string>(property_id);
    if (cellGroup == nullptr){
        cellGroup = model->mesh->createCellGroup(cellGroupName, property_id, command);
    }
    else{
        // If the Group already exists, and if it was not already done, we enforce the name and comment of the Group
        if (cellGroup->getName().substr(0,6)=="CGVEGA"){
        	model->mesh->renameGroup(cellGroup->getName(), cellGroupName, command);
        }
    }
    return cellGroup;
}

int NastranParser::parseOrientation(int point1, int point2, NastranTokenizer& tok,
        shared_ptr<Model> model) {

    vector<string> line = tok.currentDataLine();
    bool alternateFormat = line.size() < 8 || line[6].empty() || line[7].empty();
    OrientationCoordinateSystem* ocs;
    if (alternateFormat) {
        int g0 = tok.nextInt();
        tok.nextDouble(true);
        tok.nextDouble(true);
        ocs = new OrientationCoordinateSystem(*model, point1, point2, g0);
    } else {
        double x1, x2, x3;
        x1 = tok.nextDouble();
        x2 = tok.nextDouble();
        x3 = tok.nextDouble();
        ocs = new OrientationCoordinateSystem(*model, point1, point2, VectorialValue(x1,x2,x3));
    }
    const int idOCS = model->addOrFindOrientation(*ocs);
    delete(ocs);
    return idOCS;
}

void NastranParser::parseCBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt();
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    int cpos = parseOrientation(point1, point2, tok, model);
    string offt = tok.nextString(true);
    if (!offt.empty() && offt != "GGG") {
        string message = string("OFFT ") + offt + string(" not supported and taken as GGG.");
        handleParsingWarning(message, tok, model);
    }

    // Pin flags : not supported.
    int pa = tok.nextInt(true);
    int pb = tok.nextInt(true);
    if ((pa != Globals::UNAVAILABLE_INT) || (pb != Globals::UNAVAILABLE_INT)) {
        string message = string("Pin flags (PA, PB) not supported and dismissed.");
        handleParsingWarning(message, tok, model);
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
        string message = string("Offset vectors (WA, WB) not supported and taken as null.");
        handleParsingWarning(message, tok, model);
    }
    vector<int> connectivity;
    connectivity += point1, point2;
    model->mesh->addCell(cell_id, CellType::SEG2, connectivity, false, cpos);
    addProperty(property_id, cell_id, model);
}

void NastranParser::parseCBEAM(NastranTokenizer& tok, shared_ptr<Model> model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt();
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    int cpos = parseOrientation(point1, point2, tok, model);
    string offt = tok.nextString(true);
    if (!offt.empty() && offt != "GGG") {
        string message = string("OFFT ") + offt + string(" not supported and taken as GGG.");
        handleParsingWarning(message, tok, model);
    }

    // Pin flags : not supported.
    int pa = tok.nextInt(true);
    int pb = tok.nextInt(true);
    if ((pa != Globals::UNAVAILABLE_INT) || (pb != Globals::UNAVAILABLE_INT)) {
        string message = string("Pin flags (PA, PB) not supported and dismissed.");
        handleParsingWarning(message, tok, model);
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
        string message = string("Grid point identification numbers (SA, SB) not supported and dismissed.");
        handleParsingWarning(message, tok, model);
    }

    vector<int> connectivity;
    connectivity += point1, point2;

    model->mesh->addCell(cell_id, CellType::SEG2, connectivity, false, cpos);
    addProperty(property_id, cell_id, model);
}


void NastranParser::parseCBUSH(NastranTokenizer& tok, shared_ptr<Model> model) {
    int eid = tok.nextInt(); // Cell Id
    int pid = tok.nextInt(); // Property Id
    int ga = tok.nextInt();  // Node A
    int gb = tok.nextInt(true);  // Node B
    bool forbidOrientation = (gb == Globals::UNAVAILABLE_INT);

    // Local element coordinate system
    int cpos = 0;
    vector<string> line = tok.currentDataLine();
    if ( (line.size()>8) && !(line[8].empty())){
        // A CID is provided by the user
        tok.skip(3);
        int cid = tok.nextInt();
        cpos = model->findOrReserveCoordinateSystem(cid);
    }else{
        // Local definition of the element coordinate system
        if (forbidOrientation){
            handleParsingWarning(string("Single node CBUSH can't support local orientation. Orientation dismissed."), tok, model);
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
        handleParsingWarning(string("S keyword not supported. Default (0.5) used."), tok, model);
        s=0.5;
    }

    // Coordinate system identification of spring-damper offset (OCID): not supported.
    int ocid =tok.nextInt(true, -1);
    double s1=tok.nextDouble(true);
    double s2=tok.nextDouble(true);
    double s3=tok.nextDouble(true);
    if (ocid!=-1){
        handleParsingWarning(string("OCID keyword not supported. Default (-1) used."), tok, model);
        handleParsingWarning(string("S1 S2 S3 dismissed :")+std::to_string(s1)+ string(" ")+std::to_string(s2)+ string(" ")+std::to_string(s3), tok, model);
        ocid=-1;
    }

    // Add cell
    vector<int> connectivity;
    if (gb == Globals::UNAVAILABLE_INT){
        connectivity += ga;
        model->mesh->addCell(eid, CellType::POINT1, connectivity, false, cpos);
    }else{
        connectivity += ga, gb;
        model->mesh->addCell(eid, CellType::SEG2, connectivity, false, cpos);
    }
    addProperty(pid, eid, model);
}

void NastranParser::parseCELAS1(NastranTokenizer& tok, shared_ptr<Model> model) {
    // Defines a scalar spring element .
    int eid = tok.nextInt();
    int pid = tok.nextInt();
    int g1 = tok.nextInt();
    int c1 = parseDOF(tok,model);
    int g2 = tok.nextInt();
    int c2 = parseDOF(tok,model);

    // Creates cell
    vector<int> connectivity;
    connectivity += g1, g2;
    int cellPosition= model->mesh->addCell(eid, CellType::SEG2, connectivity);
    shared_ptr<CellGroup> cellGroup = getOrCreateCellGroup(pid, model);
    cellGroup->addCellId(model->mesh->findCell(cellPosition).id);

    // Creates or update the ElementSet defined by the PELAS key.
    std::shared_ptr<ElementSet> elementSet = model->elementSets.find(pid);
    if (elementSet == nullptr){
        ScalarSpring scalarSpring(*model, pid);
        scalarSpring.assignCellGroup(cellGroup);
        scalarSpring.addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        model->add(scalarSpring);
    }else{
        if (elementSet->type == ElementSet::SCALAR_SPRING){
            std::shared_ptr<ScalarSpring> springElementSet = static_pointer_cast<ScalarSpring>(elementSet);
            springElementSet->addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
        }else{
            string message = "The part of PID "+std::to_string(pid)+" already exists with the wrong NATURE.";
            handleParsingError(message, tok, model);
        }
    }
}

void NastranParser::parseCELAS2(NastranTokenizer& tok, shared_ptr<Model> model) {
    // Defines a scalar spring element without reference to a property entry.
    int eid = tok.nextInt();
    double k = tok.nextDouble();
    int g1 = tok.nextInt();
    int c1 = parseDOF(tok,model);
    int g2 = tok.nextInt();
    int c2 = parseDOF(tok,model);
    double ge = tok.nextDouble(true, 0.0);

    // S is only used for post-treatment, and so discarded.
    double s = tok.nextDouble(true);
    if (!is_equal(s, Globals::UNAVAILABLE_DOUBLE)){
        if (this->logLevel >= LogLevel::DEBUG) {
            handleParsingWarning("Stress coefficient (S) is only used for post-treatment and dismissed.", tok, model);
        }
    }

    // Create a Cell and a cellgroup
    shared_ptr<CellGroup> springGroup = model->mesh->createCellGroup("CELAS2_" + to_string(eid), Group::NO_ORIGINAL_ID, "CELAS2");
    vector<int> connectivity;
    connectivity += g1, g2;
    int cellPosition= model->mesh->addCell(eid, CellType::SEG2, connectivity);
    springGroup->addCellId(model->mesh->findCell(cellPosition).id);

    // Create ElementSet
    ScalarSpring scalarSpring(*model, eid, k ,ge);
    scalarSpring.assignCellGroup(springGroup);
    scalarSpring.addSpring(cellPosition, DOF::findByPosition(c1), DOF::findByPosition(c2));
    model->add(scalarSpring);
}

void NastranParser::parseCELAS4(NastranTokenizer& tok, shared_ptr<Model> model) {
    // Defines a scalar spring element that is connected only to scalar points, without
    // reference to a property entry.
    int eid = tok.nextInt();
    double k = tok.nextDouble();
    int s1 = tok.nextInt();
    int s2 = tok.nextInt();

    // Create a Cell and a cellgroup
    shared_ptr<CellGroup> springGroup = model->mesh->createCellGroup("CELAS4_" + to_string(eid), Group::NO_ORIGINAL_ID, "CELAS4");
    vector<int> connectivity;
    connectivity += s1, s2;
    int cellPosition= model->mesh->addCell(eid, CellType::SEG2, connectivity);
    springGroup->addCellId(model->mesh->findCell(cellPosition).id);

    // Create ElementSet
    ScalarSpring scalarSpring(*model, eid, k);
    scalarSpring.assignCellGroup(springGroup);
    scalarSpring.addSpring(cellPosition, DOF::DX, DOF::DX);
    model->add(scalarSpring);
}

void NastranParser::parseElem(NastranTokenizer& tok, shared_ptr<Model> model,
                                  vector<CellType> cellTypes) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);
    auto it = cellTypes.begin();
    CellType& cellType = *it;
    vector<int> nastranConnect;
    unsigned int i = 0;
    while (tok.isNextInt()) {
        if (it == cellTypes.end())
            handleParsingError("Format element not supported "+cellType.to_str(), tok, model);
        cellType = *it;
        for (; i < cellType.numNodes; i++)
            nastranConnect.push_back(tok.nextInt());
        it++;
    }
    vector<int> medConnect;
    auto nastran2med_it = nastran2medNodeConnectByCellType.find(cellType.code);
    if (nastran2med_it == nastran2medNodeConnectByCellType.end()) {
        medConnect = nastranConnect;
    } else {
        vector<int> nastran2medNodeConnect = nastran2med_it->second;
        medConnect.resize(cellType.numNodes);
        for (unsigned int i = 0; i < cellType.numNodes; i++)
            medConnect[nastran2medNodeConnect[i]] = nastranConnect[i];
    }
    model->mesh->addCell(cell_id, cellType, medConnect);
    addProperty(property_id, cell_id, model);
}

void NastranParser::parseCGAP(NastranTokenizer& tok, shared_ptr<Model> model) {
    int eid = tok.nextInt();
    int pid = tok.nextInt(true, eid);
    int ga = tok.nextInt();
    int gb = tok.nextInt();
    parseOrientation(ga, gb, tok, model);
    int cid = tok.nextInt(true,CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);

    if (cid != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        handleParsingWarning(string("CID not supported and dismissed."), tok, model);
    }

    shared_ptr<Constraint> gapPtr = model->find(Reference<Constraint>(Constraint::GAP, pid));
    if (!gapPtr) {
        GapTwoNodes gapConstraint(*model, pid);
        gapConstraint.addGapNodes(ga, gb);
        model->add(gapConstraint);
        model->addConstraintIntoConstraintSet(gapConstraint, model->commonConstraintSet);
    } else {
        shared_ptr<GapTwoNodes> gapConstraint = static_pointer_cast<GapTwoNodes>(gapPtr);
        gapConstraint->addGapNodes(ga, gb);
    }
}

void NastranParser::parseCHEXA(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseElem(tok, model, { CellType::HEXA8, CellType::HEXA20 });
}

void NastranParser::parseCMASS2(NastranTokenizer& tok, shared_ptr<Model> model) {
    // Defines a scalar mass element without reference to a property entry.
    int eid = tok.nextInt();
    double m = tok.nextDouble();
    int g1 = tok.nextInt();
    int c1 = tok.nextInt(); // Nastran coordinate goes from 1 to 6, VEGA from 0 to 5.
    int g2 = tok.nextInt();
    int c2 = tok.nextInt(); // Nastran coordinate goes from 1 to 6, VEGA from 0 to 5.
    MassMatrix matrix(*model, eid);
    matrix.addComponent(g1, DOF::findByPosition(c1-1), g2, DOF::findByPosition(c2-1), m);
    model->add(matrix);
}

void NastranParser::parseCPENTA(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseElem(tok, model, { CellType::PENTA6, CellType::PENTA15 });
}

void NastranParser::parseCPYRAM(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseElem(tok, model, { CellType::PYRA5, CellType::PYRA13 });
}

void NastranParser::parseCQUAD(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseElem(tok, model, { CellType::QUAD4, CellType::QUAD8, CellType::QUAD9 });
}

void NastranParser::parseCQUAD4(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::QUAD4);
}

void NastranParser::parseCQUAD8(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::QUAD8);
}

void NastranParser::parseCQUADR(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::QUAD4);
}

void NastranParser::parseCROD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);
    int point1 = tok.nextInt();
    int point2 = tok.nextInt();
    CellType cellType = CellType::SEG2;
    vector<int> coords;
    coords += point1, point2;
    model->mesh->addCell(cell_id, cellType, coords);
    addProperty(property_id, cell_id, model);
}

void NastranParser::parseCTETRA(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseElem(tok, model, { CellType::TETRA4, CellType::TETRA10 });
}

void NastranParser::parseCTRIA3(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::TRI3);
}

void NastranParser::parseCTRIA6(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::TRI6);
}

void NastranParser::parseCTRIAR(NastranTokenizer& tok, shared_ptr<Model> model) {
    parseShellElem(tok, model, CellType::TRI3);
}

void NastranParser::parseShellElem(NastranTokenizer& tok, shared_ptr<Model> model,
        CellType cellType) {
    int cell_id = tok.nextInt();
    int property_id = tok.nextInt(true, cell_id);

    vector<int> coords;
    vector<double> ti;
    double thetaOrMCID=0.0;
    double zoffs=0.0;
    int tflag=0;
    bool isThereT=false;

    CellType::Code code = cellType.code;
    switch (code) {
    case CellType::TRI3_CODE:
        for (int i=0; i < 3; i++)
            coords.push_back(tok.nextInt());
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

    case CellType::QUAD4_CODE:
        for (int i=0; i < 4; i++)
            coords.push_back(tok.nextInt());
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

    case CellType::TRI6_CODE:
        for (int i=0; i < 6; i++)
            coords.push_back(tok.nextInt());
        thetaOrMCID = tok.nextDouble(true,0.0);
        zoffs = tok.nextDouble(true,0.0);
        for (int i=0; i < 3; i++){
            double t = tok.nextDouble(true);
            ti.push_back(t);
            isThereT = isThereT or (!is_equal(t, Globals::UNAVAILABLE_DOUBLE));
        }
        tflag = tok.nextInt(true,0);
        break;

    case CellType::QUAD8_CODE:
        for (int i=0; i < 8; i++)
            coords.push_back(tok.nextInt());
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
        const string msg = "Unrecognized shell element: "+to_string(code);
        handleParsingError(msg, tok, model);
    }

    // A lot of things are ignored in this shell
    if (!is_zero(thetaOrMCID)){
        const string msg = "THETA or MCID parameter ignored.";
        handleParsingWarning(msg, tok, model);
    }
    if (!is_zero(zoffs)){
        const string msg = "non-null ZOFFS parameter ignored.";
        handleParsingWarning(msg, tok, model);
    }
    if (tflag!=0){
        const string msg = "non-null TFLAG ("+ to_string(tflag)+") parameter ignored.";
        handleParsingWarning(msg, tok, model);
    }
    if (isThereT){
        const string msg = "membrane thickness is ignored.";
        handleParsingWarning(msg, tok, model);
    }

    model->mesh->addCell(cell_id, cellType, coords);
    addProperty(property_id, cell_id, model);

}

/*
 * Defining allowDOFS::ONE should suffice for VEGA to know, during the translation
 * that these GRID points have only one DOF. IT's not the case, so we create SPC to
 * compensate.
 */
void NastranParser::parseSPOINT(NastranTokenizer& tok, shared_ptr<Model> model) {

    int id1 = tok.nextInt();
    double x1 = 0.0;
    double x2 = 0.0;
    double x3 = 0.0;
    int cpos = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;

    // Creating or finding the Node Group and Constraint
    SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::ALL_DOFS-DOFS::ONE, 0.0);
    string name = string("SPC_SPOINT");
    shared_ptr<NodeGroup> spcNodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SPOINT");

    int nodePosition = model->mesh->addNode(id1, x1, x2, x3, cpos, cpos);
    model->mesh->allowDOFS(nodePosition, DOFS::DOFS::ONE);
    spcNodeGroup->addNodeId(id1);
    spc.addNodeId(id1);
    if (this->logLevel >= LogLevel::TRACE) {
        cout << fixed << "SPOINT " << id1 << endl;
    }

    bool format1 = tok.isNextInt() || tok.isEmptyUntilNextKeyword();
    if (format1) {
        while (tok.isNextInt()){
            const int id2 = tok.nextInt();
            nodePosition = model->mesh->addNode(id2, x1, x2, x3, cpos, cpos);
            model->mesh->allowDOFS(nodePosition, DOFS::DOFS::ONE);
            spcNodeGroup->addNodeId(id2);
            spc.addNodeId(id2);
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
                nodePosition = model->mesh->addNode(id, x1, x2, x3, cpos, cpos);
                model->mesh->allowDOFS(nodePosition, DOFS::DOFS::ONE);
                spcNodeGroup->addNodeId(id);
                spc.addNodeId(id);
                if (this->logLevel >= LogLevel::TRACE) {
                    cout << fixed << "SPOINT " << id << endl;
                }
            }
        } else {
            handleParsingError("Invalid format.", tok, model);
        }
    }

    // Adding the constraint to the model
    model->add(spc);
    model->addConstraintIntoConstraintSet(spc, model->commonConstraintSet);
}




} /* namespace nastran */

} /* namespace vega */
