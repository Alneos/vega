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
        { Target::Type::BOUNDARY_SURFACE, "BOUNDARY_SURFACE" },
        { Target::Type::CONTACT_BODY, "CONTACT_BODY" },
        { Target::Type::BOUNDARY_ELEMENTFACE, "BOUNDARY_ELEMENTFACE" },
};

ostream &operator<<(ostream &out, const Target& target) {
    out << to_str(target);
    return out;
}

const string Target::to_str() const{
    const auto& typePair = stringByType.find(type);
    if (typePair == stringByType.end())
        return "Unknown constraint";
    else
        return typePair->second;
}

BoundaryNodeCloud::BoundaryNodeCloud(const Model& model, list<int> nodeids, int original_id) :
        Target(model, Target::Type::BOUNDARY_NODECLOUD, original_id), nodeids{nodeids} {
}

shared_ptr<Target> BoundaryNodeCloud::clone() const {
    return make_shared<BoundaryNodeCloud>(*this);
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

BoundarySurface::BoundarySurface(const Model& model, shared_ptr<CellGroup> cellGroup, int original_id) :
        Target(model, Target::Type::BOUNDARY_SURFACE, original_id), cellGroup{cellGroup} {
}

shared_ptr<Target> BoundarySurface::clone() const {
    return make_shared<BoundarySurface>(*this);
}

ContactBody::ContactBody(const Model& model, Reference<Target> boundary, int original_id) :
        Target(model, Target::Type::CONTACT_BODY, original_id), boundary{boundary} {
}

shared_ptr<Target> ContactBody::clone() const {
    return make_shared<ContactBody>(*this);
}

BoundaryElementFace::ElementFaceByTwoNodes::ElementFaceByTwoNodes(int cellId, int nodeid1, int nodeid2, bool swapNormal):
    cellId(cellId), nodeid1(nodeid1), nodeid2(nodeid2), swapNormal(swapNormal) {
}

BoundaryElementFace::BoundaryElementFace(const Model& model, list<ElementFaceByTwoNodes> faceInfos, int original_id):
    Target(model, Target::Type::BOUNDARY_ELEMENTFACE, original_id), faceInfos(faceInfos) {

}

shared_ptr<Target> BoundaryElementFace::clone() const {
    return make_shared<BoundaryElementFace>(*this);
}

} // namespace vega
