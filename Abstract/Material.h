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
 * Material.h
 *
 *  Created on: Sep 7, 2013
 *      Author: devel
 */

#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "Utility.h"
#include "Object.h"
#include "Reference.h"
#include "MeshComponents.h"
#include "Value.h"
#include <map>
#include <vector>

#include <fstream>

namespace vega {

class Model;

class Nature {
protected:
    const Model& model;
public:
    enum class NatureType {
        NATURE_ELASTIC = 0,
        NATURE_VISCOELASTIC = 2,
        NATURE_BILINEAR_ELASTIC,
        NATURE_NONLINEAR_ELASTIC,
        NATURE_RIGID,
        NATURE_ORTHOTROPIC
    };
    static const std::map<NatureType, std::string> stringByType;
    static const std::string name;
    friend std::string to_str(const Nature& nature);
    friend std::ostream &operator<<(std::ostream&, const Nature&);
    static const double UNAVAILABLE_DOUBLE;

    const NatureType type;
    Nature(const Model&, NatureType);
    virtual std::shared_ptr<Nature> clone() const = 0;
};

class ElasticNature: public Nature {
    double e;
    double nu;
    double g;
    double rho;
    double alpha;
    double tref;
    double ge;
public:
    ElasticNature(const Model&, const double e = UNAVAILABLE_DOUBLE, const double nu =
            UNAVAILABLE_DOUBLE, const double g = UNAVAILABLE_DOUBLE, const double rho =
            UNAVAILABLE_DOUBLE, const double alpha = UNAVAILABLE_DOUBLE, const double tref =
            UNAVAILABLE_DOUBLE, const double ge = UNAVAILABLE_DOUBLE);
    double getE() const;
    double getNu() const;
    double getG() const;
    double getGE() const;
    /**
     * Get mass density (in kg.m^{-3})
     */
    double getRho() const;
    /**
     * Get mass density as equivalent force density (in N.m^{-3})
     */
    double getRhoAsForceDensity() const;
    /**
     * Get the thermal expansion coefficient
     */
    double getAlpha() const;
    /**
     * Get temperature reference for thermal expansion
     */
    double getTref() const;

    virtual std::shared_ptr<Nature> clone() const;

};

class OrthotropicNature: public Nature {
    double _e_longitudinal;
    double _e_transverse;
    double _nu_longitudinal_transverse;
    double _g_longitudinal_transverse;
    double _g_transverse_normal;
    double _g_longitudinal_normal;
public:
    OrthotropicNature(const Model&, const double e_longitudinal, const double e_transverse,
            const double nu_longitudinal_transverse, const double g_longitudinal_transverse,
            const double g_transverse_normal = UNAVAILABLE_DOUBLE, const double g_longitudinal_normal = UNAVAILABLE_DOUBLE);
    double getE_longitudinal() const;
    double getE_transverse() const;
    double getNu_longitudinal_transverse() const;
    double getG_longitudinal_transverse() const;
    double getG_transverse_normal() const;
    double getG_longitudinal_normal() const;
    virtual std::shared_ptr<Nature> clone() const;

};

class BilinearElasticNature: public Nature {
public:
    double elastic_limit = UNAVAILABLE_DOUBLE;
    double secondary_slope = UNAVAILABLE_DOUBLE;
    bool yield_function_von_mises = true;
    bool hardening_rule_isotropic = true;
    BilinearElasticNature(const Model&, const double elastic_limit, const double secondary_slope);
    BilinearElasticNature(const Model&);
    virtual std::shared_ptr<Nature> clone() const;
};

class NonLinearElasticNature: public Nature {
    Reference<NamedValue> stress_strain_function_ref;
public:
    NonLinearElasticNature(const Model&, const FunctionTable& stress_strain_function);
    NonLinearElasticNature(const Model&, const int stress_strain_function_id);
    std::shared_ptr<FunctionTable> getStressStrainFunction() const;
    virtual std::shared_ptr<Nature> clone() const;
};


/**
 *   Rigid nature is used by Rigid Element Set, like RBE2, RBAR, RBE3
 */
class RigidNature: public Nature {
private:
    double rigidity;
    double lagrangian;
public:
    RigidNature(const Model&, const double rigidity = UNAVAILABLE_DOUBLE,
            const double lagrangian = UNAVAILABLE_DOUBLE);
    double getRigidity() const;
    void setRigidity(double rigidity);
    double getLagrangian() const;
    void setLagrangian(double lagrangian);

    virtual std::shared_ptr<Nature> clone() const;

};


/**
 * Base class for materials
 */
class Material: public Identifiable<Material> {
    Model* const model;
    std::map<Nature::NatureType, std::shared_ptr<Nature>> nature_by_type;

public:
    friend std::ostream &operator<<(std::ostream&, const Material&);    //output
    //dummy type to fit in a container class
    enum class Type {
        MATERIAL
    };
    const Type type = Type::MATERIAL;
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
    Material(Model* model, int material_id = NO_ORIGINAL_ID);
    void addNature(const Nature &nature);
    const std::shared_ptr<Nature> findNature(Nature::NatureType) const;
    virtual bool validate() const override;
    virtual std::shared_ptr<Material> clone() const;
    /**
     * Get all the cells assigned to a specific material. This inspects
     * both the elementSets with a material assigned and the materials assigned
     * directly.
     */
    CellContainer getAssignment() const;
    /**
     * Assign a material to a group of cells. There are two ways of assigning
     * a material: either trough this method or with an ElementSet.
     * Choose the one appropriate to your input model.
     */
    void assignMaterial(const CellContainer& cellsToAssign);
};

}

/* namespace vega */
#endif /* MATERIAL_H_ */
