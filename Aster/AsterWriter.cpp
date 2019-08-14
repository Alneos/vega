/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterBuilder.cpp
 *
 *  Created on: 5 mars 2013
 *      Author: dallolio
 */

#include "AsterWriter.h"
#include "MedWriter.h"
#include "build_properties.h"
#include "../Abstract/Model.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <fstream>
#include <limits>

#include <ciso646>

namespace fs = boost::filesystem;
using namespace std;

namespace vega {
namespace aster {

const string AsterWriter::toString() const {
	return "AsterWriter";
}

string AsterWriter::writeModel(Model& model,
		const ConfigurationParameters &configuration) {
	asterModel = make_unique<AsterModel>(model, configuration);
//string currentOutFile = asterModel.getOutputFileName();

	string path = asterModel->configuration.outputPath;
	if (!fs::exists(path)) {
		throw iostream::failure("Directory " + path + " don't exist.");
	}

    fs::path inputFile(asterModel->configuration.inputFile);
	if (fs::exists(inputFile)) {
		fs::copy_file(inputFile, fs::absolute(path) / inputFile.filename(), fs::copy_option::overwrite_if_exists);
	}

    fs::path testFile = asterModel->configuration.resultFile;
	if (fs::exists(testFile)) {
		fs::copy_file(testFile, fs::absolute(path) / testFile.filename(), fs::copy_option::overwrite_if_exists);
	}

	if (configuration.createGraph) {
        for (const auto& analysis : asterModel->model.analyses) {
            string graphviz_path = asterModel->getOutputFileName("_" + to_string(analysis->getId()) + ".dot");
            string png_path = asterModel->getOutputFileName("_" + to_string(analysis->getId()) + ".png");
            ofstream graphviz_file_ofs;
            graphviz_file_ofs.open(graphviz_path.c_str(), ios::out | ios::trunc);
            analysis->createGraph(graphviz_file_ofs);
            if (boost::filesystem::exists( DOXYGEN_DOT_EXECUTABLE )) {
                string dot_cmd = string(DOXYGEN_DOT_EXECUTABLE) + " -Tpng " + graphviz_path + " -o " + png_path;
                system(dot_cmd.c_str());
            }
        }
	}

	string exp_path = asterModel->getOutputFileName(".export");
	string med_path = asterModel->getOutputFileName(".med");
	string comm_path = asterModel->getOutputFileName(".comm");

	ofstream comm_file_ofs;
	//comm_file_ofs.setf(ios::scientific);
 	comm_file_ofs.precision(DBL_DIG);

	ofstream exp_file_ofs;
	exp_file_ofs.open(exp_path.c_str(), ios::trunc | ios::out);
	if (!exp_file_ofs.is_open()) {
		string message = string("Can't open file ") + exp_path + " for writing.";
		throw ios::failure(message);
	}
	this->writeExport(*asterModel, exp_file_ofs);
	exp_file_ofs.close();

	comm_file_ofs.open(comm_path.c_str(), ios::out | ios::trunc);

	if (!comm_file_ofs.is_open()) {
		string message = string("Can't open file ") + comm_path + " for writing.";
		throw ios::failure(message);
	}
	this->writeComm(*asterModel, comm_file_ofs);
	comm_file_ofs.close();

	MedWriter medWriter;
	medWriter.writeMED(model, med_path.c_str());
	return exp_path;
}

void AsterWriter::writeExport(AsterModel &model, ostream& out) {
	out << "P actions make_etude" << endl;
	out << "P mem_aster 100.0" << endl;
	out << "P mode interactif" << endl;
	if (model.model.analyses.empty()) {
		out << "P copy_result_alarm no" << endl;
	}
	out << "P nomjob " << model.model.name << endl;
	out << "P origine Vega++ " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << endl;
	out << "P version " << model.getAsterVersion() << endl;
	out << "A memjeveux " << model.getMemjeveux() << endl;
	out << "A tpmax " << model.getTpmax() << endl;
	out << "F comm " << model.getOutputFileName(".comm", false) << " D 1" << endl;
	out << "F mail " << model.getOutputFileName(".med", false) << " D 20" << endl;
	out << "F mess " << model.getOutputFileName(".mess", false) << " R 6" << endl;
	out << "F resu " << model.getOutputFileName(".resu", false) << " R 8" << endl;
	out << "F rmed " << model.getOutputFileName(".rmed", false) << " R 80" << endl;
	out << "R repe " << model.getOutputFileName("_repe_out", false) << " R 0" << endl;

}

void AsterWriter::writeImprResultats(const AsterModel& asterModel, ostream& out) {
	if (not asterModel.model.analyses.empty()) {
		out << "IMPR_RESU(FORMAT='RESULTAT'," << endl;
		out << "          RESU=(" << endl;
		for (const auto& it : asterModel.model.analyses) {
			const Analysis& analysis = *it;
			switch (analysis.type) {
            case (Analysis::Type::COMBINATION):
			case (Analysis::Type::LINEAR_MECA_STAT): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_CHAM='DEPL'," << " VALE_MAX='OUI'," << " VALE_MIN='OUI',),"
						<< endl;
				break;
			}
			case (Analysis::Type::LINEAR_MODAL): {
				//out << "                _F(RESULTAT=RESU" << analysis.getId()
				//		<< ", TOUT_PARA='OUI', TOUT_CHAM='NON')," << endl;
				break;
			}
            case (Analysis::Type::LINEAR_BUCKLING): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_PARA='CHAR_CRIT', TOUT_CHAM='NON')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_PARA='FREQ', TOUT_CHAM='NON')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
				break;
			}
			case (Analysis::Type::NONLINEAR_MECA_STAT): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_CHAM='DEPL'," << " VALE_MAX='OUI'," << " VALE_MIN='OUI',),"
						<< endl;
				break;
			}
			default:
				out << "# WARN analysis " << analysis << " not supported. Skipping." << endl;
			}
		}
		out << "                )," << endl;
		out << "          );" << endl << endl;
		out << "IMPR_RESU(FORMAT='MED',UNITE=80," << endl;
		out << "          RESU=(" << endl;
		for (const auto& it : asterModel.model.analyses) {
			const Analysis& analysis = *it;
			switch (analysis.type) {
            case (Analysis::Type::COMBINATION):
			case (Analysis::Type::LINEAR_MECA_STAT): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_CHAM=('DEPL'";
				if (calc_sigm) {
					out << ",'" << sigm_noeu << "'";
				}
				out << ",),)," << endl;
				break;
			}
			case (Analysis::Type::NONLINEAR_MECA_STAT):
			case (Analysis::Type::LINEAR_BUCKLING):
			case (Analysis::Type::LINEAR_MODAL): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", NOM_CHAM = 'DEPL',)," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", PARTIE='REEL')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
				out << "                _F(RESULTAT=MODES" << analysis.getId()
						<< ", NOM_CHAM = 'DEPL',)," << endl;
				out << "                _F(RESULTAT=RESU" << analysis.getId()
						<< ", PARTIE='REEL')," << endl;
				break;
			}
			default:
				out << "# WARN analysis " << analysis << " not supported. Skipping." << endl;
			}
		}
		out << "                )," << endl;
		out << "          );" << endl << endl;
		for (const auto& it : asterModel.model.analyses) {
			const Analysis& analysis = *it;

			vector<shared_ptr<Objective>> displacementOutputs = asterModel.model.objectives.filter(Objective::Type::NODAL_DISPLACEMENT_OUTPUT);
			out << "RETB" << analysis.getId();
			if (displacementOutputs.size() >= 1) {
                out << "=POST_RELEVE_T(ACTION=(" << endl;
                for (auto output : displacementOutputs) {
                    shared_ptr<const NodalDisplacementOutput> displacementOutput = dynamic_pointer_cast<const NodalDisplacementOutput>(output);
                    out << "                _F(INTITULE='DISP" << output->bestId() << "',OPERATION='EXTRACTION',RESULTAT=RESU" << analysis.getId() << ",";
                    writeNodeContainer(*displacementOutput, out);
                    out << "NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                }
                out << "),)" << endl << endl;
			} else {

                out << "=CREA_TABLE(RESU=(" << endl;
                switch (analysis.type) {
                case (Analysis::Type::COMBINATION):
                case (Analysis::Type::LINEAR_MODAL):
                case (Analysis::Type::LINEAR_BUCKLING):
                case (Analysis::Type::NONLINEAR_MECA_STAT):
                case (Analysis::Type::LINEAR_MECA_STAT): {
                    out << "                _F(RESULTAT=RESU" << analysis.getId()
                            << ",TOUT='OUI',NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                    break;
                }
                case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
                    break;
                }
                case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
                    out << "                _F(RESULTAT=MODES" << analysis.getId()
                            << ",TOUT='OUI',NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                    break;
                }
                default:
                    out << "# WARN analysis " << analysis << " not supported. Skipping." << endl;
                }
			out << "),)" << endl << endl;
			}

			out << "unite=DEFI_FICHIER(ACTION='ASSOCIER'," << endl;
			out << "             FICHIER='REPE_OUT/tbresu_" << analysis.getId() << ".csv')" << endl
					<< endl;

			out << "IMPR_TABLE(TABLE=RETB" << analysis.getId() << "," << endl;
			out << "           FORMAT='TABLEAU'," << endl;
			out << "           UNITE=unite," << endl;
			out << "           SEPARATEUR=' ,'," << endl;
			out << "           TITRE='RESULTS',)" << endl << endl;

			out << "DEFI_FICHIER(ACTION='LIBERER'," << endl;
			out << "             UNITE=unite,)" << endl << endl;
			out << "DETRUIRE(CONCEPT=(_F(NOM=unite),))" << endl << endl;
		}

		for (const auto& analysis : asterModel.model.analyses) {
		    bool hasRecoveryPoints = false;
            for (const auto& elementSet : asterModel.model.elementSets) {
                if (not elementSet->isBeam()) continue;
                auto beam = dynamic_pointer_cast<Beam>(elementSet);
                if (beam->recoveryPoints.size() >= 1) {
                    hasRecoveryPoints = true;
                    break;
                }
            }
            if (not hasRecoveryPoints) continue;

            for (const auto& elementSet : asterModel.model.elementSets) {
                if (not elementSet->isBeam()) continue; // to avoid CALCUL_37 Le TYPE_ELEMENT MECA_BARRE  ne sait pas encore calculer l'option:  SIPO_ELNO.
                out << "RESU" << analysis->getId() << "=CALC_CHAMP(reuse=RESU" << analysis->getId() << ","
                        << endl;
                out << "           RESULTAT=RESU" << analysis->getId() << "," << endl;
                out << "           MODELE=MODMECA," << endl;
                out << "           CONTRAINTE =('SIPO_NOEU')," << endl;
                writeCellContainer(*elementSet, out);
                out << ")" << endl;
            }

            out << "RCTB" << analysis->getId() << "=MACR_LIGN_COUPE(" << endl;
            out << "            RESULTAT=RESU" << analysis->getId() << "," << endl;
            out << "            NOM_CHAM='SIPO_NOEU'," << endl;
            out << "            MODELE=MODMECA," << endl;
            out << "            LIGN_COUPE=(" << endl;
            for (const auto& elementSet : asterModel.model.elementSets) {
                if (not elementSet->isBeam()) continue;
                auto beam = dynamic_pointer_cast<Beam>(elementSet);
                for (const auto& recoveryPoint : beam->recoveryPoints) {
                    const VectorialValue& localCoords = recoveryPoint.getLocalCoords();
                    for (const Cell& cell : beam->getCellsIncludingGroups()) {
                        const Node& node1 = asterModel.model.mesh.findNode(cell.nodePositions[0]);
                        const VectorialValue& globalCoords = recoveryPoint.getGlobalCoords(cell.id);
                        out << "                    _F(" << endl;
                        out << "                        INTITULE='Cell " << cell.id << " stress recovery at (local):" << localCoords << ", global:" << globalCoords << "'," << endl;
                        out << "                        NOM_CMP=('SN','SMFY','SMFZ','SVY','SVZ','SMT')," << endl;
                        out << "                        TYPE='SEGMENT'," << endl;
                        out << "                        DISTANCE_MAX=" << abs(max(localCoords.y(), localCoords.z()))*2 << "," << endl;
                        out << "                        NB_POINTS=2," << endl;
                        out << "                        COOR_ORIG=(" << globalCoords.x() << "," << globalCoords.y() << "," << globalCoords.z() << ")," << endl;
                        // TODO LD find a better solution here
                        out << "                        COOR_EXTR=(" << node1.x << "," << node1.y << "," << node1.z << ")," << endl;
                        out << "                    )," << endl;
                    }
                }
            }
            out << "                    )" << endl;
			out << "            )" << endl;

			int unit = 10 + analysis->getId();
            out << "DEFI_FICHIER(ACTION='ASSOCIER'," << endl;
			out << "             UNITE=" << unit << "," << endl;
			out << "             FICHIER='REPE_OUT/tbrecup_" << analysis->getId() << ".csv')" << endl << endl;

            out << "IMPR_TABLE(TABLE=RCTB" << analysis->getId() << "," << endl;
			out << "           FORMAT='TABLEAU'," << endl;
			out << "           UNITE=" << unit << "," << endl;
			out << "           SEPARATEUR=' ,'," << endl;
			out << "           TITRE='RESULTS',)" << endl << endl;

            out << "DEFI_FICHIER(ACTION='LIBERER'," << endl;
			out << "             UNITE=" << unit << ")" << endl << endl;
		}
	}
}

void AsterWriter::writeAnalyses(const AsterModel& asterModel, ostream& out) {
	double debut = 0;
	for (const auto& it : asterModel.model.analyses) {
		Analysis& analysis = *it;
		debut = writeAnalysis(asterModel, analysis, out, debut);

		if (calc_sigm) {
			out << "RESU" << analysis.getId() << "=CALC_CHAMP(reuse=RESU" << analysis.getId() << ","
					<< endl;
			out << "           RESULTAT=RESU" << analysis.getId() << "," << endl;
			out << "           MODELE=MODMECA," << endl;
            if (asterModel.model.materials.size() >= 1) {
                out << "           CHAM_MATER=CHMAT," << endl;
            }
            out << "           CARA_ELEM=CAEL," << endl;
			out << "           CONTRAINTE =('SIGM_ELNO')," << endl;
			out << "           FORCE = 'REAC_NODA'," << endl;
			out << ")" << endl;
		}
		vector<shared_ptr<Assertion>> assertions = analysis.getAssertions();
		if (!assertions.empty()) {
			out << "TEST_RESU(RESU = (" << endl;

			for (shared_ptr<Assertion> assertion : assertions) {
				switch (assertion->type) {
				case Objective::Type::NODAL_DISPLACEMENT_ASSERTION:
					out << "                  _F(RESULTAT=RESU" << analysis.getId() << "," << endl;
					writeNodalDisplacementAssertion(asterModel, *assertion, out);
                    out << "                     )," << endl;
					break;
				case Objective::Type::FREQUENCY_ASSERTION:
					writeFrequencyAssertion(analysis, *assertion, out);
					break;
				case Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION:
					out << "                  _F(RESULTAT=RESU" << analysis.getId() << "," << endl;
					writeNodalComplexDisplacementAssertion(asterModel, *assertion, out);
                    out << "                     )," << endl;
					break;
				default:
					handleWritingError(string("Not implemented"));
				}
			}
			out << "                  )" << endl;
			out << "          );" << endl << endl;
		}
	}
}

void AsterWriter::writeComm(const AsterModel& asterModel, ostream& out) {
	string asterVersion(asterModel.getAsterVersion());
	out << "#Vega++ version " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << endl;
	out << "#Aster version " << asterModel.getAsterVersion() << endl;
	out << "DEBUT(PAR_LOT='NON', IGNORE_ALARM=('SUPERVIS_1'))" << endl;

	mail_name = "MAIL";
	sigm_noeu = "SIGM_NOEU";
	sigm_elno = "SIGM_ELNO";
	sief_elga = "SIEF_ELGA";

	writeLireMaillage(asterModel, out);

	writeAffeModele(asterModel, out);

	for (const auto& value : asterModel.model.values) {
		writeValue(*value, out);
	}

	writeMaterials(asterModel, out);

	writeAffeCaraElem(asterModel, out);

	writeAffeCharMeca(asterModel, out);

	writeDefiContact(asterModel, out);

	writeAnalyses(asterModel, out);

	writeImprResultats(asterModel, out);

	out << "FIN()" << endl;
}

void AsterWriter::writeLireMaillage(const AsterModel& asterModel, ostream& out) {
	out << mail_name << "=LIRE_MAILLAGE(FORMAT='MED',";
	if (asterModel.configuration.logLevel >= LogLevel::DEBUG and asterModel.model.mesh.countNodes() < 100) {
		out << "INFO_MED=2,VERI_MAIL=_F(VERIF='OUI',),INFO=2";
	} else {
		out << "VERI_MAIL=_F(VERIF='NON',),";
	}
	out << ");" << endl << endl;

    for (const auto& constraintSet : asterModel.model.constraintSets) {
		if (not constraintSet->hasContacts()) {
			continue;
		}
        const auto& zones = constraintSet->getConstraintsByType(Constraint::Type::ZONE_CONTACT);
        if (zones.empty()) {
            continue;
        }
        out << mail_name << "=MODI_MAILLAGE(reuse="<< mail_name << ",";
        out << "MAILLAGE=" << mail_name << "," << endl;
        // TODO LD should find a better solution
        int firstNodePosition = *((*zones.begin())->nodePositions().begin());
        if (asterModel.model.mesh.findNode(firstNodePosition).dofs == DOFS::ALL_DOFS) {
            out << "         ORIE_PEAU_2D=(" << endl;
        } else {
            out << "         ORIE_PEAU_3D=(" << endl;
        }
		for (shared_ptr<Constraint> constraint : zones) {
		    shared_ptr<const ZoneContact> zone = dynamic_pointer_cast<const ZoneContact>(constraint);
		    shared_ptr<const ContactBody> master = dynamic_pointer_cast<const ContactBody>(asterModel.model.find(zone->master));
		    shared_ptr<const BoundarySurface> masterSurface = dynamic_pointer_cast<const BoundarySurface>(asterModel.model.find(master->boundary));
		    shared_ptr<const ContactBody> slave = dynamic_pointer_cast<const ContactBody>(asterModel.model.find(zone->slave));
		    shared_ptr<const BoundarySurface> slaveSurface = dynamic_pointer_cast<const BoundarySurface>(asterModel.model.find(slave->boundary));
            out << "                             _F(";
            writeCellContainer(*masterSurface, out);
            out << ")," << endl;
            out << "                             _F(";
            writeCellContainer(*slaveSurface, out);
            out << ")," << endl;

		}
		out << "                             )," << endl;
		out << "                   )" << endl << endl;
	}

	for (const auto& constraintSet : asterModel.model.constraintSets) {
        const auto& surfaces = constraintSet->getConstraintsByType(
				Constraint::Type::SURFACE_SLIDE_CONTACT);
        if (surfaces.empty()) {
            continue;
        }
        out << mail_name << "=MODI_MAILLAGE(reuse="<< mail_name << ",";
        out << "MAILLAGE=" << mail_name << "," << endl;
        out << "         ORIE_PEAU_3D=(" << endl;
		for (shared_ptr<Constraint> constraint : surfaces) {
		    shared_ptr<const SurfaceSlide> surface = dynamic_pointer_cast<const SurfaceSlide>(constraint);
		    shared_ptr<const BoundaryElementFace> masterSurface = dynamic_pointer_cast<const BoundaryElementFace>(asterModel.model.find(surface->master));
		    shared_ptr<const BoundaryElementFace> slaveSurface = dynamic_pointer_cast<const BoundaryElementFace>(asterModel.model.find(surface->slave));
            out << "                             _F(";
            out << "GROUP_MA=('"
                    << masterSurface->surfaceCellGroup->getName() << "', '"
                    << slaveSurface->surfaceCellGroup->getName()
                    << "'),";
            out << ")," << endl;
		}
		out << "                             )," << endl;
		out << "                   )" << endl << endl;
	}
}

void AsterWriter::writeAffeModele(const AsterModel& asterModel, ostream& out) {
	out << "MODMECA=AFFE_MODELE(MAILLAGE=" << mail_name << "," << endl;
	out << "                    AFFE=(" << endl;
	for (const auto& elementSet : asterModel.model.elementSets) {
		if (!elementSet->empty()) {
			out << "                          _F(";
			writeCellContainer(*elementSet, out);
			out << endl;
			out << "                             PHENOMENE='" << asterModel.phenomene << "',"
					<< endl;

			out << "                             MODELISATION="
					<< asterModel.getModelisations(elementSet) << ")," << endl;
		} else {
			out << "#Skipping element El" << *elementSet << " because no assignment" << endl;
		}
	}
	out << "                          )," << endl;
	out << "                    );" << endl << endl;
}

string AsterWriter::writeValue(NamedValue& value, ostream& out) {
	string concept_name;

	switch (value.type) {
	case Value::Type::LIST: {
		ListValueBase& listValue = dynamic_cast<ListValueBase&>(value);

		ostringstream list_concept_ss;
		list_concept_ss << "LST" << setfill('0') << setw(5) << listValue.getId();
		concept_name = list_concept_ss.str();
		out << concept_name << "=DEFI_LIST_REEL(" << endl;
		out << "                        VALE = (";
		if (listValue.isintegral()) {
		    const ListValue<int>& listIntValue = dynamic_cast<ListValue<int>&>(value);
            for (const int val : listIntValue.getList()) {
                out << val << ",";
            }
		} else {
		    const ListValue<double>& listDblValue = dynamic_cast<ListValue<double>&>(value);
            for (const double val : listDblValue.getList()) {
                out << val << ",";
            }
		}
        out << ")," << endl;
        out << "                        );" << endl << endl;
        listValue.markAsWritten();
        break;
	}
    case Value::Type::SET: {
		SetValueBase& setValue = dynamic_cast<SetValueBase&>(value);

		ostringstream list_concept_ss;
		list_concept_ss << "LST" << setfill('0') << setw(5) << setValue.getId();
		concept_name = list_concept_ss.str();
		out << concept_name << "=DEFI_LIST_REEL(" << endl;
		out << "                        VALE = (";
		if (setValue.isintegral()) {
		    const SetValue<int>& setIntValue = dynamic_cast<SetValue<int>&>(value);
            for (const int val : setIntValue.getSet()) {
                out << val << ",";
            }
		} else {
		    handleWritingError("non-integral set not yet implemented");
		}
        out << ")," << endl;
        out << "                        );" << endl << endl;
        setValue.markAsWritten();
        break;
	}
	case Value::Type::STEP_RANGE: {
		StepRange& stepRange = dynamic_cast<StepRange&>(value);
        ostringstream list_concept_ss;
        list_concept_ss << "LST" << setfill('0') << setw(5) << stepRange.getId();
        concept_name = list_concept_ss.str();
		if (not is_equal(stepRange.end, Globals::UNAVAILABLE_DOUBLE)) {
            out << concept_name << "=DEFI_LIST_REEL(" << endl;
            out << "                        DEBUT = " << stepRange.start << "," << endl;
            out << "                        INTERVALLE = _F(JUSQU_A = " << stepRange.end << "," << endl;
            out << "                                        NOMBRE = " << stepRange.count << endl;
            out << "                                        )," << endl;
            out << "                        );" << endl << endl;
            stepRange.markAsWritten();
		} else {
            out << "# Ignoring " << concept_name << " because: no end value" << endl;
		}
		break;
	}
	case Value::Type::SPREAD_RANGE: {
        SpreadRange& spreadRange = dynamic_cast<SpreadRange&>(value);
        ostringstream list_concept_ss;
        list_concept_ss << "LST" << setfill('0') << setw(5) << spreadRange.getId();
        concept_name = list_concept_ss.str();
        out << concept_name << "=DEFI_LIST_FREQ(" << endl;
        out << "                        DEBUT = " << spreadRange.start << "," << endl;
        out << "                        INTERVALLE = _F(JUSQU_A = " << spreadRange.end << "," << endl;
        out << "                                        NOMBRE = " << spreadRange.count << endl;
        out << "                                        )," << endl;
        out << "                        RAFFINEMENT = _F(LIST_RAFFINE = XXXX" << "," << endl;
        out << "                                        CRITERE = 'RELATIF'," << endl;
        out << "                                        DISPERSION = " << spreadRange.spread << endl;
        out << "                                        )," << endl;
        out << "                        );" << endl << endl;
        spreadRange.markAsWritten();
        break;
		}
	case Value::Type::FUNCTION_TABLE: {
		FunctionTable& functionTable = dynamic_cast<FunctionTable&>(value);
		ostringstream concept_ss;
		concept_ss << "FCT" << setfill('0') << setw(5) << functionTable.getId();
		concept_name = concept_ss.str();
		out << concept_name << "=DEFI_FONCTION(" << endl;
		if (functionTable.hasParaX())
			out << "                       NOM_PARA="
					<< AsterModel::NomParaByParaName.find(functionTable.getParaX())->second << ","
					<< endl;

		if (functionTable.hasParaY())
			out << "                       NOM_RESU="
					<< AsterModel::NomParaByParaName.find(functionTable.getParaY())->second << ","
					<< endl;

		out << "                       VALE = (" << endl;
		for (auto it = functionTable.getBeginValuesXY();
				it != functionTable.getEndValuesXY(); it++)
			out << "                               " << it->first << ", " << it->second << ","
					<< endl;
		out << "                               )," << endl;
		out << "                       INTERPOL = ("
				<< AsterModel::InterpolationByInterpolation.find(functionTable.parameter)->second
				<< "," << AsterModel::InterpolationByInterpolation.find(functionTable.value)->second
				<< ")," << endl;
		out << "                       PROL_GAUCHE="
				<< AsterModel::ProlongementByInterpolation.find(functionTable.left)->second << ","
				<< endl;
		out << "                       PROL_DROITE="
				<< AsterModel::ProlongementByInterpolation.find(functionTable.right)->second << ","
				<< endl;
		out << "                       );";
		if (functionTable.isOriginal()) {
			out << "# Original id:" << functionTable.getOriginalId();
		}
		out << endl << endl;
		functionTable.markAsWritten();
		break;
	}
	case Value::Type::SCALAR:
	case Value::Type::BAND_RANGE:
	case Value::Type::DYNA_PHASE: {
		break;
	}
	default:
		handleWritingError(string("NamedValue not yet implemented"));
	}
	asternameByValue[value] = concept_name;

	return concept_name;
}

void AsterWriter::writeMaterials(const AsterModel& asterModel, ostream& out) {

	for (auto& material : asterModel.model.materials) {
		if (material->isOriginal()) {
			out << "# Material original id " << material->getOriginalId() << endl;
		}
		out << "M" << material->getId() << "=DEFI_MATERIAU(" << endl;
		const shared_ptr<const Nature> enature = material->findNature(Nature::NatureType::NATURE_ELASTIC);
		if (enature) {
			const auto& elasticNature = dynamic_pointer_cast<const ElasticNature>(enature);
			out << "                 ELAS=_F(" << endl;
			out << "                         E=" << elasticNature->getE() << "," << endl;
			out << "                         NU=" << elasticNature->getNu() << "," << endl;
			out << "                         RHO=" << elasticNature->getRho() << "," << endl;
			if (not is_equal(elasticNature->getGE(), Globals::UNAVAILABLE_DOUBLE)) {
                out << "                         AMOR_HYST=" << elasticNature->getGE() << "," << endl;
			}
			out << "                         )," << endl;
		}
		const shared_ptr<const Nature> hynature = material->findNature(Nature::NatureType::NATURE_HYPERELASTIC);
		if (hynature) {
			const auto& hyperElasticNature = dynamic_pointer_cast<const HyperElasticNature>(hynature);
			out << "                 ELAS_HYPER=_F(" << endl;
			out << "                         C10=" << hyperElasticNature->c10 << "," << endl;
			out << "                         C01=" << hyperElasticNature->c01 << "," << endl;
			out << "                         C20=" << hyperElasticNature->c20 << "," << endl;
			out << "                         RHO=" << hyperElasticNature->rho << "," << endl;
			out << "                         K=" << hyperElasticNature->k << "," << endl;
			out << "                         )," << endl;
		}
		const shared_ptr<const Nature> onature = material->findNature(Nature::NatureType::NATURE_ORTHOTROPIC);
		if (onature) {
			const auto& orthoNature = dynamic_pointer_cast<const OrthotropicNature>(onature);
			out << "                 ELAS_ORTH=_F(" << endl;
			out << "                         E_L=" << orthoNature->getE_longitudinal() << "," << endl;
            out << "                         E_T=" << orthoNature->getE_transverse() << "," << endl;
            out << "                         G_LT=" << orthoNature->getG_longitudinal_transverse() << "," << endl;
            if (!is_equal(orthoNature->getG_transverse_normal(),Globals::UNAVAILABLE_DOUBLE)){
                out << "                         G_TN=" << orthoNature->getG_transverse_normal() << "," << endl;
            }
            if (!is_equal(orthoNature->getG_longitudinal_normal(),Globals::UNAVAILABLE_DOUBLE)){
                out << "                         G_LN=" << orthoNature->getG_longitudinal_normal() << "," << endl;
            }
			out << "                         NU_LT=" << orthoNature->getNu_longitudinal_transverse() << "," << endl;
			out << "                         )," << endl;
		}
		const shared_ptr<const Nature> binature = material->findNature(Nature::NatureType::NATURE_BILINEAR_ELASTIC);
		if (binature) {
			const auto& bilinearNature = dynamic_pointer_cast<const BilinearElasticNature>(binature);
			out << "                 ECRO_LINE=_F(" << endl;
			out << "                         D_SIGM_EPSI=" << bilinearNature->secondary_slope << ","
					<< endl;
			out << "                         SY=" << bilinearNature->elastic_limit << "," << endl;
			out << "                         )," << endl;
		}
		out << "                 );" << endl << endl;
		material->markAsWritten();
	}

	vector<shared_ptr<ElementSet>> composites = asterModel.model.elementSets.filter(ElementSet::Type::COMPOSITE);
    out << "# writing " << composites.size() << " composites" << endl;
    for (shared_ptr<ElementSet> c : composites) {
        shared_ptr<Composite> composite = dynamic_pointer_cast<Composite>(c);
        out << "MC" << composite->getId() << "=DEFI_COMPOSITE(" << endl;
        out << "                 COUCHE=(" << endl;
        for (const auto& layer : composite->getLayers()) {
            out << "                     _F(EPAIS=" << layer.getThickness() << ",  MATER=M" << layer.getMaterialId() << ", ORIENTATION=" << layer.getOrientation() << ")," << endl;
        }
        out << "                         )," << endl;
        out << "                 );" << endl << endl;
        composite->markAsWritten();
    }

  if (asterModel.model.materials.size() >= 1) {
    out << "CHMAT=AFFE_MATERIAU(MAILLAGE=" << mail_name << "," << endl;
    out << "                    AFFE=(" << endl;
    for (const auto& material : asterModel.model.materials) {
      CellContainer cells = material->getAssignment();
      if (!cells.empty()) {
        out << "                          _F(MATER=M" << material->getId() << ",";
        writeCellContainer(cells, out);
        out << ")," << endl;
      } else {
        out << "# WARN Skipping material id " << material->getId() << " because no assignment"
            << endl;
      }
    }
    for (shared_ptr<ElementSet> c : composites) {
        shared_ptr<Composite> composite = dynamic_pointer_cast<Composite>(c);
          out << "                          _F(MATER=MC" << composite ->getId() << ",";
          writeCellContainer(*composite, out);
          out << ")," << endl;
    }
    out << "                          )," << endl;
    out << "                    );" << endl << endl;
  }
}

void AsterWriter::writeAffeCaraElem(const AsterModel& asterModel, ostream& out) {
	calc_sigm = false;
	if (asterModel.model.elementSets.size() > 0) {
		out << "CAEL=AFFE_CARA_ELEM(MODELE=MODMECA," << endl;

		vector<shared_ptr<ElementSet>> discrets_0d = asterModel.model.elementSets.filter(
				ElementSet::Type::DISCRETE_0D);
		vector<shared_ptr<ElementSet>> discrets_1d = asterModel.model.elementSets.filter(ElementSet::Type::DISCRETE_1D);
                vector<shared_ptr<ElementSet>> scalar_springs = asterModel.model.elementSets.filter(ElementSet::Type::SCALAR_SPRING);
                vector<shared_ptr<ElementSet>> structural_segments = asterModel.model.elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT);
                discrets_1d.insert(discrets_1d.end(), scalar_springs.begin(), scalar_springs.end());
                discrets_1d.insert(discrets_1d.end(), structural_segments.begin(), structural_segments.end());
		vector<shared_ptr<ElementSet>> nodal_masses = asterModel.model.elementSets.filter(
				ElementSet::Type::NODAL_MASS);
        auto numDiscrets = discrets_0d.size() + nodal_masses.size() + discrets_1d.size();
		out << "                    # writing " << numDiscrets << " discrets" << endl;
		if (numDiscrets > 0) {
			out << "                    DISCRET=(" << endl;
			for (shared_ptr<ElementSet> discret : discrets_0d) {
				if (!discret->empty()) {
                    shared_ptr<DiscretePoint> discret_0d = dynamic_pointer_cast<DiscretePoint>(discret);
                    string rotationMarker = "";
                    if (discret_0d->hasRotations()) {
                        rotationMarker = "R";
                    }
                    string diagonalMarker = "";
                    // M_T_D_N and M_TR_D_N do not allow to specify the diagonal for PO1 elements
                    //if (discret_0d->isDiagonal()) {
                    //    diagonalMarker = "D_";
                    //}
                    string syme = "";
                    if (not discret_0d->isSymmetric()) {
                        syme = "SYME='NON',";
                    }
                    out << "                             _F(";
                    writeCellContainer(*discret, out);
                    out << endl;
                    out << "                                " << syme << "CARA='K_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                    for (double rigi : discret_0d->asStiffnessVector())
                        out << rigi << ",";
                    out << "),)," << endl;

					if (discret_0d->hasDamping()) {
                        out << "                             _F(";
                        writeCellContainer(*discret, out);
                        out << endl;
                        out << "                                " << syme << "CARA='A_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                        for (double amor : discret_0d->asDampingVector())
                            out << amor << ",";
                        out << "),)," << endl;
					}

					if (discret_0d->hasMass()) {
                        out << "                             _F(";
                        writeCellContainer(*discret, out);
                        out << endl;
                        out << "                                " << syme << "CARA='M_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                        for (double mass : discret_0d->asMassVector())
                            out << mass << ",";
                        out << "),)," << endl;
					}

				} else {
					out
							<< "                             # WARN Finite Element : DISCRETE_0D ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			for (shared_ptr<ElementSet> discret : discrets_1d) {
				if (!discret->empty()) {
                    shared_ptr<Discrete> discret_1d = dynamic_pointer_cast<Discrete>(discret);
                    string rotationMarker = "";
                    if (discret_1d->hasRotations()) {
                        rotationMarker = "R";
                    }
                    string diagonalMarker = "";
                    if (discret_1d->isDiagonal()) {
                        diagonalMarker = "D_";
                    }
                    string syme = "";
                    if (not discret_1d->isSymmetric()) {
                        syme = "SYME='NON',";
                    }
                    out << "                             _F(";
                    writeCellContainer(*discret, out);
                    out << endl;
                    out << "                                " << syme << "CARA='K_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";
                    for (double rigi : discret_1d->asStiffnessVector())
                        out << rigi << ",";
                    out << "),)," << endl;

                    if (discret_1d->hasDamping()) {
                        out << "                             _F(";
                        writeCellContainer(*discret, out);
                        out << endl;
                        out << "                                " << syme << "CARA='A_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";
                        for (double amor : discret_1d->asDampingVector())
                            out << amor << ",";
                        out << "),)," << endl;
                    }

                    if (discret_1d->hasMass()) {
                        out << "                             _F(";
                        writeCellContainer(*discret, out);
                        out << endl;
                        out << "                                " << syme << "CARA='M_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";

                        for (double mass : discret_1d->asMassVector())
                            out << mass << ",";
                        out << "),)," << endl;
                    }

				} else {
					out
							<< "                             # WARN Finite Element : DISCRETE_1D ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			for (shared_ptr<ElementSet> discret : nodal_masses) {
				if (!discret->empty()) {
					out << "                             _F(";
					writeCellContainer(*discret, out);
					out << endl;
					shared_ptr<NodalMass> nodalMass = dynamic_pointer_cast<NodalMass>(discret);
					if (nodalMass->hasRotations()) {
                        out << "                                CARA='M_TR_D_N',VALE=("
                                << nodalMass->getMass() << "," << nodalMass->ixx << ","
                                << nodalMass->iyy << "," << nodalMass->izz << "," << nodalMass->ixy
                                << "," << nodalMass->iyz << "," << nodalMass->ixz << ","
                                << nodalMass->ex << "," << nodalMass->ey << "," << nodalMass->ez
                                << "),)," << endl;
					} else {
                        out << "                                CARA='M_T_D_N',VALE=("
							<< nodalMass->getMass()	<< "),)," << endl;
					}
				} else {
					out
							<< "                             # WARN Finite Element : NODAL_MASS ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			out << "                             )," << endl;
		}
		vector<shared_ptr<Beam>> poutres = asterModel.model.getBeams();
        vector<shared_ptr<Beam>> barres = asterModel.model.getTrusses();
		out << "                    # writing " << poutres.size() << " poutres" << endl;
		out << "                    # writing " << barres.size() << " barres" << endl;
		if (poutres.size() > 0 or (asterModel.model.needsLargeDisplacements() and barres.size() > 0)) {
			out << "                    POUTRE=(" << endl;
			for (const auto& poutre : poutres) {
				writeAffeCaraElemPoutre(asterModel, *poutre, out);
			}
			if (asterModel.model.needsLargeDisplacements()) {
                for (const auto& barre : barres) {
                    writeAffeCaraElemPoutre(asterModel, *barre, out);
                }
			}
			out << "                            )," << endl;
		}

		if (barres.size() > 0 and not asterModel.model.needsLargeDisplacements()) {
			out << "                    BARRE=(" << endl;
			for (const auto& barre : barres) {
				writeAffeCaraElemPoutre(asterModel, *barre, out);
			}
			out << "                            )," << endl;
		}
		vector<shared_ptr<ElementSet>> shells = asterModel.model.elementSets.filter(ElementSet::Type::SHELL);
		vector<shared_ptr<ElementSet>> composites = asterModel.model.elementSets.filter(ElementSet::Type::COMPOSITE);
		out << "                    # writing " << shells.size()+composites.size() << " shells (ou composites)" << endl;
		if (shells.size() + composites.size() > 0) {
			calc_sigm = true;
			out << "                    COQUE=(" << endl;
			for (shared_ptr<ElementSet> shell : shells) {
				if (!shell->empty()) {
					out << "                           _F(";
					writeCellContainer(*shell, out);
					out << endl;
					out << "                              EPAIS="
							<< dynamic_pointer_cast<Shell>(shell)->thickness << "," << endl;
					out << "                              VECTEUR=(0.9,0.1,0.2))," << endl;
				} else {
					out
							<< "                           # WARN Finite Element : COQUE ignored because its GROUP_MA is empty."
							<< endl;
				}
				shell->markAsWritten();
			}
			for (shared_ptr<ElementSet> c : composites) {
                shared_ptr<Composite> composite = dynamic_pointer_cast<Composite>(c);
				if (!composite->empty()) {
					out << "                           _F(";
					writeCellContainer(*composite, out);
					out << endl;
					out << "                              EPAIS="
							<< composite->getTotalThickness() << "," << endl;
					out << "                              COQUE_NCOU="
							<< composite->getLayers().size() << "," << endl;
					out << "                              VECTEUR=(1.0,0.0,0.0))," << endl;
				} else {
					out
							<< "                           # WARN Finite Element : COMPOSITE ignored because its GROUP_MA is empty."
							<< endl;
				}
				composite->markAsWritten();
			}
			out << "                           )," << endl;
		}
		vector<shared_ptr<ElementSet>> solids = asterModel.model.elementSets.filter(
				ElementSet::Type::CONTINUUM);
        vector<shared_ptr<ElementSet>> skins = asterModel.model.elementSets.filter(
				ElementSet::Type::SKIN);
		out << "                    # writing " << solids.size() << " solids" << endl;
		out << "                    # writing " << skins.size() << " skins" << endl;
		if (solids.size() > 0) {
			out << "                    MASSIF=(" << endl;
			for (shared_ptr<ElementSet> solid : solids) {
				if (!solid->empty()) {
					out << "                           _F(";
					writeCellContainer(*solid, out);
					out << endl;
					out << "                               ANGL_REP=(0.,0.,0.,),)," << endl;
				} else {
					out << "# WARN Finite Element : MASSIF ignored because its GROUP_MA is empty."
							<< endl;
				}
				solid->markAsWritten();
			}
			for (shared_ptr<ElementSet> skin : skins) {
				if (!skin->empty()) {
					out << "                           _F(";
					writeCellContainer(*skin, out);
					out << endl;
					out << "                               ANGL_REP=(0.,0.,0.,),)," << endl;
				} else {
					out << "# WARN Finite Element : MASSIF (skin) ignored because its GROUP_MA is empty."
							<< endl;
				}
				skin->markAsWritten();
			}
			out << "                            )," << endl;
		}
	}

	//orientations
	bool orientationsPrinted = false;
	for (const auto& it : asterModel.model.mesh.cellGroupNameByCspos){
        if (asterModel.model.elementSets.filter(ElementSet::Type::CONTINUUM).size() == asterModel.model.elementSets.size()) {
            // LD workaround for case no beams, discrete, shells, composites... only solids in model, but segments in geometry
            // TODO : probably this loop should be applyed to elementSets and not over groups!!
            continue;
        }
        std::shared_ptr<vega::CoordinateSystem> cs= asterModel.model.mesh.getCoordinateSystemByPosition(it.first);
		if (cs->type!=CoordinateSystem::Type::RELATIVE){
		   //handleWritingError("Coordinate System of Group "+ it.second+" is not an ORIENTATION.");
		   continue;
		}
		if (!orientationsPrinted) {
			out << "                    ORIENTATION=(" << endl;
			orientationsPrinted = true;
		}
		std::shared_ptr<OrientationCoordinateSystem> ocs = std::dynamic_pointer_cast<OrientationCoordinateSystem>(cs);

		out << "                                 _F(CARA ='VECT_Y',VALE=(";
		out << ocs->getV().x() << "," << ocs->getV().y() << "," << ocs->getV().z() << ")";
		out<<",GROUP_MA='"<< it.second << "')," << endl;
	}
	if (orientationsPrinted) {
		out << "                                 )," << endl;
	}
	out << "                    );" << endl << endl;
}
void AsterWriter::writeAffeCaraElemPoutre(const AsterModel& asterModel, ElementSet& elementSet, ostream& out) {
	out << "                            _F(";
	writeCellContainer(elementSet, out);
	out << endl;
	switch (elementSet.type) {
	case ElementSet::Type::RECTANGULAR_SECTION_BEAM: {
		RectangularSectionBeam& rectBeam =
				static_cast<RectangularSectionBeam&>(elementSet);
		out << "                               VARI_SECT='CONSTANT'," << endl;
		out << "                               SECTION='RECTANGLE'," << endl;
		out << "                               CARA=('HY','HZ',)," << endl;
		out << "                               VALE=(" << rectBeam.height << "," << rectBeam.width
				<< ")," << endl;
        rectBeam.markAsWritten();
		break;
	}
	case ElementSet::Type::CIRCULAR_SECTION_BEAM: {
		CircularSectionBeam& circBeam = static_cast<CircularSectionBeam&>(elementSet);
		out << "                               SECTION='CERCLE'," << endl;
		out << "                               CARA=('R',)," << endl;
		out << "                               VALE=(" << circBeam.radius << ")," << endl;
		circBeam.markAsWritten();
		break;
	}
	case ElementSet::Type::TUBE_SECTION_BEAM: {
		TubeSectionBeam& tubeBeam = static_cast<TubeSectionBeam&>(elementSet);
		out << "                               SECTION='CERCLE'," << endl;
		out << "                               CARA=('R','EP')," << endl;
		out << "                               VALE=(" << tubeBeam.radius << ","<< tubeBeam.thickness << ")," << endl;
		tubeBeam.markAsWritten();
		break;
	}
	default:
		Beam& beam =
				static_cast<Beam&>(elementSet);
		out << "                               SECTION='GENERALE'," << endl;
		if (not beam.isTruss() or asterModel.model.needsLargeDisplacements()) {
		    out << "                               CARA=('A','IY','IZ','JX','AY','AZ',)," << endl;
		} else {
		    out << "                               CARA=('A',)," << endl;
		}

		out << "                               VALE=(";
        if (not beam.isTruss() or asterModel.model.needsLargeDisplacements()) {
            out << max(std::numeric_limits<double>::epsilon(), beam.getAreaCrossSection()) << ","
				<< max(std::numeric_limits<double>::epsilon(), beam.getMomentOfInertiaY()) << "," << max(std::numeric_limits<double>::epsilon(), beam.getMomentOfInertiaZ())
				<< "," << max(std::numeric_limits<double>::epsilon(), beam.getTorsionalConstant()) << ",";
            if (! is_zero(beam.getShearAreaFactorY()))
                out << beam.getShearAreaFactorY();
            else
                out << 0.0;
            out << ",";
            if (! is_zero(beam.getShearAreaFactorZ()))
                out << beam.getShearAreaFactorZ();
            else
                out << 0.0;
		} else {
            out << max(std::numeric_limits<double>::epsilon(), beam.getAreaCrossSection()) << ",";
		}

		out << ")," << endl;
		beam.markAsWritten();
	}
	out << "                               )," << endl;
}

void AsterWriter::writeAffeCharMeca(const AsterModel& asterModel, ostream& out) {
	for (const auto& it : asterModel.model.constraintSets) {
		ConstraintSet& constraintSet = *it;
		if (constraintSet.getConstraints().size() == 0) {
			//GC fix for http://hotline.alneos.fr/redmine/issues/801.
			//What is the meaning of an empty constraintset?
			//maybe it should be fixed elsewhere
			constraintSet.markAsWritten();
			continue;
		}
		bool contactOnly = true;
		for(const auto& constraint : constraintSet.getConstraints()) {
		    if (not constraint->isContact()) {
		        contactOnly = false;
                break;
		    }
		}
		if (contactOnly) {
			continue;
		}
		if (constraintSet.isOriginal()) {
			out << "# ConstraintSet : " << constraintSet << endl;
		}
		for(bool withFunctions : {false, true} ) {
            string asterName;
            if (withFunctions and constraintSet.hasFunctions()) {
                asterName = string("BLF") + to_string(constraintSet.getId());
                out << asterName << "=AFFE_CHAR_MECA_F(MODELE=MODMECA," << endl;
            } else if (not withFunctions and not constraintSet.hasFunctions()) {
                asterName = string("BL") + to_string(constraintSet.getId());
                out << asterName << "=AFFE_CHAR_MECA(MODELE=MODMECA," << endl;
            } else {
                continue;
            }
            asternameByConstraintSet[constraintSet] = asterName;

            writeSPC(asterModel, constraintSet, out);
            writeLIAISON_SOLIDE(asterModel, constraintSet, out);
            writeLIAISON_MAIL(asterModel, constraintSet, out);
            writeRBE3(asterModel, constraintSet, out);
            writeLMPC(asterModel, constraintSet, out);
            out << "                   );" << endl << endl;
		}
		constraintSet.markAsWritten();
	}

	for (const auto& it : asterModel.model.loadSets) {
		LoadSet& loadSet = *it;
		if (loadSet.type == LoadSet::Type::DLOAD) {
			continue;
		}
		if (loadSet.getLoadings().size() == 0) {
            loadSet.markAsWritten();
			continue;
		}
		if (loadSet.getLoadings().size()
				== loadSet.getLoadingsByType(Loading::Type::INITIAL_TEMPERATURE).size()) {
			out << "# Ignoring INITIAL_TEMPERATURES!!!!!!" << endl;
			cout << "!!!!!!Ignoring INITIAL_TEMPERATURES!!!!!!" << endl;
			continue;
		}
		if (loadSet.isOriginal()) {
			out << "# LoadSet " << loadSet << endl;
		}
		for(bool withFunctions : {false, true} ) {
            string asterName;
		    if (withFunctions and loadSet.hasFunctions()) {
                asterName = string("CHMEF") + to_string(loadSet.getId());
                out << asterName << "=AFFE_CHAR_MECA_F(MODELE=MODMECA," << endl;
		    } else if (not withFunctions and not loadSet.hasFunctions()) {
		        asterName = string("CHMEC") + to_string(loadSet.getId());
                out << asterName << "=AFFE_CHAR_MECA(MODELE=MODMECA," << endl;
            } else {
                continue;
            }
            out << "                 VERI_NORM='NON'," << endl; // Workaround for PREPOST4_97 see test pload4-ctetra-multi
            asternameByLoadSet[loadSet] = asterName;
            writeSPCD(asterModel, loadSet, out);
            writePression(loadSet, out);
            writeForceCoque(loadSet, out);
            writeNodalForce(asterModel, loadSet, out);
            writeForceSurface(loadSet, out);
            writeForceLine(loadSet, out);
            writeGravity(loadSet, out);
            writeRotation(loadSet, out);
            out << "                      );" << endl << endl;
		}
		loadSet.markAsWritten();
	}
}

void AsterWriter::writeDefiContact(const AsterModel& asterModel, ostream& out) {
	for (const auto& it : asterModel.model.constraintSets) {
		ConstraintSet& constraintSet = *it;
		if (constraintSet.getConstraints().size() == 0) {
			// LD filter empty constraintSet
			continue;
		}
		if (not constraintSet.hasContacts()) {
			continue;
		}
		const auto& gaps = constraintSet.getConstraintsByType(Constraint::Type::GAP);
        const auto& slides = constraintSet.getConstraintsByType(Constraint::Type::SLIDE);
        const auto& surfaces = constraintSet.getConstraintsByType(Constraint::Type::SURFACE_CONTACT);
//        const auto& surfaceSlides = constraintSet.getConstraintsByType(
//				Constraint::Type::SURFACE_SLIDE_CONTACT);
        const auto& zones = constraintSet.getConstraintsByType(Constraint::Type::ZONE_CONTACT);
		for (shared_ptr<Constraint> constraint : gaps) {
			shared_ptr<const Gap> gap = dynamic_pointer_cast<const Gap>(constraint);
			int gapCount = 0;
			for (shared_ptr<Gap::GapParticipation> gapParticipation : gap->getGaps()) {
				gapCount++;
				out << "C" << constraintSet.getId() << "I" << to_string(gapCount)
						<< "=DEFI_CONSTANTE(VALE=" << gap->initial_gap_opening << ")" << endl;
				if (!is_zero(gapParticipation->direction.x())) {
					out << "C" << constraintSet.getId() << "MX" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.x() << ")"
							<< endl;
				}
				if (!is_zero(gapParticipation->direction.y())) {
					out << "C" << constraintSet.getId() << "MY" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.y() << ")"
							<< endl;
				}
				if (!is_zero(gapParticipation->direction.y())) {
					out << "C" << constraintSet.getId() << "MZ" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.z() << ")"
							<< endl;
				}
			}
		}
		string asterName = string("CN") + to_string(constraintSet.getId());
		asternameByConstraintSet[constraintSet] = asterName;
		if (constraintSet.isOriginal()) {
            out << "# ConstraintSet original id:" << constraintSet.getOriginalId() << endl;
		}
		out << asterName << "=DEFI_CONTACT(MODELE=MODMECA," << endl;
		if (gaps.size() >= 1) {
            out << "                   FORMULATION='LIAISON_UNIL'," << endl;
		} else if (slides.size() >= 1) {
		    out << "                   FORMULATION='CONTINUE'," << endl;
		    out << "                   FROTTEMENT='COULOMB'," << endl;
		} else if (surfaces.size() >= 1) {
		    out << "                   FORMULATION='CONTINUE'," << endl;
		}
		out << "                   ZONE=(" << endl;
		for (shared_ptr<Constraint> constraint : gaps) {
			shared_ptr<Gap> gap = dynamic_pointer_cast<Gap>(constraint);
			int gapCount = 0;
			for (shared_ptr<Gap::GapParticipation> gapParticipation : gap->getGaps()) {
				gapCount++;
				out << "                             _F(";
				out << "NOEUD='"
						<< Node::MedName(gapParticipation->nodePosition)
						<< "',";
				out << "COEF_IMPO=" << "C" << constraintSet.getId() << "I" << to_string(gapCount)
						<< ",";
				out << "COEF_MULT=(";
				if (!is_zero(gapParticipation->direction.x() )) {
					out << "C" << constraintSet.getId() << "MX" << to_string(gapCount) << ",";
				}
				if (!is_zero(gapParticipation->direction.y() )) {
					out << "C" << constraintSet.getId() << "MY" << to_string(gapCount) << ",";
				}
				if (!is_zero(gapParticipation->direction.z())) {
					out << "C" << constraintSet.getId() << "MZ" << to_string(gapCount);
				}
				out << "),";
				out << "NOM_CMP=(";
				if (!is_zero(gapParticipation->direction.x())) {
					out << "'DX',";
				}
				if (!is_zero(gapParticipation->direction.y() )) {
					out << "'DY',";
				}
				if (!is_zero(gapParticipation->direction.z() )) {
					out << " 'DZ'";
				}
				out << "),";
				out << ")," << endl;
			}
			gap->markAsWritten();
		}
		for (shared_ptr<Constraint> constraint : slides) {
		    shared_ptr<SlideContact> slide = dynamic_pointer_cast<SlideContact>(constraint);
            out << "                             _F(";
            out << "GROUP_MA_MAIT='"
                    << slide->masterCellGroup->getName()
                    << "',";
            out << "GROUP_MA_ESCL='"
                    << slide->slaveCellGroup->getName()
                    << "',";
            out << "COULOMB=" << slide->getFriction() << ",";
            out << ")," << endl;
            slide->markAsWritten();
		}
//		for (shared_ptr<Constraint> constraint : surfaceSlides) {
//		    shared_ptr<const SurfaceSlide> surfaceSlide = dynamic_pointer_cast<const SurfaceSlide>(constraint);
//		    shared_ptr<const BoundaryElementFace> surfaceSlideMaster = dynamic_pointer_cast<const BoundaryElementFace>(asterModel.model.find(surfaceSlide->master));
//		    shared_ptr<const BoundaryElementFace> surfaceSlideSlave = dynamic_pointer_cast<const BoundaryElementFace>(asterModel.model.find(surfaceSlide->slave));
//                out << "                             _F(";
//				out << "GROUP_MA_MAIT='"
//						<< surfaceSlideMaster->cellGroup->getName()
//						<< "',";
//				out << "GROUP_MA_ESCL='"
//						<< surfaceSlideSlave->cellGroup->getName()
//						<< "',";
//                out << "GLISSIERE='OUI',";
//				out << ")," << endl;
//		}
		for (shared_ptr<Constraint> constraint : surfaces) {
		    shared_ptr<SurfaceContact> surface = dynamic_pointer_cast<SurfaceContact>(constraint);
            out << "                             _F(";
            out << "GROUP_MA_MAIT='"
                    << surface->masterCellGroup->getName()
                    << "',";
            out << "GROUP_MA_ESCL='"
                    << surface->slaveCellGroup->getName()
                    << "',";
            out << ")," << endl;
            surface->markAsWritten();
		}
		for (shared_ptr<Constraint> constraint : zones) {
		    shared_ptr<ZoneContact> zone = dynamic_pointer_cast<ZoneContact>(constraint);
		    shared_ptr<ContactBody> master = dynamic_pointer_cast<ContactBody>(asterModel.model.find(zone->master));
		    shared_ptr<BoundarySurface> masterSurface = dynamic_pointer_cast<BoundarySurface>(asterModel.model.find(master->boundary));
		    shared_ptr<ContactBody> slave = dynamic_pointer_cast<ContactBody>(asterModel.model.find(zone->slave));
		    shared_ptr<BoundarySurface> slaveSurface = dynamic_pointer_cast<BoundarySurface>(asterModel.model.find(slave->boundary));
            out << "                             _F(";

            out << "GROUP_MA_MAIT=(";
                for(const auto& cellGroup : masterSurface->getCellGroups()) {
                    out << "'" << cellGroup->getName() << "',";
                }
                out << "),";
            out << "GROUP_MA_ESCL=(";
                for(const auto& cellGroup : slaveSurface->getCellGroups()) {
                    out << "'" << cellGroup->getName() << "',";
                }
                out << "),";
            out << ")," << endl;
            zone->markAsWritten();
            master->markAsWritten();
            masterSurface->markAsWritten();
            slave->markAsWritten();
            slaveSurface->markAsWritten();
		}
		out << "                             )," << endl;
		out << "                   );" << endl << endl;
		constraintSet.markAsWritten();
	}
}

void AsterWriter::writeSPC(const AsterModel& asterModel, const ConstraintSet& cset,
		ostream&out) {
    UNUSEDV(asterModel);
	const auto& spcs = cset.getConstraintsByType(Constraint::Type::SPC);
	if (spcs.size() > 0) {
		out << "                   DDL_IMPO=(" << endl;
		for (shared_ptr<Constraint> constraint : spcs) {
			shared_ptr<SinglePointConstraint> spc = dynamic_pointer_cast<
					SinglePointConstraint>(constraint);
			//FIXME: filter spcs with type function.
			if (spc->hasReferences()) {
				cerr << "SPC references not supported " << *spc << endl;
				out << " ************************" << endl << "SPC references not supported "
						<< *spc
						<< endl;
			} else {
				out << "                             _F(";
				if (spc->group == nullptr) {
					out << "NOEUD=(";
					for (int nodePosition : spc->nodePositions()) {
						out << "'"
								<< Node::MedName(nodePosition)
								<< "', ";
					}
					out << "),";
				} else {
					out << "GROUP_NO='" << spc->group->getName() << "',";
				}
				//parameter 0 ignored
				for (const DOF dof : spc->getDOFSForNode(0)) {
					if (dof == DOF::DX)
						out << "DX";
					if (dof == DOF::DY)
						out << "DY";
					if (dof == DOF::DZ)
						out << "DZ";
					if (dof == DOF::RX)
						out << "DRX";
					if (dof == DOF::RY)
						out << "DRY";
					if (dof == DOF::RZ)
						out << "DRZ";
					out << "=" << spc->getDoubleForDOF(dof) << ", ";
				}
				out << ")," << endl;
			}
			spc->markAsWritten();
		}
		out << "                             )," << endl;
	}
}

void AsterWriter::writeSPCD(const AsterModel& asterModel, const LoadSet& lset,
		ostream&out) {
    UNUSEDV(asterModel);
	const auto& spcds = lset.getLoadingsByType(Loading::Type::IMPOSED_DISPLACEMENT);
	if (spcds.size() > 0) {
		out << "                   DDL_IMPO=(" << endl;
		for (shared_ptr<Loading> loading : spcds) {
			shared_ptr<ImposedDisplacement> spcd = dynamic_pointer_cast<ImposedDisplacement>(loading);
            out << "                             _F(";
            writeNodeContainer(*spcd, out);
            for (const DOF dof : spcd->getDOFSForNode(0)) { // parameter 0 ignored
                if (dof == DOF::DX)
                    out << "DX";
                if (dof == DOF::DY)
                    out << "DY";
                if (dof == DOF::DZ)
                    out << "DZ";
                if (dof == DOF::RX)
                    out << "DRX";
                if (dof == DOF::RY)
                    out << "DRY";
                if (dof == DOF::RZ)
                    out << "DRZ";
                out << "=" << spcd->getDoubleForDOF(dof) << ", ";
            }
            out << ")," << endl;
            spcd->markAsWritten();
		}
		out << "                             )," << endl;
	}
}

void AsterWriter::writeLIAISON_SOLIDE(const AsterModel& asterModel, const ConstraintSet& cset,
		ostream& out) {
  UNUSEDV(asterModel);

	const auto& rigidConstraints = cset.getConstraintsByType(Constraint::Type::RIGID);
	const auto& quasiRigidConstraints = cset.getConstraintsByType(Constraint::Type::QUASI_RIGID);
	vector<shared_ptr<Constraint>> constraints;
	constraints.reserve(rigidConstraints.size() + quasiRigidConstraints.size());
	constraints.assign(rigidConstraints.begin(), rigidConstraints.end());
	for (shared_ptr<Constraint> quasiRigidConstraint : quasiRigidConstraints) {
		if ((dynamic_pointer_cast<QuasiRigidConstraint>(quasiRigidConstraint)->isCompletelyRigid())) {
			constraints.push_back(quasiRigidConstraint);
		}
	}
	bool needLiaisonSolide = constraints.size() > 0;

	if (needLiaisonSolide) {
		out << "                   LIAISON_SOLIDE=(" << endl;
		for (const auto& constraintPtr : constraints) {
			out << "                                   _F(NOEUD=(";
			for (int node : constraintPtr->nodePositions()) {
				out << "'" << Node::MedName(node) << "',";
			}
			out << ")," << endl;
			out << "                                      )," << endl;
			constraintPtr->markAsWritten();

		}
		out << "                                   )," << endl;
	}
}

void AsterWriter::writeLIAISON_MAIL(const AsterModel& asterModel, const ConstraintSet& cset,
		ostream& out) {

	const auto& slideSurfaces = cset.getConstraintsByType(
			Constraint::Type::SURFACE_SLIDE_CONTACT);

	if (slideSurfaces.size() >= 1) {
		out << "                   LIAISON_MAIL=(" << endl;
        for (shared_ptr<Constraint> constraint : slideSurfaces) {
		    shared_ptr<SurfaceSlide> surface = dynamic_pointer_cast<SurfaceSlide>(constraint);
		    shared_ptr<BoundaryElementFace> masterSurface = dynamic_pointer_cast<BoundaryElementFace>(asterModel.model.find(surface->master));
		    shared_ptr<BoundaryElementFace> slaveSurface = dynamic_pointer_cast<BoundaryElementFace>(asterModel.model.find(surface->slave));
			out << "                                   _F(TYPE_RACCORD='MASSIF', DDL_MAIT='DNOR', DDL_ESCL='DNOR',";
			out << "GROUP_MA_MAIT='" << masterSurface->elementCellGroup->getName() << "',";
			out << "GROUP_MA_ESCL='" << slaveSurface->surfaceCellGroup->getName() << "',";
            out << ")," << endl;
            surface->markAsWritten();
            masterSurface->markAsWritten();
            slaveSurface->markAsWritten();
		}
		out << "                                   )," << endl;
	}
}

void AsterWriter::writeRBE3(const AsterModel& asterModel, const ConstraintSet& cset,
		ostream& out) {
    UNUSEDV(asterModel);
	const auto& constraints = cset.getConstraintsByType(Constraint::Type::RBE3);
	if (constraints.size() > 0) {
		out << "                   LIAISON_RBE3=(" << endl;
		for (const auto& constraint : constraints) {
			shared_ptr<RBE3> rbe3 = dynamic_pointer_cast<RBE3>(constraint);
			int masterNode = rbe3->getMaster();
			out << "                                 _F(NOEUD_MAIT='"
					<< Node::MedName(masterNode) << "',"
					<< endl;
			out << "                                    DDL_MAIT=(";
			DOFS dofs = rbe3->getDOFSForNode(masterNode);
			if (dofs.contains(DOF::DX))
				out << "'DX',";
			if (dofs.contains(DOF::DY))
				out << "'DY',";
			if (dofs.contains(DOF::DZ))
				out << "'DZ',";
			if (dofs.contains(DOF::RX))
				out << "'DRX',";
			if (dofs.contains(DOF::RY))
				out << "'DRY',";
			if (dofs.contains(DOF::RZ))
				out << "'DRZ',";
			out << ")," << endl;
			set<int> slaveNodes = rbe3->getSlaves();

			out << "                                    NOEUD_ESCL=(";
			for (int slaveNode : slaveNodes) {
				out << "'" << Node::MedName(slaveNode) << "',";
			}
			out << ")," << endl;
			out << "                                    DDL_ESCL=(";
			for (int slaveNode : slaveNodes) {
				DOFS slaveDofs = rbe3->getDOFSForNode(slaveNode);
				int size = 0;
				out << "'";
				if (slaveDofs.contains(DOF::DX)) {
					out << "DX";
					if (++size < slaveDofs.size())
						out << "-";
				}
				if (slaveDofs.contains(DOF::DY)) {
					out << "DY";
					if (++size < slaveDofs.size())
						out << "-";
				}
				if (slaveDofs.contains(DOF::DZ)) {
					out << "DZ";
					if (++size < slaveDofs.size())
						out << "-";
				}
				if (slaveDofs.contains(DOF::RX)) {
					out << "DRX";
					if (++size < slaveDofs.size())
						out << "-";
				}
				if (slaveDofs.contains(DOF::RY)) {
					out << "DRY";
					if (++size < slaveDofs.size())
						out << "-";
				}
				if (slaveDofs.contains(DOF::RZ)) {
					out << "DRZ";
				}
				out << "',";
			}
			out << ")," << endl;
			out << "                                    COEF_ESCL=(";
			for (int slaveNode : slaveNodes) {
				out << rbe3->getCoefForNode(slaveNode) << ",";
			}
			out << ")," << endl;
			out << "                                    )," << endl;
			rbe3->markAsWritten();
		}
		out << "                                 )," << endl;
	}
}

void AsterWriter::writeLMPC(const AsterModel& asterModel, const ConstraintSet& cset,
		ostream& out) {
    UNUSEDV(asterModel);
	const auto& lmpcs = cset.getConstraintsByType(Constraint::Type::LMPC);
	if (lmpcs.size() > 0) {
		out << "                   LIAISON_DDL=(" << endl;
		for (shared_ptr<Constraint> constraint : lmpcs) {
			shared_ptr<LinearMultiplePointConstraint> lmpc = dynamic_pointer_cast<LinearMultiplePointConstraint>(constraint);
			out << "                                _F(NOEUD=(";
			set<int> nodes = lmpc->nodePositions();
			for (int nodePosition : nodes) {
				string nodeName = Node::MedName(nodePosition);
				DOFS dofs = lmpc->getDOFSForNode(nodePosition);
				for (int i = 0; i < dofs.size(); i++) {
					out << "'" << nodeName << "', ";
				}
			}
			out << ")," << endl;
			out << "                                   DDL=(";
			for (int nodePosition : nodes) {
				DOFS dofs = lmpc->getDOFSForNode(nodePosition);
				if (dofs.contains(DOF::DX))
					out << "'DX', ";
				if (dofs.contains(DOF::DY))
					out << "'DY', ";
				if (dofs.contains(DOF::DZ))
					out << "'DZ', ";
				if (dofs.contains(DOF::RX))
					out << "'DRX', ";
				if (dofs.contains(DOF::RY))
					out << "'DRY', ";
				if (dofs.contains(DOF::RZ))
					out << "'DRZ', ";
			}
			out << ")," << endl;
			out << "                                   COEF_MULT=(";
			for (int nodePosition : nodes) {
			    DOFCoefs dofcoef = lmpc->getDoFCoefsForNode(nodePosition);
				for (int i = 0; i < 6; i++) {
					if (!is_zero(dofcoef[i]))
						out << dofcoef[i] << ", ";
				}
			}
			out << ")," << endl;
			out << "                                   COEF_IMPO=" << lmpc->coef_impo << "),"
					<< endl;
            lmpc->markAsWritten();
		}
		out << "                               )," << endl;
	}
}

void AsterWriter::writeGravity(const LoadSet& loadSet, ostream& out) {
	const auto& gravities = loadSet.getLoadingsByType(Loading::Type::GRAVITY);
	if (gravities.size() > 0) {
		out << "                      PESANTEUR=(" << endl;
		for (shared_ptr<Loading> loading : gravities) {
			shared_ptr<Gravity> gravity = dynamic_pointer_cast<Gravity>(loading);
			out << "                                 _F(GRAVITE=" << gravity->getAccelerationVector().norm()
					<< "," << endl;
			VectorialValue direction = gravity->getAccelerationVector().normalized();
			out << "                                    DIRECTION=(" << direction.x() << ","
					<< direction.y() << "," << direction.z() << "),)," << endl;
            gravity->markAsWritten();
		}
		out << "                                 )," << endl;
	}
}

void AsterWriter::writeRotation(const LoadSet& loadSet, ostream& out) {
	const auto& rotations = loadSet.getLoadingsByType(Loading::Type::ROTATION);
	if (rotations.size() > 0) {
		out << "                      ROTATION=(" << endl;
		for (shared_ptr<Loading> loading : rotations) {
			shared_ptr<Rotation> rotation = dynamic_pointer_cast<Rotation>(loading);
			out << "                                 _F(VITESSE=" << rotation->getSpeed() << ","
					<< endl;
			VectorialValue axis = rotation->getAxis();
			out << "                                    AXE=(" << axis.x() << "," << axis.y() << ","
					<< axis.z() << ")," << endl;
			VectorialValue center = rotation->getCenter();
			out << "                                    CENTRE=(" << center.x() << "," << center.y()
					<< "," << center.z() << ")";
			out << ",)," << endl;
			rotation->markAsWritten();
		}
		out << "                                 )," << endl;
	}
}

void AsterWriter::writeNodalForce(const AsterModel& asterModel, const LoadSet& loadSet, ostream& out) {
  UNUSEDV(asterModel);
	const auto& nodalForces = loadSet.getLoadingsByType(Loading::Type::NODAL_FORCE);
	if (nodalForces.size() > 0) {
		out << "                      FORCE_NODALE=(" << endl;
		for (shared_ptr<Loading> loading : nodalForces) {
			shared_ptr<NodalForce> nodal_force = dynamic_pointer_cast<NodalForce>(loading);
			for(const int nodePosition : nodal_force->nodePositions()) {
                VectorialValue force = nodal_force->getForceInGlobalCS(nodePosition);
                VectorialValue moment = nodal_force->getMomentInGlobalCS(nodePosition);
                out << "                                    _F(NOEUD='"
                        << Node::MedName(nodePosition) << "',";
                if (!is_zero(force.x()))
                    out << "FX=" << force.x() << ",";
                if (!is_zero(force.y()))
                    out << "FY=" << force.y() << ",";
                if (!is_zero(force.z()))
                    out << "FZ=" << force.z() << ",";
                if (!is_zero(moment.x()))
                    out << "MX=" << moment.x() << ",";
                if (!is_zero(moment.y()))
                    out << "MY=" << moment.y() << ",";
                if (!is_zero(moment.z()))
                    out << "MZ=" << moment.z() << ",";
                out << ")," << endl;
                nodal_force->markAsWritten();
			}
		}
		out << "                                    )," << endl;
	}
}

void AsterWriter::writePression(const LoadSet& loadSet, ostream& out) {
	const auto& loading = loadSet.getLoadingsByType(
			Loading::Type::NORMAL_PRESSION_FACE);
	if (loading.size() > 0) {
		out << "           PRES_REP=(" << endl;
		for (shared_ptr<Loading> pressionFace : loading) {
			shared_ptr<NormalPressionFace> normalPressionFace = dynamic_pointer_cast<
					NormalPressionFace>(pressionFace);
			out << "                         _F(PRES= " << normalPressionFace->intensity << ",";
			writeCellContainer(*normalPressionFace, out);
			out << "                         )," << endl;
			pressionFace->markAsWritten();
		}
		out << "                      )," << endl;
	}
}

void AsterWriter::writeForceCoque(const LoadSet& loadSet, ostream&out) {
    return; // TODO : change type of loading in parser to something specific for shell elements
	const auto& pressionFaces = loadSet.getLoadingsByType(Loading::Type::NORMAL_PRESSION_FACE);
	if (pressionFaces.size() > 0) {
		out << "           FORCE_COQUE=(" << endl;
		for (shared_ptr<Loading> pressionFace : pressionFaces) {
			shared_ptr<NormalPressionFace> normalPressionFace = dynamic_pointer_cast<
					NormalPressionFace>(pressionFace);
			out << "                        _F(PRES=" << normalPressionFace->intensity << ",";
			writeCellContainer(*normalPressionFace, out);
			out << "                         )," << endl;
			normalPressionFace->markAsWritten();
		}
		out << "            )," << endl;
	}
}

void AsterWriter::writeForceLine(const LoadSet& loadset, ostream& out) {
	const auto& forcesLine = loadset.getLoadingsByType(Loading::Type::FORCE_LINE);
	vector<shared_ptr<ForceLine>> forcesOnPoutres;
	vector<shared_ptr<ForceLine>> forcesOnGeometry;

	for (shared_ptr<Loading> loadingForceLine : forcesLine) {
		shared_ptr<ForceLine> forceLine = dynamic_pointer_cast<ForceLine>(loadingForceLine);
		if (forceLine->appliedToGeometry()) {
			forcesOnGeometry.push_back(forceLine);
		} else {
			forcesOnPoutres.push_back(forceLine);
		}
	}
	if (forcesOnPoutres.size() >= 1) {
		out << "           FORCE_POUTRE=(" << endl;
		for (shared_ptr<ForceLine> forceLine : forcesOnPoutres) {
            out << "                   _F(";
            switch(forceLine->dof.code) {
            case DOF::Code::DX_CODE:
                out << "FX";
                break;
            case DOF::Code::DY_CODE:
                out << "FY";
                break;
            case DOF::Code::DZ_CODE:
                out << "FZ";
                break;
            case DOF::Code::RX_CODE:
                out << "MX";
                break;
            case DOF::Code::RY_CODE:
                out << "MY";
                break;
            case DOF::Code::RZ_CODE:
                out << "MZ";
                break;
            default:
                handleWritingError("DOF not yet handled");
            }
            out << "=" << asternameByValue[forceLine->force] << ",";
            writeCellContainer(*forceLine, out);
            out << "          )," << endl;
            forceLine->markAsWritten();
		}
		out << "            )," << endl;
	}

	if (forcesOnGeometry.size() >= 1) {
		cerr << "ERROR! force_arete not implemented" << endl;
		out << "# warn! force_arete not implemented" << endl;
	}

}
void AsterWriter::writeForceSurface(const LoadSet& loadSet, ostream&out) {
	const auto& forceSurfaces = loadSet.getLoadingsByType(Loading::Type::FORCE_SURFACE);
	if (forceSurfaces.size() > 0) {
		out << "           FORCE_FACE=(" << endl;
		for (const auto& loading : forceSurfaces) {
			shared_ptr<ForceSurface> forceSurface = dynamic_pointer_cast<ForceSurface>(loading);
			VectorialValue force = forceSurface->getForce();
			VectorialValue moment = forceSurface->getMoment();
			out << "                       _F(";
			writeCellContainer(*forceSurface, out);
			if (!is_equal(force.x(), 0))
				out << "FX=" << force.x() << ",";
			if (!is_equal(force.y(),0))
				out << "FY=" << force.y() << ",";
			if (!is_equal(force.z(),0))
				out << "FZ=" << force.z() << ",";
			if (!is_equal(moment.x(), 0))
				out << "MX=" << moment.x() << ",";
			if (!is_equal(moment.y(), 0))
				out << "MY=" << moment.y() << ",";
			if (!is_equal(moment.z(), 0))
				out << "MZ=" << moment.z() << ",";
			out << ")," << endl;
			forceSurface->markAsWritten();
		}
		out << "            )," << endl;
	}
}

void AsterWriter::writeNodeContainer(const NodeContainer& nodeContainer, ostream& out) {
    int cnode = 0;
    if (nodeContainer.hasNodeGroups()) {
      out << "GROUP_NO=(";
      for (const auto& nodeGroup : nodeContainer.getNodeGroups()) {
        cnode++;
        out << "'" << nodeGroup->getName() << "',";
        if (cnode % 6 == 0) {
          out << endl << "                             ";
        }
      }
      out << "),";
    }
    if (nodeContainer.hasNodes()) {
      out << "NOEUD=(";
      for (int nodePosition : nodeContainer.getNodePositionsExcludingGroups()) {
        cnode++;
        out << "'" << Node::MedName(nodePosition) << "',";
        if (cnode % 6 == 0) {
          out << endl << "                             ";
        }
      }
      out << "),";
    }
}

void AsterWriter::writeCellContainer(const CellContainer& cellContainer, ostream& out) {
    int celem = 0;
    if (cellContainer.hasCellGroups()) {
      out << "GROUP_MA=(";
      for (const auto& cellGroup : cellContainer.getCellGroups()) {
        celem++;
        out << "'" << cellGroup->getName() << "',";
        if (celem % 6 == 0) {
          out << endl << "                             ";
        }
      }
      out << "),";
    }
    if (cellContainer.hasCells()) {
        // Creating single cell groups to avoid using MAILLE
      out << "MAILLE=(";
      //out << "GROUP_MA=(";
      for (int cellPosition : cellContainer.getCellPositionsExcludingGroups()) {
        celem++;
//        const string& groupName = Cell::MedName(cellPosition);
//        auto entry = singleGroupCellPositions.find(cellPosition);
//        if (entry == end(singleGroupCellPositions)) {
//            auto singleCellGroup = asterModel->model.mesh.createCellGroup(groupName);
//            singleCellGroup->addCellPosition(cellPosition);
//            singleGroupCellPositions.insert(cellPosition);
//        }
        out << "'" << Cell::MedName(cellPosition) << "',";
        if (celem % 6 == 0) {
          out << endl << "                             ";
        }
      }
      out << "),";
    }
}

shared_ptr<NonLinearStrategy> AsterWriter::getNonLinearStrategy(
		NonLinearMecaStat& nonLinAnalysis) {
	shared_ptr<NonLinearStrategy> nonLinearStrategy;
	shared_ptr<vega::Objective> strategy = nonLinAnalysis.model.find(
			nonLinAnalysis.strategy_reference);
    if (strategy == nullptr) {
        handleWritingError("Cannot find nonlinear strategy" + to_str(nonLinAnalysis.strategy_reference));
    }
	switch (strategy->type) {
	case Objective::Type::NONLINEAR_STRATEGY: {
		nonLinearStrategy = dynamic_pointer_cast<NonLinearStrategy>(strategy);
		break;
	}
	default:
		// Nothing to do
		break;
	}
	return nonLinearStrategy;
}

void AsterWriter::writeAssemblage(const AsterModel& asterModel, Analysis& analysis,
		ostream& out) {
    bool hasDynamicExcit = asterModel.model.loadSets.contains(LoadSet::Type::DLOAD);
    bool isBuckling = analysis.type == Analysis::Type::LINEAR_BUCKLING;
    out << "ASSEMBLAGE(MODELE=MODMECA," << endl;
    if (asterModel.model.materials.size() >= 1) {
        out << "           CHAM_MATER=CHMAT," << endl;
    }
    out << "           CARA_ELEM=CAEL," << endl;
    out << "           CHARGE=(" << endl;
    for (shared_ptr<ConstraintSet> constraintSet : analysis.getConstraintSets()) {
        out << "                   BL" << constraintSet->getId() << "," << endl;
    }
    out << "                   )," << endl;
    out << "           NUME_DDL=CO('NUMDDL" << analysis.getId() << "')," << endl;
    out << "           MATR_ASSE=(_F(OPTION='RIGI_MECA', MATRICE=CO('RIGI"
            << analysis.getId() << "'),)," << endl;
    if (isBuckling) {
        out << "                      _F(OPTION='RIGI_GEOM', MATRICE=CO('RIGE"
                << analysis.getId() << "'),SIEF_ELGA=FSIG" << analysis.getId() << ",)," << endl;
    } else {
        out << "                      _F(OPTION='MASS_MECA', MATRICE=CO('MASS"
                << analysis.getId() << "'),)," << endl;
        out << "                      _F(OPTION='AMOR_MECA', MATRICE=CO('AMOR"
                << analysis.getId() << "'),)," << endl;
    }
    out << "                      )," << endl;
    if (hasDynamicExcit) {
        out << "           VECT_ASSE=(" << endl;
        for (shared_ptr<LoadSet> loadSet : analysis.getLoadSets()) {
            for (shared_ptr<Loading> loading : loadSet->getLoadings()) {
                if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
                    DynamicExcitation& dynamicExcitation =
                            dynamic_cast<DynamicExcitation&>(*loading);
                    out << "                      _F(OPTION='CHAR_MECA', VECTEUR=CO('FX"
                            << analysis.getId() << "_" << dynamicExcitation.getId() << "')," << endl;
                    out << "                        CHARGE=(" << endl;
                    out << "                                CHMEC"
                            << dynamicExcitation.getLoadSet()->getId() << "," << endl;
                    out << "                                )," << endl;
                    out << "                      )," << endl;
                    dynamicExcitation.markAsWritten();
                    loadSet->markAsWritten();
                }
            }
        }
        out << "             )," << endl;
    }
    out << "           );" << endl << endl;
}

void AsterWriter::writeCalcFreq(const AsterModel& asterModel, LinearModal& linearModal, ostream& out) {
    string suffix;
    bool isBuckling = linearModal.type == Analysis::Type::LINEAR_BUCKLING;
    if (isBuckling) {
        suffix = "CHAR_CRIT";
    } else {
        suffix = "FREQ";
    }
    FrequencySearch& frequencySearch = *(linearModal.getFrequencySearch());
    if (not isBuckling) {
        switch (frequencySearch.norm) {
        case(FrequencySearch::NormType::MASS): {
            out << "                       NORM_MODE=_F(NORME='MASS_GENE')," << endl;
            break;
        }
        case(FrequencySearch::NormType::MAX): {
            out << "                       NORM_MODE=_F(NORME='TRAN_ROTA')," << endl;
            break;
        }
        default:
            handleWritingError(
                    "Norm for frequency search " + to_string(static_cast<int>(frequencySearch.frequencyType)) + " not (yet) implemented");
        }
    }
    switch(frequencySearch.frequencyType) {
    case FrequencySearch::FrequencyType::BAND: {
        BandRange band = dynamic_cast<BandRange&>(*frequencySearch.getValue());
        double fstart = band.start;
        if (is_equal(fstart, Globals::UNAVAILABLE_DOUBLE)) {
            auto lower_cutoff_frequency = asterModel.model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY);
            if (lower_cutoff_frequency != asterModel.model.parameters.end()) {
                if (asterModel.model.configuration.logLevel >= LogLevel::TRACE) {
                    cout << "Parameter LOWER_CUTOFF_FREQUENCY present, redefining frequency band" << endl;
                }
                fstart = lower_cutoff_frequency->second;
            } else {
                fstart = 0.0;
            }
        }
        double fend = band.end;
        if (isBuckling and not is_equal(band.end, Globals::UNAVAILABLE_DOUBLE)) {
            fstart = -fstart;
            fend = -fend;
            swap(fstart, fend);
        }
        if (linearModal.use_power_iteration) {
            out << "                       OPTION='SEPARE'," << endl;
        } else if (isBuckling or (is_equal(fstart, Globals::UNAVAILABLE_DOUBLE) and is_equal(fend, Globals::UNAVAILABLE_DOUBLE))) {
            // CALC_CHAR_CRIT is not filtering eigenvalues correctly in Aster 13.6
            out << "                       OPTION='PLUS_PETITE'," << endl;
        } else if (is_equal(fend, Globals::UNAVAILABLE_DOUBLE)) {
            out << "                       OPTION='CENTRE'," << endl; // LD : could be replaced by PLUS_PETITE + FILTRE
        } else {
            out << "                       OPTION='BANDE'," << endl;
        }
        out << "                       CALC_" << suffix << "=_F(" << endl;
        if (not isBuckling and not is_equal(fstart, Globals::UNAVAILABLE_DOUBLE)) {
            out << "                                    " << suffix << "=(";
            out << fstart;
            if (is_equal(fend, Globals::UNAVAILABLE_DOUBLE)) {
                out << ", " << ")," << endl;
            } else {
                out << ", " << fend;
                out << ")," << endl;
            }
        } else if (not is_equal(band.maxsearch, Globals::UNAVAILABLE_DOUBLE)) {
            out << "                                    NMAX_" << suffix << "=" << band.maxsearch << endl;
        }

        out << "                                    )," << endl;
        break;
    }
    case FrequencySearch::FrequencyType::STEP: {
        StepRange frequencyStep = dynamic_cast<StepRange&>(*frequencySearch.getValue());
        if (linearModal.use_power_iteration) {
            out << "                       OPTION='SEPARE'," << endl; // Not using 'PROCHE' because it will always produce modes, even if not finding them
        } else {
            out << "                       OPTION='CENTRE'," << endl;
        }
        out << "                       CALC_" << suffix << "=_F(" << endl;
        out << "                                    " << suffix << "=(";
        for (double frequency = frequencyStep.start; frequency < frequencyStep.end; frequency += frequencyStep.step ) {
            out << frequency << ",";
        }
        out << ")," << endl;
        out << "                                    )," << endl;
        break;
    }
    case FrequencySearch::FrequencyType::LIST: {
      ListValue<double>& frequencyList = dynamic_cast<ListValue<double>&>(*frequencySearch.getValue());
      if (linearModal.use_power_iteration) {
          out << "                       OPTION='SEPARE'," << endl; // Not using 'PROCHE' because it will always produce modes, even if not finding them
      } else {
          out << "                       OPTION='CENTRE'," << endl;
      }
      out << "                       CALC_" << suffix << "=_F(" << endl;
      out << "                                    " << suffix << "=(";
      for (const double frequency : frequencyList.getList()) {
          out << frequency << ",";
      }
      out << ")," << endl;
      out << "                                    )," << endl;
      break;
    }
    default:
        handleWritingError(
                "Frequency search " + to_string(static_cast<int>(frequencySearch.frequencyType)) + " not (yet) implemented");
    }
}

double AsterWriter::writeAnalysis(const AsterModel& asterModel, Analysis& analysis,
		ostream& out, double debut) {
	if (analysis.isOriginal()) {
		out << "# Analysis original id : " << analysis.getOriginalId() << endl;
	}
	switch (analysis.type) {
    case Analysis::Type::COMBINATION: {
        Combination& combination = dynamic_cast<Combination&>(analysis);
        for (const auto& subPair : combination.coefByAnalysis) {
            out << "D" << combination.getId() << "_" << subPair.first.id << "=CREA_CHAMP(TYPE_CHAM='NOEU_DEPL_R'," << endl;
            out << "          OPERATION='EXTR'," << endl;
            out << "          RESULTAT=RESU" << subPair.first.id << "," << endl;
            out << "          NOM_CHAM='DEPL'," << endl;
            out << "          NUME_ORDRE=1,);" << endl << endl;
        }

        out << "DEP" << combination.getId() << "=CREA_CHAMP(TYPE_CHAM='NOEU_DEPL_R'," << endl;
        out << "          OPTION='DEPL'," << endl;
        out << "          OPERATION='ASSE'," << endl;
        out << "          MODELE=MODMECA," << endl;
        out << "          ASSE=(" << endl;
        for (const auto& subPair : combination.coefByAnalysis) {
            out << "                _F(TOUT='OUI'," << endl;
            out << "                   CHAM_GD=D" << combination.getId() << "_" << subPair.first.id << "," << endl;
            out << "                   CUMUL='OUI'," << endl;
            out << "                   COEF_R=" << subPair.second << ",)," << endl;
        }
        out << "                )," << endl;
        out << "          )" << endl << endl;

        out << "RESU" << combination.getId() << "=CREA_RESU(OPERATION='AFFE'," << endl;
        out << "              TYPE_RESU='MULT_ELAS'," << endl;
        out << "              NOM_CHAM='DEPL'," << endl;
        out << "              AFFE=_F(CHAM_GD=DEP" << combination.getId() << "," << endl;
        out << "                      MODELE=MODMECA," << endl;
        out << "                      CHAM_MATER=CHMAT," << endl;
        out << "                      )," << endl;
        out << "              )" << endl << endl;
        combination.markAsWritten();
        break;
    }
	case Analysis::Type::LINEAR_MECA_STAT: {
		LinearMecaStat& linearMecaStat = dynamic_cast<LinearMecaStat&>(analysis);

		out << "RESU" << linearMecaStat.getId() << "=MECA_STATIQUE(MODELE=MODMECA," << endl;
		if (asterModel.model.materials.size() >= 1) {
            out << "                    CHAM_MATER=CHMAT," << endl;
		}
		out << "                    CARA_ELEM=CAEL," << endl;
		out << "                    EXCIT=(" << endl;
		for (shared_ptr<LoadSet> loadSet : linearMecaStat.getLoadSets()) {
			out << "                           _F(CHARGE=" << asternameByLoadSet[loadSet] << ")," << endl;
		}
		for (shared_ptr<ConstraintSet> constraintSet : linearMecaStat.getConstraintSets()) {
			//GC: dirty fix for #801, a deeper analysis must be done
			if (constraintSet->getConstraints().size() > 0) {
                cout << "constraintSet:" << *constraintSet << " AsterName: " << asternameByConstraintSet[constraintSet] << "Size:" << constraintSet->size() << endl;
				out << "                           _F(CHARGE=" << asternameByConstraintSet[constraintSet] << "),"
						<< endl;
			}
		}
		out << "                           )," << endl;
        out << "                    SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS')," << endl;
        out << "                    OPTION='SIEF_ELGA'," << endl;
		out << "                    );" << endl << endl;
		linearMecaStat.markAsWritten();
		break;
	}
	case Analysis::Type::NONLINEAR_MECA_STAT: {
		NonLinearMecaStat& nonLinAnalysis = dynamic_cast<NonLinearMecaStat&>(analysis);
		shared_ptr<NonLinearStrategy> nonLinearStrategy = getNonLinearStrategy(nonLinAnalysis);
		if (!nonLinAnalysis.previousAnalysis) {
			debut = 0;
		}
		double fin = debut + 1.0;
		StepRange stepRange(asterModel.model, debut, nonLinearStrategy->number_of_increments, fin);
		debut = fin;
		string list_name = writeValue(stepRange, out);
		out << "LAUTO" << nonLinAnalysis.getId()
				<< "=DEFI_LIST_INST(METHODE='AUTO', DEFI_LIST=_F(LIST_INST=" << list_name << ",),);"
				<< endl;
		out << "RAMP" << nonLinAnalysis.getId()
				<< "=DEFI_FONCTION(NOM_PARA='INST', PROL_DROITE='LINEAIRE', VALE=("
				<< stepRange.start << ",0.0," << stepRange.end << ",1.0,));" << endl;
		if (nonLinAnalysis.previousAnalysis) {
			out << "IRAMP" << nonLinAnalysis.getId()
					<< "=DEFI_FONCTION(NOM_PARA='INST', PROL_DROITE='LINEAIRE', VALE=("
					<< stepRange.start << ",1.0," << stepRange.end << ",0.0,));" << endl;
		}
		out << "RESU" << nonLinAnalysis.getId() << "=STAT_NON_LINE(MODELE=MODMECA," << endl;
		if (asterModel.model.materials.size() >= 1) {
            out << "                    CHAM_MATER=CHMAT," << endl;
		}
		out << "                    CARA_ELEM=CAEL," << endl;
		out << "                    EXCIT=(" << endl;
		if (nonLinAnalysis.previousAnalysis) {
			for (shared_ptr<LoadSet> loadSet : nonLinAnalysis.previousAnalysis->getLoadSets()) {
				out << "                           _F(CHARGE=" << asternameByLoadSet[loadSet]
						<< ",FONC_MULT=IRAMP" << nonLinAnalysis.getId() << ")," << "# Original id:" << loadSet->getOriginalId() << endl;
			}
		}
		for (shared_ptr<LoadSet> loadSet : nonLinAnalysis.getLoadSets()) {
			out << "                           _F(CHARGE=" << asternameByLoadSet[loadSet]
					<< ",FONC_MULT=RAMP" << nonLinAnalysis.getId() << ")," << "# Original id:" << loadSet->getOriginalId() << endl;
		}
		for (shared_ptr<ConstraintSet> constraintSet : nonLinAnalysis.getConstraintSets()) {
			if (constraintSet->hasContacts()) {
				continue;
			}
			//GC: dirty fix for #801, a deeper analysis must be done
			if (constraintSet->getConstraints().size() >= 1) {
				out << "                           _F(CHARGE=" << asternameByConstraintSet[constraintSet] << "),"
						 << "# Original id:" << constraintSet->getOriginalId() << endl;
			}
		}
		out << "                           )," << endl;
		for (shared_ptr<ConstraintSet> constraintSet : asterModel.model.constraintSets) {
			if (constraintSet->getConstraints().size() == 0) {
				// LD filter empty constraintSet
				continue;
			}
			if (not constraintSet->hasContacts()) {
				continue;
			}
			out << "                    CONTACT=" << asternameByConstraintSet[constraintSet] << "," << "# Original id:" << constraintSet->getOriginalId() << endl;
		}
		out << "                    COMPORTEMENT=(" << endl;
		for (const auto& elementSet : asterModel.model.elementSets) {
			if (elementSet->material != nullptr and !elementSet->empty()) {
				out << "                          _F(";
				writeCellContainer(*elementSet, out);
                const shared_ptr<const Nature> hyelas = elementSet->material->findNature(
                        Nature::NatureType::NATURE_HYPERELASTIC);
                const shared_ptr<const Nature> binature = elementSet->material->findNature(
                        Nature::NatureType::NATURE_BILINEAR_ELASTIC);
                const shared_ptr<const Nature> nlelas = elementSet->material->findNature(
                        Nature::NatureType::NATURE_NONLINEAR_ELASTIC);
                if (binature) {
                    out << "RELATION='VMIS_ISOT_LINE',";
                } else if (nlelas) {
                    out << "RELATION='ELAS_VMIS_LINE',";
                } else if (hyelas) {
                    out << "RELATION='ELAS_HYPER',";
                    out << "DEFORMATION='GROT_GDEP',";
                } else if (asterModel.model.needsLargeDisplacements() and (elementSet->isBeam() or elementSet->isTruss())) {
                    out << "RELATION='ELAS_POUTRE_GR',";
                    out << "DEFORMATION='GROT_GDEP',";
                } else {
                    out << "RELATION='ELAS',";
                }
                out << ")," << endl;
			} else {
//				out << "# WARN Skipping material id " << *elementSet << " because no assignment"
//						<< endl;
			}

		}
		out << "                           )," << endl;
		out << "                    INCREMENT=_F(LIST_INST=LAUTO" << nonLinAnalysis.getId() << ",),"
				<< endl;
		out << "                    ARCHIVAGE=_F(LIST_INST=" << list_name << ",)," << endl;
		out << "                    NEWTON=_F(REAC_ITER=1,)," << endl;
		if (nonLinAnalysis.previousAnalysis) {
			out << "                    ETAT_INIT=_F(EVOL_NOLI =RESU"
					<< nonLinAnalysis.previousAnalysis->getId() << ")," << endl;
		}
		out << "                    SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS')," << endl;
		out << "                    );" << endl << endl;
		nonLinAnalysis.markAsWritten();
		break;
	}
	case Analysis::Type::LINEAR_DYNA_DIRECT_FREQ: {
        writeAssemblage(asterModel, analysis, out);
        auto structural_damping = asterModel.model.parameters.find(Model::Parameter::STRUCTURAL_DAMPING);
        auto frequency_of_interest_radians = asterModel.model.parameters.find(Model::Parameter::FREQUENCY_OF_INTEREST_RADIANS);
        bool has_structural_damping = structural_damping != asterModel.model.parameters.end() and frequency_of_interest_radians != asterModel.model.parameters.end() and frequency_of_interest_radians->second > 0;
		if (has_structural_damping) {
            out << "AMST" << analysis.getId()
                <<  " = COMB_MATR_ASSE(COMB_R = _F ( MATR_ASSE = RIGI"
                << analysis.getId()
                << ", COEF_R = " << structural_damping->second / frequency_of_interest_radians->second << "))" << endl;
        }
		LinearDynaDirectFreq& linearDirect = dynamic_cast<LinearDynaDirectFreq&>(analysis);
        out << "RESU" << linearDirect.getId() << " = DYNA_VIBRA(" << endl;
        out << "                   TYPE_CALCUL='HARM'," << endl;
        out << "                   BASE_CALCUL='PHYS'," << endl;
		out << "                   MATR_MASS  = MASS" << linearDirect.getId() << ","
				<< endl;
		out << "                   MATR_RIGI  = RIGI" << linearDirect.getId() << ","
				<< endl;
        if (has_structural_damping) {
            out << "                   MATR_AMOR  = AMST" << linearDirect.getId() << ","
				<< endl;
        } else /* LD Maybe should combine these two cases ? */ {
            out << "                   MATR_AMOR  = AMOR" << linearDirect.getId() << ","
				<< endl;
        }
        out << "                   LIST_FREQ  = LST" << setfill('0') << setw(5)
        << linearDirect.getExcitationFrequencies()->getValue()->getId() << "," << endl;
		out << "                   EXCIT      = (" << endl;
		for (shared_ptr<LoadSet> loadSet : linearDirect.getLoadSets()) {
			for (shared_ptr<Loading> loading : loadSet->getLoadings()) {
				if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
					DynamicExcitation& dynamicExcitation =
							dynamic_cast<DynamicExcitation&>(*loading);
					out << "                                 _F(" << endl;
                    out << "                                    VECT_ASSE=FX"
                            << linearDirect.getId() << "_"
                            << dynamicExcitation.getId() << "," << endl;
					out << "                                    FONC_MULT = FCT" << setfill('0')
							<< setw(5) << dynamicExcitation.getFunctionTableB()->getId() << ","
							<< endl;
					out << "                                    PHAS_DEG = "
							<< dynamicExcitation.getDynaPhase()->get() << ",)," << endl;
                    dynamicExcitation.markAsWritten();
                    loadSet->markAsWritten();
				}
			}
		}
		out << "                                 )," << endl;
		out << "                   #SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS',NPREC=8)," << endl; // MUMPS: Error in function orderMinPriority no valid number of stages in multisector (#stages = 2)
		out << "                   );" << endl << endl;
		linearDirect.markAsWritten();
	    break;
	}
	case Analysis::Type::LINEAR_MODAL:
	case Analysis::Type::LINEAR_DYNA_MODAL_FREQ: {
        writeAssemblage(asterModel, analysis, out);

		LinearModal& linearModal = dynamic_cast<LinearModal&>(analysis);

		string resuName;
		if (analysis.type == Analysis::Type::LINEAR_MODAL)
			resuName = "RESU" + to_string(linearModal.getId());
		else
			resuName = "MODES" + to_string(linearModal.getId());
		out << "U" << resuName << "=CALC_MODES(MATR_RIGI=RIGI" << linearModal.getId()
				<< "," << endl;
		out << "                       MATR_MASS=MASS" << linearModal.getId() << "," << endl;
		if (linearModal.use_power_iteration) {
            out << "                       SOLVEUR_MODAL=_F(OPTION_INV='DIRECT')," << endl;
		} else {
            out << "                       SOLVEUR_MODAL=_F(METHODE='TRI_DIAG')," << endl;
		}
        writeCalcFreq(asterModel, linearModal, out);
		out << "                       VERI_MODE=_F(STOP_ERREUR='NON',)," << endl;
		//out << "                       IMPRESSION=_F(CUMUL='OUI',CRIT_EXTR='MASS_EFFE_UN',TOUT_PARA='OUI')," << endl;
		out << "                       SOLVEUR=_F(METHODE='MUMPS'," << endl;
		out << "                                  RENUM='PORD'," << endl;
		out << "                                  NPREC=8," << endl;
		out << "                                  )," << endl;
		out << "                       )" << endl << endl;

		double lowFreq = 0;
		if (asterModel.model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY) != asterModel.model.parameters.end()) {
		    lowFreq = asterModel.model.parameters[Model::Parameter::LOWER_CUTOFF_FREQUENCY];
		}

		double highFreq = 1e30;
		if (asterModel.model.parameters.find(Model::Parameter::UPPER_CUTOFF_FREQUENCY) != asterModel.model.parameters.end()) {
		    highFreq = asterModel.model.parameters[Model::Parameter::UPPER_CUTOFF_FREQUENCY];
		}
		out << resuName << "=EXTR_MODE(FILTRE_MODE=_F(MODE=U" << resuName << ", FREQ_MIN=" << lowFreq << ", FREQ_MAX=" << highFreq << "),)" << endl;

		out << "I" << resuName << "=RECU_TABLE(CO=" << resuName << ",NOM_PARA = ('FREQ','MASS_GENE','RIGI_GENE','AMOR_GENE'))" << endl;
        out << "IMPR_TABLE(TABLE=I" << resuName << ")" << endl << endl;
        out << "J" << resuName << "=POST_ELEM(RESULTAT=" << resuName << ", MASS_INER=_F(TOUT='OUI'))" << endl;
        out << "IMPR_TABLE(TABLE=J" << resuName << ")" << endl << endl;

		if (analysis.type == Analysis::Type::LINEAR_MODAL) {
            linearModal.markAsWritten();
			break;
		}

		LinearDynaModalFreq& linearDynaModalFreq = dynamic_cast<LinearDynaModalFreq&>(analysis);

		if (linearDynaModalFreq.residual_vector) {
			out << "MOSTA" << linearDynaModalFreq.getId() << "=MODE_STATIQUE(MATR_RIGI=RIGI"
					<< linearDynaModalFreq.getId() << "," << endl;
			out << "                     MATR_MASS=MASS" << linearDynaModalFreq.getId() << ","
					<< endl;
			out << "                     FORCE_NODALE=(" << endl;
			for (shared_ptr<LoadSet> loadSet : linearDynaModalFreq.getLoadSets()) {
				for (const auto& loading : loadSet->getLoadings()) {
					if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
						DynamicExcitation& dynamicExcitation =
								dynamic_cast<DynamicExcitation&>(*loading);
						const auto& nodalForces =
								dynamicExcitation.getLoadSet()->getLoadingsByType(
										Loading::Type::NODAL_FORCE);
						for (const auto& loading2 : nodalForces) {
							shared_ptr<NodalForce> nodal_force = dynamic_pointer_cast<NodalForce>(
									loading2);
                            for(const int nodePosition : nodal_force->nodePositions()) {
                                VectorialValue force = nodal_force->getForceInGlobalCS(nodePosition);
                                VectorialValue moment = nodal_force->getMomentInGlobalCS(nodePosition);
                                out << "                                    _F(NOEUD='"
                                        << Node::MedName(nodePosition) << "'," << endl;
                                out << "                                      AVEC_CMP=(";
                                if (!is_equal(force.x(),0))
                                    out << "'DX',";
                                if (!is_equal(force.y(),0))
                                    out << "'DY',";
                                if (!is_equal(force.z(),0))
                                    out << "'DZ',";
                                if (!is_equal(moment.x(),0))
                                    out << "'DRX',";
                                if (!is_equal(moment.y(),0))
                                    out << "'DRY',";
                                if (!is_equal(moment.z(),0))
                                    out << "'DRZ',";
                                out << "))," << endl;
                            }
                            dynamicExcitation.markAsWritten();
                            loadSet->markAsWritten();
						}
					}
				}
			}
			out << "                                   )" << endl;
			out << "                     );" << endl << endl;

			out << "RESVE" << linearDynaModalFreq.getId()
					<< "=DEFI_BASE_MODALE(RITZ =(_F(MODE_MECA=MODES" << linearDynaModalFreq.getId()
					<< ",)," << endl;
			out << "                               _F(MODE_INTF=MOSTA"
					<< linearDynaModalFreq.getId() << ",)," << endl;
			out << "                               )," << endl;
			out << "                        NUME_REF=NUMDDL" << linearDynaModalFreq.getId() << ","
					<< endl;
			out << "                        MATRICE=MASS" << linearDynaModalFreq.getId() << ","
					<< endl;
			out << "                        ORTHO='OUI'," << endl;
			out << "                        );" << endl << endl;

			out << "modes" << linearDynaModalFreq.getId() << "=" << "RESVE"
					<< linearDynaModalFreq.getId() << endl;
		} else {
			out << "modes" << linearDynaModalFreq.getId() << "=" << "MODES"
					<< linearDynaModalFreq.getId() << endl;
		}

        out << "PROJ_BASE(BASE=modes" << linearDynaModalFreq.getId() << "," << endl;
        out << "          MATR_ASSE_GENE=(_F(MATRICE=CO('MASSG" << linearDynaModalFreq.getId()
                << "')," << endl;
        out << "                             MATR_ASSE=MASS" << linearDynaModalFreq.getId() << ",),"
                << endl;
        out << "                          _F(MATRICE=CO('RIGIG" << linearDynaModalFreq.getId()
                << "')," << endl;
        out << "                             MATR_ASSE=RIGI" << linearDynaModalFreq.getId() << ",),"
                << endl;
        if (linearDynaModalFreq.getModalDamping() == nullptr) {
            out << "                          _F(MATRICE=CO('AMORG" << linearDynaModalFreq.getId()
                    << "')," << endl;
            out << "                             MATR_ASSE=AMOR" << linearDynaModalFreq.getId() << ",),"
                    << endl;
        }
        out << "                          )," << endl;
        out << "          VECT_ASSE_GENE=(" << endl;
        for (shared_ptr<LoadSet> loadSet : linearDynaModalFreq.getLoadSets()) {
            for (shared_ptr<Loading> loading : loadSet->getLoadings()) {
                if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
                    DynamicExcitation& dynamicExcitation =
                            dynamic_cast<DynamicExcitation&>(*loading);
                    out << "                          _F(VECTEUR=CO('VG"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation.getId() << "')," << endl;
                    out << "                             VECT_ASSE=FX"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation.getId() << ",";
                    out << "TYPE_VECT=";
                    switch(dynamicExcitation.excitType) {
                    case DynamicExcitation::DynamicExcitationType::LOAD: {
                        out << "'FORC',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::DISPLACEMENT: {
                        out << "'DEPL',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::VELOCITY: {
                        out << "'VITE',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::ACCELERATION: {
                        out << "'ACCE',";
                        break;
                    };
                    default:
                        handleWritingError("Dynamic excitation type " + to_string(static_cast<int>(dynamicExcitation.excitType)) + " not (yet) implemented");
                    }
                    out << ")," << endl;
                    dynamicExcitation.markAsWritten();
                    loadSet->markAsWritten();
                }
            }
        }
        out << "                         )," << endl;
        out << "          );" << endl << endl;

		out << "LIMODE" << linearDynaModalFreq.getId() << "=RECU_TABLE(CO=MODES"
				<< linearDynaModalFreq.getId() << "," << endl;
		out << "                  NOM_PARA = 'FREQ');" << endl << endl;

		out << "pfreq" << linearDynaModalFreq.getId() << "= LIMODE" << linearDynaModalFreq.getId()
				<< ".EXTR_TABLE().values()['FREQ']" << endl;

    if (linearDynaModalFreq.getModalDamping() != nullptr) {
      out << "AMMO_I" << linearDynaModalFreq.getId() << "=CALC_FONC_INTERP(FONCTION = FCT"
          << setfill('0') << setw(5)
          << linearDynaModalFreq.getModalDamping()->getFunctionTable()->getId() << ","
          << endl;
      out << "                         VALE_PARA = pfreq" << linearDynaModalFreq.getId() << endl;
      out << "                         );" << endl << endl;

      out << "AMMO_T" << linearDynaModalFreq.getId()
          << "=CREA_TABLE(FONCTION=_F(FONCTION = AMMO_I" << linearDynaModalFreq.getId()
          << ")," << endl;
      out << "                   );" << endl << endl;

      out << "AMMO" << linearDynaModalFreq.getId() << "=AMMO_T" << linearDynaModalFreq.getId()
          << ".EXTR_TABLE().values()['TOUTRESU']" << endl;
    }

    out << "GENE" << linearDynaModalFreq.getId() << " = DYNA_VIBRA(" << endl;
    out << "                   TYPE_CALCUL='HARM'," << endl;
    out << "                   BASE_CALCUL='GENE'," << endl;
    out << "                   MATR_MASS  = MASSG" << linearDynaModalFreq.getId() << ","
            << endl;
    out << "                   MATR_RIGI  = RIGIG" << linearDynaModalFreq.getId() << ","
            << endl;
    if (linearDynaModalFreq.getModalDamping() != nullptr) {
        out << "                   AMOR_MODAL = _F(AMOR_REDUIT = AMMO"
            << linearDynaModalFreq.getId() << ",)," << endl;
    } else {
        // LD MATR_AMOR nullifies AMOR_MODAL
        out << "                   MATR_AMOR  = AMORG" << linearDynaModalFreq.getId() << ","
            << endl;
    }
    out << "                   LIST_FREQ  = LST" << setfill('0') << setw(5)
        << linearDynaModalFreq.getExcitationFrequencies()->getValue()->getId() << "," << endl;
		out << "                   EXCIT      = (" << endl;
		for (shared_ptr<LoadSet> loadSet : linearDynaModalFreq.getLoadSets()) {
			for (shared_ptr<Loading> loading : loadSet->getLoadings()) {
				if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
					DynamicExcitation& dynamicExcitation =
							dynamic_cast<DynamicExcitation&>(*loading);
					out << "                                 _F(" << endl;
                    out << "                                    VECT_ASSE_GENE = VG"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation.getId() << "," << endl;
					out << "                                    FONC_MULT = FCT" << setfill('0')
							<< setw(5) << dynamicExcitation.getFunctionTableB()->getId() << ","
							<< endl;
					out << "                                    PHAS_DEG = "
							<< dynamicExcitation.getDynaPhase()->get() << ",)," << endl;
                    dynamicExcitation.markAsWritten();
                    loadSet->markAsWritten();
				}
			}
		}
		out << "                                 )," << endl;
		out << "                   #SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS',NPREC=8)," << endl; // MUMPS: Error in function orderMinPriority no valid number of stages in multisector (#stages = 2)
		out << "                   );" << endl << endl;

        out << "RESU" << linearDynaModalFreq.getId() << " = REST_GENE_PHYS(RESU_GENE = GENE"
                << linearDynaModalFreq.getId() << "," << endl;
        out << "                       TOUT_ORDRE = 'OUI'," << endl;
        out << "                       NOM_CHAM = ('DEPL','VITE','ACCE')," << endl;
        out << "                       );" << endl << endl;
        linearDynaModalFreq.markAsWritten();
		break;
	}
    case Analysis::Type::LINEAR_BUCKLING: {
        LinearBuckling& linearBuckling = dynamic_cast<LinearBuckling&>(analysis);

        out << "FSIG" << linearBuckling.getId() << " = CREA_CHAMP(OPERATION='EXTR'," << endl;
        out << "            TYPE_CHAM='ELGA_SIEF_R'," << endl;
        out << "            RESULTAT=RESU" << linearBuckling.previousAnalysis->getId() << "," << endl;
        //out << "            NUME_ORDRE=1," << endl;
        out << "            NOM_CHAM='SIEF_ELGA',)" << endl << endl;

        writeAssemblage(asterModel, analysis, out);

        out << "RESU" << linearBuckling.getId() << "=CALC_MODES(MATR_RIGI=RIGI" << linearBuckling.getId() << "," << endl;
        out << "                 MATR_RIGI_GEOM=RIGE" << linearBuckling.getId() << "," << endl;
        out << "                 TYPE_RESU='MODE_FLAMB'," << endl;
		if (linearBuckling.use_power_iteration) {
            out << "                       SOLVEUR_MODAL=_F(OPTION_INV='DIRECT')," << endl;
		} else {
            out << "                       SOLVEUR_MODAL=_F(METHODE='TRI_DIAG')," << endl;
		}
        writeCalcFreq(asterModel, linearBuckling, out);
        out << "             VERI_MODE=_F(STOP_ERREUR='NON',)," << endl;
        out << "             SOLVEUR=_F(METHODE='MUMPS',)," << endl;
        out << "       )" << endl << endl;

        out << "RESU" << linearBuckling.getId() << " = NORM_MODE(reuse=RESU"<< linearBuckling.getId() << ",MODE=RESU" << linearBuckling.getId() << ",NORME='TRAN_ROTA',)" << endl;
        out << "TBCRT" << linearBuckling.getId() << " = RECU_TABLE(CO=RESU" << linearBuckling.getId() << ",NOM_PARA='CHAR_CRIT')" << endl;
        linearBuckling.markAsWritten();
        break;
    }
	default:
		handleWritingError(
				"Analysis " + Analysis::stringByType.at(analysis.type) + " not (yet) implemented");
	}
	return debut;
}

void AsterWriter::writeNodalDisplacementAssertion(const AsterModel& asterModel,
		const Assertion& assertion, ostream& out) const {
  UNUSEDV(asterModel);
	const NodalDisplacementAssertion& nda = dynamic_cast<const NodalDisplacementAssertion&>(assertion);
	bool relativeComparison = abs(nda.value) >= SMALLEST_RELATIVE_COMPARISON;
	out << "                     CRITERE = "
			<< (relativeComparison ? "'RELATIF'," : "'ABSOLU',") << endl;
	out << "                     NOEUD='" << Node::MedName(nda.nodePosition) << "'," << endl;
	out << "                     NOM_CMP    = '" << AsterModel::DofByPosition.at(nda.dof.position) << "'," << endl;
	out << "                     NOM_CHAM   = 'DEPL'," << endl;
	if (!is_equal(nda.instant, -1)) {
		out << "                     INST = " << nda.instant << "," << endl;
	} else {
		out << "                     NUME_ORDRE = 1," << endl;
	}
    out << "                     REFERENCE = 'SOURCE_EXTERNE'," << endl;
    out << "                     PRECISION = " << nda.tolerance << "," << endl;
	out << "                     VALE_REFE = " << nda.value << "," << endl;
	out << "                     VALE_CALC = " << (is_zero(nda.value) ? 1e-10 : nda.value) << "," << endl;
	out << "                     TOLE_MACHINE = (" << nda.tolerance << "," << 1e-5 << ")," << endl;

}

void AsterWriter::writeNodalComplexDisplacementAssertion(const AsterModel& asterModel,
		const Assertion& assertion, ostream& out) const {
  UNUSEDV(asterModel);
	const NodalComplexDisplacementAssertion& nda =
			dynamic_cast<const NodalComplexDisplacementAssertion&>(assertion);
	bool relativeComparison = abs(nda.value) >= SMALLEST_RELATIVE_COMPARISON;
	out << "                     CRITERE = "
			<< (relativeComparison ? "'RELATIF'," : "'ABSOLU',") << endl;
	out << "                     NOEUD='" << Node::MedName(nda.nodePosition) << "'," << endl;
	out << "                     NOM_CMP = '" << AsterModel::DofByPosition.at(nda.dof.position)
			<< "'," << endl;
	out << "                     NOM_CHAM = 'DEPL'," << endl;
	out << "                     FREQ = " << nda.frequency << "," << endl;
	out << "                     VALE_CALC_C = " << nda.value.real() << "+" << nda.value.imag()
			<< "j,";
	out << endl;
	out << "                     TOLE_MACHINE = (" << (relativeComparison ? nda.tolerance : 1e-5) << "," << 1e-5 << ")," << endl;
}

void AsterWriter::writeFrequencyAssertion(const Analysis& analysis, const Assertion& assertion, ostream& out) const {
	const FrequencyAssertion& frequencyAssertion = dynamic_cast<const FrequencyAssertion&>(assertion);
    bool isBuckling = analysis.type == Analysis::Type::LINEAR_BUCKLING;

    double lowFreq = 0;
    if (analysis.model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY) != analysis.model.parameters.end()) {
        lowFreq = analysis.model.parameters[Model::Parameter::LOWER_CUTOFF_FREQUENCY];
    }

    double highFreq = 1e30;
    if (analysis.model.parameters.find(Model::Parameter::UPPER_CUTOFF_FREQUENCY) != analysis.model.parameters.end()) {
        highFreq = analysis.model.parameters[Model::Parameter::UPPER_CUTOFF_FREQUENCY];
    }
    if (not isBuckling and (frequencyAssertion.cycles < lowFreq or frequencyAssertion.cycles > highFreq)) {
        return; // Cannot check this frequency: it will be excluded by results
    }
	string resuName = ((analysis.type == Analysis::Type::LINEAR_MODAL or analysis.type == Analysis::Type::LINEAR_BUCKLING) ? "RESU" : "MODES") + to_string(analysis.getId());
	string critere = (!is_zero(frequencyAssertion.cycles) ? "'RELATIF'," : "'ABSOLU',");
    out << "                  _F(RESULTAT=" << resuName << "," << endl;
	out << "                     CRITERE = " << critere << endl;
    if (isBuckling) {
        out << "                     PARA = 'CHAR_CRIT'," << endl;
        out << "                     NUME_MODE = " << analysis.getAssertions().size() - frequencyAssertion.number + 1 << "," << endl;
        out << "                     VALE_CALC = " << -frequencyAssertion.eigenValue << "," << endl;
    } else {
        out << "                     PARA = 'FREQ'," << endl;
        out << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
        out << "                     VALE_CALC = " << frequencyAssertion.cycles << "," << endl;
    }
	out << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
    out << "                     )," << endl;

    if (not isBuckling) {
        out << "                  _F(RESULTAT=" << resuName << "," << endl;
        out << "                     CRITERE = " << critere << endl;
        out << "                     PARA = 'MASS_GENE'," << endl;
        out << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
        out << "                     VALE_CALC = " << frequencyAssertion.generalizedMass << "," << endl;
        out << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
        out << "                     )," << endl;
        if (not is_equal(frequencyAssertion.generalizedMass, 1.0)) {
            // Do not check generalized stiffness k_g when generalize mass m_g is normalized
            // since it is the same as checking the frequency : (2*pi*f)**2=k_g/m_g
            // but the error would be squared
            out << "                  _F(RESULTAT=" << resuName << "," << endl;
            out << "                     CRITERE = " << critere << endl;
            out << "                     PARA = 'RIGI_GENE'," << endl;
            out << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
            out << "                     VALE_CALC = " << frequencyAssertion.generalizedStiffness << "," << endl;
            out << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
            out << "                     )," << endl;
        }
    }

}
}
} //end of namespace vega

