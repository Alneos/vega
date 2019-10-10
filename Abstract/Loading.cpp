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
//if with "or" and "and" under windows
#include <ciso646>

namespace vega {

using namespace std;

Loading::Loading(Model& model, const std::shared_ptr<LoadSet> loadset, Loading::Type type,
		const int original_id, const Reference<CoordinateSystem> csref) :
		Identifiable(original_id), model(model), type(type), loadset(loadset), csref(csref) {
    if (loadset != nullptr) {
        model.addLoadingIntoLoadSet(this->getReference(), loadset->getReference());
    }
}

const string Loading::name = "Loading";

const map<Loading::Type, string> Loading::stringByType = {
		{ Loading::Type::NODAL_FORCE, "NODAL_FORCE" },
		{ Loading::Type::GRAVITY, "GRAVITY" },
		{ Loading::Type::ROTATION, "ROTATION" },
		{ Loading::Type::NORMAL_PRESSION_FACE, "NORMAL_PRESSION_FACE" },
		{ Loading::Type::INITIAL_TEMPERATURE, "INITIAL_TEMPERATURE" },
		{ Loading::Type::FORCE_LINE, "FORCE_LINE" },
		{ Loading::Type::FORCE_SURFACE, "FORCE_SURFACE" },
		{ Loading::Type::DYNAMIC_EXCITATION, "DYNAMIC_EXCITATION" },
		{ Loading::Type::IMPOSED_DISPLACEMENT, "IMPOSED_DISPLACEMENT" }
};

ostream &operator<<(ostream &out, const Loading& loading) {
	out << to_str(loading);
	return out;
}

bool Loading::validate() const {
	bool valid = true;
	if (hasCoordinateSystem()) {
		valid = model.mesh.findCoordinateSystem(csref) != nullptr;
		if (!valid) {
			cerr
			<< "Coordinate system:"
					<< csref
					<< " for loading " << *this << " not found." << endl;
		}
	}
	return valid;
}

LoadSet::LoadSet(Model& model, Type type, int original_id) :
		Identifiable(original_id), model(model), type(type) {
}

LoadSet::LoadSet(Model& model, const Reference<LoadSet>& loadSetRef) :
		Identifiable(loadSetRef.original_id), model(model), type(loadSetRef.type) {
}

const string LoadSet::name = "LoadSet";

const map<LoadSet::Type, string> LoadSet::stringByType = { { LoadSet::Type::LOAD, "LOAD" }, { LoadSet::Type::DLOAD, "DLOAD" }, {
		LoadSet::Type::EXCITEID, "EXCITEID" }, { LoadSet::Type::LOADSET, "LOADSET" }, { LoadSet::Type::ALL, "ALL" } };

string LoadSet::getGroupName(Loading::Type loadingType) {
    string loadingGroupName = Loading::stringByType.find(loadingType)->second;
    if (loadingGroupName.size() >= 15) {
        // LD Workaround to try to limit group name size (max 24 chars in Aster)
        loadingGroupName = "TYPE" + to_string(static_cast<int>(loadingType));
    }
    return stringByType.find(this->type)->second + "_" + loadingGroupName + "_ID" + to_string(this->bestId());
}

ostream &operator<<(ostream &out, const LoadSet& loadset) {
	out << to_str(loadset);
	return out;
}

size_t LoadSet::size() const {
	return getLoadings().size();
}

set<shared_ptr<Loading>, ptrLess<Loading> > LoadSet::getLoadings() const {
	return model.getLoadingsByLoadSet(this->getReference());
}

set<shared_ptr<Loading>, ptrLess<Loading> > LoadSet::getLoadingsByType(Loading::Type loadingType) const {
	set<shared_ptr<Loading>, ptrLess<Loading> > result;
	for (const auto& loading : getLoadings()) {
		if (loading->type == loadingType) {
			result.insert(loading);
		}
	}
	return result;
}

bool LoadSet::validate() const {
	if (empty()) { //or loadings.find(0) != loadings.end()) {
        if (model.configuration.logLevel >= LogLevel::INFO) {
            cout << "Loadset " << *this << " is not valid, no loads associated" << endl;
        }
		return false;
	}
	return true;
}

bool LoadSet::hasFunctions() const {
    for (const auto& loading : getLoadings()) {
		if (loading->hasFunctions()) {
			return true;
		}
	}
	return false;
}

shared_ptr<LoadSet> LoadSet::clone() const {
	return make_shared<LoadSet>(*this);
}

NodeLoading::NodeLoading(Model& model, const std::shared_ptr<LoadSet> loadset, Loading::Type type, int original_id,
		const Reference<CoordinateSystem> csref) :
		Loading(model, loadset, type, original_id, csref), NodeContainer(model.mesh) {
}

set<int> NodeLoading::nodePositions() const {
	return NodeContainer::getNodePositionsIncludingGroups();
}

VolumicLoading::VolumicLoading(Model& model, const std::shared_ptr<LoadSet> loadset, Loading::Type type, int original_id) :
    Loading(model, loadset, type, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM) {
}

Gravity::Gravity(Model& model, const std::shared_ptr<LoadSet> loadset, double scalingFactor, const VectorialValue& gravityVector,
		const int original_id) :
		VolumicLoading(model, loadset, Loading::Type::GRAVITY, original_id), scalingFactor(
				scalingFactor), gravityVector(gravityVector) {
}

DOFS Gravity::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::NO_DOFS;
}
set<int> Gravity::nodePositions() const {
	return set<int>();
}

VectorialValue Gravity::getGravityVector() const {
	return gravityVector;
}

VectorialValue Gravity::getAccelerationVector() const {
	return gravityVector.scaled(scalingFactor);
}

double Gravity::getAccelerationScale() const {
    // Nastran doc: WTMASS does not affect loads generated by GRAV ...
	return scalingFactor;
}

shared_ptr<Loading> Gravity::clone() const {
	return make_shared<Gravity>(*this);
}

void Gravity::scale(const double factor) {
	scalingFactor *= factor;
}

bool Gravity::ineffective() const {
	return is_zero(scalingFactor) or gravityVector.iszero();
}

Rotation::Rotation(Model& model, const std::shared_ptr<LoadSet> loadset, const int original_id) :
		VolumicLoading(model, loadset, Loading::Type::ROTATION, original_id) {
}

DOFS Rotation::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::NO_DOFS;
}

set<int> Rotation::nodePositions() const {
	return set<int>();
}

bool Rotation::ineffective() const {
	return is_zero(getSpeed()) or getAxis().iszero();
}

RotationCenter::RotationCenter(Model& model, const std::shared_ptr<LoadSet> loadset, double speed, double center_x, double center_y,
		double center_z, double axis_x, double axis_y, double axis_z, const int original_id) :
		Rotation(model, loadset, original_id), speed(speed), axis(axis_x, axis_y, axis_z), center(center_x,
				center_y, center_z) {
}

double RotationCenter::getSpeed() const {
	return speed;
}

VectorialValue RotationCenter::getAxis() const {
	return axis;
}

VectorialValue RotationCenter::getCenter() const {
	return center;
}

shared_ptr<Loading> RotationCenter::clone() const {
	return make_shared<RotationCenter>(*this);
}

void RotationCenter::scale(const double factor) {
	speed *= factor;
}

RotationNode::RotationNode(Model& model, const std::shared_ptr<LoadSet> loadset, double speed, const int node_id, double axis_x,
		double axis_y, double axis_z, const int original_id) :
		Rotation(model, loadset, original_id), speed(speed), axis(axis_x, axis_y, axis_z), node_position(
				model.mesh.findOrReserveNode(node_id)) {
}

double RotationNode::getSpeed() const {
	return speed;
}

VectorialValue RotationNode::getAxis() const {
	return axis;
}

VectorialValue RotationNode::getCenter() const {
	const Node& node = model.mesh.findNode(node_position);
	return VectorialValue(node.x, node.y, node.z);
}

shared_ptr<Loading> RotationNode::clone() const {
	return make_shared<RotationNode>(*this);
}

void RotationNode::scale(const double factor) {
	speed *= factor;
}

ImposedDisplacement::ImposedDisplacement(Model& model, const std::shared_ptr<LoadSet> loadset, DOFS dofs, double value, const int original_id, const Reference<CoordinateSystem> csref) :
    NodeLoading(model, loadset, Loading::Type::IMPOSED_DISPLACEMENT, original_id, csref), displacements(dofs, value) {
}

DOFS ImposedDisplacement::getDOFSForNode(int nodePosition) const {
    UNUSEDV(nodePosition);
    return displacements.getDOFS();
}

bool ImposedDisplacement::ineffective() const {
    return this->empty() or displacements.isEmpty();
}

shared_ptr<Loading> ImposedDisplacement::clone() const {
    return make_shared<ImposedDisplacement>(*this);
}

double ImposedDisplacement::getDoubleForDOF(const DOF& dof) const {
    return displacements.getValue(dof);
}

void ImposedDisplacement::scale(const double factor) {
    for(DOF dof : DOFS::ALL_DOFS) {
        if (not is_equal(displacements[dof.position], Globals::UNAVAILABLE_DOUBLE))
            displacements *= factor;
    }
}

NodalForce::NodalForce(Model& model, const std::shared_ptr<LoadSet> loadset, const VectorialValue& force,
		const VectorialValue& moment, const int original_id, const Reference<CoordinateSystem> csref) :
		NodeLoading(model, loadset, Loading::Type::NODAL_FORCE, original_id, csref), force(force), moment(moment) {
}

NodalForce::NodalForce(Model& model, const std::shared_ptr<LoadSet> loadset, double fx, double fy, double fz, double mx,
		double my, double mz, const int original_id, const Reference<CoordinateSystem> csref) :
		NodeLoading(model, loadset, Loading::Type::NODAL_FORCE, original_id, csref), force(fx, fy, fz), moment(mx, my, mz) {
}

VectorialValue NodalForce::localToGlobal(int nodePosition, const VectorialValue& vectorialValue) const {
	if (!hasCoordinateSystem())
		return vectorialValue;
	shared_ptr<CoordinateSystem> coordSystem = model.mesh.findCoordinateSystem(csref);
	if (!coordSystem) {
		ostringstream oss;
		oss << "Coordinate system: " << csref
				<< " for nodal force not found." << endl;
		throw logic_error(oss.str());
	}
	const Node& node = model.mesh.findNode(nodePosition);
	coordSystem->updateLocalBase(VectorialValue(node.x, node.y, node.z));
	return coordSystem->vectorToGlobal(vectorialValue);
}

VectorialValue NodalForce::getForceInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end())
        throw logic_error("Requested node has not been assigned to this loading");
	return localToGlobal(nodePosition, force);
}

VectorialValue NodalForce::getMomentInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end())
        throw logic_error("Requested node has not been assigned to this loading");
	return localToGlobal(nodePosition, moment);
}

DOFS NodalForce::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) != posSet.end()) {
        VectorialValue globalForce = getForceInGlobalCS(nodePosition);
        VectorialValue globalTorque = getMomentInGlobalCS(nodePosition);
		if (!is_zero(globalForce.x()))
			dofs += DOF::DX;
		if (!is_zero(globalForce.y()))
			dofs += DOF::DY;
		if (!is_zero(globalForce.z()))
			dofs += DOF::DZ;
		if (!is_zero(globalTorque.x()))
			dofs += DOF::RX;
		if (!is_zero(globalTorque.y()))
			dofs += DOF::RY;
		if (!is_zero(globalTorque.z()))
			dofs += DOF::RZ;
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

NodalForceTwoNodes::NodalForceTwoNodes(Model& model, const std::shared_ptr<LoadSet> loadset, const int node1_id,
		const int node2_id, double magnitude, const int original_id) :
		NodalForce(model, loadset, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM), node_position1(
				model.mesh.findOrReserveNode(node1_id)), node_position2(
				model.mesh.findOrReserveNode(node2_id)), magnitude(magnitude) {
}

VectorialValue NodalForceTwoNodes::getForceInGlobalCS(int nodePosition) const {
	const Node& node1 = model.mesh.findNode(node_position1);
	const Node& node2 = model.mesh.findNode(node_position2);
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

NodalForceFourNodes::NodalForceFourNodes(Model& model, const std::shared_ptr<LoadSet> loadset, const int node1_id,
        const int node2_id, const int node3_id, const int node4_id, double magnitude, const int original_id) :
        NodalForce(model, loadset, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, original_id, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM),
                node_position1(model.mesh.findOrReserveNode(node1_id)),
                node_position2(model.mesh.findOrReserveNode(node2_id)),
                node_position3(model.mesh.findOrReserveNode(node3_id)),
                node_position4(model.mesh.findOrReserveNode(node4_id)), magnitude(magnitude) {
}

VectorialValue NodalForceFourNodes::getForceInGlobalCS(int nodePosition) const {
    const Node& node1 = model.mesh.findNode(node_position1);
    const Node& node2 = model.mesh.findNode(node_position2);
    const Node& node3 = model.mesh.findNode(node_position3);
    const Node& node4 = model.mesh.findNode(node_position4);
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

StaticPressure::StaticPressure(Model& model, const std::shared_ptr<LoadSet> loadset, const int node1_id,
        const int node2_id, const int node3_id, const int node4_id, double magnitude, const int original_id) :
        NodalForce(model, loadset, original_id, 0.0, 0.0, 0.0, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID),
                node_position1(model.mesh.findOrReserveNode(node1_id)),
                node_position2(model.mesh.findOrReserveNode(node2_id)),
                node_position3(model.mesh.findOrReserveNode(node3_id)),
                node_position4(node4_id == Globals::UNAVAILABLE_INT ? Globals::UNAVAILABLE_INT : model.mesh.findOrReserveNode(node4_id)),
                magnitude(magnitude) {
    addNodePosition(node_position1);
    addNodePosition(node_position2);
    addNodePosition(node_position3);
    if (node4_id != Globals::UNAVAILABLE_INT) {
        addNodePosition(node_position4);
    }
}

VectorialValue StaticPressure::getForceInGlobalCS(int nodePosition) const {
    set<int> posSet = nodePositions();
	if (posSet.find(nodePosition) == posSet.end()) {
	    return VectorialValue();
	}
	VectorialValue forceNode;
    const Node& node1 = model.mesh.findNode(node_position1);
    const Node& node2 = model.mesh.findNode(node_position2);
    const Node& node3 = model.mesh.findNode(node_position3);
    const VectorialValue& v12 = VectorialValue(node2.x, node2.y, node2.z) - VectorialValue(node1.x, node1.y, node1.z);
    const VectorialValue& v13 = VectorialValue(node3.x, node3.y, node3.z) - VectorialValue(node1.x, node1.y, node1.z);
    if (node_position4 != Globals::UNAVAILABLE_INT) {
        /*
        In the case of a quadrilateral surface, the grid points G1, G2, G3, and G4 should form a consecutive sequence around the perimeter. The right-hand rule is applied to find the assumed direction of the pressure. Four concentrated loads are applied to the grid points in approximately the same manner as for a triangular surface. The following specific procedures are adopted to accommodate irregular and/or warped surfaces:
        The surface is divided into two sets of overlapping triangular surfaces. Each triangular surface is bounded by two of the sides and one of the diagonals of the quadrilateral.
        One-half of the pressure is applied to each triangle, which is then treated in the manner described in Remark 2.
        */
        const Node& node4 = model.mesh.findNode(node_position4);
        const VectorialValue& v14 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node1.x, node1.y, node1.z);
        const VectorialValue& v24 = VectorialValue(node4.x, node4.y, node4.z) - VectorialValue(node2.x, node2.y, node2.z);
        VectorialValue force1;
        VectorialValue force2;
        if (v13.norm() < v24.norm()) {
            // triangles : G1,G2,G3 and G1,G3,G4
            const VectorialValue& crossTria1 = v12.cross(v13);
            double tria1Area = crossTria1.norm() / 2;
            const VectorialValue& direction1 = crossTria1.normalized();
            const VectorialValue& crossTria2 = v13.cross(v14);
            double tria2Area = crossTria2.norm() / 2;
            const VectorialValue& direction2 = crossTria2.normalized();
            force1 = (magnitude / 4 * tria1Area) * direction1;
            force2 = (magnitude / 4 * tria2Area) * direction2;
            if (nodePosition == node_position1 || nodePosition == node_position3) {
                forceNode = force1 + force2;
            } else if (nodePosition == node_position2) {
                forceNode = 2 * force1;
            } else if (nodePosition == node_position4) {
                forceNode = 2 * force2;
            }
        } else {
            // triangles : G1,G2,G4 and G2,G3,G4
            const VectorialValue& crossTria1 = v12.cross(v14);
            double tria1Area = crossTria1.norm() / 2;
            const VectorialValue& v23 = VectorialValue(node3.x, node3.y, node3.z) - VectorialValue(node2.x, node2.y, node2.z);
            const VectorialValue& direction1 = crossTria1.normalized();
            const VectorialValue& crossTria2 = v23.cross(v24);
            double tria2Area = crossTria2.norm() / 2;
            const VectorialValue& direction2 = crossTria2.normalized();
            force1 = (magnitude / 4 * tria1Area) * direction1;
            force2 = (magnitude / 4 * tria2Area) * direction2;
            if (nodePosition == node_position2 || nodePosition == node_position4) {
                forceNode = (force1 + force2);
            } else if (nodePosition == node_position1) {
                forceNode = 2 * force1;
            } else if (nodePosition == node_position3) {
                forceNode = 2 * force2;
            }
        }
    } else {
        const VectorialValue& crossTria = v12.cross(v13);
        double triaArea = crossTria.norm() / 2;
        if (model.configuration.logLevel >= LogLevel::TRACE) {
            cout << "StaticPressure triaArea(" << node1.id << "," << node2.id << "," << node3.id << ")=" << triaArea << endl;
        }
        const VectorialValue& direction = crossTria.normalized();
        forceNode = (magnitude / 3 * triaArea) * direction;
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

CellLoading::CellLoading(Model& model, const std::shared_ptr<LoadSet> loadset, Loading::Type type, int original_id,
		const Reference<CoordinateSystem> csref) :
		Loading(model, loadset, type, original_id, csref), CellContainer(model.mesh) {
}

set<int> CellLoading::nodePositions() const {
	return CellContainer::getNodePositionsIncludingGroups();
}

bool CellLoading::cellDimensionGreatherThan(SpaceDimension dimension) {
	bool result = false;
	for (const Cell& cell : this->getCellsIncludingGroups()) {
		result = result or cell.type.dimension > dimension;
		if (result)
			break;
	}
	return result;

}

bool CellLoading::appliedToGeometry() {
	bool isForceOnPoutre = false;
	bool assigned = false;
	for (const Cell& cell : getCellsIncludingGroups()) {
		int element_id = cell.elementId;
		if (element_id != Cell::UNAVAILABLE_CELL) {
			shared_ptr<ElementSet> element = model.find(
					Reference<ElementSet>(ElementSet::Type::UNKNOWN, Reference<ElementSet>::NO_ID,
							element_id));
			if (element) {
				if (element->isBeam()) {
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

void CellLoading::createSkin() {
    const auto& faceIds = this->getApplicationFaceNodeIds();
    if (not faceIds.empty()) {
        const int cellPosition = model.mesh.generateSkinCell(faceIds, SpaceDimension::DIMENSION_2D);

        // LD : try to solve https://github.com/Alneos/vega/issues/25 but it should be done only for loadings that can be grouped together (i.e. same pressure values, same directions etc)
        this->clear(); //< To remove the volumic cell and then add the skin at its place
        const auto& skin = make_shared<Skin>(model, model.modelType);
        if (model.configuration.alwaysUseGroupsForCells) {
            shared_ptr<CellGroup> cellGrp = model.mesh.createCellGroup(Cell::MedName(cellPosition), Group::NO_ORIGINAL_ID, "Single cell group over skin element");
            this->add(*cellGrp);
            cellGrp->addCellPosition(cellPosition);

            // LD : Workaround for Aster problem : MODELISA6_96
            //  les 1 mailles imprimées ci-dessus n'appartiennent pas au modèle et pourtant elles ont été affectées dans le mot-clé facteur : !
            //   ! FORCE_FACE

            skin->add(*cellGrp);
        } else {
            skin->addCellPosition(cellPosition);
            this->addCellPosition(cellPosition);
        }
        model.add(skin);
    }
}

ForceSurface::ForceSurface(Model& model, const std::shared_ptr<LoadSet> loadset, const VectorialValue& force,
		const VectorialValue& moment, const int original_id) :
		CellLoading(model, loadset, Loading::Type::FORCE_SURFACE, original_id,
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM), force(force), moment(moment) {
}

VectorialValue ForceSurface::getForce() const {
	return force;
}

VectorialValue ForceSurface::getMoment() const {
	return moment;
}

DOFS ForceSurface::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		if (!is_zero(force.x()))
			dofs += DOF::DX;
		if (!is_zero(force.y()))
			dofs += DOF::DY;
		if (!is_zero(force.z()))
			dofs += DOF::DZ;
		if (!is_zero(moment.x()))
			dofs += DOF::RX;
		if (!is_zero(moment.y()))
			dofs += DOF::RY;
		if (!is_zero(moment.z()))
			dofs += DOF::RZ;
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

ForceSurfaceTwoNodes::ForceSurfaceTwoNodes(Model& model, const std::shared_ptr<LoadSet> loadset, int nodeId1, int nodeId2,
		const VectorialValue& force, const VectorialValue& moment, const int original_id) :
		ForceSurface(model, loadset, force, moment, original_id), nodePosition1(
				model.mesh.findOrReserveNode(nodeId1)), nodePosition2(
				model.mesh.findOrReserveNode(nodeId2)) {
}

ForceSurfaceTwoNodes::ForceSurfaceTwoNodes(Model& model, const std::shared_ptr<LoadSet> loadset, int nodeId1,
		const VectorialValue& force, const VectorialValue& moment, const int original_id) :
		ForceSurface(model, loadset, force, moment, original_id), nodePosition1(
				model.mesh.findOrReserveNode(nodeId1)), nodePosition2(Globals::UNAVAILABLE_INT) {
}

vector<int> ForceSurfaceTwoNodes::getApplicationFaceNodeIds() const {
	const auto& cells = getCellsIncludingGroups();
	if (cells.size() != 1) {
		throw logic_error("More than one cell specified for a ForceSurfaceTwoNodes");
	}
	if (model.configuration.logLevel >= LogLevel::TRACE) {
        cout << "ForceSurfaceTwoNodes getApplicationFaceNodeIds() called" << endl;
	}
	const int nodeId1 = model.mesh.findNodeId(nodePosition1);
	int nodeId2 = Globals::UNAVAILABLE_INT;
	if (nodePosition2 != Globals::UNAVAILABLE_INT) {
        nodeId2 = model.mesh.findNodeId(nodePosition2);
	}
	return cells.begin()->faceids_from_two_nodes(nodeId1, nodeId2);
}

shared_ptr<Loading> ForceSurfaceTwoNodes::clone() const {
	return make_shared<ForceSurfaceTwoNodes>(*this);
}

ForceLine::ForceLine(Model& model, const std::shared_ptr<LoadSet> loadset, const shared_ptr<NamedValue> force, DOF dof,
			const int original_id) :
		CellLoading(model, loadset, Loading::Type::FORCE_LINE, original_id), force(force), dof(dof) {

}

DOFS ForceLine::getDOFSForNode(const int nodePosition) const {
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

NormalPressionFace::NormalPressionFace(Model& model, const std::shared_ptr<LoadSet> loadset, double intensity, const int original_id) :
		CellLoading(model, loadset, Loading::Type::NORMAL_PRESSION_FACE, original_id,
				CoordinateSystem::GLOBAL_COORDINATE_SYSTEM), intensity(intensity) {
}

DOFS NormalPressionFace::getDOFSForNode(const int nodePosition) const {
	DOFS dofs(DOFS::NO_DOFS);
	set<int> nodes = nodePositions();
	if (nodes.find(nodePosition) != nodes.end()) {
		dofs += DOFS::TRANSLATIONS;
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

NormalPressionFaceTwoNodes::NormalPressionFaceTwoNodes(Model& model, const std::shared_ptr<LoadSet> loadset, int nodeId1, int nodeId2,
		double intensity, const int original_id) :
		NormalPressionFace(model, loadset, intensity, original_id), nodePosition1(
				model.mesh.findOrReserveNode(nodeId1)), nodePosition2(
				model.mesh.findOrReserveNode(nodeId2)) {
}

NormalPressionFaceTwoNodes::NormalPressionFaceTwoNodes(Model& model, const std::shared_ptr<LoadSet> loadset, int nodeId1,
		double intensity, const int original_id) :
		NormalPressionFace(model, loadset, intensity, original_id), nodePosition1(
				model.mesh.findOrReserveNode(nodeId1)), nodePosition2(Globals::UNAVAILABLE_INT) {
}

vector<int> NormalPressionFaceTwoNodes::getApplicationFaceNodeIds() const {
	const auto& cells = getCellsIncludingGroups();
	if (cells.size() != 1) {
		throw logic_error("More than one cell specified for a NormalPressionFaceTwoNodes");
	}
	const int nodeId1 = model.mesh.findNodeId(nodePosition1);
    int nodeId2 = Globals::UNAVAILABLE_INT;
	if (nodePosition2 != Globals::UNAVAILABLE_INT) {
        nodeId2 = model.mesh.findNodeId(nodePosition2);
	}
	const vector<int>& faceIds = cells.begin()->faceids_from_two_nodes(nodeId1, nodeId2);
	return faceIds;
}

shared_ptr<Loading> NormalPressionFaceTwoNodes::clone() const {
	return make_shared<NormalPressionFaceTwoNodes>(*this);
}


DynamicExcitation::DynamicExcitation(Model& model, const std::shared_ptr<LoadSet> loadset, const Reference<NamedValue> dynaDelay, const Reference<NamedValue> dynaPhase,
        const Reference<NamedValue> functionTableB, const Reference<NamedValue> functionTableP, const Reference<LoadSet> loadSet,
        const DynamicExcitationType excitType,
        const int original_id) :
                Loading(model, loadset, Loading::Type::DYNAMIC_EXCITATION, original_id), dynaDelay(dynaDelay), dynaPhase(dynaPhase),
                functionTableB(functionTableB), functionTableP(functionTableP), loadSet(loadSet), excitType(excitType) {
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

shared_ptr<FunctionPlaceHolder> DynamicExcitation::getFunctionTableBPlaceHolder() const {
    return make_shared<FunctionPlaceHolder>(model, functionTableB.type, functionTableB.original_id, Function::ParaName::FREQ);
}

shared_ptr<FunctionPlaceHolder> DynamicExcitation::getFunctionTablePPlaceHolder() const {
    return make_shared<FunctionPlaceHolder>(model, functionTableP.type, functionTableP.original_id, Function::ParaName::FREQ);
}

set<int> DynamicExcitation::nodePositions() const {
    return set<int>();
}

DOFS DynamicExcitation::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::NO_DOFS;
}

void DynamicExcitation::scale(const double factor) {
	model.find(functionTableB)->scale(factor);
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

InitialTemperature::InitialTemperature(Model& model, const std::shared_ptr<LoadSet> loadset, const double temperature, const int original_id) :
                NodeLoading(model, loadset, Loading::Type::INITIAL_TEMPERATURE, original_id), temperature(temperature) {
}

shared_ptr<Loading> InitialTemperature::clone() const {
	return make_shared<InitialTemperature>(*this);
}

DOFS InitialTemperature::getDOFSForNode(const int nodePosition) const {
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

