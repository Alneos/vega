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

const VectorialValue CartesianCoordinateSystem::vectorToGlobal(const VectorialValue& local) const {
	double x = local.x() * ex.x() + local.y() * ey.x() + local.z() * ez.x();
	double y = local.x() * ex.y() + local.y() * ey.y() + local.z() * ez.y();
	double z = local.x() * ex.z() + local.y() * ey.z() + local.z() * ez.z();
	return VectorialValue(x, y, z);
}

shared_ptr<CoordinateSystem> CartesianCoordinateSystem::clone() const {
	return shared_ptr<CoordinateSystem>(new CartesianCoordinateSystem(*this));
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

//bool Orientation::operator ==(const Orientation& orientation) const {
//	VectorialValue vect = (this->toVectY().vectY - orientation.toVectY().vectY);
//	return is_equal(vect.norm(), 0);
//}
//
//VectY::VectY(double x, double y, double z) :
//		vectY(VectorialValue(x, y, z)) {
//}
//
//VectY::VectY(VectorialValue vectY) :
//		vectY(vectY) {
//}
//
//VectY VectY::toVectY() const {
//	return *this;
//}
//
//shared_ptr<Orientation> VectY::clone() const {
//	return shared_ptr<Orientation>(new VectY(*this));
//}
//
//TwoNodesOrientation::TwoNodesOrientation(const Model& model, int node_id1, int node_id2) :
//		model(model), node_id1(node_id1), node_id2(node_id2) {
//}
//
//VectY TwoNodesOrientation::toVectY() const {
//	Node node1 = model.mesh->findNode(model.mesh->findNodePosition(node_id1));
//	Node node2 = model.mesh->findNode(model.mesh->findNodePosition(node_id2));
//	return VectY(node2.x - node1.x, node2.y - node1.y, node2.z - node1.z);
//}
//
//shared_ptr<Orientation> TwoNodesOrientation::clone() const {
//	return shared_ptr<Orientation>(new TwoNodesOrientation(*this));
//}



OrientationCoordinateSystem::OrientationCoordinateSystem(const Model& model, const int nO, const int nX,
		const int nV, int original_id) :
		CoordinateSystem(model, ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), original_id),
		nO(nO),	nX(nX), nV(nV) {
}

OrientationCoordinateSystem::OrientationCoordinateSystem(const Model& model, const int nO, const int nX,
		const  VectorialValue v, int original_id) :
		CoordinateSystem(model, ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), original_id),
		nO(nO),	nX(nX), v(v.normalized()), nV(Node::UNAVAILABLE_NODE){
}

void OrientationCoordinateSystem::build(){

	Node nodeO = model.mesh->findNode(model.mesh->findNodePosition(this->nO));
	this->origin = VectorialValue(nodeO.x, nodeO.y, nodeO.z);
	//this->updateLocalBase(this->origin);

	Node nodeX = model.mesh->findNode(model.mesh->findNodePosition(this->nX));
	this->ex = VectorialValue(nodeX.x - nodeO.x, nodeX.y - nodeO.y, nodeX.z - nodeO.z).normalized();

	if(this->nV != Node::UNAVAILABLE_NODE){
		Node nodeV = model.mesh->findNode(model.mesh->findNodePosition(this->nV));
	    this->v = VectorialValue(nodeV.x - nodeO.x, nodeV.y - nodeO.y, nodeV.z - nodeO.z).normalized();
	}
	this->ey = this->v.orthonormalized(this->ex);
	this->ez = this->ex.cross(this->ey);
	isVirtual=false;
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
