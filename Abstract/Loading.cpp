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

using namespace std;

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
	return static_cast<int>(getLoadings().size());
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
	if (loadings.size() == 0 ) { //or loadings.find(0) != loadings.end()) {
        if (model.configuration.logLevel >= LogLevel::INFO) {
            cout << "Loadset " << *this << " is not valid, no loads associated" << endl;
        }
		return false;
	}
	return true;
}

bool LoadSet::hasFunctions() const {
    for (shared_ptr<Loading> loading : getLoadings()) {
		if (loading->hasFunctions()) {
			return true;
		}
	}
	return false;
}

shared_ptr<LoadSet> LoadSet::clone() const {
	return make_shared<LoadSet>(*this);
}

NodeLoading::NodeLoading(const Model& model, Loading::Type type, int original_id,
		int coordinateSystemId) :
		Loading(model, type, Loading::NODE, original_id, coordinateSystemId), NodeContainer(*(model.mesh)) {
}

set<int> NodeLoading::nodePositions() const {
	return NodeContainer::nodePositions();
}

Gravity::Gravity(const Model& model, double acceleration, const VectorialValue& direction,
		const int original_id) :
		Loading(model, GRAVITY, NONE, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), acceleration(
				acceleration), direction(direction) {
}

const DOFS Gravity::getDOFSForNode(const int nodePosition) const {
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
	return make_shared<Gravity>(*this);
}

void Gravity::scale(const double factor) {
	acceleration *= factor;
}

bool Gravity::ineffective() const {
	return is_zero(acceleration) or direction.iszero();
}

Rotation::Rotation(const Model& model, const int original_id) :
		Loading(model, ROTATION, NONE, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
}

const DOFS Rotation::getDOFSForNode(const int nodePosition) const {
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
	return make_shared<RotationCenter>(*this);
}

void RotationCenter::scale(const double factor) {
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
	const Node& node = model.mesh->findNode(node_position, true, &model);
	return VectorialValue(node.x, node.y, node.z);
}

shared_ptr<Loading> RotationNode::clone() const {
	return make_shared<RotationNode>(*this);
}

void RotationNode::scale(const double factor) {
	speed *= factor;
}

/*NodalForce::NodalForce(const Model& model, const int original_id,
		int coordinate_system_id) :
		NodeLoading(model, NODAL_FORCE, original_id, coordinate_system_id), force(VectorialValue(0, 0, 0)), moment(
				VectorialValue(0, 0, 0)) {
}*/

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
	// TODO : LD try to avoid the need to copy and update node object
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

const DOFS NodalForce::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) != posSet.end()) {
        VectorialValue globalForce = getForceInGlobalCS(nodePosition);
        VectorialValue globalTorque = getMomentInGlobalCS(nodePosition);
		if (!is_zero(globalForce.x()))
			dofs = dofs + DOF::DX;
		if (!is_zero(globalForce.y()))
			dofs = dofs + DOF::DY;
		if (!is_zero(globalForce.z()))
			dofs = dofs + DOF::DZ;
		if (!is_zero(globalTorque.x()))
			dofs = dofs + DOF::RX;
		if (!is_zero(globalTorque.y()))
			dofs = dofs + DOF::RY;
		if (!is_zero(globalTorque.z()))
			dofs = dofs + DOF::RZ;
	}
	return dofs;
}

shared_ptr<Loading> NodalForce::clone() const {
	return make_shared<NodalForce>(*this);
}

void NodalForce::scale(const double factor) {
	force.scale(factor);
	moment.scale(factor);
}

bool NodalForce::ineffective() const {
	return force.iszero() and moment.iszero();
}

NodalForceTwoNodes::NodalForceTwoNodes(const Model& model, const int node1_id,
		const int node2_id, double magnitude, const int original_id) :
		NodalForce(model, original_id, 0.0, 0.0, 0.0, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), node_position1(
				model.mesh->findOrReserveNode(node1_id)), node_position2(
				model.mesh->findOrReserveNode(node2_id)), magnitude(magnitude) {
}

const VectorialValue NodalForceTwoNodes::getForceInGlobalCS(int nodePosition) const {
	const Node& node1 = model.mesh->findNode(node_position1, true, &model);
	const Node& node2 = model.mesh->findNode(node_position2, true, &model);
	VectorialValue direction = (VectorialValue(node2.x, node2.y, node2.z)
			- VectorialValue(node1.x, node1.y, node1.z)).normalized();
	return localToGlobal(nodePosition, magnitude * direction);
}

shared_ptr<Loading> NodalForceTwoNodes::clone() const {
	return make_shared<NodalForceTwoNodes>(*this);
}

void NodalForceTwoNodes::scale(const double factor) {
	magnitude *= factor;
}

bool NodalForceTwoNodes::ineffective() const {
	return is_zero(magnitude) or force.iszero();
}

NodalForceFourNodes::NodalForceFourNodes(const Model& model, const int node1_id,
        const int node2_id, const int node3_id, const int node4_id, double magnitude, const int original_id) :
        NodalForce(model, original_id, 0.0, 0.0, 0.0, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID),
                node_position1(model.mesh->findOrReserveNode(node1_id)),
                node_position2(model.mesh->findOrReserveNode(node2_id)),
                node_position3(model.mesh->findOrReserveNode(node3_id)),
                node_position4(model.mesh->findOrReserveNode(node4_id)), magnitude(magnitude) {
}

const VectorialValue NodalForceFourNodes::getForceInGlobalCS(int nodePosition) const {
    const Node& node1 = model.mesh->findNode(node_position1, true, &model);
    const Node& node2 = model.mesh->findNode(node_position2, true, &model);
    const Node& node3 = model.mesh->findNode(node_position3, true, &model);
    const Node& node4 = model.mesh->findNode(node_position4, true, &model);
    const VectorialValue& v1 = VectorialValue(node2.x, node2.y, node2.z) - VectorialValue(node1.x, node1.y, node1.z);
    const VectorialValue& v2 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node3.x, node3.y, node3.z);
    const VectorialValue& direction = v1.cross(v2).normalized();
    return localToGlobal(nodePosition, magnitude * direction);
}

shared_ptr<Loading> NodalForceFourNodes::clone() const {
    return make_shared<NodalForceFourNodes>(*this);
}

void NodalForceFourNodes::scale(const double factor) {
    magnitude *= factor;
}

bool NodalForceFourNodes::ineffective() const {
    return is_zero(magnitude);
}

StaticPressure::StaticPressure(const Model& model, const int node1_id,
        const int node2_id, const int node3_id, const int node4_id, double magnitude, const int original_id) :
        NodalForce(model, original_id, 0.0, 0.0, 0.0, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID),
                node_position1(model.mesh->findOrReserveNode(node1_id)),
                node_position2(model.mesh->findOrReserveNode(node2_id)),
                node_position3(model.mesh->findOrReserveNode(node3_id)),
                node_position4(node4_id == Globals::UNAVAILABLE_INT ? Globals::UNAVAILABLE_INT : model.mesh->findOrReserveNode(node4_id)),
                magnitude(magnitude) {
    addNodeId(node1_id);
    addNodeId(node2_id);
    addNodeId(node3_id);
    if (node4_id != Globals::UNAVAILABLE_INT) {
        addNodeId(node4_id);
    }
}

const VectorialValue StaticPressure::getForceInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end()) {
	    return VectorialValue();
	}
	VectorialValue forceNode;
    const Node& node1 = model.mesh->findNode(node_position1, true, &model);
    const Node& node2 = model.mesh->findNode(node_position2, true, &model);
    const Node& node3 = model.mesh->findNode(node_position3, true, &model);
    const VectorialValue& v12 = VectorialValue(node2.x, node2.y, node2.z) - VectorialValue(node1.x, node1.y, node1.z);
    const VectorialValue& v13 = VectorialValue(node3.x, node3.y, node3.z) - VectorialValue(node1.x, node1.y, node1.z);
    if (node_position4 != Globals::UNAVAILABLE_INT) {
        /*
        In the case of a quadrilateral surface, the grid points G1, G2, G3, and G4 should form a consecutive sequence around the perimeter. The right-hand rule is applied to find the assumed direction of the pressure. Four concentrated loads are applied to the grid points in approximately the same manner as for a triangular surface. The following specific procedures are adopted to accommodate irregular and/or warped surfaces:
        The surface is divided into two sets of overlapping triangular surfaces. Each triangular surface is bounded by two of the sides and one of the diagonals of the quadrilateral.
        One-half of the pressure is applied to each triangle, which is then treated in the manner described in Remark 2.
        */
        const Node& node4 = model.mesh->findNode(node_position4, true, &model);
        const VectorialValue& v14 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node1.x, node1.y, node1.z);
        const VectorialValue& v24 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node2.x, node2.y, node2.z);
        VectorialValue force1;
        VectorialValue force2;
        if (v13.norm() < v24.norm()) {
            // triangles : G1,G2,G3 and G1,G3,G4
            const VectorialValue& direction1 = v12.cross(v13).normalized();
            const VectorialValue& direction2 = v13.cross(v14).normalized();
            force1 = (magnitude / 6) * direction1;
            force2 = (magnitude / 6) * direction2;
            if (nodePosition == node_position1 || nodePosition == node_position3) {
                forceNode = force1 + force2;
            } else if (nodePosition == node_position2) {
                forceNode = force1;
            } else if (nodePosition == node_position4) {
                forceNode = force2;
            }
        } else {
            // triangles : G1,G2,G4 and G2,G3,G4
            const VectorialValue& v23 = VectorialValue(node3.x, node3.y, node3.z) - VectorialValue(node2.x, node2.y, node2.z);
            const VectorialValue& direction1 = v12.cross(v14).normalized();
            const VectorialValue& direction2 = v23.cross(v14).normalized();
            force1 = (magnitude / 6) * direction1;
            force2 = (magnitude / 6) * direction2;
            if (nodePosition == node_position2 || nodePosition == node_position4) {
                forceNode = force1;// + force2;
            } else if (nodePosition == node_position1) {
                forceNode = force1;
            } else if (nodePosition == node_position3) {
                forceNode = force1;//force2;
            }
        }
    } else {
        const VectorialValue& direction = v12.cross(v13).normalized();
        forceNode = (magnitude / 3) * direction;
    }
    return localToGlobal(nodePosition, forceNode);
}

shared_ptr<Loading> StaticPressure::clone() const {
    return make_shared<StaticPressure>(*this);
}

void StaticPressure::scale(const double factor) {
    magnitude *= factor;
}

bool StaticPressure::ineffective() const {
    return is_zero(magnitude);
}

ElementLoading::ElementLoading(const Model& model, Loading::Type type, int original_id,
		int coordinateSystemId) :
		Loading(model, type, Loading::ELEMENT, original_id, coordinateSystemId), CellContainer(*(model.mesh)) {
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

const DOFS ForceSurface::getDOFSForNode(const int nodePosition) const {
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

void ForceSurface::scale(const double factor) {
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
	const Node& node1 = model.mesh->findNode(nodePosition1);
	const Node& node2 = model.mesh->findNode(nodePosition2);
	const vector<int>& nodeIds = cells[0].faceids_from_two_nodes(node1.id, node2.id);
	return nodeIds;
}

ForceLine::ForceLine(const Model& model, const shared_ptr<NamedValue> force, DOF dof,
			const int original_id) :
		ElementLoading(model, FORCE_LINE, original_id), force(force), dof(dof) {

}

const DOFS ForceLine::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		if (!force->iszero())
			dofs = dof;
	}
	return dofs;
}

shared_ptr<Loading> ForceLine::clone() const {
	return make_shared<ForceLine>(*this);
}

void ForceLine::scale(const double factor) {
	force->scale(factor);
}

bool ForceLine::ineffective() const {
	return force->iszero();
}

bool ForceLine::validate() const {
	return true;
}

shared_ptr<Loading> PressionFaceTwoNodes::clone() const {
	return make_shared<PressionFaceTwoNodes>(*this);
}

NormalPressionFace::NormalPressionFace(const Model& model, double intensity, const int original_id) :
		ElementLoading(model, NORMAL_PRESSION_FACE, original_id,
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID), intensity(intensity) {
}

const DOFS NormalPressionFace::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		dofs += DOFS::ALL_DOFS;
	}
	return dofs;
}

shared_ptr<Loading> NormalPressionFace::clone() const {
	return make_shared<NormalPressionFace>(*this);
}

void NormalPressionFace::scale(const double factor) {
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

const DOFS DynamicExcitation::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}

shared_ptr<Loading> DynamicExcitation::clone() const {
    return make_shared<DynamicExcitation>(*this);
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

const DOFS InitialTemperature::getDOFSForNode(const int nodePosition) const {
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

