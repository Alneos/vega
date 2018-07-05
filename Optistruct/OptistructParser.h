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
 * OptistructParser.h
 *
 *  Created on: Jul 5, 2018
 *      Author: Luca Dall'Olio
 */

#ifndef OPTISTRUCTPARSER_H_
#define OPTISTRUCTPARSER_H_

#include <boost/filesystem.hpp>
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "../Nastran/NastranParser.h"

namespace vega {

namespace optistruct {

class OptistructParser: public nastran::NastranParser {

};

} /* namespace optistruct */
} /* namespace vega */
#endif /* OPTISTRUCTPARSER_H_ */
