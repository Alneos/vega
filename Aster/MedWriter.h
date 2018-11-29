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
 * NastranParser.cpp
 *
 *  Created on: Nov 29, 2018
 *      Author: Luca Dall'Olio
 */


#ifndef MEDWRITER_H_
#define MEDWRITER_H_

#include "../Abstract/ConfigurationParameters.h"
#include "../Abstract/MeshComponents.h"
#include "../Abstract/Mesh.h"
#include <med.h>
#define MESGERR 1

namespace vega {

namespace aster {

class MedWriter {
private:
    friend Mesh;
    friend NodeData;
	void createFamilies(med_idt fid, const char meshname[MED_NAME_SIZE + 1],
			const std::vector<Family>& families);
public:
	MedWriter();
	void writeMED(const Model& model, const std::string& medFileName);
};

}
}
#endif /* MEDWRITER_H_ */
