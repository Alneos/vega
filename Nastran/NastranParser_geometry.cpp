/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * NastranParser.cpp
 *
 *  Created on: Dec 24, 2012
 *      Author: dallolio
 */

//#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "NastranParser.h"

using namespace std;
namespace alg = boost::algorithm;
using boost::assign::list_of;
using boost::lexical_cast;
using namespace boost::assign;

namespace vega {

namespace nastran {

const unordered_map<CellType::Code, vector<int>, hash<int>> NastranParserImpl::nastran2medNodeConnectByCellType =
		{
				{ CellType::TRI3_CODE, { 0, 2, 1 } },
				{ CellType::TRI6_CODE, { 0, 2, 1, 5, 4, 3 } },
				{ CellType::QUAD4_CODE, { 0, 3, 2, 1 } },
				{ CellType::QUAD8_CODE, { 0, 3, 2, 1, 7, 6, 5, 4 } },
				{ CellType::QUAD9_CODE, { 0, 3, 2, 1, 7, 6, 5, 4, 8 } },
				{ CellType::TETRA4_CODE, { 0, 2, 1, 3 } },
				{ CellType::TETRA10_CODE, { 0, 2, 1, 3, 6, 5, 4, 7, 9, 8 } },
				{ CellType::PYRA5_CODE, { 0, 3, 2, 1, 4 } },
				{ CellType::PYRA13_CODE, { 0, 3, 2, 1, 4, 8, 7, 6, 5, 9, 12, 11, 10 } },
				{ CellType::PENTA6_CODE, { 0, 2, 1, 3, 5, 4 } },
				{ CellType::PENTA15_CODE, { 0, 2, 1, 3, 5, 4, 8, 7, 6, 12, 14, 13, 11, 10, 9 } },
				{ CellType::HEXA8_CODE, { 0, 3, 2, 1, 4, 7, 6, 5 } },
				{ CellType::HEXA20_CODE, { 0, 3, 2, 1, 4, 7, 6, 5, 11, 10, 9, 8, 16, 19, 18, 17, 15,
						14, 13, 12 } }
		};

void NastranParserImpl::parseGRDSET(NastranTokenizer& tok, shared_ptr<Model> model) {
	UNUSEDV(model);
	tok.skip(1);
	grdSet.cp = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	tok.skip(3);
	grdSet.cd = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	grdSet.ps = tok.nextInt(true, 0);
	grdSet.seid = tok.nextInt(true, 0);

}

void NastranParserImpl::parseGRID(NastranTokenizer& tok, shared_ptr<Model> model) {
	int id = tok.nextInt();
	int cp = tok.nextInt(true, grdSet.cp);
	if (cp != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID)
		handleParsingError("GRID CP coordinate system not supported", tok, model);

	double x1 = tok.nextDouble();
	double x2 = tok.nextDouble();
	double x3 = tok.nextDouble();
	int cd = tok.nextInt(true, grdSet.cd);
	model->mesh->addNode(id, x1, x2, x3, cd);
	int ps = tok.nextInt(true, grdSet.ps);
	if (ps) {
		string spcName = string("SPC") + lexical_cast<string>(id);
		SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(ps));
		spc.addNodeId(id);
		model->add(spc);
		model->addConstraintIntoConstraintSet(spc, model->commonConstraintSet);
	}

	if (this->logLevel >= LogLevel::TRACE) {
		cout << fixed << "GRID " << id << ":" << x1 << ";" << x2 << ";" << x3 << endl;
	}
}

void NastranParserImpl::addProperty(int property_id, int cell_id, shared_ptr<Model> model) {
	CellGroup* cellGroup = getOrCreateCellGroup(property_id, model);
	cellGroup->addCell(cell_id);
}

CellGroup* NastranParserImpl::getOrCreateCellGroup(int property_id, shared_ptr<Model> model, const string & command) {
	CellGroup* cellGroup = dynamic_cast<CellGroup*>(model->mesh->findGroup(property_id));

	string cellGroupName= command + string("_") + lexical_cast<string>(property_id);
	if (cellGroup == nullptr){
		cellGroup = model->mesh->createCellGroup(cellGroupName, property_id, command);
	}else{
		/* If the Group already exists, and if it was not already done, we enforce the name and comment of the Group */
		if (cellGroup->getName().substr(0,6)=="CGVEGA"){
			cellGroup->setName(cellGroupName);
			cellGroup->setComment(command);
		}
	}
	return cellGroup;
}

Orientation* NastranParserImpl::parseOrientation(int point1, int cell_id, NastranTokenizer& tok,
		shared_ptr<Model> model) {
	UNUSEDV(cell_id);
	vector<string> line = tok.currentDataLine();
	bool alternateFormat = line.size() < 8 || line[6].empty() || line[7].empty();
	Orientation* orientation;
	if (alternateFormat) {
		int g0 = tok.nextInt();
		orientation = new TwoNodesOrientation(*model, point1, g0);
	} else {
		double x1, x2, x3;
		x1 = tok.nextDouble();
		x2 = tok.nextDouble();
		x3 = tok.nextDouble();
		orientation = new VectY(x1, x2, x3);
	}
	return orientation;
}

void NastranParserImpl::parseCBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
	int cell_id = tok.nextInt();
	int property_id = tok.nextInt();
	int point1 = tok.nextInt();
	int point2 = tok.nextInt();
	Orientation* orientation = parseOrientation(point1, cell_id, tok, model);
	string offt = tok.nextString(true);
	if (!offt.empty() && offt != "GGG") {
		string message = string("CBAR OFFT not supported.") + string(" OFFT:") + offt;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	vector<int> connectivity;
	connectivity += point1, point2;

	model->mesh->addCell(cell_id, CellType::SEG2, connectivity, false, orientation);
	delete (orientation);
	addProperty(property_id, cell_id, model);
}

void NastranParserImpl::parseCBEAM(NastranTokenizer& tok, shared_ptr<Model> model) {
	int cell_id = tok.nextInt();
	int property_id = tok.nextInt();
	int point1 = tok.nextInt();
	int point2 = tok.nextInt();
	Orientation* orientation = parseOrientation(point1, cell_id, tok, model);
	string offt = tok.nextString(true);
	if (!offt.empty() && offt != "GGG") {
		string message = string("CBEAM OFFT not supported.") + string(" OFFT:") + offt;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	double w1A = tok.nextDouble(true, 0.0);
	double w2A = tok.nextDouble(true, 0.0);
	double w3A = tok.nextDouble(true, 0.0);
	double w1B = tok.nextDouble(true, 0.0);
	double w2B = tok.nextDouble(true, 0.0);
	double w3B = tok.nextDouble(true, 0.0);
	if (!is_zero(w1A) || !is_zero(w2A) || !is_zero(w3A) || !is_zero(w1B) || !is_zero(w2B)
			|| !is_zero(w3B)) {
		handleParsingError("CBEAM Wxx parameter !=0.0 not supported.", tok, model);
	}
	vector<int> connectivity;
	connectivity += point1, point2;

	model->mesh->addCell(cell_id, CellType::SEG2, connectivity, false, orientation);
	delete (orientation);
	addProperty(property_id, cell_id, model);
}

void NastranParserImpl::parseElem(NastranTokenizer& tok, shared_ptr<Model> model,
								  vector<CellType> cellTypes) {
	int cell_id = tok.nextInt();
	int property_id = tok.nextInt(true, cell_id);
	auto it = cellTypes.begin();
	CellType& cellType = *it;
	vector<int> nastranConnect;
	unsigned int i = 0;
	while (tok.isNextInt()) {
		if (it == cellTypes.end())
			throw ParsingException("format element not supported", tok.fileName, tok.lineNumber);
		cellType = *it;
		for (; i < cellType.numNodes; i++)
			nastranConnect.push_back(tok.nextInt());
		it++;
	}
	vector<int> medConnect;
	auto nastran2med_it = nastran2medNodeConnectByCellType.find(cellType.code);
	if (nastran2med_it == nastran2medNodeConnectByCellType.end()) {
		medConnect = nastranConnect;
	} else {
		vector<int> nastran2medNodeConnect = nastran2med_it->second;
		medConnect.resize(cellType.numNodes);
		for (unsigned int i = 0; i < cellType.numNodes; i++)
			medConnect[nastran2medNodeConnect[i]] = nastranConnect[i];
	}
	model->mesh->addCell(cell_id, cellType, medConnect);
	addProperty(property_id, cell_id, model);
}

void NastranParserImpl::parseCGAP(NastranTokenizer& tok, shared_ptr<Model> model) {
	int eid = tok.nextInt();
	int pid = tok.nextInt();
	int ga = tok.nextInt();
	int gb = tok.nextInt();
	Orientation* orientation = parseOrientation(ga, eid, tok, model);
	delete (orientation);

	shared_ptr<Constraint> gapPtr = model->find(Reference<Constraint>(Constraint::GAP, pid));
	if (!gapPtr) {
		GapTwoNodes gapConstraint(*model, pid);
		gapConstraint.addGapNodes(ga, gb);
		model->add(gapConstraint);
		model->addConstraintIntoConstraintSet(gapConstraint, model->commonConstraintSet);
	} else {
		shared_ptr<GapTwoNodes> gapConstraint = static_pointer_cast<GapTwoNodes>(gapPtr);
		gapConstraint->addGapNodes(ga, gb);
	}
}

void NastranParserImpl::parseCHEXA(NastranTokenizer& tok, shared_ptr<Model> model) {
	try {
		parseElem(tok, model, { CellType::HEXA8, CellType::HEXA20 });
	} catch (ParsingException &e) {
		handleParseException(e, model, "HEXA mandatory field missing");
	}
}

void NastranParserImpl::parseCPENTA(NastranTokenizer& tok, shared_ptr<Model> model) {
	try {
		parseElem(tok, model, { CellType::PENTA6, CellType::PENTA15 });
	} catch (ParsingException &e) {
		handleParseException(e, model, "PENTA mandatory field missing");
	}
}

void NastranParserImpl::parseCPYRAM(NastranTokenizer& tok, shared_ptr<Model> model) {
	try {
		parseElem(tok, model, { CellType::PYRA5, CellType::PYRA13 });
	} catch (ParsingException &e) {
		handleParseException(e, model, "PYRAM mandatory field missing");
	}
}

void NastranParserImpl::parseCQUAD(NastranTokenizer& tok, shared_ptr<Model> model) {
	try {
		parseElem(tok, model, { CellType::QUAD4, CellType::QUAD8, CellType::QUAD9 });
	} catch (ParsingException &e) {
		handleParseException(e, model, "QUAD mandatory field missing");
	}
}

void NastranParserImpl::parseCROD(NastranTokenizer& tok, shared_ptr<Model> model) {
	//not found in doc, copied from Vega
	int cell_id = tok.nextInt();
	int property_id = tok.nextInt(true, cell_id);
	int point1 = tok.nextInt();
	int point2 = tok.nextInt();
	CellType cellType = CellType::SEG2;
	vector<int> coords;
	coords += point1, point2;
	model->mesh->addCell(cell_id, cellType, coords);
	addProperty(property_id, cell_id, model);
}

void NastranParserImpl::parseCTETRA(NastranTokenizer& tok, shared_ptr<Model> model) {
	try {
		parseElem(tok, model, { CellType::TETRA4, CellType::TETRA10 });
	} catch (ParsingException &e) {
		handleParseException(e, model, "TETRA mandatory field missing");
	}
}

void NastranParserImpl::parseShellElem(NastranTokenizer& tok, shared_ptr<Model> model,
		CellType cellType) {
	int cell_id = tok.nextInt();
	int property_id = tok.nextInt(true, cell_id);

	vector<int> coords;
	vector<double> ti;
	int i = 0;
	double thetaOrMCID;
	double zoffs;
	int tflag;

	CellType::Code code = cellType.code;
	switch (code) {
	case CellType::TRI3_CODE:
		for (; i < 3; i++)
			coords.push_back(tok.nextInt());
		thetaOrMCID = tok.nextDouble(true);
		zoffs = tok.nextDouble(true);
		tok.skip(2);
		tflag = tok.nextInt(true);
		if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			for (; i < 3; i++)
				ti.push_back(tok.nextDouble());
		}
		break;
	case CellType::QUAD4_CODE:
		for (; i < 4; i++)
			coords.push_back(tok.nextInt());
		thetaOrMCID = tok.nextDouble(true);
		zoffs = tok.nextDouble(true);
		tflag = tok.nextInt(true);
		if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			for (; i < 4; i++)
				ti.push_back(tok.nextDouble());
		}
		break;
	case CellType::TRI6_CODE:
		for (; i < 6; i++)
			coords.push_back(tok.nextInt());
		thetaOrMCID = tok.nextDouble(true);
		zoffs = tok.nextDouble(true);
		if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			for (; i < 6; i++)
				ti.push_back(tok.nextDouble());
		}
		tflag = tok.nextInt(true);
		break;

	case CellType::QUAD8_CODE:
		for (; i < 8; i++)
			coords.push_back(tok.nextInt());
		thetaOrMCID = tok.nextDouble(true);
		zoffs = tok.nextDouble(true);
		if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			for (; i < 8; i++)
				ti.push_back(tok.nextDouble());
		}
		tflag = tok.nextInt(true);
		break;

	default:
		//nothing
		cerr << "not impl" << endl;
	}
	if (!is_equal(thetaOrMCID, NastranTokenizer::UNAVAILABLE_DOUBLE)
			|| !is_equal(zoffs, NastranTokenizer::UNAVAILABLE_DOUBLE)
			|| tflag != NastranTokenizer::UNAVAILABLE_INT) {
		const string msg = "Keywords not supported in: " + tok.currentRawDataLine();
		if (translationMode == ConfigurationParameters::MODE_STRICT) {
			throw ParsingException(msg, tok.fileName, tok.lineNumber);
		} else if (translationMode == ConfigurationParameters::MESH_AT_LEAST) {
			model->onlyMesh = true;
		}
		cerr << msg << " file: " << tok.fileName << " line: " << tok.lineNumber << endl;
	}

	model->mesh->addCell(cell_id, cellType, coords);
	addProperty(property_id, cell_id, model);

}

} /* namespace nastran */

} /* namespace vega */
