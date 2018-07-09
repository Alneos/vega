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

OptistructParser::OptistructParser() :
        nastran::NastranParser() {
    nastran::NastranParser::IGNORED_KEYWORDS.insert(OPTISTRUCT_IGNORED_KEYWORDS.begin(), OPTISTRUCT_IGNORED_KEYWORDS.end());
}

nastran::NastranParser::parseElementFPtr OptistructParser::findCmdParser(string keyword) const {
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

void OptistructParser::parseSET(NastranTokenizer& tok, shared_ptr<Model> model) {
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
    } else if (des == "FREQ") {
        list<double> values;
        while (tok.isNextDouble()) {
            values.push_back(tok.nextDouble());
        }
        FrequencyList frequencyValues(*model, values, sid);
        model->add(frequencyValues);
    } else {
        throw logic_error("Unsupported DES value in SET3");
    }

}

} //namespace optistruct

} //namespace vega
