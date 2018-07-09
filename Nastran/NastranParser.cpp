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
using boost::to_upper;

const unordered_map<string, NastranParser::parseElementFPtr> NastranParser::PARSE_FUNCTION_BY_KEYWORD =
        {
                { "CBAR", &NastranParser::parseCBAR },
                { "CBEAM", &NastranParser::parseCBEAM },
                { "CBUSH", &NastranParser::parseCBUSH },
                { "CGAP", &NastranParser::parseCGAP },
                { "CELAS1", &NastranParser::parseCELAS1 },
                { "CELAS2", &NastranParser::parseCELAS2 },
                { "CELAS4", &NastranParser::parseCELAS4 },
                { "CHEXA", &NastranParser::parseCHEXA },
                { "CMASS2", &NastranParser::parseCMASS2 },
                { "CONM2", &NastranParser::parseCONM2 },
                { "CORD1R", &NastranParser::parseCORD1R },
                { "CORD2C", &NastranParser::parseCORD2C },
                { "CORD2R", &NastranParser::parseCORD2R },
                { "CPENTA", &NastranParser::parseCPENTA },
                { "CPYRAMID", &NastranParser::parseCPYRAM },
                { "CQUAD", &NastranParser::parseCQUAD },
                { "CQUAD4", &NastranParser::parseCQUAD4 },
                { "CQUAD8", &NastranParser::parseCQUAD8 },
                { "CQUADR", &NastranParser::parseCQUADR },
                { "CROD", &NastranParser::parseCROD },
                { "CTETRA", &NastranParser::parseCTETRA },
                { "CTRIA3", &NastranParser::parseCTRIA3 },
                { "CTRIA6", &NastranParser::parseCTRIA6 },
                { "CTRIAR", &NastranParser::parseCTRIAR },
                { "DAREA", &NastranParser::parseDAREA },
                { "DELAY", &NastranParser::parseDELAY },
                { "DLOAD", &NastranParser::parseDLOAD },
                { "DMIG", &NastranParser::parseDMIG },
                { "DPHASE", &NastranParser::parseDPHASE },
                { "EIGR", &NastranParser::parseEIGR },
                { "EIGRL", &NastranParser::parseEIGRL },
                { "FORCE", &NastranParser::parseFORCE },
                { "FORCE1", &NastranParser::parseFORCE1 },
                { "FORCE2", &NastranParser::parseFORCE2 },
                { "FREQ1", &NastranParser::parseFREQ1 },
                { "FREQ4", &NastranParser::parseFREQ4 },
                { "GRAV", &NastranParser::parseGRAV },
                { "GRID", &NastranParser::parseGRID },
                { "INCLUDE", &NastranParser::parseInclude },
                { "LSEQ", &NastranParser::parseLSEQ },
                { "LOAD", &NastranParser::parseLOAD },
                { "MAT1", &NastranParser::parseMAT1 },
                { "MATS1", &NastranParser::parseMATS1 },
                { "MOMENT", &NastranParser::parseMOMENT },
                { "MPC", &NastranParser::parseMPC },
                { "NLPARM", &NastranParser::parseNLPARM },
                { "PARAM", &NastranParser::parsePARAM },
                { "PBAR", &NastranParser::parsePBAR },
                { "PBARL", &NastranParser::parsePBARL },
                { "PBEAM", &NastranParser::parsePBEAM },
                { "PBEAML", &NastranParser::parsePBEAML },
                { "PBUSH", &NastranParser::parsePBUSH },
                { "PELAS", &NastranParser::parsePELAS },
                { "PGAP", &NastranParser::parsePGAP },
                { "PLOAD1", &NastranParser::parsePLOAD1 },
                { "PLOAD2", &NastranParser::parsePLOAD2 },
                { "PLOAD4", &NastranParser::parsePLOAD4 },
                { "PROD", &NastranParser::parsePROD },
                { "PSHELL", &NastranParser::parsePSHELL },
                { "PSOLID", &NastranParser::parsePSOLID },
                { "RBAR", &NastranParser::parseRBAR },
                { "RBAR1", &NastranParser::parseRBAR1 },
                { "RBE2", &NastranParser::parseRBE2 },
                { "RBE3", &NastranParser::parseRBE3 },
                { "RFORCE", &NastranParser::parseRFORCE },
                { "RLOAD2", &NastranParser::parseRLOAD2 },
                { "SET3", &NastranParser::parseSET3 },
                { "SLOAD", &NastranParser::parseSLOAD },
                { "SPC", &NastranParser::parseSPC },
                { "SPC1", &NastranParser::parseSPC1 },
                { "SPCD", &NastranParser::parseSPCD },
                { "SPCADD", &NastranParser::parseSPCADD },
                { "SPOINT", &NastranParser::parseSPOINT },
                { "TABDMP1", &NastranParser::parseTABDMP1 },
                { "TABLED1", &NastranParser::parseTABLED1 },
                { "TEMP", &NastranParser::parseTEMP },
                { "GRDSET", &NastranParser::parseGRDSET }
        };

NastranParser::NastranParser() :
        Parser() {
}

string NastranParser::parseSubcase(NastranTokenizer& tok, shared_ptr<Model> model,
        map<string, string> context) {
    int subCaseId = tok.nextInt(true, 0);
    string nextKeyword;
    bool bParseSubcase =true;

    while (bParseSubcase){
        tok.nextLine();
        if (tok.nextSymbolType == NastranTokenizer::SYMBOL_EOF){
            bParseSubcase=false;
            continue;
        }
        nextKeyword = tok.nextString(true,"");
        if ((!nextKeyword.empty()) && (nextKeyword != "BEGIN") && (nextKeyword != "SUBCASE")) {
            string line ="";
            string sep="";
            while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
                line+=sep+tok.nextString(true,"");
                sep="_";
            }
            context[nextKeyword] = line;
        }else{
            bParseSubcase=false;
        }
    }

    try{
        addAnalysis(tok, model, context, subCaseId);
    }catch (std::string&){
        return nextKeyword;
    }
    return nextKeyword;
}

NastranParser::~NastranParser() {
}

void NastranParser::parseExecutiveSection(NastranTokenizer& tok, shared_ptr<Model> model,
        map<string, string>& context) {
    bool canContinue = true;
    bool readNewKeyword = true;
    bool subCaseFound = false;
    string keyword = "";

    tok.nextLine();
    keyword = tok.nextString(true, "");
    trim(keyword);

    while (canContinue){

        try{
            tok.setCurrentKeyword(keyword);
            if (keyword.find("BEGIN") != string::npos) {
                canContinue = false;
                if (!subCaseFound) {
                    addAnalysis(tok, model, context);
                }
            } else if (keyword == "B2GG") {
                // Selects direct input damping matrix or matrices.
                string line;
                while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
                    line += tok.nextString(true,"");
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
                //Nothing to do
                tok.skipToNextKeyword();
            } else if (keyword == "K2GG") {
                // Selects direct input stiffness matrix or matrices.
                string line;
                while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
                    line += tok.nextString();
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
                    line += tok.nextString();
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
                    line += tok.nextString(true,"");
                model->description = line;
            } else if (keyword == "TITLE") {
                string line;
                while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD)
                    line += tok.nextString(true,"");
                model->title = line;
            } else {
                if (tok.nextSymbolType != NastranTokenizer::SYMBOL_FIELD) {
                    context[keyword] = string("");
                } else {
                    string line;
                    while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD){
                         line += tok.nextString();
                    }
                    context[keyword] = line;
                }
            }

        } catch (std::string&) {
            // Parsing errors are catched by VegaCommandLine.
            // If we are not in strict mode, we dismiss this command and continue, hoping for the best.
            tok.skipToNextKeyword();
            canContinue = canContinue && (tok.nextSymbolType != NastranTokenizer::SYMBOL_EOF);
        }

        if (readNewKeyword) {
            tok.nextLine();
            if (tok.nextSymbolType != NastranTokenizer::SYMBOL_EOF){
                keyword = tok.nextString(true, "");
                trim(keyword);
            }
        }else{
            //new keyword was read by a parsing method
            readNewKeyword = true;
        }

        canContinue = canContinue && (tok.nextSymbolType != NastranTokenizer::SYMBOL_EOF);

    }
}

NastranParser::parseElementFPtr NastranParser::findCmdParser(string keyword) const {
    auto result = PARSE_FUNCTION_BY_KEYWORD.find(keyword);
    if (result != PARSE_FUNCTION_BY_KEYWORD.end()) {
        return result->second;
    } else {
        return nullptr;
    }
}

void NastranParser::parseBULKSection(NastranTokenizer &tok, shared_ptr<Model> model) {

    while (tok.nextSymbolType == NastranTokenizer::SYMBOL_KEYWORD) {
        string keyword = tok.nextString(true,"");
        tok.setCurrentKeyword(keyword);
        try{
            auto parser = findCmdParser(keyword);
            if (parser != nullptr) {
                (this->*parser)(tok, model);

            } else if (IGNORED_KEYWORDS.find(keyword) != IGNORED_KEYWORDS.end()) {
                if (model->configuration.logLevel >= LogLevel::TRACE) {
                    cout << "Keyword " << keyword << " ignored." << endl;
                }
                tok.skipToNextKeyword();

            } else if (!keyword.empty()) {
                handleParsingError(string("Unknown keyword."), tok, model);
                tok.skipToNextKeyword();
            }

            //Warning if there are unparsed fields. Skip the empty ones
            if (!tok.isEmptyUntilNextKeyword()) {
                string message(string("Parsing of line not complete."));
                handleParsingError(message, tok, model);
            }

        } catch (std::string&) {
            // Parsing errors are catched by VegaCommandLine.
            // If we are not in strict mode, we dismiss this command and continue, hoping for the best.
            tok.skipToNextKeyword();
        }
        tok.nextLine();
    }

}

fs::path NastranParser::findModelFile(const string& filename) {
    if (!fs::exists(filename)) {
        throw invalid_argument("Can't find file : " + fs::absolute(filename).string());
    }
    fs::path inputFilePath(filename);
    return inputFilePath;
}

shared_ptr<Model> NastranParser::parse(const ConfigurationParameters& configuration) {
    this->translationMode = configuration.translationMode;
    this->logLevel = configuration.logLevel;

    const string filename = configuration.inputFile;

    fs::path inputFilePath = findModelFile(filename);
    const string modelName = inputFilePath.filename().string();
    shared_ptr<Model> model = make_shared<Model>(modelName, "UNKNOWN", NASTRAN,
            configuration.getModelConfiguration());
    map<string, string> executive_section_context;
    const string inputFilePathStr = inputFilePath.string();
    ifstream istream(inputFilePathStr);
    NastranTokenizer tok = NastranTokenizer(istream, logLevel, inputFilePath.string(), this->translationMode);

    if (model->configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Parsing Executive section." << endl;
    }
    parseExecutiveSection(tok, model, executive_section_context);

    if (model->configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Parsing BULK section." << endl;
    }
    tok.bulkSection();
    parseBULKSection(tok, model);
    istream.close();

    if (model->configuration.logLevel >= LogLevel::DEBUG) {
        cout << "Parsing finished." << endl;
    }

    return model;
}

void NastranParser::addAnalysis(NastranTokenizer& tok, shared_ptr<Model> model, map<string, string> &context,
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
                handleParsingError(message, tok, model);
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
            handleParsingError("METHOD not found for linear modal analysis", tok, model);

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
            handleParsingError("NLPARM not found for non linear analysis", tok, model);
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
            handleParsingError("METHOD not found for linear dynamic modal frequency analysis", tok, model);
        if (modal_damping_original_id == 0)
            handleParsingError("SDAMPING not found for linear dynamic modal frequency analysis", tok, model);
        if (frequency_value_original_id == 0)
            handleParsingError("FREQ not found for linear dynamic modal frequency analysis", tok, model);

        /*
         auto it = context.find("METHOD");
         int frequency_band_original_id = 0;
         if (it == context.end())
         it = context.find("METHOD(STRUCTURE)");
         if (it == context.end())
         handleParsingError("METHOD not found for linear dynamic modal frequency analysis", tok, model);
         else
         frequency_band_original_id = atoi(it->second.c_str());

         it = context.find("SDAMPING");
         int modal_damping_original_id = 0;
         if (it == context.end())
         it = context.find("SDAMPING(STRUCTURE)");
         if (it == context.end())
         handleParsingError("SDAMPING not found for linear dynamic modal frequency analysis", tok, model);
         else
         modal_damping_original_id = atoi(it->second.c_str());

         it = context.find("FREQ");
         int frequency_value_original_id = 0;
         if (it == context.end())
         it = context.find("FREQUENCY");
         if (it == context.end())
         handleParsingError("FREQ not found for linear dynamic modal frequency analysis", tok, model);
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
        handleParsingError(message, tok, model);
    }
}

void NastranParser::parseCONM2(NastranTokenizer& tok, shared_ptr<Model> model) {
    int elemId = tok.nextInt();
    int g = tok.nextInt(); // Grid point identification number
    int ci = tok.nextInt(true, 0);
    if (ci == -1) {
        string message = "coordinate system CID=-1 not supported and dismissed.";
        handleParsingWarning(message, tok, model);
        ci = 0;
    }
    const double mass = tok.nextDouble();
    const double x1 = tok.nextDouble(true, 0.0);
    const double x2 = tok.nextDouble(true, 0.0);
    const double x3 = tok.nextDouble(true, 0.0);

    // User defined CID is only important if the offset is not null
    if ((ci != 0) && (!is_zero(x1) || !is_zero(x2) || !is_zero(x3))) {
        string message = "coordinate system CID not supported and dismissed.";
        handleParsingWarning(message, tok, model);
        ci = 0;
    }
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
    auto mnodale = model->mesh->createCellGroup(mn, CellGroup::NO_ORIGINAL_ID, "NODAL MASS");
    mnodale->addCellId(model->mesh->findCell(cellPosition).id);
    nodalMass.assignCellGroup(mnodale);
    nodalMass.assignMaterial(model->getVirtualMaterial());

    model->add(nodalMass);
}

void NastranParser::parseCORD1R(NastranTokenizer& tok, shared_ptr<Model> model) {

    while (tok.isNextInt()) {
        int cid = tok.nextInt();
        int nA  = tok.nextInt();
        int nB  = tok.nextInt();
        int nC  = tok.nextInt();
        CartesianCoordinateSystem coordinateSystem(*model, nA, nB, nC, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID, cid);
        model->add(coordinateSystem);
        }
}

void NastranParser::parseCORD2C(NastranTokenizer& tok, shared_ptr<Model> model) {
    int cid = tok.nextInt();
    //reference coordinate system 0 for global.
    int rid = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);

    double coor[3];
    VectorialValue vect[3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            coor[j] = tok.nextDouble(true,0.0);
        vect[i] = VectorialValue(coor[0], coor[1], coor[2]);
    }

    VectorialValue ez = vect[1] - vect[0];
    VectorialValue ex = (vect[2] - vect[0]).orthonormalized(ez);
    VectorialValue ey = ez.cross(ex);

    CylindricalCoordinateSystem coordinateSystem(*model, vect[0], ex, ey, rid, cid);
    model->add(coordinateSystem);
}

void NastranParser::parseCORD2R(NastranTokenizer& tok, shared_ptr<Model> model) {
    int cid = tok.nextInt();
    int rid = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    double coor[3];
    VectorialValue vect[3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            coor[j] = tok.nextDouble(true, 0.0);
        vect[i] = VectorialValue(coor[0], coor[1], coor[2]);
    }

    VectorialValue ez = vect[1] - vect[0];
    VectorialValue ex = (vect[2] - vect[0]).orthonormalized(ez);
    VectorialValue ey = ez.cross(ex);

    CartesianCoordinateSystem coordinateSystem(*model, vect[0], ex, ey, rid, cid);
    model->add(coordinateSystem);
}

void NastranParser::parseDAREA(NastranTokenizer& tok, shared_ptr<Model> model) {
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

        NodalForce force1(*model, tx, ty, tz, rx, ry, rz, Loading::NO_ORIGINAL_ID);
        force1.addNode(node_id);
        model->add(force1);
        model->addLoadingIntoLoadSet(force1, loadset_ref);
    }
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::EXCITEID, loadset_id);
        model->add(loadSet);
    }
}

//TODO: Delay should not be a DynaPhase object
//TODO: But I guess this will be enough for now
void NastranParser::parseDELAY(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    int p = tok.nextInt(true);
    if (p!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Point identification (P) ignored and dismissed.", tok, model);
    }
    int c = tok.nextInt(true);
    if (c!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Component number (C) ignored and dismissed.", tok, model);
    }
    double delay = tok.nextDouble();

    if (!tok.isEmptyUntilNextKeyword()){
        handleParsingWarning("Second dynamic load phase dismissed.", tok, model);
    }
    DynaPhase dynaphase(*model, -2*M_PI*delay, original_id);
    model->add(dynaphase);
}



void NastranParser::parseDLOAD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    LoadSet loadSet(*model, LoadSet::DLOAD, loadset_id);

    double S = tok.nextDouble(true, 1);

    //vega::FunctionTable functionTable(*model, FunctionTable::LINEAR);
    while (tok.isNextDouble()) {
        double scale = tok.nextDouble(true, 1);
        int rload2_id = tok.nextInt();
        Reference<LoadSet> loadSetReference(LoadSet::LOAD, rload2_id);
        loadSet.embedded_loadsets.push_back(
                            pair<Reference<LoadSet>, double>(loadSetReference, S * scale));
    }
    model->add(loadSet);
}

void NastranParser::parseDMIG(NastranTokenizer& tok, shared_ptr<Model> model) {
    string name = tok.nextString();
    if (name == "UACCEL") {
        handleParsingWarning("UACCEL not supported and dismissed.", tok, model);
        tok.skipToNextKeyword();
        return;
    }
    if (name == "CDSHUT") {
        handleParsingWarning("CDSHUT is ignored.", tok, model);
        tok.skipToNextKeyword();
        return; // currently ignored, see CDPCH
    }

    auto it = directMatrixByName.find(name);

    int headerIndicator = tok.nextInt();
    if (headerIndicator == 0) {

        // If the matrix doesn't exist, it means it's not used by the model.
        // It's often the case on industrial cases, when various matrices are written in the same file, but only one is used.
        if (it == directMatrixByName.end()) {
            handleParsingWarning("Matrix "+name+" is not used by the model and ignored.", tok, model);
            tok.skipToNextKeyword();
            return;
        }

        //  Field 3 of the header entry must contain an integer 0.
        int ifo = tok.nextInt();
        if (ifo != 6) {
            handleParsingError("Non-symmetric DMIG not yet implemented.", tok, model);
        }
        int tin = tok.nextInt();
        if (tin != 1 && tin != 2) {
            handleParsingError("Non-real or non-single precision DMIG not yet implemented", tok, model);
        }
        int tout = tok.nextInt(true, 0);
        if (tout != 0) {
            handleParsingError("TOUT in DMIG not yet implemented", tok, model);
        }
        int polar = tok.nextInt(true, 0);
        if (polar != 0) {
            handleParsingError("POLAR in DMIG not yet implemented", tok, model);
        }

        tok.skip(1);
        int ncol = tok.nextInt(true,0); // NCOL is not used for now
        UNUSEDV(ncol);
        return;
    }else{
        // Matrix doesn't exists, we skip it (quietly, because the warning message was already displayed once).
        if (it == directMatrixByName.end()) {
            tok.skipToNextKeyword();
            return;
        }
    }

    shared_ptr<MatrixElement> matrix = static_pointer_cast<MatrixElement>(
                model->find(*(it->second)));


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

void NastranParser::parseDPHASE(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    int p = tok.nextInt(true);
    if (p!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Point identification (P) ignored and dismissed.", tok, model);
    }
    int c = tok.nextInt(true);
    if (c!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Component number (C) ignored and dismissed.", tok, model);
    }
    double dphase = tok.nextDouble();

    if (!tok.isEmptyUntilNextKeyword()){
        handleParsingWarning("Second dynamic load phase dissmissed.", tok, model);
    }
    DynaPhase dynaphase(*model, dphase, original_id);
    model->add(dynaphase);
}

void NastranParser::parseEIGR(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    string method = tok.nextString(true);
    if (method !="LAN"){
        handleParsingError("Only Lanczos method (LAN) is supported.", tok, model);
    }
    double lower = tok.nextDouble(true);
    double upper = tok.nextDouble(true);
    int ne = tok.nextInt(true); // Estimate of number of roots in range: not use by the LANCZOS method
    UNUSEDV(ne);
    int nd = tok.nextInt(true); // Desired number of roots in range

    // See Nastran Quick reference guide for the treatment of nd in the Lancszos method
    if ((nd==Globals::UNAVAILABLE_INT) && (is_equal(upper, Globals::UNAVAILABLE_DOUBLE))){
        nd=1;
    }

    tok.skip(2);

    string norm = tok.nextString(true, "MASS");
    if ((norm !="MASS") && (norm != "MAX")){
        handleParsingWarning("Only MASS and MAX normalizing method (NORM) supported. Default (MASS) assumed.", tok, model);
        norm = "MASS";
    }
    int g = tok.nextInt(true);
    if (g!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Grid identification number (G) not supported.", tok, model);
    }
    int c = tok.nextInt(true);
    if (c!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Component number (C) not supported.", tok, model);
    }

    FrequencyBand frequencyBand(*model, lower, upper, nd, norm, original_id);
    model->add(frequencyBand);
}

void NastranParser::parseEIGRL(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    double lower = tok.nextDouble(true);
    double upper = tok.nextDouble(true);
    int nd = tok.nextInt(true);

    // See Nastran Quick reference guide for the treatment of nd
    if ((nd==Globals::UNAVAILABLE_INT) && (is_equal(upper, Globals::UNAVAILABLE_DOUBLE))){
        nd=1;
    }

    // Diagnostic level: not supported, but useless.
    int msglvl = tok.nextInt(true);
    UNUSEDV(msglvl);

    // Unsupported fields
    int maxset = tok.nextInt(true);
    if (maxset!=Globals::UNAVAILABLE_INT){
        handleParsingWarning("Numbers of vectors in set (MAXSET) not supported and dismissed.", tok, model);
    }
    double shfscl = tok.nextDouble(true);
    if (!is_equal(shfscl, Globals::UNAVAILABLE_DOUBLE)){
        handleParsingWarning("Estimate of the first flexible mode natural frequency (SHFSCL) not supported and dismissed.", tok, model);
    }

    // Normalization method
    string norm = tok.nextString(true, "MASS");
    if ((norm !="MASS") && (norm != "MAX")){
        handleParsingWarning("Only MASS and MAX normalizing method (NORM) supported. Default (MASS) assumed.", tok, model);
        norm="MASS";
    }

    FrequencyBand frequencyBand(*model, lower, upper, nd, norm, original_id);
    model->add(frequencyBand);
}

void NastranParser::parseFORCE(NastranTokenizer& tok, shared_ptr<Model> model) {

    int loadset_id = tok.nextInt();
    int node_id = tok.nextInt();
    int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    double force = tok.nextDouble(true,0.0);
    double fx = tok.nextDouble(true,0.0) * force;
    double fy = tok.nextDouble(true,0.0) * force;
    double fz = tok.nextDouble(true,0.0) * force;

    NodalForce force1(*model, fx, fy, fz, 0., 0., 0., Loading::NO_ORIGINAL_ID,
            coordinate_system_id);
    force1.addNode(node_id);

    model->add(force1);
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
    model->addLoadingIntoLoadSet(force1, loadset_ref);
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}

void NastranParser::parseFORCE1(NastranTokenizer& tok, shared_ptr<Model> model) {
//nodal force two nodes
    int loadset_id = tok.nextInt();
    int node_id = tok.nextInt();
    double force = tok.nextDouble();
    int node1 = tok.nextInt();
    int node2 = tok.nextInt();

    NodalForceTwoNodes force1(*model, node1, node2, force);
    force1.addNode(node_id);

    model->add(force1);
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
    model->addLoadingIntoLoadSet(force1, loadset_ref);
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}

void NastranParser::parseFORCE2(NastranTokenizer& tok, shared_ptr<Model> model) {
    int sid = tok.nextInt();
    int node_id = tok.nextInt();
    double force = tok.nextDouble();
    int node1 = tok.nextInt();
    int node2 = tok.nextInt();
    int node3 = tok.nextInt();
    int node4 = tok.nextInt();

    NodalForceFourNodes force2(*model, node1, node2, node3, node4, force);
    force2.addNode(node_id);

    model->add(force2);
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, sid);
    model->addLoadingIntoLoadSet(force2, loadset_ref);
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, sid);
        model->add(loadSet);
    }
}

void NastranParser::parseFREQ1(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    double start = tok.nextDouble();
    double step = tok.nextDouble();
    int count = tok.nextInt(true, 1);

    vega::StepRange stepRange(*model, start, step, count);
    stepRange.setParaX(NamedValue::FREQ);
    FrequencyRange frequencyValues(*model, stepRange, original_id);

    model->add(stepRange);
    model->add(frequencyValues);
}

void NastranParser::parseFREQ4(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    double f1 = tok.nextDouble();
    double f2 = tok.nextDouble();
    double spread = tok.nextDouble();
    int count = tok.nextInt(true, 1);

    vega::SpreadRange spreadRange(*model, f1, count, f2, spread);
    spreadRange.setParaX(NamedValue::FREQ);
    FrequencyRange frequencyValues(*model, spreadRange, original_id);

    model->add(spreadRange);
    model->add(frequencyValues);
}

void NastranParser::parseGRAV(NastranTokenizer& tok, shared_ptr<Model> model) {
    int sid = tok.nextInt();
    int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (coordinate_system_id != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        string message = "CoordinateSystem not supported.";
        handleParsingWarning(message, tok, model);
    }
    double acceleration = tok.nextDouble(true, 0);
    double x = tok.nextDouble(true, 0);
    double y = tok.nextDouble(true, 0);
    double z = tok.nextDouble(true, 0);
    int mb = tok.nextInt(true, 0);
    if (mb != 0) {
        string message = "MB not supported.";
        handleParsingWarning(message, tok, model);
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
void NastranParser::parseInclude(NastranTokenizer& tok, shared_ptr<Model> model) {
    string currentRawDataLine = tok.currentRawDataLine();
    string fileName = currentRawDataLine.substr(7, currentRawDataLine.length() - 7);
    trim(fileName);
    if (!fileName.compare(0, 1, "'")
            && !fileName.compare(fileName.size() - 1, fileName.size(), "'"))
        fileName = fileName.substr(1, fileName.size() - 2);
    fs::path currentFname(tok.getFileName());
    fs::path includePath = currentFname.parent_path() / fileName;
    const string includePathStr = includePath.string();
    if (fs::exists(includePath)) {
        ifstream istream(includePathStr);
        NastranTokenizer tok2 = NastranTokenizer(istream, this->logLevel, includePathStr, this->translationMode);
        tok2.bulkSection();
        tok2.nextLine();
        parseBULKSection(tok2, model);
        istream.close();
    } else {
        handleParsingError("Missing include file "+includePathStr, tok, model);
    }
    tok.skipToNextKeyword();
}

void NastranParser::parseLSEQ(NastranTokenizer& tok, shared_ptr<Model> model) {
    int set_id = tok.nextInt();
    LoadSet loadSet(*model, LoadSet::Type::LOAD, set_id);
    int darea_id = tok.nextInt();
    // LD : not sure about this interpretation
    Reference<LoadSet> dareaReference(LoadSet::LOAD, darea_id);
    loadSet.embedded_loadsets.push_back(
                    pair<Reference<LoadSet>, double>(dareaReference, 1.0));
    int loadSet_id = tok.nextInt();
    Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadSet_id);
    loadSet.embedded_loadsets.push_back(
            pair<Reference<LoadSet>, double>(loadSetReference, 1.0));
    model->add(loadSet);

}

void NastranParser::parseLOAD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int set_id = tok.nextInt();
    LoadSet loadSet(*model, LoadSet::Type::LOAD, set_id);
    double S = tok.nextDouble(true, 1);
    while (tok.isNextDouble()) {
        double scale = tok.nextDouble(true, 1);
        int loadSet_id = tok.nextInt();
        Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadSet_id);
        loadSet.embedded_loadsets.push_back(
                pair<Reference<LoadSet>, double>(loadSetReference, S * scale));
    }
    model->add(loadSet);
}

void NastranParser::parseMAT1(NastranTokenizer& tok, shared_ptr<Model> model) {
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

    /*  ST, SC, SS
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
    /*  MCSID
     Material coordinate system identification number. Used only for
     PARAM,CURV processing. See “Parameters” on page 659.
     (Integer > 0 or blank)*/
    int mcsid = tok.nextInt(true, 0);
    if (mcsid != 0) {
        handleParsingWarning("mcsid value ignored " + to_string(mcsid), tok, model);
    }
    shared_ptr<Material> material = model->getOrCreateMaterial(material_id);
    material->addNature(ElasticNature(*model, e, nu, g, rho, a, tref, ge));
}

void NastranParser::parseMATS1(NastranTokenizer& tok, shared_ptr<Model> model) {
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

void NastranParser::parseMOMENT(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    int node_id = tok.nextInt();
    int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (coordinate_system_id != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        handleParsingWarning("MOMENT coordinate system not supported", tok, model);
    }
    double scale = tok.nextDouble(true);
    double frx = tok.nextDouble(true) * scale;
    double fry = tok.nextDouble(true) * scale;
    double frz = tok.nextDouble(true) * scale;

    NodalForce force1(*model, VectorialValue(0, 0, 0), VectorialValue(frx, fry, frz),
            Loading::NO_ORIGINAL_ID);
    force1.addNode(node_id);
    model->add(force1);
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);
    model->addLoadingIntoLoadSet(force1, loadset_ref);
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}

void NastranParser::parseMPC(NastranTokenizer& tok, shared_ptr<Model> model) {
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

void NastranParser::parseNLPARM(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    int number_of_increments = tok.nextInt(true, 10);

    if (!tok.isEmptyUntilNextKeyword()){
        tok.skipToNextKeyword();
        handleParsingWarning("All parameters are ignored except NINC.", tok, model);
    }

    NonLinearStrategy nonLinearStrategy(*model, number_of_increments, original_id);
    model->add(nonLinearStrategy);
}


void NastranParser::parsePBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
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
        handleParsingWarning(message, tok, model);
    }

    GenericSectionBeam genericSectionBeam(*model, area, i2, i1, j, invk2, invk1, Beam::EULER, nsm,
            elemId);
    genericSectionBeam.assignMaterial(material_id);
    genericSectionBeam.assignCellGroup(getOrCreateCellGroup(elemId, model, "PBAR"));
    model->add(genericSectionBeam);
}

void NastranParser::parsePBARL(NastranTokenizer& tok, shared_ptr<Model> model) {
    int propertyId = tok.nextInt(); // PID
    int material_id = tok.nextInt();

    string group = tok.nextString(true, "MSCBML0");
    if (group!="MSCBML0"){
        string message = "PBARL users defined groups are not supported.";
        handleParsingError(message, tok, model);
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
    } else if (type == "I") {
        tok.skip(4);
        double beam_height = tok.nextDouble();
        double lower_flange_width = tok.nextDouble();
        double upper_flange_width = tok.nextDouble();
        double web_thickness = tok.nextDouble();
        double lower_flange_thickness = tok.nextDouble();
        double upper_flange_thickness = tok.nextDouble();
        nsm = tok.nextDouble(true, 0.0);
        ISectionBeam iSectionBeam(*model, upper_flange_width, lower_flange_width,
                upper_flange_thickness, lower_flange_thickness, beam_height, web_thickness,
                Beam::EULER, nsm, propertyId);
        iSectionBeam.assignMaterial(material_id);
        iSectionBeam.assignCellGroup(getOrCreateCellGroup(propertyId, model, "PBARL"));
        model->add(iSectionBeam);
    } else {
        string message = "PBARL type " + type + " not implemented.";
        handleParsingError(message, tok, model);
    }

    if (!tok.isEmptyUntilNextKeyword()) {
        tok.skipToNextKeyword();
        handleParsingWarning("Ignoring rest of line.", tok,
             model);
    }

}

void NastranParser::parsePBEAM(NastranTokenizer& tok, shared_ptr<Model> model) {
    int elemId = tok.nextInt();
    int material_id = tok.nextInt();

    double area_cross_section = tok.nextDouble(true, 0.0);
    double moment_of_inertia_Z = tok.nextDouble(true, 0.0);
    double moment_of_inertia_Y = tok.nextDouble(true, 0.0);
    double areaProductOfInertia = tok.nextDouble(true, 0.0);
    if (!is_equal(areaProductOfInertia, 0.0)) {
        handleParsingWarning("Area product of inertia not implemented.", tok, model);
    }
    double torsionalConstant = tok.nextDouble(true, 0.0);
    double nsm = tok.nextDouble(true, 0.0);
    if (!is_equal(nsm, 0.0)) {
        handleParsingWarning("NSM not implemented.", tok, model);
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
        handleParsingWarning("Shear center for stress analysis not implemented.", tok,
                model);
    }
    GenericSectionBeam genericSectionBeam(*model, area_cross_section, moment_of_inertia_Y,
            moment_of_inertia_Z, torsionalConstant, 0.0, 0.0, GenericSectionBeam::EULER, nsm,
            elemId);
    genericSectionBeam.assignMaterial(material_id);
    genericSectionBeam.assignCellGroup(getOrCreateCellGroup(elemId, model, "PBEAM"));
    model->add(genericSectionBeam);

    // Intermediate stations are not supported
    int nbStations=0;
    while (! (tok.isEmptyUntilNextKeyword() || tok.isNextDouble() || tok.isNextEmpty())){
        nbStations++;
        tok.skip(16);
        if (nbStations == 1) {
            handleParsingWarning("PBEAM intermediate stations are not supported", tok, model);
        }
    }

    // A bunch of parameters we don't support
    double K1 = tok.nextDouble(true, 1.0); // Shear stiffness factor for Plane 1
    double K2 = tok.nextDouble(true, 1.0); // Shear stiffness factor for Plane 2
    if ((!is_equal(K1, 1.0)) || (!is_equal(K2,1.0))){
        handleParsingWarning("Shear stiffness factors (K1, K2) are not supported.", tok, model);
    }

    double S1 = tok.nextDouble(true, 0.0); // Shear relief coefficient for Plane 1
    double S2 = tok.nextDouble(true, 0.0); // Shear relief coefficient for Plane 2
    if ((!is_zero(S1)) || (!is_zero(S2))){
        handleParsingWarning("Shear relief coefficients (S1, S2) are not supported.", tok, model);
    }

    double NSIA = tok.nextDouble(true, 0.0); // Nonstructural mass moment of inertia per unit length about nonstructural mass center of gravity at end A and end B.
    double NSIB = tok.nextDouble(true, 0.0);
    if ((!is_zero(NSIA)) || (!is_zero(NSIB))){
        handleParsingWarning("Nonstructural mass centers of gravity (NSIA, NSIB) are not supported.", tok, model);
    }

    double CWA = tok.nextDouble(true, 0.0); // Warping coefficient for end A and end B.
    double CWB = tok.nextDouble(true, 0.0);
    if ((!is_zero(CWA)) || (!is_zero(CWB))){
        handleParsingWarning("Warping coefficients (CWA, CWB) are not supported.", tok, model);
    }

    double M1A = tok.nextDouble(true, 0.0); // (y,z) coordinates of center of gravity of nonstructural mass for end A and end B.
    double M2A = tok.nextDouble(true, 0.0);
    double M1B = tok.nextDouble(true, 0.0);
    double M2B = tok.nextDouble(true, 0.0);
    if ( (!is_zero(M1A)) || (!is_zero(M2A)) || (!is_zero(M1B)) || (!is_zero(M2B))){
        handleParsingWarning("Center of gravity of nonstructural mass (M1A, M2A, M1B, M2B) are not supported.", tok, model);
    }

    double N1A = tok.nextDouble(true, 0.0); // (y,z) coordinates of neutral axis for end A and end B.
    double N2A = tok.nextDouble(true, 0.0);
    double N1B = tok.nextDouble(true, 0.0);
    double N2B = tok.nextDouble(true, 0.0);
    if ( (!is_zero(N1A)) || (!is_zero(N2A)) || (!is_zero(N1B)) || (!is_zero(N2B))){
        handleParsingWarning("Neutral axis (N1A, N2A, N1B, N2B) are not supported.", tok, model);
    }

}

void NastranParser::parsePBEAML(NastranTokenizer& tok, shared_ptr<Model> model) {
    int pid = tok.nextInt();
    int mid = tok.nextInt();

    string group = tok.nextString(true, "MSCBML0");
    if (group!="MSCBML0"){
        string message = "PBARL users defined groups are not supported.";
        handleParsingError(message, tok, model);
    }


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

    if (!tok.isEmptyUntilNextKeyword()) {
        tok.skipToNextKeyword();
        handleParsingWarning("Ignoring rest of line.", tok,
             model);
    }

}



/** Parse the NASTRAN PBUSH Keyword: Generalized Spring-And-Damper Property **/
void NastranParser::parsePBUSH(NastranTokenizer& tok, shared_ptr<Model> model) {

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
    // specified null B or null GE.
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
            handleParsingWarning(string("unknown flag: ")+flag, tok, model);
        }
    }
    // Ony K is supported yet
    if (!is_equal(b1, 0) || !is_equal(b2, 0) || !is_equal(b3, 0) || !is_equal(b4, 0)
            || !is_equal(b5, 0) || !is_equal(b6, 0) ) {
        b1=0.0; b2=0.0; b3=0.0; b4=0.0; b5=0.0; b6=0.0;
        handleParsingWarning(string("Force-Per-velocity Damping B not supported. Default (0.0) assumed."), tok, model);
    }
    if (!is_equal(ge1, 0) || !is_equal(ge2, 0) || !is_equal(ge3, 0) || !is_equal(ge4, 0)
            || !is_equal(ge5, 0) || !is_equal(ge6, 0) ) {
        ge1=0.0; ge2=0.0; ge3=0.0; ge4=0.0; ge5=0.0; ge6=0.0;
        handleParsingWarning(string("Structural Damping constants GE not supported. Default (0.0) assumed."), tok, model);
    }
    if (!is_equal(sa, 1.0) || !is_equal(st, 1.0) || !is_equal(ea, 1.0) || !is_equal(et,1.0)) {
        sa=1.0; st=1.0; ea=1.0; et=1.0;
        handleParsingWarning(string("Stress and Strain recovery coefficients (SA, ST, EA, ET ) not supported. Default (1.0) assumed."), tok, model);
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

void NastranParser::parsePELAS(NastranTokenizer& tok, shared_ptr<Model> model) {

    int nbProperties=0;
    // One or two elastic spring properties can be defined on a single entry.
    while ((tok.isNextInt())&&(nbProperties<2)){

        const int pid = tok.nextInt();
        const double k = tok.nextDouble(true, 0.0);
        const double ge= tok.nextDouble(true, 0.0);
        const double s = tok.nextDouble(true);

        // S is only used for post-treatment, and so discarded.
        if (!is_equal(s, NastranTokenizer::UNAVAILABLE_DOUBLE)){
            if (this->logLevel >= LogLevel::DEBUG) {
                handleParsingWarning("Stress coefficient (S) is only used for post-treatment and dismissed.", tok, model);
            }
        }

        shared_ptr<CellGroup> cellGroup = getOrCreateCellGroup(pid, model, "PELAS");
        shared_ptr<ElementSet> elementSet = model->elementSets.find(pid);
        if (elementSet == nullptr){
            ScalarSpring scalarSpring(*model, pid, k, ge);
            scalarSpring.assignCellGroup(cellGroup);
            model->add(scalarSpring);
        }else{
            if (elementSet->type == ElementSet::SCALAR_SPRING){
                shared_ptr<ScalarSpring> springElementSet = static_pointer_cast<ScalarSpring>(elementSet);
                springElementSet->setStiffness(k);
                springElementSet->setDamping(ge);
                springElementSet->assignCellGroup(cellGroup);
            }else{
                string message = "The part of PID "+std::to_string(pid)+" already exists with the wrong NATURE.";
                handleParsingError(message, tok, model);
            }
        }
        nbProperties++;
    }
}

void NastranParser::parsePGAP(NastranTokenizer& tok, shared_ptr<Model> model) {
    int pid = tok.nextInt();
    double u0 = tok.nextDouble(true, 0.0);
    double f0 = tok.nextDouble(true, 0.0);
    if (!is_equal(f0, 0)) {
        handleParsingWarning(string("Unsupported f0 ") + to_string(f0), tok, model);
        f0 = 0.0;
    }
    double ka = tok.nextDouble();
    handleParsingWarning(string("Ignored ka ") + to_string(ka), tok, model);

    // A bunch of parameters that VEGA does not use. To keep for completion and warning
    double kb = tok.nextDouble(true, 0.0001*ka); UNUSEDV(kb);
    double kt = tok.nextDouble(true);
    double mu1 = tok.nextDouble(true, 0.0);
    double mu2 = tok.nextDouble(true, mu1); UNUSEDV(mu2);
    tok.skip(1);
    double tmax = tok.nextDouble(true, 0.0); UNUSEDV(tmax);
    double mar = tok.nextDouble(true, 100); UNUSEDV(mar);
    double trmin = tok.nextDouble(true, 0.001); UNUSEDV(trmin);
    if (is_equal(kt, Globals::UNAVAILABLE_DOUBLE)){
        kt = mu1*ka;
    }

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

void NastranParser::parsePLOAD1(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    int eid = tok.nextInt();
    string type = tok.nextString();
    string scale = tok.nextString();
    shared_ptr<FunctionTable> force = make_shared<FunctionTable>(*model, FunctionTable::LINEAR, FunctionTable::LINEAR, FunctionTable::NONE, FunctionTable::NONE);
    force->setParaX(FunctionTable::PARAX);
    DOF dof = DOF::DX;
    double x1 = tok.nextDouble(true, 0.0);
    double p1 = tok.nextDouble(true, 0.0);
    double x2 = tok.nextDouble(true, -1.0);
    double p2 = tok.nextDouble(true, 0.0);
    int smalldistancefactor = 1000;

    if (type == "FX")
        dof = DOF::DX;
    else if (type == "FY")
        dof = DOF::DY;
    else if (type == "FZ")
        dof = DOF::DZ;
    else if (type == "MX")
        dof = DOF::RX;
    else if (type == "MY")
        dof = DOF::RY;
    else if (type == "MZ")
        dof = DOF::RZ;
    else
        handleParsingError(string("PLOAD1 TYPE not yet implemented."), tok, model);

    double effx1, effx2, effp1, effp2;

    if (scale == "LE") {
        /* If SCALE = LE, the total load applied to the bar is P1(X2 - X1) in the yb direction. */
        effx1 = x1;
        effx2 = x2;
        effp1 = p1;
        effp2 = p2;
    } else if (scale == "FRPR") {
        /* If SCALE = FRPR (fractional projected), the Xi values are ratios of the actual distance to the length of the bar
         * and (X1 ≠ X2) the distributed load is input in terms of the projected length of the bar.*/
        // TODO LD: encapsulate all this in mesh/cell/node (but it needs model)
        int cellPos = model->mesh->findCellPosition(eid);
        Cell cell = model->mesh->findCell(cellPos);
        std::shared_ptr<OrientationCoordinateSystem> ocs = cell.getOrientation(model.get());
        Node firstNode = model->mesh->findNode(cell.nodePositions.front());
        firstNode.buildGlobalXYZ(model.get());
        Node lastNode = model->mesh->findNode(cell.nodePositions.back());
        lastNode.buildGlobalXYZ(model.get());
        VectorialValue barvect = VectorialValue(lastNode.x - firstNode.x, lastNode.y - firstNode.y, lastNode.z - firstNode.z);
        double dist = barvect.norm();
        /* If SCALE = LE (length), the Xi values are actual distances along the bar x-axis,
         * and (if X1 ≠ X2) Pi are load intensities per unit length of the bar. */
        effx1 = x1 * dist;
        effx2 = x2 * dist;
        effp1 = p1;
        effp2 = p2;
    } else {
        handleParsingError(string("PLOAD1 SCALE not yet implemented."), tok, model);
    }

    if (effx1 > 0.0)
        force->setXY(effx1 - effx1 / smalldistancefactor, 0.0);
    if (x2 < 0 || is_equal(x2, x1)) {
        // If X2 is blank or equal to X1, a concentrated load of value P1 will be applied at position X1.
        //effp1 = effp1 * 2 / (2 * effx1 / smalldistancefactor); // compunting equivalent triangle area to get the resultant force
        //force->setXY(effx1, effp1);
        //force->setXY(effx1 + effx1 / smalldistancefactor, 0.0);
        // TODO : this does not seem to work, should split beam and add one node, then apply nodal force
        handleParsingError(string("Concentrated force not yet handled."), tok, model);
    } else {
        // TODO LD: this won't do, really need an absolute space function (at least in the writer)
        force->setXY(effx1, effp1);
        force->setXY(effx2, effp2);
        force->setXY(effx2 + effx2 / smalldistancefactor, 0.0);
    }
    model->add(*force);
    Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadset_id);
    ForceLine forceLine(*model, force, dof);
    forceLine.addCell(eid);
    model->add(forceLine);
    model->addLoadingIntoLoadSet(forceLine, loadSetReference);
    if (!model->find(loadSetReference)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}

void NastranParser::parsePLOAD2(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    double p = tok.nextDouble();
    Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadset_id);
    NormalPressionFace normalPressionFace(*model, p);
    for(int cellId : tok.nextInts()) {
        normalPressionFace.addCell(cellId);
    }
    model->addLoadingIntoLoadSet(normalPressionFace, loadSetReference);
    if (!model->find(loadSetReference)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}

void NastranParser::parsePLOAD4(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    int eid1 = tok.nextInt();
    double p1 = tok.nextDouble();
    double p2 = tok.nextDouble(true, p1);
    double p3 = tok.nextDouble(true, p1);
    double p4 = tok.nextDouble(true, p1);
    if (!is_equal(p2, p1) || !is_equal(p3, p1) || !is_equal(p4, p1)) {
        handleParsingWarning("Non uniform pressure not implemented.", tok, model);
    }
    bool format1 = tok.isNextInt() || tok.isNextEmpty();
    int g1 = NastranTokenizer::UNAVAILABLE_INT;
    int g3_or_4 = NastranTokenizer::UNAVAILABLE_INT;
    int eid2 = NastranTokenizer::UNAVAILABLE_INT;
    if (format1) {
        //format 1
        g1 = tok.nextInt(true);
        g3_or_4 = tok.nextInt(true);
    } else {
        if (tok.isNextTHRU()) {
            //format2
            tok.skip(1);
            eid2 = tok.nextInt();
        } else {
            //format not recognized
            handleParsingError(string("Format not recognized."), tok, model);
        }
    }
    int cid = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (cid != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        string message = "CoordinateSystem not supported.";
        handleParsingWarning(message, tok, model);
    }
    double n1 = tok.nextDouble(true, 0.0);
    double n2 = tok.nextDouble(true, 0.0);
    double n3 = tok.nextDouble(true, 0.0);

    string sorl = tok.nextString(true, "SURF");
    if (sorl != "SURF"){
        handleParsingWarning("SORL field not supported: default (SURF) assumed.", tok, model);
    }
    string ldir = tok.nextString(true, "NORM");
    if (ldir != "NORM"){
        handleParsingWarning("LDIR field not supported: default (NORM) assumed.", tok, model);
    }

    Reference<LoadSet> loadSetReference(LoadSet::LOAD, loadset_id);
    if (is_equal(n1, 0.0) && is_equal(n2, 0.0) && is_equal(n3, 0.0) && cid == 0
            && g1 == NastranTokenizer::UNAVAILABLE_INT) {
        NormalPressionFace normalPressionFace(*model, p1);
        for(int cellId = eid1; cellId < eid2; cellId++) {
            normalPressionFace.addCell(cellId);
        }

        model->add(normalPressionFace);
        model->addLoadingIntoLoadSet(normalPressionFace, loadSetReference);
    } else if (g1 != NastranTokenizer::UNAVAILABLE_INT) {
        PressionFaceTwoNodes pressionFaceTwoNodes(*model, g1, g3_or_4,
                VectorialValue(n1 * p1, n2 * p1, n3 * p1), VectorialValue(0.0, 0.0, 0.0));
        for(int cellId = eid1; cellId < eid2; cellId++) {
            pressionFaceTwoNodes.addCell(cellId);
        }

        model->add(pressionFaceTwoNodes);
        model->addLoadingIntoLoadSet(pressionFaceTwoNodes, loadSetReference);
    } else {
        ForceSurface forceSurface(*model, VectorialValue(n1 * p1, n2 * p1, n3 * p1),
                VectorialValue(0.0, 0.0, 0.0));
        for(int cellId = eid1; cellId < eid2; cellId++) {
            forceSurface.addCell(cellId);
        }

        model->add(forceSurface);
        model->addLoadingIntoLoadSet(forceSurface, loadSetReference);
    }
    if (!model->find(loadSetReference)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }
}


void NastranParser::parsePROD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int propId = tok.nextInt();
    int material_id = tok.nextInt();
    double a = tok.nextDouble();
    double j = tok.nextDouble(true, 0.0);
    double c = tok.nextDouble(true, 0.0);
    double nsm = tok.nextDouble(true, 0.0);
    if (!is_equal(c, 0)) {
        handleParsingWarning("Stress coefficient (C) not supported and dismissed.", tok, model);
    }
    if (!is_equal(nsm, 0)) {
        handleParsingWarning("Non Structural mass (NSM) not supported and dismissed.", tok, model);
    }
    GenericSectionBeam genericSectionBeam(*model, a, 0, 0, j, 0, 0, GenericSectionBeam::EULER, nsm,
            propId);
    genericSectionBeam.assignMaterial(material_id);
    genericSectionBeam.assignCellGroup(getOrCreateCellGroup(propId, model, "PROD"));
    model->add(genericSectionBeam);
}

void NastranParser::parsePSHELL(NastranTokenizer& tok, shared_ptr<Model> model) {
    int propId = tok.nextInt();
    int material_id1 = tok.nextInt();
    double thickness = tok.nextDouble(true, 0.0);
    int material_id2 = tok.nextInt(true);
    if (material_id2 != NastranTokenizer::UNAVAILABLE_INT && material_id2 != material_id1) {
        handleParsingWarning("Material 2 not yet supported and dismissed.", tok, model);
    }
    double bending_moment = tok.nextDouble(true, 1.0);
    if (!is_equal(bending_moment, 1.0)) {
        handleParsingWarning("Bending moment of inertia ratio != 1.0 not supported", tok,
                model);
    }
    int material_id3 = tok.nextInt(true);
    if (material_id3 != NastranTokenizer::UNAVAILABLE_INT && material_id3 != material_id1) {
        handleParsingWarning("Material 3 not yet supported and dismissed.", tok, model);
    }
    double ts_t_ratio = tok.nextDouble(true, 0.833333);
    if (!is_equal(ts_t_ratio, 0.833333)) {
        handleParsingWarning("ts/t ratio !=  0.833333 not supported", tok, model);
    }
    double nsm = tok.nextDouble(true, 0);
    double z1 = tok.nextDouble(true);
    double z2 = tok.nextDouble(true);
    if (!is_equal(z1, NastranTokenizer::UNAVAILABLE_DOUBLE)
            || !is_equal(z2, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
        handleParsingWarning("Fiber distances z1,z2 not supported", tok, model);
    }
    int material_id4 = tok.nextInt(true);
    if (material_id4 != NastranTokenizer::UNAVAILABLE_INT && material_id4 != material_id1) {
        handleParsingWarning("Material 4 not yet supported and dismissed.", tok, model);
    }

    Shell shell(*model, thickness, nsm, propId);
    shell.assignMaterial(material_id1);
    shell.assignCellGroup(getOrCreateCellGroup(propId, model,"PSHELL"));
    model->add(shell);
}

void NastranParser::parsePSOLID(NastranTokenizer& tok, shared_ptr<Model> model) {
    int elemId = tok.nextInt();
    int material_id = tok.nextInt();
    int material_coordinate_system = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (material_coordinate_system != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        handleParsingWarning("Material coordinate system!=0 not supported and dismissed.", tok, model);
    }
    string psolid_in = tok.nextString(true, "BUBBLE");
    /*
     * Location selection for stress output.
     * Stress output may be requested at the Gauss points (STRESS = “GAUSS” or
     * 1) of CHEXA and CPENTA elements with no midside nodes. Gauss point
     * output is available for the CTETRA element with or without midside nodes.
     *
     */
    string stress = tok.nextString(true, "GRID");
    if (stress != "GRID") {
        handleParsingWarning("STRESS field " + stress + " not supported", tok, model);
    }
    string isop = tok.nextString(true, "REDUCED");
    const ModelType * modelType;
    if (psolid_in == "BUBBLE" && isop == "REDUCED") {
        modelType = &ModelType::TRIDIMENSIONAL_SI;
    } else if ((psolid_in == "TWO" || psolid_in == "2") && (isop == "FULL" || isop == "1")) {
        modelType = &ModelType::TRIDIMENSIONAL;
    } else {
        modelType = &ModelType::TRIDIMENSIONAL_SI;
        handleParsingWarning("Integration IN " + psolid_in + " ISOP " + isop + " not implemented : default assumed",
                tok, model);
    }
    string fctn = tok.nextString(true, "SMECH");
    if (fctn != "SMECH") {
        handleParsingWarning("PSOLID fctn " + fctn + " Not implemented", tok, model);
    }
    Continuum continuum(*model, modelType, elemId);
    continuum.assignMaterial(material_id);
    continuum.assignCellGroup(getOrCreateCellGroup(elemId, model, "PSOLID"));
    model->add(continuum);
}

void NastranParser::parseRBAR(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id=tok.nextInt();
    int ga = tok.nextInt();
    int gb = tok.nextInt();
    int cna = tok.nextInt(true, 0);
    int cnb = tok.nextInt(true, 0);
    int cma = tok.nextInt(true, 0);
    int cmb = tok.nextInt(true, 0);
    if (cna != 0 && cnb != 0) {
        handleParsingError("cna & cnb both specified.", tok, model);
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
        handleParsingWarning("cma or cmb not supported.", tok, model);
    }
    double alpha = tok.nextDouble(true);
    if (!is_equal(alpha, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
        handleParsingWarning("ALPHA field: " + lexical_cast<string>(alpha) + " not supported",
                tok, model);
    }

}

void NastranParser::parseRBAR1(NastranTokenizer& tok, shared_ptr<Model> model) {
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

void NastranParser::parseRBE2(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    int masterId = tok.nextInt();
    int dofs = tok.nextInt();
    if (dofs != 123456) {
        handleParsingWarning("QuasiRigid constraint not supported.",tok, model);
    }

    RigidConstraint qrc(*model, masterId, original_id);
    while (tok.isNextInt()) {
        qrc.addSlave(tok.nextInt());
    }
    model->add(qrc);
    model->addConstraintIntoConstraintSet(qrc, model->commonConstraintSet);

    double alpha = tok.nextDouble(true);
    if (!is_equal(alpha, NastranTokenizer::UNAVAILABLE_DOUBLE)) {
        handleParsingWarning("ALPHA field: " + lexical_cast<string>(alpha) + " not supported",
                tok, model);
    }
}

void NastranParser::parseRBE3(NastranTokenizer& tok, shared_ptr<Model> model) {
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

void NastranParser::parseRFORCE(NastranTokenizer& tok, shared_ptr<Model> model) {
    // RFORCE  2       1               200.    0.0     0.0     1.0     2
    int sid = tok.nextInt();
    int g = tok.nextInt();
    int coordinate_system_id = tok.nextInt(true, CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID);
    if (coordinate_system_id != CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID) {
        coordinate_system_id = CoordinateSystem::GLOBAL_COORDINATE_SYSTEM_ID;
        string message = "CoordinateSystem not supported and taken as 0.";
        handleParsingWarning(message, tok, model);
    }
    double a = tok.nextDouble();
    double r1 = tok.nextDouble();
    double r2 = tok.nextDouble();
    double r3 = tok.nextDouble();

    int method = tok.nextInt(true, 1);
    if (method != 1) {
        string message = "METHOD not supported. Default (1) assumed.";
        handleParsingWarning(message, tok, model);
    }

    double racc = tok.nextDouble(true, 0);
    if (!is_equal(racc, 0.0)) {
        string message = "Scale factor of angular acceleration (RACC) not supported. Default (0.0) assumed.";
        handleParsingWarning(message, tok, model);
    }

    int mb = tok.nextInt(true, 0);
    if (mb != 0) {
        string message = "MB not supported. Default (0) assumed.";
        handleParsingWarning(message, tok, model);
    }

    RotationNode rotation(*model, a, g, r1, r2, r3);

    model->add(rotation);
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, sid);
    model->addLoadingIntoLoadSet(rotation, loadset_ref);
    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, sid);
        model->add(loadSet);
    }

}

void NastranParser::parseRLOAD2(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    int darea_set_id = tok.nextInt();

    // Delay
    int delay_id = 0;
    double delay = 0.0;
    if (tok.isNextInt())
        delay_id = tok.nextInt(true, 0);
    else
        delay = tok.nextDouble(true, 0.0);

    // DPhase
    int dphase_id = 0;
    double dphase = 0.0;
    if (tok.isNextInt())
        dphase_id = tok.nextInt(true, 0);
    else
        dphase = tok.nextDouble(true, 0.0);

    int functionTableB_original_id = tok.nextInt(); //TB
    int functionTableP_original_id = tok.nextInt(true, 0); //TP

    // Type
    string type = tok.nextString(true, "LOAD").substr(0, 1);
    if (type != "L" && type != "0")
        handleParsingError("TYPE in RLOAD2 not supported. ", tok, model);

    Reference<NamedValue> dynaDelay_ref = Reference<NamedValue>(NamedValue::DYNA_PHASE, delay_id);
    if (delay_id == 0) {
        DynaPhase dynadelay(*model, -2*M_PI*delay);
        model->add(dynadelay);
        dynaDelay_ref = Reference<NamedValue>(dynadelay);
    }
    Reference<NamedValue> dynaPhase_ref = Reference<NamedValue>(NamedValue::DYNA_PHASE, dphase_id);
    if (dphase_id == 0) {
        DynaPhase dynaphase(*model, dphase);
        model->add(dynaphase);
        dynaPhase_ref = Reference<NamedValue>(dynaphase);
    }
    Reference<NamedValue> functionTableB_ref(NamedValue::FUNCTION_TABLE, functionTableB_original_id);
    //TODO: should not create a ref when the P value is 0
    Reference<NamedValue> functionTableP_ref(NamedValue::FUNCTION_TABLE, functionTableP_original_id);



    // If needed, creates a LoadSet EXCITEID for the DynamicExcitation
    LoadSet darea(*model, LoadSet::EXCITEID, darea_set_id);
    Reference<LoadSet> darea_ref(darea);
    if (!model->find(darea_ref)){
       model->add(darea);
    }

    // if loadSet DLOAD does not exist (was not declared in the bulk), loadset_id become the original id of DynamicExcitation
    // else DynamicExcitation is created without original_id and is mapped to this loadSet
    int original_id;
    Reference<LoadSet> loadSetReference(LoadSet::DLOAD, loadset_id);
    if (model->find(loadSetReference))
        original_id = Loading::NO_ORIGINAL_ID;
    else
        original_id = loadset_id;

    DynamicExcitation dynamicExcitation(*model, dynaDelay_ref, dynaPhase_ref, functionTableB_ref, functionTableP_ref, darea_ref,
            original_id);
    model->add(dynamicExcitation);

    model->addLoadingIntoLoadSet(dynamicExcitation, loadSetReference);
    if (!model->find(loadSetReference)) {
        LoadSet loadSet(*model, LoadSet::DLOAD, loadset_id);
        model->add(loadSet);
    }

    // PlaceHolder to complete the Value attribute paraX of FunctionTable
    model->add(dynamicExcitation.getFunctionTableBPlaceHolder());
    if (functionTableP_original_id>0){
        model->add(dynamicExcitation.getFunctionTablePPlaceHolder());
    }
}

void NastranParser::parseSET3(NastranTokenizer& tok, shared_ptr<Model> model) {
    // page 2457 Labeled Set Definition, defines a list of grids, elements or points.
    int sid = tok.nextInt();
    string name = string("SET") + "_" + to_string(sid);
    string des = tok.nextString();

    if (des == "GRID") {
        shared_ptr<NodeGroup> nodeGroup = model->mesh->findOrCreateNodeGroup(name,sid,"SET");
        while (tok.isNextInt()) {
            nodeGroup->addNodeId(tok.nextInt());
        }
    } else if (des == "ELEM") {
        shared_ptr<CellGroup> cellGroup = model->mesh->createCellGroup(name,sid,"SET");
        while (tok.isNextInt()) {
            cellGroup->addCellId(tok.nextInt());
        }
    } else {
        throw logic_error("Unsupported DES value in SET3");
    }

}

//FIXME: SLOAD uses the CID of the Grid point to determine X... Not sure it's done here.
// The "scalar" DOF is supposed to be DOF::DX
void NastranParser::parseSLOAD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int loadset_id = tok.nextInt();
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, loadset_id);

    while (tok.isNextInt()) {
        int grid_id = tok.nextInt();
        double magnitude = tok.nextDouble();
        NodalForce force1(*model, grid_id, magnitude, 0.0, 0.0, 0., 0., 0., Loading::NO_ORIGINAL_ID);
        model->add(force1);
        model->addLoadingIntoLoadSet(force1, loadset_ref);
    }

    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, loadset_id);
        model->add(loadSet);
    }

}
void NastranParser::parseSPC(NastranTokenizer& tok, shared_ptr<Model> model) {
    int spcSet_id = tok.nextInt();
    string name = string("SPC") + "_" + to_string(spcSet_id);
    shared_ptr<NodeGroup> spcNodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SPC");

    while (tok.nextSymbolType == NastranTokenizer::SYMBOL_FIELD) {
        const int nodeId = tok.nextInt(true);
        if (nodeId == NastranTokenizer::UNAVAILABLE_INT) {
            //space at the end of line found,consume it.
            continue;
        }
        const int gi = tok.nextInt(true, 123456);
        const double displacement = tok.nextDouble(true, 0.0);
        SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(gi), displacement);
        spc.addNodeId(nodeId);
        spcNodeGroup->addNodeId(nodeId);

        model->add(spc);
        model->addConstraintIntoConstraintSet(spc,
                Reference<ConstraintSet>(ConstraintSet::SPC, spcSet_id));
    }
}

void NastranParser::parseSPC1(NastranTokenizer& tok, shared_ptr<Model> model) {
    int set_id = tok.nextInt();
    const int dofInt = tok.nextInt();

    // We create a constraint
    SinglePointConstraint spc = SinglePointConstraint(*model, DOFS::nastranCodeToDOFS(dofInt), 0.0);

    // Nodes are added to the constraint Node Group
    string name = string("SPC1") + "_" + to_string(set_id);
    shared_ptr<NodeGroup> spcNodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SPC1");

    // Parsing Nodes
    for(int gridId : tok.nextInts()) {
        spcNodeGroup->addNodeId(gridId);
        spc.addNodeId(gridId);
    }

    // Adding the constraint to the model
    model->add(spc);
    model->addConstraintIntoConstraintSet(spc,
            Reference<ConstraintSet>(ConstraintSet::SPC, set_id));

}

void NastranParser::parseSPCADD(NastranTokenizer& tok, shared_ptr<Model> model) {
    int set_id = tok.nextInt();
    // retrieve the ConstraintSet that was created in the executive section
    shared_ptr<ConstraintSet> constraintSet_ptr = model->find(
            Reference<ConstraintSet>(ConstraintSet::SPC, set_id));

    if (!constraintSet_ptr)
        handleParsingError("ConstraintSet "+ to_string(set_id)+ " does not exist in Executive section",
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

void NastranParser::parseSPCD(NastranTokenizer& tok, shared_ptr<Model> model) {
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

void NastranParser::parseTABDMP1(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    string type = tok.nextString(true, "G");
    if (type != "CRIT"){
        handleParsingError("Only CRIT type supported.", tok, model);
    }
    vega::FunctionTable functionTable(*model, FunctionTable::LINEAR, FunctionTable::LINEAR,
            FunctionTable::NONE, FunctionTable::CONSTANT);

    // The next 6 fields are empty.
    // We do a special treatment for the second one, which can be filled in the OPTISTRUCT syntax
    tok.skip(1);
    int flat = tok.nextInt(true, 0);
    if (flat != 0){
        handleParsingWarning("FLAT field is dismissed (OPTISTRUCT syntax)", tok, model);
    }
    tok.skip(4);

    double x,y;
    string sField="";
    while (sField != "ENDT") {
        if (tok.isNextDouble()){
            x = tok.nextDouble();
        }else{
            sField=tok.nextString();
            if (sField=="ENDT"){
                continue;
            }
            if (sField=="SKIP"){ // SKIP means the pair (x,y) is skipped
                tok.skip(1);
                continue;
            }else{
                handleParsingWarning("Invalid key ("+sField+") should be ENDT, SKIP or a real.", tok, model);
            }
        }
        if (tok.isNextDouble()){
            y = tok.nextDouble();
        }else{
            string sField=tok.nextString(); //Code_Aster convention : Nastran is coherent with factor 2 for sdamping : not so sure
            if (sField=="SKIP"){
                continue;
            }else{
                handleParsingWarning("Invalid key ("+sField+") should be SKIP or a real.", tok, model);
            }
        }
        functionTable.setXY(x, y);
    }

    functionTable.setParaX(NamedValue::FREQ);
    functionTable.setParaY(NamedValue::AMOR);
    vega::ModalDamping modalDamping(*model, functionTable, original_id);

    model->add(functionTable);
    model->add(modalDamping);
}

void NastranParser::parseTABLED1(NastranTokenizer& tok, shared_ptr<Model> model) {
    int original_id = tok.nextInt();
    string interpolation = tok.nextString(true, "LINEAR");
    FunctionTable::Interpolation parameter =
            (interpolation == "LINEAR") ? FunctionTable::LINEAR : FunctionTable::LOGARITHMIC;
    interpolation = tok.nextString(true, "LINEAR");
    FunctionTable::Interpolation value =
            (interpolation == "LINEAR") ? FunctionTable::LINEAR : FunctionTable::LOGARITHMIC;

    vega::FunctionTable functionTable(*model, parameter, value, FunctionTable::NONE,
            FunctionTable::NONE, original_id);

    // The next 5 fields are empty.
    // We do a special treatment for the first one, which can be filled in the OPTISTRUCT syntax
    int flat = tok.nextInt(true, 0);
    if (flat != 0){
        handleParsingWarning("FLAT field is dismissed (OPTISTRUCT syntax)", tok, model);
    }
    tok.skip(4);

    double x,y;
    string sField="";
    while (sField != "ENDT") {
        if (tok.isNextDouble()){
            x = tok.nextDouble();
        }else{
            sField=tok.nextString();
            if (sField=="ENDT"){
                continue;
            }
            if (sField=="SKIP"){ // SKIP means the pair (x,y) is skipped
                tok.skip(1);
                continue;
            }else{
                handleParsingWarning("Invalid key ("+sField+") should be ENDT, SKIP or a real.", tok, model);
            }
        }
        if (tok.isNextDouble()){
            y = tok.nextDouble();
        }else{
            string sField=tok.nextString(); //Code_Aster convention : Nastran is coherent with factor 2 for sdamping : not so sure
            if (sField=="SKIP"){
                continue;
            }else{
                handleParsingWarning("Invalid key ("+sField+") should be SKIP or a real.", tok, model);
            }
        }
        functionTable.setXY(x, y);
    }

    model->add(functionTable);

}

void NastranParser::parseTEMP(NastranTokenizer& tok, shared_ptr<Model> model) {
    int sid = tok.nextInt();
    Reference<vega::LoadSet> loadset_ref(LoadSet::LOAD, sid);
    int g1 = tok.nextInt();
    double t1 = tok.nextDouble();
    InitialTemperature temp1(*model, t1);
    temp1.addNode(g1);
    model->add(temp1);
    model->addLoadingIntoLoadSet(temp1, loadset_ref);

    if (tok.isNextInt()) {
        int g2 = tok.nextInt();
        double t2 = tok.nextDouble();
        InitialTemperature temp2(*model, t2);
        temp2.addNode(g2);
        model->add(temp2);
        model->addLoadingIntoLoadSet(temp2, loadset_ref);
    }

    if (tok.isNextInt()) {
        int g3 = tok.nextInt();
        double t3 = tok.nextDouble();
        InitialTemperature temp3(*model, t3);
        temp3.addNode(g3);
        model->add(temp3);
        model->addLoadingIntoLoadSet(temp3, loadset_ref);
    }

    if (!model->find(loadset_ref)) {
        LoadSet loadSet(*model, LoadSet::LOAD, sid);
        model->add(loadSet);
    }
}


int NastranParser::parseDOF(NastranTokenizer& tok, shared_ptr<Model> model){

    int dofread = tok.nextInt();

    // Check for errors
    if ((dofread<0) || (dofread>6)){
        string message = "Out of bound degrees of freedom : " + std::to_string(dofread);
        handleParsingWarning(message, tok, model);
    }
    // Scalar point have a "0" Nastran DOF, translated as a "0" (DX) Vega DOF
    if (dofread==0){
        return 0;
    }
    // Nastran dofs goes from 1 to 6, VEGA from 0 to 5.
    return dofread-1;

}



} //namespace nastran

} //namespace vega
