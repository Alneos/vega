/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Constraint.cpp
 *
 *  Created on: 2 juin 2014
 *      Author: siavelis
 */

#include "Constraint.h"
#include "Model.h"
#include <ciso646>

namespace vega {

using namespace std;

Constraint::Constraint(const Model& model, Type type, int original_id) :
		Identifiable(original_id), model(model), type(type) {
}

const string Constraint::name = "Constraint";

const map<Constraint::Type, string> Constraint::stringByType = {
		{ QUASI_RIGID, "QUASI_RIGID" },
		{ RIGID, "RIGID" },
		{ SPC, "SPC" },
		{ RBE3, "RBE3" },
		{ GAP, "GAP" },
		{ LMPC, "LMPC" }
};

ostream &operator<<(ostream &out, const Constraint& constraint) {
	out << to_str(constraint);
	return out;
}

ConstraintSet::ConstraintSet(const Model& model, Type type, int original_id) :
		Identifiable(original_id), model(model), type(type) {
}

void ConstraintSet::add(const Reference<ConstraintSet>& constraintSetReference) {
	constraintSetReferences.push_back(constraintSetReference);
}

const set<shared_ptr<Constraint> > ConstraintSet::getConstraints() const {
	set<shared_ptr<Constraint>> result = model.getConstraintsByConstraintSet(this->getReference());
	for (auto constraintSetReference : constraintSetReferences) {
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
	return (int) getConstraints().size();
}

const string ConstraintSet::name = "ConstraintSet";

const map<ConstraintSet::Type, string> ConstraintSet::stringByType = { { SPC, "SPC" },
		{ MPC, "MPC" }, { SPCD, "SPCD" }, { ALL, "ALL" } };

ostream &operator<<(ostream &out, const ConstraintSet& constraintSet) {
	out << to_str(constraintSet);
	return out;
}

shared_ptr<ConstraintSet> ConstraintSet::clone() const {
	return shared_ptr<ConstraintSet>(new ConstraintSet(*this));
}

ConstraintSet::~ConstraintSet() {
}

const int HomogeneousConstraint::UNAVAILABLE_MASTER = INT_MIN;

HomogeneousConstraint::HomogeneousConstraint(Model& model, Type type, const DOFS& dofs,
		int masterId, int original_id, const set<int>& slaveIds) :
		Constraint(model, type, original_id), dofs(dofs), //

		slavePositions(model.mesh->findOrReserveNodes(slaveIds) //
				) {
	if (masterId != UNAVAILABLE_MASTER) {
		this->masterPosition = model.mesh->findOrReserveNode(masterId);
	} else {
		this->masterPosition = UNAVAILABLE_MASTER;
	}
}

int HomogeneousConstraint::getMaster() const {
	if (this->masterPosition != HomogeneousConstraint::UNAVAILABLE_MASTER) {
		return this->masterPosition;
	} else {
		throw logic_error("getMaster: automatic resolution of master id not implemented");
	}
}

void HomogeneousConstraint::addSlave(int slaveId) {
	this->slavePositions.insert(model.mesh->findOrReserveNode(slaveId));
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
		throw logic_error("getMaster: automatic resolution of master id not implemented");
	}
}

HomogeneousConstraint::~HomogeneousConstraint() {
}

QuasiRigidConstraint::QuasiRigidConstraint(Model& model, const DOFS& dofs, int masterId,
		int constraintGroup, const set<int>& slaveIds) :
		HomogeneousConstraint(model, QUASI_RIGID, dofs, masterId, constraintGroup, slaveIds) {

}
shared_ptr<Constraint> QuasiRigidConstraint::clone() const {
	return shared_ptr<Constraint>(new QuasiRigidConstraint(*this));
}

set<int> QuasiRigidConstraint::getSlaves() const {
	return this->slavePositions;
}



RigidConstraint::RigidConstraint(Model& model, int masterId, int constraintGroup,
		const set<int>& slaveIds) :
		HomogeneousConstraint(model, RIGID, DOFS::ALL_DOFS, masterId, constraintGroup, slaveIds) {
}

shared_ptr<Constraint> RigidConstraint::clone() const {
	return shared_ptr<Constraint>(new RigidConstraint(*this));
}

RBE3::RBE3(Model& model, int masterId, const DOFS dofs, int original_id) :
		HomogeneousConstraint(model, Constraint::RBE3, dofs, masterId, original_id) {
}

void RBE3::addSlave(int slaveId, DOFS slaveDOFS, double slaveCoef) {
	int nodePosition = model.mesh->findOrReserveNode(slaveId);
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

shared_ptr<Constraint> RBE3::clone() const {
	return shared_ptr<Constraint>(new RBE3(*this));
}

const ValueOrReference& SinglePointConstraint::NO_SPC = ValueOrReference::EMPTY_VALUE;

SinglePointConstraint::SinglePointConstraint(const Model& _model,
		const array<ValueOrReference, 6>& _spcs, Group* _group, int original_id) :
		Constraint(_model, SPC, original_id), spcs(_spcs), group(_group) {
}

SinglePointConstraint::SinglePointConstraint(const Model& _model,
		const array<ValueOrReference, 3>& _spcs, Group* _group, int original_id) :
		Constraint(_model, SPC, original_id), spcs( { { NO_SPC, NO_SPC, NO_SPC, NO_SPC, NO_SPC,
				NO_SPC } }), group(_group) {
	copy_n(_spcs.begin(), 3, spcs.begin());
}

SinglePointConstraint::SinglePointConstraint(const Model& _model, DOFS dofs, double value, Group* _group,
		int original_id) :
		Constraint(_model, SPC, original_id), spcs( { { NO_SPC, NO_SPC, NO_SPC, NO_SPC, NO_SPC,
				NO_SPC } }), group(_group) {
	for (DOF dof : dofs) {
		setDOF(dof, value);
	}
}

SinglePointConstraint::SinglePointConstraint(const Model& _model, Group* _group, int original_id) :
		Constraint(_model, SPC, original_id), group(_group) {
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
	int nodePosition = model.mesh->findOrReserveNode(nodeId);
	if (group == nullptr) {
		_nodePositions.insert(nodePosition);
	} else {
		if (group->type == Group::NODEGROUP) {
			NodeGroup* const ngroup = static_cast<NodeGroup* const >(group);
			ngroup->addNodeByPosition(nodePosition);
		} else {
			throw logic_error("SPC:: addNodeId on unknown group type");
		}
	}
}

shared_ptr<Constraint> SinglePointConstraint::clone()
const {
	return shared_ptr<Constraint>(new SinglePointConstraint(*this));
}

set<int> SinglePointConstraint::nodePositions() const {
	set<int> result;
	result.insert(_nodePositions.begin(), _nodePositions.end());
	if (group != nullptr) {
		if (this->group->type == Group::NODEGROUP) {
			NodeGroup* const ngroup = static_cast<NodeGroup* const >(group);
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
		if (group->type == Group::NODEGROUP) {
			NodeGroup* const ngroup = static_cast<NodeGroup* const >(group);
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

LinearMultiplePointConstraint::LinearMultiplePointConstraint(Model& model, double coef_impo,
		int original_id) :
		Constraint(model, Constraint::LMPC, original_id), coef_impo(coef_impo) {
}

shared_ptr<Constraint> LinearMultiplePointConstraint::clone() const {
	return shared_ptr<Constraint>(new LinearMultiplePointConstraint(*this));
}

void LinearMultiplePointConstraint::addParticipation(int nodeId, double dx, double dy, double dz,
		double rx, double ry, double rz) {
	int nodePosition = model.mesh->findOrReserveNode(nodeId);
	auto it = dofCoefsByNodePosition.find(nodePosition);
	if (it == dofCoefsByNodePosition.end())
		dofCoefsByNodePosition[nodePosition] = DofCoefs(dx, dy, dz, rx, ry, rz);
	else
		dofCoefsByNodePosition[nodePosition] += DofCoefs(dx, dy, dz, rx, ry, rz);
}

set<int> LinearMultiplePointConstraint::nodePositions() const {
	set<int> result;
	for (auto it : dofCoefsByNodePosition) {
		result.insert(it.first);
	}
	return result;
}

const DOFS LinearMultiplePointConstraint::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	auto it = dofCoefsByNodePosition.find(nodePosition);
	if (it != dofCoefsByNodePosition.end()) {
		DofCoefs dofCoefs = it->second;
		if (!is_zero(dofCoefs[0]))
			dofs = dofs + DOF::DX;
		if (!is_zero(dofCoefs[1]))
			dofs = dofs + DOF::DY;
		if (!is_zero(dofCoefs[2]))
			dofs = dofs + DOF::DZ;
		if (!is_zero(dofCoefs[3]))
			dofs = dofs + DOF::RX;
		if (!is_zero(dofCoefs[4]))
			dofs = dofs + DOF::RY;
		if (!is_zero(dofCoefs[5]))
			dofs = dofs + DOF::RZ;
	}
	return dofs;
}

void LinearMultiplePointConstraint::removeNode(int nodePosition) {
	dofCoefsByNodePosition.erase(nodePosition);
}

bool LinearMultiplePointConstraint::ineffective() const  {
	return dofCoefsByNodePosition.size() == 0;
}

LinearMultiplePointConstraint::DofCoefs LinearMultiplePointConstraint::getDoFCoefsForNode(
		int nodePosition) const {
	DofCoefs dofCoefs(0, 0, 0, 0, 0, 0);
	auto it = dofCoefsByNodePosition.find(nodePosition);
	if (it != dofCoefsByNodePosition.end())
		dofCoefs = it->second;
	return dofCoefs;
}

Gap::Gap(Model& model, int original_id) :
		Constraint(model, Constraint::GAP, original_id) {
}

Gap::~Gap() {
}

GapTwoNodes::GapTwoNodes(Model& model, int original_id) :
		Gap(model, original_id) {
}

shared_ptr<Constraint> GapTwoNodes::clone() const {
	return shared_ptr<Constraint>(new GapTwoNodes(*this));
}

void GapTwoNodes::addGapNodes(int constrainedNodeId, int directionNodeId) {
	int constrainedNodePosition = model.mesh->findOrReserveNode(constrainedNodeId);
	int directionNodePosition = model.mesh->findOrReserveNode(directionNodeId);
	directionNodePositionByconstrainedNodePosition[constrainedNodePosition] = directionNodePosition;
}

vector<shared_ptr<Gap::GapParticipation>> GapTwoNodes::getGaps() const {
	vector<shared_ptr<Gap::GapParticipation>> result;
	result.reserve(directionNodePositionByconstrainedNodePosition.size());
	for (auto it : directionNodePositionByconstrainedNodePosition) {
		Node constrainedNode = model.mesh->findNode(it.first);
		Node directionNode = model.mesh->findNode(it.second);
		VectorialValue direction(directionNode.x - constrainedNode.x,
				directionNode.y - constrainedNode.y, directionNode.z - constrainedNode.z);
		shared_ptr<GapParticipation> gp(
				new GapParticipation(constrainedNode.position, direction.normalized()));
		result.push_back(gp);
	}
	return result;
}

set<int> GapTwoNodes::nodePositions() const {
	set<int> result;
	for (auto it : directionNodePositionByconstrainedNodePosition) {
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
	auto it = directionNodePositionByconstrainedNodePosition.find(nodePosition);
	DOFS dofs(DOFS::NO_DOFS);
	if (it != directionNodePositionByconstrainedNodePosition.end()) {
		Node constrainedNode = model.mesh->findNode(it->first);
		Node directionNode = model.mesh->findNode(it->second);
		VectorialValue direction(directionNode.x - constrainedNode.x,
				directionNode.y - constrainedNode.y, directionNode.z - constrainedNode.z);
		if (!is_zero(direction.x()))
			dofs = dofs + DOF::DX;
		if (!is_zero(direction.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(direction.z()))
			dofs = dofs + DOF::DZ;
	}
	return dofs;
}

GapNodeDirection::GapNodeDirection(Model& model, int original_id) :
		Gap(model, original_id) {
}

shared_ptr<Constraint> GapNodeDirection::clone() const {
	return shared_ptr<Constraint>(new GapNodeDirection(*this));
}

void GapNodeDirection::addGapNodeDirection(int constrainedNodeId, double directionX,
		double directionY, double directionZ) {
	int constrainedNodePosition = model.mesh->findOrReserveNode(constrainedNodeId);
	directionBynodePosition[constrainedNodePosition] = VectorialValue(directionX, directionY,
			directionZ);
}

vector<shared_ptr<Gap::GapParticipation>> GapNodeDirection::getGaps() const {
	vector<shared_ptr<Gap::GapParticipation>> result;
	result.reserve(directionBynodePosition.size());
	for (auto it : directionBynodePosition) {
		Node constrainedNode = model.mesh->findNode(it.first);
		shared_ptr<GapParticipation> gp(
				new GapParticipation(constrainedNode.position, it.second.normalized()));
		result.push_back(gp);
	}
	return result;
}

set<int> GapNodeDirection::nodePositions() const {
	set<int> result;
	for (auto it : directionBynodePosition) {
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
			dofs = dofs + DOF::DX;
		if (!is_zero(direction.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(direction.z()))
			dofs = dofs + DOF::DZ;
	}
	return dofs;
}

} // namespace vega
