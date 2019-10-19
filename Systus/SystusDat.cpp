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
 * SystusDat.cpp
 *
 *  Created on: Oct 17, 2019
 *      Author: Luca Dall'Olio
 */


#include "SystusWriter.h"


namespace vega {
namespace systus {

using namespace std;

void SystusWriter::writeFrequencyExcit(std::ostream& out, const std::shared_ptr<FrequencyExcit>& frequencyExcit) {
    out << "FREQUENCY ";
    const auto& freqValue = frequencyExcit->getValue();
    switch (freqValue->type) {
        case Value::Type::STEP_RANGE: {
            const StepRange& freqValueSteps = dynamic_cast<StepRange&>(*freqValue);
            out << "INITIAL "  << (freqValueSteps.start - freqValueSteps.step) << endl;
            out << " " << freqValueSteps.end << " STEP " << freqValueSteps.step;
            break;
        }
        default:{
            handleWritingWarning("Frequency range of type "+ to_string(static_cast<int>(freqValue->type))+" not available yet.", "Analysis file");
        }
    }
    out << endl;
}

void SystusWriter::writeModalDamping(std::ostream& out, const shared_ptr<ModalDamping>& modalDamping) {
    out << "DAMPING ";
    shared_ptr<FunctionTable> modalDampingTable = modalDamping->getFunctionTable();
    if ((modalDampingTable->getParaX()!=Function::ParaName::FREQ) || (modalDampingTable->getParaY()!=Function::ParaName::AMOR)) {
        handleWritingWarning("Dismissing damping table with wrong units ("+to_string(static_cast<int>(modalDampingTable->getParaY()))+"/"+to_string(static_cast<int>(modalDampingTable->getParaX()))+")", "Analysis file");
    } else {
        double firstDamping = modalDampingTable->getBeginValuesXY()->second;
        out << "GAMMA "<< firstDamping;
        // Systus 2017 allows to define damping for each modes, and not for frequency range. So we can only translate
        // constant dampings !
        for (auto it = modalDampingTable->getBeginValuesXY(); it != modalDampingTable->getEndValuesXY(); it++){
            if (!is_equal(firstDamping, it->second)){
                handleWritingWarning("SYSTUS modal damping must be defined by mode number and not by frequency. Constant damping is assumed.", "Analysis file");
            }
        }
    }
    out << endl;
}

int SystusWriter::writeLinearModalAnalysis(ostream& out, const SystusModel& systusModel, const shared_ptr<LinearModal>& linearModal) {

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

    // Parameters of the analysis
    const auto& frequencySearch = linearModal->getFrequencySearch();
    double upperF;
    double lowerF;
    int nmodes;
    switch(frequencySearch->frequencyType) {
    case FrequencySearch::FrequencyType::BAND: {
      const auto& band = dynamic_pointer_cast<BandRange>(frequencySearch->getValue());
      upperF = band->end;
      lowerF = band->start;
      if (is_equal(lowerF, Globals::UNAVAILABLE_DOUBLE)) {
        auto lower_cutoff_frequency = systusModel.model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY);
        if (lower_cutoff_frequency != systusModel.model.parameters.end()) {
            if (systusModel.model.configuration.logLevel >= LogLevel::TRACE) {
                cout << "Parameter LOWER_CUTOFF_FREQUENCY present, redefining frequency band" << endl;
            }
            lowerF = lower_cutoff_frequency->second;
        }
      }
      nmodes = (band->maxsearch == vega::Globals::UNAVAILABLE_INT ? defaultNbDesiredRoots : band->maxsearch);
      break;
    }
    default:
      handleWritingError(
        "Frequency search " + to_string(static_cast<int>(frequencySearch->frequencyType)) + " not (yet) implemented");
    }

    // We can't treat the case where only lowerF is defined
    if (is_equal(upperF, vega::Globals::UNAVAILABLE_DOUBLE)){
        if (!is_equal(lowerF,vega::Globals::UNAVAILABLE_DOUBLE)){
            lowerF = vega::Globals::UNAVAILABLE_DOUBLE;
            handleWritingWarning("Modal analysis with lower bound frequency not supported. Will search for "+to_string(nmodes)+" Eigenmodes instead.", "DAT file");
        }
    }else{
        // If upperF is defined, an unavailable lower bound is treated as 0;
        if (is_equal(lowerF,vega::Globals::UNAVAILABLE_DOUBLE)){
            lowerF = 0;
        }
    }
    // Number of iteration
    int niter = 2*nmodes + 2;

    // Corresponding Systus commands
    string sShift="";
    string sSturm="";
    string sBand="";
    if (!is_equal(upperF, vega::Globals::UNAVAILABLE_DOUBLE)){
        // This method is only available since the 2017 version of Systus
        if (systusModel.getSystusVersion()<2017){
            handleWritingWarning("Modal analysis with upper bound frequency not supported for version under 2017. Will search for "+to_string(nmodes)+" Eigenmodes instead.", "DAT file");
            lowerF=vega::Globals::UNAVAILABLE_DOUBLE;
            upperF=vega::Globals::UNAVAILABLE_DOUBLE;
        }else{
            sShift = " FREQUENCY "+ to_string(0.5*(lowerF+upperF));
            sSturm = " STURM FREQUENCY "+ to_string(0.5*(upperF-lowerF));
            sBand  = " BETWEEN "+to_string(lowerF)+" AND "+to_string(upperF);
        }
    }
    if ((is_equal(lowerF, vega::Globals::UNAVAILABLE_DOUBLE)) && (is_equal(upperF, vega::Globals::UNAVAILABLE_DOUBLE))){
        sShift =" SHIFT";
        sSturm="";
        sBand="";
    }

    // Choice of norm
    string sNorm;
    switch (frequencySearch->norm) {
      case(FrequencySearch::NormType::MASS): {
        sNorm=" NORM MASS";
        break;
      }
      case(FrequencySearch::NormType::MAX): {
        sNorm="";
        break;
      }
      default: {
        handleWritingWarning("Requested normalisation not yet implemented, assuming MASS.", "DAT file");
        sNorm=" NORM MASS";
      }
    }

    // Writing the DAT file
    out << "# WE COMPUTE "<< nmodes << " MODES" << sBand <<"." << endl;
    out << "# IT'S AN ITERATIVE MEHOD, WITH A MAXIMUM OF "<< niter <<" ITERATIONS" << endl;
    out << "MODE SUBSPACE BLOCK 3" << sShift<<endl;
    out << "VECTOR "<< nmodes << sSturm <<" ITER "<< niter <<" PRECISION 1*-6 FORCE"<< sNorm<<endl;
    out << "METHOD OPTIMIZED" << endl;
    out << "RETURN" << endl;
    out << endl;

    // Addentum to compute frequency criterion : maybe useless now
    out << "# COMPUTE THE STRESS TENSORS." << endl;
    out << "# MANDATORY TO COMPUTE THE GRADIENTS OF THE FREQUENCY CRITERIONS." << endl;
    out << "SOLVE FORCE" << endl;
    linearModal->markAsWritten();

    return nmodes;
}

void SystusWriter::writeDynaDirectAnalysis(ostream& out, const shared_ptr<Analysis>& linearDynaFreq) {
    out << "# COMPUTING MASS MATRIX" << endl;
    out << "# AS THE COMMAND DYNAMIC COMPUTE THEM, IT SHOULD BE USELESS." << endl;
    out << "# BUT THERE SEEM TO BE BUGS ON THE COMMAND, SO WE USE EXPLICITLY THE COMMAND" << endl;
    out << "CLOSE STIFFNESS MASS" << endl;
    out << endl;
    out << "# SOLVER FILE FOR HARMONIC ANALYSIS WITH DIRECT METHOD" << endl;
    out << "DYNAMIC" << endl;
    out << "HARMONIC RESPONSE VELOCITY ACCELERATION REACTION"<<endl;
    switch (linearDynaFreq->type) {
    case Analysis::Type::LINEAR_DYNA_MODAL_FREQ: {
        const auto& linearDynaModalFreq = static_pointer_cast<LinearDynaModalFreq>(linearDynaFreq);
        writeModalDamping(out, linearDynaModalFreq->getModalDamping());
        writeFrequencyExcit(out, linearDynaModalFreq->getExcitationFrequencies());
        break;
    }
    case Analysis::Type::LINEAR_DYNA_DIRECT_FREQ: {
        const auto& linearDynaDirectFreq = static_pointer_cast<LinearDynaDirectFreq>(linearDynaFreq);
        writeFrequencyExcit(out, linearDynaDirectFreq->getExcitationFrequencies());
        break;
    }
    default:
        handleWritingError(
                "Analysis " + Analysis::stringByType.at(linearDynaFreq->type) + " not (yet) implemented");
    }
    out << "METHOD OPTIMIZED COMPLEX"<<endl;
    out << "RETURN"<<endl;
}

int SystusWriter::writeDynaModalAnalysis(ostream& out, const SystusModel& systusModel, const shared_ptr<LinearDynaModalFreq>& linearDynaModalFreq) {
    // See SYSTUS Reference Manual 11.4 "Dynamic Response - Modal method"

    // First, we need to do a static analysis
    const int iStaticData=42;
    out << "# RUN A STATIC ANALYSIS AND SAVE THE RESULTS."<< endl;
    out << "SOLVE METHOD OPTI" << endl;
    out << "SAVE DATA RESU " << iStaticData<< endl;
    out << "CLOSE MASS" << endl;
    out << endl;

    int nModes = writeLinearModalAnalysis(out, systusModel, linearDynaModalFreq);


    // Participation part
    int nbLoadcases=static_cast<int>(localLoadingListName.size());
    if (nbLoadcases!=1){
        handleWritingWarning("Dynamic modal analysis only work with one loadcase.", "Analysis file");
        nbLoadcases=1;
    }
    out << "# PARTICIPATION" << endl;
    out << "SEARCH DATA RESU " << iStaticData << endl;
    out << "DYNAMIC" << endl;
    out << "# IF THERE IS NNN RIGID BODY MODES, ADD 'RIGID NNN' TO THE NEXT LINE."<<endl;
    out << "PARTICIPATION "<< nModes<< " DOUBLE FORCE "<< nbLoadcases <<endl;
    out << "RETURN" << endl;
    out << endl;

    // Harmonic part
    const int nbNodes = static_cast<int>(systusModel.model.mesh.countNodes());
    const int nbElements = systusModel.model.mesh.countCells();
    out << "# MODAL DYNAMIC ANALYSIS"<<endl;
    out << "DYNAMIC" << endl;
    out << "# IF THERE IS NNN RIGID BODY MODES, ADD 'RIGID NNN' TO THE NEXT LINE."<<endl;
    out << "HARMONIC RESPONSE MODAL "<< nModes<< " FORCE "<< nbLoadcases <<endl;
    if (tableByLoadcase.size()>0){
        out <<"FUNCTION "<< tableByLoadcase[1] <<endl;
        cout <<"FUNCTION "<< tableByLoadcase[0] <<endl;
    }
    writeModalDamping(out, linearDynaModalFreq->getModalDamping());
    writeFrequencyExcit(out, linearDynaModalFreq->getExcitationFrequencies());
    out << "TRANSFER STATIONARY" << endl;
    out << "DISPLACEMENT 1 TO " << nbNodes << " INTERNAL" << endl;
    out << "VELOCITY 1 TO " << nbNodes << " INTERNAL" << endl;
    out << "ACCELERATION 1 TO " << nbNodes << " INTERNAL" << endl;
    out << "REACTION 1 TO " << nbNodes << " INTERNAL" << endl;
    out << "FORCE 1 TO " << nbElements << " INTERNAL" << endl;
    out << "RETURN" << endl;
    out << endl;

    return iStaticData;
}

} //namespace systus
} //namespace vega
