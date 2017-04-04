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
      // First line is what the standard says, but is seems to be wrong
      //os <<" "<<st.type;
      os << "  ";
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



} //namespace Vega
