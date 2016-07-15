/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * CoordinateSystem.cpp
 *
 *  Created on: Oct 19, 2013
 *      Author: devel
 */

#include "CoordinateSystem.h"
#include "Model.h"

namespace vega {

CoordinateSystem::CoordinateSystem(const Model& model, Type type, const VectorialValue origin,
		const VectorialValue ex, const VectorialValue ey, int original_id) :
		Identifiable(original_id), model(model), type(type), origin(origin), ex(ex.normalized()), ey(
				ey.orthonormalized(this->ex)), ez(this->ex.cross(this->ey)) {
}


const string CoordinateSystem::name = "CoordinateSystem";

const map<CoordinateSystem::Type, string> CoordinateSystem::stringByType = {
		{ CARTESIAN, "CARTESIAN" },
		{ CYLINDRICAL, "CYLINDRICAL"},
		{ ORIENTATION, "ORIENTATION"},
		{ SPHERICAL, "SPHERICAL" },
		{ UNKNOWN, "UNKNOWN" }
};

ostream &operator<<(ostream &out, const CoordinateSystem& coordinateSystem) {
	out << to_str(coordinateSystem);
	return out;
}

const VectorialValue CoordinateSystem::getEulerAnglesIntrinsicZYX() const {
	double ax, ay, az = 0;
	double cy = sqrt(ez.z() * ez.z() + ey.z() * ey.z());
	if (cy > 1e-8) {
		ax = atan2(ex.y(), ex.x());
		ay = atan2(-ex.z(), cy);
		az = atan2(ey.z(), ez.z());
	} else {
		ax = atan2(-ey.x(), ey.y());
		ay = atan2(-ex.z(), cy);
	}
	return VectorialValue(ax, ay, az);
}


CartesianCoordinateSystem::CartesianCoordinateSystem(const Model& model,
		const VectorialValue& origin, const VectorialValue& ex, const VectorialValue& ey,
		int _original_id) :
		CoordinateSystem(model, CARTESIAN, origin, ex, ey, _original_id) {
}
CartesianCoordinateSystem::CartesianCoordinateSystem(const Model& model,
		int nO, int nZ, int nXZ, int _original_id) :
		CoordinateSystem(model, CARTESIAN, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), _original_id){
	nodesId.push_back(nO);
	nodesId.push_back(nZ);
	nodesId.push_back(nXZ);
	isVirtual=true;
}

const VectorialValue CartesianCoordinateSystem::vectorToGlobal(const VectorialValue& local) const {
	double x = local.x() * ex.x() + local.y() * ey.x() + local.z() * ez.x();
	double y = local.x() * ex.y() + local.y() * ey.y() + local.z() * ez.y();
	double z = local.x() * ex.z() + local.y() * ey.z() + local.z() * ez.z();
	return VectorialValue(x, y, z);
}

shared_ptr<CoordinateSystem> CartesianCoordinateSystem::clone() const {
	return shared_ptr<CoordinateSystem>(new CartesianCoordinateSystem(*this));
}

void CartesianCoordinateSystem::build(){

	if (isVirtual){
		int nO  = this->nodesId[0];
		int nZ  = this->nodesId[1];
		int nXZ = this->nodesId[2];

		Node nodeO = model.mesh->findNode(model.mesh->findNodePosition(nO));
		this->origin = VectorialValue(nodeO.x, nodeO.y, nodeO.z);

		Node nodeZ = model.mesh->findNode(model.mesh->findNodePosition(nZ));
		this->ez = VectorialValue(nodeZ.x - nodeO.x, nodeZ.y - nodeO.y, nodeZ.z - nodeO.z).normalized();

		Node nodeXZ = model.mesh->findNode(model.mesh->findNodePosition(nXZ));
		VectorialValue v = VectorialValue(nodeXZ.x - nodeO.x, nodeXZ.y - nodeO.y, nodeXZ.z - nodeO.z).normalized();

		this->ex = v.orthonormalized(this->ez);
		this->ey = this->ez.cross(this->ex);
		isVirtual=false;
	}
}


CylindricalCoordinateSystem::CylindricalCoordinateSystem(const Model& model,
		const VectorialValue origin, const VectorialValue ex, const VectorialValue ey,
		int original_id) :
		CoordinateSystem(model, CYLINDRICAL, origin, ex, ey, original_id), ur(this->ex), utheta(
				this->ey) {
}

void CylindricalCoordinateSystem::updateLocalBase(const VectorialValue& point) {
	utheta = ez.cross(point - origin).normalized();
	ur = utheta.cross(ez);
}

const VectorialValue CylindricalCoordinateSystem::vectorToGlobal(
		const VectorialValue& local) const {
	double x = local.x() * ur.x() + local.y() * utheta.x() + local.z() * ez.x();
	double y = local.x() * ur.y() + local.y() * utheta.y() + local.z() * ez.y();
	double z = local.x() * ur.z() + local.y() * utheta.z() + local.z() * ez.z();
	return VectorialValue(x, y, z);
}

shared_ptr<CoordinateSystem> CylindricalCoordinateSystem::clone() const {
	return shared_ptr<CoordinateSystem>(new CylindricalCoordinateSystem(*this));
}





OrientationCoordinateSystem::OrientationCoordinateSystem(const Model& model, const int nO, const int nX,
		const int nV, int original_id) :
		CoordinateSystem(model, ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), original_id){
	nodesId.push_back(nO);
	nodesId.push_back(nX);
	nodesId.push_back(nV);
	isVirtual=true;
}

OrientationCoordinateSystem::OrientationCoordinateSystem(const Model& model, const int nO, const int nX,
		const  VectorialValue v, int original_id) :
		CoordinateSystem(model, ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), original_id),
		v(v.normalized()){
	nodesId.push_back(nO);
	nodesId.push_back(nX);
	int a = Node::UNAVAILABLE_NODE;
	nodesId.push_back(a);
	isVirtual=true;
}

void OrientationCoordinateSystem::build(){

	if (isVirtual){
		Node nodeO = model.mesh->findNode(model.mesh->findNodePosition(this->getNodeO()));
		this->origin = VectorialValue(nodeO.x, nodeO.y, nodeO.z);

		Node nodeX = model.mesh->findNode(model.mesh->findNodePosition(this->getNodeX()));
		this->ex = VectorialValue(nodeX.x - nodeO.x, nodeX.y - nodeO.y, nodeX.z - nodeO.z).normalized();

		if(this->getNodeV() != Node::UNAVAILABLE_NODE){
			Node nodeV = model.mesh->findNode(model.mesh->findNodePosition(this->getNodeV()));
			this->v = VectorialValue(nodeV.x - nodeO.x, nodeV.y - nodeO.y, nodeV.z - nodeO.z).normalized();
		}
		this->ey = this->v.orthonormalized(this->ex);
		this->ez = this->ex.cross(this->ey);
		isVirtual=false;
	}
}

const VectorialValue OrientationCoordinateSystem::getOrigin() const{
	if (isVirtual){
		throw logic_error("Coordinate System is still virtual.");
	}
	return origin;
}
const VectorialValue OrientationCoordinateSystem::getEx() const{
	if (isVirtual){
		throw logic_error("Coordinate System is still virtual.");
	}
	return ex;
}
const VectorialValue OrientationCoordinateSystem::getEy() const{
	if (isVirtual){
		throw logic_error("Coordinate System is still virtual.");
	}
	return ey;
}
const VectorialValue OrientationCoordinateSystem::getEz() const {
	if (isVirtual){
		throw logic_error("Coordinate System is still virtual.");
	}
	return ez;
}

bool OrientationCoordinateSystem::operator ==(const OrientationCoordinateSystem& ocs) const {
	if ((this->getNodeO() != ocs.getNodeO()) || (this->getNodeX() != ocs.getNodeX())){
		return false;
	}
	if (this->getNodeV() == Node::UNAVAILABLE_NODE){
		VectorialValue vect = (this->getV() - ocs.getV());
		return is_equal(vect.norm(), 0);
	}else{
		return (this->getNodeV() == ocs.getNodeV());
	}
}

const VectorialValue OrientationCoordinateSystem::vectorToGlobal(const VectorialValue& local) const {
	if (isVirtual){
		throw logic_error("Coordinate System is still virtual.");
	}
	double x = local.x() * ex.x() + local.y() * ey.x() + local.z() * ez.x();
	double y = local.x() * ex.y() + local.y() * ey.y() + local.z() * ez.y();
	double z = local.x() * ex.z() + local.y() * ey.z() + local.z() * ez.z();
	return VectorialValue(x, y, z);
}


shared_ptr<CoordinateSystem> OrientationCoordinateSystem::clone() const {
	return shared_ptr<CoordinateSystem>(new OrientationCoordinateSystem(*this));
}

} /* namespace vega */
