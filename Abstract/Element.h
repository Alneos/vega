/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Element.h
 *
 *  Created on: Sep 11, 2013
 *      Author: devel
 */

#ifndef ELEMENT_H_
#define ELEMENT_H_

#include "Material.h"
#include "Mesh.h"
#include "Dof.h"

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
//win
#define _USE_MATH_DEFINES 
#include <math.h>

namespace vega {

class Model;

class ModelType {
private:
	string name;
	SpaceDimension dimension;
	ModelType(string name, const SpaceDimension dimension);
	public:
	inline bool operator==(const ModelType& rhs) {
		return this->name == rhs.name;
	}

	static const ModelType PLANE_STRESS;
	static const ModelType PLANE_STRAIN;
	static const ModelType AXISYMMETRIC;
	static const ModelType TRIDIMENSIONAL;
	static const ModelType TRIDIMENSIONAL_SI;
};

class ElementSet: public Identifiable<ElementSet> {
	friend ostream &operator<<(ostream&, const ElementSet&);    //output
public:
	enum Type {
		DISCRETE_0D,
		DISCRETE_1D,
		NODAL_MASS,
		CIRCULAR_SECTION_BEAM,
		RECTANGULAR_BEAM,
		I_SECTION_BEAM,
		GENERIC_SECTION_BEAM,
		SHELL,
		CONTINUUM,
		STIFFNESS_MATRIX,
		MASS_MATRIX,
		DAMPING_MATRIX,
		UNKNOWN,
	};
	protected:
	Model& model;
	ElementSet(Model&, Type, const ModelType* modelType = nullptr,
			int original_id = NO_ORIGINAL_ID);
	public:
	static const string name;
	static const map<Type, string> stringByType;
	const Type type;
	const ModelType* modelType;
	CellGroup* cellGroup;
	std::shared_ptr<Material> material;
	virtual ~ElementSet() {
	}
	void assignMaterial(int materialId);
	void assignMaterial(std::shared_ptr<Material> material) {
		this->material = material;
	}
	void assignCellGroup(CellGroup *cellGroup);
	const ModelType getModelType() const;
	virtual bool validate() const override;
	virtual std::shared_ptr<ElementSet> clone() const = 0;
	virtual const std::set<int> nodePositions() const {
		return cellGroup->nodePositions();
	}
	virtual const DOFS getDOFSForNode(int nodePosition) const = 0;
	virtual double getAdditionalRho() const {
		return 0;
	}
	virtual bool isBeam() const {
		return false;
	}
	virtual bool isShell() const {
		return false;
	}
	virtual bool isDiscrete() const {
		return false;
	}
	virtual bool isMatrixElement() const {
		return false;
	}
};

class Beam: public ElementSet {

public:
	enum BeamModel {
		EULER,
		TIMOSHENKO
	};
	BeamModel beamModel;
	protected:
	double additional_mass;
	Beam(Model&, Type type, ModelType* modelType = nullptr, BeamModel beamModel = EULER,
			double additionalMass = 0.0, int original_id = NO_ORIGINAL_ID);
	public:
	double getAdditionalRho() const {
		return additional_mass / std::max(getAreaCrossSection(), DBL_MIN);
	}
	bool isBeam() const override final {
		return true;
	}
	virtual double getAreaCrossSection() const = 0;
	virtual double getMomentOfInertiaY() const = 0;
	virtual double getMomentOfInertiaZ() const = 0;
	virtual double getTorsionalConstant() const = 0;
	virtual double getShearAreaFactorY() const = 0;
	virtual double getShearAreaFactorZ() const = 0;
	const DOFS getDOFSForNode(int nodePosition) const override final;
	virtual ~Beam() {
	}

};

class CircularSectionBeam: public Beam {

public:
	const double radius;
	CircularSectionBeam(Model&, double _radius, BeamModel beamModel = EULER,
			double additional_mass = 0, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<ElementSet> clone() const override;
	double getAreaCrossSection() const override;
	double getMomentOfInertiaY() const override;
	double getMomentOfInertiaZ() const override;
	double getTorsionalConstant() const override;
	double getShearAreaFactorY() const override;
	double getShearAreaFactorZ() const override;
};

class GenericSectionBeam: public Beam {
private:
	const double area_cross_section;
	const double moment_of_inertia_Y;
	const double moment_of_inertia_Z;
	const double torsional_constant;
	const double shear_area_factor_Y;
	const double shear_area_factor_Z;
public:
	GenericSectionBeam(Model& model, double area_cross_section, double moment_of_inertia_Y,
			double moment_of_inertia_Z, double torsional_constant, double shear_area_factor_Y,
			double shear_area_factor_Z, BeamModel beamModel = EULER, double additional_mass = 0,
			int original_id = NO_ORIGINAL_ID);
	/* dans AFFE_CARA_ELEM les coefficients de cisaillement
	 AY et AZ ne doivent pas etre entres comme le font
	 tous les autres codes (i.e. un coefficient <=1.) mais a l'inverse (i.e. >=1.)*/
	double getAreaCrossSection() const;
	double getMomentOfInertiaY() const override;
	double getMomentOfInertiaZ() const override;
	double getTorsionalConstant() const override;
	double getShearAreaFactorY() const override;
	double getShearAreaFactorZ() const override;
	double getInvShearAreaFactorY() const;
	double getInvShearAreaFactorZ() const;
	std::shared_ptr<ElementSet> clone() const;
};

class RectangularSectionBeam: public Beam {

public:
	const double width;
	const double height;
	RectangularSectionBeam(Model&, double width, double height, BeamModel beamModel = EULER,
			double additional_mass = 0, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<ElementSet> clone() const override;
	double getAreaCrossSection() const override;
	double getMomentOfInertiaY() const override;
	double getMomentOfInertiaZ() const override;
	double getTorsionalConstant() const override;
	double getShearAreaFactorY() const override;
	double getShearAreaFactorZ() const override;
	virtual ~RectangularSectionBeam() {
	}
};

/**
 *     '''
 *   see http://en.wikipedia.org/wiki/I-beam for attribute terms
 *   see http://en.wikipedia.org/wiki/Second_moment_of_area
 *   '''
 */
class ISectionBeam: public Beam {

public:
	const double upper_flange_width;
	const double lower_flange_width;
	const double upper_flange_thickness;
	const double lower_flange_thickness;
	const double beam_height;
	const double web_thickness;

	ISectionBeam(Model& model, double upper_flange_width, double lower_flange_width,
			double upper_flange_thickness, double lower_flange_thickness, double beam_height,
			double web_thickness, BeamModel beamModel = EULER, double additional_mass = 0,
			int original_id = NO_ORIGINAL_ID);

	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new ISectionBeam(*this));
	}

	double getAreaCrossSection() const override;
	double getMomentOfInertiaY() const override;
	double getMomentOfInertiaZ() const override;
	double getTorsionalConstant() const override;
	double getShearAreaFactorY() const override;
	double getShearAreaFactorZ() const override;
	virtual ~ISectionBeam() {
	}
};

class Shell: public ElementSet {

public:
	const double thickness;
	protected:
	double additional_mass;
	public:
	Shell(Model&, double thickness, double additional_mass = 0, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new Shell(*this));
	}
	double getAdditionalRho() const {
		return additional_mass / std::max(thickness, DBL_MIN);
	}
	bool isShell() const override final {
		return true;
	}
	const DOFS getDOFSForNode(int nodePosition) const override final;
	virtual ~Shell() {
	}
};

class Continuum: public ElementSet {

public:
	Continuum(Model&, const ModelType* modelType, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new Continuum(*this));
	}
	const DOFS getDOFSForNode(int nodePosition) const override final;
	virtual ~Continuum() {
	}
};

class Discrete: public ElementSet {
public:
	static const double NOT_BOUNDED;
	const bool symmetric = false;
	Discrete(Model&, ElementSet::Type type, bool symmetric = false, int original_id = NO_ORIGINAL_ID);
	bool isDiscrete() const override final {
		return true;
	}
	virtual bool hasTranslations() const = 0;
	virtual bool hasRotations() const = 0;
	const DOFS getDOFSForNode(int nodePosition) const override final;
	virtual ~Discrete() {
	}
};

class DiscretePoint final: public Discrete {
private:
	DOFMatrix stiffness;
	DOFMatrix mass;
	DOFMatrix damping;
public:
	DiscretePoint(Model&, double x, double y, double z, double rx = NOT_BOUNDED, double ry =
			NOT_BOUNDED, double rz = NOT_BOUNDED, bool symmetric = true, int original_id = NO_ORIGINAL_ID);
	DiscretePoint(Model&, vector<double> components = vector<double>(), bool symmetric = true, int original_id =
			NO_ORIGINAL_ID);
	bool hasTranslations() const override;
	bool hasRotations() const override;
	void addStiffness(DOF rowcode, DOF colcode,	double value);
	double findStiffness(DOF rowcode, DOF colcode) const;
	void addComponent(DOF, double value);
	vector<double> asVector(bool addRotationsIfNotPresent = false);
	std::shared_ptr<ElementSet> clone() const override;
};

class DiscreteSegment final : public Discrete {
private:
	DOFMatrix stiffness[2][2];
	DOFMatrix mass[2][2];
	DOFMatrix damping[2][2];
public:
	DiscreteSegment(Model&, bool symmetric = true, int original_id = NO_ORIGINAL_ID);
	bool hasTranslations() const override;
	bool hasRotations() const override;
	void addStiffness(int rowindex, int colindex, DOF rowdof, DOF coldof,
			double value);
	double findStiffness(int rowindex, int colindex, DOF rowdof, DOF coldof) const;
	vector<double> asVector(bool addRotationsIfNotPresent = false);
	std::shared_ptr<ElementSet> clone() const override;
};

class NodalMass: public ElementSet {
	const double m;
	public:
	const double ixx;
	const double iyy;
	const double izz;
	const double ixy;
	const double iyz;
	const double ixz;
	const double ex;
	const double ey;
	const double ez;

	NodalMass(Model&, double m = 0, double ixx = 0, double iyy = 0, double izz = 0, double ixy = 0,
			double iyz = 0, double ixz = 0, double ex = 0, double ey = 0, double ez = 0,
			int original_id = NO_ORIGINAL_ID);
	/**
	 * Get mass (in kg)
	 */
	double getMass() const;

	/**
	 * Get mass as equivalent force (in N)
	 */
	double getMassAsForce() const;

	const DOFS getDOFSForNode(int nodePosition) const override final;

	~NodalMass();

	inline std::shared_ptr<ElementSet> clone() const {
		return std::shared_ptr<ElementSet>(new NodalMass(*this));
	}
};

/* Matrix for a group nodes.*/
class MatrixElement : public ElementSet {
private:
	std::map<std::pair<int, int>, shared_ptr<DOFMatrix>> submatrixByNodes;
	bool symmetric = false;
public:
	MatrixElement(Model&, Type type, bool symmetric = false, int original_id = NO_ORIGINAL_ID);
	void addComponent(const int nodeid1, const DOF dof1, const int nodeid2, const DOF dof2, const double value);
	const shared_ptr<DOFMatrix> findSubmatrix(const int nodePosition1, const int nodePosition2) const;
	const std::set<int> nodePositions() const override;
	const std::set<std::pair<int, int>> nodePairs() const;
	const std::set<std::pair<int, int>> findInPairs(int nodePosition) const;
	const DOFS getDOFSForNode(int nodePosition) const override final;
	bool isMatrixElement() const override final {
		return true;
	}
	virtual bool validate() const override {
		return true;
	}
	virtual ~MatrixElement() {
	}
};

class StiffnessMatrix : public MatrixElement {
public:
	StiffnessMatrix(Model&, int original_id = NO_ORIGINAL_ID);
	void addStiffness(const int nodeid1, const DOF dof1, const int nodeid2, const DOF dof2, const double stiffness);
	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new StiffnessMatrix(*this));
	}
};

class MassMatrix : public MatrixElement {
public:
	MassMatrix(Model&, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new MassMatrix(*this));
	}
};

class DampingMatrix : public MatrixElement {
public:
	DampingMatrix(Model&, int original_id = NO_ORIGINAL_ID);
	void addDamping(const int nodeid1, const DOF dof1, const int nodeid2, const DOF dof2, const double damping);
	std::shared_ptr<ElementSet> clone() const override {
		return std::shared_ptr<ElementSet>(new DampingMatrix(*this));
	}
};

} /* namespace vega */
#endif /* ELEMENT_H_ */
