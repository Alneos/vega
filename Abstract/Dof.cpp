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

unordered_map<DOF::Code, DOF*, EnumClassHash> DOF::dofByCode;
unordered_map<dof_int, DOF*> DOF::dofByPosition;

DOF::DOF(Code _code, bool _isTranslation, bool _isRotation, const string _label, dof_int _position) noexcept :
		code(_code), isTranslation(_isTranslation), isRotation(_isRotation), label(_label), position(
				_position) {
	dofByCode[code] = this;
	dofByPosition[position] = this;
}

bool DOF::operator<(const DOF& other) const noexcept {
	return this->code < other.code;
}

bool DOF::operator==(const DOF& other) const noexcept {
	return this->code == other.code;
}

dof_int DOF::operator|(const DOF& other) const noexcept {
	return static_cast<dof_int>(this->code) | static_cast<dof_int>(other.code);
}

const DOF DOF::DX(DOF::Code::DX_CODE, true, false, "DX", 0);
const DOF DOF::DY(DOF::Code::DY_CODE, true, false, "DY", 1);
const DOF DOF::DZ(DOF::Code::DZ_CODE, true, false, "DZ", 2);
const DOF DOF::RX(DOF::Code::RX_CODE, false, true, "RX", 3);
const DOF DOF::RY(DOF::Code::RY_CODE, false, true, "RY", 4);
const DOF DOF::RZ(DOF::Code::RZ_CODE, false, true, "RZ", 5);

DOF DOF::findByPosition(dof_int position) {
	if (position > 5) {
        throw invalid_argument("DOF Position not allowed : "+std::to_string(position));
	}
	auto dofIter = dofByPosition.find(position);
	if (dofIter == dofByPosition.end()) {
		throw invalid_argument("unexpected error");
	}
	return *(dofIter->second);
}

DOF::operator dof_int() const noexcept {
	return static_cast<dof_int>(code);
}

ostream &operator<<(ostream &out, const DOF& dof) noexcept {
	out << dof.label;
	return out;
}

const double DOFS::FREE_DOF = -DBL_MAX;

const boost::bimap<int, DOF::Code> DOFS::DOF_BY_NASTRANCODE = boost::assign::list_of<
		boost::bimap<int, DOF::Code>::relation>
		( 1, DOF::Code::DX_CODE )
		( 2, DOF::Code::DY_CODE )
		( 3, DOF::Code::DZ_CODE )
		( 4, DOF::Code::RX_CODE )
		( 5, DOF::Code::RY_CODE )
		( 6, DOF::Code::RZ_CODE );

const DOFS DOFS::NO_DOFS(static_cast<char>(0));
const DOFS DOFS::TRANSLATIONS(static_cast<char>(static_cast<char>(DOF::Code::DX_CODE) | static_cast<char>(DOF::Code::DY_CODE) | static_cast<char>(DOF::Code::DZ_CODE)));
const DOFS DOFS::ROTATIONS(static_cast<char>(static_cast<char>(DOF::Code::RX_CODE) | static_cast<char>(DOF::Code::RY_CODE) | static_cast<char>(DOF::Code::RZ_CODE)));
const DOFS DOFS::ALL_DOFS = TRANSLATIONS + ROTATIONS;

DOFS::DOFS(char _dofsCode) noexcept :
		dofsCode(_dofsCode) {
}

DOFS::DOFS(const DOF dof) noexcept :
		dofsCode(static_cast<char>(dof.code)) {
}

VectorialValue DOFS::getTranslations() noexcept {
	double dx = contains(DOF::DX) ? FREE_DOF : 0.0;
	double dy = contains(DOF::DY) ? FREE_DOF : 0.0;
	double dz = contains(DOF::DZ) ? FREE_DOF : 0.0;
	return VectorialValue(dx, dy, dz);
}

VectorialValue DOFS::getRotations() noexcept {
	double rx = contains(DOF::RX) ? FREE_DOF : 0.0;
	double ry = contains(DOF::RY) ? FREE_DOF : 0.0;
	double rz = contains(DOF::RZ) ? FREE_DOF : 0.0;
	return VectorialValue(rx, ry, rz);
}

DOFS::DOFS(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz) noexcept :
		dofsCode(combineCodes(dx, dy, dz, rx, ry, rz)) {
}

bool DOFS::containsAll(const DOFS& dofs) const noexcept {
	return (dofs.dofsCode | dofsCode) == dofsCode;
}

bool DOFS::containsAnyOf(const DOFS& dofs) const noexcept {
	return (dofsCode & dofs.dofsCode) != 0;
}

bool DOFS::contains(const DOF& dof) const noexcept {
	return (dofsCode & static_cast<char>(dof.code)) != 0;
}

DOFS DOFS::intersection(const DOFS& dofs) const noexcept {
	return DOFS(static_cast<char>(dofsCode & dofs.dofsCode));
}

DOFS::operator char() const noexcept {
	return this->dofsCode;
}

bool DOFS::operator ==(const DOFS& other) const noexcept {
	return this->dofsCode == other.dofsCode;
}

bool DOFS::operator ==(const DOF& other) const noexcept {
	return this->dofsCode == static_cast<char>(other.code);
}

bool DOFS::operator ==(const char other) const noexcept {
	return this->dofsCode == other;
}

DOFS& DOFS::operator+=(const DOF& dof) noexcept {
	this->dofsCode |= static_cast<char>(dof.code);
	return *this;
}

DOFS& DOFS::operator=(const DOF& dof) noexcept {
	this->dofsCode = static_cast<char>(dof.code);
	return *this;
}

DOFS& DOFS::operator+=(const DOFS& other) noexcept {
	this->dofsCode |= other.dofsCode;
	return *this;
}

int DOFS::size() const noexcept {
	int result = contains(DOF::DX) ? 1 : 0;
	result += contains(DOF::DY) ? 1 : 0;
	result += contains(DOF::DZ) ? 1 : 0;
	result += contains(DOF::RX) ? 1 : 0;
	result += contains(DOF::RY) ? 1 : 0;
	result += contains(DOF::RZ) ? 1 : 0;
	return result;
}

int DOFS::nastranCode() const noexcept {
	string dofsString = "";
	for (DOF dof : *this) {
		int dofCode = DOF_BY_NASTRANCODE.right.find(dof.code)->second;
		dofsString += to_string(dofCode);
	}
	return dofsString.empty() ? 0: boost::lexical_cast<int>(dofsString);
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
			throw invalid_argument("Invalid Nastran code: " + nastranCode);
		}
		DOFS internalDOFCode {static_cast<char>(codeiter->second)};
		dofs += internalDOFCode;
	}
	return dofs;
}

DOFS operator +(const DOFS lhs, const DOF& rhs) noexcept {
	return DOFS(static_cast<char>(lhs.dofsCode | static_cast<char>(rhs.code)));
}

DOFS operator -(const DOFS lhs, const DOF& rhs) noexcept {
	return DOFS(static_cast<char>(lhs.dofsCode & ~static_cast<char>(rhs.code)));
}

DOFS operator+(const DOFS lhs, const DOFS& rhs) noexcept {
	return DOFS(static_cast<char>(lhs.dofsCode | static_cast<char>(rhs.dofsCode)));
}
DOFS operator-(const DOFS lhs, const DOFS& rhs) noexcept {
	return DOFS(static_cast<char>(lhs.dofsCode & (~rhs.dofsCode)));
}

DOFS operator&(const DOFS lhs, const DOFS& rhs) noexcept {
	return DOFS(static_cast<char>(lhs.dofsCode & rhs.dofsCode));
}

DOFS DOFS::combineCodes(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz) noexcept {
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

ostream &operator<<(ostream &out, const DOFS::iterator& dofs_iter) noexcept {
	out << "DOFS_iterator pos: " << dofs_iter.dofPosition;
	return out;
}

DOFS::iterator::iterator(char _dofPosition, const DOFS *dofs) noexcept :
		dofPosition(_dofPosition), outer_dofs(dofs) {
	next_available_dof();
}

void DOFS::iterator::next_available_dof() noexcept {
	for (; dofPosition < 6; dofPosition++) {
		if (outer_dofs->contains(DOF::findByPosition(dofPosition))) {
			break;
		}
	}
}

DOFS::iterator DOFS::begin() const noexcept {
	return DOFS::iterator(0, this);
}

DOFS::iterator DOFS::end() const noexcept {
	return DOFS::iterator(6, this);
}

ostream &operator<<(ostream &out, const DOFS& dofs) noexcept {
	bool first = true;
	out << "[";
	for (char i = 0; i < 6; i++) {
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

DOFMatrix::DOFMatrix(MatrixType matrixType) noexcept : matrixType(matrixType) {

}

void DOFMatrix::addComponent(const DOF dof1, const DOF dof2, const double value) {
    if (matrixType == MatrixType::DIAGONAL and dof1 != dof2 and not is_zero(value)) {
        throw logic_error("Cannot assign non-zero value out of diagonal for a diagonal matrix");
    }
	if (matrixType == MatrixType::SYMMETRIC and dof1 > dof2) {
		componentByDofs[{dof2, dof1}] = value;
	} else {
		componentByDofs[{dof1, dof2}] = value;
	}
}

double DOFMatrix::findComponent(const DOF dof1, const DOF dof2) const noexcept {
    if (matrixType == MatrixType::DIAGONAL and dof1 != dof2) {
        return 0.0;
	} else if (matrixType == MatrixType::SYMMETRIC and dof1 > dof2) {
		auto it = componentByDofs.find({dof2, dof1});
		if (it != componentByDofs.end()) {
			return it->second;
		}
	} else {
		auto it = componentByDofs.find({dof1, dof2});
		if (it != componentByDofs.end()) {
			return it->second;
		}
	}
	return 0.0;
}

vector<double> DOFMatrix::asColumnsVector(bool addRotationsIfNotPresent) const noexcept {
	vector<double> result;
	DOFS dofs = (addRotationsIfNotPresent || hasRotations()) ? DOFS::ALL_DOFS : DOFS::TRANSLATIONS;
    for (const DOF coldof : dofs) {
        for (const DOF rowdof : dofs) {
            result.push_back(this->findComponent(rowdof, coldof));
        }
    }
	return result;
}

vector<double> DOFMatrix::asUpperTriangularColumnsVector(bool addRotationsIfNotPresent) const noexcept {
	vector<double> result;
	DOFS dofs = (addRotationsIfNotPresent || hasRotations()) ? DOFS::ALL_DOFS : DOFS::TRANSLATIONS;
    for (char coldof = 0; coldof < dofs.size(); coldof++) {
        const DOF colcode = DOF::findByPosition(coldof);
        for (char rowdof = 0; rowdof <= coldof; rowdof++) {
            const DOF rowcode = DOF::findByPosition(rowdof);
            result.push_back(this->findComponent(rowcode, colcode));
        }
    }
	return result;
}

vector<double> DOFMatrix::diagonal(bool addRotationsIfNotPresent) const noexcept {
	vector<double> result;
	DOFS dofs = (addRotationsIfNotPresent || hasRotations()) ? DOFS::ALL_DOFS : DOFS::TRANSLATIONS;
	for (const DOF& dof: dofs) {
		result.push_back(this->findComponent(dof, dof));
	}
	return result;
}

bool DOFMatrix::hasRotations() const noexcept {
	bool hasRotations = false;
	for (const auto& kv : componentByDofs) {
		DOF dof1 = kv.first.first;
		DOF dof2 = kv.first.second;
		if (dof1.isRotation or dof2.isRotation) {
			hasRotations = true;
			break;
		}
	}
	return hasRotations;
}

bool DOFMatrix::hasTranslations() const noexcept {
	bool hasTranslations = false;
	for (const auto& kv : componentByDofs) {
		DOF dof1 = kv.first.first;
		DOF dof2 = kv.first.second;
		if (dof1.isTranslation or dof2.isTranslation) {
			hasTranslations = true;
			break;
		}
	}
	return hasTranslations;
}

bool DOFMatrix::isDiagonal() const noexcept {
	if (matrixType == MatrixType::DIAGONAL) {
        return true;
    }
	for (const auto& kv : componentByDofs) {
		if (kv.first.first != kv.first.second and !is_zero(kv.second)) {
			return false;
		}
	}

	return true;
}

bool DOFMatrix::isSymmetric() const noexcept {
	if (matrixType == MatrixType::SYMMETRIC) {
        return true;
    }
	if (matrixType == MatrixType::DIAGONAL) {
        return true;
    }
	for (const auto& kv : componentByDofs) {
	    const auto& kv2 = componentByDofs.find({kv.first.second, kv.first.first});
		if (kv2 == componentByDofs.end()) {
            if (!is_zero(kv.second)) {
                return false;
            }
		} else {
		    if (!is_equal(kv.second, kv2->second)) {
                return false;
            }
		}
	}
	return true;
}

bool DOFMatrix::isMaxDiagonal() const noexcept {
	bool isMaxDiagonal = true;
	for (const auto& kv : componentByDofs) {
		if ((kv.first.first != kv.first.second and !is_zero(kv.second)) or !is_equal(kv.second, DBL_MAX)) {
			isMaxDiagonal = false;
			break;
		}
	}
	return isMaxDiagonal;
}

bool DOFMatrix::isEmpty() const noexcept {
	return componentByDofs.empty();
}

bool DOFMatrix::isZero() const noexcept {
    bool isZero = true;
	for (const auto& kv : componentByDofs) {
		if (not is_zero(kv.second)) {
			isZero = false;
			break;
		}
	}
	return isZero;
}

void DOFMatrix::setAllZero() noexcept {
    for (auto& entry : componentByDofs) {
        entry.second = 0.0;
    }
}

DOFMatrix DOFMatrix::transposed() const noexcept {
    DOFMatrix transposed(matrixType);
    for (auto& entry : componentByDofs) {
        transposed.addComponent(entry.first.second, entry.first.first, entry.second);
    }
    return transposed;
}

bool DOFMatrix::isEqual(const DOFMatrix& other) const noexcept {
    if (componentByDofs.size() != other.componentByDofs.size())
        return false; // This should be relaxed: maybe other matrix has some zero values explicitely
    for (auto& entry : componentByDofs) {
        const auto& otherit = other.componentByDofs.find(entry.first);
        if (otherit == other.componentByDofs.end() and not is_zero(entry.second)) {
            return false;
        }
        if (not is_equal(entry.second, otherit->second)) {
            return false;
        }
    }
    return true;
}

DOFCoefs::DOFCoefs(double dx, double dy, double dz, double rx, double ry, double rz) noexcept {
    coefs[0] = dx;
    coefs[1] = dy;
    coefs[2] = dz;
    coefs[3] = rx;
    coefs[4] = ry;
    coefs[5] = rz;
}

DOFCoefs::DOFCoefs(DOFS dofs, double val) noexcept {
    for(const DOF dof : DOFS::ALL_DOFS) {
        if (dofs.contains(dof))
            coefs[dof.position] = val;
        else
            coefs[dof.position] = Globals::UNAVAILABLE_DOUBLE;
    }
}

DOFCoefs::DOFCoefs(DOF dof, double val) noexcept {
    for(const DOF dof2 : DOFS::ALL_DOFS) {
        if (dof2 == dof)
            coefs[dof2.position] = val;
        else
            coefs[dof2.position] = Globals::UNAVAILABLE_DOUBLE;
    }
}

DOFS DOFCoefs::getDOFS() const noexcept {
    DOFS dofs;
    for(const DOF dof : DOFS::ALL_DOFS) {
        if (not is_equal(coefs[dof.position], Globals::UNAVAILABLE_DOUBLE))
            dofs += dof;
    }
    return dofs;
}

bool DOFCoefs::isEmpty() const noexcept {
    for(const DOF dof : DOFS::ALL_DOFS) {
        if (not is_equal(coefs[dof.position], Globals::UNAVAILABLE_DOUBLE))
            return false;
    }
    return true;
}

double DOFCoefs::getValue(const DOF dof) const noexcept {
    return coefs[dof.position];
}

void DOFCoefs::setValue(const DOF dof, double val) noexcept {
    coefs[dof.position] = val;
}

DOFCoefs& DOFCoefs::operator+=(const DOFCoefs& rv) noexcept {
    for (dof_int i = 0; i < 6; i++) {
        if (not is_equal(coefs[i], Globals::UNAVAILABLE_DOUBLE)) {
            coefs[i] += rv.coefs[i];
        } else {
            coefs[i] = rv.coefs[i];
        }
    }
    return *this;
}

DOFCoefs& DOFCoefs::operator*=(const double factor) noexcept {
    for (dof_int i = 0; i < 6; i++) {
        if (not is_equal(coefs[i], Globals::UNAVAILABLE_DOUBLE)) {
            coefs[i] *= factor;
        }
    }
    return *this;
}

double DOFCoefs::operator[](const dof_int i) noexcept {
    if (i > 6)
        return 0; // TODO LD: why this case ???
    return coefs[i];
}

bool DOFCoefs::operator< (const DOFCoefs& other) const noexcept {
    for (dof_int i=0; i<6; i++){
        if (is_equal(this->coefs[i],other.coefs[i])) continue;
        if (this->coefs[i]<other.coefs[i]) return true;
        return false;
    }
    return false;
}

bool DOFCoefs::operator== (const DOFCoefs & other) const noexcept {
    for (dof_int i=0; i<6; i++){
        if (is_equal(this->coefs[i],other.coefs[i])) continue;
        return false;
    }
    return true;
}

ostream &operator<<(ostream &out, const DOFCoefs& dofCoefs) noexcept {
	bool first = true;
	out << "[";
	for (char i = 0; i < 6; i++) {
		DOF curDof = DOF::findByPosition(i);
        if (!first) {
            out << ",";
        } else {
            first = false;
        }
        out << curDof << "=" << dofCoefs.getValue(curDof);
	}
	out << "]";
	return out;
}


}
