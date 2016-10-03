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

#include "NastranParser.h"
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <ciso646>

namespace vega {

namespace nastran {

using namespace std;
namespace fs = boost::filesystem;
namespace alg = boost::algorithm;
using boost::lexical_cast;
using boost::trim;
using boost::trim_copy;

// see also http://www.altairhyperworks.com/hwhelp/Altair/hw12.0/help/hm/hmbat.htm?design_variables.htm
const set<string> NastranParserImpl::IGNORED_KEYWORDS = {
		"DCONSTR", "DCONADD", "DESVAR", "DLINK", //nastran optimization keywords
		"DRAW", "DRESP1", "DRESP2", //ignored in Vega
		//optistruct optimization variable
		"DOPTPRM", "DCOMP", //Manufacturing constraints for composite sizing optimization.
		"DESVAR", //Design variable definition.
		"DSHAPE", //Free-shape design variable definition.
		"DSHUFFLE", //Parameters for the generation of composite shuffling design variables.
		"DSIZE", "DTPG", //Topography design variable definition.
		"DTPL", //Topology design variable definition.
		"TOPVAR", //  Topological Design Variable
		"DVGRID", "DEQATN",
		"DREPORT", "DREPADD", // Optistruct Cards
		"ENDDATA"
};

const unordered_map<string, NastranParserImpl::parseElementFPtr> NastranParserImpl::PARSE_FUNCTION_BY_KEYWORD =
		{
				{ "CBAR", &NastranParserImpl::parseCBAR },
				{ "CBEAM", &NastranParserImpl::parseCBEAM },
				{ "CBUSH", &NastranParserImpl::parseCBUSH },
				{ "CGAP", &NastranParserImpl::parseCGAP },
				{ "CELAS2", &NastranParserImpl::parseCELAS2 },
				{ "CELAS4", &NastranParserImpl::parseCELAS4 },
				{ "CHEXA", &NastranParserImpl::parseCHEXA },
				{ "CMASS2", &NastranParserImpl::parseCMASS2 },
				{ "CONM2", &NastranParserImpl::parseCONM2 },
				{ "CORD1R", &NastranParserImpl::parseCORD1R },
				{ "CORD2C", &NastranParserImpl::parseCORD2C },
				{ "CORD2R", &NastranParserImpl::parseCORD2R },
				{ "CPENTA", &NastranParserImpl::parseCPENTA },
				{ "CPYRAMID", &NastranParserImpl::parseCPYRAM },
				{ "CQUAD", &NastranParserImpl::parseCQUAD },
				{ "CROD", &NastranParserImpl::parseCROD },
				{ "CTETRA", &NastranParserImpl::parseCTETRA },
				{ "DAREA", &NastranParserImpl::parseDAREA },
				{ "DLOAD", &NastranParserImpl::parseDLOAD },
				{ "DMIG", &NastranParserImpl::parseDMIG },
				{ "EIGR", &NastranParserImpl::parseEIGR },
				{ "EIGRL", &NastranParserImpl::parseEIGRL },
				{ "FORCE", &NastranParserImpl::parseFORCE },
				{ "FORCE1", &NastranParserImpl::parseFORCE1 },
				{ "FREQ1", &NastranParserImpl::parseFREQ1 },
				{ "GRAV", &NastranParserImpl::parseGRAV },
				{ "GRID", &NastranParserImpl::parseGRID },
				{ "INCLUDE", &NastranParserImpl::parseInclude },
				{ "LOAD", &NastranParserImpl::parseLOAD },
				{ "MAT1", &NastranParserImpl::parseMAT1 },
				{ "MATS1", &NastranParserImpl::parseMATS1 },
				{ "MOMENT", &NastranParserImpl::parseMOMENT },
				{ "MPC", &NastranParserImpl::parseMPC },
				{ "NLPARM", &NastranParserImpl::parseNLPARM },
				{ "PARAM", &NastranParserImpl::parsePARAM },
				{ "PBAR", &NastranParserImpl::parsePBAR },
				{ "PBARL", &NastranParserImpl::parsePBARL },
				{ "PBEAM", &NastranParserImpl::parsePBEAM },
				{ "PBEAML", &NastranParserImpl::parsePBEAML },
				{ "PBUSH", &NastranParserImpl::parsePBUSH },
				{ "PGAP", &NastranParserImpl::parsePGAP },
				{ "PLOAD4", &NastranParserImpl::parsePLOAD4 },
				{ "PROD", &NastranParserImpl::parsePROD },
				{ "PSHELL", &NastranParserImpl::parsePSHELL },
				{ "PSOLID", &NastranParserImpl::parsePSOLID },
				{ "RBAR", &NastranParserImpl::parseRBAR },
				{ "RBAR1", &NastranParserImpl::parseRBAR1 },
				{ "RBE2", &NastranParserImpl::parseRBE2 },
				{ "RBE3", &NastranParserImpl::parseRBE3 },
				{ "RFORCE", &NastranParserImpl::parseRFORCE },
				{ "RLOAD2", &NastranParserImpl::parseRLOAD2 },
				{ "SPC", &NastranParserImpl::parseSPC },
				{ "SPC1", &NastranParserImpl::parseSPC1 },
				{ "SPCD", &NastranParserImpl::parseSPCD },
				{ "SPCADD", &NastranParserImpl::parseSPCADD },
				{ "TABDMP1", &NastranParserImpl::parseTABDMP1 },
				{ "DPHASE", &NastranParserImpl::parseDPHASE },
				{ "TABLED1", &NastranParserImpl::parseTABLED1 },
				{ "GRDSET", &NastranParserImpl::parseGRDSET }
		};

NastranParserImpl::NastranParserImpl() :
		Parser() {
	this->translationMode = ConfigurationParameters::BEST_EFFORT;

}

string NastranParserImpl::parseSubcase(NastranTokenizer& tok, shared_ptr<Model> model,
		map<string, string> context) {
	int subCaseId = tok.nextInt(true, 0);
	while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
		tok.nextSymbolString();
	}
	string nextKeyword = tok.nextString();
	while (nextKeyword != "BEGIN" && nextKeyword != "SUBCASE") {
		context[nextKeyword] = tok.nextString(true);
		while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			tok.nextSymbolString();
		}
		nextKeyword = tok.nextString();
	}
	addAnalysis(model, context, subCaseId);
	return nextKeyword;
}

NastranParserImpl::~NastranParserImpl() {
}

void NastranParserImpl::parseExecutiveSection(NastranTokenizer& tok, shared_ptr<Model> model,
		map<string, string>& context) {
	bool canContinue = true;
	bool readNewKeyword = true;
	bool subCaseFound = false;
	string keyword = "";
	do {
		if (readNewKeyword) {
			keyword = tok.nextSymbolString();
			trim(keyword);
		} else {
			//new keyword was read by a parsing method
			readNewKeyword = true;
		}
		if (keyword.find("BEGIN") != string::npos) {
			if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				tok.nextLine();
			}
			if (!subCaseFound) {
				addAnalysis(model, context);
			}
			canContinue = false;
		} else if (keyword == "B2GG") {
			// Selects direct input damping matrix or matrices.
			string line;
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				line += tok.nextSymbolString();
			}
			trim(line);
			/*
			 The matrices are additive if multiple matrices are referenced on the B2GG
			 command.
			 The formats of the name list:
			 a. Names without factor.
			 Names separated by comma or blank.
			 b. Names with factors.
			 Each entry in the list consists of a factor followed by a star followed by a
			 name. The entries are separated by comma or blank. The factors are real
			 numbers. Each name must be with a factor including 1.0.
			 */
			if (line.find_first_of(", *") != std::string::npos) {
				throw logic_error("complex names not yet implemented");
			}
			istringstream iss(line);
			int num = 0;
			if (!(iss >> num).fail()) {
				throw logic_error("set references not yet implemented " + to_string(num));
			}
			DampingMatrix matrix(*model); // LD : TODO string identifier here
			directMatrixByName[line] = matrix.getReference().clone();
			model->add(matrix);
		} else if (keyword == "CEND") {
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				tok.nextSymbolString();
			}
		} else if (keyword == "K2GG") {
			// Selects direct input stiffness matrix or matrices.
			string line;
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				line += tok.nextSymbolString();
			}
			trim(line);
			/*
			 The matrices are additive if multiple matrices are referenced on the K2GG
			 command.
			 The formats of the name list:
			 a. Names without factor.
			 Names separated by comma or blank.
			 b. Names with factors.
			 Each entry in the list consists of a factor followed by a star followed by a
			 name. The entries are separated by comma or blank. The factors are real
			 numbers. Each name must be with a factor including 1.0.
			 */
			if (line.find_first_of(", *") != std::string::npos) {
				throw logic_error("complex names not yet implemented");
			}
			istringstream iss(line);
			int num = 0;
			if (!(iss >> num).fail()) {
				throw logic_error("set references not yet implemented " + to_string(num));
			}
			StiffnessMatrix matrix(*model); // LD : TODO string identifier here
			directMatrixByName[line] = matrix.getReference().clone();
			model->add(matrix);
		} else if (keyword == "M2GG") {
			// Selects direct input mass matrix or matrices.
			string line;
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				line += tok.nextSymbolString();
			}
			trim(line);
			/*
			 The matrices are additive if multiple matrices are referenced on the M2GG
			 command.
			 The formats of the name list:
			 a. Names without factor.
			 Names separated by comma or blank.
			 b. Names with factors.
			 Each entry in the list consists of a factor followed by a star followed by a
			 name. The entries are separated by comma or blank. The factors are real
			 numbers. Each name must be with a factor including 1.0.
			 */
			if (line.find_first_of(", *") != std::string::npos) {
				throw logic_error("complex names not yet implemented");
			}
			istringstream iss(line);
			int num = 0;
			if (!(iss >> num).fail()) {
				throw logic_error("set references not yet implemented " + to_string(num));
			}
			MassMatrix matrix(*model); // LD : TODO string identifier here
			directMatrixByName[line] = matrix.getReference().clone();
			model->add(matrix);
		} else if (keyword == "SUBCASE") {
			keyword = parseSubcase(tok, model, context);
			readNewKeyword = false;
			subCaseFound = true;
		} else if (keyword == "SUBTITLE") {
			string line;
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD)
				line += tok.nextSymbolString();
			model->description = line;
		} else if (keyword == "TITLE") {
			string line;
			while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD)
				line += tok.nextSymbolString();
			model->title = line;
		} else {
			if (tok.nextSymbolType != NastranTokenizer::SYMBOL_FIELD) {
				context[keyword] = string("");
			} else {
				string line;
				while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD)
					line += tok.nextSymbolString();
				context[keyword] = line;
			}
		}
		canContinue = canContinue && (tok.nextSymbolType != NastranTokenizer::SYMBOL_EOF);
	} while (canContinue);
}

void NastranParserImpl::parseBULKSection(NastranTokenizer &tok, shared_ptr<Model> model) {

	while (tok.nextSymbolType == NastranTokenizer::SYMBOL_KEYWORD) {
		string keyword = tok.nextSymbolString();
		trim(keyword);
		unordered_map<string, NastranParserImpl::parseElementFPtr>::const_iterator parseFunctionFptrKeywordPair;
		//TODO: move these to the map
		if (keyword == "CQUAD4" || keyword == "CQUADR") {
			parseShellElem(tok, model, CellType::QUAD4);
		} else if (keyword == "CQUAD8") {
			parseShellElem(tok, model, CellType::QUAD8);
		} else if (keyword == "CTRIA3" || keyword == "CTRIAR") {
			parseShellElem(tok, model, CellType::TRI3);
		} else if (keyword == "CTRIA6") {
			parseShellElem(tok, model, CellType::TRI6);
		} else if ((parseFunctionFptrKeywordPair = PARSE_FUNCTION_BY_KEYWORD.find(keyword))
				!= PARSE_FUNCTION_BY_KEYWORD.end()) {
			try {
				NastranParserImpl::parseElementFPtr fptr = parseFunctionFptrKeywordPair->second;
				(this->*fptr)(tok, model);
			} catch (ParsingException &e) {
				if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
					tok.nextLine();
				}
				string message = string("Error parsing keyword ")
						+ parseFunctionFptrKeywordPair->first;
				handleParseException(e, model, message);
			}
		} else if (IGNORED_KEYWORDS.find(keyword) != IGNORED_KEYWORDS.end()) {
			if (model->configuration.logLevel >= LogLevel::TRACE) {
				cout << "Keyword " << keyword << " ignored." << endl;
			}
			tok.nextLine();
		} else if (keyword.empty()) {
			continue;
		} else {
			handleParsingError(string("Unknown keyword ") + keyword, tok, model);
			if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
				tok.nextLine();
			}
			continue;
		}

		//there are unparsed fields. Skip the empty ones
		if (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD && !tok.isNextEmpty()) {
			string message(
					string("Error parsing keyword ") + keyword
							+ ": parsing of line not complete. ");
			handleParsingError(message, tok, model);
		}
		while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			tok.nextSymbolString();
		}
	}

}

fs::path NastranParserImpl::findModelFile(const string& filename) {
	if (!fs::exists(filename)) {
		throw invalid_argument("Can't find file : " + fs::absolute(filename).string());
	}
	fs::path inputFilePath(filename);
	return inputFilePath;
}

shared_ptr<Model> NastranParserImpl::parse(const ConfigurationParameters& configuration) {
	this->translationMode = configuration.translationMode;
	this->logLevel = configuration.logLevel;

	const string filename = configuration.inputFile;

	fs::path inputFilePath = findModelFile(filename);
	const string modelName = inputFilePath.filename().string();
	shared_ptr<Model> model = shared_ptr<Model>(new Model(modelName, "UNKNOWN", NASTRAN,
			configuration.getModelConfiguration()));
	map<string, string> executive_section_context;
	const string inputFilePathStr = inputFilePath.string();
	ifstream istream(inputFilePathStr);
	NastranTokenizer tok = NastranTokenizer(istream, logLevel, inputFilePath.string());

	parseExecutiveSection(tok, model, executive_section_context);

	tok.bulkSection();
	parseBULKSection(tok, model);
	istream.close();

	return model;
}

void NastranParserImpl::addAnalysis(shared_ptr<Model> model, map<string, string> &context,
		int analysis_id) {

	string analysis_str;
	auto it = context.find("SOL");
	if (it != context.end())
		analysis_str = trim_copy(it->second);
	else
		analysis_str = "101";

	if (analysis_str == "200" || analysis_str == "DESOPT") {
		auto it = context.find("ANALYSIS");
		if (it != context.end()) {
			string analysis = trim_copy(it->second);
			if (analysis == "STATICS" || analysis == "")
				analysis_str = "101";
			else if (analysis == "MODES")
				analysis_str = "103";
			else if (analysis == "NLSTATIC")
				analysis_str = "106";
			else if (analysis == "MFREQ")
				analysis_str = "111";
			else {
				string message = "Analysis " + analysis + " Not implemented";
				throw ParsingException(message, "", 0);
			}
		} else
			analysis_str = "101";
	}

	// Finding label
	string labelAnalysis="Analysis_"+to_string(analysis_id);
	it = context.find("LABEL");
	if (it != context.end())
		labelAnalysis = trim_copy(it->second);


	if (analysis_str == "101" || analysis_str == "SESTATIC") {

		LinearMecaStat analysis(*model, labelAnalysis, analysis_id);

		for (auto it = context.begin(); it != context.end(); it++) {
			string key = it->first;
			int id = atoi(it->second.c_str());
			if (!key.compare(0, 3, "SPC")) {
				Reference<ConstraintSet> constraintReference(ConstraintSet::SPC, id);
				analysis.add(constraintReference);
				if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
					ConstraintSet constraintSet(*model, ConstraintSet::SPC, id);
					model->add(constraintSet);
				}
			} else if (!key.compare(0, 3, "MPC")) {
				Reference<ConstraintSet> constraintReference(ConstraintSet::MPC, id);
				analysis.add(constraintReference);
				if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
					ConstraintSet constraintSet(*model, ConstraintSet::MPC, id);
					model->add(constraintSet);
				}
			} else if (!key.compare(0, 4, "LOAD")) {
				Reference<LoadSet> loadsetReference(LoadSet::LOAD, id);
				analysis.add(loadsetReference);
			}
		}

		model->add(analysis);

	} else if (analysis_str == "103" || analysis_str == "SEMODES") {

		map<string, string>::iterator it;
		int frequency_band_original_id = 0;
		for (it = context.begin(); it != context.end(); it++) {
			if (it->first.find("METHOD") != string::npos) {
				frequency_band_original_id = atoi(it->second.c_str());
				break;
			}
		}
		if (it == context.end())
			throw ParsingException("METHOD not found for linear modal analysis", "", 0);

		LinearModal analysis(*model, frequency_band_original_id, labelAnalysis, analysis_id);

		it = context.find("SPC");
		if (it != context.end()) {
			int id = atoi(it->second.c_str());
			Reference<ConstraintSet> constraintReference(ConstraintSet::SPC, id);
			analysis.add(constraintReference);
			if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
				ConstraintSet constraintSet(*model, ConstraintSet::SPC, id);
				model->add(constraintSet);
			}
		}
		it = context.find("MPC");
		if (it != context.end()) {
			int id = atoi(it->second.c_str());
			Reference<ConstraintSet> constraintReference(ConstraintSet::MPC, id);
			analysis.add(constraintReference);
			if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
				ConstraintSet constraintSet(*model, ConstraintSet::MPC, id);
				model->add(constraintSet);
			}
		}

		model->add(analysis);

	} else if (analysis_str == "106" || analysis_str == "NLSTATIC") {

		auto it = context.find("NLPARM");
		int strategy_original_id = 0;
		if (it == context.end())
			throw ParsingException("NLPARM not found for non linear analysis", "", 0);
		else
			strategy_original_id = atoi(it->second.c_str());

		NonLinearMecaStat analysis(*model, strategy_original_id, labelAnalysis, analysis_id);

		for (auto it = context.begin(); it != context.end(); it++) {
			string key = it->first;
			int id = atoi(it->second.c_str());
			if (!key.compare(0, 3, "SPC")) {
				Reference<ConstraintSet> constraintReference(ConstraintSet::SPC, id);
				analysis.add(constraintReference);
				if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
					ConstraintSet constraintSet(*model, ConstraintSet::SPC, id);
					model->add(constraintSet);
				}
			} else if (!key.compare(0, 3, "MPC")) {
				Reference<ConstraintSet> constraintReference(ConstraintSet::MPC, id);
				analysis.add(constraintReference);
				if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
					ConstraintSet constraintSet(*model, ConstraintSet::MPC, id);
					model->add(constraintSet);
				}
			} else if (!key.compare(0, 4, "LOAD")) {
				Reference<LoadSet> loadsetReference(LoadSet::LOAD, id);
				analysis.add(loadsetReference);
			}
		}
		/*
		 The subcase structure provides a unique means of changing loads,
		 boundary conditions, and solution methods by making selections from the Bulk Data.
		 Confining the discussion to SOL 66 (or 106) and SOL 99 (or 129),
		 loads and solution methods may change from subcase to subcase
		 on an incremental basis. However, constraints can be changed
		 from subcase to subcase only in the static solution sequence.
		 As a result, the subcase structure determines a sequence of loading
		 and constraint paths in a nonlinear analysis.
		 The subcase structure also allows the user to selectand change
		 output requests for printout, plot, etc., by specifying set numbers with keywords.
		 Any selections made above the subcase specifications are applicable to all the subcases.
		 Selectionsmade in an individual subcase supersede the selections made above the subcases.
		 */
		if (model->analyses.size() >= 1) {
			for (auto previous : model->analyses) {
				analysis.previousAnalysis = previous;
			}
		}
		model->add(analysis);

	} else if (analysis_str == "111" || analysis_str == "SEMFREQ") {

		int frequency_band_original_id = 0;
		int modal_damping_original_id = 0;
		int frequency_value_original_id = 0;

		map<string, string>::iterator it;
		for (it = context.begin(); it != context.end(); it++) {
			if (it->first.find("METHOD") == 0)
				frequency_band_original_id = stoi(it->second);
			if (it->first.find("SDAMPING") == 0)
				modal_damping_original_id = stoi(it->second);
			if (it->first.find("FREQ") == 0)
				frequency_value_original_id = stoi(it->second);
		}

		if (frequency_band_original_id == 0)
			throw ParsingException("METHOD not found for linear dynamic modal frequency analysis",
					"", 0);
		if (modal_damping_original_id == 0)
			throw ParsingException("SDAMPING not found for linear dynamic modal frequency analysis",
					"", 0);
		if (frequency_value_original_id == 0)
			throw ParsingException("FREQ not found for linear dynamic modal frequency analysis", "",
					0);

		/*
		 auto it = context.find("METHOD");
		 int frequency_band_original_id = 0;
		 if (it == context.end())
		 it = context.find("METHOD(STRUCTURE)");
		 if (it == context.end())
		 throw ParsingException("METHOD not found for linear dynamic modal frequency analysis",
		 "", 0);
		 else
		 frequency_band_original_id = atoi(it->second.c_str());

		 it = context.find("SDAMPING");
		 int modal_damping_original_id = 0;
		 if (it == context.end())
		 it = context.find("SDAMPING(STRUCTURE)");
		 if (it == context.end())
		 throw ParsingException("SDAMPING not found for linear dynamic modal frequency analysis",
		 "", 0);
		 else
		 modal_damping_original_id = atoi(it->second.c_str());

		 it = context.find("FREQ");
		 int frequency_value_original_id = 0;
		 if (it == context.end())
		 it = context.find("FREQUENCY");
		 if (it == context.end())
		 throw ParsingException("FREQ not found for linear dynamic modal frequency analysis", "",
		 0);
		 else
		 frequency_value_original_id = atoi(it->second.c_str());
		 */

		bool residual_vector = false;
		it = context.find("RESVEC(NOINREL)");
		if (it != context.end() && it->second == "YES")
			residual_vector = true;

		LinearDynaModalFreq analysis(*model, frequency_band_original_id, modal_damping_original_id,
				frequency_value_original_id, residual_vector, labelAnalysis, analysis_id);

		it = context.find("SPC");
		if (it != context.end()) {
			int id = atoi(it->second.c_str());
			Reference<ConstraintSet> constraintReference(ConstraintSet::SPC, id);
			analysis.add(constraintReference);
			if (!model->find(constraintReference)) { // constraintSet is added in the model if not found in the model
				ConstraintSet constraintSet(*model, ConstraintSet::SPC, id);
				model->add(constraintSet);
			}
		}
		it = context.find("DLOAD");
		if (it != context.end()) {
			int id = atoi(it->second.c_str());
			Reference<LoadSet> loadsetReference(LoadSet::DLOAD, id);
			analysis.add(loadsetReference);
		}

		model->add(analysis);

	} else {
		string message = "Analysis " + analysis_str + " Not implemented";
		throw ParsingException(message, "", 0);
	}
}

void NastranParserImpl::parseCONM2(NastranTokenizer& tok, shared_ptr<Model> model) {
	int elemId = tok.nextInt();
	int g = tok.nextInt(); // Grid point identification number
	int ci = tok.nextInt(true, 0);
	if (ci != 0) {
		string message = "CONM2 coordinate system not implemented.";
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	const double mass = tok.nextDouble();
	const double x1 = tok.nextDouble(true, 0.0);
	const double x2 = tok.nextDouble(true, 0.0);
	const double x3 = tok.nextDouble(true, 0.0);
	if (!tok.isEmptyUntilNextKeyword())
		tok.skip(1);
	const double i11 = tok.nextDouble(true, 0.0);
	const double i21 = tok.nextDouble(true, 0.0);
	const double i22 = tok.nextDouble(true, 0.0);
	const double i31 = tok.nextDouble(true, 0.0);
	const double i32 = tok.nextDouble(true, 0.0);
	const double i33 = tok.nextDouble(true, 0.0);

	NodalMass nodalMass(*model, mass, i11, i22, i33, -i21, -i31, -i32, x1, x2, x3, elemId);

	int cellPosition = model->mesh->addCell(elemId, CellType::POINT1, { g });
	string mn = string("CONM2_") + lexical_cast<string>(elemId);
	CellGroup* mnodale = model->mesh->createCellGroup(mn, CellGroup::NO_ORIGINAL_ID, "NODAL MASS");
	mnodale->addCell(model->mesh->findCell(cellPosition).id);
	nodalMass.assignCellGroup(mnodale);
	nodalMass.assignMaterial(model->getVirtualMaterial());

	model->add(nodalMass);
}

void NastranParserImpl::parseCORD1R(NastranTokenizer& tok, shared_ptr<Model> model) {

	while (tok.isNextInt()) {
		int cid = tok.nextInt();
		int nA  = tok.nextInt();
		int nB  = tok.nextInt();
		int nC  = tok.nextInt();
	    CartesianCoordinateSystem coordinateSystem(*model, nA, nB, nC, cid);
	    model->add(coordinateSystem);
		}
}

void NastranParserImpl::parseCORD2R(NastranTokenizer& tok, shared_ptr<Model> model) {
	int cid = tok.nextInt();
	//reference coordinate system 0 for global.
	int rid = tok.nextInt(true, 0);
	if (rid != 0) {
		throw ParsingException("REference coordinate system != 0 not implemented", tok.fileName,
				tok.lineNumber);
	}
	double coor[3];
	VectorialValue vect[3];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			coor[j] = tok.nextDouble();
		vect[i] = VectorialValue(coor[0], coor[1], coor[2]);
	}

	VectorialValue ez = vect[1] - vect[0];
	VectorialValue ex = (vect[2] - vect[0]).orthonormalized(ez);
	VectorialValue ey = ez.cross(ex);

	CartesianCoordinateSystem coordinateSystem(*model, vect[0], ex, ey, cid);
	model->add(coordinateSystem);
}

void NastranParserImpl::parseCORD2C(NastranTokenizer& tok, shared_ptr<Model> model) {
	int cid = tok.nextInt();
	//reference coordinate system 0 for global.
	int rid = tok.nextInt(true, 0);
	if (rid != 0) {
		throw ParsingException("REference coordinate system != 0 not implemented", tok.fileName,
				tok.lineNumber);
	}

	double coor[3];
	VectorialValue vect[3];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			coor[j] = tok.nextDouble();
		vect[i] = VectorialValue(coor[0], coor[1], coor[2]);
	}

	VectorialValue ez = vect[1] - vect[0];
	VectorialValue ex = (vect[2] - vect[0]).orthonormalized(ez);
	VectorialValue ey = ez.cross(ex);

	CylindricalCoordinateSystem coordinateSystem(*model, vect[0], ex, ey, cid);
	model->add(coordinateSystem);
}

void NastranParserImpl::parseFORCE(NastranTokenizer& tok, shared_ptr<Model> model) {

	int loadset_id = tok.nextInt();
	int node_id = tok.nextInt();
	int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	double force = tok.nextDouble(true);
	double fx = tok.nextDouble(true) * force;
	double fy = tok.nextDouble(true) * force;
	double fz = tok.nextDouble(true) * force;

	NodalForce force1(*model, node_id, fx, fy, fz, 0., 0., 0., Loading::NO_ORIGINAL_ID,
			coordinate_system_id);

	model->add(force1);
	Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
	model->addLoadingIntoLoadSet(force1, loadset_ref);
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
		model->add(loadSet);
	}
}
void NastranParserImpl::parseFORCE1(NastranTokenizer& tok, shared_ptr<Model> model) {
//nodal force two nodes
	int loadset_id = tok.nextInt();
	int node_id = tok.nextInt();
	double force = tok.nextDouble();
	int node1 = tok.nextInt();
	int node2 = tok.nextInt();

	NodalForceTwoNodes force1(*model, node_id, node1, node2, force);

	model->add(force1);
	Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
	model->addLoadingIntoLoadSet(force1, loadset_ref);
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
		model->add(loadSet);
	}
}
void NastranParserImpl::parseGRAV(NastranTokenizer& tok, shared_ptr<Model> model) {
	int sid = tok.nextInt();
	int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	if (coordinate_system_id != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
		string message = "GRAV : CoordinateSystem not supported" + tok.lineNumber;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	double acceleration = tok.nextDouble(true, 0);
	double x = tok.nextDouble(true, 0);
	double y = tok.nextDouble(true, 0);
	double z = tok.nextDouble(true, 0);
	int mb = tok.nextInt(true, 0);
	if (mb != 0) {
		string message = "GRAV MB not supported line" + tok.lineNumber;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	Gravity gravity(*model, acceleration, VectorialValue(x, y, z));

	model->add(gravity);
	Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, sid);
	model->addLoadingIntoLoadSet(gravity, loadset_ref);
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::LOAD, sid);
		model->add(loadSet);
	}
}
void NastranParserImpl::parseInclude(NastranTokenizer& tok, shared_ptr<Model> model) {
	string currentRawDataLine = tok.currentRawDataLine();
	string fileName = currentRawDataLine.substr(7, currentRawDataLine.length() - 7);
	trim(fileName);
	if (!fileName.compare(0, 1, "'")
			&& !fileName.compare(fileName.size() - 1, fileName.size(), "'"))
		fileName = fileName.substr(1, fileName.size() - 2);
	fs::path currentFname(tok.fileName);
	fs::path includePath = currentFname.parent_path() / fileName;
	if (fs::exists(includePath)) {
		const string includePathStr = includePath.string();
		ifstream istream(includePathStr);
		NastranTokenizer tok2 = NastranTokenizer(istream, this->logLevel, includePathStr);
		tok2.bulkSection();
		try {
			parseBULKSection(tok2, model);
		} catch (ParsingException & e) {
			cerr << e.what();
			if (translationMode == ConfigurationParameters::MODE_STRICT) {
				throw ParsingException(string("Error parsing include ") + includePathStr,
						tok.fileName, tok.lineNumber);
			}
		}
		istream.close();
	} else {
		cerr << "File " << includePath << " included by " << tok.fileName << " at line "
				<< tok.lineNumber << " can not be found" << endl;
		if (translationMode == ConfigurationParameters::MODE_STRICT) {
			throw ParsingException("Missing include", tok.fileName, tok.lineNumber);
		}
	}
	tok.nextLine();
}

void NastranParserImpl::parseMAT1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int material_id = tok.nextInt();
	double e = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double g = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double nu = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double rho = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double a = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double tref = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double ge = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
    
	// Default behavior from page 1664 of MDN Nastran 2006 Quick Reference Guide
	if ((is_equal(e,NastranTokenizer::UNAVAILABLE_DOUBLE))&&(is_equal(g,NastranTokenizer::UNAVAILABLE_DOUBLE))){
		string message = "Material " + to_string(material_id)+": E and G may not both be blank.";
		handleParsingWarning(message, tok, model);
	}
	if (is_equal(nu,NastranTokenizer::UNAVAILABLE_DOUBLE)){
		if (is_equal(g,NastranTokenizer::UNAVAILABLE_DOUBLE)){
			nu = 0.0;
			g = 0.0;
		}else if (is_equal(e,NastranTokenizer::UNAVAILABLE_DOUBLE)){
			nu = 0.0;
			e = 0.0;
		}else
			nu = e/(2.0*g)-1;
	}
	if ((is_equal(e,NastranTokenizer::UNAVAILABLE_DOUBLE))&&
			(!is_equal(g,NastranTokenizer::UNAVAILABLE_DOUBLE))&&(!is_equal(nu,NastranTokenizer::UNAVAILABLE_DOUBLE))){
		e= 2.0 * (1+nu) * g;
	}
	if ((is_equal(g,NastranTokenizer::UNAVAILABLE_DOUBLE))&&
			(!is_equal(e,NastranTokenizer::UNAVAILABLE_DOUBLE))&&(!is_equal(nu,NastranTokenizer::UNAVAILABLE_DOUBLE))){
        g = e / (2.0 * (1+nu));
	}


	if (!is_equal(ge, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		string message = "GE not supported line " + to_string(tok.lineNumber);
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	/*	ST, SC, SS
	 (Real)
	 Stress limits for tension, compression, and shear are optionally
	 supplied, used only to compute margins of safety in certain elements;
	 and have no effect on the computational procedures. See “Beam
	 Element (CBEAM)” in Chapter 3 of the MSC.Nastran Reference Guide.
	 (Real > 0.0 or blank)*/
	double st = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	if (!is_equal(st, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingWarning("st value ignored " + to_string(st), tok, model);
	}
	double sc = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	if (!is_equal(sc, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingWarning("sc value ignored " + to_string(sc), tok, model);
	}
	double ss = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	if (!is_equal(ss, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingWarning("ss value ignored " + to_string(ss), tok, model);
	}
	/*	MCSID
	 Material coordinate system identification number. Used only for
	 PARAM,CURV processing. See “Parameters” on page 659.
	 (Integer > 0 or blank)*/
	int mcsid = tok.nextInt(true, 0);
	if (mcsid != 0) {
		handleParsingWarning("mcsid value ignored " + to_string(mcsid), tok, model);
	}
	shared_ptr<Material> material = model->getOrCreateMaterial(material_id);
	material->addNature(ElasticNature(*model, e, nu, g, rho, a, tref));
}

void NastranParserImpl::parseMATS1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int mid = tok.nextInt();
	auto material = model->getOrCreateMaterial(mid);
	int tid = tok.nextInt(true, 0);
	string type = tok.nextString();
	double h = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	int yf = tok.nextInt(true, 1);
	int hr = tok.nextInt(true, 1);
	double limit1 = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	double limit2 = tok.nextDouble(true, NastranTokenizer::UNAVAILABLE_DOUBLE);
	if (type == "NLELAST") {
		NonLinearElasticNature nonLinearElasticNature = NonLinearElasticNature(*model, tid);
		material->addNature(nonLinearElasticNature);
	} else if (type == "PLASTIC") {
		if (tid == 0) {
			BilinearElasticNature biNature = BilinearElasticNature(*model);
			biNature.elastic_limit = limit1;
			if (!is_equal(limit2, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
				handleParsingError("MATS1 limit2 " + to_string(yf) + " not yet implemented.", tok,
						model);
			}
			biNature.secondary_slope = h;
			switch (hr) {
			case 1:
				biNature.hardening_rule_isotropic = true;
				break;
			default:
				handleParsingError("MATS1 hr " + to_string(hr) + " not yet implemented.", tok,
						model);
				break;
			}
			switch (yf) {
			case 1:
				biNature.yield_function_von_mises = true;
				break;
			default:
				handleParsingError("MATS1 yf " + to_string(yf) + " not yet implemented.", tok,
						model);
				break;
			}
			material->addNature(biNature);
		} else {
			handleParsingError(
					"MATS1 TID " + to_string(tid) + " not yet implemented for plastic law.", tok,
					model);
		}
	} else {
		handleParsingError("MATS1 type " + type + " not implemented.", tok, model);
	}
}

void NastranParserImpl::parseMOMENT(NastranTokenizer& tok, shared_ptr<Model> model) {
	int loadset_id = tok.nextInt();
	int node_id = tok.nextInt();
	int coordinate_system_id = tok.nextInt(true, 0);
	if (coordinate_system_id != 0) {
		throw ParsingException("MOMENT coordinate system not supported", tok.fileName,
				tok.lineNumber);
	}
	double scale = tok.nextDouble(true);
	double frx = tok.nextDouble(true) * scale;
	double fry = tok.nextDouble(true) * scale;
	double frz = tok.nextDouble(true) * scale;

	NodalForce force1(*model, node_id, VectorialValue(0, 0, 0), VectorialValue(frx, fry, frz),
			Loading::NO_ORIGINAL_ID);
	model->add(force1);
	Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
	model->addLoadingIntoLoadSet(force1, loadset_ref);
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
		model->add(loadSet);
	}
}

void NastranParserImpl::parsePBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
	int elemId = tok.nextInt();
	int material_id = tok.nextInt();
	double area = tok.nextDouble();
	const double i1 = tok.nextDouble(true, 0.0); // I1 = Izz
	const double i2 = tok.nextDouble(true, 0.0); // I2 = Iyy
	const double j = tok.nextDouble(true, 0.0);  // J = Ixx
	const double nsm = tok.nextDouble(true, 0.0);
	if (!tok.isEmptyUntilNextKeyword())
		tok.skip(1);
	double c1 = tok.nextDouble(true, 0.0);
	double c2 = tok.nextDouble(true, 0.0);
	double d1 = tok.nextDouble(true, 0.0);
	double d2 = tok.nextDouble(true, 0.0);
	double e1 = tok.nextDouble(true, 0.0);
	double e2 = tok.nextDouble(true, 0.0);
	double f1 = tok.nextDouble(true, 0.0);
	double f2 = tok.nextDouble(true, 0.0);
	if (!is_equal(c1, 0.0)
			|| !is_equal(c2, 0.0)
			|| !is_equal(d1, 0.0)
			|| !is_equal(d2, 0.0)
			|| !is_equal(e1, 0.0)
			|| !is_equal(e2, 0.0)
			|| !is_equal(f1, 0.0)
			|| !is_equal(f2, 0.0)) {
		/* Ci, Di, Ei, Fi Stress recovery coefficients. (Real; Default = 0.0)
		 * The stress recovery coefficients C1 and C2, etc., are the y and z coordinates
		 * in the bar element coordinate system of a point at which stresses are
		 * computed. Stresses are computed at both ends of the bar.
		 */
		// Output and stress assertions, currently ignored
		string message = "PBAR Stress coefficients are dismissed and taken as 0.0";
		handleParsingWarning(message, tok, model);
	}

	// K1, K2: Area factors for shear. VEGA works with 1/K1 and 1/K2
	// Default values is infinite
	const double k1 = tok.nextDouble(true); // K1 = Kzz
	const double k2 = tok.nextDouble(true); // K2 = Kyy
	double invk1, invk2;
	if (is_equal(k1, NastranTokenizer::UNAVAILABLE_DOUBLE)){
		invk1= 0.0;
	}else{
		invk1 = is_zero(k1) ? NastranTokenizer::UNAVAILABLE_DOUBLE : 1.0/k1;
	}
	if (is_equal(k2, NastranTokenizer::UNAVAILABLE_DOUBLE)){
		invk2= 0.0;
	}else{
		invk2 = is_zero(k2) ? NastranTokenizer::UNAVAILABLE_DOUBLE : 1.0/k2;
	}

	double i12 = tok.nextDouble(true, 0.0);
	if (!is_equal(i12, 0)) {
		string message = "PBAR i12 not implemented.";
		handleParsingError(message, tok, model);
	}

	GenericSectionBeam genericSectionBeam(*model, area, i2, i1, j, invk2, invk1, Beam::EULER, nsm,
			elemId);
	genericSectionBeam.assignMaterial(material_id);
	genericSectionBeam.assignCellGroup(getOrCreateCellGroup(elemId, model, "PBAR"));
	model->add(genericSectionBeam);
}

void NastranParserImpl::parsePBARL(NastranTokenizer& tok, shared_ptr<Model> model) {
	int propertyId = tok.nextInt(); // PID
	int material_id = tok.nextInt();

	string group = tok.nextString(true, "MSCBML0");
	if (group!="MSCBML0"){
	    string message = "PBARL users defined groups are not supported.";
	    handleParsingWarning(message, tok, model);
	}

	string type = tok.nextString();
	double nsm;
	if (type == "BAR") {
		tok.skip(4);
		double width = tok.nextDouble();
		double height = tok.nextDouble();
		nsm = tok.nextDouble(true, 0.0);
		RectangularSectionBeam rectangularSectionBeam(*model, width, height, Beam::EULER, nsm,
				propertyId);
		rectangularSectionBeam.assignMaterial(material_id);
		rectangularSectionBeam.assignCellGroup(getOrCreateCellGroup(propertyId, model, "PBARL"));
		model->add(rectangularSectionBeam);
	} else if (type == "ROD") {
		tok.skip(4);
		double radius = tok.nextDouble();
		nsm = tok.nextDouble(true, 0.0);
		CircularSectionBeam circularSectionBeam(*model, radius, Beam::EULER, nsm, propertyId);
		circularSectionBeam.assignMaterial(material_id);
		circularSectionBeam.assignCellGroup(getOrCreateCellGroup(propertyId, model, "PBARL"));
		model->add(circularSectionBeam);
	} else {
		string message = "PBARL type " + type + " not implemented.";
		handleParsingError(message, tok, model);
	}

}

void NastranParserImpl::parsePBEAM(NastranTokenizer& tok, shared_ptr<Model> model) {
	int elemId = tok.nextInt();
	int material_id = tok.nextInt();
	int numBeamParts = 0;
	while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
		if (numBeamParts > 0) {
			throw ParsingException("PBEAM sections not supported", tok.fileName, tok.lineNumber);
			tok.skip(2);
		}
		double area_cross_section = tok.nextDouble(true, 0.0);
		double moment_of_inertia_Z = tok.nextDouble(true, 0.0);
		double moment_of_inertia_Y = tok.nextDouble(true, 0.0);
		double areaProductOfInertia = tok.nextDouble(true, 0.0);
		if (!is_equal(areaProductOfInertia, 0.0)) {
			throw ParsingException("Area product of inertia not implemented", tok.fileName,
					tok.lineNumber);
		}
		double torsionalConstant = tok.nextDouble(true, 0.0);
		double nsm = tok.nextDouble(true, 0.0);
		if (!is_equal(nsm, 0.0)) {
			throw ParsingException("PBEAM: NSM not implemented", tok.fileName, tok.lineNumber);
		}
		double c1 = tok.nextDouble(true, 0.0);
		double c2 = tok.nextDouble(true, 0.0);
		double d1 = tok.nextDouble(true, 0.0);
		double d2 = tok.nextDouble(true, 0.0);
		double e1 = tok.nextDouble(true, 0.0);
		double e2 = tok.nextDouble(true, 0.0);
		double f1 = tok.nextDouble(true, 0.0);
		double f2 = tok.nextDouble(true, 0.0);
		if (!is_equal(c1, 0) || !is_equal(c2, 0) || !is_equal(d1, 0) || !is_equal(d2, 0)
				|| !is_equal(e1, 0) || !is_equal(e2, 0) || !is_equal(f1, 0) || !is_equal(f2, 0)) {
			/*
			 * Ci(A), Di(A) Ei(A), Fi(A)
			 * The y and z locations (i = 1 corresponds to y
			 * and i = 2 corresponds to z) in element
			 * coordinates relative to the shear center (see the
			 * diagram following the remarks) at end A for
			 * stress data recovery. (Real)
			 * y = z = 0.0
			 * Ci, Di, Ei, Fi
			 * The y and z locations (i = 1 corresponds to y
			 * and i = 2 corresponds to z) in element
			 * coordinates relative to the shear center (see
			 * Figure 8-134 in Remark 10.) for the cross section
			 * located at X/XB. The values are fiber locations
			 * for stress data recovery. Ignored for beam p-
			 * elements. (Real)
			 */
			handleParsingWarning("PBEAM:shear center for stress analysis not implemented", tok,
					model);
		}
		GenericSectionBeam genericSectionBeam(*model, area_cross_section, moment_of_inertia_Y,
				moment_of_inertia_Z, torsionalConstant, 0.0, 0.0, GenericSectionBeam::EULER, nsm,
				elemId);
		genericSectionBeam.assignMaterial(material_id);
		genericSectionBeam.assignCellGroup(getOrCreateCellGroup(elemId, model, "PBEAM"));
		model->add(genericSectionBeam);
		numBeamParts++;
	}

}

void NastranParserImpl::parsePBEAML(NastranTokenizer& tok, shared_ptr<Model> model) {
	int pid = tok.nextInt();
	int mid = tok.nextInt();
	string group = tok.nextString(true, "MSCBML0");
	string type = tok.nextString();
	double nsm;
	if (type == "BAR") {
		tok.skip(4);
		double width = tok.nextDouble();
		double height = tok.nextDouble();
		nsm = tok.nextDouble(true, 0.0);
		RectangularSectionBeam rectangularSectionBeam(*model, width, height, Beam::EULER, nsm, pid);
		rectangularSectionBeam.assignMaterial(mid);
		rectangularSectionBeam.assignCellGroup(getOrCreateCellGroup(pid, model,"PBEAML"));
		model->add(rectangularSectionBeam);
	} else if (type == "ROD") {
		tok.skip(4);
		double radius = tok.nextDouble();
		nsm = tok.nextDouble(true, 0.0);
		CircularSectionBeam circularSectionBeam(*model, radius, Beam::EULER, nsm, pid);
		circularSectionBeam.assignMaterial(mid);
		circularSectionBeam.assignCellGroup(getOrCreateCellGroup(pid, model,"PBEAML"));
		model->add(circularSectionBeam);
	} else if (type == "I") {
		tok.skip(4);
		double beam_height = tok.nextDouble();
		double lower_flange_width = tok.nextDouble();
		double upper_flange_width = tok.nextDouble();
		double web_thickness = tok.nextDouble();
		double lower_flange_thickness = tok.nextDouble();
		double upper_flange_thickness = tok.nextDouble();
		nsm = tok.nextDouble(true, 0.0);
		string so = tok.nextString(true, "YES");
		ISectionBeam iSectionBeam(*model, upper_flange_width, lower_flange_width,
				upper_flange_thickness, lower_flange_thickness, beam_height, web_thickness,
				Beam::EULER, nsm, pid);
		iSectionBeam.assignMaterial(mid);
		iSectionBeam.assignCellGroup(getOrCreateCellGroup(pid, model,"PBEAML"));
		model->add(iSectionBeam);
	} else {
		string message = "PBEAML type " + type + " not implemented.";
		handleParsingError(message, tok, model);
	}

}



/** Parse the NASTRAN PBUSH Keyword: Generalized Spring-And-Damper Property **/
void NastranParserImpl::parsePBUSH(NastranTokenizer& tok, shared_ptr<Model> model) {

	int pid = tok.nextInt();
	double k1=0.0;
	double k2=0.0;
	double k3=0.0;
	double k4=0.0;
	double k5=0.0;
	double k6=0.0;
	double b1=0.0;
	double b2=0.0;
	double b3=0.0;
	double b4=0.0;
	double b5=0.0;
	double b6=0.0;
	double ge1=0.0;
	double ge2=0.0;
	double ge3=0.0;
	double ge4=0.0;
	double ge5=0.0;
	double ge6=0.0;
	double sa=1.0;
	double st=1.0;
	double ea=1.0;
	double et=1.0;

    // Parsing the keyword: done this way to avoid warning message when the user
	// speficied null B or null GE.
	while (!(tok.isEmptyUntilNextKeyword())){
		tok.skipToNotEmpty();
		string flag = tok.nextString();
		if (flag=="K"){ // Stiffness values (Default 0.0)
			k1=tok.nextDouble(true, 0.0);
			k2=tok.nextDouble(true, 0.0);
			k3=tok.nextDouble(true, 0.0);
			k4=tok.nextDouble(true, 0.0);
			k5=tok.nextDouble(true, 0.0);
			k6=tok.nextDouble(true, 0.0);
		}else if (flag=="B"){ // Force-Per-velocity Damping (Default 0.0)
			b1=tok.nextDouble(true, 0.0);
			b2=tok.nextDouble(true, 0.0);
			b3=tok.nextDouble(true, 0.0);
			b4=tok.nextDouble(true, 0.0);
			b5=tok.nextDouble(true, 0.0);
			b6=tok.nextDouble(true, 0.0);
		}else if (flag=="GE"){ // Structural Damping constants (Default 0.0)
			ge1=tok.nextDouble(true, 0.0);
			ge2=tok.nextDouble(true, 0.0);
			ge3=tok.nextDouble(true, 0.0);
			ge4=tok.nextDouble(true, 0.0);
			ge5=tok.nextDouble(true, 0.0);
			ge6=tok.nextDouble(true, 0.0);
		}else if(flag=="RCV"){ // Stress and Strain recovery coefficient (Default 1.0)
			sa=tok.nextDouble(true, 1.0);
			st=tok.nextDouble(true, 1.0);
			ea=tok.nextDouble(true, 1.0);
			et=tok.nextDouble(true, 1.0);
		}else{
			handleParsingWarning(string("PBUSH: unknown flag: ")+string(flag), tok, model);
		}
	}
	// Ony K is supported yet
	if (!is_equal(b1, 0) || !is_equal(b2, 0) || !is_equal(b3, 0) || !is_equal(b4, 0)
			|| !is_equal(b5, 0) || !is_equal(b6, 0) ) {
		b1=0.0; b2=0.0; b3=0.0; b4=0.0; b5=0.0; b6=0.0;
		handleParsingWarning(string("PBUSH: Force-Per-velocity Damping B not supported. Default (0.0) assumed."), tok, model);
	}
	if (!is_equal(ge1, 0) || !is_equal(ge2, 0) || !is_equal(ge3, 0) || !is_equal(ge4, 0)
			|| !is_equal(ge5, 0) || !is_equal(ge6, 0) ) {
		ge1=0.0; ge2=0.0; ge3=0.0; ge4=0.0; ge5=0.0; ge6=0.0;
		handleParsingWarning(string("PBUSH: Structural Damping constants GE not supported. Default (0.0) assumed."), tok, model);
	}
	if (!is_equal(sa, 1.0) || !is_equal(st, 1.0) || !is_equal(ea, 1.0) || !is_equal(et,1.0)) {
		sa=1.0; st=1.0; ea=1.0; et=1.0;
		handleParsingWarning(string("PBUSH: Stress and Strain recovery coefficients (SA, ST, EA, ET ) not supported. Default (1.0) assumed."), tok, model);
	}

	StructuralSegment structuralElement(*model, true, pid);
	structuralElement.assignCellGroup(getOrCreateCellGroup(pid, model, "PBUSH"));
	structuralElement.addStiffness(DOF::DX, DOF::DX, k1);
	structuralElement.addStiffness(DOF::DY, DOF::DY, k2);
	structuralElement.addStiffness(DOF::DZ, DOF::DZ, k3);
	structuralElement.addStiffness(DOF::RX, DOF::RX, k4);
	structuralElement.addStiffness(DOF::RY, DOF::RY, k5);
	structuralElement.addStiffness(DOF::RZ, DOF::RZ, k6);

	model->add(structuralElement);

}


void NastranParserImpl::parsePGAP(NastranTokenizer& tok, shared_ptr<Model> model) {
	int pid = tok.nextInt();
	double u0 = tok.nextDouble();
	double f0 = tok.nextDouble(true, 0.0);
	if (!is_equal(f0, 0)) {
		handleParsingError(string("unsupported f0 ") + to_string(f0) + string(" in PGAP. "), tok,
				model);
	}
	double ka = tok.nextDouble();
	handleParsingWarning(string("ignored ka ") + to_string(ka) + string(" in PGAP. "), tok, model);
	shared_ptr<Constraint> gapPtr = model->find(Reference<Constraint>(Constraint::GAP, pid));
	if (!gapPtr) {
		GapTwoNodes gapConstraint(*model, pid);
		gapConstraint.initial_gap_opening = u0;
		model->add(gapConstraint);
		model->addConstraintIntoConstraintSet(gapConstraint, model->commonConstraintSet);
	} else {
		shared_ptr<GapTwoNodes> gap = static_pointer_cast<GapTwoNodes>(gapPtr);
		gap->initial_gap_opening = u0;
	}
}

void NastranParserImpl::parsePLOAD4(NastranTokenizer& tok, shared_ptr<Model> model) {
	int loadset_id = tok.nextInt();
	int eid1 = tok.nextInt();
	double p1 = tok.nextDouble();
	double p2 = tok.nextDouble(true, p1);
	double p3 = tok.nextDouble(true, p1);
	double p4 = tok.nextDouble(true, p1);
	if (!is_equal(p2, p1) || !is_equal(p3, p1) || !is_equal(p4, p1)) {
		throw ParsingException("Non uniform pressure not implemented", tok.fileName,
				tok.lineNumber);
	}
	bool format1 = tok.isNextInt() || tok.isNextEmpty();
	int g1 = NastranTokenizer::UNAVAILABLE_INT;
	int g3_or_4 = NastranTokenizer::UNAVAILABLE_INT;
	int eid2 = NastranTokenizer::UNAVAILABLE_INT;
	if (format1) {
		//format 1
		g1 = tok.nextInt(true);
		g3_or_4 = tok.nextInt(true);
	} else if (tok.nextString() == "THRU") {
		//format2
		eid2 = tok.nextInt();
	} else {
		//format not recognized
		throw ParsingException(string("PLOAD4 format not recognized") + tok.currentRawDataLine(),
				tok.fileName, tok.lineNumber);
	}
	int cid = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	if (cid != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
		string message = "PLOAD4 : CoordinateSystem not supported" + tok.lineNumber;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	double n1 = tok.nextDouble(true, 0.0);
	double n2 = tok.nextDouble(true, 0.0);
	double n3 = tok.nextDouble(true, 0.0);
	Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadset_id);
	if (is_equal(n1, 0.0) && is_equal(n2, 0.0) && is_equal(n3, 0.0) && cid == 0
			&& g1 == NastranTokenizer::UNAVAILABLE_INT) {
		NormalPressionFace normalPressionFace(*model, p1);
		addCellIds(normalPressionFace, eid1, eid2);

		model->add(normalPressionFace);
		model->addLoadingIntoLoadSet(normalPressionFace, loadSetReference);
	} else if (g1 != NastranTokenizer::UNAVAILABLE_INT) {
		PressionFaceTwoNodes pressionFaceTwoNodes(*model, g1, g3_or_4,
				VectorialValue(n1 * p1, n2 * p1, n3 * p1), VectorialValue(0.0, 0.0, 0.0));
		addCellIds(pressionFaceTwoNodes, eid1, eid2);

		model->add(pressionFaceTwoNodes);
		model->addLoadingIntoLoadSet(pressionFaceTwoNodes, loadSetReference);
	} else {
		ForceSurface forceSurface(*model, VectorialValue(n1 * p1, n2 * p1, n3 * p1),
				VectorialValue(0.0, 0.0, 0.0));
		addCellIds(forceSurface, eid1, eid2);

		model->add(forceSurface);
		model->addLoadingIntoLoadSet(forceSurface, loadSetReference);
	}
	if (!model->find(loadSetReference)) {
		LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
		model->add(loadSet);
	}
}

void NastranParserImpl::addCellIds(ElementLoading& loading, int eid1, int eid2) {
	if (eid2 == NastranTokenizer::UNAVAILABLE_INT) {
		loading.addCell(eid1);
	} else {
		for (int i = eid1; i < eid2; i++) {
			loading.addCell(i);
		}
	}
}

void NastranParserImpl::parsePROD(NastranTokenizer& tok, shared_ptr<Model> model) {
	int propId = tok.nextInt();
	int material_id = tok.nextInt();
	double a = tok.nextDouble();
	double j = tok.nextDouble(true, 0.0);
	double c = tok.nextDouble(true, 0.0);
	double nsm = tok.nextDouble(true, 0.0);
	if (!is_equal(c, 0)) {
		throw ParsingException("PROD stress coefficient C not supported", tok.fileName,
				tok.lineNumber);
	}
	if (!is_equal(nsm, 0)) {
		throw ParsingException("PROD Non Structural mass not supported", tok.fileName,
				tok.lineNumber);
	}
	GenericSectionBeam genericSectionBeam(*model, a, 0, 0, j, 0, 0, GenericSectionBeam::EULER, nsm,
			propId);
	genericSectionBeam.assignMaterial(material_id);
	genericSectionBeam.assignCellGroup(getOrCreateCellGroup(propId, model, "PROD"));
	model->add(genericSectionBeam);
}

void NastranParserImpl::parsePSHELL(NastranTokenizer& tok, shared_ptr<Model> model) {
	int propId = tok.nextInt();
	int material_id1 = tok.nextInt();
	double thickness = tok.nextDouble(true, 0.0);
	int material_id2 = tok.nextInt(true);
	if (material_id2 != NastranTokenizer::UNAVAILABLE_INT && material_id2 != material_id1) {
		throw ParsingException("Material 2 not yet supported", tok.fileName, tok.lineNumber);
	}
	double bending_moment = tok.nextDouble(true, 1.0);
	if (!is_equal(bending_moment, 1.0)) {
		handleParsingError("PSHELL: Bending moment of inertia ratio != 1.0 not supported", tok,
				model);
	}
	int material_id3 = tok.nextInt(true);
	if (material_id3 != NastranTokenizer::UNAVAILABLE_INT && material_id3 != material_id1) {
		throw ParsingException("Material 3 not yet supported", tok.fileName, tok.lineNumber);
	}
	double ts_t_ratio = tok.nextDouble(true, 0.833333);
	if (!is_equal(ts_t_ratio, 0.833333)) {
		throw ParsingException("ts/t ratio !=  0.833333 not supported", tok.fileName,
				tok.lineNumber);
	}
	double nsm = tok.nextDouble(true, 0);
	double z1 = tok.nextDouble(true);
	double z2 = tok.nextDouble(true);
	if (!is_equal(z1, NastranTokenizer::UNAVAILABLE_DOUBLE)
			|| !is_equal(z2, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		throw ParsingException("Fiber distances z1,z2 not supported", tok.fileName, tok.lineNumber);
	}
	int material_id4 = tok.nextInt(true);
	if (material_id4 != NastranTokenizer::UNAVAILABLE_INT && material_id4 != material_id1) {
		throw ParsingException("Material 4 not yet supported", tok.fileName, tok.lineNumber);
	}

	Shell shell(*model, thickness, nsm, propId);
	shell.assignMaterial(material_id1);
	shell.assignCellGroup(getOrCreateCellGroup(propId, model,"PSHELL"));
	model->add(shell);
}

void NastranParserImpl::parsePSOLID(NastranTokenizer& tok, shared_ptr<Model> model) {
	int elemId = tok.nextInt();
	int material_id = tok.nextInt();
	int material_coordinate_system = tok.nextInt(true, 0);
	if (material_coordinate_system != 0) {
		handleParsingError("PSOLID: Material coordinate system!=0 not supported", tok, model);
	}
	string psolid_in = tok.nextString(true, "BUBBLE");
	/*
	 * Location selection for stress output.
	 * Stress output may be requested at the Gauss points (STRESS = “GAUSS” or
	 * 1) of CHEXA and CPENTA elements with no midside nodes. Gauss point
	 * output is available for the CTETRA element with or without midside nodes.
	 *
	 */
	string stress = tok.nextString(true);
	if (!stress.empty() and stress != "GRID") {
		handleParsingError("PSOLID: STRESS field: " + stress + " not supported", tok, model);
	}
	string isop = tok.nextString(true, "REDUCED");
	const ModelType * modelType;
	if (psolid_in == "BUBBLE" && isop == "REDUCED") {
		modelType = &ModelType::TRIDIMENSIONAL_SI;
	} else if ((psolid_in == "TWO" || psolid_in == "2") && (isop == "FULL" || isop == "1")) {
		modelType = &ModelType::TRIDIMENSIONAL;
	} else {
		throw ParsingException("PSOLID IN " + psolid_in + " ISOP " + isop + " Not implemented",
				tok.fileName, tok.lineNumber);
	}
	string fctn = tok.nextString(true, "SMECH");
	if (fctn != "SMECH") {
		throw ParsingException("PSOLID fctn " + fctn + " Not implemented", tok.fileName,
				tok.lineNumber);
	}
	Continuum continuum(*model, modelType, elemId);
	continuum.assignMaterial(material_id);
	continuum.assignCellGroup(getOrCreateCellGroup(elemId, model, "PSOLID"));
	model->add(continuum);
}

void NastranParserImpl::parseRBE2(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	int masterId = tok.nextInt();
	int dofs = tok.nextInt();
	if (dofs == 123456) {
		RigidConstraint qrc(*model, masterId, original_id);
		while (tok.isNextInt()) {
			qrc.addSlave(tok.nextInt());
		}
		model->add(qrc);
		model->addConstraintIntoConstraintSet(qrc, model->commonConstraintSet);

	} else {
		throw ParsingException("QuasiRigid constraint not yet implemented", tok.fileName,
				tok.lineNumber);
	}
	double alpha = tok.nextDouble(true);
	if (!is_equal(alpha, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingError("RBE2: ALPHA field: " + lexical_cast<string>(alpha) + " not supported",
				tok, model);
	}
}

void NastranParserImpl::parseRBE3(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	tok.skip(1); // ignoring blank
	int masterId = tok.nextInt();
	int nastranDofs = tok.nextInt();
	DOFS dofs = DOFS::nastranCodeToDOFS(nastranDofs);
	RBE3 rbe3(*model, masterId, dofs, original_id);
	while (tok.isNextDouble()) {
		double coef = tok.nextDouble();
		int nastranSDofs = tok.nextInt();
		DOFS sdofs = DOFS::nastranCodeToDOFS(nastranSDofs);
		while (tok.isNextInt()) {
			int slaveId = tok.nextInt();
			rbe3.addSlave(slaveId, sdofs, coef);
		}
	}
	model->add(rbe3);
	model->addConstraintIntoConstraintSet(rbe3, model->commonConstraintSet);
}

void NastranParserImpl::parseRBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id=tok.nextInt();
	int ga = tok.nextInt();
	int gb = tok.nextInt();
	int cna = tok.nextInt(true, 0);
	int cnb = tok.nextInt(true, 0);
	int cma = tok.nextInt(true, 0);
	int cmb = tok.nextInt(true, 0);
	if (cna != 0 && cnb != 0) {
		throw ParsingException("RBAR cna & cnb both specified.", tok.fileName, tok.lineNumber);
	} else if (cna == 0 && cnb == 0) {
		cna = 123456;
	}
	if (cna != 0) {
		QuasiRigidConstraint qrc = QuasiRigidConstraint(*model, DOFS::nastranCodeToDOFS(cna), HomogeneousConstraint::UNAVAILABLE_MASTER, original_id);
		qrc.addSlave(ga);
		qrc.addSlave(gb);
		model->add(qrc);
		model->addConstraintIntoConstraintSet(qrc, model->commonConstraintSet);
	} else if (cnb != 0) {
		QuasiRigidConstraint qrc = QuasiRigidConstraint(*model, DOFS::nastranCodeToDOFS(cnb), HomogeneousConstraint::UNAVAILABLE_MASTER, original_id);
		qrc.addSlave(ga);
		qrc.addSlave(gb);
		model->add(qrc);
		model->addConstraintIntoConstraintSet(qrc, model->commonConstraintSet);
	}
	if (cma != 0 || cmb != 0) {
		throw ParsingException("RBAR cma or cmb not supported.", tok.fileName, tok.lineNumber);
	}
	double alpha = tok.nextDouble(true);
	if (!is_equal(alpha, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingError("RBAR: ALPHA field: " + lexical_cast<string>(alpha) + " not supported",
				tok, model);
	}

}

void NastranParserImpl::parseRBAR1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id=tok.nextInt();
	int ga = tok.nextInt();
	int gb = tok.nextInt();
	int cna = tok.nextInt(true, 0);

	QuasiRigidConstraint qrc = QuasiRigidConstraint(*model, DOFS::nastranCodeToDOFS(cna), HomogeneousConstraint::UNAVAILABLE_MASTER, original_id);
	qrc.addSlave(ga);
	qrc.addSlave(gb);
	model->add(qrc);
	model->addConstraintIntoConstraintSet(qrc, model->commonConstraintSet);

	double alpha = tok.nextDouble(true);
	if (!is_equal(alpha, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
		handleParsingError("RBAR1: ALPHA field: " + lexical_cast<string>(alpha) + " not supported",
				tok, model);
	}

}

void NastranParserImpl::parseRFORCE(NastranTokenizer& tok, shared_ptr<Model> model) {
	// RFORCE  2       1               200.    0.0     0.0     1.0     2
	int sid = tok.nextInt();
	int g = tok.nextInt();
	int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
	if (coordinate_system_id != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
		string message = "RFORCE : CoordinateSystem not supported" + tok.lineNumber;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
	double a = tok.nextDouble();
	double r1 = tok.nextDouble();
	double r2 = tok.nextDouble();
	double r3 = tok.nextDouble();
	/*if (!tok.isNextEmpty()) {
	 int method = tok.nextInt(true, 1);
	 if (method != 1) {
	 string message = "RFORCE METHOD not supported line"
	 + tok.lineNumber;
	 throw ParsingException(message, tok.fileName, tok.lineNumber);
	 }
	 }
	 if (!tok.isNextEmpty()) {
	 double racc = tok.nextDouble(true, 0);
	 if (racc != 0) {
	 string message = "RFORCE RACC not supported line" + tok.lineNumber;
	 throw ParsingException(message, tok.fileName, tok.lineNumber);
	 }
	 }
	 if (!tok.isNextEmpty()) {
	 int mb = tok.nextInt(true, 0);
	 if (mb != 0) {
	 string message = "RFORCE MB not supported line" + tok.lineNumber;
	 throw ParsingException(message, tok.fileName, tok.lineNumber);
	 }
	 }*/
	tok.skip(255);
	RotationNode rotation(*model, a, g, r1, r2, r3);

	model->add(rotation);
	Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, sid);
	model->addLoadingIntoLoadSet(rotation, loadset_ref);
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::LOAD, sid);
		model->add(loadSet);
	}

}

void NastranParserImpl::parseSPC(NastranTokenizer& tok, shared_ptr<Model> model) {
	int spcSet_id = tok.nextInt();
	string name = string("SPC") + "_" + to_string(spcSet_id);
	NodeGroup *spcNodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SPC");
	try {
		while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
			const int nodeId = tok.nextInt(true);
			if (nodeId == NastranTokenizer::UNAVAILABLE_INT) {
				//space at the end of line found,consume it.
				continue;
			}
			const int gi = tok.nextInt(true, 123456);
			const double displacement = tok.nextDouble(true, 0.0);
			//if (displacement != 0.0) {
			//	handleParsingError(string("Displacement ") + boost::lexical_cast<string>(displacement) + "(!= 0) not supported", tok, model);
			//}
			SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(gi),
					displacement);
			spc.addNodeId(nodeId);
			spcNodeGroup->addNode(nodeId);

			model->add(spc);
			model->addConstraintIntoConstraintSet(spc,
					Reference<ConstraintSet>(ConstraintSet::SPC, spcSet_id));
		}
	} catch (ParsingException &e) {
		handleParseException(e, model, "Problem parsing SPC");
	}
}

void NastranParserImpl::parseSPC1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int set_id = tok.nextInt();
	const int dofInt = tok.nextInt();
	const int g1 = tok.nextInt();

	// We create a constraint
	SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(dofInt), 0.0);

    // Nodes are added to the constraint Node Group
	string name = string("SPC1") + "_" + to_string(set_id);
	NodeGroup *spcNodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SPC1");

	// Parsing Nodes
	string pos2 = trim_copy(tok.nextString(true));
	if (pos2 == "THRU") {
		//parse "through" format
		const int g2 = tok.nextInt();
		for (int curNode = g1; curNode <= g2; curNode++) {
			spcNodeGroup->addNode(curNode);
			spc.addNodeId(curNode);
		}
	} else {
		spcNodeGroup->addNode(g1);
		spc.addNodeId(g1);
		if (!pos2.empty()) {
		    int nodeG2 = lexical_cast<int>(pos2);
			spcNodeGroup->addNode(nodeG2);
			spc.addNodeId(nodeG2);
			while (tok.isNextInt()) {
				int nodeId = tok.nextInt();
				spcNodeGroup->addNode(nodeId);
				spc.addNodeId(nodeId);
			}
		}
	}

	// Adding the constraint to the model
    model->add(spc);
	model->addConstraintIntoConstraintSet(spc,
			Reference<ConstraintSet>(ConstraintSet::SPC, set_id));

}

void NastranParserImpl::parseSPCD(NastranTokenizer& tok, shared_ptr<Model> model) {
	int set_id = tok.nextInt();
	const int g1 = tok.nextInt();
	const int c1 = tok.nextInt();
	const double d1 = tok.nextDouble();
	int g1pos = model->mesh->findNodePosition(g1);
	int g2 = -1;
	int c2;
	double d2;
	int g2pos = -1;
	if (tok.isNextInt()) {
		g2 = tok.nextInt();
		c2 = tok.nextInt();
		d2 = tok.nextDouble();
		g2pos = model->mesh->findNodePosition(g2);
	}
	Reference<ConstraintSet> constraintSetReference(ConstraintSet::SPCD, set_id);
	if (!model->find(constraintSetReference)) {
		ConstraintSet constraintSet(*model, ConstraintSet::SPCD, set_id);
		model->add(constraintSet);
	}
	for (shared_ptr<Analysis> analysis : model->analyses) {
		if (analysis->contains(constraintSetReference)
				|| analysis->type != Analysis::LINEAR_MECA_STAT) {
			continue;
		}
		for (shared_ptr<LoadSet> loadSetPtr : analysis->getLoadSets()) {
			if (loadSetPtr->type == LoadSet::LOAD && loadSetPtr->getOriginalId() == set_id) {
				// In the static solution sequences, the set ID of the SPCD entry (SID) is selected
				//  by the LOAD Case Control command.
				analysis->add(constraintSetReference);
			}
		}
	}

	for (auto analysis : model->analyses) {
		if (!analysis->contains(constraintSetReference)) {
			continue;
		}

		// Values of Di will override the values specified on an SPC Bulk Data entry, if
		// the SID is selected as indicated above.
		for (const shared_ptr<ConstraintSet>& constraintSet : analysis->getConstraintSets()) {
			if (constraintSet->type != ConstraintSet::SPC) {
				continue;
			}
			const set<shared_ptr<Constraint> > spcs = constraintSet->getConstraintsByType(
					Constraint::SPC);
			if (spcs.size() == 0) {
				continue;
			}
			for (shared_ptr<Constraint> constraint : spcs) {
				shared_ptr<SinglePointConstraint> spc = static_pointer_cast<SinglePointConstraint>(
						constraint);
				for (int nodePosition : spc->nodePositions()) {
					int g;
					int c;
					if (nodePosition == g1pos) {
						g = g1;
						c = c1;
					} else if (nodePosition == g2pos) {
						g = g2;
						c = c2;
					} else {
						continue;
					}
					DOFS spcDofs = spc->getDOFSForNode(nodePosition);
					DOFS blockingDofs = DOFS::nastranCodeToDOFS(c);
					if (!spcDofs.containsAnyOf(blockingDofs)) {
						continue;
					}
					analysis->removeSPCNodeDofs(*spc, nodePosition, blockingDofs);
					if (model->configuration.logLevel >= LogLevel::DEBUG) {
						cout << "In analysis : " + to_str(*analysis) + ", SPCD "
								<< to_string(set_id) <<
								" replaced DOFS " << blockingDofs << " for node id : " << g
								<< " from spc : " << *spc << endl;
					}
				}
			}
		}
	}

	SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(c1), d1);
	spc.addNodeId(g1);
	model->add(spc);
	model->addConstraintIntoConstraintSet(spc, constraintSetReference);
	if (g2 != -1) {
		SinglePointConstraint spc2 = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(c2), d2);
		spc.addNodeId(g2);
		model->add(spc2);
		model->addConstraintIntoConstraintSet(spc2, constraintSetReference);
	}
}

void NastranParserImpl::parseMPC(NastranTokenizer& tok, shared_ptr<Model> model) {
	int set_id = tok.nextInt();
	LinearMultiplePointConstraint lmpc(*model);
	int i = 2;
	while (tok.isNextInt()) {
		const int g1 = tok.nextInt();
		const int c1 = tok.nextInt();
		const double a1 = tok.nextDouble();
		DOFS dofs = DOFS::nastranCodeToDOFS(c1);
		i += 3;
		if (i % 8 == 0 && !tok.isEmptyUntilNextKeyword()) {
			tok.skip(2);
			i += 2;
		}
		lmpc.addParticipation(g1, dofs.contains(DOF::DX) * a1, dofs.contains(DOF::DY) * a1,
				dofs.contains(DOF::DZ) * a1, dofs.contains(DOF::RX) * a1,
				dofs.contains(DOF::RY) * a1, dofs.contains(DOF::RZ) * a1);
	}
	model->add(lmpc);
	model->addConstraintIntoConstraintSet(lmpc,
			Reference<ConstraintSet>(ConstraintSet::MPC, set_id));
}

// new keywords for modal and dyna harm:
void NastranParserImpl::parseDAREA(NastranTokenizer& tok, shared_ptr<Model> model) {
	int loadset_id = tok.nextInt();
	Reference<vega::LoadSet> loadset_ref(LoadSet::EXCITEID, loadset_id);
	while (tok.isNextInt()) {
		int node_id = tok.nextInt();
		int ci = tok.nextInt(true, 123456);
		double ai = tok.nextDouble();

		DOFS dofs = DOFS::nastranCodeToDOFS(ci);
		double tx = dofs.contains(DOF::DX) ? ai : 0;
		double ty = dofs.contains(DOF::DY) ? ai : 0;
		double tz = dofs.contains(DOF::DZ) ? ai : 0;
		double rx = dofs.contains(DOF::RX) ? ai : 0;
		double ry = dofs.contains(DOF::RY) ? ai : 0;
		double rz = dofs.contains(DOF::RZ) ? ai : 0;

		NodalForce force1(*model, node_id, tx, ty, tz, rx, ry, rz, Loading::NO_ORIGINAL_ID);
		model->add(force1);
		model->addLoadingIntoLoadSet(force1, loadset_ref);
	}
	if (!model->find(loadset_ref)) {
		LoadSet loadSet(*model, LoadSet::EXCITEID, loadset_id);
		model->add(loadSet);
	}
}

void NastranParserImpl::parseEIGR(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	string method = tok.nextString(true); //TODO : add something to frequecyBand with method. Default can be Lanczos if EIGRL
	double lower = tok.nextDouble(true);
	double upper = tok.nextDouble(true);
	int num_max = tok.nextInt(true);

	FrequencyBand frequencyBand(*model, lower, upper, num_max, original_id);
	model->add(frequencyBand);
}

void NastranParserImpl::parseEIGRL(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	double lower = tok.nextDouble(true);
	double upper = tok.nextDouble(true);
	int num_max = tok.nextInt(true);
	tok.skip(4); // ignoring MSGLVL, MAXSET, SHFSCL and NORM

	FrequencyBand frequencyBand(*model, lower, upper, num_max, original_id);

	model->add(frequencyBand);
}

void NastranParserImpl::parseFREQ1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	double start = tok.nextDouble();
	double step = tok.nextDouble();
	int count = tok.nextInt(true, 1);

	vega::StepRange stepRange(*model, start, step, count);
	stepRange.setParaX(Value::FREQ);
	FrequencyValues frequencyValues(*model, stepRange, original_id);

	model->add(stepRange);
	model->add(frequencyValues);
}

void NastranParserImpl::parseDLOAD(NastranTokenizer& tok, shared_ptr<Model> model) {
	int loadset_id = tok.nextInt();
	Reference<LoadSet> loadSetReference(LoadSet::DLOAD, loadset_id);

	double S = tok.nextDouble(true, 1);
	if (!is_equal(S, 1))
		handleParsingError("S != 1 in DLOAD not supported. ", tok, model);

	//vega::FunctionTable functionTable(*model, FunctionTable::LINEAR);
	while (tok.isNextDouble()) {
		double scale = tok.nextDouble(true, 1);
		if (!is_equal(scale, 1))
			handleParsingError(string("scale != 1 in DLOAD not supported. "), tok, model);
		tok.skipToNotEmpty();
		int rload2_id = tok.nextInt();
		tok.skipToNotEmpty();
		model->addLoadingIntoLoadSet(Reference<Loading>(Loading::DYNAMIC_EXCITATION, rload2_id),
				loadSetReference);
	}
	if (!model->find(loadSetReference)) {
		LoadSet loadSet(*model, LoadSet::DLOAD, loadset_id);
		model->add(loadSet);
	}
}

void NastranParserImpl::parseDMIG(NastranTokenizer& tok, shared_ptr<Model> model) {
	string name = tok.nextString();
	if (name == "UACCEL") {
		throw logic_error(
				"DMIG UACCEL Defines rigid body accelerations in the basic coordinate system.");
	}
	if (name == "CDSHUT") {
		return; // currently ignored, see CDPCH
	}
	auto it = directMatrixByName.find(name);
	if (it == directMatrixByName.end()) {
		throw logic_error("Missing declaration : " + name);
	}
	shared_ptr<MatrixElement> matrix = static_pointer_cast<MatrixElement>(
			model->find(*(it->second)));

	int headerIndicator = tok.nextInt();
	if (headerIndicator == 0) {
		//  Field 3 of the header entry must contain an integer 0.
		int ifo = tok.nextInt();
		if (ifo != 6) {
			throw logic_error("Non-symmetric DMIG not yet implemented");
		}
		int tin = tok.nextInt();
		if (tin != 1 && tin != 2) {
			throw logic_error("Non-real or non-single precision DMIG not yet implemented");
		}
		int tout = tok.nextInt(true, 0);
		if (tout != 0) {
			throw logic_error("TOUT in DMIG not yet implemented");
		}
		int polar = tok.nextInt(true, 0);
		if (polar != 0) {
			throw logic_error("POLAR in DMIG not yet implemented");
		}
		return; // Actually ignoring header DMIG
	}

	int gj = headerIndicator;
	int cj = tok.nextInt();
	DOF dofj = *(DOFS::nastranCodeToDOFS(cj).begin());
	tok.skip(1);
	while (tok.isNextInt()) {
		int g1 = tok.nextInt();
		int c1 = tok.nextInt();
		DOF dof1 = *(DOFS::nastranCodeToDOFS(c1).begin());
		double a1 = tok.nextDouble();
		tok.nextDouble(true, 0.0);
		matrix->addComponent(gj, dofj, g1, dof1, a1);
	}
}

void NastranParserImpl::parseRLOAD2(NastranTokenizer& tok, shared_ptr<Model> model) {
	int loadset_id = tok.nextInt();
	int darea_set_id = tok.nextInt();
	double delay = tok.nextDouble(true, 0);
	if (!is_equal(delay, 0))
		handleParsingError("DELAY in RLOAD2 not supported. ", tok, model);
	int dphase_id = 0;
	double dphase = 0.0;
	if (tok.isNextInt())
		dphase_id = tok.nextInt(true, 0);
	else
		dphase = tok.nextDouble(true, 0.0);
	int functionTableB_original_id = tok.nextInt(true, 0);
	int functionTableP_original_id = tok.nextInt(true, 0);
	if (functionTableP_original_id != 0)
		handleParsingError("TP in RLOAD2 not supported. ", tok, model);
	string type = tok.nextString(true, "LOAD").substr(0, 1);
	if (type != "L" && type != "0")
		handleParsingError("TYPE in RLOAD2 not supported. ", tok, model);

	Reference<Value> dynaPhase_ref = Reference<Value>(Value::DYNA_PHASE, dphase_id);
	if (dphase_id == 0) {
		DynaPhase dynaphase(*model, dphase);
		model->add(dynaphase);
		dynaPhase_ref = Reference<Value>(dynaphase);
	}
	Reference<Value> functionTableB_ref(Value::FUNCTION_TABLE, functionTableB_original_id);

	// new LoadSet EXCITEID for the DynamicExcitation
	LoadSet darea(*model, LoadSet::EXCITEID, darea_set_id);
	model->add(darea);
	Reference<LoadSet> darea_ref(darea);
	// if loadSet DLOAD does not exist (was not declared in the bulk), loadset_id become the original id of DynamicExcitation
	// else DynamicExcitation is created without original_id and is mapped to this loadSet
	int original_id;
	Reference<LoadSet> loadSetReference(LoadSet::DLOAD, loadset_id);
	if (model->find(loadSetReference))
		original_id = Loading::NO_ORIGINAL_ID;
	else
		original_id = loadset_id;

	DynamicExcitation dynamicExcitation(*model, dynaPhase_ref, functionTableB_ref, darea_ref,
			original_id);
	model->add(dynamicExcitation);

	model->addLoadingIntoLoadSet(dynamicExcitation, loadSetReference);
	if (!model->find(loadSetReference)) {
		LoadSet loadSet(*model, LoadSet::DLOAD, loadset_id);
		model->add(loadSet);
	}

	// PlaceHolder to complete the Value attribute paraX of FunctionTable
	model->add(dynamicExcitation.getFunctionTableBPlaceHolder());
}

void NastranParserImpl::parseDPHASE(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	tok.skip(2);
	double dphase = tok.nextDouble();
	DynaPhase dynaphase(*model, dphase, original_id);
	model->add(dynaphase);
}

void NastranParserImpl::parseTABDMP1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	string type = tok.nextSymbolString();
	assert(type == "CRIT");
	vega::FunctionTable functionTable(*model, FunctionTable::LINEAR, FunctionTable::LINEAR,
			FunctionTable::NONE, FunctionTable::CONSTANT);
	tok.skipToNotEmpty();

	while (tok.isNextDouble()) {
		double x = tok.nextDouble();
		tok.skipToNotEmpty();
		double y = tok.nextDouble(); //Code_Aster convention : Nastran is coherent with factor 2 for sdamping : not so sure
		tok.skipToNotEmpty();
		functionTable.setXY(x, y);
	}
	assert(tok.nextSymbolString() == "ENDT");
	functionTable.setParaX(Value::FREQ);
	functionTable.setParaY(Value::AMOR);
	vega::ModalDamping modalDamping(*model, functionTable, original_id);

	model->add(functionTable);
	model->add(modalDamping);
}

void NastranParserImpl::parseTABLED1(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	string interpolation = tok.nextString(true, "LINEAR");
	FunctionTable::Interpolation parameter =
			(interpolation == "LINEAR") ? FunctionTable::LINEAR : FunctionTable::LOGARITHMIC;
	interpolation = tok.nextString(true, "LINEAR");
	FunctionTable::Interpolation value =
			(interpolation == "LINEAR") ? FunctionTable::LINEAR : FunctionTable::LOGARITHMIC;

	vega::FunctionTable functionTable(*model, parameter, value, FunctionTable::NONE,
			FunctionTable::NONE, original_id);
	tok.skipToNotEmpty();

	while (tok.isNextDouble()) {
		double x = tok.nextDouble();
		tok.skipToNotEmpty();
		double y = tok.nextDouble();
		tok.skipToNotEmpty();
		functionTable.setXY(x, y);
	}
	assert(tok.nextSymbolString() == "ENDT");

	model->add(functionTable);

}

void NastranParserImpl::parseNLPARM(NastranTokenizer& tok, shared_ptr<Model> model) {
	int original_id = tok.nextInt();
	int number_of_increments = tok.nextInt(true, 10);
	tok.skip(255); // ignore anything else for the moment
	NonLinearStrategy nonLinearStrategy(*model, number_of_increments, original_id);
	model->add(nonLinearStrategy);
}

void NastranParserImpl::parsePARAM(NastranTokenizer& tok, shared_ptr<Model> model) {
	string param = tok.nextString();
	boost::to_upper(param);
	if (param == "AUTOSPC") {
		/*
		 AUTOSPC specifies the action to take when singularities exist in the stiffness
		 matrix. AUTOSPC=YES means that singularities will be constrained
		 automatically. AUTOSPC=NO means that singularities will not be
		 constrained. If AUTOSPC=NO then the user should take extra caution
		 analyzing the results of the grid point singularity table and the computed
		 epsilons. See “Constraint and Mechanism Problem Identification in
		 SubDMAP SEKR” on page 409 of the MSC.Nastran Reference Guide for
		 details of singularity and mechanism identification and constraint.
		 */

		string value = tok.nextString(true, "NO");
		if (value == "YES") {
			handleParsingError(
					string("unsupported parameter ") + param + string(" value in parsePARAM. "),
					tok, model);
		}
	} else if (param == "COUPMASS") {
		/*
		 COUPMASS>0 requests the generation of coupled rather than
		 lumped mass matrices for elements with coupled mass capability, as
		 listed in Table 3-1 in the MSC.Nastran Reference Guide. This option
		 applies to both structural and nonstructural mass for the following
		 elements: CBAR, CBEAM, CONROD, CQUAD4, CHEXA,
		 CPENTA, CQUAD8, CROD, CTETRA, CTRIA3, CTRlA6, CTRIAX6,
		 CTUBE. A negative value (the default) causes the generation of
		 lumped mass matrices (which may include torsion inertia for beam
		 elements, and some coupling if there are beam offsets) for all of the
		 above elements.
		 If SYSTEM(414) is greater than zero, then a negative value causes the
		 generation of lumped mass matrices (translational components only)
		 for all of the above elements.
		 P-elements are always generated with coupled mass and are not
		 affected by COUPMASS.
		 */
		int value = tok.nextInt(true, -1);
		if (value != -1) {
			handleParsingError(
					string("unsupported parameter ") + param + string(" value in parsePARAM. "),
					tok, model);
		}
	} else if (param == "GRDPNT") {
		/*GRDPNT
		 Default = -1
		 GRDPNT>-1 will cause the grid point weight generator to be
		 executed. The default value (GRDPNT=-1) suppresses the
		 computation and output of this data. GRDPNT specifies the
		 identification number of the grid point to be used as a reference
		 point. If GRDPNT=0 or is not a defined grid point, the reference
		 point is taken as the origin of the basic coordinate system. All
		 fluid-related masses and masses on scalar points are ignored. The
		 following weight and balance information is automatically printed
		 following the execution of the grid point weight generator.
		 • Reference point.
		 • Rigid body mass matrix [MO] relative to the reference point
		 in the basic coordinate system.
		 • Transformation matrix [S] from the basic coordinate system
		 to principal mass axes.
		 • Principal masses (mass) and associated centers of gravity
		 (X-C.G., Y-C.G., Z-C.G.).
		 • Inertia matrix I(S) about the center of gravity relative to the
		 principal mass axes. Note: Change the signs of the
		 off-diagonal terms to produce the “inertia tensor.”
		 • Principal inertias I(Q) about the center of gravity.
		 • Transformation matrix [Q] between S-axes and Q-axes. The
		 columns of [Q] are the unit direction vectors for the
		 corresponding principal inertias.
		 In superelement static or geometric nonlinear analysis, GRDPNT> -1
		 also specifies the grid point to be used in computing resultants, in the
		 basic coordinate system, of external loads and single point constraint
		 forces applied to each superelement. If GRDPNT is not a grid point
		 (including the default value of -1), then the resultants are computed
		 about the origin of the basic coordinate system. In superelement
		 analysis, weights and resultants are computed for each superelement
		 without the effects of its upstream superelements.
		 For the CTRIAX6, CTRIAX, and CQUADX elements, the masses and
		 inertias are reported for the entire model of revolution but the center
		 of gravity is reported for the cross section in the x-z plane.
		 */
		int value = tok.nextInt(true, -1);
		handleParsingWarning(
				string("Ignored parameter ") + param + string("value ") + to_string(value)
						+ string(" in parsePARAM. "), tok, model);
	} else if (param == "K6ROT") {
		/* K6ROT specifies the scaling factor of the penalty stiffness to be added
		 to the normal rotation for CQUAD4 and CTRIA3 elements. The
		 contribution of the penalty term to the strain energy functional is ...*/
		double val = tok.nextDouble(true, 100.0);
		if (!is_equal(val, 100.0)) {
			handleParsingError(
					string("unsupported parameter ") + param + string(" value in parsePARAM. "),
					tok, model);
		}
	} else if (param == "LGDISP") {
		/* Default = -1
		 If LGDlSP = 1, all the nonlinear element types that have a large
		 displacement capability in SOLs 106, 129, 153, 159, 400, and 600 (see
		 Table 3-1 in the MSC.Nastran Reference Guide, under “Geometric
		 Nonlinear”) will be assumed to have large displacement effects
		 (updated element coordinates and follower forces). If LGDlSP = -1,
		 then no large displacement effects will be considered.
		 If LGDISP = 2, then follower force effects will be ignored but large
		 displacement effects will be considered.
		 If LGDISP ≥ 0 , then the differential stiffness is computed for the
		 linear elements and added to the differential stiffness of the
		 nonlinear elements.
		 */
		double val = tok.nextDouble(true, -1);
		if (!is_equal(val, -1)) {
			model->parameters[Model::LARGE_DISPLACEMENTS] = val;
		}
	} else if (param == "NOCOMPS") {
		/*
		 NOCOMPS controls the computation and printout of composite
		 element ply stresses, strains and failure indices. If NOCOMPS = 1,
		 composite element ply stresses, strains and failure indices are
		 printed. If NOCOMPS = 0, the same quantities plus element stresses
		 and strains for the equivalent homogeneous element are printed. If
		 NOCOMPS=-1, only element stresses and strains are printed.
		 Composite ply stress recovery is not available for complex stresses.
		 Homogenous stresses are based upon a smeared representation of
		 the laminate’s properties and in general will be incorrect. Element
		 strains are correct however.
		 */
		int value = tok.nextInt(true, 1);
		handleParsingWarning(
				string("Ignored parameter ") + param + string("value ") + to_string(value)
						+ string(" in parsePARAM. "), tok, model);
	} else if (param == "PATVER") {
		double val = tok.nextDouble(true, 3.0);
		if (!is_equal(val, 3.0)) {
			handleParsingError(
					string("unsupported parameter ") + param + string(" value in parsePARAM. "),
					tok, model);
			;
		}
	} else if (param == "PRTMAXIM") {
		/*
		 * PRTMAXIM
		 * Default = NO
		 * PRTMAXIM controls the printout of the maximums of applied loads,
		 * single-point forces of constraint, multipoint forces of constraint, and
		 * displacements. The printouts are titled “MAXIMUM APPLIED
		 * LOADS”, “MAXIMUM SPCFORCES”, “MAXIMUM
		 * MPCFORCES”, and “MAXIMUM DISPLACEMENTS”.
		 */
		string value = tok.nextString(true, "NO");
		if (value == "YES") {
			model->parameters[Model::PRINT_MAXIM] = 1.0;
		}
	} else if (param == "POST") {
		/*
		 * post treatment format, nothing to do
		 */
		tok.nextInt(true, 1);
	} else if (param == "WTMASS") {
		double value = tok.nextDouble(true, 1);
		model->parameters[Model::MASS_OVER_FORCE_MULTIPLIER] = value;
	} else {
		handleParsingError(string("Ignored parameter ") + param + string(" in parsePARAM. "), tok,
				model);
	}
}

void NastranParserImpl::handleParseException(vega::ParsingException &e, shared_ptr<Model> model,
		string message) {
	switch (translationMode) {
	case ConfigurationParameters::MODE_STRICT:
		throw e;
	case ConfigurationParameters::MESH_AT_LEAST:
		model->onlyMesh = true;
		cerr << message << " " << e.what() << endl;
		break;
	case ConfigurationParameters::BEST_EFFORT:
		cerr << message << " " << e.what() << endl;
		break;
	default:
		cerr << "Unknown enum in Translation mode, assuming MODE_STRICT" << endl;
		throw e;
	}
}

void NastranParserImpl::parseSPCADD(NastranTokenizer& tok, shared_ptr<Model> model) {
	int set_id = tok.nextInt();
	// retrieve the ConstraintSet that was created in the executive section
	shared_ptr<ConstraintSet> constraintSet_ptr = model->find(
			Reference<ConstraintSet>(ConstraintSet::SPC, set_id));

	if (!constraintSet_ptr)
		handleParsingError(
				string("Error in parseSPCADD. ConstraintSet does not exist in Executive section"),
				tok, model);
	else {
		while (tok.isNextInt()) {
			int constraintSet_id = tok.nextInt();
			ConstraintSet constraintSet(*model, ConstraintSet::SPC, constraintSet_id);
			model->add(constraintSet);
			Reference<ConstraintSet> constraintSetReference(constraintSet);
			constraintSet_ptr->add(constraintSetReference);
		}
	}
}

void NastranParserImpl::parseLOAD(NastranTokenizer& tok, shared_ptr<Model> model) {
	int set_id = tok.nextInt();
	LoadSet loadSet(*model, LoadSet::Type::LOAD, set_id);
	double S = tok.nextDouble(true, 1);
	while (tok.isNextDouble()) {
		double scale = tok.nextDouble(true, 1);
		tok.skipToNotEmpty();
		int loadSet_id = tok.nextInt();
		tok.skipToNotEmpty();
		Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadSet_id);
		loadSet.embedded_loadsets.push_back(
				pair<Reference<LoadSet>, double>(loadSetReference, S * scale));
	}
	model->add(loadSet);
}

void NastranParserImpl::handleParsingError(const string& message, NastranTokenizer& tok,
		shared_ptr<Model> model) {

	switch (translationMode) {
	case ConfigurationParameters::MODE_STRICT:
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	case ConfigurationParameters::MESH_AT_LEAST:
		model->onlyMesh = true;
		cerr << message << " Line: " << tok.lineNumber << " in file: " << tok.fileName << endl;
		break;
	case ConfigurationParameters::BEST_EFFORT:
		cerr << message << " Line: " << tok.lineNumber << " in file: " << tok.fileName << endl;
		break;
	default:
		cerr << "Unknown enum in Translation mode, assuming MODE_STRICT" << endl;
		throw ParsingException(message, tok.fileName, tok.lineNumber);
	}
}

void NastranParserImpl::handleParsingWarning(const string& message, NastranTokenizer& tok,
		shared_ptr<Model> model) {
	UNUSEDV(model);
	cerr << message << " Warning: Line: " << tok.lineNumber << " in file: " << tok.fileName << endl;
}

void NastranParserImpl::parseCELAS2(NastranTokenizer& tok, shared_ptr<Model> model) {
	// Defines a scalar spring element without reference to a property entry.
	int eid = tok.nextInt();
	double k = tok.nextDouble();
	int g1 = tok.nextInt();
	int c1 = tok.nextInt();
	int g2 = tok.nextInt();
	int c2 = tok.nextInt();
	StiffnessMatrix matrix(*model, eid);
	matrix.addStiffness(g1, DOF::findByPosition(c1), g2, DOF::findByPosition(c2), k);
	model->add(matrix);
}

void NastranParserImpl::parseCELAS4(NastranTokenizer& tok, shared_ptr<Model> model) {
	// Defines a scalar spring element that is connected only to scalar points, without
	// reference to a property entry.
	int eid = tok.nextInt();
	double k = tok.nextDouble();
	int s1 = tok.nextInt();
	int s2 = tok.nextInt();
	StiffnessMatrix matrix(*model, eid);
	// LD : TODO scalar point dofs treated as DX in abstract?
	matrix.addStiffness(s1, DOF::DX, s2, DOF::DX, k);
	model->add(matrix);
}

void NastranParserImpl::parseCMASS2(NastranTokenizer& tok, shared_ptr<Model> model) {
	// Defines a scalar mass element without reference to a property entry.
	int eid = tok.nextInt();
	double m = tok.nextDouble();
	int g1 = tok.nextInt();
	int c1 = tok.nextInt();
	int g2 = tok.nextInt();
	int c2 = tok.nextInt();
	MassMatrix matrix(*model, eid);
	matrix.addComponent(g1, DOF::findByPosition(c1), g2, DOF::findByPosition(c2), m);
	model->add(matrix);
}

} //namespace nastran

} //namespace vega
