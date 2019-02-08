/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * BoundaryCondition.h
 *
 *  Created on: 20 févr. 2013
 *      Author: dallolio
 */

#ifndef DOF_H_
#define DOF_H_

#include <cfloat>
#include "Value.h"
#include <boost/bimap.hpp>
#include <unordered_map>
#include <set>

namespace vega {

class DOF {
private:
	friend std::ostream &operator<<(std::ostream &out, const DOF& node);
public:
	enum class Code {
		DX_CODE = 1,
		DY_CODE = 2,
		DZ_CODE = 4,
		RX_CODE = 8,
		RY_CODE = 16,
		RZ_CODE = 32
	};

	// enum class value DECLARATIONS - they are defined later
	static std::unordered_map<DOF::Code, DOF*, EnumClassHash> dofByCode;
	static std::unordered_map<int, DOF*> dofByPosition;

	static const DOF DX;
	static const DOF DY;
	static const DOF DZ;
	static const DOF RX;
	static const DOF RY;
	static const DOF RZ;

private:
	DOF(Code code, bool isTranslation, bool isRotation, const std::string label, int position);
public:
	Code code;
	bool isTranslation;
	bool isRotation;
	std::string label;
	int position;

	static DOF findByPosition(int position);
	bool operator<(const DOF& other) const;
	bool operator==(const DOF& other) const;
	char operator|(const DOF& other) const;
	operator char() const;
};

class DOFS {
private:
	static const boost::bimap<int, DOF::Code> DOF_BY_NASTRANCODE;

	friend DOFS operator+(const DOFS lhs, const DOFS& rhs);
	friend DOFS operator-(const DOFS lhs, const DOFS& rhs);
	friend DOFS operator+(const DOFS lhs, const DOF& rhs);
	friend DOFS operator-(const DOFS lhs, const DOF& rhs);
	friend DOFS operator&(const DOFS lhs, const DOFS& rhs);
	friend std::ostream &operator<<(std::ostream &out, const DOFS& node);

	char dofsCode;

public:
	static const double FREE_DOF;
	static const DOFS NO_DOFS;
	static const DOFS DX;
	static const DOFS TRANSLATIONS;
	static const DOFS ROTATIONS;
	static const DOFS ALL_DOFS;

	static DOFS nastranCodeToDOFS(int nastranCode);
	static DOFS combineCodes(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz);

	DOFS(char dofsCode);
	DOFS(const DOF dof);
	DOFS(bool dx = false, bool dy = false, bool dz = false, bool rx = false, bool ry = false,
			bool rz = false);

	bool containsAll(DOFS dofs) const;
	bool containsAnyOf(DOFS dofs) const;
	bool contains(DOF dofs) const;
	DOFS intersection(DOFS) const;
	VectorialValue getTranslations();
	VectorialValue getRotations();

	operator char() const;
	bool operator==(const DOFS& other) const;
	bool operator==(const DOF& other) const;
	bool operator==(const char other) const;
	DOFS& operator+=(const DOFS& other );
	DOFS& operator+=(const DOF& other );
	DOFS& operator=(const DOF& dof);
	int size() const;
	int nastranCode() const;

	class iterator;
	friend class iterator;
	class iterator: public std::iterator<std::input_iterator_tag, DOF, ptrdiff_t> {
	private:
		int dofPosition;
		const DOFS *outer_dofs;
		void next_available_dof();
	public:
 		friend std::ostream &operator<<(std::ostream &out, const DOFS::iterator& dofs_iter);
		iterator(int dofPosition, const DOFS *outer_dofs);

		bool operator==(const iterator& x) const {
			return dofPosition == x.dofPosition;
		}

		bool operator!=(const iterator& x) const {
			return !(*this == x);
		}

		const DOF operator*() const {
			return DOF::findByPosition(dofPosition);
		}

		iterator& operator++() {
			dofPosition++;
			next_available_dof();
			return *this;
		}

		iterator operator++(int) {
			iterator tmp = *this;
			++*this;
			return tmp;
		}
	};
	iterator begin() const;
	const iterator end() const;
};

DOFS operator+(const DOFS lhs, const DOFS& rhs);
DOFS operator-(const DOFS lhs, const DOFS& rhs);
DOFS operator+(const DOFS lhs, const DOF& rhs);
DOFS operator-(const DOFS lhs, const DOF& rhs);
DOFS operator&(const DOFS lhs, const DOF& rhs);

std::ostream &operator<<(std::ostream &out, const DOFS& dofs);
std::ostream &operator<<(std::ostream &out, const DOFS::iterator& dofs_iter);

/* Sparse matrix between two dofs (of the same node), or the same dof with itself.*/
class DOFMatrix final {
private:
		bool symmetric;
public:
		std::unordered_map<std::pair<DOF, DOF>, double,
				boost::hash<std::pair<int, int>>>componentByDofs{};
		DOFMatrix(bool it2 = false);
		void addComponent(const DOF dof1, const DOF dof2, const double value);
		double findComponent(const DOF dof1, const DOF dof2) const;
		bool hasTranslations() const;
		bool hasRotations() const;
		bool isDiagonal() const;
		/**
		 * Checks if diagonal is filled with DBL_MAX values
		 * useful (for now) to understand if an element is totally rigid
		 **/
		bool isMaxDiagonal() const;
		bool isSymmetric() const;
		bool isEmpty() const;
};


/**
 * This class regroups one coefficient by DOF.
 * It's basically a double[6] with a few extra security test.
 */
class DOFCoefs {
private:
    double coefs[6];
public:
    DOFCoefs(double dx = 0, double dy = 0, double dz = 0, double rx = 0, double ry = 0,
            double rz = 0);

    DOFCoefs(DOFS dofs, double val = 0);
    DOFS getDOFS() const;
    bool isEmpty() const;
    double getValue(const DOF dof) const;
    DOFCoefs& operator+=(const DOFCoefs&);
    DOFCoefs& operator*=(const double factor);
    double operator[](const int);
    bool operator<(const DOFCoefs& other) const;
    bool operator==(const DOFCoefs& other) const;
};



} /* namespace vega */


namespace std {

template<>
struct hash<vega::DOF> {
	size_t operator()(const vega::DOF& dof) const {
		return hash<int>()(dof.position);
	}
};

template<>
struct hash<vega::DOFS> {
	size_t operator()(const vega::DOFS& dofs) const {
		return hash<char>()(static_cast<char>(dofs));
	}
};

} /* namespace std */

#endif /* DOF_H_ */
