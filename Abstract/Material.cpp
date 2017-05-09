/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * Material.cpp
 *
 *  Created on: Sep 7, 2013
 *      Author: devel
 */
#include "Material.h"
#include "Model.h"
#include <float.h>
#include <stdexcept>      // invalid_argument
#include <iostream>

using namespace std;

namespace vega {


const string Material::name = "Material";
const map<Material::Type, string> Material::stringByType = {
        { MATERIAL, "MATERIAL" }
};


ostream &operator<<(ostream &out, const Material& material) {
	out << to_str(material);
	if (material.nature_by_type.size() > 0) {
		cout << " with:";
		for (auto nature : material.nature_by_type) {
		    out<< " "<< *nature.second;
		}
	}
	return out;
}

Material::Material(Model* model, int original_id) :
		Identifiable(original_id), model(model) {
}

shared_ptr<Material> Material::clone() const {
	return shared_ptr<Material>(new Material(*this));
}

bool Material::validate() const {
	bool validMaterial = nature_by_type.size() > 0;
	if (!validMaterial) {
		cerr << *this << " has no nature assigned.";
	}
	/*
	 * if the model is configured to assign materials to cells directly check
	 * that every material is assigned to some cell or cellgroup
	 */
	if (!model->configuration.partitionModel) {
		CellContainer assignment = getAssignment();
		validMaterial &= !assignment.empty();
	}
	return validMaterial;
}

void Material::addNature(const Nature &nature) {
	if (findNature(nature.type)) {
		string message = "Nature already in use " + nature.type;
		throw invalid_argument(message);
	}
	nature_by_type[nature.type] = nature.clone();
}

const shared_ptr<Nature> Material::findNature(const Nature::NatureType natureType) const {
	shared_ptr<Nature> nature;
	auto it = nature_by_type.find(natureType);
	if (it != nature_by_type.end())
		nature = it->second;
	return nature;
}

Nature::Nature(const Model& model, Nature::NatureType type) :
		model(model), type(type) {

}

const double Nature::UNAVAILABLE_DOUBLE = -DBL_MAX;

const string Nature::name = "NATURE";
const map<Nature::NatureType, string> Nature::stringByType = {
         {NATURE_ELASTIC , "NATURE ELASTIC"}
        ,{NATURE_VISCOELASTIC , "NATURE VISCOELASTIC"}
        ,{NATURE_BILINEAR_ELASTIC, "NATURE BILINEAR ELASTIC"}
        ,{NATURE_NONLINEAR_ELASTIC, "NATURE NONLINEAR ELASTIC"}
};

ostream &operator<<(ostream &out, const Nature& nature) {
    out << to_str(nature);
    return out;
}

std::string to_str(const Nature& nature){
    std::ostringstream oss;
    std::string type;
    auto it = Nature::stringByType.find(nature.type);
    if (it != Nature::stringByType.end())
        type = "type=" + it->second;
    else
        type = "unknown type";

    oss << Nature::name << "{" << type << "}";
    return oss.str();
}


Nature::~Nature() {

}

shared_ptr<Nature> ElasticNature::clone() const {
	return shared_ptr<Nature>(new ElasticNature(*this));
}

ElasticNature::~ElasticNature() {

}

ElasticNature::ElasticNature(const Model& model, const double e, const double nu, const double g,
		const double rho, const double alpha, const double tref, const double ge) :
		Nature(model, NATURE_ELASTIC), e(e), nu(nu), g(g), rho(rho), alpha(alpha), tref(tref), ge(ge) {

	if (is_equal(e, UNAVAILABLE_DOUBLE) && is_equal(g, UNAVAILABLE_DOUBLE))
		throw invalid_argument("E and G may not both be blank.");
}

double ElasticNature::getE() const {
	if (is_equal(nu, UNAVAILABLE_DOUBLE) && is_equal(e, UNAVAILABLE_DOUBLE))
		// If NU and E are both blank, then both are set to 0.0.
		return 0.0;
	else
		return (is_equal(e, UNAVAILABLE_DOUBLE)) ? (2 * (1 + nu) * g) : e;
}

double ElasticNature::getNu() const {
	if ((is_equal(nu, UNAVAILABLE_DOUBLE) && is_equal(g, UNAVAILABLE_DOUBLE))
			|| (is_equal(nu, UNAVAILABLE_DOUBLE) && is_equal(e, UNAVAILABLE_DOUBLE)))
		// If NU and E, or NU and G, are both blank, then both are set to 0.0.
		return 0.0;
	else
		return is_equal(nu, UNAVAILABLE_DOUBLE) ? (e / (2 * g) - 1) : nu;
}

double ElasticNature::getG() const {
	if (is_equal(nu, UNAVAILABLE_DOUBLE) && is_equal(g, UNAVAILABLE_DOUBLE))
		// If NU and G are both blank, then both are set to 0.0.
		return 0.0;
	else
		return (is_equal(g, UNAVAILABLE_DOUBLE)) ? (e / (2 * (1 + nu))) : g;
}

double ElasticNature::getRho() const {
	double mass_multiplier = 1;
	auto it = model.parameters.find(Model::MASS_OVER_FORCE_MULTIPLIER);
	if (it != model.parameters.end()) {
		mass_multiplier = it->second;
		assert(!is_zero(it->second));
	}
	return (is_equal(rho, UNAVAILABLE_DOUBLE)) ? 0 : rho * mass_multiplier;
}

double ElasticNature::getRhoAsForceDensity() const {
	return (is_equal(rho, UNAVAILABLE_DOUBLE)) ? 0 : rho;
}

double ElasticNature::getAlpha() const {
	return (is_equal(alpha, UNAVAILABLE_DOUBLE)) ? 0 : alpha;
}

double ElasticNature::getTref() const {
	return (is_equal(tref, UNAVAILABLE_DOUBLE)) ? 0 : tref;
}

double ElasticNature::getGE() const {
    return ge;
}

shared_ptr<Nature> BilinearElasticNature::clone() const {
	return shared_ptr<Nature>(new BilinearElasticNature(*this));
}

BilinearElasticNature::~BilinearElasticNature() {

}

BilinearElasticNature::BilinearElasticNature(const Model& model, const double elastic_limit,
		const double secondary_slope) :
		Nature(model, NATURE_BILINEAR_ELASTIC), elastic_limit(elastic_limit), secondary_slope(
				secondary_slope) {
}

BilinearElasticNature::BilinearElasticNature(const Model& model) :
		Nature(model, NATURE_BILINEAR_ELASTIC) {
}

shared_ptr<Nature> NonLinearElasticNature::clone() const {
	return shared_ptr<Nature>(new NonLinearElasticNature(*this));
}

NonLinearElasticNature::~NonLinearElasticNature() {

}

NonLinearElasticNature::NonLinearElasticNature(const Model& model,
		const int stress_strain_function_id) :
		Nature(model, NATURE_NONLINEAR_ELASTIC), stress_strain_function_ref(Value::FUNCTION_TABLE,
				stress_strain_function_id) {
}

NonLinearElasticNature::NonLinearElasticNature(const Model& model,
		const FunctionTable& stress_strain_function) :
		Nature(model, NATURE_NONLINEAR_ELASTIC), stress_strain_function_ref(stress_strain_function) {
}

shared_ptr<FunctionTable> NonLinearElasticNature::getStressStrainFunction() const {
	return dynamic_pointer_cast<FunctionTable>(model.find(stress_strain_function_ref));
}

CellContainer Material::getAssignment() const {
	return this->model->getMaterialAssignment(this->getId());
}

void Material::assignMaterial(const CellContainer& cellsToAssign) {
	this->model->assignMaterial(this->getId(), cellsToAssign);
}

}
/* namespace vega */

