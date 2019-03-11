/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranWriter.cpp
 *
 *  Created on: 5 mars 2013
 *      Author: dallolio
 */

#include "build_properties.h"
#include "../Abstract/Model.h"
#include "NastranWriter.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <fstream>
#include <limits>
#include <ciso646>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;
using namespace std;

namespace vega {
namespace nastran {

int Line::newlineCounter = 0;

ostream &operator<<(ostream &out, const Line& line) {
	out << left << setw(8);
	out << line.keyword;
	int fieldCount = 0;
	for(const auto& field : line.fields) {
		fieldCount++;
		if (fieldCount % line.fieldNum == 0) {
            string newlinesep = "+" + to_string(++Line::newlineCounter);
			out << newlinesep << endl;
			out << setw(line.fieldLength) << newlinesep;
		}
		out << field;
	}
	out << endl;
	return out;
}

Line::Line(string _keyword) : keyword(_keyword) {
	if (boost::algorithm::ends_with(keyword, "*")) {
		fieldLength = 16;
		fieldNum = 5;
	} else {
		fieldLength = 8;
		fieldNum = 9;
	}
}

Line& Line::add() {
	this->add(string());
	return *this;
}

Line& Line::add(double value) {
	std::ostringstream strs;
	if (is_zero(value)) {
        strs << "0.";
	} else {
	    // https://github.com/SteveDoyle2/pyNastran/blob/master/pyNastran/bdf/field_writer_8.py
	    string str1 = str(boost::format("%8.11e") % value);
	    boost::algorithm::trim(str1);
	    size_t pos = str1.find("e");
	    double mant = stod(str1.substr(0, pos));
	    string exp2 = to_string(stoi(str1.substr(pos + 1))); // "00" becomes 0, "01" becomes 1 etc.
	    boost::algorithm::trim_left_if(exp2, boost::is_any_of("-+"));
	    char sign = abs(value) < 1. ? '-' : '+';
	    size_t leftover = 5 - exp2.size();
	    leftover -= value < 0 ? 1 : 0;
	    const string fmt = str(boost::format("%%1.%sf") % leftover);
        string svalue3 = str(boost::format(fmt) % mant);
        boost::algorithm::trim_if(svalue3, boost::is_any_of("0"));
        strs << boost::format("%8s") % (svalue3 + sign + exp2);
	}
	//strs << boost::format("%.5f") % value;
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

const unordered_map<CellType::Code, vector<int>, EnumClassHash> NastranWriter::med2nastranNodeConnectByCellType =
        {
                { CellType::Code::TRI3_CODE, { 0, 2, 1 } },
                { CellType::Code::TRI6_CODE, { 0, 2, 1, 5, 4, 3 } },
                { CellType::Code::QUAD4_CODE, { 0, 3, 2, 1 } },
                { CellType::Code::QUAD8_CODE, { 0, 3, 2, 1, 7, 6, 5, 4 } },
                { CellType::Code::QUAD9_CODE, { 0, 3, 2, 1, 7, 6, 5, 4, 8 } },
                { CellType::Code::TETRA4_CODE, { 0, 2, 1, 3 } },
                { CellType::Code::TETRA10_CODE, { 0, 2, 1, 3, 6, 5, 4, 7, 9, 8 } },
                { CellType::Code::PYRA5_CODE, { 0, 3, 2, 1, 4 } },
                { CellType::Code::PYRA13_CODE, { 0, 3, 2, 1, 4, 8, 7, 6, 5, 9, 12, 11, 10 } },
                { CellType::Code::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
                { CellType::Code::PENTA15_CODE, { 0, 2, 1, 3, 5, 4, 8, 7, 6, 12, 14, 13, 11, 10, 9 } },
                { CellType::Code::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
                { CellType::Code::HEXA20_CODE, { 0, 3, 2, 1, 4, 7, 6, 5, 11, 10, 9, 8, 16, 19, 18, 17, 15,
                        14, 13, 12 } }
        };

const string NastranWriter::toString() const {
	return string("NastranWriter");
}

string NastranWriter::getNasFilename(const Model& model,
		const string& outputPath) const
		{
	string outputFileName;
	if (model.name.empty()) {
		outputFileName = "nastran";
	} else {
		outputFileName = model.name;
		const size_t period_idx = outputFileName.rfind('.');
		if (string::npos != period_idx) {
			outputFileName = outputFileName.substr(0, period_idx);
		}
	}
	string modelPath = outputFileName + "_vg.nas";
	bool absolute = true;
	if (absolute) {
		modelPath = (fs::absolute(outputPath) / modelPath).string();
	}
	return modelPath;
}

void NastranWriter::writeSOL(const Model& model, ofstream& out) const
		{
	auto& firstAnalysis = *model.analyses.begin();
	string analysisLabel;
	switch(dialect) {
    case(Dialect::COSMIC95): {
        switch (firstAnalysis->type) {
        case (Analysis::Type::LINEAR_MECA_STAT): {
            analysisLabel = "1,1";
            break;
        }
        case (Analysis::Type::LINEAR_MODAL): {
            analysisLabel = "1,3";
            break;
        }
        case (Analysis::Type::NONLINEAR_MECA_STAT): {
            analysisLabel = "1,6";
            break;
        }
        default:
            out << "$ WARN analysis " << firstAnalysis << " not supported. Skipping." << endl;
        }

        break;
    }
    case(Dialect::MODERN): {
        switch (firstAnalysis->type) {
        case (Analysis::Type::LINEAR_MECA_STAT): {
            analysisLabel = "101";
            break;
        }
        case (Analysis::Type::LINEAR_MODAL): {
            analysisLabel = "103";
            break;
        }
        case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
            analysisLabel = "111";
            break;
        }
        case (Analysis::Type::NONLINEAR_MECA_STAT): {
            analysisLabel = "106";
            break;
        }
        default:
            out << "$ WARN analysis " << firstAnalysis << " not supported. Skipping." << endl;
        }
        break;
    }
	default:
		handleWritingError("Unsupported dialect");
	}

	out << "SOL " << analysisLabel << endl;
}

void NastranWriter::writeCells(const Model& model, ofstream& out) const
		{
	for (const auto& elementSet : model.elementSets) {
		if (elementSet->isDiscrete() || elementSet->isMatrixElement()) {
			continue;
		}
		shared_ptr<CellGroup> cellGroup = elementSet->cellGroup;
		for (const Cell& cell : cellGroup->getCells()) {
			string keyword;
			if (elementSet->isBeam()) {
				keyword = "CBEAM";
			} else
			if (elementSet->isShell()) {
				switch (cell.type.code) {
				case CellType::Code::TRI3_CODE:
					keyword = "CTRIA3";
					break;
				case CellType::Code::TRI6_CODE:
					keyword = "CTRIA6";
					break;
				case CellType::Code::QUAD4_CODE:
					keyword = "CQUAD4";
					break;
				case CellType::Code::QUAD8_CODE:
					keyword = "CQUAD8";
					break;
				default:
					throw logic_error("Unimplemented type");
				}
			} else if (elementSet->type == ElementSet::Type::CONTINUUM) {
			    if (dialect == Dialect::COSMIC95) {
                    switch (cell.type.code) {
                    case CellType::Code::HEXA8_CODE:
                        keyword = "CIHEX1";
                        break;
                    case CellType::Code::HEXA20_CODE:
                        keyword = "CIHEX2";
                        break;
                    case CellType::Code::TETRA4_CODE:
                    case CellType::Code::TETRA10_CODE:
                        keyword = "CTETRA";
                        break;
                    default:
                        throw logic_error("Unimplemented type");
                    }
			    } else {
                    switch (cell.type.code) {
                    case CellType::Code::HEXA8_CODE:
                    case CellType::Code::HEXA20_CODE:
                        keyword = "CHEXA";
                        break;
                    case CellType::Code::TETRA4_CODE:
                    case CellType::Code::TETRA10_CODE:
                        keyword = "CTETRA";
                        break;
                    default:
                        throw logic_error("Unimplemented type");
                    }
			    }
			}
            vector<int> nasConnect;
            auto entry = med2nastranNodeConnectByCellType.find(cell.type.code);
            if (entry == med2nastranNodeConnectByCellType.end()) {
                nasConnect = cell.nodeIds;
            } else {
                vector<int> med2nastranNodeConnectByCellType = entry->second;
                nasConnect.resize(cell.type.numNodes);
                for (unsigned int i2 = 0; i2 < cell.type.numNodes; i2++)
                    nasConnect[med2nastranNodeConnectByCellType[i2]] = cell.nodeIds[i2];
            }
			out << Line(keyword).add(cell.id).add(elementSet->bestId()).add(nasConnect);
		}
	}
}

void NastranWriter::writeNodes(const Model& model, ofstream& out) const
		{
	for (Node node : model.mesh.nodes) {
	    if (node.positionCS!= CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID)
	        cerr << "Warning in GRID "<<node.id<<" CP not supported and dismissed."<<endl;
        if (node.displacementCS!= CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID)
            cerr << "Warning in GRID "<<node.id<<" CD not supported and dismissed."<<endl;
		out << Line("GRID").add(node.id).add().add(node.lx).add(node.ly).add(node.lz);
	}
}

void NastranWriter::writeMaterials(const Model& model, ofstream& out) const
		{
	for (const auto& material : model.materials) {
		Line mat1("MAT1");
		mat1.add(material->bestId());
		const auto& enature = material->findNature(Nature::NatureType::NATURE_ELASTIC);
		if (enature) {
			const ElasticNature& elasticNature = dynamic_cast<const ElasticNature&>(*enature);
			mat1.add(elasticNature.getE());
			mat1.add(elasticNature.getG());
			mat1.add(elasticNature.getNu());
			mat1.add(elasticNature.getRho());
			if (!is_zero(elasticNature.getGE())){
			    cerr <<"Warning in Material: no support for GE material properties yet.";
			}
		}
		out << mat1;
	}
}

void NastranWriter::writeConstraints(const Model& model, ofstream& out) const
		{
	for (const auto& constraintSet : model.constraintSets) {
		const set<shared_ptr<Constraint> >& spcs = constraintSet->getConstraintsByType(
				Constraint::Type::SPC);
		if (spcs.size() > 0) {
			for (shared_ptr<Constraint> constraint : spcs) {
				shared_ptr<const SinglePointConstraint> spc = dynamic_pointer_cast<
						const SinglePointConstraint>(constraint);
				for (int nodePosition : spc->nodePositions()) {
					const int nodeId = model.mesh.findNodeId(nodePosition);
					out
							<< Line("SPC1").add(constraintSet->bestId()).add(
									spc->getDOFSForNode(nodePosition)).add(nodeId);
				}
			}
		}
		const set<shared_ptr<Constraint> >& rigidConstraints = constraintSet->getConstraintsByType(
				Constraint::Type::RIGID);
		if (rigidConstraints.size() > 0) {
			for (shared_ptr<Constraint> constraint : rigidConstraints) {
				shared_ptr<const RigidConstraint> rigid =
						dynamic_pointer_cast<const RigidConstraint>(constraint);
				Line rbe2("RBE2");
				rbe2.add(constraintSet->bestId());
				const int masterId = model.mesh.findNodeId(rigid->getMaster());
				rbe2.add(masterId);
				rbe2.add(DOFS::ALL_DOFS);
				for (int slavePosition : rigid->getSlaves()) {
					const int slaveId = model.mesh.findNodeId(slavePosition);
					rbe2.add(slaveId);
				}
				out << rbe2;
			}
		}
	}
}

void NastranWriter::writeLoadings(const Model& model, ofstream& out) const
		{
	for (const auto& loadingSet : model.loadSets) {
		const set<shared_ptr<Loading> > gravities = loadingSet->getLoadingsByType(Loading::Type::GRAVITY);
		if (gravities.size() > 0) {
			for (shared_ptr<Loading> loading : gravities) {
				shared_ptr<const Gravity> gravity = dynamic_pointer_cast<const Gravity>(loading);
				Line grav("GRAV");
				grav.add(loadingSet->bestId());
				if (gravity->hasCoordinateSystem()) {
					throw logic_error("Coordinate System ID writing not yet implemented");
				} else {
					grav.add(0);
				}
				grav.add(gravity->getAccelerationScale());
				grav.add(gravity->getGravityVector());
				out << grav;
			}
		}

		const set<shared_ptr<Loading> > forceSurfaces = loadingSet->getLoadingsByType(
				Loading::Type::FORCE_SURFACE);
		if (forceSurfaces.size() > 0) {
			for (shared_ptr<Loading> loading : forceSurfaces) {
				shared_ptr<ForceSurface> forceSurface = dynamic_pointer_cast<ForceSurface>(loading);
				Line pload4("PLOAD4");
				pload4.add(loadingSet->bestId());
				vector<Cell> cells = forceSurface->getCells(true);
				if (cells.size() == 1) {
					pload4.add(cells[0].id);
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
				pload4.add(forceSurface->getApplicationFaceNodeIds()[0]);
				pload4.add(forceSurface->getApplicationFaceNodeIds()[2]);
				if (forceSurface->hasCoordinateSystem()) {
					shared_ptr<CoordinateSystem> coordinateSystem = model.mesh.findCoordinateSystem(forceSurface->csref);
					pload4.add(coordinateSystem->bestId());
					pload4.add(coordinateSystem->vectorToGlobal(forceSurface->getForce().normalized()));
				} else {
					pload4.add(0);
					pload4.add(forceSurface->getForce().normalized());
				}
				out << pload4;
			}
		}

		const set<shared_ptr<Loading> > nodalForces = loadingSet->getLoadingsByType(
				Loading::Type::NODAL_FORCE);
		if (nodalForces.size() > 0) {
			for (shared_ptr<Loading> loading : nodalForces) {
				shared_ptr<NodalForce> nodalForce = dynamic_pointer_cast<NodalForce>(loading);
				Line force("FORCE");
				force.add(loadingSet->bestId());
				if (nodalForce->nodePositions().size() != 1) {
                    throw logic_error("Multiple nodes in nodal force, not yet implemented (but easy)");
				}
				int nodePosition = *(nodalForce->nodePositions().begin());
				force.add(model.mesh.findNodeId(nodePosition));
				force.add(0);
				const auto& forceVector = nodalForce->getForceInGlobalCS(nodePosition);
				force.add(1.0);
				force.add(forceVector.x());
				force.add(forceVector.y());
				force.add(forceVector.z());
				if (!nodalForce->getMomentInGlobalCS(nodePosition).iszero()) {
					throw logic_error("Unimplemented moment in FORCE");
				}
                out << force;
			}
		}
	}
}

void NastranWriter::writeRuler(ofstream& out) const
		{
	out << "$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]"
			<< endl;
}

void NastranWriter::writeElements(const Model& model, ofstream& out) const
		{
	for (shared_ptr<Beam> beam : model.getBeams()) {
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
	for (shared_ptr<ElementSet> shell : model.elementSets.filter(ElementSet::Type::SHELL)) {
		Line pshell("PSHELL");
		pshell.add(shell->bestId());
		pshell.add(shell->material->bestId());
		out << pshell;
	}
	for (shared_ptr<ElementSet> continuum : model.elementSets.filter(ElementSet::Type::CONTINUUM)) {
	    string keyword = dialect == Dialect::COSMIC95 ? "PIHEX" : "PSOLID";
		Line psolid(keyword);
		psolid.add(continuum->bestId());
		psolid.add(continuum->material->bestId());
		out << psolid;
	}
}

string NastranWriter::writeModel(Model& model,
		const vega::ConfigurationParameters &configuration) {

    if (configuration.nastranOutputDialect == "cosmic95") {
        dialect = Dialect::COSMIC95;
    } else {
        dialect = Dialect::MODERN;
    }

	string outputPath = configuration.outputPath;
	if (!fs::exists(outputPath)) {
		throw iostream::failure("Directory " + outputPath + " don't exist.");
	}

	string nasPath = getNasFilename(model, outputPath);
	ofstream out;
	out.precision(DBL_DIG);
	out.open(nasPath.c_str(), ios::out | ios::trunc);
	if (!out.is_open()) {
		string message = string("Can't open file ") + nasPath + " for writing.";
		throw ios::failure(message);
	}

	out << "$ " << model.name << endl;
	out << "ID " << model.name << "," << "NASTRAN" << endl;
	writeSOL(model, out);
	out << "APP   DISP" << endl;
	out << "TIME  10000" << endl;
	out << "CEND" << endl;
	for (const auto& analysis : model.analyses) {
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
	out << "$" << endl;
	out << "TITLE=Vega Exported Model" << endl;
	out << "BEGIN BULK" << endl;
	out << "PARAM,PRGPST,NO" << endl;
	out << "PARAM,AUTOSPC,1" << endl;

	for (const auto& coordinateSystemEntry : model.mesh.coordinateSystemStorage.coordinateSystemByRef) {
        shared_ptr<CoordinateSystem> coordinateSystem = coordinateSystemEntry.second;
        if (coordinateSystem->type != CoordinateSystem::Type::ABSOLUTE) {
            continue;
        }
		switch (coordinateSystem->coordType) {
			case CoordinateSystem::CoordinateType::CARTESIAN:
				// TODO LD complete
				out << Line("CORD2R").add(coordinateSystem->bestId()).add(coordinateSystem->getOrigin());
				break;
			case CoordinateSystem::CoordinateType::SPHERICAL:
				// TODO LD complete
				out << Line("CORD2S").add(coordinateSystem->bestId()).add(coordinateSystem->getOrigin());
				break;
			case CoordinateSystem::CoordinateType::CYLINDRICAL:
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
	return nasPath;
}

} //end of namespace nastran
} //end of namespace vega

