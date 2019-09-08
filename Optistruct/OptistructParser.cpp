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
#include <iostream>
#include <string>
#include <vector>
#include <ciso646>

namespace vega {

namespace optistruct {

using namespace std;

const unordered_map<string, OptistructParser::parseOptistructElementFPtr> OptistructParser::OPTISTRUCT_PARSE_FUNCTION_BY_KEYWORD =
        {
                { "CONTACT", &OptistructParser::parseCONTACT },
                { "SET", &OptistructParser::parseSET },
                { "SURF", &OptistructParser::parseSURF },
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

void OptistructParser::parseCONTACT(nastran::NastranTokenizer& tok, Model& model) {
    // https://www.sharcnet.ca/Software/Hyperworks/help/hwsolvers/contact_bulk.htm
    int ctid = tok.nextInt();
    string type = tok.nextString();
    int ssid = tok.nextInt();
    int msid = tok.nextInt();

    Reference<ConstraintSet> constraintSetReference(ConstraintSet::Type::CONTACT, ctid);
    if (!model.find(constraintSetReference)) {
        const auto& constraintSet = make_shared<ConstraintSet>(model, ConstraintSet::Type::CONTACT, ctid);
        model.add(constraintSet);
    }
    const auto& surface = make_shared<SurfaceSlide>(model, Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, msid), Reference<Target>(Target::Type::BOUNDARY_ELEMENTFACE, ssid));
    model.add(surface);
    //model.addConstraintIntoConstraintSet(surface, constraintSetReference);
    model.addConstraintIntoConstraintSet(*surface, *model.commonConstraintSet);
}

void OptistructParser::parseSET(nastran::NastranTokenizer& tok, Model& model) {
    // https://www.sharcnet.ca/Software/Hyperworks/help/hwsolvers/set_bulk_data.htm
    // https://knowledge.autodesk.com/support/nastran/learn-explore/caas/CloudHelp/cloudhelp/2019/ENU/NSTRN-Reference/files/GUID-B2CE1526-DEDE-4694-B944-83880E9047A1-htm.html
    int sid = tok.nextInt();
    string name = "SET_" + to_string(sid);
    string type = tok.nextString();
    string subtype = tok.nextString(true, "LIST");
    tok.skipToNotEmpty();

    if (subtype != "LIST") {
        handleParsingError("Unsupported SUBTYPE value in SET", tok, model);
    }

    if (type == "GRID") {
        const auto& ids = tok.nextInts();
        const auto& setValue = make_shared<SetValue<int>>(model, set<int>{ids.begin(), ids.end()}, sid);
        model.add(setValue);
        shared_ptr<NodeGroup> nodeGroup = model.mesh.findOrCreateNodeGroup(name,NodeGroup::NO_ORIGINAL_ID,"SET");
        for (const auto& id : ids) {
            nodeGroup->addNodeId(id);
        }
        tok.skipToNotEmpty();
        setValue->markAsWritten();
        tok.skipToNextKeyword();
    } else if (type == "ELEM") {
        const auto& ids = tok.nextInts();
        const auto& setValue = make_shared<SetValue<int>>(model, set<int>{ids.begin(), ids.end()}, sid);
        model.add(setValue);
        shared_ptr<CellGroup> cellGroup = model.mesh.createCellGroup(name,CellGroup::NO_ORIGINAL_ID,"SET");
        for (const auto& id : tok.nextInts()) {
            cellGroup->addCellId(id);
        }
        tok.skipToNotEmpty();
        setValue->markAsWritten();
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
        const auto& setValue = make_shared<SetValue<double>>(model, set<double>{values.begin(), values.end()}, sid);
        model.add(setValue);
        setValue->markAsWritten();
        const auto& frequencyValue = make_shared<ListValue<double>>(model, values);
        model.add(frequencyValue);
        const auto& frequencyRange = make_shared<FrequencySearch>(model, FrequencySearch::FrequencyType::LIST, *frequencyValue, FrequencySearch::NormType::MASS, sid);
        model.add(frequencyRange);
    } else {
        handleParsingError("Unsupported TYPE value in SET", tok, model);
    }

}

void OptistructParser::parseSURF(nastran::NastranTokenizer& tok, Model& model) {
    // https://www.sharcnet.ca/Software/Hyperworks/help/hwsolvers/hwsolvers.htm?surf.htm
    int sid = tok.nextInt();
    if (not tok.isNextInt()) {
        tok.skip(1); // To avoid strange ELFACE text found in one case
    }
    list<BoundaryElementFace::ElementFaceByTwoNodes> faceInfos;
    while (! tok.isEmptyUntilNextKeyword()){
        tok.skipToNotEmpty();
        int eid = tok.nextInt();
        int ga1 = tok.nextInt(true, Globals::UNAVAILABLE_INT);
        int ga2 = tok.nextInt(true, Globals::UNAVAILABLE_INT);
        bool swapNormal = tok.nextInt(true, 0) == 1;
        BoundaryElementFace::ElementFaceByTwoNodes faceInfo(eid, ga1, ga2, swapNormal);
        faceInfos.push_back(faceInfo);
    }
    const auto& bef = make_shared<BoundaryElementFace>(model, faceInfos, sid);
    model.add(bef);
}

} //namespace optistruct

} //namespace vega
