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
	friend std::ostream &operator<<(std::ostream &out, const DOF& node) noexcept;
public:
	enum class Code : unsigned char {
		DX_CODE = 1,
		DY_CODE = 2,
		DZ_CODE = 4,
		RX_CODE = 8,
		RY_CODE = 16,
		RZ_CODE = 32
	};

	// enum class value DECLARATIONS - they are defined later
	static std::unordered_map<DOF::Code, DOF*, EnumClassHash> dofByCode;
	static std::unordered_map<unsigned char, DOF*> dofByPosition;

	static const DOF DX;
	static const DOF DY;
	static const DOF DZ;
	static const DOF RX;
	static const DOF RY;
	static const DOF RZ;

private:
	DOF(Code code, bool isTranslation, bool isRotation, const std::string label, unsigned char position) noexcept;
public:
	Code code;
	bool isTranslation;
	bool isRotation;
	std::string label;
	unsigned char position;

	static DOF findByPosition(unsigned char position);
	bool operator<(const DOF& other) const noexcept;
	bool operator==(const DOF& other) const noexcept;
	unsigned char operator|(const DOF& other) const noexcept;
	operator unsigned char() const noexcept;
};

class DOFS {
private:
	static const boost::bimap<int, DOF::Code> DOF_BY_NASTRANCODE;

	friend DOFS operator+(const DOFS lhs, const DOFS& rhs) noexcept;
	friend DOFS operator-(const DOFS lhs, const DOFS& rhs) noexcept;
	friend DOFS operator+(const DOFS lhs, const DOF& rhs) noexcept;
	friend DOFS operator-(const DOFS lhs, const DOF& rhs) noexcept;
	friend DOFS operator&(const DOFS lhs, const DOFS& rhs) noexcept;
	friend std::ostream &operator<<(std::ostream &out, const DOFS& node) noexcept;

	char dofsCode;

public:
	static const double FREE_DOF;
	static const DOFS NO_DOFS;
	static const DOFS DX;
	static const DOFS TRANSLATIONS;
	static const DOFS ROTATIONS;
	static const DOFS ALL_DOFS;

	static DOFS nastranCodeToDOFS(int nastranCode);
	static DOFS combineCodes(bool dx, bool dy, bool dz, bool rx, bool ry, bool rz) noexcept;

	DOFS(char dofsCode) noexcept;
	DOFS(const DOF dof) noexcept;
	DOFS(bool dx = false, bool dy = false, bool dz = false, bool rx = false, bool ry = false,
			bool rz = false) noexcept;

	bool containsAll(const DOFS&) const noexcept;
	bool containsAnyOf(const DOFS&) const noexcept;
	bool contains(const DOF&) const noexcept;
	DOFS intersection(const DOFS&) const noexcept;
	VectorialValue getTranslations() noexcept;
	VectorialValue getRotations() noexcept;

	operator char() const noexcept;
	bool operator==(const DOFS& other) const noexcept;
	bool operator==(const DOF& other) const noexcept;
	bool operator==(const char other) const noexcept;
	DOFS& operator+=(const DOFS& other ) noexcept;
	DOFS& operator+=(const DOF& other ) noexcept;
	DOFS& operator=(const DOF& dof) noexcept;
	int size() const noexcept;
	bool empty() const noexcept {
        return dofsCode == 0;
	}
	int nastranCode() const noexcept;

	class iterator;
	friend class iterator;
	class iterator: public std::iterator<std::input_iterator_tag, DOF, ptrdiff_t> {
	private:
		char dofPosition;
		const DOFS *outer_dofs;
		void next_available_dof() noexcept;
	public:
 		friend std::ostream &operator<<(std::ostream &out, const DOFS::iterator& dofs_iter) noexcept;
		iterator(char dofPosition, const DOFS *outer_dofs) noexcept;

		bool operator==(const iterator& x) const noexcept {
			return dofPosition == x.dofPosition;
		}

		bool operator!=(const iterator& x) const noexcept {
			return !(*this == x);
		}

		const DOF operator*() const noexcept {
			return DOF::findByPosition(dofPosition);
		}

		iterator& operator++() noexcept {
			dofPosition++;
			next_available_dof();
			return *this;
		}

		iterator operator++(int) noexcept {
			iterator tmp = *this;
			++*this;
			return tmp;
		}
	};
	iterator begin() const noexcept;
	const iterator end() const noexcept;
};

DOFS operator+(const DOFS lhs, const DOFS& rhs) noexcept;
DOFS operator-(const DOFS lhs, const DOFS& rhs) noexcept;
DOFS operator+(const DOFS lhs, const DOF& rhs) noexcept;
DOFS operator-(const DOFS lhs, const DOF& rhs) noexcept;
DOFS operator&(const DOFS lhs, const DOF& rhs) noexcept;

std::ostream &operator<<(std::ostream &out, const DOFS& dofs) noexcept;
std::ostream &operator<<(std::ostream &out, const DOFS::iterator& dofs_iter) noexcept;

/**
 * Sparse matrix between two dofs (of the same node), or the same dof with itself.
 */
class DOFMatrix final {
public:
		std::unordered_map<std::pair<DOF, DOF>, double,
				boost::hash<std::pair<int, int>>>componentByDofs;
		const MatrixType matrixType;
		DOFMatrix(MatrixType matrixType = MatrixType::FULL) noexcept;
		void addComponent(const DOF dof1, const DOF dof2, const double value);
		double findComponent(const DOF dof1, const DOF dof2) const noexcept;
		bool hasTranslations() const noexcept;
		bool hasRotations() const noexcept;
		bool isDiagonal() const noexcept;
		bool isSymmetric() const noexcept;
		/**
		 * Checks if diagonal is filled with DBL_MAX values
		 * useful (for now) to understand if an element is totally rigid
		 **/
		bool isMaxDiagonal() const noexcept;
		bool isEmpty() const noexcept;
		bool isZero() const noexcept;
		void setAllZero() noexcept;
		DOFMatrix transposed() const noexcept;
		bool isEqual(const DOFMatrix& other) const noexcept;
		std::vector<double> diagonal(bool addRotationsIfNotPresent) const noexcept;
		std::vector<double> asUpperTriangularColumnsVector(bool addRotationsIfNotPresent) const noexcept;
		std::vector<double> asColumnsVector(bool addRotationsIfNotPresent) const noexcept;
};


/**
 * This class regroups one coefficient by DOF.
 * It's basically a double[6] with a few extra security test.
 */
class DOFCoefs {
private:
    std::array<double, 6> coefs;
public:
    DOFCoefs(double dx = 0, double dy = 0, double dz = 0, double rx = 0, double ry = 0,
            double rz = 0) noexcept;

    DOFCoefs(DOFS dofs, double val = 0) noexcept;
    DOFCoefs(DOF dof, double val = 0) noexcept;
    DOFS getDOFS() const noexcept;
    bool isEmpty() const noexcept;
    double getValue(const DOF dof) const noexcept;
    void setValue(const DOF dof, double val) noexcept;
    DOFCoefs& operator+=(const DOFCoefs&) noexcept;
    DOFCoefs& operator*=(const double factor) noexcept;
    double operator[](const unsigned char) noexcept;
    bool operator<(const DOFCoefs& other) const noexcept;
    bool operator==(const DOFCoefs& other) const noexcept;
};



} /* namespace vega */


namespace std {

template<>
struct hash<vega::DOF> {
	size_t operator()(const vega::DOF& dof) const noexcept {
		return hash<int>()(dof.position);
	}
};

template<>
struct hash<vega::DOFS> {
	size_t operator()(const vega::DOFS& dofs) const noexcept {
		return hash<char>()(static_cast<char>(dofs));
	}
};

} /* namespace std */

#endif /* DOF_H_ */
