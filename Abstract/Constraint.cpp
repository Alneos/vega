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
 * Constraint.cpp
 *
 *  Created on: 2 juin 2014
 *      Author: siavelis
 */

#include "Constraint.h"
#include "Model.h"
#include <ciso646>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/geometries.hpp>

namespace vega {

using namespace std;

Constraint::Constraint(Model& model, Type type, int original_id) :
        Identifiable(original_id), model(model), type(type) {
}

const string Constraint::name = "Constraint";

const map<Constraint::Type, string> Constraint::stringByType = {
        { Constraint::Type::QUASI_RIGID, "QUASI_RIGID" },
        { Constraint::Type::RIGID, "RIGID" },
        { Constraint::Type::SPC, "SPC" },
        { Constraint::Type::RBE3, "RBE3" },
        { Constraint::Type::GAP, "GAP" },
        { Constraint::Type::LMPC, "LMPC" },
        { Constraint::Type::SLIDE, "SLIDE" },
        { Constraint::Type::SURFACE_CONTACT, "SURFACE_CONTACT" },
        { Constraint::Type::ZONE_CONTACT, "ZONE_CONTACT" },
        { Constraint::Type::SURFACE_SLIDE_CONTACT, "SURFACE_SLIDE_CONTACT" },
};

ostream &operator<<(ostream &out, const Constraint& constraint) {
    out << to_str(constraint);
    return out;
}

const string Constraint::to_str() const{
    const auto& typePair = stringByType.find(type);
    if (typePair == stringByType.end())
        return "Unknown constraint";
    else
        return typePair->second;
}

ConstraintSet::ConstraintSet(Model& model, Type type, int original_id) :
        Identifiable(original_id), model(model), type(type) {
}

void ConstraintSet::add(const Reference<ConstraintSet>& constraintSetReference) {
    // LD Hack : see parseSPCADD
    constraintSetReferences.push_back(constraintSetReference);
}

const set<shared_ptr<Constraint> > ConstraintSet::getConstraints() const {
    set<shared_ptr<Constraint>> result = model.getConstraintsByConstraintSet(this->getReference());
    for (const auto& constraintSetReference : constraintSetReferences) {
        set<shared_ptr<Constraint>> setToInsert = model.getConstraintsByConstraintSet(
                constraintSetReference);
        result.insert(setToInsert.begin(), setToInsert.end());
    }
    return result;
}

const set<shared_ptr<Constraint> > ConstraintSet::getConstraintsByType(
        Constraint::Type type) const {
    set<shared_ptr<Constraint> > result;
    for (shared_ptr<Constraint> constraint : getConstraints()) {
        if (constraint->type == type) {
            result.insert(constraint);
        }
    }
    return result;
}

int ConstraintSet::size() const {
    return static_cast<int>(getConstraints().size());
}

const string ConstraintSet::name = "ConstraintSet";

const map<ConstraintSet::Type, string> ConstraintSet::stringByType = {
    { ConstraintSet::Type::SPC, "SPC" },
    { ConstraintSet::Type::MPC, "MPC" },
    { ConstraintSet::Type::CONTACT, "CONTACT" },
    { ConstraintSet::Type::ALL, "ALL" }
    };

bool ConstraintSet::hasFunctions() const {
    for (shared_ptr<Constraint> constraint : getConstraints()) {
		if (constraint->hasFunctions()) {
			return true;
		}
	}
	return false;
}

bool ConstraintSet::hasContacts() const {
    for (shared_ptr<Constraint> constraint : getConstraints()) {
		if (constraint->isContact()) {
			return true;
		}
	}
	return false;
}

ostream &operator<<(ostream &out, const ConstraintSet& constraintSet) {
    out << to_str(constraintSet);
    return out;
}

const int HomogeneousConstraint::UNAVAILABLE_MASTER = INT_MIN;

HomogeneousConstraint::HomogeneousConstraint(Model& model, Type type, const DOFS& dofs,
        int masterId, int original_id, const set<int>& slaveIds) :
        Constraint(model, type, original_id), dofs(dofs),
        masterPosition{masterId == UNAVAILABLE_MASTER ? UNAVAILABLE_MASTER : model.mesh.findOrReserveNode(masterId)},
        slavePositions(model.mesh.findOrReserveNodes(slaveIds)) {
}

int HomogeneousConstraint::getMaster() const {
    if (this->masterPosition != HomogeneousConstraint::UNAVAILABLE_MASTER) {
        return this->masterPosition;
    } else {
        throw logic_error("getMaster: automatic resolution of master id not implemented");
    }
}

/** Getter for dofs data. **/
const DOFS HomogeneousConstraint::getDOFS() const {
    return this->dofs;
}

void HomogeneousConstraint::addSlave(int slaveId) {
    this->slavePositions.insert(model.mesh.findOrReserveNode(slaveId));
}

set<int> HomogeneousConstraint::nodePositions() const {
    set<int> result;
    if (this->masterPosition != UNAVAILABLE_MASTER) {
        result.insert(masterPosition);
    }

    for (int slavePosition : slavePositions) {
        result.insert(slavePosition);
    }
    return result;
}

const DOFS HomogeneousConstraint::getDOFSForNode(int nodePosition) const {
    if (nodePosition == masterPosition
            || slavePositions.find(nodePosition) != slavePositions.end()) {
        return this->dofs;
    } else {
        return DOFS::NO_DOFS;
    }

}

void HomogeneousConstraint::removeNode(int nodePosition) {
    UNUSEDV(nodePosition);
    throw logic_error("removeNode for HomogeneousConstraint not implemented");
}

bool HomogeneousConstraint::ineffective() const {
    return masterPosition == UNAVAILABLE_MASTER && slavePositions.size() == 0;
}

set<int> HomogeneousConstraint::getSlaves() const {
    if (this->masterPosition != HomogeneousConstraint::UNAVAILABLE_MASTER) {
        return this->slavePositions;
    } else {
        throw logic_error("getSlaves: automatic resolution of master id not implemented");
    }
}

QuasiRigidConstraint::QuasiRigidConstraint(Model& model, const DOFS& dofs, int masterId,
        int constraintGroup, const set<int>& slaveIds) :
        HomogeneousConstraint(model, Constraint::Type::QUASI_RIGID, dofs, masterId, constraintGroup, slaveIds) {

}

set<int> QuasiRigidConstraint::getSlaves() const {
    return this->slavePositions;
}



RigidConstraint::RigidConstraint(Model& model, int masterId, int constraintGroup,
        const set<int>& slaveIds) :
        HomogeneousConstraint(model, Constraint::Type::RIGID, DOFS::ALL_DOFS, masterId, constraintGroup, slaveIds) {
}

RBE3::RBE3(Model& model, int masterId, const DOFS dofs, int original_id) :
        HomogeneousConstraint(model, Constraint::Type::RBE3, dofs, masterId, original_id) {
}

void RBE3::addSlave(int slaveId, DOFS slaveDOFS, double slaveCoef) {
    int nodePosition = model.mesh.findOrReserveNode(slaveId);
    slavePositions.insert(nodePosition);
    slaveDofsByPosition[nodePosition] = slaveDOFS;
    slaveCoefByPosition[nodePosition] = slaveCoef;
}

double RBE3::getCoefForNode(int nodePosition) const {
    double result = 0;
    if (nodePosition == masterPosition)
        result = 1;
    else {
        auto it = slaveCoefByPosition.find(nodePosition);
        if (it != slaveCoefByPosition.end())
            result = it->second;
    }
    return result;
}

const DOFS RBE3::getDOFSForNode(int nodePosition) const {
    DOFS result = DOFS::ALL_DOFS;
    if (nodePosition == masterPosition){
        result = dofs;
    }
    else {
        auto it = slaveDofsByPosition.find(nodePosition);
        if (it != slaveDofsByPosition.end())
            result = it->second;
    }
    return result;
}

const ValueOrReference& SinglePointConstraint::NO_SPC = ValueOrReference::EMPTY_VALUE;

SinglePointConstraint::SinglePointConstraint(Model& _model,
        const array<ValueOrReference, 6>& _spcs, shared_ptr<Group> _group, int original_id) :
        Constraint(_model, Constraint::Type::SPC, original_id), spcs(_spcs), group(_group) {
}

SinglePointConstraint::SinglePointConstraint(Model& _model,
        const array<ValueOrReference, 3>& _spcs, shared_ptr<Group> _group, int original_id) :
        Constraint(_model, Constraint::Type::SPC, original_id), spcs( { { NO_SPC, NO_SPC, NO_SPC, NO_SPC, NO_SPC,
                NO_SPC } }), group(_group) {
    copy_n(_spcs.begin(), 3, spcs.begin());
}

SinglePointConstraint::SinglePointConstraint(Model& _model, DOFS dofs, double value, shared_ptr<Group> _group,
        int original_id) :
        Constraint(_model, Constraint::Type::SPC, original_id), spcs( { { NO_SPC, NO_SPC, NO_SPC, NO_SPC, NO_SPC,
                NO_SPC } }), group(_group) {
    for (DOF dof : dofs) {
        setDOF(dof, value);
    }
}

SinglePointConstraint::SinglePointConstraint(Model& _model, shared_ptr<Group> _group, int original_id) :
        Constraint(_model, Constraint::Type::SPC, original_id), spcs( { { NO_SPC, NO_SPC, NO_SPC, NO_SPC, NO_SPC,
                NO_SPC } }), group(_group) {
}

void SinglePointConstraint::setDOF(const DOF& dof, const ValueOrReference& value) {
    spcs[dof.position] = value;
}

void SinglePointConstraint::setDOFS(const DOFS& dofs, const ValueOrReference& value) {
    for (DOF dof : dofs) {
        spcs[dof.position] = value;
    }
}

void SinglePointConstraint::addNodeId(int nodeId) {
    int nodePosition = model.mesh.findOrReserveNode(nodeId);
    if (group == nullptr) {
        _nodePositions.insert(nodePosition);
    } else {
        if (group->type == Group::Type::NODEGROUP) {
            shared_ptr<NodeGroup> const ngroup = dynamic_pointer_cast<NodeGroup>(group);
            ngroup->addNodeByPosition(nodePosition);
        } else {
            throw logic_error("SPC:: addNodeId on unknown group type");
        }
    }
}

set<int> SinglePointConstraint::nodePositions() const {
    set<int> result;
    result.insert(_nodePositions.begin(), _nodePositions.end());
    if (group != nullptr) {
        if (this->group->type == Group::Type::NODEGROUP) {
            shared_ptr<NodeGroup> const ngroup = dynamic_pointer_cast<NodeGroup>(group);
            set<int> nodePositions = ngroup->nodePositions();
            result.insert(nodePositions.begin(), nodePositions.end());
        } else {
            throw logic_error("SPC:: getAllNodes on unknown group type");
        }
    }
    return result;
}

void SinglePointConstraint::removeNode(int nodePosition) {
    if (_nodePositions.find(nodePosition) != _nodePositions.end()) {
        _nodePositions.erase(nodePosition);
    }
    if (group != nullptr) {
        if (group->type == Group::Type::NODEGROUP) {
            shared_ptr<NodeGroup> const ngroup = dynamic_pointer_cast<NodeGroup>(group);
            ngroup->removeNodeByPosition(nodePosition);
        } else {
            throw logic_error("SPC:: removeNode on unknown group type");
        }
    }
}

const DOFS SinglePointConstraint::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    DOFS requiredDofs;
    for (int i = 0; i < 6; i++) {
        const ValueOrReference& valOrRef = spcs[i];
        if (valOrRef != NO_SPC) {
            requiredDofs = requiredDofs + DOF::findByPosition(i);
        }
    }
    return requiredDofs;
}

bool SinglePointConstraint::hasReferences() const {
    bool hasReferences = false;
    for (ValueOrReference value : spcs) {
        hasReferences |= (value.isReference() && value != NO_SPC);
    }
    return hasReferences;
}

bool SinglePointConstraint::ineffective() const {
    return nodePositions().size() == 0;
}

double SinglePointConstraint::getDoubleForDOF(const DOF& dof) const {
    const ValueOrReference& spcVal = spcs[dof.position];
    if (spcVal == NO_SPC) {
        cerr << "SPC" << *this << " requested getDoubleForDof, dof " << dof
                << " but the dof is free.";
        throw invalid_argument("SPC requested getDoubleForDof for a free DOF");
    }
    if (spcVal.isReference()) {
        cerr << "SPC" << *this << " requested getDoubleForDof, dof " << dof
                << " but the dof is a reference.";
        throw invalid_argument("SPC requested getDoubleForDof for a DOF of type reference");
    }
    return spcVal.getValue();
}

shared_ptr<Value> SinglePointConstraint::getReferenceForDOF(const DOF& dof) const {
    const ValueOrReference& spcVal = spcs[dof.position];
    if (spcVal == NO_SPC) {
        cerr << "SPC" << *this << " requested getValueForDOF, dof " << dof
                << " but the dof is free.";
        throw invalid_argument("SPC requested getValueForDOF for a free DOF");
    }
    if (!spcVal.isReference()) {
        cerr << "SPC" << *this << " requested getValueForDOF, dof " << dof
                << " but the dof is a reference.";
        throw invalid_argument("SPC requested getValueForDOF for a DOF of type double");
    }
    return model.find(spcVal.getReference());
}

void SinglePointConstraint::emulateLocalDisplacementConstraint() {
    set<shared_ptr<LinearMultiplePointConstraint>> lmpcs;
    for (int nodePosition : this->nodePositions()) {
        const Node& node = model.mesh.findNode(nodePosition);
        if (node.displacementCS != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
            shared_ptr<CoordinateSystem> coordSystem = model.mesh.getCoordinateSystemByPosition(node.displacementCS);
            // TODO LD: this should be done differently: is used to compute different nodes but every this it changes the coordinate system instance!
            coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
            const DOFS& dofs = this->getDOFSForNode(nodePosition);
            if (model.configuration.logLevel >= LogLevel::DEBUG)
                cout << "Replacing local spc " << *this << " for: " << node << ",dofs " << this->getDOFSForNode(nodePosition) << endl;
            for (int i = 0; i < 6; i++) {
                const DOF& currentDOF = *DOF::dofByPosition[i];
                if (dofs.contains(currentDOF)) {
                    const VectorialValue& participation = coordSystem->vectorToGlobal(
                            VectorialValue::XYZ[i % 3]);
                    shared_ptr<LinearMultiplePointConstraint> lmpc =
                            make_shared<LinearMultiplePointConstraint>(model,
                                    this->getDoubleForDOF(currentDOF));
                    if (i < 3) {
                        lmpc->addParticipation(node.id, participation.x(),
                                participation.y(), participation.z());
                    } else {
                        lmpc->addParticipation(node.id, 0, 0, 0, participation.x(),
                                participation.y(), participation.z());
                    }
                    if (model.configuration.logLevel >= LogLevel::DEBUG)
                        cout << "Adding: " << node << ", current dof:" << currentDOF << ", participation:" << participation << ", coef:" << lmpc->coef_impo << endl;
                    lmpcs.insert(lmpc);
                }
            }
            this->removeNode(nodePosition);
        }
    }
    const set<shared_ptr<ConstraintSet>> constraintSets = model.getConstraintSetsByConstraint(
                this->getReference());
    for (const auto& linearMultiplePointConstraint : lmpcs) {
        model.add(linearMultiplePointConstraint);
        for (const auto& constraintSet : constraintSets) {
            if (model.configuration.logLevel >= LogLevel::DEBUG)
                cout << "Adding Local emulation constraint:" << *linearMultiplePointConstraint << " to constraintset: " << *constraintSet << ";" << endl;
            model.addConstraintIntoConstraintSet(linearMultiplePointConstraint->getReference(),
                    constraintSet->getReference());
        }
    }
}

LinearMultiplePointConstraint::LinearMultiplePointConstraint(Model& model, double coef_impo,
        int original_id) :
        Constraint(model, Constraint::Type::LMPC, original_id), coef_impo(coef_impo) {
}

void LinearMultiplePointConstraint::addParticipation(int nodeId, double dx, double dy, double dz,
        double rx, double ry, double rz) {
    int nodePosition = model.mesh.findOrReserveNode(nodeId);
    auto it = dofCoefsByNodePosition.find(nodePosition);
    if (it == dofCoefsByNodePosition.end())
        dofCoefsByNodePosition[nodePosition] = DOFCoefs(dx, dy, dz, rx, ry, rz);
    else
        dofCoefsByNodePosition[nodePosition] += DOFCoefs(dx, dy, dz, rx, ry, rz);
}

set<int> LinearMultiplePointConstraint::nodePositions() const {
    set<int> result;
    for (const auto& it : dofCoefsByNodePosition) {
        result.insert(it.first);
    }
    return result;
}

const DOFS LinearMultiplePointConstraint::getDOFSForNode(int nodePosition) const {
    DOFS dofs(DOFS::NO_DOFS);
    auto it = dofCoefsByNodePosition.find(nodePosition);
    if (it != dofCoefsByNodePosition.end()) {
        DOFCoefs dofCoefs = it->second;
        if (!is_zero(dofCoefs[0]))
            dofs += DOF::DX;
        if (!is_zero(dofCoefs[1]))
            dofs += DOF::DY;
        if (!is_zero(dofCoefs[2]))
            dofs += DOF::DZ;
        if (!is_zero(dofCoefs[3]))
            dofs += DOF::RX;
        if (!is_zero(dofCoefs[4]))
            dofs += DOF::RY;
        if (!is_zero(dofCoefs[5]))
            dofs += DOF::RZ;
    }
    return dofs;
}

void LinearMultiplePointConstraint::removeNode(int nodePosition) {
    dofCoefsByNodePosition.erase(nodePosition);
}

bool LinearMultiplePointConstraint::ineffective() const  {
    return dofCoefsByNodePosition.size() == 0;
}

DOFCoefs LinearMultiplePointConstraint::getDoFCoefsForNode(
        int nodePosition) const {
    DOFCoefs dofCoefs(0, 0, 0, 0, 0, 0);
    auto it = dofCoefsByNodePosition.find(nodePosition);
    if (it != dofCoefsByNodePosition.end())
        dofCoefs = it->second;
    return dofCoefs;
}

std::vector<int> LinearMultiplePointConstraint::sortNodePositionByCoefs() const{

    std::set<int> np=this->nodePositions();
    std::vector<int> indices(np.size());
    int i = 0;
    for (const auto n : np ){
        indices[i]=n;
        i++;
    }
    std::sort(
            begin(indices), end(indices),
            [&](int a, int b) { return this->dofCoefsByNodePosition.at(a) < this->dofCoefsByNodePosition.at(b); }
    );
    return indices;
}

Contact::Contact(Model& model, Type type, int original_id) :
        Constraint(model, type, original_id) {
}

Gap::Gap(Model& model, int original_id) :
        Contact(model, Constraint::Type::GAP, original_id) {
}

GapTwoNodes::GapTwoNodes(Model& model, int original_id) :
        Gap(model, original_id) {
}

void GapTwoNodes::addGapNodes(int constrainedNodeId, int directionNodeId) {
    int constrainedNodePosition = model.mesh.findOrReserveNode(constrainedNodeId);
    int directionNodePosition = model.mesh.findOrReserveNode(directionNodeId);
    directionNodePositionByconstrainedNodePosition[constrainedNodePosition] = directionNodePosition;
}

vector<shared_ptr<Gap::GapParticipation>> GapTwoNodes::getGaps() const {
    vector<shared_ptr<Gap::GapParticipation>> result;
    result.reserve(directionNodePositionByconstrainedNodePosition.size());
    for (const auto& it : directionNodePositionByconstrainedNodePosition) {
        const Node& constrainedNode = model.mesh.findNode(it.first);
        const Node& directionNode = model.mesh.findNode(it.second);
        VectorialValue direction(directionNode.x - constrainedNode.x,
                directionNode.y - constrainedNode.y, directionNode.z - constrainedNode.z);
        shared_ptr<GapParticipation> gp = make_shared<GapParticipation>(constrainedNode.position, direction.normalized());
        result.push_back(gp);
    }
    return result;
}

set<int> GapTwoNodes::nodePositions() const {
    set<int> result;
    for (const auto& it : directionNodePositionByconstrainedNodePosition) {
        result.insert(it.first);
    }
    return result;
}

void GapTwoNodes::removeNode(int nodePosition) {
    directionNodePositionByconstrainedNodePosition.erase(nodePosition);
}

bool GapTwoNodes::ineffective() const {
    return directionNodePositionByconstrainedNodePosition.size() == 0;
}

const DOFS GapTwoNodes::getDOFSForNode(int nodePosition) const {
    const auto& it = directionNodePositionByconstrainedNodePosition.find(nodePosition);
    DOFS dofs(DOFS::NO_DOFS);
    if (it != directionNodePositionByconstrainedNodePosition.end()) {
        const Node& constrainedNode = model.mesh.findNode(it->first);
        const Node& directionNode = model.mesh.findNode(it->second);
        VectorialValue direction(directionNode.x - constrainedNode.x,
                directionNode.y - constrainedNode.y, directionNode.z - constrainedNode.z);
        if (!is_zero(direction.x()))
            dofs += DOF::DX;
        if (!is_zero(direction.y()))
            dofs += DOF::DY;
        if (!is_zero(direction.z()))
            dofs += DOF::DZ;
    }
    return dofs;
}

GapNodeDirection::GapNodeDirection(Model& model, int original_id) :
        Gap(model, original_id) {
}

void GapNodeDirection::addGapNodeDirection(int constrainedNodeId, double directionX,
        double directionY, double directionZ) {
    int constrainedNodePosition = model.mesh.findOrReserveNode(constrainedNodeId);
    directionBynodePosition[constrainedNodePosition] = VectorialValue(directionX, directionY,
            directionZ);
}

vector<shared_ptr<Gap::GapParticipation>> GapNodeDirection::getGaps() const {
    vector<shared_ptr<Gap::GapParticipation>> result;
    result.reserve(directionBynodePosition.size());
    for (const auto& it : directionBynodePosition) {
        const int constrainedNodePosition = it.first;
        shared_ptr<GapParticipation> gp = make_shared<GapParticipation>(constrainedNodePosition, it.second.normalized());
        result.push_back(gp);
    }
    return result;
}

set<int> GapNodeDirection::nodePositions() const {
    set<int> result;
    for (const auto& it : directionBynodePosition) {
        result.insert(it.first);
    }
    return result;
}

void GapNodeDirection::removeNode(int nodePosition) {
    directionBynodePosition.erase(nodePosition);
}

bool GapNodeDirection::ineffective() const {
    return directionBynodePosition.size() == 0;
}

const DOFS GapNodeDirection::getDOFSForNode(int nodePosition) const {
    auto it = directionBynodePosition.find(nodePosition);
    DOFS dofs(DOFS::NO_DOFS);
    if (it != directionBynodePosition.end()) {
        VectorialValue direction = it->second;
        if (!is_zero(direction.x()))
            dofs += DOF::DX;
        if (!is_zero(direction.y()))
            dofs += DOF::DY;
        if (!is_zero(direction.z()))
            dofs += DOF::DZ;
    }
    return dofs;
}

SlideContact::SlideContact(Model& model, double friction, Reference<Target> master, Reference<Target> slave, int original_id) :
    Contact(model, Constraint::Type::SLIDE, original_id), friction{friction}, master{master}, slave{slave} {
}

SlideContact::SlideContact(Model& model, Reference<NamedValue> friction, Reference<Target> master, Reference<Target> slave, int original_id) :
    Contact(model, Constraint::Type::SLIDE, original_id), friction{friction}, master{master}, slave{slave} {
}

double SlideContact::getFriction() const {
    if (friction != ValueOrReference::EMPTY_VALUE) {
        if (friction.isReference()) {
            const auto val = dynamic_pointer_cast<const ScalarValue<double>>(model.find(friction.getReference()));
            return val->number;

        } else {
            return friction.getValue();
        }
    } else {
        return 0.0;
    }
}

set<int> SlideContact::nodePositions() const {
    set<int> result;
    const auto& masterLine = dynamic_pointer_cast<BoundaryNodeLine>(model.find(master));
    if (masterLine== nullptr) {
        throw logic_error("Cannot find master node list");
    }
    for (int nodeId : masterLine->nodeids) {
        result.insert(model.mesh.findNodePosition(nodeId));
    }
    const auto& slaveLine = dynamic_pointer_cast<BoundaryNodeLine>(model.find(slave));
    if (slaveLine== nullptr) {
        throw logic_error("Cannot find slave node list");
    }
    for (int nodeId : slaveLine->nodeids) {
        result.insert(model.mesh.findNodePosition(nodeId));
    }
    return result;
}

void SlideContact::removeNode(int nodePosition) {
    UNUSEDV(nodePosition);
    throw logic_error("Not yet implemented");
}

bool SlideContact::ineffective() const {
    const auto& masterLine = dynamic_pointer_cast<BoundaryNodeLine>(model.find(master));
    const auto& slaveLine = dynamic_pointer_cast<BoundaryNodeLine>(model.find(slave));
    return masterLine->nodeids.size() == 0 or slaveLine->nodeids.size() == 0;
}

const DOFS SlideContact::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::TRANSLATIONS;
}

SurfaceContact::SurfaceContact(Model& model, Reference<Target> master, Reference<Target> slave, int original_id) :
    Contact(model, Constraint::Type::SURFACE_CONTACT, original_id), master(master), slave(slave) {
}

set<int> SurfaceContact::nodePositions() const {
    set<int> result;
    const auto& masterSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(master));
    if (masterSurface== nullptr) {
        throw logic_error("Cannot find master node list");
    }
    for (int nodeId : masterSurface->nodeids) {
        result.insert(model.mesh.findNodePosition(nodeId));
    }
    const auto& slaveSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(slave));
    if (slaveSurface== nullptr) {
        throw logic_error("Cannot find slave node list");
    }
    for (int nodeId : slaveSurface->nodeids) {
        result.insert(model.mesh.findNodePosition(nodeId));
    }
    return result;
}

void SurfaceContact::removeNode(int nodePosition) {
    UNUSEDV(nodePosition);
    throw logic_error("Not yet implemented");
}

bool SurfaceContact::ineffective() const {
    const auto& masterSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(master));
    const auto& slaveSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(slave));
    return masterSurface->nodeids.size() == 0 or slaveSurface->nodeids.size() == 0;
}

const DOFS SurfaceContact::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::TRANSLATIONS;
}

void SurfaceContact::makeBoundarySurfaces() {
    shared_ptr<CellGroup> masterCellGroup = model.mesh.createCellGroup("SURFCONT_M_"+to_string(this->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
    shared_ptr<CellGroup> slaveCellGroup = model.mesh.createCellGroup("SURFCONT_S_"+to_string(this->bestId()), Group::NO_ORIGINAL_ID, "created by makeBoundarySurfaces() for the master in a SURFACE_CONTACT");
    shared_ptr<BoundaryNodeSurface> slaveNodeSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(this->slave));
    if (slaveNodeSurface == nullptr) {
        throw logic_error("Cannot find master node list");
    }
    const list<int>& slaveNodeIds = slaveNodeSurface->nodeids;
    auto it2 = slaveNodeIds.begin();
    for(unsigned int i = 0; i < slaveNodeIds.size();i+=4) {
        int nodeId1 = *(it2++);
        int nodeId2 = *(it2++);
        int nodeId3 = *(it2++);
        int nodeId4 = *(it2++);
        vector<int> connectivity {nodeId1, nodeId2, nodeId3};
        CellType cellType {CellType::TRI3};
        if (nodeId4 != 0) {
            connectivity.push_back(nodeId4);
            cellType = CellType::QUAD4;
        }
        int cellPosition = model.mesh.addCell(Cell::AUTO_ID, cellType, connectivity, true);
        slaveCellGroup->addCellPosition(cellPosition);
    }
    this->slaveCellGroup = slaveCellGroup;
    shared_ptr<BoundaryNodeSurface> masterNodeSurface = dynamic_pointer_cast<BoundaryNodeSurface>(model.find(this->master));
    if (masterNodeSurface == nullptr) {
        throw logic_error("Cannot find master node list");
    }
    const list<int>& masterNodeIds = masterNodeSurface->nodeids;
    auto it = masterNodeIds.begin();
    for(unsigned int i = 0; i < masterNodeIds.size();i+=4) {
        int nodeId1 = *(it++);
        int nodeId2 = *(it++);
        int nodeId3 = *(it++);
        int nodeId4 = *(it++);
        vector<int> connectivity {nodeId1, nodeId2, nodeId3};
        CellType cellType {CellType::TRI3};
        if (nodeId4 != 0) {
            connectivity.push_back(nodeId4);
            cellType = CellType::QUAD4;
        }
        int cellPosition = model.mesh.addCell(Cell::AUTO_ID, cellType, connectivity, true);
        masterCellGroup->addCellPosition(cellPosition);
    }
    this->masterCellGroup = masterCellGroup;
}

ZoneContact::ZoneContact(Model& model, Reference<Target> master, Reference<Target> slave, int original_id) :
    Contact(model, Constraint::Type::ZONE_CONTACT, original_id), master(master), slave(slave) {
}


set<int> ZoneContact::nodePositions() const {
    set<int> result;
    const auto& masterBody = dynamic_pointer_cast<ContactBody>(model.find(master));
    if (masterBody == nullptr) {
        throw logic_error("Cannot find master body");
    }
    const auto& masterBoundary = dynamic_pointer_cast<BoundarySurface>(model.find(masterBody->boundary));
    if (masterBoundary == nullptr) {
        throw logic_error("Cannot find master body boundary");
    }
    const auto& masterNodePositions = masterBoundary->nodePositions();
    result.insert(masterNodePositions.begin(), masterNodePositions.end());
    const auto& slaveBody = dynamic_pointer_cast<ContactBody>(model.find(slave));
    if (slaveBody == nullptr) {
        throw logic_error("Cannot find slave body");
    }
    const auto& slaveBoundary = dynamic_pointer_cast<BoundarySurface>(model.find(slaveBody->boundary));
    if (slaveBoundary == nullptr) {
        throw logic_error("Cannot find slave body boundary");
    }
    const auto& slaveNodePositions = slaveBoundary->nodePositions();
    result.insert(slaveNodePositions.begin(), slaveNodePositions.end());
    return result;
}

void ZoneContact::removeNode(int nodePosition) {
    UNUSEDV(nodePosition);
    throw logic_error("Not yet implemented");
}

bool ZoneContact::ineffective() const {
    const auto& masterBody = dynamic_pointer_cast<ContactBody>(model.find(master));
    if (masterBody == nullptr) {
        throw logic_error("Cannot find master body");
    }
    const auto& masterBoundary = dynamic_pointer_cast<BoundarySurface>(model.find(masterBody->boundary));
    if (masterBoundary == nullptr) {
        throw logic_error("Cannot find master body boundary");
    }
    const auto& slaveBody = dynamic_pointer_cast<ContactBody>(model.find(slave));
    if (slaveBody == nullptr) {
        throw logic_error("Cannot find slave body");
    }
    const auto& slaveBoundary = dynamic_pointer_cast<BoundarySurface>(model.find(slaveBody->boundary));
    if (slaveBoundary == nullptr) {
        throw logic_error("Cannot find slave body boundary");
    }
    return masterBoundary->empty() or slaveBoundary->empty();
}

const DOFS ZoneContact::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::TRANSLATIONS;
}

SurfaceSlide::SurfaceSlide(Model& model, Reference<Target> master, Reference<Target> slave, int original_id) :
    Constraint(model, Constraint::Type::SURFACE_SLIDE_CONTACT, original_id), master(master), slave(slave) {
}

set<int> SurfaceSlide::nodePositions() const {
    set<int> result;
    const auto& masterBoundary = dynamic_pointer_cast<BoundaryElementFace>(model.find(master));
    if (masterBoundary == nullptr) {
        throw logic_error("Cannot find master body boundary");
    }
    if (masterBoundary->surfaceCellGroup == nullptr) {
        throw logic_error("Cannot find master body cells");
    }
    const auto& masterNodePositions = masterBoundary->surfaceCellGroup->nodePositions();
    result.insert(masterNodePositions.begin(), masterNodePositions.end());
    const auto& slaveBoundary = dynamic_pointer_cast<BoundaryElementFace>(model.find(slave));
    if (slaveBoundary == nullptr) {
        throw logic_error("Cannot find slave body boundary");
    }
    if (slaveBoundary->surfaceCellGroup == nullptr) {
        throw logic_error("Cannot find slave body cells");
    }

    const auto& slaveNodePositions = slaveBoundary->surfaceCellGroup->nodePositions();
    result.insert(slaveNodePositions.begin(), slaveNodePositions.end());
    return result;
}

void SurfaceSlide::removeNode(int nodePosition) {
    UNUSEDV(nodePosition);
    throw logic_error("Not yet implemented");
}

bool SurfaceSlide::ineffective() const {
    const auto& masterBoundary = dynamic_pointer_cast<BoundaryElementFace>(model.find(master));
    if (masterBoundary == nullptr) {
        throw logic_error("Cannot find master body boundary (or unexpected type)");
    }
    const auto& slaveBoundary = dynamic_pointer_cast<BoundaryElementFace>(model.find(slave));
    if (slaveBoundary == nullptr) {
        throw logic_error("Cannot find slave body boundary (or unexpected type)");
    }
    return masterBoundary->faceInfos.size() == 0 or slaveBoundary->faceInfos.size() == 0;
}

const DOFS SurfaceSlide::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::TRANSLATIONS;
}

void SurfaceSlide::makeCellsFromSurfaceSlide() const {
    namespace bg = boost::geometry;
    namespace bgi = boost::geometry::index;
    using node_point = bg::model::point<double, 3, bg::cs::cartesian>;
    using node_polygon = bg::model::polygon<node_point >;
    using node_box = bg::model::box<node_point>;
    using value = pair<node_box, size_t>;

    shared_ptr<CellGroup> group = model.mesh.createCellGroup("SURF_"+to_string(this->bestId()), CellGroup::NO_ORIGINAL_ID, "SURF");
    const auto& elementsetSlide = make_shared<SurfaceSlideSet>(model);
    elementsetSlide->assignCellGroup(group);
    model.add(elementsetSlide);

    shared_ptr<const BoundaryElementFace> masterSurface = dynamic_pointer_cast<const BoundaryElementFace>(model.find(this->master));
    vector<node_polygon> masterFaces;
    vector<vector<int>> masterFaceNodeIds;
    bgi::rtree< value, bgi::rstar<16> > rtree;
    for (const auto& faceInfo : masterSurface->faceInfos) {
        const Cell& masterCell = model.mesh.findCell(model.mesh.findCellPosition(faceInfo.cellId));
        const vector<int>& faceIds = masterCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
        if (faceIds.size() > 0) {
            node_polygon masterFace;
            for (const int faceId : faceIds) {
                int nodePosition = model.mesh.findNodePosition(faceId);
                const Node& node = model.mesh.findNode(nodePosition);
                const node_point np{node.x, node.y, node.z};
                masterFace.outer().push_back(np);
            }
            masterFaces.push_back(masterFace);
            masterFaceNodeIds.push_back(faceIds);
            for (size_t i = 0 ; i < masterFaces.size() ; ++i) {
                node_box b = bg::return_envelope<node_box>(masterFaces[i]);
                rtree.insert(std::make_pair(b, i));
            }
        }
    }
    shared_ptr<const BoundaryElementFace> slaveSurface = dynamic_pointer_cast<const BoundaryElementFace>(model.find(this->slave));
    for (const auto& faceInfo : slaveSurface->faceInfos) {
        const Cell& slaveCell = model.mesh.findCell(model.mesh.findCellPosition(faceInfo.cellId));
        const vector<int>& faceIds = slaveCell.faceids_from_two_nodes(faceInfo.nodeid1, faceInfo.nodeid2);
        for (const int faceId : faceIds) {
            int nodePosition = model.mesh.findNodePosition(faceId);
            const Node& node = model.mesh.findNode(nodePosition);
            vector<value> result_n;
            rtree.query(bgi::nearest(node_point{node.x, node.y, node.z}, 1), std::back_inserter(result_n));
            size_t masterFaceIndex = result_n[0].second;
            const node_polygon& masterFace = masterFaces[masterFaceIndex];
            vector<int> contactNodeIds = masterFaceNodeIds[masterFaceIndex];
            contactNodeIds.push_back(faceId);
            size_t num_master_points = bg::num_points(masterFace);
            switch(num_master_points) {
            case 3: {
                int cellPosition = model.mesh.addCell(Cell::AUTO_ID, CellType::TETRA4, contactNodeIds , true);
                group->addCellPosition(cellPosition);
                break;
            }
            case 4: {
                int cellPosition = model.mesh.addCell(Cell::AUTO_ID, CellType::PYRA5, contactNodeIds , true);
                group->addCellPosition(cellPosition);
                break;
            }
            default:
                throw logic_error("Slide element not yet implemented:" + to_string(num_master_points));
            }

        }
    }
}

} // namespace vega
