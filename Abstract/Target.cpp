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

Target::Target(Model& model, Target::Type type, int original_id) :
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

NodeTarget::NodeTarget(Model& model, Target::Type type, int original_id) :
        Target(model, type, original_id) {
}

BoundaryNodeCloud::BoundaryNodeCloud(Model& model, list<int> nodeids, int original_id) :
        NodeTarget(model, Target::Type::BOUNDARY_NODECLOUD, original_id), nodeids{nodeids} {
}

BoundaryNodeLine::BoundaryNodeLine(Model& model, list<int> nodeids, int original_id) :
        NodeTarget(model, Target::Type::BOUNDARY_NODELINE, original_id), nodeids{nodeids} {
}

BoundaryNodeSurface::BoundaryNodeSurface(Model& model, list<int> nodeids, int original_id) :
        NodeTarget(model, Target::Type::BOUNDARY_NODESURFACE, original_id), nodeids{nodeids} {
}

CellTarget::CellTarget(Model& model, Target::Type type, int original_id) :
        Target(model, type, original_id) {
}

BoundarySurface::BoundarySurface(Model& model, int original_id) :
        CellTarget(model, Target::Type::BOUNDARY_SURFACE, original_id), CellContainer(model.mesh) {
}

ContactBody::ContactBody(Model& model, Reference<Target> boundary, int original_id) :
        Target(model, Target::Type::CONTACT_BODY, original_id), boundary{boundary} {
}

BoundaryElementFace::ElementFaceByTwoNodes::ElementFaceByTwoNodes(int cellId, int nodeid1, int nodeid2, bool swapNormal):
    cellId(cellId), nodeid1(nodeid1), nodeid2(nodeid2), swapNormal(swapNormal) {
}

BoundaryElementFace::BoundaryElementFace(Model& model, list<ElementFaceByTwoNodes> faceInfos, int original_id):
    CellTarget(model, Target::Type::BOUNDARY_ELEMENTFACE, original_id), faceInfos(faceInfos) {

}

void BoundaryElementFace::createSkin() {
    ostringstream oss2;
    oss2 << "created by generateSkin() because of BOUNDARY_ELEMENTFACE";
    shared_ptr<CellGroup> surfGrp = model.mesh.createCellGroup("SURF" + to_string(bestId()), Group::NO_ORIGINAL_ID, oss2.str());
    shared_ptr<CellGroup> elemGrp = model.mesh.createCellGroup("SURFO" + to_string(bestId()), Group::NO_ORIGINAL_ID, "BOUNDARY ELEMENTFACE");
    for(const auto& faceInfo: faceInfos) {
        elemGrp->addCellId(faceInfo.cellId);
        elementCellGroup = elemGrp;
        const Cell& cell0 = model.mesh.findCell(model.mesh.findCellPosition(faceInfo.cellId));
        const vector<int>& faceIds = cell0.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
        if (faceIds.size() > 0) {
            //addedSkin = true;
            const int cellPosition = model.mesh.generateSkinCell(faceIds, SpaceDimension::DIMENSION_2D);
            //mappl->addCellPosition(cell.position);
            surfGrp->addCellPosition(cellPosition);
            surfaceCellGroup = surfGrp;
        }
    }
    // LD : Workaround for Aster problem : MODELISA6_96
    //  les 1 mailles imprimées ci-dessus n'appartiennent pas au modèle et pourtant elles ont été affectées dans le mot-clé facteur : !
    //   ! FORCE_FACE
    const auto& skin = make_shared<Continuum>(model, model.modelType);
    skin->assignCellGroup(surfGrp);
    model.add(skin);
}

} // namespace vega
