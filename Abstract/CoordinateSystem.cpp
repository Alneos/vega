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
 * CoordinateSystem.cpp
 *
 *  Created on: Oct 19, 2013
 *      Author: devel
 */

#include "CoordinateSystem.h"
#include "Mesh.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <math.h>

namespace vega {

using namespace std;

CoordinateSystem::CoordinateSystem(const Mesh& mesh, Type type, const VectorialValue origin,
        const VectorialValue ex, const VectorialValue ey, const int rcs, int original_id) :
        Identifiable(original_id), mesh(mesh), type(type), origin(origin), ex(ex.normalized()), ey(
                ey.orthonormalized(this->ex)), rcs(rcs),  ez(this->ex.cross(this->ey)),
                inverseMatrix(3, 3){

}


const string CoordinateSystem::name = "CoordinateSystem";

const map<CoordinateSystem::Type, string> CoordinateSystem::stringByType = {
        { CoordinateSystem::Type::CARTESIAN, "CARTESIAN" },
        { CoordinateSystem::Type::CYLINDRICAL, "CYLINDRICAL"},
        { CoordinateSystem::Type::ORIENTATION, "ORIENTATION"},
        { CoordinateSystem::Type::SPHERICAL, "SPHERICAL" },
        { CoordinateSystem::Type::UNKNOWN, "UNKNOWN" }
};

ostream &operator<<(ostream &out, const CoordinateSystem& coordinateSystem) {
    out << to_str(coordinateSystem);
    return out;
}

const VectorialValue CoordinateSystem::getEulerAnglesIntrinsicZYX(const CoordinateSystem *cs) const {
    double ax, ay, az = 0;
    VectorialValue EX, EY, EZ;

    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        EX= this->ex;
        EY= this->ey;
        EZ= this->ez;
    }else{
        EX= vectorToGlobal(this->ex);
        EY= vectorToGlobal(this->ey);
        EZ= vectorToGlobal(this->ez);
    }
    if (cs != nullptr){
        EX = cs->vectorToLocal(EX);
        EY = cs->vectorToLocal(EY);
        EZ = cs->vectorToLocal(EZ);
    }

    double cy = sqrt(EZ.z() * EZ.z() + EY.z() * EY.z());
    if (cy > 1e-8) {
        ax = atan2(EX.y(), EX.x());
        ay = atan2(-EX.z(), cy);
        az = atan2(EY.z(), EZ.z());
    } else {
        ax = atan2(-EY.x(), EY.y());
        ay = atan2(-EX.z(), cy);
    }
    return VectorialValue(ax, ay, az);
}


CartesianCoordinateSystem::CartesianCoordinateSystem(const Mesh& mesh,
        const VectorialValue& origin, const VectorialValue& ex, const VectorialValue& ey, const int rcs,
        int _original_id) :
        CoordinateSystem(mesh, CoordinateSystem::Type::CARTESIAN, origin, ex, ey, rcs, _original_id) {
}
CartesianCoordinateSystem::CartesianCoordinateSystem(const Mesh& mesh,
        int nO, int nZ, int nXZ, const int rcs, int _original_id) :
        CoordinateSystem(mesh, CoordinateSystem::Type::CARTESIAN, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), rcs, _original_id){
    nodesId.push_back(nO);
    nodesId.push_back(nZ);
    nodesId.push_back(nXZ);
    isVirtual=true;
}

const VectorialValue CartesianCoordinateSystem::positionToGlobal(const VectorialValue& local) const{
    VectorialValue global = vectorToGlobal(local);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return this->getOrigin()+ global;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        return coordSystem->positionToGlobal(this->getOrigin())+global;
    }
}

const VectorialValue CartesianCoordinateSystem::vectorToGlobal(const VectorialValue& local) const {
    double x = local.x() * ex.x() + local.y() * ey.x() + local.z() * ez.x();
    double y = local.x() * ex.y() + local.y() * ey.y() + local.z() * ez.y();
    double z = local.x() * ex.z() + local.y() * ey.z() + local.z() * ez.z();

    VectorialValue vect = VectorialValue(x, y, z);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return vect;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        assert(coordSystem != nullptr);
        return coordSystem->vectorToGlobal(vect);
    }
}

const VectorialValue CartesianCoordinateSystem::vectorToLocal(const VectorialValue& global) const {

    VectorialValue vect;
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        vect = global;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        vect = coordSystem->vectorToLocal(global);
    }

    double x = vect.x() * this->inverseMatrix(0,0) + vect.y() * this->inverseMatrix(0,1) + vect.z() * this->inverseMatrix(0,2);
    double y = vect.x() * this->inverseMatrix(1,0) + vect.y() * this->inverseMatrix(1,1) + vect.z() * this->inverseMatrix(1,2);
    double z = vect.x() * this->inverseMatrix(2,0) + vect.y() * this->inverseMatrix(2,1) + vect.z() * this->inverseMatrix(2,2);
    return VectorialValue(x, y, z);
}

shared_ptr<CoordinateSystem> CartesianCoordinateSystem::clone() const {
    return make_shared<CartesianCoordinateSystem>(*this);
}

void CartesianCoordinateSystem::build(){

    if (isVirtual){
        int nO  = this->nodesId[0];
        int nZ  = this->nodesId[1];
        int nXZ = this->nodesId[2];

        const Node& nodeO = mesh.findNode(mesh.findNodePosition(nO));
        this->origin = VectorialValue(nodeO.x, nodeO.y, nodeO.z);

        const Node& nodeZ = mesh.findNode(mesh.findNodePosition(nZ));
        this->ez = VectorialValue(nodeZ.x - nodeO.x, nodeZ.y - nodeO.y, nodeZ.z - nodeO.z).normalized();

        const Node& nodeXZ = mesh.findNode(mesh.findNodePosition(nXZ));
        const VectorialValue& v = VectorialValue(nodeXZ.x - nodeO.x, nodeXZ.y - nodeO.y, nodeXZ.z - nodeO.z).normalized();

        this->ex = v.orthonormalized(this->ez);
        this->ey = this->ez.cross(this->ex);
        isVirtual=false;
    }

    boost::numeric::ublas::matrix<double> m(3, 3);
    m(0,0)= this->ex.x() ; m(0,1)= this->ey.x() ; m(0,2)= this->ez.x() ;
    m(1,0)= this->ex.y() ; m(1,1)= this->ey.y() ; m(1,2)= this->ez.y() ;
    m(2,0)= this->ex.z() ; m(2,1)= this->ey.z() ; m(2,2)= this->ez.z() ;
    InvertMatrix(m,this->inverseMatrix);

}


CylindricalCoordinateSystem::CylindricalCoordinateSystem(const Mesh& mesh,
        const VectorialValue origin, const VectorialValue ex, const VectorialValue ey, const int rcs,
        int original_id) :
        CoordinateSystem(mesh, CoordinateSystem::Type::CYLINDRICAL, origin, ex, ey, rcs, original_id), ur(this->ex), utheta(
                this->ey) {
}

// TODO : LD What's this ? Should this method be renamed moveOrigin ?
void CylindricalCoordinateSystem::updateLocalBase(const VectorialValue& point) {
    VectorialValue localOrigin = point - origin;
    if (!localOrigin.iszero()) {
        utheta = ez.cross(point - origin).normalized();
        ur = utheta.cross(ez);
    }
}

const VectorialValue CylindricalCoordinateSystem::positionToGlobal(const VectorialValue& local) const{
    double rcosth = local.x()*cos(M_PI*local.y()/180.0);
    double rsinth = local.x()*sin(M_PI*local.y()/180.0);
    double x = rcosth*ex.x() + rsinth*ey.x() + local.z()*ez.x();
    double y = rcosth*ex.y() + rsinth*ey.y() + local.z()*ez.y();
    double z = rcosth*ex.z() + rsinth*ey.z() + local.z()*ez.z();
    VectorialValue position = this->getOrigin()+VectorialValue(x,y,z);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return position;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        return coordSystem->positionToGlobal(position);
    }
}

const VectorialValue CylindricalCoordinateSystem::vectorToGlobal(
        const VectorialValue& local) const {
    double x = local.x() * ur.x() + local.y() * utheta.x() + local.z() * ez.x();
    double y = local.x() * ur.y() + local.y() * utheta.y() + local.z() * ez.y();
    double z = local.x() * ur.z() + local.y() * utheta.z() + local.z() * ez.z();
    VectorialValue vect = VectorialValue(x, y, z);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return vect;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        return coordSystem->vectorToGlobal(vect);
    }
}

const VectorialValue CylindricalCoordinateSystem::vectorToLocal(const VectorialValue& global) const {
    UNUSEDV(global);
    throw logic_error("Global To Local vector conversion not done for Cylindrical Coordinate System");
    return VectorialValue(0,0,0);
}


shared_ptr<CoordinateSystem> CylindricalCoordinateSystem::clone() const {
    return make_shared<CylindricalCoordinateSystem>(*this);
}

const VectorialValue CylindricalCoordinateSystem::getLocalEulerAnglesIntrinsicZYX(const CoordinateSystem *cs) const {
    CartesianCoordinateSystem localCS(this->mesh, VectorialValue(0,0,0), this->ur, this->utheta);
    return localCS.getEulerAnglesIntrinsicZYX(cs);
}




OrientationCoordinateSystem::OrientationCoordinateSystem(const Mesh& mesh, const int nO, const int nX,
        const int nV, const int rcs, int original_id) :
        CoordinateSystem(mesh, CoordinateSystem::Type::ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), rcs, original_id){
    nodesId.push_back(nO);
    nodesId.push_back(nX);
    nodesId.push_back(nV);
    isVirtual=true;
}

OrientationCoordinateSystem::OrientationCoordinateSystem(const Mesh& mesh, const int nO, const int nX,
        const  VectorialValue v, const int rcs, int original_id) :
        CoordinateSystem(mesh, CoordinateSystem::Type::ORIENTATION, VectorialValue(0,0,0), VectorialValue(0,0,0), VectorialValue(0,0,0), rcs, original_id),
        v(v.normalized()){
    nodesId.push_back(nO);
    nodesId.push_back(nX);
    int a = Node::UNAVAILABLE_NODE;
    nodesId.push_back(a);
    isVirtual=true;
}

void OrientationCoordinateSystem::build(){

    if (isVirtual){
        const Node& nodeO = mesh.findNode(mesh.findNodePosition(this->getNodeO()));
        this->origin = VectorialValue(nodeO.x, nodeO.y, nodeO.z);

        const Node& nodeX = mesh.findNode(mesh.findNodePosition(this->getNodeX()));
        this->ex = VectorialValue(nodeX.x - nodeO.x, nodeX.y - nodeO.y, nodeX.z - nodeO.z).normalized();

        if(this->getNodeV() != Node::UNAVAILABLE_NODE){
            const Node& nodeV = mesh.findNode(mesh.findNodePosition(this->getNodeV()));
            this->v = VectorialValue(nodeV.x - nodeO.x, nodeV.y - nodeO.y, nodeV.z - nodeO.z).normalized();
        }
        this->ey = this->v.orthonormalized(this->ex);
        this->ez = this->ex.cross(this->ey);
        isVirtual=false;
    }
    boost::numeric::ublas::matrix<double> m(3, 3);
    m(0,0)= this->ex.x() ; m(0,1)= this->ey.x() ; m(0,2)= this->ez.x() ;
    m(1,0)= this->ex.y() ; m(1,1)= this->ey.y() ; m(1,2)= this->ez.y() ;
    m(2,0)= this->ex.z() ; m(2,1)= this->ey.z() ; m(2,2)= this->ez.z() ;
    InvertMatrix(m,this->inverseMatrix);

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

const VectorialValue OrientationCoordinateSystem::positionToGlobal(const VectorialValue& local) const{
    if (isVirtual){
        throw logic_error("Coordinate System is still virtual.");
    }
    VectorialValue global = vectorToGlobal(local);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return origin + global;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        return coordSystem->positionToGlobal(origin + global);
    }
}

const VectorialValue OrientationCoordinateSystem::vectorToGlobal(const VectorialValue& local) const {
    if (isVirtual){
        throw logic_error("Coordinate System is still virtual.");
    }
    double x = local.x() * ex.x() + local.y() * ey.x() + local.z() * ez.x();
    double y = local.x() * ex.y() + local.y() * ey.y() + local.z() * ez.y();
    double z = local.x() * ex.z() + local.y() * ey.z() + local.z() * ez.z();
    VectorialValue vect = VectorialValue(x, y, z);
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        return vect;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        return coordSystem->vectorToGlobal(vect);
    }
}

const VectorialValue OrientationCoordinateSystem::vectorToLocal(const VectorialValue& global) const {

    VectorialValue vect;
    if (rcs == CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        vect = global;
    } else {
        shared_ptr<CoordinateSystem> coordSystem = mesh.getCoordinateSystem(rcs);
        vect = coordSystem->vectorToLocal(global);
    }
    double x = vect.x() * this->inverseMatrix(0,0) + vect.y() * this->inverseMatrix(0,1) + vect.z() * this->inverseMatrix(0,2);
    double y = vect.x() * this->inverseMatrix(1,0) + vect.y() * this->inverseMatrix(1,1) + vect.z() * this->inverseMatrix(1,2);
    double z = vect.x() * this->inverseMatrix(2,0) + vect.y() * this->inverseMatrix(2,1) + vect.z() * this->inverseMatrix(2,2);
    return VectorialValue(x, y, z);
}

shared_ptr<CoordinateSystem> OrientationCoordinateSystem::clone() const {
    return make_shared<OrientationCoordinateSystem>(*this);
}





/**
 * Coordinate System Container class
 */
int CoordinateSystemStorage::cs_next_position= CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID + 1;


CoordinateSystemStorage::CoordinateSystemStorage(const Mesh& mesh, LogLevel logLevel) :
        logLevel(logLevel), mesh(mesh) {
}

int CoordinateSystemStorage::getId(int cpos) const{
    const auto it=modelIdByPosition.find(cpos);
    if (it!= modelIdByPosition.end()){
        return it->second;
    }
    return UNAVAILABLE_ID;
}

int CoordinateSystemStorage::findPositionByUserId(int user_id) const{
    int cpos = UNAVAILABLE_POSITION;
    for (auto it= userIdByPosition.begin(); it!=userIdByPosition.end(); ++it){
        if (it->second == user_id){
            cpos = it->first;
            break;
        }
    }
    return cpos;
}

int CoordinateSystemStorage::findPositionById(int model_id) const{
    int cpos = UNAVAILABLE_POSITION;
    for (auto it= modelIdByPosition.begin(); it!=modelIdByPosition.end(); ++it){
        if (it->second == model_id){
            cpos = it->first;
            break;
        }
    }
    return cpos;
}

int CoordinateSystemStorage::add(const CoordinateSystem& coordinateSystem){
    int vid = coordinateSystem.getId();  // Model intern number
    int uid = coordinateSystem.getOriginalId(); // User Original number
    int cpos = UNAVAILABLE_POSITION;

    // Is this CS already reserved ?
    if (uid != Identifiable<CoordinateSystem>::NO_ORIGINAL_ID)
        cpos = findPositionByUserId(uid);

    if (cpos == UNAVAILABLE_POSITION){
        cpos = cs_next_position;
        cs_next_position++;
    }

    // We check some errors
    const auto it= modelIdByPosition.find(cpos);
    if ((it!=modelIdByPosition.end()) && (it->second!=UNAVAILABLE_ID)){
        throw logic_error("Coordinate system already added: Position "+to_string(cpos)+" VEGA Ids: "+ to_string(vid));
    }
    const auto it2= userIdByPosition.find(cpos);
    if ((it2!=userIdByPosition.end()) && (it2->second!=uid)){
        throw logic_error("Mismatch in coordinate system: Position "+to_string(cpos)+" has two original Ids: "+ to_string(uid) + " "+ to_string(it2->second));
    }

    userIdByPosition[cpos] = uid;
    modelIdByPosition[cpos] = vid;
    coordinateSystemById[vid] = coordinateSystem.clone();

    if (this->logLevel >= LogLevel::TRACE) {
        cout << "Add coordinate system id:" << vid << " (user id: "<<uid<<") in position:" << cpos << endl;
    }
    return cpos;
}

shared_ptr<CoordinateSystem> CoordinateSystemStorage::get(int cid) const {
  auto it = coordinateSystemById.find(cid);
  if (it == coordinateSystemById.end()) {
    return nullptr;
  }
  return it->second;
}

int CoordinateSystemStorage::reserve(int user_id) {

    if (user_id==CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID){
        throw logic_error("We don't reserve a position for the GLOBAL Coordinate System "+to_string(CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID));
    }

    int cpos= cs_next_position;
    cs_next_position++;

    userIdByPosition[cpos] = user_id;
    modelIdByPosition[cpos] = UNAVAILABLE_ID;

    if (this->logLevel >= LogLevel::TRACE) {
        cout << "Reserve coordinate user id:" << user_id << " in position:" << cpos << endl;
    }
    return cpos;
}

} /* namespace vega */
