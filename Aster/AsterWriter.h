/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterBuilder.h
 *
 *  Created on: 5 mars 2013
 *      Author: dallolio
 */

#ifndef ASTERBUILDER_H_
#define ASTERBUILDER_H_

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "AsterModel.h"
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "../Abstract/ConfigurationParameters.h"

namespace vega {
namespace aster {

class AsterWriter final : public Writer {
    std::unique_ptr<AsterModel> asterModel = nullptr;
	std::string mail_name, sigm_noeu, sigm_elno, sief_elga;
	bool calc_sigm = false;
	std::map<Reference<NamedValue>, std::string> asternameByValue;
	std::map<Reference<LoadSet>, std::string> asternameByLoadSet;
	std::map<Reference<ConstraintSet>, std::string> asternameByConstraintSet;
	std::set<int> singleGroupCellPositions;
	static constexpr double SMALLEST_RELATIVE_COMPARISON = 1e-7;

	void writeExport(AsterModel& model, std::ostream&);
	void writeComm(const AsterModel& model, std::ostream&);
	void writeLireMaillage(const AsterModel&, std::ostream&);
	void writeAffeModele(const AsterModel&, std::ostream&);
	void writeValues(const AsterModel&, std::ostream&);
	void writeMaterials(const AsterModel&, std::ostream&);
	void writeAffeCaraElem(const AsterModel&, std::ostream&);
	void writeAffeCaraElemPoutre(const AsterModel&, const ElementSet&, std::ostream&);
	void writeAffeCharMeca(const AsterModel&, std::ostream&);
	void writeDefiContact(const AsterModel&, std::ostream&);
	void writeSPC(const AsterModel&, const ConstraintSet&, std::ostream&);
	void writeSPCD(const AsterModel&, const LoadSet&, std::ostream&);
	void writeLIAISON_SOLIDE(const AsterModel&, const ConstraintSet&, std::ostream&);
	void writeLIAISON_MAIL(const AsterModel&, const ConstraintSet&, std::ostream&);
	void writeRBE3(const AsterModel&, const ConstraintSet&, std::ostream&);
	void writeLMPC(const AsterModel&, const ConstraintSet&, std::ostream&);
	void writeGravity(const LoadSet&, std::ostream&);
	void writeRotation(const LoadSet&, std::ostream&);
	void writeNodalForce(const AsterModel&, const LoadSet&, std::ostream&);
	void writePression(const LoadSet&, std::ostream&);
	void writeForceCoque(const LoadSet&, std::ostream&);
	void writeForceLine(const LoadSet&, std::ostream&);
	void writeForceSurface(const LoadSet&, std::ostream&);
	void writeNodeContainer(const NodeContainer& nodeContainer, std::ostream&);
	void writeCellContainer(const CellContainer& cellContainer, std::ostream&);
	double writeAnalysis(const AsterModel&, Analysis& analysis, std::ostream&, double debut);
	void writeAssemblage(const AsterModel&, Analysis& analysis, std::ostream&);
	void writeCalcFreq(const AsterModel&, LinearModal& analysis, std::ostream&);
	void writeNodalDisplacementAssertion(const AsterModel&, const Assertion&, std::ostream&) const;
	void writeNodalComplexDisplacementAssertion(const AsterModel&, const Assertion&, std::ostream&) const;
	void writeFrequencyAssertion(const Analysis&, const Assertion&, std::ostream&) const;
	void writeLoadset(LoadSet& loadSet, std::ostream& out);
	std::string writeValue(NamedValue& value, std::ostream& out);
	void writeImprResultats(const AsterModel& asterModel, std::ostream& out);
	void list_concept_name(StepRange& stepRange);
	std::shared_ptr<NonLinearStrategy> getNonLinearStrategy(NonLinearMecaStat& nonLinAnalysis);
	void writeAnalyses(const AsterModel& asterModel, std::ostream& out);

public:
    AsterWriter() = default;
	AsterWriter(const AsterWriter& that) = delete;
	std::string writeModel(Model&, const ConfigurationParameters&) override;
	const std::string toString() const override;
};

}
}
#endif /* ASTERBUILDER_H_ */
