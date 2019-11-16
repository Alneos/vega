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
	std::list<std::string> destroyableConcepts;
//	std::set<int> singleGroupCellPositions;
	static constexpr double SMALLEST_RELATIVE_COMPARISON = 1e-7;
	std::ofstream exp_file_ofs;
	std::ofstream comm_file_ofs;

	void writeExport();
	void writeComm();
	void writeLireMaillage();
	void writeAffeModele();
	void writeValues();
	void writeMaterials();
	void writeAffeCaraElem();
	void writeAffeCaraElemPoutre(Beam&);
	void writeAffeCharMeca();
	void writeDefiContact();
	void writeSPC(const ConstraintSet&);
	void writeSPCD(const LoadSet&);
	void writeLIAISON_SOLIDE(const ConstraintSet&);
	void writeLIAISON_MAIL(const ConstraintSet&);
	void writeRBE3(const ConstraintSet&);
	void writeLMPC(const ConstraintSet&);
	void writeGravity(const LoadSet&);
	void writeRotation(const LoadSet&);
	void writeNodalForce(const LoadSet&);
	void writePression(const LoadSet&);
	void writeForceCoque(const LoadSet&);
	void writeForceLine(const LoadSet&);
	void writeForceSurface(const LoadSet&);
	void writeNodeContainer(const NodeContainer& nodeContainer);
	void writeCellContainer(const CellContainer& cellContainer);
	double writeAnalysis(const std::shared_ptr<Analysis>& analysis, double debut);
	void writeAssemblage(const std::shared_ptr<Analysis>& analysis);
	void writeCalcFreq(const std::shared_ptr<LinearModal>& analysis);
	void writeNodalDisplacementAssertion(const NodalDisplacementAssertion&);
	void writeNodalComplexDisplacementAssertion(const NodalComplexDisplacementAssertion&);
	void writeNodalCellVonMisesAssertion(const NodalCellVonMisesAssertion&);
	void writeFrequencyAssertion(const Analysis&, const FrequencyAssertion&);
	void writeLoadset(LoadSet&);
	std::string writeValue(NamedValue&);
	void writeImprResultats(const std::shared_ptr<Analysis>& analysis);
	void list_concept_name(StepRange&);
	std::shared_ptr<NonLinearStrategy> getNonLinearStrategy(const std::shared_ptr<NonLinearMecaStat>&);
	void writeAnalyses();

public:
    AsterWriter() = default;
	AsterWriter(const AsterWriter& that) = delete;
	std::string writeModel(Model&, const ConfigurationParameters&) override;
	std::string toString() const override;
};

}
}
#endif /* ASTERBUILDER_H_ */
