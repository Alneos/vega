/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Utility.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: devel
 */

#include "Utility.h"
#include <math.h>       /* pow */
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <cmath>

namespace ublas = boost::numeric::ublas;

namespace vega {

using namespace std;
VectorialValue::VectorialValue(double x, double y, double z) :
		value(ublas::vector<double>(3)) {
	value[0] = x;
	value[1] = y;
	value[2] = z;
}

VectorialValue::VectorialValue(ublas::vector<double>& value) :
		value(value) {
}

VectorialValue::VectorialValue(std::initializer_list<double> init_list):
				value(ublas::vector<double>(3)) {
	std::copy_n(init_list.begin(),std::min((int)init_list.size(),3), value.begin());
}

VectorialValue::VectorialValue() :
		value(ublas::vector<double>(3)) {
}

double VectorialValue::norm() const {
	return ublas::norm_2(value);
}

bool VectorialValue::iszero() const {
	return is_zero(value[0]) && is_zero(value[1]) && is_zero(value[2]);
}

VectorialValue VectorialValue::orthonormalized(const VectorialValue& other) const {
	ublas::vector<double> v0rth = this->value
			- (other.value * dot(other)) / (pow(ublas::norm_2(other.value),2));
	//v - numpy.dot(v,v2)*v2/numpy.linalg.norm(v2)
	v0rth = v0rth / ublas::norm_2(v0rth);
	return VectorialValue(v0rth);
}

VectorialValue VectorialValue::normalized() const {
	//double norm = sqrt(pow(x(), 2) + pow(y(), 2) + pow(z(), 2));
	double norm = ublas::norm_2(value);
	return VectorialValue(x() / norm, y() / norm, z() / norm);
}

VectorialValue VectorialValue::scaled(double factor) const {
	return VectorialValue(x() * factor, y() * factor, z() * factor);
}

void vega::VectorialValue::scale(double factor) {
	value[0] *= factor;
	value[1] *= factor;
	value[2] *= factor;
}

// Dot product with another VectorialValue
double VectorialValue::dot(const VectorialValue &v) const {
	return x() * v.x() + y() * v.y() + z() * v.z();
}

VectorialValue VectorialValue::cross(const VectorialValue &v) const {
	return VectorialValue(y() * v.z() - z() * v.y(), z() * v.x() - x() * v.z(),
			x() * v.y() - y() * v.x());
}

ostream& operator<<(ostream& os, const VectorialValue& obj) {
	os << "[x:" << obj.x() << ",y:" << obj.y() << ",z:" << obj.z() << "]";
	return os;
}

const VectorialValue operator+(const VectorialValue& left, const VectorialValue& right) {
	return VectorialValue(left.x() + right.x(), left.y() + right.y(), left.z() + right.z());
}
const VectorialValue operator-(const VectorialValue& left, const VectorialValue& right) {
	return VectorialValue(left.x() - right.x(), left.y() - right.y(), left.z() - right.z());
}
const VectorialValue operator*(const double& left, const VectorialValue& right) {
	return VectorialValue(left * right.x(), left * right.y(), left * right.z());
}

const VectorialValue operator/(const VectorialValue& left, const double& right) {
	return VectorialValue(left.x() / right, left.y() / right, left.z() / right);
}


bool operator==(const VectorialValue& left, const VectorialValue& right) {
	double norm = max(left.norm(), right.norm());
	if (is_zero(norm))
		return true;
	return (left - right).norm() / norm < 1e-8;
}

bool operator!=(const VectorialValue& left, const VectorialValue& right) {
	return !(left==right);
}


const VectorialValue VectorialValue::X(1, 0, 0);
const VectorialValue VectorialValue::Y(0, 1, 0);
const VectorialValue VectorialValue::Z(0, 0, 1);
const VectorialValue VectorialValue::XYZ[3] = { VectorialValue::X, VectorialValue::Y,
		VectorialValue::Z };

/**
 * ValueOrReference
 */
ostream& operator<<(ostream &out, const ValueOrReference& valueOrReference) {
	out << "ValueOrReference: ";
	if (valueOrReference.isReference()) {
		out << "Ref:" << valueOrReference.getReference().id;
	} else if (valueOrReference.isEmpty()) {
		out << "empty";
	} else {
		out << "value:" << valueOrReference.getValue();
	}
	return out;
}

const ValueOrReference ValueOrReference::EMPTY_VALUE(
		Reference<Value>(Value::STEP_RANGE, Reference<Value>::NO_ID, Reference<Value>::NO_ID));

ValueOrReference::ValueOrReference(const boost::variant<double, Reference<Value>>& _value) :
		storage(_value) {
}

ValueOrReference::ValueOrReference() :
		storage(EMPTY_VALUE.storage) {
}

ValueOrReference::ValueOrReference(double _value) :
		storage(_value) {
}

double ValueOrReference::getValue() const {
	return boost::get<double>(storage);
}

Reference<Value> ValueOrReference::getReference() const {
	return boost::get<Reference<Value>>(storage);
}

bool ValueOrReference::isReference() const {
	return storage.which() == 1;
}

bool ValueOrReference::isEmpty() const {
	return (*this) == EMPTY_VALUE;
}

bool ValueOrReference::operator==(const ValueOrReference& rhs) const {
	bool result;
	if (storage.which() != rhs.storage.which()) {
		result = false;
	} else if (storage.which() == 0) {
		result = is_equal(getValue() , rhs.getValue());
	} else {
		result = (getReference() == rhs.getReference());
	}
	return result;
}

bool ValueOrReference::operator<(const ValueOrReference& rhs) const {
	bool result;
	if (storage.which() != rhs.storage.which()) {
		result = storage.which() < rhs.storage.which();
	} else if (storage.which() == 0) {
		result = getValue() < rhs.getValue();
	} else {
		result = (getReference() < rhs.getReference());
	}
	return result;
}


bool InvertMatrix(const ublas::matrix<double>& input, ublas::matrix<double>& inverse)
{
   typedef ublas::permutation_matrix<std::size_t> pmatrix;

   // create a working copy of the input
   ublas::matrix<double> A(input);

   // create a permutation matrix for the LU-factorization
   pmatrix pm(A.size1());

   // perform LU-factorization
   int res = lu_factorize(A, pm);
   if (res != 0)
       return false;

   // create identity matrix of "inverse"
   inverse.assign(ublas::identity_matrix<double> (A.size1()));

   // backsubstitute to get the inverse
   ublas::lu_substitute(A, pm, inverse);
   return true;
}


//__________ ValueOrReference

} /* namespace vega */
