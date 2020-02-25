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
 *  SystusAsc.cpp
 *
 *  Created on: 28/03/2017
 *      Author: Thomas Abballe
 *     Comment: Regroups classes that modelize parts of the Systus ASC
 *              format, in order to simplify the code in SystusWriter
 */

#include "SystusAsc.h"


namespace vega {
namespace systus {

using namespace std;

string SystusTableLabelToString(const SystusTableLabel stl){
    return stringSystusTableLabel.find(stl)->second;
}


SystusTable::SystusTable(systus_ascid_t id, SystusTableLabel label, systus_ascid_t type) :
        id(id), label(label), type(type) {
}

ostream& operator<<(ostream& os, const SystusTable & st)
{
  os << st.id <<" "<<SystusTableLabelToString(st.label);
  switch (st.label){
  case SystusTableLabel::TL_STANDARD:{
      // Type 0 is "no type" just a bunch of values without an interpolation rule.
      // Other Type are needed as an "INTERPOLATION KEY"
      if (st.type>0){
          os <<" "<<st.type;
      }
      for (const auto v : st.values){
          os <<" "<<v;
      }
      break;
  }
  default:{
      cerr << "Systus Writer: table of type "<<SystusTableLabelToString(st.label)<<" are not supported yet."<<endl;
  }
  }
  os << endl;
  return os;
}

void SystusTable::add(const double value){
    this->values.push_back(value);
}

void SystusTable::fill(std::shared_ptr<MatrixElement> me, int nbDOFS){

    //Numbering the node internally to the element
    map<int, int> positionToSytusNumber;
    int iSystus= 1;
    for (const int pos : me->nodePositions()){
        positionToSytusNumber[pos]=iSystus;
        iSystus++;
    }

    // Building the table
    for (const auto np : me->nodePairs()){
        int pairCode = positionToSytusNumber[np.first]*1000 + positionToSytusNumber[np.second]*100;
        shared_ptr<const DOFMatrix> dM = me->findSubmatrix(np.first, np.second);
        for (const auto dof: dM->componentByDofs){
            int dofi=DOFToInt(dof.first.first);
            if (dofi> nbDOFS)
                throw logic_error("Invalid degree of freedom ("+to_string(dofi)+") for Systus Table.");
            int dofj=DOFToInt(dof.first.second);
            if (dofj> nbDOFS)
                throw logic_error("Invalid degree of freedom ("+to_string(dofj)+") for Systus Table.");
            int dofCode = 10*dofi + dofj;
            this->values.push_back(pairCode+dofCode);
            this->values.push_back(dof.second);
        }
    }

}




// Start of Systus Matrix

SystusMatrix::SystusMatrix(systus_ascid_t id, int nbDOFS, int nbNodes ) :
        id(id), nbDOFS(nbDOFS), nbNodes(nbNodes), size{nbNodes*nbNodes*nbDOFS*nbDOFS} {
    this->values.resize(this->size, 0.0);

}

void SystusMatrix::setValue(int i, int j, int dofi, int dofj, double value){
    if (dofi> this->nbDOFS)
        throw logic_error("Invalid degree of freedom ("+to_string(dofi)+") for Systus Matrix.");
    if (dofj> this->nbDOFS)
        throw logic_error("Invalid degree of freedom ("+to_string(dofj)+") for Systus Matrix.");
    int pos = (dofi-1) + nbDOFS*(dofj-1) + nbDOFS*nbDOFS*(i-1)+ nbDOFS*nbDOFS*nbNodes*(j-1);
    if (pos> this->size)
        throw logic_error("Invalid access to Systus Matrix.");
    this->values[pos]=value;
}

// A lot of fields are filled with 0, because we don't know what to put here
// Nonetheless, it seems to work fine this way
// TODO: Complete the writer
ostream& operator<<(ostream& os, const SystusMatrix & sm)
{
  os << "0"<<endl;  // Size of matrix ?
  os << sm.id <<endl;  //
  os << sm.nbNodes <<endl;  //
  os << sm.nbNodes <<endl;  //
  os << sm.size <<endl;  //
  os << "0"<<endl;  //
  os << "0"<<endl;  //

  // Nodes
  for (int i=1; i<=sm.nbNodes;i++)
      os << i <<endl;

  // Matrix elements. All dofs of SM(i,j) are written in one line
  systus_ascid_t ioff=0;
  systus_ascid_t sizeM = sm.nbDOFS*sm.nbDOFS;
  for (int i=0; i<sm.nbNodes; i++){
      for (int j=0; j<sm.nbNodes; j++){
          for (systus_ascid_t k=0; k<sizeM; k++){
              os << sm.values[k +ioff]<<" ";
          }
          os << endl;
          ioff+= sizeM;
      }
  }

  //os << "0"<<endl;
  return os;
}


// Start of SystusMatrices

void SystusMatrices::add(SystusMatrix sm){
    this->matrices.push_back(sm);
}

void SystusMatrices::clear(){
    this->matrices.clear();
}

vector<SystusMatrix>::size_type SystusMatrices::size() const {
    return this->matrices.size();
}

// A lot of fields are filled with 0, because we don't know what to put here
// Nonetheless, it seems to work fine this way
// TODO: Complete the writer
ostream& operator<<(ostream& os, const SystusMatrices & sms)
{
  for (int i=1 ; i<12; i++)
      os << "0"<<endl;  // Useless parameters ?
  os << sms.nbDOFS << endl;
  for (int i=13 ; i<22; i++)
      os << "0"<<endl;  // Useless parameters ?

  for (systus_ascid_t i=0; i< sms.matrices.size(); i++)
      os << sms.matrices[i];

  // End of file
  os << SystusMatrix(0,0,0) <<endl;  //
  return os;
}



string SystusOptionToString(SystusOption sO, SystusSubOption ssO){
    string s1 = SystusOptiontoString.find(sO)->second;
    string s2 = SystusSubOptiontoString.find(ssO)->second;
    if (s2 != "")
        s1+="-"+s2;
    return s1;
}

std::ostream& operator<<(std::ostream& os, const SystusOption & sO){
    os << static_cast<int>(sO);
    return os;
}
std::ostream& operator<<(std::ostream& os, const SystusSubOption & ssO){
    os << static_cast<int>(ssO);
    return os;
}

double initSystusAscConstraintVector(std::vector<double> & vec){

    vec.clear();
    vec.push_back(4);
    vec.push_back(0);
    vec.push_back(0);
    vec.push_back(0);
    vec.push_back(0);
    vec.push_back(0);
    return 0.0;
}


int DOFToInt(const DOF dof){
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


} //namespace systus
} //namespace vega
