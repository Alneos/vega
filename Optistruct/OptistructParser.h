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
 * OptistructParser.h
 *
 *  Created on: Jul 5, 2018
 *      Author: Luca Dall'Olio
 */

#ifndef OPTISTRUCTPARSER_H_
#define OPTISTRUCTPARSER_H_

#include <boost/filesystem.hpp>
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "../Nastran/NastranParser.h"

namespace vega {

namespace optistruct {

class OptistructParser: public nastran::NastranParser {
private:
    typedef void (OptistructParser::*parseOptistructElementFPtr)(NastranTokenizer& tok, std::shared_ptr<Model> model);

    /**
     * Parse the SET keyword
     */
    void parseSET(NastranTokenizer& tok, std::shared_ptr<Model> model);

    const std::set<std::string> OPTISTRUCT_IGNORED_KEYWORDS = {
        //optistruct optimization variable
        "DOPTPRM", "DCOMP", //Manufacturing constraints for composite sizing optimization.
        "DESVAR", //Design variable definition.
        "DSHAPE", //Free-shape design variable definition.
        "DSHUFFLE", //Parameters for the generation of composite shuffling design variables.
        "DSIZE", "DTPG", //Topography design variable definition.
        "DTPL", //Topology design variable definition.
        "DVGRID", "DEQATN",
        "DREPORT", "DREPADD", // Optistruct Cards
    };

    static const std::unordered_map<std::string, parseOptistructElementFPtr> OPTISTRUCT_PARSE_FUNCTION_BY_KEYWORD;
protected:
    parseElementFPtr findParser(std::string keyword) const override;
public:
    OptistructParser();
    //std::shared_ptr<Model> parse(const ConfigurationParameters& configuration) override;
    virtual ~OptistructParser() {
	}
};

} /* namespace optistruct */
} /* namespace vega */
#endif /* OPTISTRUCTPARSER_H_ */
