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


std::string SystusTableLabelToString(const SystusTableLabel stl){
    return stringSystusTableLabel.find(stl)->second;
}


SystusTable::SystusTable(long unsigned int id, SystusTableLabel label, long unsigned int type) :
        id(id), label(label), type(type){
}

SystusTable::~SystusTable(){
}

std::ostream& operator<<(std::ostream& os, const SystusTable & st)
{
  os << st.id <<" "<<SystusTableLabelToString(st.label);
  switch (st.label){
  case TL_STANDARD:{
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
      std::cerr << "Systus Writer: table of type "<<SystusTableLabelToString(st.label)<<" are not supported yet."<<std::endl;
  }
  }
  os << std::endl;
  return os;
}

void SystusTable::add(const double value){
    this->values.push_back(value);
}


// Start of Systus Matrix 

SystusMatrix::SystusMatrix(long unsigned int id, int nbDOFS, int nbNodes ) :
        id(id), nbDOFS(nbDOFS), nbNodes(nbNodes){
    this->size=nbNodes*nbNodes*nbDOFS*nbDOFS;
    this->values.resize(this->size, 0.0);

}

SystusMatrix::~SystusMatrix(){
}

void SystusMatrix::setValue(int i, int j, int dofi, int dofj, double value){

    int pos = (dofi-1) + nbDOFS*(dofj-1) + nbDOFS*nbDOFS*(i-1)+ nbDOFS*nbDOFS*nbNodes*(j-1);
    if (pos> this->size)
        throw std::logic_error("Invalid access to Systus Matrix.");
    this->values[pos]=value;
}

// A lot of fields are filled with 0, because we don't know what to put here
// Nonetheless, it seems to work fine this way
// TODO: Complete the writer
std::ostream& operator<<(std::ostream& os, const SystusMatrix & sm)
{
  os << "0"<<std::endl;  // Size of matrix ?
  os << sm.id <<std::endl;  //
  os << sm.nbNodes <<std::endl;  //
  os << sm.nbNodes <<std::endl;  //
  os << sm.size <<std::endl;  //
  os << "0"<<std::endl;  //
  os << "0"<<std::endl;  //

  // Nodes
  for (int i=1; i<=sm.nbNodes;i++)
      os << i <<std::endl;

  // Matrix elements. All dofs of SM(i,j) are written in one line
  long unsigned int ioff=0;
  long unsigned int sizeM = sm.nbDOFS*sm.nbDOFS;
  for (int i=0; i<sm.nbNodes; i++){
      for (int j=0; j<sm.nbNodes; j++){
          for (long unsigned int k=0; k<sizeM; k++){
              os << sm.values[k +ioff]<<" ";
          }
          os << std::endl;
          ioff+= sizeM;
      }
  }

  //os << "0"<<std::endl;
  return os;
}


// Start of SystusMatrices

SystusMatrices::SystusMatrices(){
}

SystusMatrices::~SystusMatrices(){
}

void SystusMatrices::add(SystusMatrix sm){
    this->matrices.push_back(sm);
}

void SystusMatrices::clear(){
    this->matrices.clear();
}

long unsigned int SystusMatrices::size(){
    return this->matrices.size();
}

// A lot of fields are filled with 0, because we don't know what to put here
// Nonetheless, it seems to work fine this way
// TODO: Complete the writer
std::ostream& operator<<(std::ostream& os, const SystusMatrices & sms)
{
  for (int i=1 ; i<12; i++)
      os << "0"<<std::endl;  // Useless parameters ?
  os << sms.nbDOFS << std::endl;
  for (int i=13 ; i<22; i++)
      os << "0"<<std::endl;  // Useless parameters ?

  for (long unsigned int i=0; i< sms.matrices.size(); i++)
      os << sms.matrices[i];

  // End of file
  os << SystusMatrix(0,0,0) <<std::endl;  //
  return os;
}



} //namespace Vega
