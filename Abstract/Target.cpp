/*
 * Copyright (C) IRT-Systemx (contact@irt-systemx.fr)
 *
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
 * Target.cpp
 *
 *  Created on: Oct 31, 2018
 *      Author: Luca Dall'Olio
 */

#include "Target.h"
#include "Model.h"

using namespace std;

namespace vega {

Target::Target(const Model& model, Target::Type type, int original_id) :
        Identifiable(original_id), model(model), type(type) {
}

const string Target::name = "Target";

const map<Target::Type, string> Target::stringByType = {
        { Target::Type::BOUNDARY_NODELINE, "BOUNDARY_NODELINE" },
        { Target::Type::BOUNDARY_NODESURFACE, "BOUNDARY_NODESURFACE" },
};

ostream &operator<<(ostream &out, const Target& objective) {
    out << to_str(objective);
    return out;
}

BoundaryNodeLine::BoundaryNodeLine(const Model& model, list<int> nodeids, int original_id) :
        Target(model, Target::Type::BOUNDARY_NODELINE, original_id), nodeids{nodeids} {
}

shared_ptr<Target> BoundaryNodeLine::clone() const {
    return make_shared<BoundaryNodeLine>(*this);
}

BoundaryNodeSurface::BoundaryNodeSurface(const Model& model, list<int> nodeids, int original_id) :
        Target(model, Target::Type::BOUNDARY_NODESURFACE, original_id), nodeids{nodeids} {
}

shared_ptr<Target> BoundaryNodeSurface::clone() const {
    return make_shared<BoundaryNodeSurface>(*this);
}

}
