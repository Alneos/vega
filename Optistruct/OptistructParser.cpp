/*
 * Copyright (C) IRT Systemx (luca.dallolio@ext.irt-systemx.fr)
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
 *  Created on: Jul 5, 2018
 *      Author: Luca Dall'Olio
 */

#include "OptistructParser.h"
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

namespace optistruct {

using namespace std;

const unordered_map<string, OptistructParser::parseOptistructElementFPtr> OptistructParser::OPTISTRUCT_PARSE_FUNCTION_BY_KEYWORD =
        {
                { "SET", &OptistructParser::parseSET },
        };

const unordered_map<string, OptistructParser::parseOptistructElementFPtr> OptistructParser::OPTISTRUCT_PARSEPARAM_FUNCTION_BY_KEYWORD =
        {
        };

OptistructParser::OptistructParser() :
        nastran::NastranParser() {
    nastran::NastranParser::IGNORED_KEYWORDS.insert(OPTISTRUCT_IGNORED_KEYWORDS.begin(), OPTISTRUCT_IGNORED_KEYWORDS.end());
    nastran::NastranParser::IGNORED_PARAMS.insert(OPTISTRUCT_IGNORED_PARAMS.begin(), OPTISTRUCT_IGNORED_PARAMS.end());
}

nastran::NastranParser::parseElementFPtr OptistructParser::findCmdParser(const string keyword) const {
    auto optistructParser = OPTISTRUCT_PARSE_FUNCTION_BY_KEYWORD.find(keyword);
    auto nastranParser = nastran::NastranParser::findCmdParser(keyword);
    if (optistructParser != OPTISTRUCT_PARSE_FUNCTION_BY_KEYWORD.end()) {
        return static_cast<nastran::NastranParser::parseElementFPtr>(optistructParser->second);
    } else if (nastranParser != nullptr) {
        return nastranParser;
    } else {
        return nullptr;
    }
}

nastran::NastranParser::parseElementFPtr OptistructParser::findParamParser(const string param) const {
    auto optistructParser = OPTISTRUCT_PARSEPARAM_FUNCTION_BY_KEYWORD.find(param);
    auto nastranParser = nastran::NastranParser::findParamParser(param);
    if (optistructParser != OPTISTRUCT_PARSEPARAM_FUNCTION_BY_KEYWORD.end()) {
        return static_cast<nastran::NastranParser::parseElementFPtr>(optistructParser->second);
    } else if (nastranParser != nullptr) {
        return nastranParser;
    } else {
        return nullptr;
    }
}

string OptistructParser::defaultAnalysis() const {
    return "200";
}

void OptistructParser::parseSET(nastran::NastranTokenizer& tok, shared_ptr<Model> model) {
    // https://www.sharcnet.ca/Software/Hyperworks/help/hwsolvers/set_bulk_data.htm
    int sid = tok.nextInt();
    string name = string("SET") + "_" + to_string(sid);
    string type = tok.nextString();
    string subtype = tok.nextString(true, "LIST");
    tok.skipToNotEmpty();

    if (subtype != "LIST") {
        throw logic_error("Unsupported SUBTYPE value in SET");
    }

    if (type == "GRID") {
        shared_ptr<NodeGroup> nodeGroup = model->mesh->findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SET");
        while(!tok.isEmptyUntilNextKeyword()) {
            for (auto& id : tok.nextInts()) {
                nodeGroup->addNodeId(id);
            }
            tok.skipToNotEmpty();
        }
        tok.skipToNextKeyword();
    } else if (type == "ELEM") {
        shared_ptr<CellGroup> cellGroup = model->mesh->createCellGroup(name,CellGroup::NO_ORIGINAL_ID,"SET");
        while(!tok.isEmptyUntilNextKeyword()) {
            for (auto& id : tok.nextInts()) {
                cellGroup->addCellId(id);
            }
            tok.skipToNotEmpty();
        }
        tok.skipToNextKeyword();
    } else if (type == "FREQ") {
        list<double> values;
        while(!tok.isEmptyUntilNextKeyword()) {
            while (tok.isNextDouble()) {
                values.push_back(tok.nextDouble());
            }
            tok.skipToNotEmpty();
        }
        tok.skipToNextKeyword();
        ListValue frequencyValue(*model, values);
        model->add(frequencyValue);
        FrequencyTarget frequencyRange(*model, FrequencyTarget::LIST, frequencyValue, FrequencyTarget::MASS, sid);
        model->add(frequencyRange);
    } else {
        throw logic_error("Unsupported TYPE value in SET");
    }

}

} //namespace optistruct

} //namespace vega
