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
 * SystusRunner.h
 *
 *  Created on: Nov 8, 2013
 *      Author: devel
 */

#ifndef SYSTUSRUNNER_H_
#define SYSTUSRUNNER_H_

#include "../Abstract/SolverInterfaces.h"

namespace vega {

class SystusRunner: public vega::Runner {
public:
    SystusRunner();
    virtual ExitCode execSolver(const ConfigurationParameters &configuration,
            std::string modelFile);

    virtual ~SystusRunner();
};

} /* namespace vega */
#endif /* SYSTUSRUNNER_H_ */
