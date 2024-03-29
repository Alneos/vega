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
 * MedWriter.h
 *
 *  Created on: Nov 29, 2018
 *      Author: Luca Dall'Olio
 */


#ifndef MEDWRITER_H_
#define MEDWRITER_H_

#include "../Abstract/ConfigurationParameters.h"
#include "../Abstract/Mesh.h"
#include <med.h>

namespace vega {

namespace aster {

struct Family final {
public:
    std::vector<std::shared_ptr<Group>> groups;
    std::string name = "";
    med_int num = Globals::UNAVAILABLE_INT;
};

class NodeGroup2Families {
    std::vector<Family> families;
    std::vector<med_int> nodes;
public:
    NodeGroup2Families(med_int nnodes, const std::vector<std::shared_ptr<NodeGroup>> nodeGroups);
    std::vector<Family> getFamilies() const;
    std::vector<med_int> getFamilyOnNodes() const;
};

class CellGroup2Families final {
private:
    std::vector<Family> families;
    std::unordered_map<CellType::Code, std::shared_ptr<std::vector<med_int>>, EnumClassHash> cellFamiliesByType;
    const Mesh& mesh;
public:
    CellGroup2Families(const Mesh& mesh, std::unordered_map<CellType::Code, size_t, EnumClassHash> cellCountByType,
            const std::vector<std::shared_ptr<CellGroup>>& cellGroups);
    std::vector<Family> getFamilies() const;
    std::unordered_map<CellType::Code, std::shared_ptr<std::vector<med_int>>, EnumClassHash> getFamilyOnCells() const;
};


class MedWriter {
private:
    friend Mesh;
    friend NodeData;
public:
    MedWriter() = default;
	MedWriter(const MedWriter& that) = delete;
	void writeMED(const Model& model, const std::string& medFileName);
};

}
}
#endif /* MEDWRITER_H_ */
