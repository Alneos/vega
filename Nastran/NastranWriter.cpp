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

Line& Line::add(const char* value) {
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
            out << "$ WARN analysis " << *firstAnalysis << " not supported. Skipping." << endl;
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
            out << "$ WARN analysis " << *firstAnalysis << " not supported. Skipping." << endl;
        }
        break;
    }
	default:
		handleWritingError("Unsupported dialect");
	}

	out << "SOL " << analysisLabel << endl;
	firstAnalysis->markAsWritten();
	if (model.analyses.size() >= 2) {
        for (auto& analysis : model.analyses) {
            out << "SUBCASE " << analysis->bestId() << endl;
            for (const auto& loadSet : analysis->getLoadSets()) {
                switch (loadSet->type) {
                case LoadSet::Type::LOAD: {
                    out << "LOAD=" << loadSet->bestId() << endl;
                    loadSet->markAsWritten();
                    break;
                }
                default:
                    handleWritingError("LoadSet type not (yet) implemented");
                }
            }
            for (const auto& constraintSet : analysis->getConstraintSets()) {
                switch (constraintSet->type) {
                case ConstraintSet::Type::SPC: {
                    out << "SPC=" << constraintSet->bestId() << endl;
                    constraintSet->markAsWritten();
                    break;
                }
                default:
                    handleWritingError("LoadSet type not (yet) implemented");
                }
            }
            analysis->markAsWritten();
        }
	}

}

void NastranWriter::writeCells(const Model& model, ofstream& out) const
		{
	for (const auto& elementSet : model.elementSets) {
		if (elementSet->isDiscrete() || elementSet->isMatrixElement()) {
			continue;
		}
		for (const Cell& cell : elementSet->getCellsIncludingGroups()) {
			string keyword;
			if (elementSet->isBeam()) {
				keyword = "CBEAM";
            } else if (elementSet->isDiscrete() and cell.type.code == CellType::Code::SEG2_CODE) {
                keyword = "CBUSH";
			} else if (elementSet->isShell()) {
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
		material->markAsWritten();
	}
}

void NastranWriter::writeConstraints(const Model& model, ofstream& out) const
		{
	for (const auto& constraintSet : model.constraintSets) {
		const auto& spcs = constraintSet->getConstraintsByType(Constraint::Type::SPC);
        for (shared_ptr<Constraint> constraint : spcs) {
            shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<
                    SinglePointConstraint>(constraint);
            for (int nodePosition : spc->nodePositions()) {
                const int nodeId = model.mesh.findNodeId(nodePosition);
                out
                        << Line("SPC1").add(constraintSet->bestId()).add(
                                spc->getDOFSForNode(nodePosition)).add(nodeId);
            }
            spc->markAsWritten();
        }
		const auto& rigidConstraints = constraintSet->getConstraintsByType(Constraint::Type::RIGID);
        for (shared_ptr<Constraint> constraint : rigidConstraints) {
            shared_ptr<RigidConstraint> rigid =
                    dynamic_pointer_cast<RigidConstraint>(constraint);
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
            rigid->markAsWritten();
        }

		const auto& quasiRigidConstraints = constraintSet->getConstraintsByType(
				Constraint::Type::QUASI_RIGID);
        for (shared_ptr<Constraint> constraint : quasiRigidConstraints) {
            shared_ptr<QuasiRigidConstraint> quasiRigid =
                    dynamic_pointer_cast<QuasiRigidConstraint>(constraint);
            if (not quasiRigid->hasMaster() and quasiRigid->getSlaves().size() == 2) {
                Line rbar("RBAR");
                rbar.add(constraintSet->bestId());
                for (int slavePosition : quasiRigid->getSlaves()) {
                    rbar.add(model.mesh.findNodeId(slavePosition));
                }
                for (int slavePosition : quasiRigid->getSlaves()) {
                    rbar.add(quasiRigid->getDOFSForNode(slavePosition));
                }
                out << rbar;
            } else {
                Line rbe2("RBE2");
                rbe2.add(constraintSet->bestId());
                const int masterId = model.mesh.findNodeId(quasiRigid->getMaster());
                rbe2.add(masterId);
                rbe2.add(quasiRigid->getDOFS());
                for (int slavePosition : quasiRigid->getSlaves()) {
                    const int slaveId = model.mesh.findNodeId(slavePosition);
                    rbe2.add(slaveId);
                }
                out << rbe2;
            }
            quasiRigid->markAsWritten();
        }

		const auto& rbe3Constraints = constraintSet->getConstraintsByType(
				Constraint::Type::RBE3);
        for (shared_ptr<Constraint> constraint : rbe3Constraints) {
            shared_ptr<RBE3> rbe3Constraint =
                    dynamic_pointer_cast<RBE3>(constraint);
            Line rbe3("RBE3");
            rbe3.add(constraintSet->bestId());
            rbe3.add();
            const int masterPosition = model.mesh.findNodeId(rbe3Constraint->getMaster());
            rbe3.add(model.mesh.findNodeId(masterPosition));
            rbe3.add(rbe3Constraint->getDOFSForNode(masterPosition));
            for (int slavePosition : rbe3Constraint->getSlaves()) {
                rbe3.add(rbe3Constraint->getCoefForNode(slavePosition));
                rbe3.add(rbe3Constraint->getDOFSForNode(slavePosition));
                rbe3.add(model.mesh.findNodeId(slavePosition));
            }
            out << rbe3;
            rbe3Constraint->markAsWritten();
        }
		constraintSet->markAsWritten();
	}
}

void NastranWriter::writeLoadings(const Model& model, ofstream& out) const
		{
	for (const auto& loadingSet : model.loadSets) {
		const auto& gravities = loadingSet->getLoadingsByType(Loading::Type::GRAVITY);
        for (shared_ptr<Loading> loading : gravities) {
            shared_ptr<Gravity> gravity = dynamic_pointer_cast<Gravity>(loading);
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
            gravity->markAsWritten();
        }

		const auto& forceSurfaces = loadingSet->getLoadingsByType(
				Loading::Type::FORCE_SURFACE);
        for (shared_ptr<Loading> loading : forceSurfaces) {
            shared_ptr<ForceSurface> forceSurface = dynamic_pointer_cast<ForceSurface>(loading);
            Line pload4("PLOAD4");
            pload4.add(loadingSet->bestId());
            const auto& cellIds = forceSurface->getCellIdsIncludingGroups();
            if (cellIds.size() > 1) {
                throw logic_error("Unimplemented multiple cells in PLOAD4");
            }
            pload4.add(*cellIds.begin());
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
            forceSurface->markAsWritten();
        }

		const auto& nodalForces = loadingSet->getLoadingsByType(
				Loading::Type::NODAL_FORCE);
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
            nodalForce->markAsWritten();
        }
		loadingSet->markAsWritten();
	}
}

void NastranWriter::writeRuler(ofstream& out) const
		{
	out << "$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]"
			<< endl;
}

void NastranWriter::writeElements(const Model& model, ofstream& out) const
		{
	for (shared_ptr<Beam> truss : model.getTrusses()) {
		Line prod("PROD");
		prod.add(truss->bestId());
		prod.add(truss->material->bestId());
		prod.add(truss->getAreaCrossSection());
		prod.add(truss->getTorsionalConstant());
		out << truss;
		truss->markAsWritten();
	}
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
		beam->markAsWritten();
	}
	for (shared_ptr<ElementSet> shell : model.elementSets.filter(ElementSet::Type::SHELL)) {
		Line pshell("PSHELL");
		pshell.add(shell->bestId());
		pshell.add(shell->material->bestId());
		out << pshell;
		shell->markAsWritten();
	}
	for (shared_ptr<ElementSet> continuum : model.elementSets.filter(ElementSet::Type::CONTINUUM)) {
	    string keyword = dialect == Dialect::COSMIC95 ? "PIHEX" : "PSOLID";
		Line psolid(keyword);
		psolid.add(continuum->bestId());
		psolid.add(continuum->material->bestId());
		out << psolid;
		continuum->markAsWritten();
	}
    for (shared_ptr<ElementSet> elementSet: model.elementSets.filter(ElementSet::Type::DISCRETE_0D)) {
        shared_ptr<const DiscretePoint> discretePoint = dynamic_pointer_cast<const DiscretePoint>(elementSet);
        if (discretePoint->hasStiffness() or discretePoint->hasDamping()) {
            throw logic_error("discrete not completely written");
        }
        for (int nodePosition: discretePoint->nodePositions()) {
            Line conm1("CONM1");
            conm1.add(discretePoint->bestId());
            conm1.add(model.mesh.findNodeId(nodePosition));
            conm1.add(0); // CID
            for (int row = 0; row < 5; row++) {
                const DOF rowdof = DOF::findByPosition(row);
                for (int col = 0; col < row; col++) {
                    const DOF coldof = DOF::findByPosition(col);
                    conm1.add(discretePoint->findMass(rowdof, coldof));
                }
            }
            out << conm1;
            elementSet->markAsWritten();
        }
	}
    for (shared_ptr<ElementSet> elementSet: model.elementSets.filter(ElementSet::Type::NODAL_MASS)) {
        shared_ptr<const NodalMass> mass = dynamic_pointer_cast<const NodalMass>(elementSet);
        for (int nodePosition: mass->nodePositions()) {
            Line conm2("CONM2");
            conm2.add(mass->bestId());
            conm2.add(model.mesh.findNodeId(nodePosition));
            conm2.add(0); // CID
            conm2.add(mass->getMass());
            out << conm2;
            elementSet->markAsWritten();
        }
	}
    for (shared_ptr<ElementSet> elementSet: model.elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT)) {
        shared_ptr<const StructuralSegment> segment = dynamic_pointer_cast<const StructuralSegment>(elementSet);
        if (segment->hasMass()) {
            throw logic_error("discrete not completely written");
        }
        Line pbush("PBUSH");
        pbush.add(segment->bestId());
        pbush.add(0); // CID
        pbush.add("K");
        for (const DOF dof: DOFS::ALL_DOFS) {
            pbush.add(segment->findStiffness(dof, dof));
        }
        pbush.add("");
        pbush.add("B");
        for (const DOF dof: DOFS::ALL_DOFS) {
            pbush.add(segment->findDamping(dof, dof));
        }
        out << pbush;
        elementSet->markAsWritten();
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

	out << "ENDDATA" << endl;

	out.close();
	return nasPath;
}

} //end of namespace nastran
} //end of namespace vega

