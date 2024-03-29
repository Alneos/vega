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
using namespace std;

namespace vega {
namespace systus {


string SMFToString(SMF key){
    return SMFtoString.find(key)->second;
}

SMF DOFtoSMF(DOF dof){
    if (dof==DOF::RX){return SMF::IX;}
    if (dof==DOF::RY){return SMF::IY;}
    if (dof==DOF::RZ){return SMF::IZ;}
    if (dof==DOF::DX){return SMF::KX;}
    if (dof==DOF::DY){return SMF::KY;}
    return SMF::KZ;
}

const unordered_map<CellType::Code, vector<int>, EnumClassHash> SystusWriter::systus2medNodeConnectByCellType =
{
        { CellType::Code::POINT1_CODE, { 0 } },
        { CellType::Code::SEG2_CODE, { 0, 1 } },
        { CellType::Code::SEG3_CODE, { 0, 2, 1 } },
        { CellType::Code::TRI3_CODE, { 0, 1, 2 } },
        { CellType::Code::TRI6_CODE, { 0, 3, 1, 4, 2, 5 } },
        { CellType::Code::QUAD4_CODE, { 0, 1, 2, 3 } },
        { CellType::Code::QUAD8_CODE, { 0, 4, 1, 5, 2, 6, 3, 7 } },
        { CellType::Code::TETRA4_CODE, { 0, 2, 1, 3 } },
        { CellType::Code::TETRA10_CODE, { 0, 6, 2, 5, 1, 4, 7, 9, 8, 3 } },
        { CellType::Code::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
        { CellType::Code::PENTA15_CODE, { 0, 8, 2, 7, 1, 6, 12, 14, 13, 3, 11, 5, 10, 4, 9 } },
        { CellType::Code::PYRA5_CODE, { 0, 3, 2, 1, 4, 4, 4, 4 } },
        { CellType::Code::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
        { CellType::Code::HEXA20_CODE, { 0, 11, 3, 10, 2, 9, 1, 8, 16, 19, 18, 17, 4, 15, 7, 14,
                6, 13, 5, 12 } },
        { CellType::Code::POLY3_CODE, { 0, 1, 2 } },
        { CellType::Code::POLY4_CODE, { 0, 1, 2, 3 } },
        { CellType::Code::POLY5_CODE, { 0, 1, 2, 3, 4 } },
        { CellType::Code::POLY6_CODE, { 0, 1, 2, 3, 4, 5 } },
        { CellType::Code::POLY7_CODE, { 0, 1, 2, 3, 4, 5, 6 } },
        { CellType::Code::POLY8_CODE, { 0, 1, 2, 3, 4, 5, 6, 7 } },
        { CellType::Code::POLY9_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8 } },
        { CellType::Code::POLY10_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
        { CellType::Code::POLY11_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
        { CellType::Code::POLY12_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } },
        { CellType::Code::POLY13_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } },
        { CellType::Code::POLY14_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 } },
        { CellType::Code::POLY15_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 } },
        { CellType::Code::POLY16_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } },
        { CellType::Code::POLY17_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } },
        { CellType::Code::POLY18_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 } },
        { CellType::Code::POLY19_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 } },
        { CellType::Code::POLY20_CODE, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } }

};

const int SystusWriter::DampingAccessId=41;
const int SystusWriter::MassAccessId=42;
const int SystusWriter::StiffnessAccessId=43;

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




// TODO: That should not be in the Writer.
CartesianCoordinateSystem SystusWriter::buildElementDefaultReferentiel(const SystusModel& systusModel, const vector<int> nodes, const int dim,
        const int celltype){

    VectorialValue O,x,y;
    const Mesh& mesh = systusModel.model.mesh;

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
        if (celltype==6) {
            x = VectorialValue::X;
            y = VectorialValue::Y;
            O = VectorialValue(0,0,0);
        // Other 1D elements are computed from the orientation of the "bar"
        } else {
            const Node& nO = mesh.findNode(mesh.findNodePosition(nodes[0]));
            const Node& nX = mesh.findNode(mesh.findNodePosition(nodes[1]));
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

        // The origin of the referentiel is the center of the element.
        O = VectorialValue(0,0,0);
        int nbnodes=0;
        for (const auto& n : nodes){
            const Node& node = mesh.findNode(mesh.findNodePosition(n));
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
                const Node& nA0 = mesh.findNode(mesh.findNodePosition(nodes[0]));
                const Node& nA1 = mesh.findNode(mesh.findNodePosition(nodes[1]));
                const Node& nA2 = mesh.findNode(mesh.findNodePosition(nodes[2]));

                const VectorialValue& a0a1 = VectorialValue(nA1.x-nA0.x, nA1.y-nA0.y, nA1.z-nA0.z);
                const VectorialValue& a0a2 = VectorialValue(nA2.x-nA0.x, nA2.y-nA0.y, nA2.z-nA0.z);
                const VectorialValue& z = a0a1.cross(a0a2);

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

        } else {
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

    CartesianCoordinateSystem rcs = CartesianCoordinateSystem(systusModel.model.mesh, O, x, y);
    rcs.build();
    return rcs;

}

int SystusWriter::auto_part_id = 9999999;

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



void SystusWriter::fillDOFSMaterialField(const DOFS dofs, map<SMF, string> & systusMat) const {
    if (dofs.contains(DOF::DX))
        fillMaterialField(SMF::KX, 1.0, systusMat);
    if (dofs.contains(DOF::DY))
        fillMaterialField(SMF::KY, 1.0, systusMat);
    if (dofs.contains(DOF::DZ))
        fillMaterialField(SMF::KZ, 1.0, systusMat);
    if (dofs.contains(DOF::RX))
        fillMaterialField(SMF::IX, 1.0, systusMat);
    if (dofs.contains(DOF::RY))
        fillMaterialField(SMF::IY, 1.0, systusMat);
    if (dofs.contains(DOF::RZ))
        fillMaterialField(SMF::IZ, 1.0, systusMat);
}


void SystusWriter::fillMaterialField(const SMF key, const double value, map<SMF, string> & systusMat) const{
    ostringstream out;
    out.precision(DBL_DIG);
    switch(key){
    case(SMF::RHO):
    case(SMF::E):
    case(SMF::NU):{
        if (value>0.0){
            out << value;
            systusMat[key]=out.str();
        }
        break;
    }
    case(SMF::S):
    case(SMF::IX):
    case(SMF::IY):
    case(SMF::IZ):
    case(SMF::H):
    case(SMF::KX):
    case(SMF::KY):
    case(SMF::KZ):
    case(SMF::COEF):{
        out << value;
        systusMat[key]=out.str();
        break;
    }
    case(SMF::ALPHA):
    case(SMF::AY):
    case(SMF::AZ):{
        if (!is_equal(value, vega::Globals::UNAVAILABLE_DOUBLE)){
            out << value;
            systusMat[key]=out.str();
        }
        break;
    }
    default:{
        cerr << "Warning : Unrecognized material field (double value) "<< SMFToString(key) <<endl;
    }
    }
}

void SystusWriter::fillMaterialField(const SMF key, const int value, map<SMF, string> & systusMat) const{
    ostringstream out;
    out.precision(DBL_DIG);
    switch(key){
    case(SMF::ID):
    case(SMF::TABLE):
    case(SMF::SHAPE):
    case(SMF::MID):
    case(SMF::DEPEND):
    case(SMF::LEVEL):
    case(SMF::TYPE):{
        out << value;
        systusMat[key]=out.str();
        break;
    }
    default:{
        cerr << "Warning : Unrecognized material field (integer value) "<< SMFToString(key) <<endl;
    }
    }
}

string SystusWriter::writeModel(Model& model,
        const vega::ConfigurationParameters &configuration) {
    SystusModel systusModel{model, configuration};
    this->translationMode= configuration.translationMode;
    cout << "Writing to SYSTUS (version " << systusModel.getSystusVersionString()<<")"<< endl;

    string path = systusModel.configuration.outputPath;

    if (configuration.outputSolver.getSolverName() != SolverName::SYSTUS) {
        handleWritingError("Translation required for a different solver : " + configuration.outputSolver.to_str() + ", so cannot write it.");
    }

    if (!fs::exists(path)) {
        throw iostream::failure("Directory " + path + " don't exist.");
    }

    // On Systus output, we build a "general" solver file
    ofstream dat_file_ofs;
    string dat_path = systusModel.getOutputFileName("_ALL.DAT");
    if (configuration.systusOutputProduct=="systus"){
        dat_file_ofs.open(dat_path.c_str(), ios::trunc);
        if (!dat_file_ofs.is_open()) {
            throw ios::failure("Can't open file " + dat_path + " for writing.");
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
            throw ios::failure("Can't open file " + asc_path + " for writing.");
        }
        this->writeAsc(systusModel, idSubcase, asc_file_ofs);
        asc_file_ofs.close();

        /* Write some matrix files, if needed */
        this->writeMatrixFiles(systusModel, idSubcase);

        /* Analysis file */
        ofstream analyse_file_ofs;
        analyse_file_ofs.precision(DBL_DIG);
        string analyse_path = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + ".DAT");
        analyse_file_ofs.open(analyse_path.c_str(), ios::trunc);

        if (!analyse_file_ofs.is_open()) {
            throw ios::failure("Can't open file " + analyse_path + " for writing.");
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

void SystusWriter::getSystusAutomaticOption(const SystusModel& systusModel, SystusOption & autoSystusOption, SystusSubOption & autoSystusSubOption) {

    bool has1DOr2DElements = false;
    bool has3DElements = false;

    /**
     *  The list of elements supported by the 3D option is available in the
     *  SYSTUS Reference Manual  (section 8.4.3 Three Dimensional Analysis)
     **/
    for (const auto& elementSet : systusModel.model.elementSets) {
        switch (elementSet->type) {

        // Those elements need 6 DDL
        case ElementSet::Type::CIRCULAR_SECTION_BEAM:
        case ElementSet::Type::GENERIC_SECTION_BEAM:
        case ElementSet::Type::I_SECTION_BEAM:
        case ElementSet::Type::RECTANGULAR_SECTION_BEAM: {
            const auto& beam = static_pointer_cast<const Beam>(elementSet);
            has1DOr2DElements = has1DOr2DElements or not beam->isTruss();
            //has1DOr2DElements=true;
            break;
        }
        case ElementSet::Type::SHELL:{
            has1DOr2DElements=true;
            break; // -Wimplicit-fallthrough
        }

        /*
         * Those elements can be either with 3 or 6 DLLs, depending on the user
         * use. We could check every one of them to know which is what but it
         * seems a waste of time. For now, we suppose they are all 6DLLs
         */
        case ElementSet::Type::STIFFNESS_MATRIX:
        case ElementSet::Type::MASS_MATRIX:
        case ElementSet::Type::DAMPING_MATRIX:{
            has1DOr2DElements= true;
            break; // -Wimplicit-fallthrough
        }

        /*
         * Those elements can be either with 3 or 6 DLLs, depending on the user
         * use.
         */
        case ElementSet::Type::LMPC:{
            const auto& lmpc = static_pointer_cast<const Lmpc>(elementSet);
            has1DOr2DElements= has1DOr2DElements or lmpc->hasRotations();
            has3DElements= has3DElements or not(lmpc->hasRotations());
            break; // -Wimplicit-fallthrough
        }
        case ElementSet::Type::SCALAR_SPRING:{
            const auto& discrete = static_pointer_cast<const Discrete>(elementSet);
            has1DOr2DElements= has1DOr2DElements or discrete->hasRotations();
            has3DElements= has3DElements or not(discrete->hasRotations());
            break; // -Wimplicit-fallthrough
        }

        // Those are solid elements: only 3DDLs needed.
        case ElementSet::Type::SKIN:
        case ElementSet::Type::CONTINUUM:{
            has3DElements=true;
            break; // -Wimplicit-fallthrough
        }

        // All these elements are available on both 3D and shell modes.
        //   - Structural Segment are translated with 1602
        //   - RBAR, RBE2 (RBE2 are viewed as an assembly of RBAR) and RBE3 are available on both 3D and Shell modes
        case ElementSet::Type::RBAR:
        case ElementSet::Type::RBE3:
        case ElementSet::Type::STRUCTURAL_SEGMENT:{
            continue;
        }

        // Not translated as cells.
        case ElementSet::Type::DISCRETE_0D:
        case ElementSet::Type::DISCRETE_1D:
        case ElementSet::Type::NODAL_MASS:{
            continue;
        }

        default: {
            handleWritingWarning("Unknown type of ElementSet for "+to_str(*elementSet), "Automatic Option");}
        } /* switch */
        // No need to test everything if we already know the needed informations.
        if (has1DOr2DElements&&has3DElements){
            break;
        }
    }

    if (has1DOr2DElements){
        autoSystusOption = SystusOption::SHELL;
        autoSystusSubOption = has3DElements ? SystusSubOption::MULTI : SystusSubOption::NONE;
    }else{
        autoSystusOption = SystusOption::CONTINUOUS;
        autoSystusSubOption = SystusSubOption::NONE;
    }

}


void SystusWriter::getSystusInformations(const SystusModel& systusModel, const ConfigurationParameters& configurationParameters) {

    // Compute automatic Options
    SystusOption autoSystusOption;
    SystusSubOption autoSystusSubOption;
    getSystusAutomaticOption(systusModel, autoSystusOption, autoSystusSubOption);
    if (configurationParameters.logLevel >= LogLevel::INFO){
       cout << "Automatic option is "<<SystusOptionToString(autoSystusOption,autoSystusSubOption)<<endl;
    }


    // We choose between the automatic and user-defined option
    if (configurationParameters.systusOptionAnalysis =="auto"){
        systusOption = autoSystusOption;
        systusSubOption = autoSystusSubOption;
    }else if (configurationParameters.systusOptionAnalysis =="shell"){
        systusOption = SystusOption::SHELL;
        systusSubOption = SystusSubOption::NONE;
    }else if(configurationParameters.systusOptionAnalysis =="shell-multi"){
        systusOption = SystusOption::SHELL;
        systusSubOption = SystusSubOption::MULTI;
    }else if(configurationParameters.systusOptionAnalysis =="3D"){
        systusOption = SystusOption::CONTINUOUS;
        systusSubOption = SystusSubOption::NONE;
    }else{
        handleWritingError("Unknown option analysis: " +configurationParameters.systusOptionAnalysis, "SYSTUS Option");
    }

    // Warning message
    if ((systusOption != autoSystusOption) && (configurationParameters.logLevel >= LogLevel::INFO)){
        handleWritingWarning("User-defined SYSTUS option ("+SystusOptionToString(systusOption, systusSubOption)+
                ") is different from the automatic one ("+SystusOptionToString(autoSystusOption, autoSystusSubOption)
                +")", "SYSTUS Option");
    }

    // Define the available DOFs
    switch (systusOption) {
    case SystusOption::SHELL:
        availableDOFS = DOFS::ALL_DOFS;
        dofCode= static_cast<char>(availableDOFS);
        nbDOFS= 6;
        break;
    case SystusOption::CONTINUOUS:
        availableDOFS = DOFS::TRANSLATIONS;
        dofCode= static_cast<char>(availableDOFS);
        nbDOFS= 3;
        break;
    default:
        availableDOFS = DOFS::NO_DOFS;
        dofCode= static_cast<char>(availableDOFS);
        nbDOFS= 0;
        handleWritingError("systusOption not supported");
        break;
    }
}

double SystusWriter::generateRbarStiffness(const SystusModel& systusModel){

    // We access the maximum of the Young Modulus of the model. No need to do it twice
    if (is_equal(maxYoungModulus,Globals::UNAVAILABLE_DOUBLE)){
        maxYoungModulus=0.0;
        for (const auto& elementSet : systusModel.model.elementSets) {
            if (elementSet->effective() and elementSet->hasMaterials()){
                const auto& nature = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_ELASTIC);
                if (nature) {
                    const ElasticNature& elasticNature = dynamic_cast<const ElasticNature&>(*nature);
                    maxYoungModulus= max(maxYoungModulus, elasticNature.getE());
                }
            }
        }
    }

    // Master node is the same for all cells
    Mesh& mesh = systusModel.model.mesh;
    const MeshStatistics mS=mesh.calcStats();
    double quadraticMeanLength=mS.quadraticMeanLength;
    if (is_zero(quadraticMeanLength)){
        handleWritingWarning("Characteristic length of Rigid Element was null and raised to 1","Rbar Stiffness");
        quadraticMeanLength=1.0;
    }

    return maxYoungModulus*quadraticMeanLength;
}

//TODO: create a single function to compute penalty
//double SystusWriter::generateLmpcRigidity(const SystusModel& systusModel, const auto& lmpc){
//
//    const Mesh& mesh = systusModel.model.mesh;
//
//    // We access the maximum of the Young Modulus of the model. No need to do it twice
//    if (is_equal(maxYoungModulus,Globals::UNAVAILABLE_DOUBLE)){
//        maxYoungModulus=0.0;
//        for (const auto& elementSet : systusModel.model.elementSets) {
//            const auto& material = elementSet->material;
//            if ((elementSet->cellGroup != nullptr)&&(material != nullptr)){
//                const auto& nature = material->findNature(Nature::NatureType::NATURE_ELASTIC);
//                if (nature) {
//                    const ElasticNature& elasticNature = dynamic_cast<ElasticNature&>(*nature);
//                    maxYoungModulus= max(maxYoungModulus, elasticNature.getE());
//                }
//            }
//        }
//    }
//
//    // Master node is the same for all cells
//    double maxLength=0.0;
//    for (const Cell& cell : lmpc->cellGroup->getCells()) {
//        // The reference one is the first one. Why ? Why not ?
//        // TODO: a much much better formulation
//        const Node& mN = mesh.findNode(cell.nodePositions[0]);
//        for (unsigned int i=1;i<cell.nodePositions.size();i++){
//            const Node& sN = mesh.findNode(cell.nodePositions[i]);
//            double lengthLmpc = sqrt( pow(mN.x-sN.x,2) +pow(mN.y-sN.y,2) + pow(mN.z-sN.z,2));
//           maxLength=max(maxLength, lengthLmpc);
//        }
//    }
//
//    double rigidity = maxYoungModulus*maxLength;
//    if (is_zero(rigidity)){
//        handleWritingWarning("Computed rigidity for "+to_str(*lmpc)+" was null and raised to 1","LMPC Rigidity");
//        rigidity=1.0;
//    }
//
//    return rigidity;
//}


void SystusWriter::generateRBEs(SystusModel& systusModel,
        const vega::ConfigurationParameters &configuration) {

    Mesh& mesh = systusModel.model.mesh;
    rotationNodeIdByTranslationNodeId.clear();

    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::RBAR)) {
        const auto& rbars = static_pointer_cast<Rbar>(elementSet);

        if (not elementSet->hasMaterials()){
            handleWritingError("Error: ElementSet::RBAR have no material.");
        }
        const auto& nature = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_RIGID);
        if (nature == nullptr) {
            handleWritingError("Error: ElementSet::RBAR have no RIGID nature.");
        }

        // We update the material with the needed value
        double stiffness;
        if (is_equal(configuration.systusRBEStiffness, Globals::UNAVAILABLE_DOUBLE)) {
            stiffness= this->generateRbarStiffness(systusModel);
        } else {
            stiffness= configuration.systusRBEStiffness;
        }
        const auto& rigidNature = static_pointer_cast<RigidNature>(nature);
        if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0) {
            rigidNature->setLagrangian(configuration.systusRBECoefficient);
        } else {
            rigidNature->setRigidity(configuration.systusRBECoefficient*stiffness);
        }


        const int masterId = rbars->masterId;
        const auto masterPosition = mesh.findNodePosition(masterId);

        const Node& master = mesh.findNode(masterPosition);
        int master_rot_id = 0;

        if (systusOption == SystusOption::CONTINUOUS){
            const auto master_rot_position = mesh.addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
            master_rot_id = mesh.findNodeId(master_rot_position);
            rotationNodeIdByTranslationNodeId[master.id]=master_rot_id;
        }

        map<pos_t, pos_t> updatedPositionByOldPosition;
        for (const pos_t cellPosition : elementSet->cellPositions()) {
            const Cell& cell = mesh.findCell(cellPosition);
            auto nodes = cell.nodeIds;

            if (nodes.size() != 2){
                handleWritingError("Error: ElementSet::RBAR cells must have exactly two nodes.");
            }
            if (nodes[0] != masterId){
                handleWritingError("Error: the first node of ElementSet::RBAR cells must be the master node.");
            }

            if (systusOption == SystusOption::CONTINUOUS)
                nodes.push_back(master_rot_id);

            // With a Lagrangian formulation, we add a Lagrange node.
            // Lagrange node must NOT have an orientation, as they inherit it from the slave node.
            if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
                const Node& slave = mesh.findNode(cell.nodePositions[1]);
                const auto slave_lagr_position = mesh.addNode(Node::AUTO_ID, slave.lx, slave.ly, slave.lz, slave.positionCS, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
                const int slave_lagr_id = mesh.findNodeId(slave_lagr_position);
                nodes.push_back(slave_lagr_id);
            }

            // If needed, we update the Cell
            switch (nodes.size()){
            case (2):{
                break;
            }
            case (3):{
                const auto newPosition = mesh.updateCell(cell.id, CellType::POLY3, nodes, true);
                updatedPositionByOldPosition[cellPosition] = newPosition;
                break;
            }
            case(4):{
                const auto newPosition = mesh.updateCell(cell.id, CellType::POLY4, nodes, true);
                updatedPositionByOldPosition[cellPosition] = newPosition;
                break;
            }
            default:
                handleWritingError("Not (yet) implemented, maybe nothing to do in this case?");
            }

        }
        const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
        if (cellElementSet != nullptr) {
            for (const auto& updateEntry : updatedPositionByOldPosition) {
                cellElementSet->removeCellPositionExcludingGroups(updateEntry.first);
                cellElementSet->addCellPosition(updateEntry.second);
            }
        }
        rbars->markAsWritten();
    }

    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::RBE3)) {

        /*
         * Translation of RBE3
         * See Systus Reference Analysis Manual, Section 8.8 "Special Elements",
         * Subsection "Use of Averaging Type Solid Elements", p500.
        */
        const auto& rbe3 = static_pointer_cast<Rbe3>(elementSet);

        if (not elementSet->hasMaterials()){
            handleWritingError("Error: ElementSet::RBE3 have no material.");
        }
        const auto& nature = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_RIGID);
        if (!nature) {
            handleWritingError("Error: ElementSet::RBE3 have no RIGID nature.");
        }

        // We dont change the nature, it has already been correctly filed.
        //RigidNature& rigidNature = dynamic_cast<RigidNature&>(*nature);
        //rigidNature.setLagrangian(lagrangianValue);

        const int masterId = rbe3->masterId;
        const auto masterPosition = mesh.findNodePosition(masterId);

        const Node& master = mesh.findNode(masterPosition);

        // Creating a Lagrange node
        // Lagrange node must NOT have an orientation, as they inherit it from the slave node.
        const auto master_lagr_position = mesh.addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
        const int master_lagr_id = mesh.findNodeId(master_lagr_position);

        // Creating rotation nodes if needed
        int master_rot_id=0;
        int master_lagr_rot_id=0;
        if (systusOption == SystusOption::CONTINUOUS){
            const auto master_rot_position = mesh.addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, master.displacementCS);
            master_rot_id = mesh.findNodeId(master_rot_position);
            rotationNodeIdByTranslationNodeId[master.id]=master_rot_id;
            const auto master_lagr_rot_position = mesh.addNode(Node::AUTO_ID, master.lx, master.ly, master.lz, master.positionCS, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
            master_lagr_rot_id = mesh.findNodeId(master_lagr_rot_position);
        }

        // Updating the cells
        map<pos_t, pos_t> updatedPositionByOldPosition;
        for (const pos_t cellPosition : elementSet->cellPositions()) {
            const Cell& cell = mesh.findCell(cellPosition);
            auto nodes = cell.nodeIds;
            if (nodes.size()!=2){
                handleWritingError("Error: ElementSet::RBE3 cells must have exactly two nodes.");
            }
            if (nodes[0] != masterId){
                handleWritingError("Error: the first node of ElementSet::RBE3 cells must be the master node.");
            }

            if (systusOption == SystusOption::CONTINUOUS)
                nodes.push_back(master_rot_id);
            nodes.push_back(master_lagr_id);
            if (systusOption == SystusOption::CONTINUOUS){
                nodes.push_back(master_lagr_rot_id);
            }

            switch (nodes.size()){
            case (3):{
                const auto newPosition = mesh.updateCell(cell.id, CellType::POLY3, nodes, true);
                updatedPositionByOldPosition[cellPosition] = newPosition;
                break;
            }
            case(5):{
                const auto newPosition = mesh.updateCell(cell.id, CellType::POLY5, nodes, true);
                updatedPositionByOldPosition[cellPosition] = newPosition;
                break;
            }
            default:
                handleWritingError("Not (yet) implemented, maybe nothing to do in this case?");
            }
        }
        const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
        if (cellElementSet != nullptr) {
            for (const auto& updateEntry : updatedPositionByOldPosition) {
                cellElementSet->removeCellPositionExcludingGroups(updateEntry.first);
                cellElementSet->addCellPosition(updateEntry.second);
            }
        }
        rbe3->markAsWritten();
    }


    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::LMPC)) {

    // Translation of LMPC elements has X9XX Level 0 elements
    // See Systus Reference Analysis Manual: RIGID BODY Element (page 498)
    // Here we compute the material properties and, if needed, add a lagrangian node
        const auto& lmpc = static_pointer_cast<Lmpc>(elementSet);

        if (not elementSet->hasMaterials()) {
            handleWritingError("Error: ElementSet::Lmpc have no material.");
        }
        const auto& nature = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_RIGID);
        if (!nature) {
            handleWritingError("Error: ElementSet::Lmpc have no RIGID nature.");
        }

        // We update the material with the needed value
        double stiffness;
        if (is_equal(configuration.systusRBEStiffness, Globals::UNAVAILABLE_DOUBLE)){
            stiffness= this->generateRbarStiffness(systusModel);
        }else{
            stiffness= configuration.systusRBEStiffness;
        }
        RigidNature& rigidNature = dynamic_cast<RigidNature&>(*nature);
        if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
            rigidNature.setLagrangian(configuration.systusRBECoefficient);
        }else{
            rigidNature.setRigidity(configuration.systusRBECoefficient*stiffness);
        }

        // With a Lagrangian formulation, we add a Lagrange node.
        // Lagrange node must NOT have an orientation, as they inherit it from the original node.
        if (configuration.systusRBE2TranslationMode.compare("lagrangian")==0){
            map<pos_t, pos_t> updatedPositionByOldPosition;
            for (const pos_t cellPosition : elementSet->cellPositions()) {
                const Cell& cell = mesh.findCell(cellPosition);
                //cout << "Adding lagrangian to cell:" << cell << endl;
                auto nodes = cell.nodeIds;
                const Node& first = mesh.findNode(cell.nodePositions[0]);
                const auto first_lagr_position = mesh.addNode(Node::AUTO_ID, first.lx, first.ly, first.lz, first.positionCS, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
                const auto first_lagr_id = mesh.findNodeId(first_lagr_position);
                nodes.push_back(first_lagr_id);
                const auto newPosition = mesh.updateCell(cell.id, CellType::polyType(static_cast<unsigned int>(nodes.size())), nodes, true);
                updatedPositionByOldPosition[cellPosition] = newPosition;
            }
            const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
            if (cellElementSet != nullptr) {
                for (const auto& updateEntry : updatedPositionByOldPosition) {
                    cellElementSet->removeCellPositionExcludingGroups(updateEntry.first);
                    cellElementSet->addCellPosition(updateEntry.second);
                }
            }
        }
        lmpc->markAsWritten();
    }
}


void SystusWriter::generateSubcases(const SystusModel& systusModel,
        const vega::ConfigurationParameters &configuration) {

    systusSubcases.clear();

    vector< vector<systus_ascid_t> > characteristicAnalysis;
    // Default is "Automatic merge of static subcases".
    if (configuration.systusSubcases.empty()){
        for (const auto& it : systusModel.model.analyses) {
            Analysis& analysis = *it;

            // We create a vector with the characteristic of the Analysis
            vector<systus_ascid_t> cAna;
            cAna.push_back(static_cast<int>(analysis.type)); //Analysis type

            // Only constraints must be considered: we can merge two analysis if they share the same matrix, even if they have different loadings
            const auto& constraintSets = analysis.getConstraintSets();
            cAna.push_back(static_cast<int>(constraintSets.size()));
            for (const auto& constraintSet : constraintSets){
                cAna.push_back(constraintSet->getId());
                constraintSet->markAsWritten();
            }

            // For mechanical static problem, we search the already defined subcases for the same characteristic
            auto idSubcase = systusSubcases.size();
            if (analysis.type == Analysis::Type::LINEAR_MECA_STAT){
                for (systus_ascid_t i=0; i<characteristicAnalysis.size(); i++){
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
            } else {
                // Already existing subcases
                systusSubcases[idSubcase].push_back(analysis.getId());
            }
            analysis.markAsWritten();

        }
    } else {

        // On "single mode", each analysis is in its own subcase.
        if ((configuration.systusSubcases.size()==1) &&
                (configuration.systusSubcases[0][0]==-1)){
            for (const auto& it : systusModel.model.analyses) {
                const Analysis& analysis = *it;
                systusSubcases.push_back({analysis.getId()});
            }
        } else {
            // We use the user defined lists

            // We make a correspondence between the User defined Id (aka "Original Id")
            // and the internal Id.
            map<int, int> analysisIdByOriginalId;
            for (const auto& it : systusModel.model.analyses) {
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
                const auto& subcase = systusSubcases[i];
                const auto& refAnalysis = systusModel.model.getAnalysis(subcase[0]);
                const auto refType = refAnalysis->type;
                for (unsigned j = 1; j < subcase.size(); j++){
                    const auto analysis = systusModel.model.getAnalysis(subcase[j]);
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
                const auto analysis = systusModel.model.getAnalysis(subcase[j]);
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
    const auto& analysisId = systusSubcases[idSubcase];

    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const auto& analysis = systusModel.model.getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            cerr << "Warning in Filling Loads : wrong analysis number ("<< analysisId[i]<<") Analysis dismissed"<<endl;
            break;
        }
        localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]={};
        const auto& analysisLoadSets = analysis->getLoadSets();
        int idSystusLoadByAnalysis=0;
        for (const auto& loadSet : analysisLoadSets) {
            localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()][loadSet->getId()]= idSystusLoad;

            // Title of Loadset is of the form AnalysisName_lLoadId.
            // It is limited to 80 characters
            string suffixe = "_LOAD"+to_string(loadSet->bestId());
            localLoadingListName[idSystusLoad]= analysis->getLabel().substr(0, 80 - suffixe.length())+ suffixe;
            idSystusLoad++;
            idSystusLoadByAnalysis++;
        }

        // We need at least one loadset by analysis
        if (idSystusLoadByAnalysis==0){
            localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()][0]= idSystusLoad;
            localLoadingListName[idSystusLoad]= analysis->getLabel().substr(0, 80);
            idSystusLoad++;
        }
    }
}

void SystusWriter::writeNodalForceVector(const SystusModel& systusModel, const shared_ptr<NodalForce>& nodalForce, const int idLoadCase, systus_ascid_t& vectorId) {

    for(const pos_t nodePosition : nodalForce->nodePositions()) {
        const auto& force = nodalForce->getForceInGlobalCS(nodePosition);
        const auto& moment = nodalForce->getMomentInGlobalCS(nodePosition);
        const int nodeId = systusModel.model.mesh.findNodeId(nodePosition);

        vector<double> vec;
        double normvec = 0.0;
        vec.push_back(0);
        vec.push_back(0);
        vec.push_back(0);
        vec.push_back(0);
        vec.push_back(0);
        vec.push_back(0);
        vec.push_back(force.x()); normvec=max(normvec, abs(force.x()));
        vec.push_back(force.y()); normvec=max(normvec, abs(force.y()));
        vec.push_back(force.z()); normvec=max(normvec, abs(force.z()));
        if (systusOption == SystusOption::SHELL){
            vec.push_back(moment.x()); normvec=max(normvec, abs(moment.x()));
            vec.push_back(moment.y()); normvec=max(normvec, abs(moment.y()));
            vec.push_back(moment.z()); normvec=max(normvec, abs(moment.z()));
        }
        if (!is_zero(normvec)){
            vectors[vectorId]=vec;
            loadingVectorsIdByLocalLoadingByNodePosition[nodePosition][idLoadCase].push_back(vectorId);
            vectorId++;
        }
        // Rigid Body Element in option 3D.
        // We report the force from the master node to the master rotational node.
        if (systusOption == SystusOption::CONTINUOUS){
            const auto & it = rotationNodeIdByTranslationNodeId.find(nodeId);
            if (it != rotationNodeIdByTranslationNodeId.end()){
                const auto rotNodePosition = systusModel.model.mesh.findNodePosition(it->second);
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
    }
}

void SystusWriter::fillLoadingsVectors(const SystusModel& systusModel, const int idSubcase){

    // First available vector
    systus_ascid_t vectorId= vectors.size()+1;

    // All analysis to do
    const auto& analysisId = systusSubcases[idSubcase];

    // Work, work
    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const auto& analysis = systusModel.model.getAnalysis(analysisId[i]);

        if (analysis == nullptr){
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

                switch (loading->type) {
                case Loading::Type::NODAL_FORCE: {
                    const auto& nodalForce = static_pointer_cast<NodalForce>(loading);
                    writeNodalForceVector(systusModel, nodalForce, idLoadCase, vectorId);
                    nodalForce->markAsWritten();
                    break;
                }

                case Loading::Type::NORMAL_PRESSION_FACE: {
                    const auto& npf = static_pointer_cast<NormalPressionFace>(loading);
                    vector<double> vec;
                    vec.push_back(1);
                    vec.push_back(0);
                    vec.push_back(1); // This is a force normal to the surface of the element
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0.0);
                    vec.push_back(0.0);
                    vec.push_back(npf->intensity);
                    if (!is_zero(npf->intensity)){
                        vectors[vectorId]=vec;
                        for (const int cellId : npf->getCellIdsIncludingGroups()){
                          loadingVectorsIdByLocalLoadingByCellId[cellId][idLoadCase].push_back(vectorId);
                        }
                        vectorId++;
                    }
                    npf->markAsWritten();
                    break;
                }
                case Loading::Type::NORMAL_PRESSION_SHELL: {
                    const auto& npf = static_pointer_cast<NormalPressionShell>(loading);
                    vector<double> vec;
                    vec.push_back(1);
                    vec.push_back(0);
                    vec.push_back(1); // This is a force normal to the surface of the element
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0.0);
                    vec.push_back(0.0);
                    vec.push_back(npf->intensity);
                    if (!is_zero(npf->intensity)){
                        vectors[vectorId]=vec;
                        for (const int cellId : npf->getCellIdsIncludingGroups()){
                          loadingVectorsIdByLocalLoadingByCellId[cellId][idLoadCase].push_back(vectorId);
                        }
                        vectorId++;
                    }
                    npf->markAsWritten();
                    break;
                }
                case Loading::Type::GRAVITY: {
                    const auto& gravity = static_pointer_cast<Gravity>(loading);
                    VectorialValue acceleration = gravity->getAccelerationVector();
                    vector<double> vec;
                    double normvec = 0.0;
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
                    gravity->markAsWritten();
                    break;
                }

                case Loading::Type::FORCE_SURFACE: {
                    const auto& forceSurface = static_pointer_cast<ForceSurface>(loading);
                    VectorialValue force = forceSurface->getForce();
                    VectorialValue moment = forceSurface->getMoment();
                    vector<double> vec;
                    double normvec = 0.0;
                    vec.push_back(1);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(force.x()); normvec=max(normvec, abs(force.x()));
                    vec.push_back(force.y()); normvec=max(normvec, abs(force.y()));
                    vec.push_back(force.z()); normvec=max(normvec, abs(force.z()));
                    if (systusOption == SystusOption::SHELL) {
                        vec.push_back(moment.x()); normvec=max(normvec, abs(moment.x()));
                        vec.push_back(moment.y()); normvec=max(normvec, abs(moment.y()));
                        vec.push_back(moment.z()); normvec=max(normvec, abs(moment.z()));
                    }
                    if (!is_zero(normvec)){
                        vectors[vectorId]=vec;
                        for (const int cellId : forceSurface->getCellIdsIncludingGroups()){
                          loadingVectorsIdByLocalLoadingByCellId[cellId][idLoadCase].push_back(vectorId);
                        }
                        vectorId++;
                    }
                    forceSurface->markAsWritten();
                    break;
                }

                case Loading::Type::FORCE_LINE: {
                    const auto& forceLine = static_pointer_cast<ForceLine>(loading);
                    double FX=0.0;
                    double FY=0.0;
                    double FZ=0.0;
                    double MX=0.0;
                    double MY=0.0;
                    double MZ=0.0;
                    switch(forceLine->dof.code) {
                    case DOF::Code::DX_CODE:
                        FX = 1.0;
                        break;
                    case DOF::Code::DY_CODE:
                        FY = 1.0;
                        break;
                    case DOF::Code::DZ_CODE:
                        FZ = 1.0;
                        break;
                    case DOF::Code::RX_CODE:
                        MX = 1.0;
                        break;
                    case DOF::Code::RY_CODE:
                        MY = 1.0;
                        break;
                    case DOF::Code::RZ_CODE:
                        MZ =1.0;
                        break;
                    default:
                        handleWritingError("DOF not yet handled", "Loadings");
                    }

                    // Only uniform distribution are supported yet
                    const auto& functionTable = static_pointer_cast<FunctionTable>(forceLine->force);
                    auto it = functionTable->getBeginValuesXY();
                    double p1 = it->second;
                    it++;
                    while (it != functionTable->getEndValuesXY()){
                        double p2 = it->second;
                        if (!(is_equal(p1,p2))){
                            handleWritingWarning("Only uniform linear forces are supported. First values kept.", "Loadings");
                            it=functionTable->getEndValuesXY();
                        }else{
                            it++;
                        }
                    }
                    vector<double> vec;
                    double normvec = 0.0;
                    vec.push_back(1);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(0);
                    vec.push_back(FX*p1); normvec=max(normvec, abs(FX));
                    vec.push_back(FY*p1); normvec=max(normvec, abs(FY));
                    vec.push_back(FZ*p1); normvec=max(normvec, abs(FZ));
                    if (systusOption == SystusOption::SHELL) {
                        vec.push_back(MX*p1); normvec=max(normvec, abs(MX));
                        vec.push_back(MY*p1); normvec=max(normvec, abs(MY));
                        vec.push_back(MZ*p1); normvec=max(normvec, abs(MZ));
                    }
                    if (!is_zero(normvec)){
                        vectors[vectorId]=vec;
                        for (const int cellId : forceLine->getCellIdsIncludingGroups()){
                          loadingVectorsIdByLocalLoadingByCellId[cellId][idLoadCase].push_back(vectorId);
                        }
                        vectorId++;
                    }
                    forceLine->markAsWritten();
                    break;
                }


                case Loading::Type::DYNAMIC_EXCITATION:{
                    const auto& dE = static_pointer_cast<DynamicExcitation>(loading);
                    const auto& dEL = dE->getLoadSet();
                    if (dEL->type != LoadSet::Type::EXCITEID){
                        handleWritingWarning("Dynamic loading must refer an EXCITED Loadset. Dismissing load "+ to_string(dE->bestId()));
                        break;
                    }

                    // Systus does not support any addition to the phase of the excitation
                    if (!is_zero(dE->getDynaDelay()->get())){
                        handleWritingWarning("Delay are not available in Systus and dismissed.", "Loadings");
                    }
                    if (!is_zero(dE->getDynaPhase()->get())){
                        handleWritingWarning("Phase are not available in Systus and dismissed.", "Loadings");
                    }
                    if (dE->getFunctionTableP()){
                        handleWritingWarning("Table Phase are not available in Systus and dismissed.", "Loadings");
                    }
                    vector<double> vec;

                    // Frequency amplitude
                    double amplitude=1.0;
                    if (systusModel.configuration.systusDynamicMethod=="direct"){
                        const auto& aTable = dE->getFunctionTableB();

                        //TODO: Test the units of the table ?
                        amplitude = aTable->getBeginValuesXY()->second;
                        // In direct mode, SYSTUS 2017 allows only amplitude that doesn't depend of the frequency
                        for (auto it = aTable->getBeginValuesXY(); it != aTable->getEndValuesXY(); it++){
                            if (!is_equal(amplitude, it->second)){
                                handleWritingWarning("In direct mode, amplitude of dynamic excitation can't depend of the frequency. Constant amplitude is assumed.", "Loadings");
                            }
                        }
                    }

                    for (const auto& dLoading : dEL->getLoadings()) {

                        if (dLoading->type!=Loading::Type::NODAL_FORCE){
                            handleWritingWarning("Dynamic loading must refer Nodal excitation. Dismissing load "+to_string(dLoading->bestId())+", part of load "+ to_string(dE->bestId()));
                            continue;
                        }

                        const auto& nodalForce = static_pointer_cast<NodalForce>(dLoading);
                        writeNodalForceVector(systusModel, nodalForce, idLoadCase, vectorId);
                        nodalForce->markAsWritten();
                    }
                    dEL->markAsWritten();
                    dE->markAsWritten();
                    break;
                }

                // This loading is translated via a constraint vector.
                case Loading::Type::IMPOSED_DISPLACEMENT:{
                    break;
                }
                default: {
                    handleWritingWarning(to_str(*loading) +" not supported.", "Loadings");
                }
                }
            }
            loadset->markAsWritten();
        }
    }
}


void SystusWriter::fillConstraintsVectors(const SystusModel& systusModel, const int idSubcase){

    // First available vector
    auto vectorId = vectors.size()+1;

    // All analysis to parse
    const auto& analysisId = systusSubcases[idSubcase];

    // We add constraints coming from ConstraintSets
    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const auto& analysis = systusModel.model.getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            handleWritingWarning("Wrong analysis number. Analysis dismissed", "Constraint vectors");
            break;
        }

        for (const auto& constraintset : analysis->getConstraintSets()){
            for (auto& constraint : constraintset->getConstraintsByType(Constraint::Type::SPC)) {
                vector<double> vec;
                double normvec=0.0;
                const auto& spc = static_pointer_cast<SinglePointConstraint>(constraint);
                if (spc->hasReferences()) {
                    handleWritingWarning(to_str(*constraint) + " with reference to functions not supported", "Constraint vectors");
                    spc->markAsWritten();
                    continue;
                }
                normvec= initSystusAscConstraintVector(vec);
                DOFS spcDOFS = spc->getDOFSForNode(0);
                for (DOF dof : availableDOFS){
                    double value = (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0);
                    normvec = max(normvec, abs(value));
                    vec.push_back(value);
                }

                if (!is_zero(normvec)){
                    vectors[vectorId]=vec;
                    for (const auto& it : localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]){
                        for (const auto nodePosition : constraint->nodePositions()){
                            constraintVectorsIdByLocalLoadingByNodePosition[nodePosition][it.second].push_back(vectorId);
                        }
                    }
                    vectorId++;
                }
                // Rigid Body Element in option 3D.
                // We report the constraints from the master node to the master rotational node.
                if (systusOption == SystusOption::CONTINUOUS){

                    // Rotation vector
                    normvec = initSystusAscConstraintVector(vec);
                    for (DOF dof : DOFS::ROTATIONS){
                        double value = (spcDOFS.contains(dof) ? spc->getDoubleForDOF(dof) : 0);
                        normvec = max(normvec, abs(value));
                        vec.push_back(value);
                    }

                    if (!is_zero(normvec)) {
                        bool firstTime=true;
                        for (const auto nodePosition : constraint->nodePositions()) {
                            const int nid = systusModel.model.mesh.findNodeId(nodePosition);
                            const auto& it = rotationNodeIdByTranslationNodeId.find(nid);
                            if (it != rotationNodeIdByTranslationNodeId.end()) {
                                const auto rotNodePosition= systusModel.model.mesh.findNodePosition(it->second);
                                for (const auto& it2 : localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]){
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
                spc->markAsWritten();
            }
            constraintset->markAsWritten();
        }
    }


    // Here, we compute the vectors needed for imposed displacement.
    // We do this AFTER the constraintSet, because the imposed displacement supersedes the generic constraint vector.
    for (unsigned i = 0 ; i < analysisId.size(); i++) {
        const auto& analysis = systusModel.model.getAnalysis(analysisId[i]);

        if (analysis==nullptr){
            handleWritingWarning("Wrong analysis number. Analysis dismissed", "Constraint vectors");
            break;
        }

        // Add constraint Vectors
        for (const auto& loadset : analysis->getLoadSets()){
            for (const auto& loading : loadset->getLoadingsByType(Loading::Type::IMPOSED_DISPLACEMENT)) {
                const auto& spcd = static_pointer_cast<ImposedDisplacement>(loading);
                vector<double> vec;
                double normvec = initSystusAscConstraintVector(vec);

                for (DOF dof : availableDOFS){
                    double value = spcd->getDoubleForDOF(dof);
                    if (is_equal(value, Globals::UNAVAILABLE_DOUBLE)) {
                        value=0.0;
                    }
                    normvec = max(normvec, abs(value));
                    vec.push_back(value);
                }

                if (!is_zero(normvec)){
                    vectors[vectorId]=vec;
                    for (const auto& it : localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()]){
                        for (const auto nodePosition : spcd->nodePositions()){
                            constraintVectorsIdByLocalLoadingByNodePosition[nodePosition][it.second].push_back(vectorId);
                        }
                    }
                    vectorId++;
                }

                // Rigid Body Element in option 3D.
                // We report the constraints from the master node to the master rotational node.
                // Todo: factorize this part
                if (systusOption==SystusOption::CONTINUOUS){

                    // Rotation vector
                    normvec = initSystusAscConstraintVector(vec);
                    for (DOF dof : DOFS::ROTATIONS){
                        //double value = (spcDOFS.contains(dof) ? spcd->getDoubleForDOF(dof) : 0);
                        double value = spcd->getDoubleForDOF(dof);
                        if (is_equal(value, Globals::UNAVAILABLE_DOUBLE)){
                            value = 0.0;
                        }
                        normvec = max(normvec, abs(value));
                        vec.push_back(value);
                    }

                    if (!is_zero(normvec)) {
                        bool firstTime = true;
                        for (const auto nodePosition : spcd->nodePositions()){
                            const int nid = systusModel.model.mesh.findNodeId(nodePosition);
                            const auto & it = rotationNodeIdByTranslationNodeId.find(nid);
                            if (it!=rotationNodeIdByTranslationNodeId.end()){
                                const auto rotNodePosition= systusModel.model.mesh.findNodePosition(it->second);
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
                spcd->markAsWritten();
            }
            loadset->markAsWritten();
        }
    }
}


void SystusWriter::fillCoordinatesVectors(const SystusModel& systusModel, const int idSubcase){

    UNUSEDV(idSubcase);

    // First available vector
    systus_ascid_t vectorId = vectors.size()+1;
    map<pos_t, systus_ascid_t> localVectorIdByCoordinateSystemPos;

    // Add vectors for Node Coordinate System
    // Remark: Element Coordinate System are not translated as vectors
    Mesh& mesh = systusModel.model.mesh;
    for (const auto& node : mesh.nodes) {
        if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){

            // Trick for not doing the Cartesian coordinate systems all over again;
            auto it = localVectorIdByCoordinateSystemPos.find(node.displacementCS);
            if (it != localVectorIdByCoordinateSystemPos.end()){
                localVectorIdByNodePosition[node.position] = it->second;
                continue;
            }

            const auto& cs = systusModel.model.mesh.getCoordinateSystemByPosition(node.displacementCS);
            vector<double> vec;
            switch (cs->type) {
            case CoordinateSystem::Type::ABSOLUTE: {
                switch (cs->coordType){
                case CoordinateSystem::CoordinateType::CARTESIAN:{
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
                // Cylyndrical orientation : we create a vector by point
                case CoordinateSystem::CoordinateType::CYLINDRICAL:{
                    const Node& nNode = mesh.findNode(node.position);
                    const auto& ccs = static_pointer_cast<CylindricalCoordinateSystem>(cs);
                    ccs->updateLocalBase(VectorialValue(nNode.x, nNode.y, nNode.z));
                    const auto& angles = ccs->getLocalEulerAnglesIntrinsicZYX(); // (PSI, THETA, PHI)
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
                    handleWritingWarning("We will fill a null vector for the node "+ to_string(mesh.findNodeId(node.position)));
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
                break;
            }
            default: {
                handleWritingWarning("Non position local coordinate system are forbidden for nodes.");
                handleWritingWarning("We will fill a null vector for the node "+ to_string(mesh.findNodeId(node.position)));
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

    Mesh& mesh = systusModel.model.mesh;


    // All analysis of the subcase
    // We only work on the first one, as they have the same constraints (normally!)
    const auto& analysisId = systusSubcases[idLoadcase];
    if (analysisId.size()==0){//Mode: mesh_only
        return;
    }
    const auto& analysis = systusModel.model.getAnalysis(analysisId[0]);
    if (analysis==nullptr){
        handleWritingError("Wrong analysis number in Constraint Node. Analysis dismissed.");
        return;
    }

    // Work
    for (const auto& constraintSet : analysis->getConstraintSets()) {
        for (auto& constraint : constraintSet->getConstraintsByType(Constraint::Type::SPC)) {
            //const auto& spc = static_pointer_cast<SinglePointConstraint>(constraint);
            for (const auto nodePosition : constraint->nodePositions()) {

                // We compute the Degree Of Freedom of the node (see ASC Manual)
                DOFS constrained = constraint->getDOFSForNode(nodePosition);
                if (constraintByNodePosition.find(nodePosition) == constraintByNodePosition.end()) {
                    constraintByNodePosition[nodePosition] = char(constrained) & dofCode;
                } else {
                    constraintByNodePosition[nodePosition] = (char(constrained) & dofCode)
                                        | constraintByNodePosition[nodePosition];
                }

                // Rigid Body Element in option 3D.
                // We report the constraints from the master node to the master rotational node.
                if (systusOption==SystusOption::CONTINUOUS){
                    const int nid =  mesh.findNodeId(nodePosition);
                    const auto & it = rotationNodeIdByTranslationNodeId.find(nid);
                    if (it != rotationNodeIdByTranslationNodeId.end()){
                        DOFS constrainedRot(constrained.contains(DOF::RX),constrained.contains(DOF::RY),constrained.contains(DOF::RZ));
                        const auto rotNodePosition= mesh.findNodePosition(it->second);
                        if (constraintByNodePosition.find(rotNodePosition) == constraintByNodePosition.end()){
                            constraintByNodePosition[rotNodePosition] = char(constrainedRot) & dofCode;
                        } else {
                            constraintByNodePosition[rotNodePosition] = (char(constrainedRot) & dofCode)
                                                | constraintByNodePosition[rotNodePosition];
                        }
                    }
                }
            }
            constraint->markAsWritten();
        }
        constraintSet->markAsWritten();
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
        vector<systus_ascid_t> sl;
        for (const auto & it2 : it.second){
            for (const systus_ascid_t vectorId : it2.second){
                sl.push_back(it2.first);
                sl.push_back(vectorId);
            }
        }
        lists[idSystusList]= sl;
        idSystusList++;
    }

    // Building lists for Loading on cells
    for (const auto& it : loadingVectorsIdByLocalLoadingByCellId){
        loadingListIdByCellId[it.first] = idSystusList;
        vector<systus_ascid_t> sl;
        for (const auto & it2 : it.second){
            for (const systus_ascid_t vectorId : it2.second){
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
        vector<systus_ascid_t> sl;
        for (const auto & it2 : it.second){
            for (const systus_ascid_t vectorId : it2.second){
                sl.push_back(it2.first);
                sl.push_back(vectorId);
            }
        }
        lists[idSystusList]= sl;
        idSystusList++;
    }
}


void SystusWriter::fillTables(const SystusModel& systusModel, const int idSubcase) {


    if (systusModel.configuration.systusOutputMatrix=="table") {


        // Fill tables for Stiffness, Mass and Damping elements
        for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::SCALAR_SPRING)) {

            const auto& ss = static_pointer_cast<ScalarSpring>(elementSet);
            const auto& dofsSpring = ss->getDOFSSpring();
            if (dofsSpring.size()!=1){
                handleWritingWarning(to_str(*elementSet) + " spring directions after the first one will be dismissed.", "FillTable");
            }
            const auto& pairDOF = dofsSpring[0];
            // If we have the same DOF, we can use a Spring element 1602.
            // Else, we need to use a tabulated element 1902 Type 0
            if (pairDOF.first != pairDOF.second){
                systus_ascid_t tId2=0;
                if (ss->hasStiffness()){
                    systus_ascid_t tId= tables.size()+1;
                    SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
                    const double stiffness = ss->getStiffness();

                    int pairCode = 1100;
                    int dofCode = 11*DOFToInt(pairDOF.first);
                    aTable.add(pairCode+dofCode);
                    aTable.add(stiffness);
                    pairCode = 2200;
                    dofCode = 11*DOFToInt(pairDOF.second);
                    aTable.add(pairCode+dofCode);
                    aTable.add(stiffness);
                    pairCode = 1200;
                    dofCode = 10*DOFToInt(pairDOF.first) + DOFToInt(pairDOF.second);
                    aTable.add(pairCode+dofCode);
                    aTable.add(-stiffness);
                    tables.push_back(aTable);
                    tId2+=tId;
                }
                if (ss->hasDamping()){
                    systus_ascid_t tId= tables.size()+1;
                    SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
                    const double damping = ss->getDamping();

                    int pairCode = 1100;
                    int dofCode = 11*DOFToInt(pairDOF.first);
                    aTable.add(pairCode+dofCode);
                    aTable.add(damping);
                    pairCode = 2200;
                    dofCode = 11*DOFToInt(pairDOF.second);
                    aTable.add(pairCode+dofCode);
                    aTable.add(damping);
                    pairCode = 1200;
                    dofCode = 10*DOFToInt(pairDOF.first) + DOFToInt(pairDOF.second);
                    aTable.add(pairCode+dofCode);
                    aTable.add(-damping);
                    tables.push_back(aTable);
                    tId2+=tId*10000;
                }
                tableByElementSet[elementSet->getId()]=tId2;
            }
            ss->markAsWritten();
        }

        for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::STIFFNESS_MATRIX)) {

        // Stiffness, Mass and Damping matrices are the same kind.
        // Only the tableByElementSet value differs:
        //   - Stiffness: 0000XX
        //   - Mass     : 00XX00
        //   - Damping  : XX0000
            const auto& sm = static_pointer_cast<StiffnessMatrix>(elementSet);
            systus_ascid_t tId= tables.size()+1;
            SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
            aTable.fill(sm, nbDOFS);
            tables.push_back(aTable);
            tableByElementSet[elementSet->getId()]=tId;
            sm->markAsWritten();
        }

        for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::MASS_MATRIX)) {
            const auto& mm = static_pointer_cast<MassMatrix>(elementSet);
            systus_ascid_t tId= tables.size()+1;
            SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
            aTable.fill(mm,nbDOFS);
            tables.push_back(aTable);
            tableByElementSet[elementSet->getId()]=tId*100;
            mm->markAsWritten();
        }

        for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::DAMPING_MATRIX)) {
            const auto& dm = static_pointer_cast<DampingMatrix>(elementSet);
            systus_ascid_t tId= tables.size()+1;
            SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
            aTable.fill(dm, nbDOFS);
            tables.push_back(aTable);
            tableByElementSet[elementSet->getId()]=tId*10000;
            dm->markAsWritten();
        }

    }

    // Build tables for linear multipoint constraints
    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::LMPC)) {
        // If the LMPC is not relevant to the current subcase, we skip it
        const auto& lmpc = static_pointer_cast<Lmpc>(elementSet);
        const auto& analysisOfSubcase =  systusSubcases[idSubcase];
        if (std::find(analysisOfSubcase.begin(), analysisOfSubcase.end(), lmpc->analysisId) == analysisOfSubcase.end()){
            continue;
        }
        // The Table for Lmpc is simply the list of coef by dof by nodes
        systus_ascid_t tId= tables.size()+1;
        SystusTable aTable{tId, SystusTableLabel::TL_STANDARD, 0};
        for (dof_int dofnum = 0; dofnum < lmpc->getDofCount(); dofnum++){
            for (DOFCoefs dofCoefs : lmpc->dofCoefsByDof[dofnum]){
                for (dof_int i =0; i< nbDOFS; i++){
                    aTable.add(dofCoefs[i]);
                }
            }
        }
        tables.push_back(aTable);
        tableByElementSet[elementSet->getId()]=tId;
    }


    // Build tables for frequency-dependent amplitude on Modal Dynamic Analysis
    if (systusModel.configuration.systusDynamicMethod=="modal") {
        const auto& analysisId = systusSubcases[idSubcase];

        for (unsigned i = 0 ; i < analysisId.size(); i++) {
            const auto& analysis = systusModel.model.getAnalysis(analysisId[i]);

            if (analysis == nullptr){
                handleWritingWarning("Wrong analysis number: analysis dismissed", "Table");
                break;
            }

            for (const auto& loadset : analysis->getLoadSets()) {
                const int idLoadCase = localLoadingIdByLoadsetIdByAnalysisId[analysis->getId()][loadset->getId()];
                for (const auto& loading : loadset->getLoadings()) {

                    switch (loading->type){

                    // Nothing to do
                    case Loading::Type::NODAL_FORCE:
                    case Loading::Type::GRAVITY:{
                        break;
                    }

                    // Here we stock the amplitude
                    case Loading::Type::DYNAMIC_EXCITATION:{
                        const auto& dE = static_pointer_cast<DynamicExcitation>(loading);
                        const auto& dEL = dE->getLoadSet();
                        if (dEL->type != LoadSet::Type::EXCITEID){
                            handleWritingWarning("Dynamic loading must refer an EXCITED Loadset. Dismissing load "+ to_string(dE->bestId()), "Table");
                            break;
                        }

                        const auto& aTable = dE->getFunctionTableB();
                        int tId= static_cast<int>(tables.size())+1;
                        SystusTable aSystusTable(tId);
                        //TODO: Test the units of the table ?
                        for (auto it = aTable->getBeginValuesXY(); it != aTable->getEndValuesXY(); it++){
                            aSystusTable.add(it->first);
                            aSystusTable.add(it->second);
                        }
                        tables.push_back(aSystusTable);
                        tableByLoadcase[idLoadCase]= tId;

                        break;
                    }

                    default: {
                        handleWritingWarning("Unknown type of LOADING "+ to_str(*loading), "Table");
                    }
                    }
                }
            }
        }
    }
}

void SystusWriter::fillMaterial(const SystusModel& systusModel, const int idSubcase) {

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that this function relies heavily on lists built before.
    // Lists that ARE dependent on the model and current subcase.
    UNUSEDV(idSubcase);

    // Specific material for specific element set
    int idSkinMaterial=-1;

    for (const auto& elementSet : systusModel.model.elementSets) {

        if (not elementSet->effective()) {
            continue;
        }

        /* Initialize a new systus material */
        map<SMF, string> systusMat;
        ostringstream ogmat;
        int materialId=static_cast<int>(systusMaterial.size()+1);
        fillMaterialField(SMF::ID, materialId, systusMat);
        fillMaterialField(SMF::MID, materialId, systusMat);
        bool isValid=true;
        bool isNewMaterial=true;

        // Elements with VEGA material
        if (elementSet->hasMaterials()){
            const auto& nature = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_ELASTIC);
            if (nature != nullptr) {
                const auto& elasticNature = static_pointer_cast<ElasticNature>(nature);

                fillMaterialField(SMF::RHO, elasticNature->getRho(), systusMat);
                fillMaterialField(SMF::E, elasticNature->getE(), systusMat);
                fillMaterialField(SMF::NU, elasticNature->getNu(), systusMat);
                fillMaterialField(SMF::ALPHA, 2*elasticNature->getGE(), systusMat);

                switch (elementSet->type) {
                case ElementSet::Type::TUBE_SECTION_BEAM:
                case ElementSet::Type::I_SECTION_BEAM:
                case ElementSet::Type::GENERIC_SECTION_BEAM: {
                    const auto& genericBeam = static_pointer_cast<Beam>(elementSet);
                    const double S= genericBeam->getAreaCrossSection();
                    fillMaterialField(SMF::S, S, systusMat);

                    if (not genericBeam->isTruss()) {
                        // VEGA stocks the inverse of our needed Shear Area Factors.
                        fillMaterialField(SMF::AY, S*genericBeam->getShearAreaFactorY(), systusMat);
                        fillMaterialField(SMF::AZ, S*genericBeam->getShearAreaFactorZ(), systusMat);

                        fillMaterialField(SMF::IX, genericBeam->getTorsionalConstant(), systusMat);
                        fillMaterialField(SMF::IY, genericBeam->getMomentOfInertiaY(), systusMat);
                        fillMaterialField(SMF::IZ, genericBeam->getMomentOfInertiaZ(), systusMat);
                    }
                    genericBeam->markAsWritten();
                    break;
                }
                case ElementSet::Type::CIRCULAR_SECTION_BEAM: {
                    const auto& circularBeam = static_pointer_cast<CircularSectionBeam>(elementSet);
                    const double S= circularBeam->getAreaCrossSection();

                    fillMaterialField(SMF::S, S, systusMat);
                    if (not circularBeam->isTruss()) {
                        fillMaterialField(SMF::AY, S*circularBeam->getShearAreaFactorY(), systusMat);
                        fillMaterialField(SMF::AZ, S*circularBeam->getShearAreaFactorZ(), systusMat);

                        fillMaterialField(SMF::IX, circularBeam->getTorsionalConstant(), systusMat);
                        fillMaterialField(SMF::IY, circularBeam->getMomentOfInertiaY(), systusMat);
                        fillMaterialField(SMF::IZ, circularBeam->getMomentOfInertiaZ(), systusMat);
                    }
                    circularBeam->markAsWritten();
                    break;
                }
                case ElementSet::Type::RECTANGULAR_SECTION_BEAM: {
                    const auto& rectangularBeam = static_pointer_cast<RectangularSectionBeam>(elementSet);
                    const double S= rectangularBeam->getAreaCrossSection();

                    if (not rectangularBeam->isTruss()) {
                        fillMaterialField(SMF::S, S, systusMat);
                        fillMaterialField(SMF::AY, S*rectangularBeam->getShearAreaFactorY(), systusMat);
                        fillMaterialField(SMF::AZ, S*rectangularBeam->getShearAreaFactorZ(), systusMat);

                        fillMaterialField(SMF::IX, rectangularBeam->getTorsionalConstant(), systusMat);
                        fillMaterialField(SMF::IY, rectangularBeam->getMomentOfInertiaY(), systusMat);
                        fillMaterialField(SMF::IZ, rectangularBeam->getMomentOfInertiaZ(), systusMat);
                    }
                    rectangularBeam->markAsWritten();
                    break;
                }

                case ElementSet::Type::SHELL: {
                    const auto& shell = static_pointer_cast<Shell>(elementSet);
                    fillMaterialField(SMF::H, shell->thickness, systusMat);
                    if (not is_zero(shell->offset)) {
                        handleWritingWarning(to_str(*shell) +" offset non zero ignored", "Shell");
                    }
                    shell->markAsWritten();
                    break;
                }
                case ElementSet::Type::SKIN:
                case ElementSet::Type::CONTINUUM: {
                    break;
                }

                // Default : we only print the default fields: E, NU, etc.
                default:
                    handleWritingWarning(to_str(*elementSet) +" not supported", "Elastic Material");

                }
            } else {

                const auto& nature2 = elementSet->mainMaterial()->findNature(Nature::NatureType::NATURE_RIGID);
                if (nature2 != nullptr) {
                    const auto& rigidNature = static_pointer_cast<RigidNature>(nature2);

                    switch (elementSet->type) {
                    // RBAR are Systus Rigid Body Element
                    case (ElementSet::Type::RBAR):{
                        fillMaterialField(SMF::LEVEL, 1, systusMat);
                        fillMaterialField(SMF::TYPE, 9, systusMat);
                        if (systusModel.configuration.systusRBE2TranslationMode.compare("lagrangian")==0) {
                            fillMaterialField(SMF::SHAPE, 19, systusMat);
                            fillMaterialField(SMF::E, rigidNature->getLagrangian(), systusMat);
                        } else {
                            fillMaterialField(SMF::SHAPE, 9, systusMat);
                            fillMaterialField(SMF::E, rigidNature->getRigidity(), systusMat);
                        }
                        break;
                    }

                    // RBE3 are Special Elements (Averaging Type Solid Elements)
                    case (ElementSet::Type::RBE3):{
                        const auto& rbe3 = static_pointer_cast<Rbe3>(elementSet);
                        fillMaterialField(SMF::SHAPE, 19, systusMat);
                        fillMaterialField(SMF::LEVEL, 3, systusMat);
                        fillDOFSMaterialField(rbe3->mdofs, systusMat); // KX KY KZ IX IY IZ
                        fillMaterialField(SMF::COEF, rigidNature->getLagrangian(), systusMat);
                        fillMaterialField(SMF::DEPEND, DOFSToInt(rbe3->sdofs), systusMat);
                        rbe3->markAsWritten();
                        break;
                    }

                    case ElementSet::Type::LMPC:{
                        // If the LMPC is not relevant to the current subcase, we skip it
                        const auto& lmpc = static_pointer_cast<Lmpc>(elementSet);
                        const auto& analysisOfSubcase =  systusSubcases[idSubcase];
                        if (std::find(analysisOfSubcase.begin(), analysisOfSubcase.end(), lmpc->analysisId) == analysisOfSubcase.end()){
                            continue;
                        }
                        auto it = tableByElementSet.find(elementSet->getId());
                        if (it == tableByElementSet.end()){
                            handleWritingWarning(to_str(*elementSet) + " has no table.", "Rigid Material");
                            break;
                        }
                        fillMaterialField(SMF::TABLE, int(it->second), systusMat);
                        if (systusModel.configuration.systusRBE2TranslationMode.compare("lagrangian")==0) {
                            fillMaterialField(SMF::SHAPE, 19, systusMat);
                            fillMaterialField(SMF::E, rigidNature->getLagrangian(), systusMat);
                        } else {
                            fillMaterialField(SMF::SHAPE, 9, systusMat);
                            fillMaterialField(SMF::E, rigidNature->getRigidity(), systusMat);
                        }
                        fillMaterialField(SMF::LEVEL, 0, systusMat);
                        lmpc->markAsWritten();
                        break;
                    }

                    // Default: unrecognized material are written with only an ID
                    default: {
                        handleWritingWarning(to_str(*elementSet) +" not supported", "Rigid Material");
                    }
                    }
                } else {
                    isValid=false;
                    handleWritingWarning("Unrecognized nature of "+to_str(*elementSet), "Material");
                }
            }
            elementSet->mainMaterial()->markAsWritten();
        } else {
            // Element without VEGA Material
            switch (elementSet->type){
            case (ElementSet::Type::STRUCTURAL_SEGMENT):{


                const auto& sS = static_pointer_cast<StructuralSegment>(elementSet);
                // K Matrix
                fillMaterialField(SMF::IX, sS->findStiffness(DOF::RX, DOF::RX), systusMat);
                fillMaterialField(SMF::IY, sS->findStiffness(DOF::RY, DOF::RY), systusMat);
                fillMaterialField(SMF::IZ, sS->findStiffness(DOF::RZ, DOF::RZ), systusMat);
                fillMaterialField(SMF::KX, sS->findStiffness(DOF::DX, DOF::DX), systusMat);
                fillMaterialField(SMF::KY, sS->findStiffness(DOF::DY, DOF::DY), systusMat);
                fillMaterialField(SMF::KZ, sS->findStiffness(DOF::DZ, DOF::DZ), systusMat);

                // A few warnings
                if (sS->hasMass()){
                    handleWritingWarning("Mass in "+to_str(*elementSet)+" is not supported and will be dismissed.", "Material");
                }
                if (sS->hasDamping()){
                    // A Matrix
                    // LD : SMF::AX does not exists ?
                    //fillMaterialField(SMF::AX, sS->findDamping(DOF::DX, DOF::DX), systusMat);
                    //fillMaterialField(SMF::AY, sS->findDamping(DOF::DY, DOF::DY), systusMat);
                    //fillMaterialField(SMF::AZ, sS->findDamping(DOF::DZ, DOF::DZ), systusMat);
                    //handleWritingWarning("Damping in "+to_str(*elementSet)+" is not supported and will be dismissed.", "Material");
                }
                sS->markAsWritten();
                break;
            }

            case ElementSet::Type::SCALAR_SPRING:{
                const auto& ss = static_pointer_cast<ScalarSpring>(elementSet);
                const auto& dofsSpring = ss->getDOFSSpring();
                if (dofsSpring.size()!=1){
                    handleWritingWarning(to_str(*elementSet) + " spring directions after the first one will be dismissed.", "Material");
                }
                const auto& pairDOF = dofsSpring[0];
                // If we have the same DOF, we can use a Spring element 1602.
                // Else, we need to use a tabulated element 1902 Type 0
                if (pairDOF.first == pairDOF.second){
                    fillMaterialField(DOFtoSMF(pairDOF.first), ss->getStiffness(), systusMat);
                    if (ss->hasDamping()){
                        handleWritingWarning(to_str(*elementSet)+" damping is not supported.","Material");
                    }
                }else{
                    auto it = tableByElementSet.find(elementSet->getId());
                    if (it == tableByElementSet.end()){
                        handleWritingWarning(to_str(*elementSet) + " has no table.", "Material");
                        break;
                    }
                    fillMaterialField(SMF::TABLE, int(it->second), systusMat);
                    if (systusModel.configuration.systusOutputMatrix=="file"){
                        auto it2 = seIdByElementSet.find(elementSet->getId());
                        if (it2 == seIdByElementSet.end()){
                            handleWritingWarning(to_str(*elementSet) + " has no reduction number.", "Material");
                            break;
                        }
                        fillMaterialField(SMF::E, double(it2->second), systusMat);
                    }
                }
                ss->markAsWritten();
                break;
            }

            // For matrix elements, material is only a "pointer" to a table.
            case ElementSet::Type::STIFFNESS_MATRIX:
            case ElementSet::Type::MASS_MATRIX:
            case ElementSet::Type::DAMPING_MATRIX:{
                auto it = tableByElementSet.find(elementSet->getId());
                if (it == tableByElementSet.end()){
                    handleWritingWarning(to_str(*elementSet)+" has no table.","Material");
                    break;
                }
                fillMaterialField(SMF::TABLE, int(it->second), systusMat);
                if (systusModel.configuration.systusOutputMatrix=="file"){
                    auto it2 = seIdByElementSet.find(elementSet->getId());
                    if (it2 == seIdByElementSet.end()){
                        handleWritingWarning(to_str(*elementSet)+" has no reduction number.","Material");
                        break;
                    }
                    fillMaterialField(SMF::E, double(it2->second), systusMat);
                }
                break;
            }

            // Skin elements are used to support surface load, don't need material
            // We all point them to the same "dummy" material, with only the id
            case ElementSet::Type::SKIN:{
                if (idSkinMaterial==-1){
                    idSkinMaterial=materialId;
                }else{
                    materialId=idSkinMaterial;
                    isNewMaterial=false;
                }
                break;
            }

            // Nodal Masses are not material in Systus
            case (ElementSet::Type::NODAL_MASS): {
                isValid=false;
                break;
            }

            default:{
                isValid=false;
                handleWritingWarning(to_str(*elementSet)+" not supported.","Material");
            }
            }
        }

        // Adds the material
        if (isValid){
            materialIdByElementSetId[elementSet->getId()]=materialId;
            if (isNewMaterial){
                systusMaterial.push_back(systusMat);
            }
        }
    }

}

void SystusWriter::fillMatrices(const SystusModel& systusModel, const int idSubcase) {

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that only one kind of matrix  are translated yet,
    // and we don't know what the others will need.
    UNUSEDV(idSubcase);

    dampingMatrices.nbDOFS=nbDOFS;
    massMatrices.nbDOFS=nbDOFS;
    stiffnessMatrices.nbDOFS=nbDOFS;

    // Fill tables for Stiffness, Mass and Damping elements
    if (systusModel.configuration.systusOutputMatrix != "file") {
        return;
    }

    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::SCALAR_SPRING)) {

        const auto& ss = static_pointer_cast<ScalarSpring>(elementSet);
        const auto& dofsSpring = ss->getDOFSSpring();
        if (dofsSpring.size() != 1) {
            handleWritingWarning(to_str(*elementSet) + " spring directions after the first one will be dismissed.", "FillMatrices");
        }
        const auto& pairDOF = dofsSpring[0];
        // If we have the same DOF, we can use a Spring element 1602.
        // Else, we need to use a tabulated element 1902 Type 0
        if (pairDOF.first != pairDOF.second){
            systus_ascid_t tId2=0;
            if (ss->hasStiffness()){
                systus_ascid_t seId= stiffnessMatrices.size()+1;
                // Building the Systus Matrix
                SystusMatrix aMatrix{seId, nbDOFS, 2};
                int dofI = DOFToInt(pairDOF.first);
                int dofJ = DOFToInt(pairDOF.second);
                aMatrix.setValue(1, 1, dofI, dofI, ss->getStiffness());
                aMatrix.setValue(2, 2, dofJ, dofJ, ss->getStiffness());
                aMatrix.setValue(1, 2, dofI, dofJ, -ss->getStiffness());
                aMatrix.setValue(2, 1, dofJ, dofI, -ss->getStiffness());
                tId2+=SystusWriter::StiffnessAccessId;
                seIdByElementSet[elementSet->getId()]= seId;
                stiffnessMatrices.add(aMatrix);

            }
            if (ss->hasDamping()){
                systus_ascid_t seId= dampingMatrices.size()+1;
                // Building the Systus Matrix
                SystusMatrix aMatrix{seId, nbDOFS, 2};
                //for (const auto np : dam->nodePairs()){
                int dofI = DOFToInt(pairDOF.first);
                int dofJ = DOFToInt(pairDOF.second);
                aMatrix.setValue(1, 1, dofI, dofI, ss->getDamping());
                aMatrix.setValue(2, 2, dofJ, dofJ, ss->getDamping());
                aMatrix.setValue(1, 2, dofI, dofJ, -ss->getDamping());
                aMatrix.setValue(2, 1, dofJ, dofI, -ss->getDamping());
                tId2+=SystusWriter::DampingAccessId*10000;
                seIdByElementSet[elementSet->getId()]= seId;
                dampingMatrices.add(aMatrix);
            }
            tableByElementSet[elementSet->getId()]=-tId2;
        }
    }

    // Stiffness, Mass and Damping matrices are the same kind.
    // Only the tableByElementSet value differs:
    //   - Stiffness: -0000XX
    //   - Mass     : -00XX00
    //   - Damping  : -XX0000
    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::DAMPING_MATRIX)) {
        const auto& dam = static_pointer_cast<DampingMatrix>(elementSet);
        systus_ascid_t seId= dampingMatrices.size()+1;

        // Numbering the node internally to the element
        map<pos_t, int> positionToSytusNumber;
        int iSystus = 1;
        for (const auto pos : dam->nodePositions()){
            positionToSytusNumber[pos] = iSystus;
            iSystus++;
        }

        // Building the Systus Matrix
        SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
        for (const auto np : dam->nodePairs()){
            int nI = positionToSytusNumber[np.first];
            int nJ = positionToSytusNumber[np.second];
            const auto& dM = dam->findSubmatrix(np.first, np.second);
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
        dam->markAsWritten();
    }


    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::MASS_MATRIX)) {
        const auto& mm = static_pointer_cast<MassMatrix>(elementSet);
        systus_ascid_t seId= massMatrices.size()+1;

        // Numbering the node internally to the element
        map<pos_t, int> positionToSytusNumber;
        int iSystus= 1;
        for (const auto pos : mm->nodePositions()) {
            positionToSytusNumber[pos] = iSystus;
            iSystus++;
        }

        // Building the Systus Matrix
        SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
        for (const auto np : mm->nodePairs()){
            int nI = positionToSytusNumber[np.first];
            int nJ = positionToSytusNumber[np.second];
            const auto& dM = mm->findSubmatrix(np.first, np.second);
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
        mm->markAsWritten();
    }

    for (const auto& elementSet : systusModel.model.elementSets.filter(ElementSet::Type::STIFFNESS_MATRIX)) {
        const auto& sm = static_pointer_cast<StiffnessMatrix>(elementSet);
        systus_ascid_t seId= stiffnessMatrices.size()+1;

        // Numbering the node internally to the element
        map<pos_t, int> positionToSytusNumber;
        int iSystus= 1;
        for (const auto pos : sm->nodePositions()){
            positionToSytusNumber[pos] = iSystus;
            iSystus++;
        }

        // Building the Systus Matrix
        SystusMatrix aMatrix = SystusMatrix(seId, nbDOFS, iSystus-1);
        for (const auto np : sm->nodePairs()){
            int nI = positionToSytusNumber[np.first];
            int nJ = positionToSytusNumber[np.second];
            const auto& dM = sm->findSubmatrix(np.first, np.second);
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
        sm->markAsWritten();
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
    loadingVectorsIdByLocalLoadingByCellId.clear();
    constraintVectorsIdByLocalLoadingByNodePosition.clear();

    // Clear lists
    lists.clear();
    loadingListIdByNodePosition.clear();
    loadingListIdByCellId.clear();
    constraintListIdByNodePosition.clear();

    // Clear tables
    tables.clear();
    tableByElementSet.clear();
    tableByLoadcase.clear();

    // Clear matrices
    seIdByElementSet.clear();
    dampingMatrices.clear();
    massMatrices.clear();
    stiffnessMatrices.clear();

    // Clear material
    systusMaterial.clear();
}

void SystusWriter::translate(const SystusModel &systusModel, const int idSubcase){

    this->clear();

    fillMatrices(systusModel, idSubcase);

    fillLoads(systusModel, idSubcase);

    fillConstraintsNodes(systusModel, idSubcase);

    fillVectors(systusModel, idSubcase);

    fillLists(systusModel, idSubcase);

    fillTables(systusModel, idSubcase);

    fillMaterial(systusModel, idSubcase);
}



void SystusWriter::writeAsc(const SystusModel &systusModel, const int idSubcase, ostream& out) {

    writeHeader(systusModel, out);

    writeInformations(systusModel, idSubcase, out);

    writeNodes(systusModel, out);

    writeElements(systusModel, idSubcase, out);

    writeGroups(systusModel, out);

    writeMaterial(systusModel, idSubcase, out);

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
    out << " 100000 " << systusOption << " " << systusModel.model.mesh.countNodes() << " ";
    out << systusModel.model.mesh.countCells() << " ";
    int kppr = static_cast<int>(localLoadingListName.size()) ; // KPPR: Number of loads
    out << kppr << " ";

    out << nbDOFS << " " ;                     // KP: Number of degrees of freedom per node
    out << 2*nbDOFS*std::max(1, kppr) << " " ; // KPC = 2*KP*max(1,KPPR) (for most cases)
    out << "0 0" << endl;                      // Two useless integers

}

void SystusWriter::writeInformations(const SystusModel &systusModel, int idSubcase , ostream& out) {
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
    auto sizeleft =  80 - ssubcase.length() - sdate.length() - slogiciel.length();
    out << slogiciel << systusModel.configuration.inputFile.substr(0, sizeleft) << ssubcase << sdate<< endl;

    // NCODES
    int ncode[20]={0};
    ncode[0]= static_cast<int>(systusOption); //IOPT: SYSTUS OPTION
    ncode[3]= 2;  // NORM: Eigenvelue norm used (1: Maximum (def), 2: Mass (Nastran  default), 4: Elastic)
    ncode[11]= 1; // KMAT : New material structure. Always set to 1.
    ncode[13]= static_cast<int>(systusSubOption); // MELANG : Mixed Options
    for (int i = 0; i<20 ; i++){
      out <<" "<< ncode[i];
    }
    out <<endl;

    // LCODES : Most of these are not really needed, as Systus recomputes them after.
    // Nonetheless, it's cleaner this way.
    int lcode[40]={0};
    lcode[0] = static_cast<int>(localLoadingListName.size()); // KPPR: Number of loads
    lcode[1] = static_cast<int>(systusModel.model.mesh.countNodes());     // NMAX: Number of nodes
    lcode[3] = static_cast<int>(systusModel.model.mesh.countCells());     // MMAXI: Number of elements
    lcode[5] = 0;                                         // JMAT: Number of material couples, will be computed in "nbmaterials" in the writeMaterials method
    lcode[6] = static_cast<int>(lists.size());            // JREP: Number of lists
    lcode[7] = static_cast<int>(vectors.size());          // JVEC: Number of vectors.
    lcode[10]= nbDOFS;                                    // KP: Number of dof per node
    lcode[11]= nbDOFS;                                    // KPMAX: Maximum Number of dof per node
    lcode[12]= nbDOFS*nbDOFS;                             // KPM2 = KPMAX*KPMAX;
    lcode[13]= 3;                                         // NCOOR: Number of coordinates of a node (2 or 3)
    lcode[17]= nbDOFS*nbDOFS;                             // KP2 = KP*KP;
    lcode[22]= 2*nbDOFS*std::max(lcode[0],1);             // KPC = 2*KP*max(1,KPPR) (for most cases)
    lcode[36]= 2;                                         // LCRP Data Type: 1:OLD, 2: NEW
    lcode[37]= 0;                                         // ICAL Computation Status 0: Not Done
    for (int i = 0; i<40 ; i++){
      out <<" "<< lcode[i];
    }
    out <<endl;

    out << "END_INFORMATIONS" << endl;
}

void SystusWriter::writeNodes(const SystusModel& systusModel, ostream& out) {
    Mesh& mesh = systusModel.model.mesh;

    out << "BEGIN_NODES ";
    out << mesh.countNodes();
    out << " 3" << endl; // number of coordinates

    for (const auto& node : mesh.nodes) {
        int nid = node.id;
        int iconst = 0;
        auto it = constraintByNodePosition.find(node.position);
        if (it != constraintByNodePosition.end())
            iconst = int(it->second);
        int imeca = 0;
        systus_ascid_t iangl = 0;
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
        out << node.x << " " << node.y << " " << node.z << endl;

        // Small warning against "infinite" node.
        if (node.x < -1.0e+300){
            handleWritingWarning("Infinite node with Id: " + std::to_string(nid),"Nodes");
        }
    }

    out << "END_NODES" << endl;
}


void SystusWriter::writeElementLocalReferentiel(const SystusModel& systusModel,
        const int dim, const int celltype, const vector<int> nodes, const pos_t cpos, ostream& out){

    const auto& cs = systusModel.model.mesh.getCoordinateSystemByPosition(cpos);
    if (cs == nullptr){
        out << " 0";
        handleWritingWarning("Unknown coordinate system of position " +to_string(cpos), "Angle Elements");
        return;
    }

    if ((cs->type!=CoordinateSystem::Type::RELATIVE and cs->coordType!=CoordinateSystem::CoordinateType::VECTOR) and
        (cs->type!=CoordinateSystem::Type::ABSOLUTE and cs->coordType!=CoordinateSystem::CoordinateType::CARTESIAN)){
        out << " 0";
        handleWritingWarning("Coordinate System "+ to_string(static_cast<int>(cs->type)) + " is not supported. Referentiel dismissed.", "Angle Elements");
        return;
    }

    CartesianCoordinateSystem rcs = buildElementDefaultReferentiel(systusModel, nodes, dim, celltype);
    VectorialValue angles = cs->getEulerAnglesIntrinsicZYX(&rcs); // (PSI, THETA, PHI)

    switch (dim) {

    // Orientation are not allowed for 0d elements.
    case 0: {
        if (cs->type==CoordinateSystem::Type::RELATIVE){
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




void SystusWriter::writeElements(const SystusModel& systusModel, const int idSubcase, ostream& out) {
    const Mesh& mesh = systusModel.model.mesh;
    out << "BEGIN_ELEMENTS " << mesh.countCells() << endl;
    for (const auto& elementSet : systusModel.model.elementSets) {

        if (systusModel.model.configuration.logLevel >= LogLevel::TRACE){
            cout << "Writing elementSet " << *elementSet << endl;
        }

        const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
        if (cellElementSet != nullptr) {
            for (auto& cellGroup : cellElementSet->getCellGroups()) {
                cellGroup->isUseful=false;
            }
        }

        int dim = 0;
        int typecell=0;


        switch (elementSet->type) {
        case ElementSet::Type::CIRCULAR_SECTION_BEAM:
        case ElementSet::Type::GENERIC_SECTION_BEAM:
        case ElementSet::Type::I_SECTION_BEAM:
        case ElementSet::Type::RECTANGULAR_SECTION_BEAM: {
            dim = 1;
            break;
        }
        case ElementSet::Type::STRUCTURAL_SEGMENT:{
            typecell=6;
            // We treat only Stiffness matrixes for now, with 1602 and 0601 Elements
            const auto& se = static_pointer_cast<StructuralSegment>(elementSet);
            if ((se->hasMass()) or (se->hasDamping())){
                handleWritingWarning(to_str(*elementSet) + " mass and damping are not supported and will be dismissed.", "Elements");
            }
            break;
        }

        case ElementSet::Type::SCALAR_SPRING:{
            dim = 1;
            const auto& ss = static_pointer_cast<ScalarSpring>(elementSet);
            const auto& dofsSpring = ss->getDOFSSpring();
            if (dofsSpring.size()!=1){
                handleWritingWarning(to_str(*elementSet) + " spring directions after the first one will be dismissed.", "Elements");
            }
            const auto& pairDOF = dofsSpring[0];
            // If we have the same DOF, we can use a Spring element 1602.
            // Else, we need to use a tabulated element 1902 Type 0
            if (pairDOF.first == pairDOF.second){
                typecell=6;
            }else{
                typecell=9;
            }
            break;
        }
        case ElementSet::Type::SKIN: {
            dim = 2;
            elementSet->markAsWritten();
            break;
        }
        case ElementSet::Type::SHELL: {
            dim = 2;
            typecell=4;
            break;
        }
        case ElementSet::Type::SURFACE_SLIDE_CONTACT:
        case ElementSet::Type::CONTINUUM: {
            dim = 3;
            elementSet->markAsWritten();
            break;
        }
        case ElementSet::Type::NODAL_MASS:{
            continue;
        }
        case ElementSet::Type::DISCRETE_0D:
        case ElementSet::Type::DISCRETE_1D:{
            continue;
        }
        case ElementSet::Type::STIFFNESS_MATRIX:
        case ElementSet::Type::MASS_MATRIX:
        case ElementSet::Type::DAMPING_MATRIX:{
            dim= 3;
            typecell = 9;
            break;
        }
        case ElementSet::Type::RBAR:
        case ElementSet::Type::RBE3:{
            dim = 1;
            typecell = 9;
            break;
        }

        case ElementSet::Type::LMPC:{
            // If the LMPC is not relevant to the current subcase, we skip it
            const auto& lmpc = static_pointer_cast<Lmpc>(elementSet);
            const auto& analysisOfSubcase =  systusSubcases[idSubcase];
            if (std::find(analysisOfSubcase.begin(), analysisOfSubcase.end(), lmpc->analysisId) == analysisOfSubcase.end()){
                continue;
            }
            // Else, it'a a 19XX element
            dim = 1;
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

        if (cellElementSet != nullptr) {
            for (auto& cellGroup : cellElementSet->getCellGroups()) {
                cellGroup->isUseful=true;
            }
        }
        for (const pos_t cellPosition : elementSet->cellPositions()) {
            const Cell& cell = mesh.findCell(cellPosition);

            if (systusModel.model.configuration.logLevel >= LogLevel::TRACE){
                cout << "Writing cell " << cell << " from " << elementSet->name << endl;
            }
            auto systus2med_it = systus2medNodeConnectByCellType.find(cell.type.code);
            if (systus2med_it == systus2medNodeConnectByCellType.end()) {
                cout << "Warning in Elements: " << cell << " not supported in Systus" << endl;
                continue;
            }

            // Putting all nodes in the Systus order
            const auto& systus2medNodeConnect = systus2med_it->second;
            const auto& medConnect = cell.nodeIds;
            vector<int> systusConnect;
            for (unsigned int medId : systus2medNodeConnect)
                systusConnect.push_back(medConnect[medId]);

            if (elementSet->type==ElementSet::Type::STRUCTURAL_SEGMENT){
                dim = (cell.nodeIds.size()==2) ? 1 : 0 ;
            }

            out << cell.id << " " << dim << typecell;              // Dimension and type of cell;
            out << setfill('0') << setw(2) << cell.nodeIds.size(); // Number of nodes in two caracters: 01, 02, 05, 10, etc.

            if (cell.nodeIds.size()>20){
                cerr<< "Warning in Elements: " << cell << " has " << cell.nodeIds.size() << " but SYSTUS only support up to 20 nodes by element."<<endl;
            }

            out << " " << materialIdByElementSetId[elementSet->getId()]; // Material Id

            // Loading List: index that describes sollicitation list (not supported yet)
            int isol = 0;
            auto it2 = loadingListIdByCellId.find(cell.id);
            if (it2 != loadingListIdByCellId.end())
                isol = it2->second;
            out << " " << isol;

            // Local Orientation
            if (cell.hasOrientation){
                writeElementLocalReferentiel(systusModel, dim, typecell, systusConnect, cell.cspos, out);
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

    out << "END_ELEMENTS" << endl;
}

// TODO: Add an option to only write the User groups, and not all vega-created groups.
void SystusWriter::writeGroups(const SystusModel& systusModel, ostream& out) {
    const auto& nodeGroups = systusModel.model.mesh.getNodeGroups();
    const auto& cellGroups = systusModel.model.mesh.getCellGroups();

    ostringstream osgr;
    int nbGroups=0;

    /* Write useful CellGroups, those corresponding to real cells and to this analysis
     * Useless groups :
     *  - NodalMass groups, as they are not cells in Systus
     *  - Orientation groups, as they are not parts.
     */
    set<int> pids={};
    for (const auto& cellGroup : cellGroups) {
        if (not cellGroup->isUseful)
            continue;
        if (cellGroup->empty())
            continue;
        nbGroups++;
        string sGroupName= cellGroup->getName();
        replace(sGroupName.begin(), sGroupName.end(), ' ', '_');
        osgr << nbGroups << " " << sGroupName << " 2 0 ";
        osgr << "\"PART_ID "<< getPartId(cellGroup->getName(), pids) << "\"  \"\"  ";
        osgr << "\"PART built in VEGA from "<< cellGroup->getComment() << "\"";
        for (const auto& cell : cellGroup->getCells())
            osgr << " " << cell.id;
        osgr << endl;
    }

    // Write NodeGroups
    for (const auto& nodeGroup : nodeGroups) {
        if (nodeGroup->empty())
            continue;
        nbGroups++;
        string sGroupName= nodeGroup->getName();
        replace(sGroupName.begin(), sGroupName.end(), ' ', '_');
        osgr << nbGroups << " " << sGroupName << " 1 0 ";
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


void SystusWriter::writeMaterial(const SystusModel& systusModel,
        const int idSubcase, ostream& out) {

    // Suppressing warnings. Technically, we don't need these variables. We
    // keep them to remember that only evrything in here is heavily dependent
    // of the model and subcase.
    UNUSEDV(systusModel);
    UNUSEDV(idSubcase);

    systus_ascid_t nbMaterials= systusMaterial.size();
    int nbElements = 0;

    ostringstream ogmat;
    ogmat.precision(DBL_DIG);
    for (auto mat : systusMaterial) {
        //ID is the very beginning of the line. Its format is "N 0"
        ogmat << mat[SMF::ID] <<" 0";
        mat.erase(SMF::ID);

        // Other fields are in the format : space-key-space-value
        for (const auto & field : mat){
            ogmat << " " << static_cast<int>(field.first) << " "<<field.second;
            nbElements++;
        }
        ogmat << endl;
        }


    // Stream to output
    out << "BEGIN_MATERIALS " << nbMaterials << " " << nbElements << endl;
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
    systus_ascid_t nbElements=0;
    for (const auto& list : lists) {
        olist << list.first;
        for (const auto d : list.second)
            olist << " " << d;
        olist << endl;
        nbElements = nbElements + list.second.size()/2;
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
    const auto& masses = systusModel.model.elementSets.filter(
            ElementSet::Type::NODAL_MASS);
    out << "BEGIN_MASSES " << masses.size() << endl;
    if (not masses.empty()) {
        for (const auto& mass : masses) {
            if (not mass->effective()) {
                continue;
            }
            const auto& nodalMass = static_pointer_cast<NodalMass>(mass);
            //  VALUES NBR VAL(NBR) NODEi
            //  NBR:            Number of values [INTEGER]
            //  VAL(NBR):   Masses values [DOUBLE[NBR]]
            //  NODEi:      List of NODES index [INTEGER[*]]
            if (!is_zero(nodalMass->ixy) || !is_zero(nodalMass->iyz) || !is_zero(nodalMass->ixz)){
                handleWritingError("Asymetric masses are not (yet) implemented.");
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
            for (const auto& cell : nodalMass->getCellsIncludingGroups()) {
                // NODEi
                out << " " << cell.nodeIds[0];
            }
            out << endl;
            nodalMass->markAsWritten();
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

    // For TOPAZE, we comment a few lines.
    string comment="";
    if (configuration.systusOutputProduct=="topaze"){
        comment="###TOPAZE###";
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
        bool isFirst=true;
        for (const auto& it : filebyAccessId){
            if (isFirst){
                out << "# ACCESS TO ELEMENTARY MATRIX FILES" << endl;
                isFirst=false;
            }
            out <<  "!filematrix ASC2BIN "<< it.second <<".ASC "<< it.second <<".TIT"<<endl;
            out << "ASSIGN "<< it.first << " "<< it.second <<".TIT BINARY"<<endl;
        }
        if (!isFirst){
            out << endl;
        }
    }


    // We find the first Analysis of the Subcase, which will be our reference
    const int idFirstAnalysis = systusSubcases[idSubcase][0];
    const auto& firstAnalysis = systusModel.model.getAnalysis(idFirstAnalysis);
    if (firstAnalysis== nullptr){
        handleWritingError("Analysis " + to_string(idFirstAnalysis) + " not found.");
    }

    string sSolver="";
    string sReload="";
    string sClean= "";

    switch (firstAnalysis->type) {
    case Analysis::Type::LINEAR_MECA_STAT: {

        out << "SOLVE METHOD OPTIMISED" << endl;
        sSolver = "RESU";
        firstAnalysis->markAsWritten();
        break;
    }
    case Analysis::Type::LINEAR_MODAL:{
        const auto& linearModal = static_pointer_cast<LinearModal>(firstAnalysis);
        writeLinearModalAnalysis(out, systusModel, linearModal);
        sSolver = "RESU";
        linearModal->markAsWritten();
        break;
    }

    // Modal Dynamic Analysis
    case Analysis::Type::LINEAR_DYNA_MODAL_FREQ: {
        sSolver = "TRAN";
        const auto& linearDynaModalFreq = static_pointer_cast<LinearDynaModalFreq>(firstAnalysis);

        if (systusModel.configuration.systusDynamicMethod=="direct") {
            handleWritingWarning("Requested direct solver, but the original study is modal.", "Analysis file");
            writeDynaDirectAnalysis(out, linearDynaModalFreq);
        } else if (systusModel.configuration.systusDynamicMethod=="auto" or systusModel.configuration.systusDynamicMethod=="modal") {
            int iStaticData = writeDynaModalAnalysis(out, systusModel, linearDynaModalFreq);

            // For a TOPAZE DAT file, we need to reload results
            if (systusModel.configuration.systusOutputProduct=="topaze"){
                sReload = "SEARCH DATA "+ std::to_string(iStaticData);
            }

            // Cleaning the temporary file
            sClean = "DELETE DATA RESU "+ std::to_string(iStaticData);

        } else {
            handleWritingError("Unknown type of Dynamic Analysis "+systusModel.configuration.systusDynamicMethod, "Analysis file");
        }
        linearDynaModalFreq->markAsWritten();
        break;
    }
    case Analysis::Type::COMBINATION: {
        string message = "Ignoring COMBINATION Analysis : " + to_str(firstAnalysis) + "(for now, to be implemented).";
        out << "#" << message << endl;
        handleWritingWarning(message);
        firstAnalysis->markAsWritten();
        return; // Do not try to post process for now to avoid errors
    }
    default:
        handleWritingError(
                "Analysis " + Analysis::stringByType.at(firstAnalysis->type) + " not (yet) implemented");
    }

    // We Save Results
    out << endl;
    out << "# SAVING RESULT" << endl;
    out << comment<<"SAVE DATA 1" << endl;
    out << endl;

    // We Post-Treat Results
    out << endl;
    out << "# CONVERSION OF RESULTS FOR POST-PROCESSING" << endl;
    out << comment<<"CONVERT "<< sSolver << endl;
    out << comment<<"POST 1" << endl;
    out << comment<<"FILE DISPLACEMENT" << endl;
    out << comment<<"RETURN" << endl;
    out << endl;

    // In some case, we need to reload previous data
    if (sReload!=""){
        out << "# RELOAD DATA"<<endl;
        out << sReload <<endl;
        out << endl;
    }

    // If needed, we ask SYSUS to clean temporary files.
    if (sClean!=""){
        out << "# CLEANING FILES"<<endl;
        out << sClean <<endl;
        out << endl;
    }

    for (const int idAnalysis : systusSubcases[idSubcase]) { // LD 20191218 handing case of multiple analyses regrouped under the same subcase
        const auto& analysis = systusModel.model.getAnalysis(idAnalysis);
        if (analysis== nullptr){
            handleWritingError("Analysis " + to_string(idAnalysis) + " not found.");
        }
        const auto& assertions = analysis->getAssertions();
        if (not assertions.empty()) {
            out << "LANGAGE" << endl;
            out << "variable displacement[" << nbDOFS << "],"
                    "frequency, phase[" << nbDOFS << "];" << endl;
            out << "iResu=open_file(\"" << systusModel.getName() << "_" << analysis->getId()
                                            << ".RESU\", \"write\");" << endl << endl;

            for (const auto& assertion : assertions) {
                switch (assertion->type) {
                case Objective::Type::NODAL_DISPLACEMENT_ASSERTION: {
                    writeNodalDisplacementAssertion(*assertion, out);
                    assertion->markAsWritten();
                    break;
                }
                case Objective::Type::FREQUENCY_ASSERTION: {
                    if (analysis->type == Analysis::Type::LINEAR_DYNA_MODAL_FREQ) {
                        handleWritingWarning("Ignoring frequency assertion for analysis of type linear dyna modal");
                        assertion->markAsWritten();
                        break;
                    }
                    writeFrequencyAssertion(*assertion, out);
                    assertion->markAsWritten();
                    break;
                }
                case Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION: {
                    writeNodalComplexDisplacementAssertion(*assertion, out);
                    assertion->markAsWritten();
                    break;
                }
                case Objective::Type::NODAL_CELL_VONMISES_ASSERTION: {
                    handleWritingWarning("Von Mises assertion check not yet implemented, ignoring");
                    assertion->markAsWritten();
                    break;
                }
                default:
                    handleWritingError("Not implemented");
                }
                out << endl;
            }

            out << "close_file(iResu)" << endl;
            out << "end;" << endl;
        }

        for (const auto& objective : analysis->getObjectives()) {
            if (objective->isOutput()) {
                handleWritingWarning("Output request " + to_str(objective) + " not yet implemented, ignoring");
                objective->markAsWritten();
            }
        }
    }
}





void SystusWriter::writeNodalDisplacementAssertion(Assertion& assertion, ostream& out) {
    NodalDisplacementAssertion& nda = dynamic_cast<NodalDisplacementAssertion&>(assertion);
//    if (systusOption == SystusOption::CONTINUOUS and nda.dof.isRotation) {
//        return;
//    }
    if (!is_equal(nda.instant, -1))
        handleWritingError("Instant in NodalDisplacementAssertion not supported");
    int nodeId = nda.nodeId;
    int dofPos = nda.dof.position + 1;

    out << scientific;
    out << "displacement = node_displacement(1" << "," << nodeId << ");" << endl;
    out << "diff = abs((displacement[" << dofPos << "]-(" << nda.value << "))/("
            << (abs(nda.value) >= 1e-9 ? nda.value : 1.) << "));" << endl;

    out << "fprintf(iResu,\" ------------------------ TEST_RESU DISPLACEMENT ASSERTION ------------------------\\n\")"
            << endl;
    out
    << "fprintf(iResu,\"      NOEUD        NUM_CMP      VALE_REFE             VALE_CALC    ERREUR       TOLE\\n\");"
    << endl;
    out << "if (diff > abs(" << nda.tolerance
            << ")) fprintf(iResu,\" NOOK \"); else fprintf(iResu,\" OK   \");" << endl;
    out << "fprintf(iResu,\"" << setw(8) << nodeId << "     " << setw(8) << dofPos << "     "
            << nda.value
            << " %e %e " << nda.tolerance << " \\n\\n\", displacement[" << dofPos << "], diff);"
            << endl;
    out.unsetf(ios::scientific);
}

void SystusWriter::writeNodalComplexDisplacementAssertion(Assertion& assertion, ostream& out) {
    NodalComplexDisplacementAssertion& ncda = dynamic_cast<NodalComplexDisplacementAssertion&>(assertion);

    int nodeId = ncda.nodeId;
    int dofPos = ncda.dof.position + 1;
    double puls = ncda.frequency*2*M_PI;
    out << scientific;
    out << "nb_map = number_of_tran_maps(1);" << endl;
    out << "nume_ordre = 1;" << endl;
    out << "puls = time_map(nume_ordre);" << endl;
    out << "while (nume_ordre<nb_map-1 && abs(puls - " << puls << ")/" << max(puls, 1.) << "> 1e-5){nume_ordre=nume_ordre+1; puls = time_map(nume_ordre);}" << endl;
    out << "displacement = trans_node_displacement(nume_ordre," << nodeId << ");" << endl;
    out << "phase = trans_node_displacement(nume_ordre+1," << nodeId << ");" << endl;
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
    out << "fprintf(iResu,\"" << setw(8) << nodeId << "     " << setw(8) << dofPos << "     " << ncda.frequency << " "
            << ncda.value << " (%e,%e) %e " << ncda.tolerance << " \\n\\n\", displacement_real, displacement_imag, diff);"
            << endl;
    out.unsetf(ios::scientific);
}

void SystusWriter::writeFrequencyAssertion(Assertion& assertion, ostream& out) {
    FrequencyAssertion& frequencyAssertion = dynamic_cast<FrequencyAssertion&>(assertion);

    out << scientific;
    out << "frequency = frequency_number(" << frequencyAssertion.number << ");" << endl;
    out << "diff = abs((frequency-(" << frequencyAssertion.cycles << "))/("
            << (abs(frequencyAssertion.cycles) >= 1e-9 ? frequencyAssertion.cycles : 1.) << "));" << endl;

    out << "fprintf(iResu,\" ------------------------ TEST_RESU FREQUENCY ASSERTION ------------------------\\n\")"
            << endl;
    out << "fprintf(iResu,\"      FREQUENCY    VALE_REFE             VALE_CALC    ERREUR       TOLE\\n\");"
            << endl;
    out << "if (diff > abs(" << frequencyAssertion.tolerance
            << ")) fprintf(iResu,\" NOOK \"); else fprintf(iResu,\" OK   \");" << endl;
    out << "fprintf(iResu,\"" << setw(8) << frequencyAssertion.number << "     " << frequencyAssertion.cycles
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
            throw ios::failure("Can't open file " + matrixFile + " for writing.");
        }
        ofsMatrixFile << dampingMatrices<<endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::DampingAccessId]= systusModel.getName()+"_SC" + to_string(idSubcase+1) + "_DAMGEN";
    }

    /* Writing Mass Matrices */
    if (massMatrices.size()>0){
        ofstream ofsMatrixFile;
        ofsMatrixFile.precision(DBL_DIG);
        string matrixFile = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + "_MASGEN.ASC");
        ofsMatrixFile.open(matrixFile.c_str(), ios::trunc);

        if (!ofsMatrixFile.is_open()) {
            throw ios::failure("Can't open file " + matrixFile + " for writing.");
        }
        ofsMatrixFile << massMatrices << endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::MassAccessId]= systusModel.getName()+"_SC" + to_string(idSubcase+1) + "_MASGEN";
    }

    /* Writing Stiffness Matrices */
    if (stiffnessMatrices.size()>0){
        ofstream ofsMatrixFile;
        ofsMatrixFile.precision(DBL_DIG);
        string matrixFile = systusModel.getOutputFileName("_SC" + to_string(idSubcase+1) + "_STIGEN.ASC");
        ofsMatrixFile.open(matrixFile.c_str(), ios::trunc);

        if (!ofsMatrixFile.is_open()) {
            throw ios::failure("Can't open file " + matrixFile + " for writing.");
        }
        ofsMatrixFile << stiffnessMatrices <<endl;
        ofsMatrixFile.close();
        filebyAccessId[SystusWriter::StiffnessAccessId]=systusModel.getName()+"_SC" + to_string(idSubcase+1) + "_STIGEN";
    }
}

} //namespace systus
} //namespace vega
