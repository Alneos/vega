/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * BoundaryCondition.cpp
 *
 *  Created on: 20 févr. 2013
 *      Author: dallolio
 */

#include "Dof.h"
#include <boost/lexical_cast.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <ciso646>
#include <math.h>

namespace vega {
using namespace std;

unordered_map<DOF::Code, DOF*, hash<int>> DOF::dofByCode;
unordered_map<int, DOF*> DOF::dofByPosition;

DOF::DOF(Code _code, bool _isTranslation, bool _isRotation, const string _label, int _position) :
		code(_code), isTranslation(_isTranslation), isRotation(_isRotation), label(_label), position(
				_position) {
	dofByCode[code] = this;
	dofByPosition[position] = this;
}

bool DOF::operator<(const DOF& other) const {
	return this->code < other.code;
}

bool DOF::operator==(const DOF& other) const {
	return this->code == other.code;
}

const DOF DOF::DX(DX_CODE, true, false, "DX", 0);
const DOF DOF::DY(DY_CODE, true, false, "DY", 1);
const DOF DOF::DZ(DZ_CODE, true, false, "DZ", 2);
const DOF DOF::RX(RX_CODE, false, true, "RX", 3);
const DOF DOF::RY(RY_CODE, false, true, "RY", 4);
const DOF DOF::RZ(RZ_CODE, false, true, "RZ", 5);

DOF DOF::findByPosition(int position) {
	if (position < 0 || position > 5) {
        throw invalid_argument("DOF Position not allowed : "+std::to_string(position));
	}
	auto dofIter = dofByPosition.find(position);
	if (dofIter == dofByPosition.end()) {
		throw invalid_argument("unexpected error");
	}
	return *(dofIter->second);
}

DOF::operator char() const {
	return (char) code;
}

ostream &operator<<(ostream &out, const DOF& dof) {
	out << dof.label;
	return out;
}

const double DOFS::FREE_DOF = -DBL_MAX;

const boost::bimap<int, char> DOFS::DOF_BY_NASTRANCODE = boost::assign::list_of<
		boost::bimap<int, char>::relation>
		( 1, DOF::DX_CODE )
		( 2, DOF::DY_CODE )
		( 3, DOF::DZ_CODE )
		( 4, DOF::RX_CODE )
		( 5, DOF::RY_CODE )
		( 6, DOF::RZ_CODE );

const DOFS DOFS::NO_DOFS((char) 0);
const DOFS DOFS::ONE((char) DOF::DX_CODE);
const DOFS DOFS::TRANSLATIONS((char) (DOF::DX_CODE | DOF::DY_CODE | DOF::DZ_CODE));
const DOFS DOFS::ROTATIONS((char) (DOF::RX_CODE | DOF::RY_CODE | DOF::RZ_CODE));
const DOFS DOFS::ALL_DOFS = TRANSLATIONS + ROTATIONS;

DOFS::DOFS(char _dofsCode) :
		dofsCode(_dofsCode) {
}

DOFS::DOFS(DOF dof) :
		dofsCode((char) dof.code) {
}

VectorialValue DOFS::getTranslations() {
	double dx = contains(DOF::DX) ? FREE_DOF : 0.0;
	double dy = contains(DOF::DY) ? FREE_DOF : 0.0;
	double dz = contains(DOF::DZ) ? FREE_DOF : 0.0;
	return VectorialValue(dx, dy, dz);
}

VectorialValue DOFS::getRotations() {
	double rx = contains(DOF::RX) ? FREE_DOF : 0.0;
	double ry = contains(DOF::RY) ? FREE_DOF : 0.0;
	double rz = contains(DOF::RZ) ? FREE_DOF : 0.0;
	return VectorialValue(rx, ry, rz);
}

DOFS::DOFS(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz) :
		dofsCode(combineCodes(dx, dy, dz, rx, ry, rz)) {
}

bool DOFS::containsAll(DOFS dofs) const {
	return (dofs.dofsCode | dofsCode) == dofsCode;
}

bool DOFS::containsAnyOf(DOFS dofs) const {
	return (dofsCode & dofs.dofsCode) != 0;
}

bool DOFS::contains(DOF dof) const {
	return (dofsCode & dof.code) != 0;
}

DOFS::operator char() const {
	return this->dofsCode;
}

bool DOFS::operator ==(const DOFS& other) const {
	return this->dofsCode == other.dofsCode;
}

bool DOFS::operator ==(const DOF& other) const {
	return this->dofsCode == other.code;
}

bool DOFS::operator ==(const char other) const {
	return this->dofsCode == other;
}

DOFS& DOFS::operator+=(const DOF& other) {
	this->dofsCode |= static_cast<char>(other.code);
	return *this;
}

DOFS& DOFS::operator+=(const DOFS& other) {
	this->dofsCode |= other.dofsCode;
	return *this;
}

int DOFS::size() {
	int result = contains(DOF::DX) ? 1 : 0;
	result += contains(DOF::DY) ? 1 : 0;
	result += contains(DOF::DZ) ? 1 : 0;
	result += contains(DOF::RX) ? 1 : 0;
	result += contains(DOF::RY) ? 1 : 0;
	result += contains(DOF::RZ) ? 1 : 0;
	return result;
}

int DOFS::nastranCode() const {
	string dofs = "";
	for (DOF dof : *this) {
		int dofCode = DOF_BY_NASTRANCODE.right.find(dof.code)->second;
		dofs = dofs + to_string(dofCode);
	}
	return dofs.size() >= 1 ? boost::lexical_cast<int>(dofs) : 0;
}

// Should be in NastranTokenizer, imho
// TODO: move to NastranTokenizer
DOFS DOFS::nastranCodeToDOFS(int nastranCode) {
	int number = nastranCode;

	// Dirty hack for scalar points, which have one DOF numbered 0
	if (number==0){
	    number=1;
	}
	DOFS dofs = DOFS::NO_DOFS;
	while (number) {
		int nastranDigit = number % 10;
		number = number / 10;
		auto codeiter = DOF_BY_NASTRANCODE.left.find(nastranDigit);
		if (codeiter == DOF_BY_NASTRANCODE.left.end()) {
			throw invalid_argument(
					string("Invalid Nastran code: " + boost::lexical_cast<string>(nastranCode)));
		}
		DOFS internalDOFCode = codeiter->second;
		dofs = dofs + internalDOFCode;
	}
	return dofs;
}

DOFS operator +(const DOFS lhs, const DOF& rhs) {
	return DOFS((char) (lhs.dofsCode | rhs.code));
}

DOFS operator -(const DOFS lhs, const DOF& rhs) {
	return DOFS((char) (lhs.dofsCode & ~rhs.code));
}

DOFS operator+(const DOFS lhs, const DOFS& rhs) {
	return DOFS((char) (lhs.dofsCode | rhs.dofsCode));
}
DOFS operator-(const DOFS lhs, const DOFS& rhs) {
	return DOFS((char) (lhs.dofsCode & (~rhs.dofsCode)));
}

DOFS DOFS::combineCodes(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz) {
	DOFS codes = DOFS::NO_DOFS;
	if (dx) {
		codes += DOF::DX;
	}
	if (dy) {
		codes += DOF::DY;
	}
	if (dz) {
		codes += DOF::DZ;
	}
	if (rx) {
		codes += DOF::RX;
	}
	if (ry) {
		codes += DOF::RY;
	}
	if (rz) {
		codes += DOF::RZ;
	}
	return codes;
}

ostream &operator<<(ostream &out, const DOFS::iterator& dofs_iter) {
	out << "DOFS_iterator pos: " << dofs_iter.dofPosition;
	return out;
}

DOFS::iterator::iterator(int _dofPosition, const DOFS *dofs) :
		dofPosition(_dofPosition), outer_dofs(dofs) {
	next_available_dof();
}

void DOFS::iterator::next_available_dof() {
	for (; dofPosition < 6; dofPosition++) {
		if (outer_dofs->contains(DOF::findByPosition(dofPosition))) {
			break;
		}
	}
}

DOFS::iterator DOFS::begin() const {
	return DOFS::iterator(0, this);
}

DOFS::iterator DOFS::end() const {
	return DOFS::iterator(6, this);
}

ostream &operator<<(ostream &out, const DOFS& dofs) {
	bool first = true;
	out << "[";
	for (int i = 0; i < 6; i++) {
		DOF curDof = DOF::findByPosition(i);
		if (dofs.contains(curDof)) {
			if (!first) {
				out << ",";
			}
			first = false;
			out << curDof;
		}
	}
	out << "]";
	return out;
}

DOFMatrix::DOFMatrix(bool symmetric) : symmetric(symmetric) {

}

void DOFMatrix::addComponent(const DOF dof1, const DOF dof2, const double value) {
	if (symmetric && dof1 > dof2) {
		componentByDofs[make_pair(dof2, dof1)] = value;
	} else {
		componentByDofs[make_pair(dof1, dof2)] = value;
	}
}

double DOFMatrix::findComponent(const DOF dof1, const DOF dof2) const {
	if (symmetric && dof1 > dof2) {
		auto it = componentByDofs.find(make_pair(dof2, dof1));
		if (it != componentByDofs.end()) {
			return it->second;
		}
	} else {
		auto it = componentByDofs.find(make_pair(dof1, dof2));
		if (it != componentByDofs.end()) {
			return it->second;
		}
	}
	return 0;
}

bool DOFMatrix::hasRotations() const {
	bool hasRotations = false;
	for (auto& kv : componentByDofs) {
		DOF dof1 = kv.first.first;
		DOF dof2 = kv.first.second;
		if (dof1.isRotation or dof2.isRotation) {
			hasRotations = true;
			break;
		}
	}
	return hasRotations;
}

bool DOFMatrix::hasTranslations() const {
	bool hasTranslations = false;
	for (auto& kv : componentByDofs) {
		DOF dof1 = kv.first.first;
		DOF dof2 = kv.first.second;
		if (dof1.isTranslation or dof2.isTranslation) {
			hasTranslations = true;
			break;
		}
	}
	return hasTranslations;
}

bool DOFMatrix::isDiagonal() const {
	bool isDiagonal = true;
	for (auto& kv : componentByDofs) {
		if (kv.first.first != kv.first.second and !is_equal(kv.second, 0)) {
			isDiagonal = false;
			break;
		}
	}
	return isDiagonal;
}

bool DOFMatrix::isSymmetric() const {
	return symmetric;
}

bool DOFMatrix::isEmpty() const {
	return componentByDofs.empty();
}

bool DOFCoefs::operator< (const DOFCoefs& other) const{
    for (int i=0; i<6; i++){
        if (is_equal(this->coefs[i],other.coefs[i])) continue;
        if (this->coefs[i]<other.coefs[i]) return true;
        return false;
    }
    return false;
}

bool DOFCoefs::operator== (const DOFCoefs & other) const{
    for (int i=0; i<6; i++){
        if (is_equal(this->coefs[i],other.coefs[i])) continue;
        return false;
    }
    return true;
}


}
