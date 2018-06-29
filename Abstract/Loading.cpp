/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Loading.cpp
 *
 *  Created on: Aug 24, 2013
 *      Author: devel
 */

#include "Loading.h"
#include "Model.h"
#include <boost/lexical_cast.hpp>
//if with "or" and "and" under windows
#include <ciso646>

namespace vega {

Loading::Loading(const Model& model, Loading::Type type, Loading::ApplicationType applicationType,
		const int original_id, int coordinate_system_id) :
		Identifiable(original_id), model(model), type(type), applicationType(applicationType), coordinateSystem_reference(
				Reference<CoordinateSystem>(CoordinateSystem::UNKNOWN, coordinate_system_id)) {
}

const string Loading::name = "Loading";

const map<Loading::Type, string> Loading::stringByType = {
		{ NODAL_FORCE, "NODAL_FORCE" },
		{ GRAVITY, "GRAVITY" },
		{ ROTATION, "ROTATION" },
		{ NORMAL_PRESSION_FACE, "NORMAL_PRESSION_FACE" },
		{ FORCE_LINE, "FORCE_LINE" },
		{ FORCE_SURFACE, "FORCE_SURFACE" },
		{ DYNAMIC_EXCITATION, "DYNAMIC_EXCITATION" }
};

ostream &operator<<(ostream &out, const Loading& loading) {
	out << to_str(loading);
	return out;
}

bool Loading::validate() const {
	bool valid = true;
	if (hasCoordinateSystem()) {
		//FIXME: GC? i don't understand: previously was valid = model.find(coordinateSystem_reference)
		valid = model.find(coordinateSystem_reference) != nullptr;
		if (!valid) {
			cerr
			<< string("Coordinate system id:")
					+ boost::lexical_cast<string>(coordinateSystem_reference.original_id)
					+ " for loading " << *this << " not found." << endl;
		}
	}
	return valid;
}

LoadSet::LoadSet(const Model& model, Type type, int original_id) :
		Identifiable(original_id), model(model), type(type) {
}

const string LoadSet::name = "LoadSet";

const map<LoadSet::Type, string> LoadSet::stringByType = { { LOAD, "LOAD" }, { DLOAD, "DLOAD" }, {
		EXCITEID, "EXCITEID" }, { ALL, "ALL" } };

ostream &operator<<(ostream &out, const LoadSet& loadset) {
	out << to_str(loadset);
	return out;
}

int LoadSet::size() const {
	return (int) getLoadings().size();
}

//bool LoadSet::operator<(const LoadSet &rhs) const {
//	return (original_id < rhs.original_id);
//}

const set<shared_ptr<Loading> > LoadSet::getLoadings() const {
	set<shared_ptr<Loading>> result = model.getLoadingsByLoadSet(this->getReference());
	//for (auto& kv : this->coefficient_by_loadset) {
	//	set<shared_ptr<Loading>> setToInsert = model.getLoadingsByLoadSet(kv.first);
	//	result.insert(setToInsert.begin(), setToInsert.end());
	//}
	return result;
}

const set<shared_ptr<Loading> > LoadSet::getLoadingsByType(Loading::Type loadingType) const {
	set<shared_ptr<Loading> > result;
	for (shared_ptr<Loading> loading : getLoadings()) {
		if (loading->type == loadingType) {
			result.insert(loading);
		}
	}
	return result;
}

bool LoadSet::validate() const {
	set<shared_ptr<Loading>> loadings = getLoadings();
	if (loadings.size() == 0 || loadings.find(0) != loadings.end())
		return false;
	return true;
}

shared_ptr<LoadSet> LoadSet::clone() const {
	return shared_ptr<LoadSet>(new LoadSet(*this));
}

NodeLoading::NodeLoading(const Model& model, Loading::Type type, int original_id,
		int coordinateSystemId) :
		Loading(model, type, Loading::NODE, original_id, coordinateSystemId), NodeContainer(model.mesh) {
}

set<int> NodeLoading::nodePositions() const {
	return NodeContainer::nodePositions();
}

Gravity::Gravity(const Model& model, double acceleration, const VectorialValue& direction,
		const int original_id) :
		Loading(model, GRAVITY, NONE, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), acceleration(
				acceleration), direction(direction) {
}

const DOFS Gravity::getDOFSForNode(int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::NO_DOFS;
}
set<int> Gravity::nodePositions() const {
	return set<int>();
}

const VectorialValue Gravity::getDirection() const {
	return direction;
}

const VectorialValue Gravity::getAccelerationVector() const {
	return direction.scaled(acceleration);
}

double Gravity::getAcceleration() const {
	double mass_multiplier = 1;
	auto it = model.parameters.find(Model::MASS_OVER_FORCE_MULTIPLIER);
	if (it != model.parameters.end()) {
		mass_multiplier = it->second;
		assert(!is_zero(it->second));
	}
	return acceleration / mass_multiplier;
}

double Gravity::getAccelerationScale() const {
	return acceleration;
}

shared_ptr<Loading> Gravity::clone() const {
	return shared_ptr<Loading>(new Gravity(*this));
}

void Gravity::scale(double factor) {
	acceleration *= factor;
}

bool Gravity::ineffective() const {
	return is_zero(acceleration) or direction.iszero();
}

Rotation::Rotation(const Model& model, const int original_id) :
		Loading(model, ROTATION, NONE, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
}

const DOFS Rotation::getDOFSForNode(int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::NO_DOFS;
}

set<int> Rotation::nodePositions() const {
	return set<int>();
}

bool Rotation::ineffective() const {
	return is_zero(getSpeed()) or getAxis().iszero();
}

RotationCenter::RotationCenter(const Model& model, double speed, double center_x, double center_y,
		double center_z, double axis_x, double axis_y, double axis_z, const int original_id) :
		Rotation(model, original_id), speed(speed), axis(axis_x, axis_y, axis_z), center(center_x,
				center_y, center_z) {
}

double RotationCenter::getSpeed() const {
	return speed;
}

const VectorialValue RotationCenter::getAxis() const {
	return axis;
}

const VectorialValue RotationCenter::getCenter() const {
	return center;
}

shared_ptr<Loading> RotationCenter::clone() const {
	return shared_ptr<Loading>(new RotationCenter(*this));
}

void RotationCenter::scale(double factor) {
	speed *= factor;
}

RotationNode::RotationNode(const Model& model, double speed, const int node_id, double axis_x,
		double axis_y, double axis_z, const int original_id) :
		Rotation(model, original_id), speed(speed), axis(axis_x, axis_y, axis_z), node_position(
				model.mesh->findOrReserveNode(node_id)) {
}

double RotationNode::getSpeed() const {
	return speed;
}

const VectorialValue RotationNode::getAxis() const {
	return axis;
}

const VectorialValue RotationNode::getCenter() const {
	Node node = model.mesh->findNode(node_position, true, &model);
	return VectorialValue(node.x, node.y, node.z);
}

shared_ptr<Loading> RotationNode::clone() const {
	return shared_ptr<Loading>(new RotationNode(*this));
}

void RotationNode::scale(double factor) {
	speed *= factor;
}

NodalForce::NodalForce(const Model& model, const int original_id,
		int coordinate_system_id) :
		NodeLoading(model, NODAL_FORCE, original_id, coordinate_system_id), force(VectorialValue(0, 0, 0)), moment(
				VectorialValue(0, 0, 0)) {
}

NodalForce::NodalForce(const Model& model, const VectorialValue& force,
		const VectorialValue& moment, const int original_id, int coordinate_system_id) :
		NodeLoading(model, NODAL_FORCE, original_id, coordinate_system_id), force(force), moment(moment) {
}

NodalForce::NodalForce(const Model& model, double fx, double fy, double fz, double mx,
		double my, double mz, const int original_id, int coordinate_system_id) :
		NodeLoading(model, NODAL_FORCE, original_id, coordinate_system_id), force(fx, fy, fz), moment(mx, my, mz) {
}

const VectorialValue NodalForce::localToGlobal(int nodePosition, const VectorialValue& vectorialValue) const {
	if (!hasCoordinateSystem())
		return vectorialValue;
	shared_ptr<CoordinateSystem> coordSystem = model.find(coordinateSystem_reference);
	if (!coordSystem) {
		ostringstream oss;
		oss << "Coordinate system id: " << coordinateSystem_reference.original_id
				<< " for nodal force not found." << endl;
		throw logic_error(oss.str());
	}
	Node node = model.mesh->findNode(nodePosition, true, &model);
	node.buildGlobalXYZ(&model);
	coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
	return coordSystem->vectorToGlobal(vectorialValue);
}

const VectorialValue NodalForce::getForceInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end())
        throw logic_error("Requested node has not been assigned to this loading");
	return localToGlobal(nodePosition, force);
}

const VectorialValue NodalForce::getMomentInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end())
        throw logic_error("Requested node has not been assigned to this loading");
	return localToGlobal(nodePosition, moment);
}

const DOFS NodalForce::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) != posSet.end()) {
		if (!is_zero(force.x()))
			dofs = dofs + DOF::DX;
		if (!is_zero(force.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(force.z()))
			dofs = dofs + DOF::DZ;
		if (!is_zero(moment.x()))
			dofs = dofs + DOF::RX;
		if (!is_zero(moment.y()))
			dofs = dofs + DOF::RY;
		if (!is_zero(moment.z()))
			dofs = dofs + DOF::RZ;
	}
	return dofs;
}

shared_ptr<Loading> NodalForce::clone() const {
	return shared_ptr<Loading>(new NodalForce(*this));
}

void NodalForce::scale(double factor) {
	force.scale(factor);
	moment.scale(factor);
}

bool NodalForce::ineffective() const {
	return force.iszero() and moment.iszero();
}

NodalForceTwoNodes::NodalForceTwoNodes(const Model& model, const int node1_id,
		const int node2_id, double magnitude, const int original_id) :
		NodalForce(model, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), node_position1(
				model.mesh->findOrReserveNode(node1_id)), node_position2(
				model.mesh->findOrReserveNode(node2_id)), magnitude(magnitude) {
}

const VectorialValue NodalForceTwoNodes::getForceInGlobalCS(int nodePosition) const {
	Node node1 = model.mesh->findNode(node_position1, true, &model);
	Node node2 = model.mesh->findNode(node_position2, true, &model);
	VectorialValue direction = (VectorialValue(node2.x, node2.y, node2.z)
			- VectorialValue(node1.x, node1.y, node1.z)).normalized();
	return localToGlobal(nodePosition, magnitude * direction);
}

shared_ptr<Loading> NodalForceTwoNodes::clone() const {
	return make_shared<NodalForceTwoNodes>(*this);
}

void NodalForceTwoNodes::scale(double factor) {
	magnitude *= factor;
}

bool NodalForceTwoNodes::ineffective() const {
	return is_zero(magnitude) or force.iszero();
}

NodalForceFourNodes::NodalForceFourNodes(const Model& model, const int node1_id,
        const int node2_id, const int node3_id, const int node4_id, double magnitude, const int original_id) :
        NodalForce(model, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID),
                node_position1(model.mesh->findOrReserveNode(node1_id)),
                node_position2(model.mesh->findOrReserveNode(node2_id)),
                node_position3(model.mesh->findOrReserveNode(node3_id)),
                node_position4(model.mesh->findOrReserveNode(node4_id)), magnitude(magnitude) {
}

const VectorialValue NodalForceFourNodes::getForceInGlobalCS(int nodePosition) const {
    Node node1 = model.mesh->findNode(node_position1, true, &model);
    Node node2 = model.mesh->findNode(node_position2, true, &model);
    Node node3 = model.mesh->findNode(node_position3, true, &model);
    Node node4 = model.mesh->findNode(node_position4, true, &model);
    VectorialValue v1 = VectorialValue(node2.x, node2.y, node2.z) - VectorialValue(node1.x, node1.y, node1.z);
    VectorialValue v2 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node3.x, node3.y, node3.z);
    VectorialValue direction = v1.cross(v2).normalized();
    return localToGlobal(nodePosition, magnitude * direction);
}

shared_ptr<Loading> NodalForceFourNodes::clone() const {
    return make_shared<NodalForceFourNodes>(*this);
}

void NodalForceFourNodes::scale(double factor) {
    magnitude *= factor;
}

bool NodalForceFourNodes::ineffective() const {
    return is_zero(magnitude) or force.iszero();
}

ElementLoading::ElementLoading(const Model& model, Loading::Type type, int original_id,
		int coordinateSystemId) :
		Loading(model, type, Loading::ELEMENT, original_id, coordinateSystemId), CellContainer(model.mesh) {
}

set<int> ElementLoading::nodePositions() const {
	return CellContainer::nodePositions();
}

bool ElementLoading::cellDimensionGreatherThan(SpaceDimension dimension) {
	vector<Cell> cells = this->getCells(true);
	bool result = false;
	for (Cell& cell : cells) {
		result = result or cell.type.dimension > dimension;
		if (result)
			break;
	}
	return result;

}

bool ElementLoading::appliedToGeometry() {
	bool isForceOnPoutre = false;
	bool assigned = false;
	vector<Cell> cells = getCells(true);
	for (Cell cell : cells) {
		int element_id = cell.elementId;
		if (element_id != Cell::UNAVAILABLE_CELL) {
			shared_ptr<ElementSet> element = model.find(
					Reference<ElementSet>(ElementSet::UNKNOWN, Reference<ElementSet>::NO_ID,
							element_id));
			if (element) {
				if (element->type >= ElementSet::CIRCULAR_SECTION_BEAM
						&& element->type <= ElementSet::GENERIC_SECTION_BEAM) {
					if (assigned && !isForceOnPoutre) {
						cerr << "Can't calculate the element type assigned to the load " << *this
								<< endl;
						return !isForceOnPoutre;
					} else {
						assigned = true;
						isForceOnPoutre = true;
					}
				} else {
					if (assigned && isForceOnPoutre) {
						cerr << "Can't calculate the element type assigned to the load " << *this
								<< endl;
						return !isForceOnPoutre;
					} else {
						assigned = true;
						isForceOnPoutre = false;
					}
				}
			}
		}
	}
	return !isForceOnPoutre;
}

ForceSurface::ForceSurface(const Model& model, const VectorialValue& force,
		const VectorialValue& moment, const int original_id) :
		ElementLoading(model, Loading::FORCE_SURFACE, original_id,
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), force(force), moment(moment) {
}

const VectorialValue ForceSurface::getForce() const {
	return force;
}

const VectorialValue ForceSurface::getMoment() const {
	return moment;
}

const DOFS ForceSurface::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		if (!is_zero(force.x()))
			dofs = dofs + DOF::DX;
		if (!is_zero(force.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(force.z()))
			dofs = dofs + DOF::DZ;
		if (!is_zero(moment.x()))
			dofs = dofs + DOF::RX;
		if (!is_zero(moment.y()))
			dofs = dofs + DOF::RY;
		if (!is_zero(moment.z()))
			dofs = dofs + DOF::RZ;
	}
	return dofs;
}

shared_ptr<Loading> ForceSurface::clone() const {
	return make_shared<ForceSurface>(*this);
}

void ForceSurface::scale(double factor) {
	force.scale(factor);
	moment.scale(factor);
}

bool ForceSurface::ineffective() const {
	return force.iszero() and moment.iszero();
}

bool ForceSurface::validate() const {
	return true;
}

PressionFaceTwoNodes::PressionFaceTwoNodes(const Model& model, int nodeId1, int nodeId2,
		const VectorialValue& force, const VectorialValue& moment, const int original_id) :
		ForceSurface(model, force, moment, original_id), nodePosition1(
				model.mesh->findOrReserveNode(nodeId1)), nodePosition2(
				model.mesh->findOrReserveNode(nodeId2)) {
}

vector<int> PressionFaceTwoNodes::getApplicationFace() const {
	vector<Cell> cells = getCells();
	if (cells.size() != 1) {
		throw logic_error("More than one cell specified for a PressionFaceTwoNodes");
	}
	Node node1 = model.mesh->findNode(nodePosition1);
	Node node2 = model.mesh->findNode(nodePosition2);
	vector<int> nodeIds = cells[0].faceids_from_two_nodes(node1.id, node2.id);
	return nodeIds;
}

ForceLine::ForceLine(const Model& model, const VectorialValue& force, const VectorialValue& torque,
		const int original_id) :
		ElementLoading(model, FORCE_LINE, original_id), force(force), torque(torque) {

}

const DOFS ForceLine::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		if (!is_zero(force.x()))
			dofs = dofs + DOF::DX;
		if (!is_zero(force.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(force.z()))
			dofs = dofs + DOF::DZ;
		if (!is_zero(torque.x()))
			dofs = dofs + DOF::RX;
		if (!is_zero(torque.y()))
			dofs = dofs + DOF::RY;
		if (!is_zero(torque.z()))
			dofs = dofs + DOF::RZ;
	}
	return dofs;
}

shared_ptr<Loading> ForceLine::clone() const {
	return shared_ptr<Loading>(new ForceLine(*this));
}

void ForceLine::scale(double factor) {
	force.scale(factor);
	torque.scale(factor);
}

bool ForceLine::ineffective() const {
	return force.iszero() and torque.iszero();
}

bool ForceLine::validate() const {
	return true;
}

ForceLineComponent::ForceLineComponent(const Model& model, const shared_ptr<NamedValue> force, DOF dof,
			const int original_id) :
		ElementLoading(model, FORCE_LINE_COMPONENT, original_id), force(force), dof(dof) {

}

const DOFS ForceLineComponent::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		if (!force->iszero())
			dofs = dof;
	}
	return dofs;
}

shared_ptr<Loading> ForceLineComponent::clone() const {
	return shared_ptr<Loading>(new ForceLineComponent(*this));
}

void ForceLineComponent::scale(double factor) {
	force->scale(factor);
}

bool ForceLineComponent::ineffective() const {
	return force->iszero();
}

bool ForceLineComponent::validate() const {
	return true;
}

shared_ptr<Loading> PressionFaceTwoNodes::clone() const {
	return shared_ptr<Loading>(new PressionFaceTwoNodes(*this));
}

NormalPressionFace::NormalPressionFace(const Model& model, double intensity, const int original_id) :
		ElementLoading(model, NORMAL_PRESSION_FACE, original_id,
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), intensity(intensity) {
}

const DOFS NormalPressionFace::getDOFSForNode(int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		dofs += DOFS::ALL_DOFS;
	}
	return dofs;
}

shared_ptr<Loading> NormalPressionFace::clone() const {
	return shared_ptr<Loading>(new NormalPressionFace(*this));
}

void NormalPressionFace::scale(double factor) {
	intensity *= factor;
}

bool NormalPressionFace::ineffective() const {
	return is_zero(intensity);
}

bool NormalPressionFace::validate() const {
	// TODO validate : Check that all the elements are 2D
	return true;
}

DynamicExcitation::DynamicExcitation(const Model& model, const Reference<NamedValue> dynaDelay, const Reference<NamedValue> dynaPhase,
        const Reference<NamedValue> functionTableB, const Reference<NamedValue> functionTableP, const Reference<LoadSet> loadSet,
        const int original_id) :
                Loading(model, DYNAMIC_EXCITATION, NONE, original_id), dynaDelay(dynaDelay), dynaPhase(dynaPhase),
                functionTableB(functionTableB), functionTableP(functionTableP), loadSet(loadSet) {
}

shared_ptr<DynaPhase> DynamicExcitation::getDynaDelay() const {
    return dynamic_pointer_cast<DynaPhase>(model.find(dynaDelay));
}

shared_ptr<DynaPhase> DynamicExcitation::getDynaPhase() const {
    return dynamic_pointer_cast<DynaPhase>(model.find(dynaPhase));
}

shared_ptr<FunctionTable> DynamicExcitation::getFunctionTableB() const {
    return dynamic_pointer_cast<FunctionTable>(model.find(functionTableB));
}

shared_ptr<FunctionTable> DynamicExcitation::getFunctionTableP() const {
    return dynamic_pointer_cast<FunctionTable>(model.find(functionTableP));
}


shared_ptr<LoadSet> DynamicExcitation::getLoadSet() const {
    return model.find(loadSet);
}

const ValuePlaceHolder DynamicExcitation::getFunctionTableBPlaceHolder() const {
    return ValuePlaceHolder(model, functionTableB.type, functionTableB.original_id, NamedValue::FREQ);
}

const ValuePlaceHolder DynamicExcitation::getFunctionTablePPlaceHolder() const {
    return ValuePlaceHolder(model, functionTableP.type, functionTableP.original_id, NamedValue::FREQ);
}

set<int> DynamicExcitation::nodePositions() const {
    return set<int>();
}

const DOFS DynamicExcitation::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}

shared_ptr<Loading> DynamicExcitation::clone() const {
    return shared_ptr<Loading>(new DynamicExcitation(*this));
}

bool DynamicExcitation::validate() const {
    //TODO: Validate should be a bit more complex
    return getLoadSet() && getFunctionTableB() && getDynaPhase() && getDynaDelay();
}

bool DynamicExcitation::ineffective() const {
    return false;
}

InitialTemperature::InitialTemperature(const Model& model, const double temperature, const int original_id) :
                NodeLoading(model, INITIAL_TEMPERATURE, original_id), temperature(temperature) {
}

shared_ptr<Loading> InitialTemperature::clone() const {
	return make_shared<InitialTemperature>(*this);
}

const DOFS InitialTemperature::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}

void InitialTemperature::scale(double factor) {
	temperature *= factor;
}

bool InitialTemperature::ineffective() const {
	return is_zero(temperature);
}

} /* namespace vega */

