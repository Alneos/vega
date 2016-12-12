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
 * SystusModel.h
 *
 *  Created on: Oct 2, 2013
 *      Author: devel
 */

#ifndef SYSTUSMODEL_H_
#define SYSTUSMODEL_H_

#include "../Abstract/Model.h"
#include "../Abstract/ConfigurationParameters.h"

namespace vega {

class SystusModel {
public:
    SystusModel(const vega::Model* model, const vega::ConfigurationParameters &configuration);
    virtual ~SystusModel();
    const vega::Model* model;
    const vega::ConfigurationParameters configuration;
    /*string phenomene;*/
    /*const string memjeveux;*/
    const string getName() const;
    const string getOutputFileName(string extension) const;
    /*const string getSystusVersion() const;*/
};

}

#endif /* SYSTUSMODEL_H_ */
