/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Utility.h
 *
 *  Created on: Nov 10, 2013
 *      Author: devel
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#define UNUSEDV(x) (void)(x)

#include "Reference.h"
#include "Value.h"
#include <climits>
#include <string>
#include <cmath>
#ifdef __GNUC__
// Avoid tons of warnings with the following code
#pragma GCC system_header
#endif
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/variant.hpp>

namespace vega {
/**
 * Tolerance used in comparing double.
 */
const double DOUBLE_COMPARE_TOLERANCE = std::numeric_limits<double>::epsilon() * 5;

inline bool is_zero(double x, double tolerance = DOUBLE_COMPARE_TOLERANCE) {
	return std::abs(x) <= tolerance;
}

inline bool is_equal(double x, double y, double tolerance = DOUBLE_COMPARE_TOLERANCE) {
	return std::abs(x - y) <= tolerance * std::max(1.0, std::max(std::abs(x), std::abs(y)));
}

namespace ublas = boost::numeric::ublas;
/*
 * Placeholder class, put here all the methods to operate on a vector.
 * @see vega expression.pyx VectorialValue
 */
class VectorialValue
final {
	private:
		ublas::vector<double> value;
		VectorialValue(ublas::vector<double>& value);
		friend std::ostream& operator<<(std::ostream& os, const VectorialValue& obj);
		public:
		VectorialValue(double x, double y, double z);
		VectorialValue(std::initializer_list<double> c);
		VectorialValue();
		inline double x() const {
			return value[0];
		}

		inline double y() const {
			return value[1];
		}

		inline double z() const {
			return value[2];
		}

		VectorialValue normalized() const;
		void scale(double factor);
		VectorialValue scaled(double factor) const;
		double dot(const VectorialValue &v) const;
		double norm() const;
		bool iszero() const;
		/**
		 *  Cross product with another VectorialValue. Result is ("this" x "other")
		 **/
		VectorialValue cross(const VectorialValue& other) const;
		/**
		 *  Return a vector orthonormal to "other", computed from this->value
		 **/
		VectorialValue orthonormalized(const VectorialValue& other) const;
		friend const VectorialValue operator+(const VectorialValue&, const VectorialValue&);
		friend const VectorialValue operator-(const VectorialValue&, const VectorialValue&);
		friend const VectorialValue operator*(const double&, const VectorialValue&);
		friend const VectorialValue operator/(const VectorialValue&, const double&);
		friend bool operator==(const VectorialValue&, const VectorialValue&);
		friend bool operator!=(const VectorialValue&, const VectorialValue&);

		static const VectorialValue X;
		static const VectorialValue Y;
		static const VectorialValue Z;
		static const VectorialValue XYZ[3];
	};

	/**
	 * Tri-state class: it can represent a double value, a reference or an empty value.
	 * It can be used in every place where a solver can put
	 */
	class ValueOrReference
		final {
			boost::variant<double, Reference<Value>> storage;
			public:
			static const ValueOrReference EMPTY_VALUE;
			ValueOrReference();
			ValueOrReference(double);
			ValueOrReference(const boost::variant<double, Reference<Value>>& value);
			double getValue() const;
			Reference<Value> getReference() const;
			bool isReference() const;
			bool isEmpty() const;
			bool operator==(const ValueOrReference& rhs) const;
			bool operator!=(const ValueOrReference& rhs) const {
				return !(*this == rhs);
			}
			;
			bool operator<(const ValueOrReference& rhs) const;
		};

		ostream& operator<<(ostream &out, const ValueOrReference& valueOrReference);
		} /* namespace vega */
#endif /* UTILITY_H_ */
