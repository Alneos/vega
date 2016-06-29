/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterBuilder.cpp
 *
 *  Created on: 5 mars 2013
 *      Author: dallolio
 */

#include "build_properties.h"
#include "../Abstract/Model.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <fstream>
#include <limits>

#include <ciso646>
#include "NastranWriter.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;
using namespace std;

namespace vega {
namespace nastran {

ostream &operator<<(ostream &out, const Line& line) {
	out << left << setw(8);
	out << line.keyword;
	int fieldCount = 0;
	for(const auto& field : line.fields) {
		fieldCount++;
		if (fieldCount % line.fieldNum == 0) {
			out << endl;
			out << setw(line.fieldLength) << "";
		}
		out << field;
	}
	out << endl;
	return out;
}

Line::Line(string _keyword) : keyword(_keyword) {
	if (boost::algorithm::ends_with(keyword, "*")) {
		fieldLength = 16;
		fieldNum = 4;
	} else {
		fieldLength = 8;
		fieldNum = 8;
	}
}

Line& Line::add() {
	this->add(string());
	return *this;
}

Line& Line::add(double value) {
	std::ostringstream strs;
	strs << boost::format("%8.7g") % value;
	string gnum = strs.str();
	strs.str("");
	strs.clear();
	if (gnum.length() <= fieldLength) {
		strs << internal << setw(this->fieldLength) << gnum;
	} else {
		strs << internal << setw(this->fieldLength) << boost::format("%4.2e") % value;
	}
	this->fields.push_back(strs.str());
	return *this;
}

Line& Line::add(string value) {
	std::ostringstream strs;
	strs << internal << setw(this->fieldLength) << value;
	this->fields.push_back(strs.str());
	return *this;
}

Line& Line::add(int value) {
	std::ostringstream strs;
	strs << internal << setw(this->fieldLength) << value;
	this->fields.push_back(strs.str());
	return *this;
}

Line& Line::add(const vector<double> values) {
	for(double value : values) {
		this->add(value);
	}
	return *this;
}

Line& Line::add(const vector<int> values) {
	for(int value : values) {
		this->add(value);
	}
	return *this;
}

Line& Line::add(const DOFS dofs) {
	this->add(dofs.nastranCode());
	return *this;
}

Line& Line::add(const VectorialValue vector) {
	this->add(vector.x());
	this->add(vector.y());
	this->add(vector.z());
	return *this;
}

NastranWriterImpl::NastranWriterImpl() {

}

NastranWriterImpl::~NastranWriterImpl() {

}

string NastranWriterImpl::getDatFilename(const shared_ptr<vega::Model>& model,
		const string& outputPath) const
		{
	string outputFileName;
	if (model->name.empty()) {
		outputFileName = "nastran";
	} else {
		outputFileName = model->name;
		const size_t period_idx = outputFileName.rfind('.');
		if (string::npos != period_idx) {
			outputFileName = outputFileName.substr(0, period_idx);
		}
	}
	string modelPath = outputFileName + ".dat";
	bool absolute = true;
	if (absolute) {
		modelPath = (fs::path(fs::absolute(outputPath)) / modelPath).string();
	}
	return modelPath;
}

void NastranWriterImpl::writeSOL(const shared_ptr<vega::Model>& model, ofstream& out) const
		{
	auto& firstAnalysis = *model->analyses.begin();
	switch (firstAnalysis->type) {
	case (Analysis::LINEAR_MECA_STAT):
		{
		out << "SOL 101" << endl;
		break;
	}
	case (Analysis::LINEAR_MODAL):
		{
		out << "SOL 103" << endl;
		break;
	}
	case (Analysis::LINEAR_DYNA_MODAL_FREQ):
		{
		out << "SOL 111" << endl;
		break;
	}
	case (Analysis::NONLINEAR_MECA_STAT):
		{
		out << "SOL 106" << endl;
		break;
	}
	default:
		out << "$ WARN analysis " << firstAnalysis << " not supported. Skipping." << endl;
	}
}

void NastranWriterImpl::writeCells(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (const auto& elementSet : model->elementSets) {
		if (elementSet->isDiscrete() || elementSet->isMatrixElement()) {
			continue;
		}
		CellGroup* cellGroup = elementSet->cellGroup;
		for (const Cell& cell : cellGroup->getCells()) {
			string keyword;
			if (elementSet->isBeam()) {
				keyword = "CBEAM";
			} else
			if (elementSet->isShell()) {
				switch (cell.type.code) {
				case CellType::TRI3_CODE:
					keyword = "CTRIA3";
					break;
				case CellType::TRI6_CODE:
					keyword = "CTRIA6";
					break;
				case CellType::QUAD4_CODE:
					keyword = "CQUAD4";
					break;
				case CellType::QUAD8_CODE:
					keyword = "CQUAD8";
					break;
				default:
					throw logic_error("Unimplemented type");
				}
			} else
			if (elementSet->type == ElementSet::CONTINUUM) {
				switch (cell.type.code) {
				case CellType::HEXA8_CODE:
					case CellType::HEXA20_CODE:
					keyword = "CHEXA";
					break;
				case CellType::TETRA4_CODE:
					case CellType::TETRA10_CODE:
					keyword = "CTETRA";
					break;
				default:
					throw logic_error("Unimplemented type");
				}
			}

			out << Line(keyword).add(cell.id).add(elementSet->bestId()).add(cell.nodeIds);
		}
	}
}

void NastranWriterImpl::writeNodes(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (Node node : model->mesh->nodes) {
		out << Line("GRID").add(node.id).add().add(node.x).add(node.y).add(node.z);
	}
}

void NastranWriterImpl::writeMaterials(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (const auto& material : model->materials) {
		Line mat1("MAT1");
		mat1.add(material->bestId());
		const shared_ptr<Nature> enature = material->findNature(Nature::NATURE_ELASTIC);
		if (enature) {
			const ElasticNature& elasticNature = dynamic_cast<ElasticNature&>(*enature);
			mat1.add(elasticNature.getE());
			mat1.add(elasticNature.getG());
			mat1.add(elasticNature.getNu());
			mat1.add(elasticNature.getRho());
		}
		out << mat1;
	}
}

void NastranWriterImpl::writeConstraints(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (const auto& constraintSet : model->constraintSets) {
		const set<shared_ptr<Constraint> > spcs = constraintSet->getConstraintsByType(
				Constraint::SPC);
		if (spcs.size() > 0) {
			for (shared_ptr<Constraint> constraint : spcs) {
				shared_ptr<const SinglePointConstraint> spc = static_pointer_cast<
						const SinglePointConstraint>(constraint);
				for (int nodePosition : spc->nodePositions()) {
					Node node = model->mesh->findNode(nodePosition);
					out
							<< Line("SPC1").add(constraintSet->bestId()).add(
									spc->getDOFSForNode(nodePosition)).add(node.id);
				}
			}
		}
		const set<shared_ptr<Constraint> > rigidConstraints = constraintSet->getConstraintsByType(
				Constraint::RIGID);
		if (rigidConstraints.size() > 0) {
			for (shared_ptr<Constraint> constraint : rigidConstraints) {
				shared_ptr<const RigidConstraint> rigid =
						static_pointer_cast<const RigidConstraint>(constraint);
				Line rbe2("RBE2");
				rbe2.add(constraintSet->bestId());
				Node master = model->mesh->findNode(rigid->getMaster());
				rbe2.add(master.id);
				rbe2.add(DOFS::ALL_DOFS);
				for (int slavePosition : rigid->getSlaves()) {
					Node slave = model->mesh->findNode(slavePosition);
					rbe2.add(slave.id);
				}
				out << rbe2;
			}
		}
	}
}

void NastranWriterImpl::writeLoadings(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (const auto& loadingSet : model->loadSets) {
		const set<shared_ptr<Loading> > gravities = loadingSet->getLoadingsByType(Loading::GRAVITY);
		if (gravities.size() > 0) {
			for (shared_ptr<Loading> loading : gravities) {
				shared_ptr<const Gravity> gravity = static_pointer_cast<const Gravity>(loading);
				Line grav("GRAV");
				grav.add(loadingSet->bestId());
				if (gravity->hasCoordinateSystem()) {
					grav.add(model->find(gravity->coordinateSystem_reference)->bestId());
				} else {
					grav.add(0);
				}
				grav.add(gravity->getAcceleration());
				grav.add(gravity->getDirection());
				out << grav;
			}
		}

		const set<shared_ptr<Loading> > forceSurfaces = loadingSet->getLoadingsByType(
				Loading::FORCE_SURFACE);
		if (forceSurfaces.size() > 0) {
			for (shared_ptr<Loading> loading : forceSurfaces) {
				shared_ptr<ForceSurface> forceSurface = static_pointer_cast<ForceSurface>(loading);
				Line pload4("PLOAD4");
				pload4.add(loadingSet->bestId());
				vector<Cell> cells = forceSurface->getCells();
				if (cells.size() == 1) {
					Cell cell = cells[0];
					pload4.add(cell.id);
				} else {
					throw logic_error("Unimplemented multiple cells in PLOAD4");
				}
				pload4.add(forceSurface->getForce().norm());
				pload4.add(0.0);
				pload4.add(0.0);
				pload4.add(0.0);
				if (!forceSurface->getMoment().iszero()) {
					throw logic_error("Unimplemented moment in PLOAD4");
				}
				// TODO LD must recalculate two opposite nodes... hack
				pload4.add(forceSurface->getApplicationFace()[0]);
				pload4.add(forceSurface->getApplicationFace()[2]);
				if (forceSurface->hasCoordinateSystem()) {
					shared_ptr<CoordinateSystem> coordinateSystem = model->find(forceSurface->coordinateSystem_reference);
					pload4.add(coordinateSystem->bestId());
					pload4.add(coordinateSystem->vectorToGlobal(forceSurface->getForce().normalized()));
				} else {
					pload4.add(0);
					pload4.add(forceSurface->getForce().normalized());
				}
			}
		}
	}
}

void NastranWriterImpl::writeRuler(ofstream& out)
		{
	out << "$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]"
			<< endl;
}

void NastranWriterImpl::writeElements(const shared_ptr<vega::Model>& model, ofstream& out)
		{
	for (shared_ptr<Beam> beam : model->getBeams()) {
		Line pbeam("PBEAM");
		pbeam.add(beam->bestId());
		pbeam.add(beam->material->bestId());
		pbeam.add(beam->getAreaCrossSection());
		pbeam.add(beam->getMomentOfInertiaZ());
		pbeam.add(beam->getMomentOfInertiaY());
		pbeam.add(0.0);
		pbeam.add(beam->getTorsionalConstant());
		out << pbeam;
	}
	for (shared_ptr<ElementSet> shell : model->filterElements(ElementSet::SHELL)) {
		Line pshell("PSHELL");
		pshell.add(shell->bestId());
		pshell.add(shell->material->bestId());
		out << pshell;
	}
	for (shared_ptr<ElementSet> continuum : model->filterElements(ElementSet::CONTINUUM)) {
		Line psolid("PSOLID");
		psolid.add(continuum->bestId());
		psolid.add(continuum->material->bestId());
		out << psolid;
	}
}

string NastranWriterImpl::writeModel(const shared_ptr<vega::Model> model,
		const vega::ConfigurationParameters &configuration) {

	string outputPath = configuration.outputPath;
	if (!fs::exists(outputPath)) {
		throw iostream::failure("Directory " + outputPath + " don't exist.");
	}

	string datPath = getDatFilename(model, outputPath);
	ofstream out;
	out.precision(DBL_DIG);
	out.open(datPath.c_str(), ios::out | ios::trunc);
	if (!out.is_open()) {
		string message = string("Can't open file ") + datPath + " for writing.";
		throw ios::failure(message);
	}

	out << "$ " << model->name << endl;
	writeSOL(model, out);
	out << "TIME 10000" << endl;
	for (const auto& analysis : model->analyses) {
		out << "SUBCASE " << analysis->bestId() << endl;
		for (shared_ptr<LoadSet> loadSet : analysis->getLoadSets()) {
			string typeName = loadSet->stringByType.find(loadSet->type)->second;
			out << "  " << typeName << "=" << loadSet->bestId() << endl;
		}
		for (shared_ptr<ConstraintSet> constraintSet : analysis->getConstraintSets()) {
			string typeName = constraintSet->stringByType.find(constraintSet->type)->second;
			out << "  " << typeName << "=" << constraintSet->bestId() << endl;
		}
	}
	out << "CEND" << endl;
	out << "$" << endl;
	out << "TITLE=Vega Exported Model" << endl;
	out << "BEGIN BULK" << endl;

	for (shared_ptr<CoordinateSystem> coordinateSystem : model->coordinateSystems) {
		switch (coordinateSystem->type) {
			case CoordinateSystem::CARTESIAN:
				// TODO LD complete
				out << Line("CORD2R").add(coordinateSystem->bestId()).add(coordinateSystem->getOrigin());
				break;
			case CoordinateSystem::SPHERICAL:
				// TODO LD complete
				out << Line("CORD2S").add(coordinateSystem->bestId()).add(coordinateSystem->getOrigin());
				break;
			case CoordinateSystem::CYLINDRICAL:
				// TODO LD complete
				out << Line("CORD2C").add(coordinateSystem->bestId()).add(coordinateSystem->getOrigin());
				break;
			default:
				throw logic_error("Unimplemented coordinate system type");
		}
	}
	writeRuler(out);
	writeNodes(model, out);
	writeRuler(out);
	writeCells(model, out);
	writeRuler(out);
	writeMaterials(model, out);
	writeRuler(out);
	writeElements(model, out);
	writeRuler(out);
	writeConstraints(model, out);
	writeRuler(out);
	writeLoadings(model, out);

//	{%- set key_counter = 0 -%}
//	{% macro lpad(text) -%}{{ "%-16s" % text }}{%- endmacro %}
//	{% macro key(text) -%}{{ "%-8s" % (text+'*') }}{%- set key_counter = 1 -%}{%- endmacro %}
//	{% macro endl(text) -%}{{ "%-8s" % text }}{%- endmacro %}
//	{% macro nextl(text) -%}{{ "%-8s" % text }}{{key_counter}}{%- endmacro %}
//	{% macro rpad(text) -%}{{ "%16s" % text }}{%- endmacro %}
//	{% macro fnum(num) -%}{%- set gnum = "%15.13g" % num -%}{%- if gnum | length <= 16 %}{%- if '.' in gnum %}{{ "%16s" % gnum }}{%- else %}{{ "%16s" % (gnum | float) }}{%- endif -%}{%- else %}{{ "%16s" % ("%12.1e" % num) }}{%- endif -%}{%- endmacro %}
//
//	{% macro render_collection(start_index, collection) -%}{% for item in collection -%}{%- if (loop.index0 - (4 - start_index)) is divisibleby 4 %}{{ endl("") }}{# next line is intentionally left empy to add a new line in the output every n items ^___^; #}
//	{{ nextl("") }}{% endif -%}{{ rpad(item) }}
//	{%- endfor %}{%- endmacro %}
//
//	{% macro render_floats(start_index, collection) -%}{% for item in collection -%}{%- if (loop.index0 - (4 - start_index)) is divisibleby 4 %}{{ endl("") }}{# next line is intentionally left empy to add a new line in the output every n items ^___^; #}
//	{{ nextl("") }}{% endif -%}{{ fnum(item) }}
//	{%- endfor %}{%- endmacro %}
//
//	BEGIN BULK
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for coordinate_system_id, coordinate_system in model.mesh.coordinate_systems_by_id|dictsort('coordinate_system_id') %}
//	{%- if coordinate_system.coordinate_system_type == 'CORD_R' %}
//	{{key('CORD2R')}}{{rpad(coordinate_system.integerid)}}{{rpad('')}}{{ render_floats(2, coordinate_system.abc) }}
//	{%- elif coordinate_system.coordinate_system_type == 'VECT_Y' %} {# Ignored, directly defined on CBAR #}
//	{%- else %}
//	$Not yet implemented {{coordinate_system.coordinate_system_type}}
//	{%- endif -%}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for node in model.mesh.nodes %}
//	{{key('GRID')}}{{ rpad(node.integerid) }}{%- if node.coordinate_system %}{{rpad(node.coordinate_system.integerid)}}{%- else -%}{{rpad('')}}{%- endif -%}{{ render_floats(2, node.global_coordinates) }}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for element_type in model.mesh.element_types %}
//	{%- for element in model.mesh.find_elements_in_type(element_type) %}
//	{%- set part = model.parts_by_id[element.part_id] -%}
//	{%- if element_type == 'SEG2' %}
//	{{key('CBAR')}}{{ rpad(element.integerid) }}{%- if part -%}{{ rpad(part.integerid) }}{%- endif -%}{{ render_collection(2, element.node_integerids) }}{%- if element.coordinate_system %}{{ render_floats(2+element.node_integerids|length, element.coordinate_system.as_vecty().components) }}{%- endif -%}
//	{%- elif element_type == 'TRIA3' %}
//	{{key('CTRIA3')}}{{ rpad(element.integerid) }}{%- if part -%}{{ rpad(part.integerid) }}{%- endif -%}{{ render_collection(2, element.node_integerids) }}
//	{%- elif element_type == 'QUAD4' %}
//	{{key('CQUAD4')}}{{ rpad(element.integerid) }}{%- if part -%}{{ rpad(part.integerid) }}{%- endif -%}{{ render_collection(2, element.node_integerids) }}
//	{%- elif element_type == 'TETRA4' %}
//	{{key('CTETRA')}}{{ rpad(element.integerid) }}{%- if part -%}{{ rpad(part.integerid) }}{%- endif -%}{{ render_collection(2, element.node_integerids) }}
//	{%- else %}
//	Not yet implemented {{element_type}}
//	{%- endif -%}
//	{%- endfor %}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for constraint_type, constraints in model.constraints_by_type|dictsort('constraint_type') %}
//	{% for constraint in constraints %}
//	{%- if constraint_type == 'SINGLE_POINT_CONSTRAINT' %}
//	{% for node_integerid in constraint.node_integerids %}
//	{{key('SPC1')}}{{rpad(constraint.group_position)}}{{ rpad(constraint.nastran_code) }}{{ rpad(node_integerid) }}
//	{%- endfor %}
//	{%- elif constraint_type == 'QUASIRIGID_CONSTRAINT' %}
//	{{key('RBE2')}}{{rpad(constraint.integerid)}}{{rpad(constraint.master.integerid)}}{{ rpad(constraint.nastran_code) }}{{ render_collection(3, constraint.slave_node_integerids) }}
//	{%- endif -%}
//	{%- endfor %}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for loading_type, loadings in model.loadings_by_type|dictsort('loading_type') %}
//	{% for loading in loadings %}
//	{%- if loading_type == 'GRAVITY' %}
//	{{key('GRAV')}}{{rpad(loading.group_position)}}{{rpad('')}}{{fnum(loading.acceleration)}}{{ render_floats(3, loading.direction.components) }}
//	{%- elif loading_type == 'NODAL_FORCE' %}
//	{% for node_integerid in loading.node_integerids %}
//	{{key('FORCE')}}{{rpad(loading.group_position)}}{{ rpad(node_integerid) }}{{rpad('')}}{{fnum(1.0)}}{{ render_floats(4, loading.force.components) }}
//	{%- endfor %}
//	{%- elif loading_type == 'NORMAL_PRESSION_FACE' %}
//	{% for element_integerid, applications in loading.element_integerids %}
//	{{key('PLOAD4')}}{{rpad(loading.group_position)}}{{ rpad(element_integerid) }}{{ fnum(loading.intensity) }}
//	{%- endfor %}
//	{%- endif -%}
//	{%- endfor %}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for material_id, material in model.materials_by_id|dictsort('material_id') %}
//	{{key('MAT1')}}{{ rpad(material.integerid) }}{{ fnum(material.e) }}{{lpad("")}}{{ fnum(material.nu) }}{{endl("")}}
//	{{nextl("")}}{{ fnum(material.rho) }}
//	{%- endfor %}
//
//	$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
//	{% for part_id, part in model.parts_by_id.iteritems() %}
//	{%- set fe = part.finite_element -%}
//	{%- if fe.finite_element_type == 'SHELL' %}
//	{{key('PSHELL')}}{{ rpad(part.integerid) }}{{ rpad(model.find_material(part.material_id).integerid) }}
//	{%- elif fe.finite_element_type == 'CONTINUUM' %}
//	{{key('PSOLID')}}{{ rpad(part.integerid) }}{{ rpad(model.find_material(part.material_id).integerid) }}
//	{%- elif fe.finite_element_type == 'GENERIC_BEAM' %}
//	{{key('PBAR')}}{{ rpad(part.integerid) }}{{ rpad(model.find_material(part.material_id).integerid) }}{% for section in fe.sections %}{{ fnum(section.area_cross_section)}}{{ fnum(section.moment_of_inertia_Z)}}{{endl("")}}
//	{{nextl("")}}{{ fnum(section.moment_of_inertia_Y)}}{{ fnum(section.torsional_constant)}}{{ fnum(fe.additional_mass)}}{%- endfor %}
//	{%- elif fe.finite_element_type == 'CIRCULAR_BEAM' %}
//	{{key('PBARL')}}{{ rpad(part.integerid) }}{{ rpad(model.find_material(part.material_id).integerid) }}{{lpad("")}}{{rpad("ROD")}}{{endl("")}}
//	{{nextl("")}}{{fnum(fe.sections[0].radius)}}
//	{%- endif -%}
//	{%- endfor %}

	out << "ENDDATA" << endl;

	out.close();
	return datPath;
}

} //end of namespace nastran
} //end of namespace vega

