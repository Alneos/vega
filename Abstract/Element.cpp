/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Element.cpp
 *
 *  Created on: Sep 11, 2013
 *      Author: devel
 */

#include "Element.h"
#include "Model.h"
#include "SolverInterfaces.h"
#include <boost/assign.hpp>
#include <ciso646>
//win
#define _USE_MATH_DEFINES
#include <math.h>

namespace vega {

using namespace std;

const ModelType ModelType::PLANE_STRESS = ModelType("PLANE_STRESS", SpaceDimension::DIMENSION_2D);
const ModelType ModelType::PLANE_STRAIN = ModelType("PLANE_STRAIN", SpaceDimension::DIMENSION_2D);
const ModelType ModelType::AXISYMMETRIC = ModelType("AXISYMMETRIC", SpaceDimension::DIMENSION_2D);
const ModelType ModelType::TRIDIMENSIONAL = ModelType("TRIDIMENSIONAL",
		SpaceDimension::DIMENSION_3D);
const ModelType ModelType::TRIDIMENSIONAL_SI = ModelType("TRIDIMENSIONAL_SI",
		SpaceDimension::DIMENSION_3D);

ModelType::ModelType(string name, const SpaceDimension dimension) :
		name(name), dimension(dimension) {
}

ElementSet::ElementSet(Model& model, Type type, const ModelType& modelType, int original_id) :
		Identifiable(original_id), model(model), type(type), modelType(modelType), cellGroup(nullptr), material(
		nullptr) {
}

const string ElementSet::name = "ElementSet";

const map<ElementSet::Type, string> ElementSet::stringByType = {
        { ElementSet::Type::DISCRETE_0D, "DISCRETE_0D" },
        { ElementSet::Type::DISCRETE_1D, "DISCRETE_1D" },
        { ElementSet::Type::NODAL_MASS, "NODAL_MASS" },
        { ElementSet::Type::CIRCULAR_SECTION_BEAM, "CIRCULAR_SECTION_BEAM" },
        { ElementSet::Type::RECTANGULAR_SECTION_BEAM, "RECTANGULAR_SECTION_BEAM" },
        { ElementSet::Type::I_SECTION_BEAM, "I_SECTION_BEAM" },
        { ElementSet::Type::GENERIC_SECTION_BEAM, "GENERIC_SECTION_BEAM" },
        { ElementSet::Type::STRUCTURAL_SEGMENT, "STRUCTURAL_SEGMENT" },
        { ElementSet::Type::SHELL, "SHELL" },
        { ElementSet::Type::COMPOSITE, "COMPOSITE" },
        { ElementSet::Type::CONTINUUM, "CONTINUUM" },
        { ElementSet::Type::STIFFNESS_MATRIX, "STIFFNESS_MATRIX" },
        { ElementSet::Type::MASS_MATRIX, "MASS_MATRIX" },
        { ElementSet::Type::DAMPING_MATRIX, "DAMPING_MATRIX" },
        { ElementSet::Type::RBAR, "RBAR"},
        { ElementSet::Type::RBE3, "RBE3"},
        { ElementSet::Type::LMPC, "LMPC"},
        { ElementSet::Type::SCALAR_SPRING, "SCALAR_SPRING"},
        { ElementSet::Type::UNKNOWN, "UNKNOWN" },
};

ostream &operator<<(ostream &out, const ElementSet& elementSet) {
	out << to_str(elementSet);
	return out;
}

void ElementSet::assignMaterial(int materialId) {
	this->material = this->model.getOrCreateMaterial(materialId);
}

void ElementSet::assignCellGroup(shared_ptr<CellGroup> cellGroup) {
	this->cellGroup = cellGroup;
}

const ModelType ElementSet::getModelType() const {
	return this->modelType;
}

bool ElementSet::validate() const {
	bool validElement = true;

	if (cellGroup == nullptr) {
		cerr << *this << " has no cellGroup assigned." << endl;
		validElement = false;
	}

	if (material == nullptr && model.configuration.partitionModel) {
		cerr << *this << " has no material assigned, "
				<< "and config. param partitionModel is set to True." << endl;
		validElement = false;
	}
	return validElement;
}

Continuum::Continuum(Model& model, const ModelType& modelType, int original_id) :
		ElementSet(model, ElementSet::Type::CONTINUUM, modelType, original_id) {

}

Beam::Beam(Model& model, Type type, const ModelType& modelType, BeamModel beamModel,
		double additional_mass, int original_id) :
		ElementSet(model, type, modelType, original_id), beamModel(beamModel), additional_mass(
				additional_mass) {
}

const DOFS Beam::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	if (this->isBar()) {
	    return DOFS::TRANSLATIONS;
	}

    return DOFS::ALL_DOFS;
}

RecoveryPoint::RecoveryPoint(const Model& model, const double lx, const double ly, const double lz) :
    model(model), localCoords(lx, ly, lz) {
}

const VectorialValue RecoveryPoint::getLocalCoords() const {
    return localCoords;
}

const VectorialValue RecoveryPoint::getGlobalCoords(const int cellId) const {
    UNUSEDV(cellId);
    const Cell& cell = model.mesh.findCell(model.mesh.findCellPosition(cellId));
    if (cell.nodeIds.size() != 2) {
        throw runtime_error("Recovery point currently implemented only for two-node cells.");
    }
    if (localCoords.x() < 0 or localCoords.x() > 1) {
        throw runtime_error("X local axis is currently considered as normalized.");
    }
    const Node& node1 = model.mesh.findNode(cell.nodePositions[0]);
    const Node& node2 = model.mesh.findNode(cell.nodePositions[1]);
    VectorialValue segment = VectorialValue(node2.x-node1.x, node2.y-node1.y, node2.z-node1.z);
    if (segment.iszero()) {
        throw runtime_error("Recovery point requested over zero-length cell.");
    }
    VectorialValue localPoint(localCoords.x()*segment.norm(), localCoords.y(), localCoords.z());
    if (cell.orientation != nullptr)
        return cell.orientation->positionToGlobal(localPoint);
    else {
        return localPoint + VectorialValue(node1.x, node1.y, node1.z);
    }
}

CircularSectionBeam::CircularSectionBeam(Model& model, double _radius, BeamModel beamModel,
		double additional_mass, int original_id) :
		Beam(model, ElementSet::Type::CIRCULAR_SECTION_BEAM, model.modelType, beamModel, additional_mass, original_id), radius(
				_radius) {
}

shared_ptr<ElementSet> CircularSectionBeam::clone() const {
	return make_shared<CircularSectionBeam>(*this);
}

double CircularSectionBeam::getAreaCrossSection() const {
	return M_PI * radius * radius;
}

double CircularSectionBeam::getMomentOfInertiaY() const {
	return pow(radius, 4) * M_PI / 4;
}

double CircularSectionBeam::getMomentOfInertiaZ() const {
	return getMomentOfInertiaY();
}

double CircularSectionBeam::getTorsionalConstant() const {
	// http://en.wikipedia.org/wiki/Torsion_constant
	return pow(radius, 4) * M_PI / 2;
}

double CircularSectionBeam::getShearAreaFactorY() const {
	return 10.0/9.0;
}

double CircularSectionBeam::getShearAreaFactorZ() const {
	return 10.0/9.0;
}

TubeSectionBeam::TubeSectionBeam(Model& model, double _radius, double _thickness, BeamModel beamModel,
		double additional_mass, int original_id) :
		Beam(model, ElementSet::Type::TUBE_SECTION_BEAM, model.modelType, beamModel, additional_mass, original_id), radius(
				_radius), thickness(_thickness) {
}

shared_ptr<ElementSet> TubeSectionBeam::clone() const {
	return make_shared<TubeSectionBeam>(*this);
}

double TubeSectionBeam::getAreaCrossSection() const {
	return M_PI * (pow(radius, 2) - pow(thickness, 2)) ;
}

double TubeSectionBeam::getMomentOfInertiaY() const {
	return (pow(radius, 4) - pow(thickness, 4)) * M_PI / 4;
}

double TubeSectionBeam::getMomentOfInertiaZ() const {
	return getMomentOfInertiaY();
}

double TubeSectionBeam::getTorsionalConstant() const {
	// http://en.wikipedia.org/wiki/Torsion_constant
	return (pow(radius, 4) - pow(thickness, 4)) * M_PI / 2;
}

double TubeSectionBeam::getShearAreaFactorY() const {
	return 10.0/9.0;
}

double TubeSectionBeam::getShearAreaFactorZ() const {
	return 10.0/9.0;
}

RectangularSectionBeam::RectangularSectionBeam(Model& model, double _width, double _height,
		BeamModel beamModel, double additional_mass, int original_id) :
		Beam(model, ElementSet::Type::RECTANGULAR_SECTION_BEAM, model.modelType, beamModel, additional_mass, original_id), width(_width), height(
				_height) {

}

double RectangularSectionBeam::getAreaCrossSection() const {
	return width * height;
}

double RectangularSectionBeam::getMomentOfInertiaY() const {
	return height * pow(width, 3) / 12;
}

double RectangularSectionBeam::getMomentOfInertiaZ() const {
	return pow(height, 3) * width / 12;
}

double RectangularSectionBeam::getTorsionalConstant() const {
	//https://en.wikipedia.org/wiki/Torsion_constant#Rectangle
	//if (width > height) {
	//	return width * pow(height, 3) / 3;
	//} else {
	//	return height * pow(width, 3) / 3;
	//}

	return width*pow(height, 3)*(1.0/3 -0.21*height*(1- pow(height,4)/(12*pow(width,4)))/width);
}

double RectangularSectionBeam::getShearAreaFactorY() const {
	return 6.0/5.0;
}

double RectangularSectionBeam::getShearAreaFactorZ() const {
	return 6.0/5.0;
}

shared_ptr<ElementSet> RectangularSectionBeam::clone() const {
	return make_shared<RectangularSectionBeam>(*this);
}

GenericSectionBeam::GenericSectionBeam(Model& model, double area_cross_section,
		double moment_of_inertia_Y, double moment_of_inertia_Z, double torsional_constant,
		double shear_area_factor_Y, double shear_area_factor_Z, BeamModel beamModel,
		double additional_mass, int original_id) :
		Beam(model, ElementSet::Type::GENERIC_SECTION_BEAM, model.modelType, beamModel, additional_mass, original_id), area_cross_section(
				area_cross_section), moment_of_inertia_Y(moment_of_inertia_Y), moment_of_inertia_Z(
				moment_of_inertia_Z), torsional_constant(torsional_constant), shear_area_factor_Y(
				shear_area_factor_Y), shear_area_factor_Z(shear_area_factor_Z) {

}

shared_ptr<ElementSet> GenericSectionBeam::clone() const {
	return make_shared<GenericSectionBeam>(*this);
}
double GenericSectionBeam::getAreaCrossSection() const {
	return area_cross_section;
}
double GenericSectionBeam::getMomentOfInertiaY() const {
	return moment_of_inertia_Y;
}
double GenericSectionBeam::getMomentOfInertiaZ() const {
	return moment_of_inertia_Z;
}
double GenericSectionBeam::getTorsionalConstant() const {
	return torsional_constant;
}
double GenericSectionBeam::getShearAreaFactorY() const {
	return shear_area_factor_Y;
}
double GenericSectionBeam::getShearAreaFactorZ() const {
	return shear_area_factor_Z;
}
double GenericSectionBeam::getInvShearAreaFactorY() const {
	return (!is_zero(shear_area_factor_Y)) ? 1 / shear_area_factor_Y : Globals::UNAVAILABLE_DOUBLE;
}
double GenericSectionBeam::getInvShearAreaFactorZ() const {
	return (!is_zero(shear_area_factor_Z)) ? 1 / shear_area_factor_Z : Globals::UNAVAILABLE_DOUBLE;
}




Shell::Shell(Model& model, double thickness, double additional_mass, int original_id) :
		ElementSet(model, ElementSet::Type::SHELL, model.modelType, original_id), thickness(thickness), additional_mass(
				additional_mass) {
}

const DOFS Shell::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::ALL_DOFS;
}

CompositeLayer::CompositeLayer(int materialId, double thickness, double orientation) :
		_materialId(materialId), _thickness(thickness), _orientation(orientation) {
}

Composite::Composite(Model& model, int original_id) :
		ElementSet(model, ElementSet::Type::COMPOSITE, model.modelType, original_id) {
}

void Composite::addLayer(int materialId, double thickness, double orientation) {
    layers.push_back(CompositeLayer(materialId, thickness, orientation));
}

double Composite::getTotalThickness() {
    double total = 0.0;
    for(const auto& layer : layers) {
        total += layer.getThickness();
    }
    return total;
}

const DOFS Composite::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::ALL_DOFS;
}

const DOFS Continuum::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	return DOFS::TRANSLATIONS;
}

Discrete::Discrete(Model& model, ElementSet::Type type, bool symmetric, int original_id) :
		ElementSet(model, type, model.modelType, original_id), symmetric(symmetric) {
}

const double Discrete::NOT_BOUNDED = -DBL_MAX;

const DOFS Discrete::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	// LD : meaning here is that the node has a variable for the dof, not that it has stiffness
	// over it
	return DOFS::ALL_DOFS;
}

DiscretePoint::DiscretePoint(Model& model, vector<double> coefficients, bool symmetric,
		int original_id) :
		Discrete(model, ElementSet::Type::DISCRETE_0D, symmetric, original_id), stiffness(
				symmetric), mass(symmetric), damping(symmetric) {
	// LD TODO : remove this vector parameter
	for (int i = 0; i < 6 && i < static_cast<int>(coefficients.size()); i++) {
		this->addComponent(DOF::findByPosition(i), coefficients[i]);
	}
}

DiscretePoint::DiscretePoint(Model& model, double x, double y, double z, double rx, double ry,
		double rz, bool symmetric,
		int original_id) :
		Discrete(model, ElementSet::Type::DISCRETE_0D, symmetric, original_id), stiffness(
				symmetric), mass(
				symmetric), damping(symmetric) {
	// LD TODO : remove this method
	if (!is_equal(x, NOT_BOUNDED)) {
		this->addComponent(DOF::DX, x);
	}
	if (!is_equal(y, NOT_BOUNDED)) {
		this->addComponent(DOF::DY, y);
	}
	if (!is_equal(z, NOT_BOUNDED)) {
		this->addComponent(DOF::DZ, x);
	}
	if (!is_equal(rx, NOT_BOUNDED)) {
		this->addComponent(DOF::RX, rx);
	}
	if (!is_equal(ry, NOT_BOUNDED)) {
		this->addComponent(DOF::RY, ry);
	}
	if (!is_equal(rz, NOT_BOUNDED)) {
		this->addComponent(DOF::RZ, rz);
	}
}

shared_ptr<ElementSet> DiscretePoint::clone() const {
	return make_shared<DiscretePoint>(*this);
}

void DiscretePoint::addComponent(DOF code, double value) {
	// LD TODO : remove this ambigous method, replace with this line
	stiffness.addComponent(code, code, value);
}

const vector<double> DiscretePoint::asStiffnessVector(bool addRotationsIfNotPresent) const {
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	for (int i = 0; i < ncomp; i++) {
		DOF code = DOF::findByPosition(i);
		auto codeIter = this->stiffness.componentByDofs.find(make_pair(code, code));
		double value;
		if (codeIter != this->stiffness.componentByDofs.end()) {
			value = codeIter->second;
		} else {
			value = 0;
		}
		result.push_back(value);
	}
	return result;
}

bool DiscretePoint::hasTranslations() const {
	return stiffness.hasTranslations() or mass.hasTranslations() or damping.hasTranslations();
}

bool DiscretePoint::hasRotations() const {
	return stiffness.hasRotations() or mass.hasRotations() or damping.hasRotations();
}

double DiscretePoint::findStiffness(DOF rowdof, DOF coldof) const {
	double result;
	auto codeIter = stiffness.componentByDofs.find(make_pair(rowdof, coldof));
	if (codeIter != stiffness.componentByDofs.end()) {
		result = codeIter->second;
	} else {
		if (symmetric) {
			codeIter = stiffness.componentByDofs.find(make_pair(coldof, rowdof));
			if (codeIter != stiffness.componentByDofs.end()) {
				result = codeIter->second;
			} else {
				result = 0.0;
			}
		} else {
			result = 0.0;
		}
	}
	return result;
}

void DiscretePoint::addStiffness(DOF rowdof, DOF coldof, double value) {
	this->stiffness.componentByDofs[make_pair(rowdof, coldof)] = value;
}

DiscreteSegment::DiscreteSegment(Model& model, bool symmetric, int original_id) :
		Discrete(model, ElementSet::Type::DISCRETE_1D, symmetric, original_id) {
	for (int i = 0; i < 2; i++){
		for (int j = 0; j < 2; j++){
			stiffness[i][j] = symmetric;
			mass[i][j] = symmetric;
			damping[i][j] = symmetric;
		}
	}
}

shared_ptr<ElementSet> DiscreteSegment::clone() const {
	return make_shared<DiscreteSegment>(*this);
}

bool DiscreteSegment::hasTranslations() const {
	bool hasTranslations = false;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			if (stiffness[i][j].hasTranslations() or mass[i][j].hasTranslations()
					or damping[i][j].hasTranslations()) {
				hasTranslations = true;
				break;
			}
		}
	}
	return hasTranslations;
}

bool DiscreteSegment::hasRotations() const {
	bool hasRotations = false;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			if (stiffness[i][j].hasRotations() or mass[i][j].hasRotations()
					or damping[i][j].hasRotations()) {
				hasRotations = true;
				break;
			}
		}
	}
	return hasRotations;
}

double DiscreteSegment::findStiffness(int rowindex, int colindex, DOF rowdof, DOF coldof) const {
	double result;
	const DOFMatrix* matrix = &stiffness[rowindex][colindex];
	auto codeIter = matrix->componentByDofs.find(make_pair(rowdof, coldof));
	if (codeIter != matrix->componentByDofs.end()) {
		result = codeIter->second;
	} else {
		if (symmetric) {
			matrix = &stiffness[colindex][rowindex];
			codeIter = matrix->componentByDofs.find(make_pair(coldof, rowdof));
			if (codeIter != matrix->componentByDofs.end()) {
				result = codeIter->second;
			} else {
				result = 0.0;
			}
		} else {
			result = 0.0;
		}
	}
	return result;
}

void DiscreteSegment::addStiffness(int rowindex, int colindex, DOF rowdof, DOF coldof, double value) {
	DOFMatrix* matrix = &stiffness[rowindex][colindex];
	matrix->componentByDofs[make_pair(rowdof, coldof)] = value;
}

const vector<double> DiscreteSegment::asStiffnessVector(bool addRotationsIfNotPresent) const {
	// LD TODO : at least rename this method (it only does stiffness and with aster convention!!)
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	int max_row_element_index = 0;
	for (int colindex = 0; colindex < 2; ++colindex) {
		for (int coldof = 0; coldof < ncomp; coldof++) {
			DOF colcode = DOF::findByPosition(coldof);
			max_row_element_index++;
			int row_element_index = 0;
			for (int rowindex = 0; rowindex < 2; ++rowindex) {
				for (int rowdof = 0; rowdof < ncomp; rowdof++) {
					row_element_index++;
					if (row_element_index > max_row_element_index) {
						break;
					}
					DOF rowcode = DOF::findByPosition(rowdof);
					double value = findStiffness(rowindex, colindex, rowcode, colcode);
					result.push_back(value);
				}
			}
		}
	}
	return result;
}


StructuralSegment::StructuralSegment(Model& model, bool symmetric, int original_id) :
				Discrete(model, ElementSet::Type::STRUCTURAL_SEGMENT, symmetric, original_id) {
	this->stiffness = symmetric;
	this->mass = symmetric;
	this->damping = symmetric;
}

bool StructuralSegment::hasTranslations() const {
	return (stiffness.hasTranslations() or mass.hasTranslations() or damping.hasTranslations());
}

bool StructuralSegment::hasRotations() const {
	return (stiffness.hasRotations() or mass.hasRotations() or damping.hasRotations());
}

bool StructuralSegment::hasStiffness() const {
	return !(stiffness.isEmpty());
}

bool StructuralSegment::isDiagonalRigid() const {
	return stiffness.isMaxDiagonal();
}

bool StructuralSegment::hasMass() const {
	return !(mass.isEmpty());
}

bool StructuralSegment::hasDamping() const {
	return !(damping.isEmpty());
}

void StructuralSegment::addStiffness(DOF rowdof, DOF coldof, double value){
	stiffness.componentByDofs[make_pair(rowdof, coldof)] = value;
}
void StructuralSegment::addMass(DOF rowdof, DOF coldof, double value){
	mass.componentByDofs[make_pair(rowdof, coldof)] = value;
}
void StructuralSegment::addDamping(DOF rowdof, DOF coldof, double value){
	damping.componentByDofs[make_pair(rowdof, coldof)] = value;
}

void StructuralSegment::setAllZero() {
    for (auto& entry : stiffness.componentByDofs) {
        entry.second = 0.0;
    }
}

double StructuralSegment::findStiffness(DOF rowdof, DOF coldof) const{
	double result=0.0;
	auto itFind = stiffness.componentByDofs.find(make_pair(rowdof, coldof));
	if (itFind != stiffness.componentByDofs.end()) {
		result = itFind->second;
	} else {
		if (symmetric){
			itFind = stiffness.componentByDofs.find(make_pair(coldof, rowdof));
			if (itFind != stiffness.componentByDofs.end()) {
				result = itFind->second;
			}
		}
	}
	return result;
}

double StructuralSegment::findDamping(DOF rowdof, DOF coldof) const{
	double result=0.0;
	auto itFind = damping.componentByDofs.find(make_pair(rowdof, coldof));
	if (itFind != damping.componentByDofs.end()) {
		result = itFind->second;
	}else{
		if (symmetric){
			itFind = damping.componentByDofs.find(make_pair(coldof, rowdof));
			if (itFind != damping.componentByDofs.end()) {
				result = itFind->second;
			}
		}
	}
	return result;
}

const vector<double> StructuralSegment::asStiffnessVector(bool addRotationsIfNotPresent) const {
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	int max_row_element_index = 0;
	for (int colindex = 0; colindex < 2; ++colindex) {
		for (int coldof = 0; coldof < ncomp; coldof++) {
			DOF colcode = DOF::findByPosition(coldof);
			max_row_element_index++;
			int row_element_index = 0;
			for (int rowindex = 0; rowindex < 2; ++rowindex) {
				for (int rowdof = 0; rowdof < ncomp; rowdof++) {
					row_element_index++;
					if (row_element_index > max_row_element_index) {
						break;
					}
					DOF rowcode = DOF::findByPosition(rowdof);
					double value = findStiffness(rowcode, colcode);
					result.push_back(value);
				}
			}
		}
	}
	return result;
}

const vector<double> StructuralSegment::asDampingVector(bool addRotationsIfNotPresent) const {
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	int max_row_element_index = 0;
	for (int colindex = 0; colindex < 2; ++colindex) {
		for (int coldof = 0; coldof < ncomp; coldof++) {
			DOF colcode = DOF::findByPosition(coldof);
			max_row_element_index++;
			int row_element_index = 0;
			for (int rowindex = 0; rowindex < 2; ++rowindex) {
				for (int rowdof = 0; rowdof < ncomp; rowdof++) {
					row_element_index++;
					if (row_element_index > max_row_element_index) {
						break;
					}
					DOF rowcode = DOF::findByPosition(rowdof);
					double value = findDamping(rowcode, colcode);
					result.push_back(value);
				}
			}
		}
	}
	return result;
}


std::shared_ptr<ElementSet> StructuralSegment::clone() const{
	return make_shared<StructuralSegment>(*this);
}





NodalMass::NodalMass(Model& model, double m, double ixx, double iyy, double izz, double ixy,
		double iyz, double ixz, double ex, double ey, double ez, int original_id) :
		ElementSet(model, ElementSet::Type::NODAL_MASS, model.modelType, original_id), m(m), ixx(ixx), iyy(iyy), izz(izz), ixy(
				ixy), iyz(iyz), ixz(ixz), ex(ex), ey(ey), ez(ez) {
}

double NodalMass::getMass() const {
	double mass_multiplier = 1;
	auto it = model.parameters.find(Model::Parameter::MASS_OVER_FORCE_MULTIPLIER);
	if (it != model.parameters.end()) {
		mass_multiplier = it->second;
		assert(!is_zero(it->second));
	}
	return m * mass_multiplier;
}

bool NodalMass::hasTranslations() const {
	return not is_zero(m);
}

bool NodalMass::hasRotations() const {
	return (not is_zero(ixx) or not is_zero(iyy) or not is_zero(izz) or not is_zero(ixy) or not is_zero(iyz) or not is_zero(ixz));
}

const DOFS NodalMass::getDOFSForNode(const int nodePosition) const {
	UNUSEDV(nodePosition);
	DOFS dofs;
	if (hasTranslations()) {
        dofs = DOFS::TRANSLATIONS;
	}
	if (hasRotations()) {
        dofs = DOFS::ALL_DOFS;
	}
	return dofs;
}

double NodalMass::getMassAsForce() const {
	return m;
}

ISectionBeam::ISectionBeam(Model& model, double upper_flange_width_p, double lower_flange_width,
		double upper_flange_thickness_p, double lower_flange_thickness, double beam_height,
		double web_thickness, BeamModel beamModel, double additional_mass, int original_id) :
		Beam(model, ElementSet::Type::I_SECTION_BEAM, model.modelType, beamModel, additional_mass, original_id), upper_flange_width(
				upper_flange_width_p), lower_flange_width(lower_flange_width), upper_flange_thickness(
				upper_flange_thickness_p), lower_flange_thickness(lower_flange_thickness), beam_height(
				beam_height), web_thickness(web_thickness) {
}

double ISectionBeam::getAreaCrossSection() const {
	double h1 = upper_flange_width;
	double h2 = (beam_height - upper_flange_thickness - lower_flange_thickness);
	double h3 = lower_flange_width;
	double e1 = upper_flange_thickness;
	double e2 = web_thickness;
	double e3 = lower_flange_thickness;
	double S = h1 * e1 + h2 * e2 + h3 * e3;
	return S;
}

double ISectionBeam::getMomentOfInertiaY() const {
	double h1 = upper_flange_width;
	double h2 = (beam_height - upper_flange_thickness - lower_flange_thickness);
	double h3 = lower_flange_width;
	double e1 = upper_flange_thickness;
	double e2 = web_thickness;
	double e3 = lower_flange_thickness;
	double iyy = (pow(h1, 3) * e1 + pow(h3, 3) * e3 + h2 * pow(e2, 3)) / 12;
	return iyy;
}

double ISectionBeam::getMomentOfInertiaZ() const {
	double h1 = upper_flange_width;
	double h2 = (beam_height - upper_flange_thickness - lower_flange_thickness);
	double h3 = lower_flange_width;
	double e1 = upper_flange_thickness;
	double e2 = web_thickness;
	double e3 = lower_flange_thickness;
	if (!is_equal(e1 , e3)) {
		throw logic_error("Different flange thickness not yet handled");
	}
	if (!is_equal(h1 , h3)) {
		throw logic_error("Different flange width not yet handled");
	}
	// LD : symplified (but correct, in some special cases) formula
	// https://web.archive.org/web/20161123001455/http://www.had2know.com/technology/I-beam-calculator-moments-engineering.html
	double izz = pow(h2, 3)*e2/12 + 2 * (pow(e1,3)*h1) / 12 + 2 * (e1*h1) * pow(h2+e1,2) / 4;
	return izz;
}

double ISectionBeam::getTorsionalConstant() const {
	double h1 = upper_flange_width;
	double h2 = (beam_height - upper_flange_thickness - lower_flange_thickness);
	double h3 = lower_flange_width;
	double e1 = upper_flange_thickness;
	double e2 = web_thickness;
	double e3 = lower_flange_thickness;
	/*
	 *  http://www.eng-tips.com/viewthread.cfm?qid=137188
	 */
	return (h1 * pow(e1, 3) + h2 * pow(e2, 3) + h3 * pow(e3, 3)) / 3;
}

double ISectionBeam::getShearAreaFactorY() const {
    //double sly = (beam_height - upper_flange_thickness - lower_flange_thickness)*web_thickness;
    //return 1.0 / sly;
    double web_height = (beam_height - upper_flange_thickness - lower_flange_thickness);
    double web_area = (web_thickness * web_height);
    return this->getAreaCrossSection() / web_area;
}

double ISectionBeam::getShearAreaFactorZ() const {
	//double slz = upper_flange_width * upper_flange_thickness
	//		+ lower_flange_width * lower_flange_thickness;
	//return 1.0 / slz;
    double web_height = (beam_height - upper_flange_thickness - lower_flange_thickness);
    double web_area = (web_thickness * web_height);
    return this->getAreaCrossSection() / web_area;
}

MatrixElement::MatrixElement(Model& model, Type type, bool symmetric, int original_id) :
		ElementSet(model, type, modelType, original_id), symmetric(symmetric) {
}

void MatrixElement::addComponent(const int nodeid1, const DOF dof1, const int nodeid2, const DOF dof2, const double value) {
	int nodePosition1 = model.mesh.findOrReserveNode(nodeid1);
	int nodePosition2 = model.mesh.findOrReserveNode(nodeid2);
	DOF myDof1 = dof1;
	DOF myDof2 = dof2;

	if (nodePosition1 > nodePosition2) {
		int swap = nodePosition2;
		nodePosition2 = nodePosition1;
		nodePosition1 = swap;
		myDof1 = dof2;
		myDof2 = dof1;
	}
	auto it = submatrixByNodes.find(make_pair(nodePosition1, nodePosition2));
	shared_ptr<DOFMatrix> subMatrix = nullptr;
	if (it != submatrixByNodes.end()) {
		subMatrix = it->second;
	} else {
		subMatrix = make_shared<DOFMatrix>(DOFMatrix(symmetric && nodeid1 == nodeid2));
		this->submatrixByNodes[make_pair(nodePosition1, nodePosition2)] = subMatrix;
	}
    subMatrix->addComponent(myDof1, myDof2, value);
}

void MatrixElement::clear() {
    submatrixByNodes.clear();
}


const shared_ptr<DOFMatrix> MatrixElement::findSubmatrix(const int nodePosition1, const int nodePosition2) const {
	shared_ptr<DOFMatrix> result = make_shared<DOFMatrix>(DOFMatrix(symmetric));
	auto it = submatrixByNodes.find(make_pair(nodePosition1, nodePosition2));
	if (it != submatrixByNodes.end()) {
		std::copy(it->second->componentByDofs.begin(), it->second->componentByDofs.end(),
				std::inserter(result->componentByDofs, result->componentByDofs.end()));
	}
	return result;
}

const set<int> MatrixElement::nodePositions() const {
	set<int> result;
	for (const auto& kv : submatrixByNodes) {
		result.insert(kv.first.first);
		result.insert(kv.first.second);
	}
	return result;
}

const DOFS MatrixElement::getDOFSForNode(const int nodePosition) const {
	DOFS dofs;
	for (const auto& kv : submatrixByNodes) {
		if (kv.first.first == nodePosition or kv.first.second) {
			if (kv.second->hasRotations()) {
				dofs += DOFS::ROTATIONS;
			}
			if (kv.second->hasTranslations()) {
				dofs += DOFS::TRANSLATIONS;
			}
		}
	}
	return dofs;
}

const set<pair<int, int>> MatrixElement::nodePairs() const {
	set<pair<int, int>> result;
	for (const auto& kv : submatrixByNodes) {
		result.insert(make_pair(kv.first.first, kv.first.second));
	}
	return result;
}

const std::set<std::pair<int, int>> MatrixElement::findInPairs(int nodePosition) const {
	set<pair<int, int>> result;
	for (const auto& kv : submatrixByNodes) {
		if (kv.first.first != nodePosition and kv.first.second != nodePosition) {
			continue;
		}
		if (kv.first.first == kv.first.second) {
			continue;
		}
		result.insert(make_pair(kv.first.first, kv.first.second));
	}
	return result;
}

StiffnessMatrix::StiffnessMatrix(Model& model, int original_id) :
		MatrixElement(model, ElementSet::Type::STIFFNESS_MATRIX, true, original_id) {
}

void StiffnessMatrix::addStiffness(const int nodeid1, const DOF dof1, const int nodeid2,
		const DOF dof2, const double stiffness_value) {
	addComponent(nodeid1, dof1, nodeid1, dof1, stiffness_value);
	addComponent(nodeid2, dof2, nodeid2, dof2, stiffness_value);
	addComponent(nodeid1, dof1, nodeid2, dof2, -stiffness_value);
}

MassMatrix::MassMatrix(Model& model, int original_id) :
		MatrixElement(model, ElementSet::Type::MASS_MATRIX, true, original_id) {
}

DampingMatrix::DampingMatrix(Model& model, int original_id) :
		MatrixElement(model, ElementSet::Type::DAMPING_MATRIX, true, original_id) {
}

void DampingMatrix::addDamping(const int nodeid1, const DOF dof1, const int nodeid2,
		const DOF dof2, const double damping_value) {
	addComponent(nodeid1, dof1, nodeid1, dof1, damping_value);
	addComponent(nodeid2, dof2, nodeid2, dof2, damping_value);
	addComponent(nodeid1, dof1, nodeid2, dof2, -damping_value);
}


RigidSet::RigidSet(Model& model, Type type, int master_id, int original_id) :
                ElementSet(model, type, model.modelType, original_id), masterId(master_id){
}


const DOFS RigidSet::getDOFSForNode(const int nodePosition) const {
    UNUSEDV(nodePosition);
    return DOFS::ALL_DOFS;
}

Rbar::Rbar(Model& model, int master_id, int original_id) :
                RigidSet(model, ElementSet::Type::RBAR, master_id, original_id){
}

shared_ptr<ElementSet> Rbar::clone() const {
    return make_shared<Rbar>(*this);
}

Rbe3::Rbe3(Model& model, int master_id, DOFS mdofs, DOFS sdofs, int original_id) :
                RigidSet(model, ElementSet::Type::RBE3, master_id, original_id),
                mdofs(mdofs), sdofs(sdofs){
}

shared_ptr<ElementSet> Rbe3::clone() const {
    return make_shared<Rbe3>(*this);
}

Lmpc::Lmpc(Model& model, int analysisId, int original_id) :
                RigidSet(model, ElementSet::Type::LMPC, Globals::UNAVAILABLE_INT, original_id),
analysisId(analysisId){
}

shared_ptr<ElementSet> Lmpc::clone() const {
    return make_shared<Lmpc>(*this);
}

void Lmpc::assignDofCoefs(std::vector<DOFCoefs> dofCoefs) {
    this->dofCoefs= dofCoefs;
}

SurfaceSlideSet::SurfaceSlideSet(Model& model, int original_id) :
                RigidSet(model, ElementSet::Type::SURFACE_SLIDE_CONTACT, Globals::UNAVAILABLE_INT, original_id) {
}

shared_ptr<ElementSet> SurfaceSlideSet::clone() const {
    return make_shared<SurfaceSlideSet>(*this);
}

// ScalarSpring Methods
ScalarSpring::ScalarSpring(Model& model, int original_id, double stiffness, double damping) :
                Discrete(model, ElementSet::Type::SCALAR_SPRING, true, original_id), stiffness(stiffness),
                damping(damping){
}

double ScalarSpring::getStiffness() const {
    return this->stiffness;
}
double ScalarSpring::getDamping() const {
    return this->damping;
}
const std::map<std::pair<DOF, DOF>, vector<int>> ScalarSpring::getCellPositionByDOFS() const{
    return this->cellpositionByDOFS;
}

void ScalarSpring::setStiffness(const double stiffness){
    this->stiffness=stiffness;
}
void ScalarSpring::setDamping (const double damping){
    this->damping=damping;
}
bool ScalarSpring::hasStiffness() const {
    return !(is_zero(this->stiffness) || is_equal(this->stiffness, Globals::UNAVAILABLE_DOUBLE));
}
bool ScalarSpring::hasDamping() const {
    return !(is_zero(this->damping) || is_equal(this->damping, Globals::UNAVAILABLE_DOUBLE));
}

shared_ptr<ElementSet> ScalarSpring::clone() const {
    return make_shared<ScalarSpring>(*this);
}

void ScalarSpring::addSpring(int cellPosition, DOF dofNodeA, DOF dofNodeB){
    this->cellpositionByDOFS[std::pair<DOF, DOF>(dofNodeA, dofNodeB)].push_back(cellPosition);
}

std::vector<std::pair<DOF, DOF>> ScalarSpring::getDOFSSpring() const {
    std::vector<std::pair<DOF, DOF>> vDOF;
    for (const auto& kv : this->cellpositionByDOFS){
        vDOF.push_back(kv.first);
    }
    return vDOF;
}

int ScalarSpring::getNbDOFSSpring() const{
    return static_cast<int>(this->cellpositionByDOFS.size());
}

bool ScalarSpring::hasTranslations() const {
	for (int i = 0; i < 2; ++i) {
        DOF code1 = DOF::findByPosition(i);
		for (int j = 0; j < 2; ++j) {
		    DOF code2 = DOF::findByPosition(j);
            auto codeIter = this->cellpositionByDOFS.find(make_pair(code1, code2));
            if (codeIter != this->cellpositionByDOFS.end()) {
				return true;
            }
		}
	}
	return false;
}

bool ScalarSpring::hasRotations() const {
	for (int i = 3; i < 5; ++i) {
        DOF code1 = DOF::findByPosition(i);
		for (int j = 3; j < 5; ++j) {
		    DOF code2 = DOF::findByPosition(j);
            auto codeIter = this->cellpositionByDOFS.find(make_pair(code1, code2));
            if (codeIter != this->cellpositionByDOFS.end()) {
				return true;
            }
		}
	}
	return false;
}

const vector<double> ScalarSpring::asStiffnessVector(bool addRotationsIfNotPresent) const {
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	int max_row_element_index = 0;
	for (int colindex = 0; colindex < 2; ++colindex) {
		for (int coldof = 0; coldof < ncomp; coldof++) {
			DOF colcode = DOF::findByPosition(coldof);
			max_row_element_index++;
			int row_element_index = 0;
			for (int rowindex = 0; rowindex < 2; ++rowindex) {
				for (int rowdof = 0; rowdof < ncomp; rowdof++) {
					row_element_index++;
					if (row_element_index > max_row_element_index) {
						break;
					}
					DOF rowcode = DOF::findByPosition(rowdof);
                    auto codeIter = this->cellpositionByDOFS.find(make_pair(rowcode, colcode));
                    if (codeIter != this->cellpositionByDOFS.end() and colindex == 1 and row_element_index <= 2) {
                        result.push_back(stiffness);
                    } else {
                        result.push_back(0.0);
                    }
				}
			}
		}
	}
	return result;
}

const vector<double> ScalarSpring::asDampingVector(bool addRotationsIfNotPresent) {
	vector<double> result;
	int ncomp = (addRotationsIfNotPresent || hasRotations()) ? 6 : 3;
	int max_row_element_index = 0;
	for (int colindex = 0; colindex < 2; ++colindex) {
		for (int coldof = 0; coldof < ncomp; coldof++) {
			DOF colcode = DOF::findByPosition(coldof);
			max_row_element_index++;
			int row_element_index = 0;
			for (int rowindex = 0; rowindex < 2; ++rowindex) {
				for (int rowdof = 0; rowdof < ncomp; rowdof++) {
					row_element_index++;
					if (row_element_index > max_row_element_index) {
						break;
					}
					DOF rowcode = DOF::findByPosition(rowdof);
                    auto codeIter = this->cellpositionByDOFS.find(make_pair(rowcode, colcode));
                    if (codeIter != this->cellpositionByDOFS.end() and colindex == 1 and row_element_index <= 2) {
                        result.push_back(damping);
                    } else {
                        result.push_back(0.0);
                    }
				}
			}
		}
	}
	return result;
}

} /* namespace vega */

