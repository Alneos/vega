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
namespace systus {

class SystusModel {
public:
    SystusModel(const vega::Model* model, const vega::ConfigurationParameters &configuration);
    virtual ~SystusModel();
    const vega::Model* model;
    const vega::ConfigurationParameters configuration;
    /*string phenomene;*/
    /*const string memjeveux;*/
    const std::string getName() const;
    const std::string getOutputFileName(std::string extension) const;
    double getSystusVersion() const;
    const std::string getSystusVersionString() const;

private:
    double systusVersion;
};

} // namespace systus
} // namespace vega

#endif /* SYSTUSMODEL_H_ */
