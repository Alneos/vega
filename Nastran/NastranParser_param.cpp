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
 * NastranParser_param.cpp
 *
 *  Created on: 2017/03/08
 *      Author: Abballe Thomas
 *     Comment: Created to host the parseParam function. As there are like,
 *              50k parameters in Nastran, it will sooner or later take too
 *              much place in the NastranParser.cpp file :/
 */


#include "NastranParser.h"
#include <boost/algorithm/string.hpp>

using namespace std;

namespace vega {

namespace nastran {

const unordered_map<string, NastranParser::parseElementFPtr> NastranParser::PARSEPARAM_FUNCTION_BY_KEYWORD =
        {
                { "AUTOSPC", &NastranParser::parseParamAUTOSPC },
                { "G", &NastranParser::parseParamG },
                { "GRDPNT", &NastranParser::parseParamGRDPNT },
                { "HFREQ", &NastranParser::parseParamHFREQ },
                { "K6ROT", &NastranParser::parseParamK6ROT },
                { "LFREQ", &NastranParser::parseParamLFREQ },
                { "LGDISP", &NastranParser::parseParamLGDISP },
                { "NOCOMPS", &NastranParser::parseParamNOCOMPS },
                { "PATVER", &NastranParser::parseParamPATVER },
                { "PRTMAXIM", &NastranParser::parseParamPRTMAXIM },
                { "W3", &NastranParser::parseParamW3 },
                { "WTMASS", &NastranParser::parseParamWTMASS },
        };

void NastranParser::parseParamAUTOSPC(NastranTokenizer& tok, Model& model) {

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
        handleParsingError("unsupported AUTOSPC value in parseParamAUTOSPC. ",
                tok, model);
    }
}

void NastranParser::parseParamGRDPNT(NastranTokenizer& tok, Model& model) {
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
    if (value!=-1){
        handleParsingWarning("Ignored parameter GRDPNT value " + to_string(value), tok, model);
    }
}

void NastranParser::parseParamHFREQ(NastranTokenizer& tok, Model& model) {

    /* Default = 1.+30
     PARAM,HFREQ gives the upper limit on the frequency range of retained modes.
     */
    double val = tok.nextDouble(true, 1e30);
    if (!is_equal(val, 1e30)) {
        model.parameters[Model::Parameter::UPPER_CUTOFF_FREQUENCY] = val;
    }
}

void NastranParser::parseParamK6ROT(NastranTokenizer& tok, Model& model) {
    /* K6ROT specifies the scaling factor of the penalty stiffness to be added
     to the normal rotation for CQUAD4 and CTRIA3 elements. The
     contribution of the penalty term to the strain energy functional is ...*/
    double val = tok.nextDouble(true, 100.0);
    if (not is_zero(val)) {
        handleParsingWarning(
                "Non zero parameter K6ROT : " + to_string(val),
                tok, model);
    }
}

void NastranParser::parseParamLFREQ(NastranTokenizer& tok, Model& model) {
    double val = tok.nextDouble(true, 0.0);
    if (!is_equal(val, 0.0)) {
        model.parameters[Model::Parameter::LOWER_CUTOFF_FREQUENCY] = val;
    }
}

void NastranParser::parseParamG(NastranTokenizer& tok, Model& model) {
    double val = tok.nextDouble(true, 0.0);
    if (!is_equal(val, 0.0)) {
        model.parameters[Model::Parameter::STRUCTURAL_DAMPING] = val;
    }
}

void NastranParser::parseParamW3(NastranTokenizer& tok, Model& model) {
    double val = tok.nextDouble(true, 0.0);
    if (!is_equal(val, 0.0)) {
        model.parameters[Model::Parameter::FREQUENCY_OF_INTEREST_RADIANS] = val;
    }
}

void NastranParser::parseParamLGDISP(NastranTokenizer& tok, Model& model) {
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
        model.parameters[Model::Parameter::LARGE_DISPLACEMENTS] = val;
    }
}

void NastranParser::parseParamNOCOMPS(NastranTokenizer& tok, Model& model) {
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
    if (not is_equal(value, -1) and not is_zero(value)) {
        handleParsingWarning("Ignored parameter NOCOMPS value " + to_string(value), tok, model);
    }
}

void NastranParser::parseParamPATVER(NastranTokenizer& tok, Model& model) {
    double val = tok.nextDouble(true, 3.0);
    if (!is_equal(val, 3.0)) {
        handleParsingError("unsupported parameter PATVER value in parseParamPATVER. ",
                tok, model);
    }
}

void NastranParser::parseParamPRTMAXIM(NastranTokenizer& tok, Model& model) {
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
        model.parameters[Model::Parameter::PRINT_MAXIM] = 1.0;
    }
}

void NastranParser::parseParamWTMASS(NastranTokenizer& tok, Model& model) {
    double value = tok.nextDouble(true, 1);
    model.parameters[Model::Parameter::MASS_OVER_FORCE_MULTIPLIER] = value;
}

NastranParser::parseElementFPtr NastranParser::findParamParser(const string keyword) const {
    auto result = PARSEPARAM_FUNCTION_BY_KEYWORD.find(keyword);
    if (result != PARSEPARAM_FUNCTION_BY_KEYWORD.end()) {
        return result->second;
    } else {
        return nullptr;
    }
}

void NastranParser::parsePARAM(NastranTokenizer& tok, Model& model) {
    string param = tok.nextString();
    boost::to_upper(param);

    auto parser = findParamParser(param);
    if (parser != nullptr) {
        (this->*parser)(tok, model);

    } else if (IGNORED_PARAMS.find(param) != IGNORED_PARAMS.end()) {
        if (model.configuration.logLevel >= LogLevel::TRACE) {
            cout << "Option PARAM, " << param << " ignored." << endl;
        }
        tok.skipToNextKeyword();
    } else {
        handleParsingError("Unknown parameter " + param + " is dismissed.", tok,
                model);
    }
}




} /* namespace nastran */

} /* namespace vega */
