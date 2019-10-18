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

string AsterWriter::toString() const {
	return "AsterWriter";
}

string AsterWriter::writeModel(Model& model,
		const ConfigurationParameters &configuration) {
    if (configuration.outputSolver.getSolverName() != SolverName::CODE_ASTER) {
        handleWritingError("Translation required for a different solver : " + configuration.outputSolver.to_str() + ", so cannot write it.");
    }
	asterModel = make_unique<AsterModel>(model, configuration);
//string currentOutFile = asterModel->getOutputFileName();

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

	//comm_file_ofs.setf(ios::scientific);
 	comm_file_ofs.precision(DBL_DIG);

	exp_file_ofs.open(exp_path.c_str(), ios::trunc | ios::out);
	if (!exp_file_ofs.is_open()) {
		string message = "Can't open file " + exp_path + " for writing.";
		throw ios::failure(message);
	}
	this->writeExport();
	exp_file_ofs.close();

	comm_file_ofs.open(comm_path.c_str(), ios::out | ios::trunc);

	if (!comm_file_ofs.is_open()) {
		string message = "Can't open file " + comm_path + " for writing.";
		throw ios::failure(message);
	}
	this->writeComm();
	comm_file_ofs.close();

	MedWriter medWriter;
	medWriter.writeMED(model, med_path.c_str());
	return exp_path;
}

void AsterWriter::writeExport() {
	exp_file_ofs << "P actions make_etude" << endl;
	exp_file_ofs << "P mem_aster 100.0" << endl;
	exp_file_ofs << "P mode interactif" << endl;
	if (asterModel->model.analyses.empty()) {
		exp_file_ofs << "P copy_result_alarm no" << endl;
	}
	exp_file_ofs << "P nomjob " << asterModel->model.name << endl;
	exp_file_ofs << "P origine Vega++ " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << endl;
	exp_file_ofs << "P version " << asterModel->getAsterVersion() << endl;
	exp_file_ofs << "A memjeveux " << asterModel->getMemjeveux() << endl;
	exp_file_ofs << "A tpmax " << asterModel->getTpmax() << endl;
	exp_file_ofs << "F comm " << asterModel->getOutputFileName(".comm", false) << " D 1" << endl;
	exp_file_ofs << "F mail " << asterModel->getOutputFileName(".med", false) << " D 20" << endl;
	exp_file_ofs << "F mess " << asterModel->getOutputFileName(".mess", false) << " R 6" << endl;
	exp_file_ofs << "F resu " << asterModel->getOutputFileName(".resu", false) << " R 8" << endl;
	exp_file_ofs << "F rmed " << asterModel->getOutputFileName(".rmed", false) << " R 80" << endl;
	exp_file_ofs << "R repe " << asterModel->getOutputFileName("_repe_out", false) << " R 0" << endl;

}

void AsterWriter::writeImprResultats() {
	if (not asterModel->model.analyses.empty()) {

		for (const auto& analysis : asterModel->model.analyses) {

			const auto& vonMisesOutputs = asterModel->model.objectives.filter(Objective::Type::VONMISES_STRESS_OUTPUT);

			for (const auto& output : vonMisesOutputs) {
                const auto& vonMisesOutput = static_pointer_cast<VonMisesStressOutput>(output);
                comm_file_ofs << "RESU" << analysis->getId() << "=CALC_CHAMP(reuse=RESU" << analysis->getId() << ","
                        << endl;
                comm_file_ofs << "           RESULTAT=RESU" << analysis->getId() << "," << endl;
                comm_file_ofs << "           MODELE=MODMECA," << endl;
                comm_file_ofs << "           CONTRAINTE =('SIEQ_ELNO','SIEQ_NOEU')," << endl;
                writeCellContainer(*vonMisesOutput);
                comm_file_ofs << ")" << endl;
			}
		}

		comm_file_ofs << "IMPR_RESU(FORMAT='RESULTAT'," << endl;
		comm_file_ofs << "          RESU=(" << endl;
		for (const auto& analysis : asterModel->model.analyses) {
			switch (analysis->type) {
            case (Analysis::Type::COMBINATION):
			case (Analysis::Type::LINEAR_MECA_STAT): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_CHAM='DEPL'," << " VALE_MAX='OUI'," << " VALE_MIN='OUI',),"
						<< endl;
				break;
			}
			case (Analysis::Type::LINEAR_MODAL): {
				//comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
				//		<< ", TOUT_PARA='OUI', TOUT_CHAM='NON')," << endl;
				break;
			}
            case (Analysis::Type::LINEAR_BUCKLING): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_PARA='CHAR_CRIT', TOUT_CHAM='NON')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_PARA='FREQ', TOUT_CHAM='NON')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
				break;
			}
			case (Analysis::Type::NONLINEAR_MECA_STAT): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_CHAM='DEPL'," << " VALE_MAX='OUI'," << " VALE_MIN='OUI',),"
						<< endl;
				break;
			}
			default:
				comm_file_ofs << "# WARN analysis " << *analysis << " not supported. Skipping." << endl;
			}
		}
		comm_file_ofs << "                )," << endl;
		comm_file_ofs << "          );" << endl << endl;

		comm_file_ofs << "IMPR_RESU(FORMAT='MED',UNITE=80," << endl;
		comm_file_ofs << "          RESU=(" << endl;
		for (const auto& analysis : asterModel->model.analyses) {
			switch (analysis->type) {
            case (Analysis::Type::COMBINATION):
			case (Analysis::Type::LINEAR_MECA_STAT): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_CHAM=('DEPL'";
				if (calc_sigm) {
					comm_file_ofs << ",'" << sigm_noeu << "'";
				}
				comm_file_ofs << ",),)," << endl;
				break;
			}
			case (Analysis::Type::NONLINEAR_MECA_STAT):
			case (Analysis::Type::LINEAR_BUCKLING):
			case (Analysis::Type::LINEAR_MODAL): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", NOM_CHAM = 'DEPL',)," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", PARTIE='REEL')," << endl;
				break;
			}
			case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
				comm_file_ofs << "                _F(RESULTAT=MODES" << analysis->getId()
						<< ", NOM_CHAM = 'DEPL',)," << endl;
				comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
						<< ", PARTIE='REEL')," << endl;
				break;
			}
			default:
				comm_file_ofs << "# WARN analysis " << *analysis << " not supported. Skipping." << endl;
			}
		}
		comm_file_ofs << "                )," << endl;
		comm_file_ofs << "          );" << endl << endl;

		for (const auto& analysis : asterModel->model.analyses) {

			const auto& displacementOutputs = asterModel->model.objectives.filter(Objective::Type::NODAL_DISPLACEMENT_OUTPUT);
			comm_file_ofs << "RETB" << analysis->getId();
			if (not displacementOutputs.empty()) {
                comm_file_ofs << "=POST_RELEVE_T(ACTION=(" << endl;
                for (auto output : displacementOutputs) {
                    const auto& displacementOutput = static_pointer_cast<const NodalDisplacementOutput>(output);
                    comm_file_ofs << "                _F(INTITULE='DISP" << output->bestId() << "',OPERATION='EXTRACTION',RESULTAT=RESU" << analysis->getId() << ",";
                    writeNodeContainer(*displacementOutput);
                    comm_file_ofs << "NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                }
                comm_file_ofs << "),)" << endl << endl;
			} else {

                comm_file_ofs << "=CREA_TABLE(RESU=(" << endl;
                switch (analysis->type) {
                case (Analysis::Type::COMBINATION):
                case (Analysis::Type::LINEAR_MODAL):
                case (Analysis::Type::LINEAR_BUCKLING):
                case (Analysis::Type::NONLINEAR_MECA_STAT):
                case (Analysis::Type::LINEAR_MECA_STAT): {
                    comm_file_ofs << "                _F(RESULTAT=RESU" << analysis->getId()
                            << ",TOUT='OUI',NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                    break;
                }
                case (Analysis::Type::LINEAR_DYNA_DIRECT_FREQ): {
                    break;
                }
                case (Analysis::Type::LINEAR_DYNA_MODAL_FREQ): {
                    comm_file_ofs << "                _F(RESULTAT=MODES" << analysis->getId()
                            << ",TOUT='OUI',NOM_CHAM='DEPL',TOUT_CMP='OUI')," << endl;
                    break;
                }
                default:
                    comm_file_ofs << "# WARN analysis " << *analysis << " not supported. Skipping." << endl;
                }
			comm_file_ofs << "),)" << endl << endl;
			}

			comm_file_ofs << "unite=DEFI_FICHIER(ACTION='ASSOCIER'," << endl;
			comm_file_ofs << "             FICHIER='REPE_OUT/tbresu_" << analysis->getId() << ".csv')" << endl
					<< endl;

			comm_file_ofs << "IMPR_TABLE(TABLE=RETB" << analysis->getId() << "," << endl;
			comm_file_ofs << "           FORMAT='TABLEAU'," << endl;
			comm_file_ofs << "           UNITE=unite," << endl;
			comm_file_ofs << "           SEPARATEUR=' ,'," << endl;
			comm_file_ofs << "           TITRE='RESULTS',)" << endl << endl;

			comm_file_ofs << "DEFI_FICHIER(ACTION='LIBERER'," << endl;
			comm_file_ofs << "             UNITE=unite,)" << endl << endl;
			comm_file_ofs << "DETRUIRE(CONCEPT=(_F(NOM=unite),))" << endl << endl;
		}

		for (const auto& analysis : asterModel->model.analyses) {
		    bool hasRecoveryPoints = false;
            for (const auto& elementSet : asterModel->model.elementSets) {
                if (not elementSet->isBeam()) continue;
                const auto& beam = static_pointer_cast<Beam>(elementSet);
                if (not beam->recoveryPoints.empty()) {
                    hasRecoveryPoints = true;
                    break;
                }
            }
            if (not hasRecoveryPoints) continue;

            for (const auto& elementSet : asterModel->model.elementSets) {
                if (not elementSet->isBeam()) continue; // to avoid CALCUL_37 Le TYPE_ELEMENT MECA_BARRE  ne sait pas encore calculer l'option:  SIPO_ELNO.
                comm_file_ofs << "RESU" << analysis->getId() << "=CALC_CHAMP(reuse=RESU" << analysis->getId() << ","
                        << endl;
                comm_file_ofs << "           RESULTAT=RESU" << analysis->getId() << "," << endl;
                comm_file_ofs << "           MODELE=MODMECA," << endl;
                comm_file_ofs << "           CONTRAINTE =('SIPO_NOEU')," << endl;
                const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
                if (cellElementSet == nullptr) {
                    handleWritingError("ElementSet which is not a CellContainer should be written using cellPositions, to be implemented here");
                }
                writeCellContainer(*cellElementSet);
                comm_file_ofs << ")" << endl;
            }

            comm_file_ofs << "RCTB" << analysis->getId() << "=MACR_LIGN_COUPE(" << endl;
            comm_file_ofs << "            RESULTAT=RESU" << analysis->getId() << "," << endl;
            comm_file_ofs << "            NOM_CHAM='SIPO_NOEU'," << endl;
            comm_file_ofs << "            MODELE=MODMECA," << endl;
            comm_file_ofs << "            LIGN_COUPE=(" << endl;
            for (const auto& elementSet : asterModel->model.elementSets) {
                if (not elementSet->isBeam()) continue;
                const auto& beam = static_pointer_cast<Beam>(elementSet);
                for (const auto& recoveryPoint : beam->recoveryPoints) {
                    const VectorialValue& localCoords = recoveryPoint.getLocalCoords();
                    for (const Cell& cell : beam->getCellsIncludingGroups()) {
                        const Node& node1 = asterModel->model.mesh.findNode(cell.nodePositions[0]);
                        const VectorialValue& globalCoords = recoveryPoint.getGlobalCoords(cell.id);
                        comm_file_ofs << "                    _F(" << endl;
                        comm_file_ofs << "                        INTITULE='Cell " << cell.id << " stress recovery at (local):" << localCoords << ", global:" << globalCoords << "'," << endl;
                        comm_file_ofs << "                        NOM_CMP=('SN','SMFY','SMFZ','SVY','SVZ','SMT')," << endl;
                        comm_file_ofs << "                        TYPE='SEGMENT'," << endl;
                        comm_file_ofs << "                        DISTANCE_MAX=" << abs(max(localCoords.y(), localCoords.z()))*2 << "," << endl;
                        comm_file_ofs << "                        NB_POINTS=2," << endl;
                        comm_file_ofs << "                        COOR_ORIG=(" << globalCoords.x() << "," << globalCoords.y() << "," << globalCoords.z() << ")," << endl;
                        // TODO LD find a better solution here
                        comm_file_ofs << "                        COOR_EXTR=(" << node1.x << "," << node1.y << "," << node1.z << ")," << endl;
                        comm_file_ofs << "                    )," << endl;
                    }
                }
            }
            comm_file_ofs << "                    )" << endl;
			comm_file_ofs << "            )" << endl;

			int unit = 10 + analysis->getId();
            comm_file_ofs << "DEFI_FICHIER(ACTION='ASSOCIER'," << endl;
			comm_file_ofs << "             UNITE=" << unit << "," << endl;
			comm_file_ofs << "             FICHIER='REPE_OUT/tbrecup_" << analysis->getId() << ".csv')" << endl << endl;

            comm_file_ofs << "IMPR_TABLE(TABLE=RCTB" << analysis->getId() << "," << endl;
			comm_file_ofs << "           FORMAT='TABLEAU'," << endl;
			comm_file_ofs << "           UNITE=" << unit << "," << endl;
			comm_file_ofs << "           SEPARATEUR=' ,'," << endl;
			comm_file_ofs << "           TITRE='RESULTS',)" << endl << endl;

            comm_file_ofs << "DEFI_FICHIER(ACTION='LIBERER'," << endl;
			comm_file_ofs << "             UNITE=" << unit << ")" << endl << endl;
		}


	}
}

void AsterWriter::writeAnalyses() {
	double debut = 0;
	for (const auto& analysis : asterModel->model.analyses) {
		debut = writeAnalysis(*analysis, debut);

		if (calc_sigm) {
			comm_file_ofs << "RESU" << analysis->getId() << "=CALC_CHAMP(reuse=RESU" << analysis->getId() << ","
					<< endl;
			comm_file_ofs << "           RESULTAT=RESU" << analysis->getId() << "," << endl;
			comm_file_ofs << "           MODELE=MODMECA," << endl;
            if (not asterModel->model.materials.empty()) {
                comm_file_ofs << "           CHAM_MATER=CHMAT," << endl;
            }
            comm_file_ofs << "           CARA_ELEM=CAEL," << endl;
			comm_file_ofs << "           CONTRAINTE =('SIGM_ELNO')," << endl;
			comm_file_ofs << "           FORCE = 'REAC_NODA'," << endl;
			comm_file_ofs << ")" << endl;
		}

        bool calc_vmis = asterModel->model.objectives.contains(Objective::Type::NODAL_CELL_VONMISES_ASSERTION);
        if (calc_vmis) {
            comm_file_ofs << "RESU" << analysis->getId() << "=CALC_CHAMP(reuse=RESU" << analysis->getId() << ","
                    << endl;
            comm_file_ofs << "           RESULTAT=RESU" << analysis->getId() << "," << endl;
            comm_file_ofs << "           MODELE=MODMECA," << endl;
            comm_file_ofs << "           CRITERES =('SIEQ_ELNO','SIEQ_NOEU')," << endl;
            comm_file_ofs << "           TOUT='OUI'," << endl;
            comm_file_ofs << ")" << endl;
        }

		const auto& assertions = analysis->getAssertions();
		if (not assertions.empty()) {
			comm_file_ofs << "TEST_RESU(RESU = (" << endl;

			for (const auto& assertion : assertions) {
				switch (assertion->type) {
				case Objective::Type::NODAL_DISPLACEMENT_ASSERTION:
					comm_file_ofs << "                  _F(RESULTAT=RESU" << analysis->getId() << "," << endl;
					writeNodalDisplacementAssertion( dynamic_cast<const NodalDisplacementAssertion&>(*assertion));
                    comm_file_ofs << "                     )," << endl;
					break;
				case Objective::Type::FREQUENCY_ASSERTION:
					writeFrequencyAssertion(*analysis, dynamic_cast<const FrequencyAssertion&>(*assertion));
					break;
				case Objective::Type::NODAL_COMPLEX_DISPLACEMENT_ASSERTION:
					comm_file_ofs << "                  _F(RESULTAT=RESU" << analysis->getId() << "," << endl;
					writeNodalComplexDisplacementAssertion( dynamic_cast<const NodalComplexDisplacementAssertion&>(*assertion));
                    comm_file_ofs << "                     )," << endl;
					break;
                case Objective::Type::NODAL_CELL_VONMISES_ASSERTION:
					comm_file_ofs << "                  _F(RESULTAT=RESU" << analysis->getId() << "," << endl;
					writeNodalCellVonMisesAssertion( dynamic_cast<const NodalCellVonMisesAssertion&>(*assertion));
                    comm_file_ofs << "                     )," << endl;
					break;
				default:
					handleWritingError("Assertion type not (yet) implemented");
				}
			}
			comm_file_ofs << "                  )" << endl;
			comm_file_ofs << "          );" << endl << endl;
		}
	}
}

void AsterWriter::writeComm() {
	string asterVersion(asterModel->getAsterVersion());
	comm_file_ofs << "#Vega++ version " << VEGA_VERSION_MAJOR << "." << VEGA_VERSION_MINOR << endl;
	comm_file_ofs << "#Aster version " << asterModel->getAsterVersion() << endl;
	comm_file_ofs << "DEBUT(PAR_LOT='NON', IGNORE_ALARM=('SUPERVIS_1'))" << endl;

	mail_name = "MAIL";
	sigm_noeu = "SIGM_NOEU";
	sigm_elno = "SIGM_ELNO";
	sief_elga = "SIEF_ELGA";

	writeLireMaillage();

	writeAffeModele();

	for (const auto& value : asterModel->model.values) {
		writeValue(*value);
	}

	writeMaterials();

	writeAffeCaraElem();

	writeAffeCharMeca();

	writeDefiContact();

	writeAnalyses();

	writeImprResultats();

	comm_file_ofs << "FIN()" << endl;
}

void AsterWriter::writeLireMaillage() {
	comm_file_ofs << mail_name << "=LIRE_MAILLAGE(FORMAT='MED',";
	if (asterModel->configuration.logLevel >= LogLevel::DEBUG and asterModel->model.mesh.countNodes() < 100) {
		comm_file_ofs << "INFO_MED=2,VERI_MAIL=_F(VERIF='OUI',),INFO=2";
	} else {
		comm_file_ofs << "VERI_MAIL=_F(VERIF='NON',),";
	}
	comm_file_ofs << ");" << endl << endl;

    for (const auto& constraintSet : asterModel->model.constraintSets) {
		if (not constraintSet->hasContacts()) {
			continue;
		}
        const auto& zones = constraintSet->getConstraintsByType(Constraint::Type::ZONE_CONTACT);
        if (zones.empty()) {
            continue;
        }
        comm_file_ofs << mail_name << "=MODI_MAILLAGE(reuse="<< mail_name << ",";
        comm_file_ofs << "MAILLAGE=" << mail_name << "," << endl;
        // TODO LD should find a better solution
        int firstNodePosition = *((*zones.begin())->nodePositions().begin());
        if (asterModel->model.mesh.findNode(firstNodePosition).dofs == DOFS::ALL_DOFS) {
            comm_file_ofs << "         ORIE_PEAU_2D=(" << endl;
        } else {
            comm_file_ofs << "         ORIE_PEAU_3D=(" << endl;
        }
		for (const auto& constraint : zones) {
		    const auto& zone = static_pointer_cast<const ZoneContact>(constraint);
		    const auto& master = static_pointer_cast<const ContactBody>(asterModel->model.find(zone->master));
		    const auto& masterSurface = static_pointer_cast<const BoundarySurface>(asterModel->model.find(master->boundary));
		    const auto& slave = static_pointer_cast<const ContactBody>(asterModel->model.find(zone->slave));
		    const auto& slaveSurface = static_pointer_cast<const BoundarySurface>(asterModel->model.find(slave->boundary));
            comm_file_ofs << "                             _F(";
            writeCellContainer(*masterSurface);
            comm_file_ofs << ")," << endl;
            comm_file_ofs << "                             _F(";
            writeCellContainer(*slaveSurface);
            comm_file_ofs << ")," << endl;

		}
		comm_file_ofs << "                             )," << endl;
		comm_file_ofs << "                   )" << endl << endl;
	}

	for (const auto& constraintSet : asterModel->model.constraintSets) {
        const auto& surfaces = constraintSet->getConstraintsByType(
				Constraint::Type::SURFACE_SLIDE_CONTACT);
        if (surfaces.empty()) {
            continue;
        }
        comm_file_ofs << mail_name << "=MODI_MAILLAGE(reuse="<< mail_name << ",";
        comm_file_ofs << "MAILLAGE=" << mail_name << "," << endl;
        comm_file_ofs << "         ORIE_PEAU_3D=(" << endl;
		for (const auto& constraint : surfaces) {
		    const auto& surface = static_pointer_cast<const SurfaceSlide>(constraint);
		    const auto& masterSurface = static_pointer_cast<const BoundaryElementFace>(asterModel->model.find(surface->master));
		    const auto& slaveSurface = static_pointer_cast<const BoundaryElementFace>(asterModel->model.find(surface->slave));
            comm_file_ofs << "                             _F(";
            comm_file_ofs << "GROUP_MA=('"
                    << masterSurface->surfaceCellGroup->getName() << "', '"
                    << slaveSurface->surfaceCellGroup->getName()
                    << "'),";
            comm_file_ofs << ")," << endl;
		}
		comm_file_ofs << "                             )," << endl;
		comm_file_ofs << "                   )" << endl << endl;
	}
}

void AsterWriter::writeAffeModele() {
	comm_file_ofs << "MODMECA=AFFE_MODELE(MAILLAGE=" << mail_name << "," << endl;
	comm_file_ofs << "                    AFFE=(" << endl;
	for (const auto& elementSet : asterModel->model.elementSets) {
		if (elementSet->effective()) {
			comm_file_ofs << "                          _F(";
            const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
            if (cellElementSet == nullptr) {
                handleWritingError("ElementSet which is not a CellContainer should be written using cellPositions, to be implemented here");
            }
            writeCellContainer(*cellElementSet);
			comm_file_ofs << endl;
			comm_file_ofs << "                             PHENOMENE='" << asterModel->phenomene << "',"
					<< endl;

			comm_file_ofs << "                             MODELISATION="
					<< asterModel->getModelisations(elementSet) << ")," << endl;
		} else {
			comm_file_ofs << "#Skipping element El" << *elementSet << " because no assignment" << endl;
		}
	}
	comm_file_ofs << "                          )," << endl;
	comm_file_ofs << "                    );" << endl << endl;
}

string AsterWriter::writeValue(NamedValue& value) {
	string concept_name;

	switch (value.type) {
	case Value::Type::LIST: {
		auto& listValue = static_cast<ListValueBase&>(value);
        if (not listValue.empty()) {
            ostringstream list_concept_ss;
            list_concept_ss << "LST" << setfill('0') << setw(5) << listValue.getId();
            concept_name = list_concept_ss.str();
            comm_file_ofs << concept_name << "=DEFI_LIST_REEL(" << endl;
            comm_file_ofs << "                        VALE = (";
            if (listValue.isintegral()) {
                const auto& listIntValue = static_cast<ListValue<int>&>(value);
                for (const int val : listIntValue.getList()) {
                    comm_file_ofs << val << ",";
                }
            } else {
                const auto& listDblValue = static_cast<ListValue<double>&>(value);
                for (const double val : listDblValue.getList()) {
                    comm_file_ofs << val << ",";
                }
            }
            comm_file_ofs << ")," << endl;
            comm_file_ofs << "                        );" << endl << endl;
        }
        listValue.markAsWritten();
        break;
	}
    case Value::Type::SET: {
		auto& setValue = static_cast<SetValueBase&>(value);
		if (not setValue.isintegral()) {
            handleWritingError("non-integral set not yet implemented");
		} else if (not setValue.empty()) {
            ostringstream list_concept_ss;
            list_concept_ss << "LST" << setfill('0') << setw(5) << setValue.getId();
            concept_name = list_concept_ss.str();
            comm_file_ofs << concept_name << "=DEFI_LIST_REEL(" << endl;
            comm_file_ofs << "                        VALE = (";
            if (setValue.isintegral()) {
                const auto& setIntValue = static_cast<SetValue<int>&>(value);
                for (const int val : setIntValue.getSet()) {
                    comm_file_ofs << val << ",";
                }
            } else {
                handleWritingError("non-integral set not yet implemented");
            }
            comm_file_ofs << ")," << endl;
            comm_file_ofs << "                        );" << endl << endl;
        }
        setValue.markAsWritten();
        break;
	}
	case Value::Type::STEP_RANGE: {
		auto& stepRange = static_cast<StepRange&>(value);
		if (not stepRange.iszero()) {
            ostringstream list_concept_ss;
            list_concept_ss << "LST" << setfill('0') << setw(5) << stepRange.getId();
            concept_name = list_concept_ss.str();
            if (not is_equal(stepRange.end, Globals::UNAVAILABLE_DOUBLE)) {
                comm_file_ofs << concept_name << "=DEFI_LIST_REEL(" << endl;
                comm_file_ofs << "                        DEBUT = " << stepRange.start << "," << endl;
                comm_file_ofs << "                        INTERVALLE = _F(JUSQU_A = " << stepRange.end << "," << endl;
                comm_file_ofs << "                                        NOMBRE = " << stepRange.count << endl;
                comm_file_ofs << "                                        )," << endl;
                comm_file_ofs << "                        );" << endl << endl;
            } else {
                comm_file_ofs << "# Ignoring " << concept_name << " because: no end value" << endl;
            }
		}
		stepRange.markAsWritten();
		break;
	}
	case Value::Type::SPREAD_RANGE: {
        auto& spreadRange = static_cast<SpreadRange&>(value);
        if (not spreadRange.iszero()) {
            ostringstream list_concept_ss;
            list_concept_ss << "LST" << setfill('0') << setw(5) << spreadRange.getId();
            concept_name = list_concept_ss.str();
            comm_file_ofs << concept_name << "=DEFI_LIST_FREQ(" << endl;
            comm_file_ofs << "                        DEBUT = " << spreadRange.start << "," << endl;
            comm_file_ofs << "                        INTERVALLE = _F(JUSQU_A = " << spreadRange.end << "," << endl;
            comm_file_ofs << "                                        NOMBRE = " << spreadRange.count << endl;
            comm_file_ofs << "                                        )," << endl;
            comm_file_ofs << "                        RAFFINEMENT = _F(LIST_RAFFINE = XXXX" << "," << endl;
            comm_file_ofs << "                                        CRITERE = 'RELATIF'," << endl;
            comm_file_ofs << "                                        DISPERSION = " << spreadRange.spread << endl;
            comm_file_ofs << "                                        )," << endl;
            comm_file_ofs << "                        );" << endl << endl;
        }
        spreadRange.markAsWritten();
        break;
		}
	case Value::Type::FUNCTION_TABLE: {
		auto& functionTable = static_cast<FunctionTable&>(value);
		ostringstream concept_ss;
		concept_ss << "FCT" << setfill('0') << setw(5) << functionTable.getId();
		concept_name = concept_ss.str();
		comm_file_ofs << concept_name << "=DEFI_FONCTION(" << endl;
		if (functionTable.hasParaX())
			comm_file_ofs << "                       NOM_PARA='"
					<< AsterModel::NomParaByParaName.find(functionTable.getParaX())->second << "',"
					<< endl;

		if (functionTable.hasParaY())
			comm_file_ofs << "                       NOM_RESU='"
					<< AsterModel::NomParaByParaName.find(functionTable.getParaY())->second << "',"
					<< endl;

		comm_file_ofs << "                       VALE = (" << endl;
		for (auto it = functionTable.getBeginValuesXY();
				it != functionTable.getEndValuesXY(); it++)
			comm_file_ofs << "                               " << it->first << ", " << it->second << ","
					<< endl;
		comm_file_ofs << "                               )," << endl;
		comm_file_ofs << "                       INTERPOL = ('"
				<< AsterModel::InterpolationByInterpolation.find(functionTable.parameter)->second
				<< "','" << AsterModel::InterpolationByInterpolation.find(functionTable.value)->second
				<< "')," << endl;
		comm_file_ofs << "                       PROL_GAUCHE='"
				<< AsterModel::ProlongementByInterpolation.find(functionTable.left)->second << "',"
				<< endl;
		comm_file_ofs << "                       PROL_DROITE='"
				<< AsterModel::ProlongementByInterpolation.find(functionTable.right)->second << "',"
				<< endl;
		comm_file_ofs << "                       );";
		if (functionTable.isOriginal()) {
			comm_file_ofs << "# Original id:" << functionTable.getOriginalId();
		}
		comm_file_ofs << endl << endl;
		functionTable.markAsWritten();
		break;
	}
	case Value::Type::SCALAR:
	case Value::Type::BAND_RANGE:
	case Value::Type::DYNA_PHASE: {
		break;
	}
	default:
		handleWritingError("NamedValue not yet implemented");
	}
	asternameByValue[value] = concept_name;

	return concept_name;
}

void AsterWriter::writeMaterials() {

	for (auto& material : asterModel->model.materials) {
		if (material->isOriginal()) {
			comm_file_ofs << "# Material original id " << material->getOriginalId() << endl;
		}
		comm_file_ofs << "M" << material->getId() << "=DEFI_MATERIAU(" << endl;
		const auto& enature = material->findNature(Nature::NatureType::NATURE_ELASTIC);
		if (enature) {
			const auto& elasticNature = static_pointer_cast<const ElasticNature>(enature);
			comm_file_ofs << "                 ELAS=_F(" << endl;
			comm_file_ofs << "                         E=" << elasticNature->getE() << "," << endl;
			comm_file_ofs << "                         NU=" << elasticNature->getNu() << "," << endl;
			comm_file_ofs << "                         RHO=" << elasticNature->getRho() << "," << endl;
			if (not is_equal(elasticNature->getGE(), Globals::UNAVAILABLE_DOUBLE)) {
                comm_file_ofs << "                         AMOR_HYST=" << elasticNature->getGE() << "," << endl;
			}
			comm_file_ofs << "                         )," << endl;
		}
		const auto& hynature = material->findNature(Nature::NatureType::NATURE_HYPERELASTIC);
		if (hynature) {
			const auto& hyperElasticNature = static_pointer_cast<const HyperElasticNature>(hynature);
			comm_file_ofs << "                 ELAS_HYPER=_F(" << endl;
			comm_file_ofs << "                         C10=" << hyperElasticNature->c10 << "," << endl;
			comm_file_ofs << "                         C01=" << hyperElasticNature->c01 << "," << endl;
			comm_file_ofs << "                         C20=" << hyperElasticNature->c20 << "," << endl;
			comm_file_ofs << "                         RHO=" << hyperElasticNature->rho << "," << endl;
			comm_file_ofs << "                         K=" << hyperElasticNature->k << "," << endl;
			comm_file_ofs << "                         )," << endl;
		}
		const auto& onature = material->findNature(Nature::NatureType::NATURE_ORTHOTROPIC);
		if (onature) {
			const auto& orthoNature = static_pointer_cast<const OrthotropicNature>(onature);
			comm_file_ofs << "                 ELAS_ORTH=_F(" << endl;
			comm_file_ofs << "                         E_L=" << orthoNature->getE_longitudinal() << "," << endl;
            comm_file_ofs << "                         E_T=" << orthoNature->getE_transverse() << "," << endl;
            comm_file_ofs << "                         G_LT=" << orthoNature->getG_longitudinal_transverse() << "," << endl;
            if (!is_equal(orthoNature->getG_transverse_normal(),Globals::UNAVAILABLE_DOUBLE)){
                comm_file_ofs << "                         G_TN=" << orthoNature->getG_transverse_normal() << "," << endl;
            }
            if (!is_equal(orthoNature->getG_longitudinal_normal(),Globals::UNAVAILABLE_DOUBLE)){
                comm_file_ofs << "                         G_LN=" << orthoNature->getG_longitudinal_normal() << "," << endl;
            }
			comm_file_ofs << "                         NU_LT=" << orthoNature->getNu_longitudinal_transverse() << "," << endl;
			comm_file_ofs << "                         )," << endl;
		}
		const auto& binature = material->findNature(Nature::NatureType::NATURE_BILINEAR_ELASTIC);
		if (binature) {
			const auto& bilinearNature = static_pointer_cast<const BilinearElasticNature>(binature);
			comm_file_ofs << "                 ECRO_LINE=_F(" << endl;
			comm_file_ofs << "                         D_SIGM_EPSI=" << bilinearNature->secondary_slope << ","
					<< endl;
			comm_file_ofs << "                         SY=" << bilinearNature->elastic_limit << "," << endl;
			comm_file_ofs << "                         )," << endl;
		}
		comm_file_ofs << "                 );" << endl << endl;
		material->markAsWritten();
	}

	const auto& composites = asterModel->model.elementSets.filter(ElementSet::Type::COMPOSITE);
    comm_file_ofs << "# writing " << composites.size() << " composites" << endl;
    for (const auto& c : composites) {
        const auto& composite = static_pointer_cast<Composite>(c);
        comm_file_ofs << "MC" << composite->getId() << "=DEFI_COMPOSITE(" << endl;
        comm_file_ofs << "                 COUCHE=(" << endl;
        for (const auto& layer : composite->getLayers()) {
            comm_file_ofs << "                     _F(EPAIS=" << layer.getThickness() << ",  MATER=M" << layer.getMaterialId() << ", ORIENTATION=" << layer.getOrientation() << ")," << endl;
        }
        comm_file_ofs << "                         )," << endl;
        comm_file_ofs << "                 );" << endl << endl;
        composite->markAsWritten();
    }

    if (not asterModel->model.materials.empty()) {
        comm_file_ofs << "CHMAT=AFFE_MATERIAU(MAILLAGE=" << mail_name << "," << endl;
        comm_file_ofs << "                    AFFE=(" << endl;
        for (const auto& material : asterModel->model.materials) {
            const auto& cells = material->getAssignment();
            if (not cells.empty()) {
                comm_file_ofs << "                          _F(MATER=M" << material->getId() << ",";
                writeCellContainer(cells);
                comm_file_ofs << ")," << endl;
            } else {
                comm_file_ofs << "# WARN Skipping material id " << material->getId() << " because no assignment"
                    << endl;
            }
        }
        for (const auto& c : composites) {
            const auto& composite = static_pointer_cast<Composite>(c);
            comm_file_ofs << "                          _F(MATER=MC" << composite ->getId() << ",";
            writeCellContainer(*composite);
            comm_file_ofs << ")," << endl;
        }
        comm_file_ofs << "                          )," << endl;
        comm_file_ofs << "                    );" << endl << endl;
    }
}

void AsterWriter::writeAffeCaraElem() {
	calc_sigm = false;
	if (not asterModel->model.elementSets.empty()) {
		comm_file_ofs << "CAEL=AFFE_CARA_ELEM(MODELE=MODMECA," << endl;

		const auto& discrets_0d = asterModel->model.elementSets.filter(
				ElementSet::Type::DISCRETE_0D);
		auto discrets_1d = asterModel->model.elementSets.filter(ElementSet::Type::DISCRETE_1D);
        const auto& scalar_springs = asterModel->model.elementSets.filter(ElementSet::Type::SCALAR_SPRING);
        const auto& structural_segments = asterModel->model.elementSets.filter(ElementSet::Type::STRUCTURAL_SEGMENT);
        discrets_1d.insert(discrets_1d.end(), scalar_springs.begin(), scalar_springs.end());
        discrets_1d.insert(discrets_1d.end(), structural_segments.begin(), structural_segments.end());
		const auto& nodal_masses = asterModel->model.elementSets.filter(
				ElementSet::Type::NODAL_MASS);
        auto numDiscrets = discrets_0d.size() + nodal_masses.size() + discrets_1d.size();
		comm_file_ofs << "                    # writing " << numDiscrets << " discrets" << endl;
		if (numDiscrets > 0) {
			comm_file_ofs << "                    DISCRET=(" << endl;
			for (const auto& discret : discrets_0d) {
				if (discret->effective()) {
                    const auto& discret_0d = static_pointer_cast<DiscretePoint>(discret);
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
                    comm_file_ofs << "                             _F(";
                    writeCellContainer(*discret_0d);
                    comm_file_ofs << endl;
                    comm_file_ofs << "                                " << syme << "CARA='K_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                    for (double rigi : discret_0d->asStiffnessVector())
                        comm_file_ofs << rigi << ",";
                    comm_file_ofs << "),)," << endl;

					if (discret_0d->hasDamping()) {
                        comm_file_ofs << "                             _F(";
                        writeCellContainer(*discret_0d);
                        comm_file_ofs << endl;
                        comm_file_ofs << "                                " << syme << "CARA='A_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                        for (double amor : discret_0d->asDampingVector())
                            comm_file_ofs << amor << ",";
                        comm_file_ofs << "),)," << endl;
					}

					if (discret_0d->hasMass()) {
                        comm_file_ofs << "                             _F(";
                        writeCellContainer(*discret_0d);
                        comm_file_ofs << endl;
                        comm_file_ofs << "                                " << syme << "CARA='M_T" << rotationMarker << "_" << diagonalMarker << "N', VALE=(";
                        for (double mass : discret_0d->asMassVector())
                            comm_file_ofs << mass << ",";
                        comm_file_ofs << "),)," << endl;
					}

				} else {
					comm_file_ofs
							<< "                             # WARN Finite Element : DISCRETE_0D ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			for (const auto& discret : discrets_1d) {
				if (discret->effective()) {
                    const auto& discret_1d = static_pointer_cast<Discrete>(discret);
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
                    comm_file_ofs << "                             _F(";
                    writeCellContainer(*discret_1d);
                    comm_file_ofs << endl;
                    comm_file_ofs << "                                " << syme << "CARA='K_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";
                    for (double rigi : discret_1d->asStiffnessVector())
                        comm_file_ofs << rigi << ",";
                    comm_file_ofs << "),)," << endl;

                    if (discret_1d->hasDamping()) {
                        comm_file_ofs << "                             _F(";
                        writeCellContainer(*discret_1d);
                        comm_file_ofs << endl;
                        comm_file_ofs << "                                " << syme << "CARA='A_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";
                        for (double amor : discret_1d->asDampingVector())
                            comm_file_ofs << amor << ",";
                        comm_file_ofs << "),)," << endl;
                    }

                    if (discret_1d->hasMass()) {
                        comm_file_ofs << "                             _F(";
                        writeCellContainer(*discret_1d);
                        comm_file_ofs << endl;
                        comm_file_ofs << "                                " << syme << "CARA='M_T" << rotationMarker << "_" << diagonalMarker << "L', VALE=(";

                        for (double mass : discret_1d->asMassVector())
                            comm_file_ofs << mass << ",";
                        comm_file_ofs << "),)," << endl;
                    }

				} else {
					comm_file_ofs
							<< "                             # WARN Finite Element : DISCRETE_1D ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			for (const auto& discret : nodal_masses) {
				if (discret->effective()) {
                    const auto& nodalMass = static_pointer_cast<NodalMass>(discret);
					comm_file_ofs << "                             _F(";
					writeCellContainer(*nodalMass);
					comm_file_ofs << endl;
					if (nodalMass->hasRotations()) {
                        comm_file_ofs << "                                CARA='M_TR_D_N',VALE=("
                                << nodalMass->getMass() << "," << nodalMass->ixx << ","
                                << nodalMass->iyy << "," << nodalMass->izz << "," << nodalMass->ixy
                                << "," << nodalMass->iyz << "," << nodalMass->ixz << ","
                                << nodalMass->ex << "," << nodalMass->ey << "," << nodalMass->ez
                                << "),)," << endl;
					} else {
                        comm_file_ofs << "                                CARA='M_T_D_N',VALE=("
							<< nodalMass->getMass()	<< "),)," << endl;
					}
				} else {
					comm_file_ofs
							<< "                             # WARN Finite Element : NODAL_MASS ignored because its GROUP_MA is empty."
							<< endl;
				}
				discret->markAsWritten();
			}
			comm_file_ofs << "                             )," << endl;
		}
		const auto& poutres = asterModel->model.getBeams();
        const auto& barres = asterModel->model.getTrusses();
		comm_file_ofs << "                    # writing " << poutres.size() << " poutres" << endl;
		comm_file_ofs << "                    # writing " << barres.size() << " barres" << endl;
		if (not poutres.empty() or (asterModel->model.needsLargeDisplacements() and not barres.empty())) {
			comm_file_ofs << "                    POUTRE=(" << endl;
			for (const auto& poutre : poutres) {
				writeAffeCaraElemPoutre( *poutre);
			}
			if (asterModel->model.needsLargeDisplacements()) {
                for (const auto& barre : barres) {
                    writeAffeCaraElemPoutre( *barre);
                }
			}
			comm_file_ofs << "                            )," << endl;
		}

		if (not barres.empty() and not asterModel->model.needsLargeDisplacements()) {
			comm_file_ofs << "                    BARRE=(" << endl;
			for (const auto& barre : barres) {
				writeAffeCaraElemPoutre( *barre);
			}
			comm_file_ofs << "                            )," << endl;
		}
		const auto& shells = asterModel->model.elementSets.filter(ElementSet::Type::SHELL);
		const auto& composites = asterModel->model.elementSets.filter(ElementSet::Type::COMPOSITE);
		comm_file_ofs << "                    # writing " << shells.size()+composites.size() << " shells (ou composites)" << endl;
		if (not (shells.empty() and composites.empty())) {
			calc_sigm = true;
			comm_file_ofs << "                    COQUE=(" << endl;
			for (const auto& elementSet : shells) {
                const auto& shell = static_pointer_cast<Shell>(elementSet);
				if (shell->effective()) {
					comm_file_ofs << "                           _F(";
					writeCellContainer(*shell);
					comm_file_ofs << endl;
					comm_file_ofs << "                              EPAIS="
							<< shell->thickness << "," << endl;
					comm_file_ofs << "                              VECTEUR=(0.9,0.1,0.2))," << endl;
				} else {
					comm_file_ofs
							<< "                           # WARN Finite Element : COQUE ignored because its GROUP_MA is empty."
							<< endl;
				}
				shell->markAsWritten();
			}
			for (const auto& c : composites) {
                const auto& composite = static_pointer_cast<Composite>(c);
				if (!composite->empty()) {
					comm_file_ofs << "                           _F(";
					writeCellContainer(*composite);
					comm_file_ofs << endl;
					comm_file_ofs << "                              EPAIS="
							<< composite->getTotalThickness() << "," << endl;
					comm_file_ofs << "                              COQUE_NCOU="
							<< composite->getLayers().size() << "," << endl;
					comm_file_ofs << "                              VECTEUR=(1.0,0.0,0.0))," << endl;
				} else {
					comm_file_ofs
							<< "                           # WARN Finite Element : COMPOSITE ignored because its GROUP_MA is empty."
							<< endl;
				}
				composite->markAsWritten();
			}
			comm_file_ofs << "                           )," << endl;
		}
		const auto& solids = asterModel->model.elementSets.filter(
				ElementSet::Type::CONTINUUM);
        const auto& skins = asterModel->model.elementSets.filter(
				ElementSet::Type::SKIN);
		comm_file_ofs << "                    # writing " << solids.size() << " solids" << endl;
		comm_file_ofs << "                    # writing " << skins.size() << " skins" << endl;
		if (not solids.empty()) {
			comm_file_ofs << "                    MASSIF=(" << endl;
			for (const auto& elementSet : solids) {
                const auto& solid = static_pointer_cast<Continuum>(elementSet);
				if (solid->effective()) {
					comm_file_ofs << "                           _F(";
					writeCellContainer(*solid);
					comm_file_ofs << endl;
					comm_file_ofs << "                               ANGL_REP=(0.,0.,0.,),)," << endl;
				} else {
					comm_file_ofs << "# WARN Finite Element : MASSIF ignored because its GROUP_MA is empty."
							<< endl;
				}
				solid->markAsWritten();
			}
			for (const auto& elementSet : skins) {
			    const auto& skin = static_pointer_cast<Skin>(elementSet);
				if (!skin->empty()) {
					comm_file_ofs << "                           _F(";
					writeCellContainer(*skin);
					comm_file_ofs << endl;
					comm_file_ofs << "                               ANGL_REP=(0.,0.,0.,),)," << endl;
				} else {
					comm_file_ofs << "# WARN Finite Element : MASSIF (skin) ignored because its GROUP_MA is empty."
							<< endl;
				}
				skin->markAsWritten();
			}
			comm_file_ofs << "                            )," << endl;
		}
	}

	//orientations
	bool orientationsPrinted = false;
	for (const auto& it : asterModel->model.mesh.cellGroupNameByCspos){
        if (asterModel->model.elementSets.filter(ElementSet::Type::CONTINUUM).size() == asterModel->model.elementSets.size()) {
            // LD workaround for case no beams, discrete, shells, composites... only solids in model, but segments in geometry
            // TODO : probably this loop should be applyed to elementSets and not over groups!!
            continue;
        }
        const auto& cs= asterModel->model.mesh.getCoordinateSystemByPosition(it.first);
		if (cs->type!=CoordinateSystem::Type::RELATIVE){
		   //handleWritingError("Coordinate System of Group "+ it.second+" is not an ORIENTATION.");
		   continue;
		}
		if (!orientationsPrinted) {
			comm_file_ofs << "                    ORIENTATION=(" << endl;
			orientationsPrinted = true;
		}
		const auto& ocs = static_pointer_cast<OrientationCoordinateSystem>(cs);

		comm_file_ofs << "                                 _F(CARA ='VECT_Y',VALE=(";
		comm_file_ofs << ocs->getV().x() << "," << ocs->getV().y() << "," << ocs->getV().z() << ")";
		comm_file_ofs << ",GROUP_MA='"<< it.second << "')," << endl;
	}
	if (orientationsPrinted) {
		comm_file_ofs << "                                 )," << endl;
	}
	comm_file_ofs << "                    );" << endl << endl;
}
void AsterWriter::writeAffeCaraElemPoutre( Beam& beam) {
	comm_file_ofs << "                            _F(";
	writeCellContainer(beam);
	comm_file_ofs << endl;
	switch (beam.type) {
	case ElementSet::Type::RECTANGULAR_SECTION_BEAM: {
		auto& rectBeam =
				static_cast<RectangularSectionBeam&>(beam);
		comm_file_ofs << "                               VARI_SECT='CONSTANT'," << endl;
		comm_file_ofs << "                               SECTION='RECTANGLE'," << endl;
		comm_file_ofs << "                               CARA=('HY','HZ',)," << endl;
		comm_file_ofs << "                               VALE=(" << rectBeam.height << "," << rectBeam.width
				<< ")," << endl;
        rectBeam.markAsWritten();
		break;
	}
	case ElementSet::Type::CIRCULAR_SECTION_BEAM: {
		auto& circBeam = static_cast<CircularSectionBeam&>(beam);
		comm_file_ofs << "                               SECTION='CERCLE'," << endl;
		comm_file_ofs << "                               CARA=('R',)," << endl;
		comm_file_ofs << "                               VALE=(" << circBeam.radius << ")," << endl;
		circBeam.markAsWritten();
		break;
	}
	case ElementSet::Type::TUBE_SECTION_BEAM: {
        auto& tubeBeam = static_cast<TubeSectionBeam&>(beam);
		comm_file_ofs << "                               SECTION='CERCLE'," << endl;
		comm_file_ofs << "                               CARA=('R','EP')," << endl;
		comm_file_ofs << "                               VALE=(" << tubeBeam.radius << ","<< tubeBeam.thickness << ")," << endl;
		tubeBeam.markAsWritten();
		break;
	}
	default:
		comm_file_ofs << "                               SECTION='GENERALE'," << endl;
		if (not beam.isTruss() or asterModel->model.needsLargeDisplacements()) {
		    comm_file_ofs << "                               CARA=('A','IY','IZ','JX','AY','AZ',)," << endl;
		} else {
		    comm_file_ofs << "                               CARA=('A',)," << endl;
		}

		comm_file_ofs << "                               VALE=(";
        if (not beam.isTruss() or asterModel->model.needsLargeDisplacements()) {
            comm_file_ofs << max(std::numeric_limits<double>::epsilon(), beam.getAreaCrossSection()) << ","
				<< max(std::numeric_limits<double>::epsilon(), beam.getMomentOfInertiaY()) << "," << max(std::numeric_limits<double>::epsilon(), beam.getMomentOfInertiaZ())
				<< "," << max(std::numeric_limits<double>::epsilon(), beam.getTorsionalConstant()) << ",";
            if (! is_zero(beam.getShearAreaFactorY()))
                comm_file_ofs << beam.getShearAreaFactorY();
            else
                comm_file_ofs << 0.0;
            comm_file_ofs << ",";
            if (! is_zero(beam.getShearAreaFactorZ()))
                comm_file_ofs << beam.getShearAreaFactorZ();
            else
                comm_file_ofs << 0.0;
		} else {
            comm_file_ofs << max(std::numeric_limits<double>::epsilon(), beam.getAreaCrossSection()) << ",";
		}

		comm_file_ofs << ")," << endl;
		beam.markAsWritten();
	}
	comm_file_ofs << "                               )," << endl;
}

void AsterWriter::writeAffeCharMeca() {
	for (const auto& it : asterModel->model.constraintSets) {
		ConstraintSet& constraintSet = *it;
		if (constraintSet.getConstraints().empty()) {
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
			comm_file_ofs << "# ConstraintSet : " << constraintSet << endl;
		}
		for(bool withFunctions : {false, true} ) {
            string asterName;
            if (withFunctions and constraintSet.hasFunctions()) {
                asterName = "BLF" + to_string(constraintSet.getId());
                comm_file_ofs << asterName << "=AFFE_CHAR_MECA_F(MODELE=MODMECA," << endl;
            } else if (not withFunctions and not constraintSet.hasFunctions()) {
                asterName = "BL" + to_string(constraintSet.getId());
                comm_file_ofs << asterName << "=AFFE_CHAR_MECA(MODELE=MODMECA," << endl;
            } else {
                continue;
            }
            asternameByConstraintSet[constraintSet] = asterName;

            writeSPC( constraintSet);
            writeLIAISON_SOLIDE( constraintSet);
            writeLIAISON_MAIL( constraintSet);
            writeRBE3( constraintSet);
            writeLMPC( constraintSet);
            comm_file_ofs << "                   );" << endl << endl;
		}
		constraintSet.markAsWritten();
	}

	for (const auto& it : asterModel->model.loadSets) {
		LoadSet& loadSet = *it;
		if (loadSet.type == LoadSet::Type::DLOAD) {
			continue;
		}
		if (loadSet.empty()) {
            loadSet.markAsWritten();
			continue;
		}
		if (loadSet.getLoadings().size()
				== loadSet.getLoadingsByType(Loading::Type::INITIAL_TEMPERATURE).size()) {
			comm_file_ofs << "# Ignoring INITIAL_TEMPERATURES!!!!!!" << endl;
			cout << "!!!!!!Ignoring INITIAL_TEMPERATURES!!!!!!" << endl;
			continue;
		}
		if (loadSet.isOriginal()) {
			comm_file_ofs << "# LoadSet " << loadSet << endl;
		}
		for(bool withFunctions : {false, true} ) {
            string asterName;
		    if (withFunctions and loadSet.hasFunctions()) {
                asterName = "CHMEF" + to_string(loadSet.getId());
                comm_file_ofs << asterName << "=AFFE_CHAR_MECA_F(MODELE=MODMECA," << endl;
		    } else if (not withFunctions and not loadSet.hasFunctions()) {
		        asterName = "CHMEC" + to_string(loadSet.getId());
                comm_file_ofs << asterName << "=AFFE_CHAR_MECA(MODELE=MODMECA," << endl;
            } else {
                continue;
            }
            comm_file_ofs << "                 VERI_NORM='NON'," << endl; // Workaround for PREPOST4_97 see test pload4-ctetra-multi
            asternameByLoadSet[loadSet] = asterName;
            writeSPCD( loadSet);
            writePression(loadSet);
            writeForceCoque(loadSet);
            writeNodalForce( loadSet);
            writeForceSurface(loadSet);
            writeForceLine(loadSet);
            writeGravity(loadSet);
            writeRotation(loadSet);
            comm_file_ofs << "                      );" << endl << endl;
		}
		loadSet.markAsWritten();
	}
}

void AsterWriter::writeDefiContact() {
	for (const auto& it : asterModel->model.constraintSets) {
		ConstraintSet& constraintSet = *it;
		if (constraintSet.getConstraints().empty()) {
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
		for (const auto& constraint : gaps) {
			const auto& gap = static_pointer_cast<const Gap>(constraint);
			int gapCount = 0;
			for (const auto& gapParticipation : gap->getGaps()) {
				gapCount++;
				comm_file_ofs << "C" << constraintSet.getId() << "I" << to_string(gapCount)
						<< "=DEFI_CONSTANTE(VALE=" << gap->initial_gap_opening << ")" << endl;
				if (!is_zero(gapParticipation->direction.x())) {
					comm_file_ofs << "C" << constraintSet.getId() << "MX" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.x() << ")"
							<< endl;
				}
				if (!is_zero(gapParticipation->direction.y())) {
					comm_file_ofs << "C" << constraintSet.getId() << "MY" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.y() << ")"
							<< endl;
				}
				if (!is_zero(gapParticipation->direction.y())) {
					comm_file_ofs << "C" << constraintSet.getId() << "MZ" << to_string(gapCount)
							<< "=DEFI_CONSTANTE(VALE=" << gapParticipation->direction.z() << ")"
							<< endl;
				}
			}
		}
		const auto& asterName = "CN" + to_string(constraintSet.getId());
		asternameByConstraintSet[constraintSet] = asterName;
		if (constraintSet.isOriginal()) {
            comm_file_ofs << "# ConstraintSet original id:" << constraintSet.getOriginalId() << endl;
		}
		comm_file_ofs << asterName << "=DEFI_CONTACT(MODELE=MODMECA," << endl;
		if (not gaps.empty()) {
            comm_file_ofs << "                   FORMULATION='LIAISON_UNIL'," << endl;
		} else if (not slides.empty()) {
		    comm_file_ofs << "                   FORMULATION='CONTINUE'," << endl;
		    comm_file_ofs << "                   FROTTEMENT='COULOMB'," << endl;
		} else if (not surfaces.empty()) {
		    comm_file_ofs << "                   FORMULATION='CONTINUE'," << endl;
		}
		comm_file_ofs << "                   ZONE=(" << endl;
		for (const auto& constraint : gaps) {
			const auto& gap = static_pointer_cast<Gap>(constraint);
			int gapCount = 0;
			for (const auto& gapParticipation : gap->getGaps()) {
				gapCount++;
				comm_file_ofs << "                             _F(";
				comm_file_ofs << "NOEUD='"
						<< Node::MedName(gapParticipation->nodePosition)
						<< "',";
				comm_file_ofs << "COEF_IMPO=" << "C" << constraintSet.getId() << "I" << to_string(gapCount)
						<< ",";
				comm_file_ofs << "COEF_MULT=(";
				if (!is_zero(gapParticipation->direction.x() )) {
					comm_file_ofs << "C" << constraintSet.getId() << "MX" << to_string(gapCount) << ",";
				}
				if (!is_zero(gapParticipation->direction.y() )) {
					comm_file_ofs << "C" << constraintSet.getId() << "MY" << to_string(gapCount) << ",";
				}
				if (!is_zero(gapParticipation->direction.z())) {
					comm_file_ofs << "C" << constraintSet.getId() << "MZ" << to_string(gapCount);
				}
				comm_file_ofs << "),";
				comm_file_ofs << "NOM_CMP=(";
				if (!is_zero(gapParticipation->direction.x())) {
					comm_file_ofs << "'DX',";
				}
				if (!is_zero(gapParticipation->direction.y() )) {
					comm_file_ofs << "'DY',";
				}
				if (!is_zero(gapParticipation->direction.z() )) {
					comm_file_ofs << " 'DZ'";
				}
				comm_file_ofs << "),";
				comm_file_ofs << ")," << endl;
			}
			gap->markAsWritten();
		}
		for (const auto& constraint : slides) {
		    const auto& slide = static_pointer_cast<SlideContact>(constraint);
            comm_file_ofs << "                             _F(";
            comm_file_ofs << "GROUP_MA_MAIT='"
                    << slide->masterCellGroup->getName()
                    << "',";
            comm_file_ofs << "GROUP_MA_ESCL='"
                    << slide->slaveCellGroup->getName()
                    << "',";
            comm_file_ofs << "COULOMB=" << slide->getFriction() << ",";
            comm_file_ofs << ")," << endl;
            slide->markAsWritten();
		}
//		for (const auto& constraint : surfaceSlides) {
//		    const auto& surfaceSlide = dynamic_pointer_cast<const SurfaceSlide>(constraint);
//		    const auto& surfaceSlideMaster = dynamic_pointer_cast<const BoundaryElementFace>(asterModel->model.find(surfaceSlide->master));
//		    const auto& surfaceSlideSlave = dynamic_pointer_cast<const BoundaryElementFace>(asterModel->model.find(surfaceSlide->slave));
//                comm_file_ofs << "                             _F(";
//				comm_file_ofs << "GROUP_MA_MAIT='"
//						<< surfaceSlideMaster->cellGroup->getName()
//						<< "',";
//				comm_file_ofs << "GROUP_MA_ESCL='"
//						<< surfaceSlideSlave->cellGroup->getName()
//						<< "',";
//                comm_file_ofs << "GLISSIERE='OUI',";
//				comm_file_ofs << ")," << endl;
//		}
		for (const auto& constraint : surfaces) {
		    const auto& surface = static_pointer_cast<SurfaceContact>(constraint);
            comm_file_ofs << "                             _F(";
            comm_file_ofs << "GROUP_MA_MAIT='"
                    << surface->masterCellGroup->getName()
                    << "',";
            comm_file_ofs << "GROUP_MA_ESCL='"
                    << surface->slaveCellGroup->getName()
                    << "',";
            comm_file_ofs << ")," << endl;
            surface->markAsWritten();
		}
		for (const auto& constraint : zones) {
		    const auto& zone = static_pointer_cast<ZoneContact>(constraint);
		    const auto& master = static_pointer_cast<ContactBody>(asterModel->model.find(zone->master));
		    const auto& masterSurface = static_pointer_cast<BoundarySurface>(asterModel->model.find(master->boundary));
		    const auto& slave = static_pointer_cast<ContactBody>(asterModel->model.find(zone->slave));
		    const auto& slaveSurface = static_pointer_cast<BoundarySurface>(asterModel->model.find(slave->boundary));
            comm_file_ofs << "                             _F(";

            comm_file_ofs << "GROUP_MA_MAIT=(";
                for(const auto& cellGroup : masterSurface->getCellGroups()) {
                    comm_file_ofs << "'" << cellGroup->getName() << "',";
                }
                comm_file_ofs << "),";
            comm_file_ofs << "GROUP_MA_ESCL=(";
                for(const auto& cellGroup : slaveSurface->getCellGroups()) {
                    comm_file_ofs << "'" << cellGroup->getName() << "',";
                }
                comm_file_ofs << "),";
            comm_file_ofs << ")," << endl;
            zone->markAsWritten();
            master->markAsWritten();
            masterSurface->markAsWritten();
            slave->markAsWritten();
            slaveSurface->markAsWritten();
		}
		comm_file_ofs << "                             )," << endl;
		comm_file_ofs << "                   );" << endl << endl;
		constraintSet.markAsWritten();
	}
}

void AsterWriter::writeSPC( const ConstraintSet& cset) {

	const auto& spcs = cset.getConstraintsByType(Constraint::Type::SPC);
	if (not spcs.empty()) {
		comm_file_ofs << "                   DDL_IMPO=(" << endl;
		for (const auto& constraint : spcs) {
			const auto& spc = dynamic_pointer_cast<SinglePointConstraint>(constraint);
			//FIXME: filter spcs with type function.
			if (spc->hasReferences()) {
				cerr << "SPC references not supported " << *spc << endl;
				comm_file_ofs << " ************************" << endl << "SPC references not supported "
						<< *spc
						<< endl;
			} else {
				comm_file_ofs << "                             _F(";
				writeNodeContainer(*spc);
				//parameter 0 ignored
				for (const DOF dof : spc->getDOFSForNode(0)) {
					if (dof == DOF::DX)
						comm_file_ofs << "DX";
					if (dof == DOF::DY)
						comm_file_ofs << "DY";
					if (dof == DOF::DZ)
						comm_file_ofs << "DZ";
					if (dof == DOF::RX)
						comm_file_ofs << "DRX";
					if (dof == DOF::RY)
						comm_file_ofs << "DRY";
					if (dof == DOF::RZ)
						comm_file_ofs << "DRZ";
					comm_file_ofs << "=" << spc->getDoubleForDOF(dof) << ", ";
				}
				comm_file_ofs << ")," << endl;
			}
			spc->markAsWritten();
		}
		comm_file_ofs << "                             )," << endl;
	}
}

void AsterWriter::writeSPCD( const LoadSet& lset) {

	const auto& spcds = lset.getLoadingsByType(Loading::Type::IMPOSED_DISPLACEMENT);
	if (not spcds.empty()) {
		comm_file_ofs << "                   DDL_IMPO=(" << endl;
		for (const auto& loading : spcds) {
			const auto& spcd = static_pointer_cast<ImposedDisplacement>(loading);
            comm_file_ofs << "                             _F(";
            writeNodeContainer(*spcd);
            for (const DOF dof : spcd->getDOFSForNode(0)) { // parameter 0 ignored
                if (dof == DOF::DX)
                    comm_file_ofs << "DX";
                if (dof == DOF::DY)
                    comm_file_ofs << "DY";
                if (dof == DOF::DZ)
                    comm_file_ofs << "DZ";
                if (dof == DOF::RX)
                    comm_file_ofs << "DRX";
                if (dof == DOF::RY)
                    comm_file_ofs << "DRY";
                if (dof == DOF::RZ)
                    comm_file_ofs << "DRZ";
                comm_file_ofs << "=" << spcd->getDoubleForDOF(dof) << ", ";
            }
            comm_file_ofs << ")," << endl;
            spcd->markAsWritten();
		}
		comm_file_ofs << "                             )," << endl;
	}
}

void AsterWriter::writeLIAISON_SOLIDE( const ConstraintSet& cset) {


	const auto& rigidConstraints = cset.getConstraintsByType(Constraint::Type::RIGID);
	const auto& quasiRigidConstraints = cset.getConstraintsByType(Constraint::Type::QUASI_RIGID);
	vector<shared_ptr<Constraint>> constraints;
	for (const auto& rigidConstraint : rigidConstraints) {
        constraints.push_back(rigidConstraint);
	}
	for (const auto& quasiRigidConstraint : quasiRigidConstraints) {
		if ((static_pointer_cast<QuasiRigidConstraint>(quasiRigidConstraint)->isCompletelyRigid())) {
			constraints.push_back(quasiRigidConstraint);
		}
	}

	if (not constraints.empty()) {
		comm_file_ofs << "                   LIAISON_SOLIDE=(" << endl;
		for (const auto& constraintPtr : constraints) {
			comm_file_ofs << "                                   _F(NOEUD=(";
			for (int node : constraintPtr->nodePositions()) {
				comm_file_ofs << "'" << Node::MedName(node) << "',";
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                      )," << endl;
			constraintPtr->markAsWritten();

		}
		comm_file_ofs << "                                   )," << endl;
	}
}

void AsterWriter::writeLIAISON_MAIL( const ConstraintSet& cset) {

	const auto& slideSurfaces = cset.getConstraintsByType(
			Constraint::Type::SURFACE_SLIDE_CONTACT);

	if (not slideSurfaces.empty()) {
		comm_file_ofs << "                   LIAISON_MAIL=(" << endl;
        for (const auto& constraint : slideSurfaces) {
		    const auto& surface = static_pointer_cast<SurfaceSlide>(constraint);
		    const auto& masterSurface = static_pointer_cast<BoundaryElementFace>(asterModel->model.find(surface->master));
		    const auto& slaveSurface = static_pointer_cast<BoundaryElementFace>(asterModel->model.find(surface->slave));
			comm_file_ofs << "                                   _F(TYPE_RACCORD='MASSIF', DDL_MAIT='DNOR', DDL_ESCL='DNOR',";
			comm_file_ofs << "GROUP_MA_MAIT='" << masterSurface->elementCellGroup->getName() << "',";
			comm_file_ofs << "GROUP_MA_ESCL='" << slaveSurface->surfaceCellGroup->getName() << "',";
            comm_file_ofs << ")," << endl;
            surface->markAsWritten();
            masterSurface->markAsWritten();
            slaveSurface->markAsWritten();
		}
		comm_file_ofs << "                                   )," << endl;
	}
}

void AsterWriter::writeRBE3( const ConstraintSet& cset) {

	const auto& constraints = cset.getConstraintsByType(Constraint::Type::RBE3);
	if (not constraints.empty()) {
		comm_file_ofs << "                   LIAISON_RBE3=(" << endl;
		for (const auto& constraint : constraints) {
			const auto& rbe3 = static_pointer_cast<RBE3>(constraint);
			int masterNode = rbe3->getMaster();
			comm_file_ofs << "                                 _F(NOEUD_MAIT='"
					<< Node::MedName(masterNode) << "',"
					<< endl;
			comm_file_ofs << "                                    DDL_MAIT=(";
			DOFS dofs = rbe3->getDOFSForNode(masterNode);
			if (dofs.contains(DOF::DX))
				comm_file_ofs << "'DX',";
			if (dofs.contains(DOF::DY))
				comm_file_ofs << "'DY',";
			if (dofs.contains(DOF::DZ))
				comm_file_ofs << "'DZ',";
			if (dofs.contains(DOF::RX))
				comm_file_ofs << "'DRX',";
			if (dofs.contains(DOF::RY))
				comm_file_ofs << "'DRY',";
			if (dofs.contains(DOF::RZ))
				comm_file_ofs << "'DRZ',";
			comm_file_ofs << ")," << endl;
			set<int> slaveNodes = rbe3->getSlaves();

			comm_file_ofs << "                                    NOEUD_ESCL=(";
			for (int slaveNode : slaveNodes) {
				comm_file_ofs << "'" << Node::MedName(slaveNode) << "',";
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                    DDL_ESCL=(";
			for (int slaveNode : slaveNodes) {
				DOFS slaveDofs = rbe3->getDOFSForNode(slaveNode);
				int size = 0;
				comm_file_ofs << "'";
				if (slaveDofs.contains(DOF::DX)) {
					comm_file_ofs << "DX";
					if (++size < slaveDofs.size())
						comm_file_ofs << "-";
				}
				if (slaveDofs.contains(DOF::DY)) {
					comm_file_ofs << "DY";
					if (++size < slaveDofs.size())
						comm_file_ofs << "-";
				}
				if (slaveDofs.contains(DOF::DZ)) {
					comm_file_ofs << "DZ";
					if (++size < slaveDofs.size())
						comm_file_ofs << "-";
				}
				if (slaveDofs.contains(DOF::RX)) {
					comm_file_ofs << "DRX";
					if (++size < slaveDofs.size())
						comm_file_ofs << "-";
				}
				if (slaveDofs.contains(DOF::RY)) {
					comm_file_ofs << "DRY";
					if (++size < slaveDofs.size())
						comm_file_ofs << "-";
				}
				if (slaveDofs.contains(DOF::RZ)) {
					comm_file_ofs << "DRZ";
				}
				comm_file_ofs << "',";
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                    COEF_ESCL=(";
			for (int slaveNode : slaveNodes) {
				comm_file_ofs << rbe3->getCoefForNode(slaveNode) << ",";
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                    )," << endl;
			rbe3->markAsWritten();
		}
		comm_file_ofs << "                                 )," << endl;
	}
}

void AsterWriter::writeLMPC( const ConstraintSet& cset) {

	const auto& lmpcs = cset.getConstraintsByType(Constraint::Type::LMPC);
	if (not lmpcs.empty()) {
		comm_file_ofs << "                   LIAISON_DDL=(" << endl;
		for (const auto& constraint : lmpcs) {
			const auto& lmpc = static_pointer_cast<LinearMultiplePointConstraint>(constraint);
			comm_file_ofs << "                                _F(NOEUD=(";
			const auto& nodePositions = lmpc->nodePositions();
			for (int nodePosition : nodePositions) {
				string nodeName = Node::MedName(nodePosition);
				DOFS dofs = lmpc->getDOFSForNode(nodePosition);
				for (int i = 0; i < dofs.size(); i++) {
					comm_file_ofs << "'" << nodeName << "', ";
				}
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                   DDL=(";
			for (int nodePosition : nodePositions) {
				DOFS dofs = lmpc->getDOFSForNode(nodePosition);
				if (dofs.contains(DOF::DX))
					comm_file_ofs << "'DX', ";
				if (dofs.contains(DOF::DY))
					comm_file_ofs << "'DY', ";
				if (dofs.contains(DOF::DZ))
					comm_file_ofs << "'DZ', ";
				if (dofs.contains(DOF::RX))
					comm_file_ofs << "'DRX', ";
				if (dofs.contains(DOF::RY))
					comm_file_ofs << "'DRY', ";
				if (dofs.contains(DOF::RZ))
					comm_file_ofs << "'DRZ', ";
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                   COEF_MULT=(";
			for (int nodePosition : nodePositions) {
			    DOFCoefs dofcoef = lmpc->getDoFCoefsForNode(nodePosition);
				for (dof_int i = 0; i < 6; i++) {
					if (!is_zero(dofcoef[i]))
						comm_file_ofs << dofcoef[i] << ", ";
				}
			}
			comm_file_ofs << ")," << endl;
			comm_file_ofs << "                                   COEF_IMPO=" << lmpc->coef_impo << "),"
					<< endl;
            lmpc->markAsWritten();
		}
		comm_file_ofs << "                               )," << endl;
	}
}

void AsterWriter::writeGravity(const LoadSet& loadSet) {
	const auto& gravities = loadSet.getLoadingsByType(Loading::Type::GRAVITY);
	if (not gravities.empty()) {
		comm_file_ofs << "                      PESANTEUR=(" << endl;
		for (const auto& loading : gravities) {
			const auto& gravity = static_pointer_cast<Gravity>(loading);
			comm_file_ofs << "                                 _F(GRAVITE=" << gravity->getAccelerationVector().norm()
					<< "," << endl;
			VectorialValue direction = gravity->getAccelerationVector().normalized();
			comm_file_ofs << "                                    DIRECTION=(" << direction.x() << ","
					<< direction.y() << "," << direction.z() << "),)," << endl;
            gravity->markAsWritten();
		}
		comm_file_ofs << "                                 )," << endl;
	}
}

void AsterWriter::writeRotation(const LoadSet& loadSet) {
	const auto& rotations = loadSet.getLoadingsByType(Loading::Type::ROTATION);
	if (not rotations.empty()) {
		comm_file_ofs << "                      ROTATION=(" << endl;
		for (const auto& loading : rotations) {
			const auto& rotation = static_pointer_cast<Rotation>(loading);
			comm_file_ofs << "                                 _F(VITESSE=" << rotation->getSpeed() << ","
					<< endl;
			VectorialValue axis = rotation->getAxis();
			comm_file_ofs << "                                    AXE=(" << axis.x() << "," << axis.y() << ","
					<< axis.z() << ")," << endl;
			VectorialValue center = rotation->getCenter();
			comm_file_ofs << "                                    CENTRE=(" << center.x() << "," << center.y()
					<< "," << center.z() << ")";
			comm_file_ofs << ",)," << endl;
			rotation->markAsWritten();
		}
		comm_file_ofs << "                                 )," << endl;
	}
}

void AsterWriter::writeNodalForce( const LoadSet& loadSet) {

	const auto& nodalForces = loadSet.getLoadingsByType(Loading::Type::NODAL_FORCE);
	if (not nodalForces.empty()) {
        map<int, pair<VectorialValue, VectorialValue>> forceAndMomentByPosition;
		for (const auto& loading : nodalForces) {
			const auto& nodal_force = static_pointer_cast<NodalForce>(loading);
			for(const int nodePosition : nodal_force->nodePositions()) {
                VectorialValue force = nodal_force->getForceInGlobalCS(nodePosition);
                VectorialValue moment = nodal_force->getMomentInGlobalCS(nodePosition);

                const auto& it = forceAndMomentByPosition.find(nodePosition);
                if (it == forceAndMomentByPosition.end()) {
                    forceAndMomentByPosition[nodePosition] = {force, moment};
                } else {
                    forceAndMomentByPosition[nodePosition] = {force+it->second.first, moment+it->second.second};
                }
			}
			nodal_force->markAsWritten();
		}

		comm_file_ofs << "                      FORCE_NODALE=(" << endl;
		for (const auto& forceAndMomentEntry : forceAndMomentByPosition) {
            int nodePosition = forceAndMomentEntry.first;
            VectorialValue force = forceAndMomentEntry.second.first;
            VectorialValue moment = forceAndMomentEntry.second.second;
            comm_file_ofs << "                                    _F(NOEUD='"
                    << Node::MedName(nodePosition) << "',";
            if (!is_zero(force.x()))
                comm_file_ofs << "FX=" << force.x() << ",";
            if (!is_zero(force.y()))
                comm_file_ofs << "FY=" << force.y() << ",";
            if (!is_zero(force.z()))
                comm_file_ofs << "FZ=" << force.z() << ",";
            if (!is_zero(moment.x()))
                comm_file_ofs << "MX=" << moment.x() << ",";
            if (!is_zero(moment.y()))
                comm_file_ofs << "MY=" << moment.y() << ",";
            if (!is_zero(moment.z()))
                comm_file_ofs << "MZ=" << moment.z() << ",";
            comm_file_ofs << ")," << endl;
		}
		comm_file_ofs << "                                    )," << endl;
	}
}

void AsterWriter::writePression(const LoadSet& loadSet) {
	const auto& loading = loadSet.getLoadingsByType(
			Loading::Type::NORMAL_PRESSION_FACE);
	if (not loading.empty()) {
		comm_file_ofs << "           PRES_REP=(" << endl;
		for (const auto& pressionFace : loading) {
			const auto& normalPressionFace = static_pointer_cast<NormalPressionFace>(pressionFace);
			comm_file_ofs << "                         _F(PRES= " << normalPressionFace->intensity << ",";
			writeCellContainer(*normalPressionFace);
			comm_file_ofs << "                         )," << endl;
			pressionFace->markAsWritten();
		}
		comm_file_ofs << "                      )," << endl;
	}
}

void AsterWriter::writeForceCoque(const LoadSet& loadSet) {
    return; // TODO : change type of loading in parser to something specific for shell elements
	const auto& pressionFaces = loadSet.getLoadingsByType(Loading::Type::NORMAL_PRESSION_FACE);
	if (not pressionFaces.empty()) {
		comm_file_ofs << "           FORCE_COQUE=(" << endl;
		for (const auto& pressionFace : pressionFaces) {
			const auto& normalPressionFace = static_pointer_cast<NormalPressionFace>(pressionFace);
			comm_file_ofs << "                        _F(PRES=" << normalPressionFace->intensity << ",";
			writeCellContainer(*normalPressionFace);
			comm_file_ofs << "                         )," << endl;
			normalPressionFace->markAsWritten();
		}
		comm_file_ofs << "            )," << endl;
	}
}

void AsterWriter::writeForceLine(const LoadSet& loadset) {
	const auto& forcesLine = loadset.getLoadingsByType(Loading::Type::FORCE_LINE);
	vector<shared_ptr<ForceLine>> forcesOnPoutres;
	vector<shared_ptr<ForceLine>> forcesOnGeometry;

	for (const auto& loadingForceLine : forcesLine) {
		const auto& forceLine = static_pointer_cast<ForceLine>(loadingForceLine);
		if (forceLine->appliedToGeometry()) {
			forcesOnGeometry.push_back(forceLine);
		} else {
			forcesOnPoutres.push_back(forceLine);
		}
	}
	if (not forcesOnPoutres.empty()) {
		comm_file_ofs << "           FORCE_POUTRE=(" << endl;
		for (const auto& forceLine : forcesOnPoutres) {
            comm_file_ofs << "                   _F(";
            switch(forceLine->dof.code) {
            case DOF::Code::DX_CODE:
                comm_file_ofs << "FX";
                break;
            case DOF::Code::DY_CODE:
                comm_file_ofs << "FY";
                break;
            case DOF::Code::DZ_CODE:
                comm_file_ofs << "FZ";
                break;
            case DOF::Code::RX_CODE:
                comm_file_ofs << "MX";
                break;
            case DOF::Code::RY_CODE:
                comm_file_ofs << "MY";
                break;
            case DOF::Code::RZ_CODE:
                comm_file_ofs << "MZ";
                break;
            default:
                handleWritingError("DOF not yet handled");
            }
            comm_file_ofs << "=" << asternameByValue[forceLine->force] << ",";
            writeCellContainer(*forceLine);
            comm_file_ofs << "          )," << endl;
            forceLine->markAsWritten();
		}
		comm_file_ofs << "            )," << endl;
	}

	if (not forcesOnGeometry.empty()) {
        handleWritingWarning("# warn! force_arete not implemented");

	}

}
void AsterWriter::writeForceSurface(const LoadSet& loadSet) {
	const auto& forceSurfaces = loadSet.getLoadingsByType(Loading::Type::FORCE_SURFACE);
	if (not forceSurfaces.empty()) {
		comm_file_ofs << "           FORCE_FACE=(" << endl;
		for (const auto& loading : forceSurfaces) {
			const auto& forceSurface = static_pointer_cast<ForceSurface>(loading);
			VectorialValue force = forceSurface->getForce();
			VectorialValue moment = forceSurface->getMoment();
			comm_file_ofs << "                       _F(";
			writeCellContainer(*forceSurface);
			if (!is_equal(force.x(), 0))
				comm_file_ofs << "FX=" << force.x() << ",";
			if (!is_equal(force.y(),0))
				comm_file_ofs << "FY=" << force.y() << ",";
			if (!is_equal(force.z(),0))
				comm_file_ofs << "FZ=" << force.z() << ",";
			if (!is_equal(moment.x(), 0))
				comm_file_ofs << "MX=" << moment.x() << ",";
			if (!is_equal(moment.y(), 0))
				comm_file_ofs << "MY=" << moment.y() << ",";
			if (!is_equal(moment.z(), 0))
				comm_file_ofs << "MZ=" << moment.z() << ",";
			comm_file_ofs << ")," << endl;
			forceSurface->markAsWritten();
		}
		comm_file_ofs << "            )," << endl;
	}
}

void AsterWriter::writeNodeContainer(const NodeContainer& nodeContainer) {
    int cnode = 0;
    if (nodeContainer.hasNodeGroups()) {
      comm_file_ofs << "GROUP_NO=(";
      for (const auto& nodeGroup : nodeContainer.getNodeGroups()) {
        if (nodeGroup->empty())
            continue;
        cnode++;
        comm_file_ofs << "'" << nodeGroup->getName() << "',";
        if (cnode % 6 == 0) {
          comm_file_ofs << endl << "                             ";
        }
      }
      comm_file_ofs << "),";
    }
    if (nodeContainer.hasNodesExcludingGroups()) {
      comm_file_ofs << "NOEUD=(";
      for (int nodePosition : nodeContainer.getNodePositionsExcludingGroups()) {
        cnode++;
        comm_file_ofs << "'" << Node::MedName(nodePosition) << "',";
        if (cnode % 6 == 0) {
          comm_file_ofs << endl << "                             ";
        }
      }
      comm_file_ofs << "),";
    }
    writeCellContainer(nodeContainer);
}

void AsterWriter::writeCellContainer(const CellContainer& cellContainer) {
    int celem = 0;
    if (cellContainer.hasCellGroups()) {
      comm_file_ofs << "GROUP_MA=(";
      for (const auto& cellGroup : cellContainer.getCellGroups()) {
        if (cellGroup->empty())
            continue; // empty cell groups are useless and will probably never be created in mesh, causing errors.
        celem++;
        comm_file_ofs << "'" << cellGroup->getName() << "',";
        if (celem % 6 == 0) {
          comm_file_ofs << endl << "                             ";
        }
      }
      comm_file_ofs << "),";
    }
    if (cellContainer.hasCellsExcludingGroups()) {
        // Creating single cell groups to avoid using MAILLE
      comm_file_ofs << "MAILLE=(";
      //comm_file_ofs << "GROUP_MA=(";
      for (int cellPosition : cellContainer.getCellPositionsExcludingGroups()) {
        celem++;
//        const string& groupName = Cell::MedName(cellPosition);
//        auto entry = singleGroupCellPositions.find(cellPosition);
//        if (entry == end(singleGroupCellPositions)) {
//            auto singleCellGroup = asterModel->model.mesh.createCellGroup(groupName);
//            singleCellGroup->addCellPosition(cellPosition);
//            singleGroupCellPositions.insert(cellPosition);
//        }
        comm_file_ofs << "'" << Cell::MedName(cellPosition) << "',";
        if (celem % 6 == 0) {
          comm_file_ofs << endl << "                             ";
        }
      }
      comm_file_ofs << "),";
    }
}

shared_ptr<NonLinearStrategy> AsterWriter::getNonLinearStrategy(
		NonLinearMecaStat& nonLinAnalysis) {
	shared_ptr<NonLinearStrategy> nonLinearStrategy;
	const auto& strategy = nonLinAnalysis.model.find(nonLinAnalysis.strategy_reference);
    if (strategy == nullptr) {
        handleWritingError("Cannot find nonlinear strategy" + to_str(nonLinAnalysis.strategy_reference));
    }
	switch (strategy->type) {
	case Objective::Type::NONLINEAR_STRATEGY: {
		nonLinearStrategy = static_pointer_cast<NonLinearStrategy>(strategy);
		break;
	}
	default:
		// Nothing to do
		break;
	}
	return nonLinearStrategy;
}

void AsterWriter::writeAssemblage( Analysis& analysis) {
    bool hasDynamicExcit = asterModel->model.loadSets.contains(LoadSet::Type::DLOAD);
    bool isBuckling = analysis.type == Analysis::Type::LINEAR_BUCKLING;
    comm_file_ofs << "ASSEMBLAGE(MODELE=MODMECA," << endl;
    if (not asterModel->model.materials.empty()) {
        comm_file_ofs << "           CHAM_MATER=CHMAT," << endl;
    }
    comm_file_ofs << "           CARA_ELEM=CAEL," << endl;
    comm_file_ofs << "           CHARGE=(" << endl;
    for (const auto& constraintSet : analysis.getConstraintSets()) {
        comm_file_ofs << "                   BL" << constraintSet->getId() << "," << endl;
    }
    comm_file_ofs << "                   )," << endl;
    comm_file_ofs << "           NUME_DDL=CO('NUMDDL" << analysis.getId() << "')," << endl;
    comm_file_ofs << "           MATR_ASSE=(_F(OPTION='RIGI_MECA', MATRICE=CO('RIGI"
            << analysis.getId() << "'),)," << endl;
    if (isBuckling) {
        comm_file_ofs << "                      _F(OPTION='RIGI_GEOM', MATRICE=CO('RIGE"
                << analysis.getId() << "'),SIEF_ELGA=FSIG" << analysis.getId() << ",)," << endl;
    } else {
        comm_file_ofs << "                      _F(OPTION='MASS_MECA', MATRICE=CO('MASS"
                << analysis.getId() << "'),)," << endl;
        comm_file_ofs << "                      _F(OPTION='AMOR_MECA', MATRICE=CO('AMOR"
                << analysis.getId() << "'),)," << endl;
    }
    comm_file_ofs << "                      )," << endl;
    if (hasDynamicExcit) {
        comm_file_ofs << "           VECT_ASSE=(" << endl;
        for (const auto& loadSet : analysis.getLoadSets()) {
            for (const auto& loading : loadSet->getLoadings()) {
                if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
                    const auto& dynamicExcitation =
                            static_pointer_cast<DynamicExcitation>(loading);
                    comm_file_ofs << "                      _F(OPTION='CHAR_MECA', VECTEUR=CO('FX"
                            << analysis.getId() << "_" << dynamicExcitation->getId() << "')," << endl;
                    comm_file_ofs << "                        CHARGE=(" << endl;
                    comm_file_ofs << "                                CHMEC"
                            << dynamicExcitation->getLoadSet()->getId() << "," << endl;
                    comm_file_ofs << "                                )," << endl;
                    comm_file_ofs << "                      )," << endl;
                    dynamicExcitation->markAsWritten();
                    loadSet->markAsWritten();
                }
            }
        }
        comm_file_ofs << "             )," << endl;
    }
    comm_file_ofs << "           );" << endl << endl;
}

void AsterWriter::writeCalcFreq( LinearModal& linearModal) {
    string suffix;
    bool isBuckling = linearModal.type == Analysis::Type::LINEAR_BUCKLING;
    if (isBuckling) {
        suffix = "CHAR_CRIT";
    } else {
        suffix = "FREQ";
    }
    const auto& frequencySearch = linearModal.getFrequencySearch();
    if (not isBuckling) {
        switch (frequencySearch->norm) {
        case(FrequencySearch::NormType::MASS): {
            comm_file_ofs << "                       NORM_MODE=_F(NORME='MASS_GENE')," << endl;
            break;
        }
        case(FrequencySearch::NormType::MAX): {
            comm_file_ofs << "                       NORM_MODE=_F(NORME='TRAN_ROTA')," << endl;
            break;
        }
        default:
            handleWritingError(
                    "Norm for frequency search " + to_string(static_cast<int>(frequencySearch->frequencyType)) + " not (yet) implemented");
        }
    }
    switch(frequencySearch->frequencyType) {
    case FrequencySearch::FrequencyType::BAND: {
        const auto& band = static_pointer_cast<BandRange>(frequencySearch->getValue());
        double fstart = band->start;
        if (is_equal(fstart, Globals::UNAVAILABLE_DOUBLE)) {
            auto lower_cutoff_frequency = asterModel->model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY);
            if (lower_cutoff_frequency != asterModel->model.parameters.end()) {
                if (asterModel->model.configuration.logLevel >= LogLevel::TRACE) {
                    cout << "Parameter LOWER_CUTOFF_FREQUENCY present, redefining frequency band" << endl;
                }
                fstart = lower_cutoff_frequency->second;
            } else {
                fstart = 0.0;
            }
        }
        double fend = band->end;
        if (isBuckling and not is_equal(band->end, Globals::UNAVAILABLE_DOUBLE)) {
            fstart = -fstart;
            fend = -fend;
            swap(fstart, fend);
        }
        if (linearModal.use_power_iteration) {
            comm_file_ofs << "                       OPTION='SEPARE'," << endl;
        } else if (isBuckling or (is_equal(fstart, Globals::UNAVAILABLE_DOUBLE) and is_equal(fend, Globals::UNAVAILABLE_DOUBLE))) {
            // CALC_CHAR_CRIT is not filtering eigenvalues correctly in Aster 13.6
            comm_file_ofs << "                       OPTION='PLUS_PETITE'," << endl;
        } else if (is_equal(fend, Globals::UNAVAILABLE_DOUBLE)) {
            comm_file_ofs << "                       OPTION='CENTRE'," << endl; // LD : could be replaced by PLUS_PETITE + FILTRE
        } else {
            comm_file_ofs << "                       OPTION='BANDE'," << endl;
        }
        comm_file_ofs << "                       CALC_" << suffix << "=_F(" << endl;
        if (not isBuckling and not is_equal(fstart, Globals::UNAVAILABLE_DOUBLE)) {
            comm_file_ofs << "                                    " << suffix << "=(";
            comm_file_ofs << fstart;
            if (is_equal(fend, Globals::UNAVAILABLE_DOUBLE)) {
                comm_file_ofs << ", " << ")," << endl;
            } else {
                comm_file_ofs << ", " << fend;
                comm_file_ofs << ")," << endl;
            }
        } else if (not is_equal(band->maxsearch, Globals::UNAVAILABLE_DOUBLE)) {
            comm_file_ofs << "                                    NMAX_" << suffix << "=" << band->maxsearch << endl;
        }

        comm_file_ofs << "                                    )," << endl;
        break;
    }
    case FrequencySearch::FrequencyType::STEP: {
        const auto& frequencyStep = static_pointer_cast<StepRange>(frequencySearch->getValue());
        if (linearModal.use_power_iteration) {
            comm_file_ofs << "                       OPTION='SEPARE'," << endl; // Not using 'PROCHE' because it will always produce modes, even if not finding them
        } else {
            comm_file_ofs << "                       OPTION='CENTRE'," << endl;
        }
        comm_file_ofs << "                       CALC_" << suffix << "=_F(" << endl;
        comm_file_ofs << "                                    " << suffix << "=(";
        for (double frequency = frequencyStep->start; frequency < frequencyStep->end; frequency += frequencyStep->step) {
            comm_file_ofs << frequency << ",";
        }
        comm_file_ofs << ")," << endl;
        comm_file_ofs << "                                    )," << endl;
        break;
    }
    case FrequencySearch::FrequencyType::LIST: {
      const auto& frequencyList = static_pointer_cast<ListValue<double>>(frequencySearch->getValue());
      if (linearModal.use_power_iteration) {
          comm_file_ofs << "                       OPTION='SEPARE'," << endl; // Not using 'PROCHE' because it will always produce modes, even if not finding them
      } else {
          comm_file_ofs << "                       OPTION='CENTRE'," << endl;
      }
      comm_file_ofs << "                       CALC_" << suffix << "=_F(" << endl;
      comm_file_ofs << "                                    " << suffix << "=(";
      for (const double frequency : frequencyList->getList()) {
          comm_file_ofs << frequency << ",";
      }
      comm_file_ofs << ")," << endl;
      comm_file_ofs << "                                    )," << endl;
      break;
    }
    default:
        handleWritingError(
                "Frequency search " + to_string(static_cast<int>(frequencySearch->frequencyType)) + " not (yet) implemented");
    }
}

double AsterWriter::writeAnalysis( Analysis& analysis, double debut) {
	if (analysis.isOriginal()) {
		comm_file_ofs << "# Analysis original id : " << analysis.getOriginalId() << endl;
	}
	switch (analysis.type) {
    case Analysis::Type::COMBINATION: {
        auto& combination = static_cast<Combination&>(analysis);
        for (const auto& subPair : combination.coefByAnalysis) {
            comm_file_ofs << "D" << combination.getId() << "_" << subPair.first.id << "=CREA_CHAMP(TYPE_CHAM='NOEU_DEPL_R'," << endl;
            comm_file_ofs << "          OPERATION='EXTR'," << endl;
            comm_file_ofs << "          RESULTAT=RESU" << subPair.first.id << "," << endl;
            comm_file_ofs << "          NOM_CHAM='DEPL'," << endl;
            comm_file_ofs << "          NUME_ORDRE=1,);" << endl << endl;
        }

        comm_file_ofs << "DEP" << combination.getId() << "=CREA_CHAMP(TYPE_CHAM='NOEU_DEPL_R'," << endl;
        comm_file_ofs << "          OPTION='DEPL'," << endl;
        comm_file_ofs << "          OPERATION='ASSE'," << endl;
        comm_file_ofs << "          MODELE=MODMECA," << endl;
        comm_file_ofs << "          ASSE=(" << endl;
        for (const auto& subPair : combination.coefByAnalysis) {
            comm_file_ofs << "                _F(TOUT='OUI'," << endl;
            comm_file_ofs << "                   CHAM_GD=D" << combination.getId() << "_" << subPair.first.id << "," << endl;
            comm_file_ofs << "                   CUMUL='OUI'," << endl;
            comm_file_ofs << "                   COEF_R=" << subPair.second << ",)," << endl;
        }
        comm_file_ofs << "                )," << endl;
        comm_file_ofs << "          )" << endl << endl;

        comm_file_ofs << "RESU" << combination.getId() << "=CREA_RESU(OPERATION='AFFE'," << endl;
        comm_file_ofs << "              TYPE_RESU='MULT_ELAS'," << endl;
        comm_file_ofs << "              NOM_CHAM='DEPL'," << endl;
        comm_file_ofs << "              AFFE=_F(CHAM_GD=DEP" << combination.getId() << "," << endl;
        comm_file_ofs << "                      MODELE=MODMECA," << endl;
        comm_file_ofs << "                      CHAM_MATER=CHMAT," << endl;
        comm_file_ofs << "                      )," << endl;
        comm_file_ofs << "              )" << endl << endl;
        combination.markAsWritten();
        break;
    }
	case Analysis::Type::LINEAR_MECA_STAT: {
		auto& linearMecaStat = static_cast<LinearMecaStat&>(analysis);

		comm_file_ofs << "RESU" << linearMecaStat.getId() << "=MECA_STATIQUE(MODELE=MODMECA," << endl;
		if (not asterModel->model.materials.empty()) {
            comm_file_ofs << "                    CHAM_MATER=CHMAT," << endl;
		}
		comm_file_ofs << "                    CARA_ELEM=CAEL," << endl;
		comm_file_ofs << "                    EXCIT=(" << endl;
		for (const auto& loadSet : linearMecaStat.getLoadSets()) {
			comm_file_ofs << "                           _F(CHARGE=" << asternameByLoadSet[loadSet] << ")," << endl;
		}
		for (const auto& constraintSet : linearMecaStat.getConstraintSets()) {
			//GC: dirty fix for #801, a deeper analysis must be done
			if (not constraintSet->empty()) {
                cout << "constraintSet:" << *constraintSet << " AsterName: " << asternameByConstraintSet[constraintSet] << "Size:" << constraintSet->size() << endl;
				comm_file_ofs << "                           _F(CHARGE=" << asternameByConstraintSet[constraintSet] << "),"
						<< endl;
			}
		}
		comm_file_ofs << "                           )," << endl;
        comm_file_ofs << "                    SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS')," << endl;
        comm_file_ofs << "                    OPTION='SIEF_ELGA'," << endl;
		comm_file_ofs << "                    );" << endl << endl;
		linearMecaStat.markAsWritten();
		break;
	}
	case Analysis::Type::NONLINEAR_MECA_STAT: {
		auto& nonLinAnalysis = static_cast<NonLinearMecaStat&>(analysis);
		shared_ptr<NonLinearStrategy> nonLinearStrategy = getNonLinearStrategy(nonLinAnalysis);
		if (!nonLinAnalysis.previousAnalysis) {
			debut = 0;
		}
		double fin = debut + 1.0;
		StepRange stepRange(asterModel->model, debut, nonLinearStrategy->number_of_increments, fin);
		debut = fin;
		string list_name = writeValue(stepRange);
		comm_file_ofs << "LAUTO" << nonLinAnalysis.getId()
				<< "=DEFI_LIST_INST(METHODE='AUTO', DEFI_LIST=_F(LIST_INST=" << list_name << ",),);"
				<< endl;
		comm_file_ofs << "RAMP" << nonLinAnalysis.getId()
				<< "=DEFI_FONCTION(NOM_PARA='INST', PROL_DROITE='LINEAIRE', VALE=("
				<< stepRange.start << ",0.0," << stepRange.end << ",1.0,));" << endl;
		if (nonLinAnalysis.previousAnalysis) {
			comm_file_ofs << "IRAMP" << nonLinAnalysis.getId()
					<< "=DEFI_FONCTION(NOM_PARA='INST', PROL_DROITE='LINEAIRE', VALE=("
					<< stepRange.start << ",1.0," << stepRange.end << ",0.0,));" << endl;
		}
		comm_file_ofs << "RESU" << nonLinAnalysis.getId() << "=STAT_NON_LINE(MODELE=MODMECA," << endl;
		if (not asterModel->model.materials.empty()) {
            comm_file_ofs << "                    CHAM_MATER=CHMAT," << endl;
		}
		comm_file_ofs << "                    CARA_ELEM=CAEL," << endl;
		comm_file_ofs << "                    EXCIT=(" << endl;
		if (nonLinAnalysis.previousAnalysis) {
			for (const auto& loadSet : nonLinAnalysis.previousAnalysis->getLoadSets()) {
				comm_file_ofs << "                           _F(CHARGE=" << asternameByLoadSet[loadSet]
						<< ",FONC_MULT=IRAMP" << nonLinAnalysis.getId() << ")," << "# Original id:" << loadSet->getOriginalId() << endl;
			}
		}
		for (const auto& loadSet : nonLinAnalysis.getLoadSets()) {
			comm_file_ofs << "                           _F(CHARGE=" << asternameByLoadSet[loadSet]
					<< ",FONC_MULT=RAMP" << nonLinAnalysis.getId() << ")," << "# Original id:" << loadSet->getOriginalId() << endl;
		}
		for (const auto& constraintSet : nonLinAnalysis.getConstraintSets()) {
			if (constraintSet->hasContacts()) {
				continue;
			}
			//GC: dirty fix for #801, a deeper analysis must be done
			if (not constraintSet->getConstraints().empty()) {
				comm_file_ofs << "                           _F(CHARGE=" << asternameByConstraintSet[constraintSet] << "),"
						 << "# Original id:" << constraintSet->getOriginalId() << endl;
			}
		}
		comm_file_ofs << "                           )," << endl;
		for (const auto& constraintSet : asterModel->model.constraintSets) {
			if (constraintSet->empty()) {
				// LD filter empty constraintSet
				continue;
			}
			if (not constraintSet->hasContacts()) {
				continue;
			}
			comm_file_ofs << "                    CONTACT=" << asternameByConstraintSet[constraintSet] << "," << "# Original id:" << constraintSet->getOriginalId() << endl;
		}
		comm_file_ofs << "                    COMPORTEMENT=(" << endl;
		for (const auto& elementSet : asterModel->model.elementSets) {
			if (elementSet->material != nullptr and elementSet->effective()) {
				comm_file_ofs << "                          _F(";
				const auto& cellElementSet = dynamic_pointer_cast<CellElementSet>(elementSet);
				if (cellElementSet == nullptr) {
                    handleWritingError("Writing of ElementSet which is not a CellContainer is not yet implemented");
				}
				writeCellContainer(*cellElementSet);
                const auto& hyelas = elementSet->material->findNature(
                        Nature::NatureType::NATURE_HYPERELASTIC);
                const auto& binature = elementSet->material->findNature(
                        Nature::NatureType::NATURE_BILINEAR_ELASTIC);
                const auto& nlelas = elementSet->material->findNature(
                        Nature::NatureType::NATURE_NONLINEAR_ELASTIC);
                if (binature) {
                    comm_file_ofs << "RELATION='VMIS_ISOT_LINE',";
                } else if (nlelas) {
                    comm_file_ofs << "RELATION='ELAS_VMIS_LINE',";
                } else if (hyelas) {
                    comm_file_ofs << "RELATION='ELAS_HYPER',";
                    comm_file_ofs << "DEFORMATION='GROT_GDEP',";
                } else if (asterModel->model.needsLargeDisplacements() and (elementSet->isBeam() or elementSet->isTruss())) {
                    comm_file_ofs << "RELATION='ELAS_POUTRE_GR',";
                    comm_file_ofs << "DEFORMATION='GROT_GDEP',";
                } else {
                    comm_file_ofs << "RELATION='ELAS',";
                }
                comm_file_ofs << ")," << endl;
			} else {
//				comm_file_ofs << "# WARN Skipping material id " << *elementSet << " because no assignment"
//						<< endl;
			}

		}
		comm_file_ofs << "                           )," << endl;
		comm_file_ofs << "                    INCREMENT=_F(LIST_INST=LAUTO" << nonLinAnalysis.getId() << ",),"
				<< endl;
		comm_file_ofs << "                    ARCHIVAGE=_F(LIST_INST=" << list_name << ",)," << endl;
		comm_file_ofs << "                    NEWTON=_F(REAC_ITER=1,)," << endl;
		if (nonLinAnalysis.previousAnalysis) {
			comm_file_ofs << "                    ETAT_INIT=_F(EVOL_NOLI =RESU"
					<< nonLinAnalysis.previousAnalysis->getId() << ")," << endl;
		}
		comm_file_ofs << "                    SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS')," << endl;
		comm_file_ofs << "                    );" << endl << endl;
		nonLinAnalysis.markAsWritten();
		break;
	}
	case Analysis::Type::LINEAR_DYNA_DIRECT_FREQ: {
        writeAssemblage( analysis);
        auto structural_damping = asterModel->model.parameters.find(Model::Parameter::STRUCTURAL_DAMPING);
        auto frequency_of_interest_radians = asterModel->model.parameters.find(Model::Parameter::FREQUENCY_OF_INTEREST_RADIANS);
        bool has_structural_damping = structural_damping != asterModel->model.parameters.end() and frequency_of_interest_radians != asterModel->model.parameters.end() and frequency_of_interest_radians->second > 0;
		if (has_structural_damping) {
            comm_file_ofs << "AMST" << analysis.getId()
                <<  " = COMB_MATR_ASSE(COMB_R = _F ( MATR_ASSE = RIGI"
                << analysis.getId()
                << ", COEF_R = " << structural_damping->second / frequency_of_interest_radians->second << "))" << endl;
        }
		LinearDynaDirectFreq& linearDirect = dynamic_cast<LinearDynaDirectFreq&>(analysis);
        comm_file_ofs << "RESU" << linearDirect.getId() << " = DYNA_VIBRA(" << endl;
        comm_file_ofs << "                   TYPE_CALCUL='HARM'," << endl;
        comm_file_ofs << "                   BASE_CALCUL='PHYS'," << endl;
		comm_file_ofs << "                   MATR_MASS  = MASS" << linearDirect.getId() << ","
				<< endl;
		comm_file_ofs << "                   MATR_RIGI  = RIGI" << linearDirect.getId() << ","
				<< endl;
        if (has_structural_damping) {
            comm_file_ofs << "                   MATR_AMOR  = AMST" << linearDirect.getId() << ","
				<< endl;
        } else /* LD Maybe should combine these two cases ? */ {
            comm_file_ofs << "                   MATR_AMOR  = AMOR" << linearDirect.getId() << ","
				<< endl;
        }
        comm_file_ofs << "                   LIST_FREQ  = LST" << setfill('0') << setw(5)
        << linearDirect.getExcitationFrequencies()->getValue()->getId() << "," << endl;
		comm_file_ofs << "                   EXCIT      = (" << endl;
		for (const auto& loadSet : linearDirect.getLoadSets()) {
			for (const auto& loading : loadSet->getLoadings()) {
				if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
					const auto& dynamicExcitation =
                            static_pointer_cast<DynamicExcitation>(loading);
					comm_file_ofs << "                                 _F(" << endl;
                    comm_file_ofs << "                                    VECT_ASSE=FX"
                            << linearDirect.getId() << "_"
                            << dynamicExcitation->getId() << "," << endl;
					comm_file_ofs << "                                    FONC_MULT = FCT" << setfill('0')
							<< setw(5) << dynamicExcitation->getFunctionTableB()->getId() << ","
							<< endl;
					comm_file_ofs << "                                    PHAS_DEG = "
							<< dynamicExcitation->getDynaPhase()->get() << ",)," << endl;
                    dynamicExcitation->markAsWritten();
                    loadSet->markAsWritten();
				}
			}
		}
		comm_file_ofs << "                                 )," << endl;
		comm_file_ofs << "                   #SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS',NPREC=8)," << endl; // MUMPS: Error in function orderMinPriority no valid number of stages in multisector (#stages = 2)
		comm_file_ofs << "                   );" << endl << endl;
		linearDirect.markAsWritten();
	    break;
	}
	case Analysis::Type::LINEAR_MODAL:
	case Analysis::Type::LINEAR_DYNA_MODAL_FREQ: {
        writeAssemblage( analysis);

		auto& linearModal = static_cast<LinearModal&>(analysis);

		string resuName;
		if (analysis.type == Analysis::Type::LINEAR_MODAL)
			resuName = "RESU" + to_string(linearModal.getId());
		else
			resuName = "MODES" + to_string(linearModal.getId());
		comm_file_ofs << "U" << resuName << "=CALC_MODES(MATR_RIGI=RIGI" << linearModal.getId()
				<< "," << endl;
		comm_file_ofs << "                       MATR_MASS=MASS" << linearModal.getId() << "," << endl;
		if (linearModal.use_power_iteration) {
            comm_file_ofs << "                       SOLVEUR_MODAL=_F(OPTION_INV='DIRECT')," << endl;
		} else {
            comm_file_ofs << "                       SOLVEUR_MODAL=_F(METHODE='TRI_DIAG')," << endl;
		}
        writeCalcFreq( linearModal);
		comm_file_ofs << "                       VERI_MODE=_F(STOP_ERREUR='NON',)," << endl;
		//comm_file_ofs << "                       IMPRESSION=_F(CUMUL='OUI',CRIT_EXTR='MASS_EFFE_UN',TOUT_PARA='OUI')," << endl;
		comm_file_ofs << "                       SOLVEUR=_F(METHODE='MUMPS'," << endl;
		comm_file_ofs << "                                  RENUM='PORD'," << endl;
		comm_file_ofs << "                                  NPREC=8," << endl;
		comm_file_ofs << "                                  )," << endl;
		comm_file_ofs << "                       )" << endl << endl;

		double lowFreq = 0;
		if (asterModel->model.parameters.find(Model::Parameter::LOWER_CUTOFF_FREQUENCY) != asterModel->model.parameters.end()) {
		    lowFreq = asterModel->model.parameters[Model::Parameter::LOWER_CUTOFF_FREQUENCY];
		}

		double highFreq = 1e30;
		if (asterModel->model.parameters.find(Model::Parameter::UPPER_CUTOFF_FREQUENCY) != asterModel->model.parameters.end()) {
		    highFreq = asterModel->model.parameters[Model::Parameter::UPPER_CUTOFF_FREQUENCY];
		}
		comm_file_ofs << resuName << "=EXTR_MODE(FILTRE_MODE=_F(MODE=U" << resuName << ", FREQ_MIN=" << lowFreq << ", FREQ_MAX=" << highFreq << "),)" << endl;

		comm_file_ofs << "I" << resuName << "=RECU_TABLE(CO=" << resuName << ",NOM_PARA = ('FREQ','MASS_GENE','RIGI_GENE','AMOR_GENE'))" << endl;
        comm_file_ofs << "IMPR_TABLE(TABLE=I" << resuName << ")" << endl << endl;
        comm_file_ofs << "J" << resuName << "=POST_ELEM(RESULTAT=" << resuName << ", MASS_INER=_F(TOUT='OUI'))" << endl;
        comm_file_ofs << "IMPR_TABLE(TABLE=J" << resuName << ")" << endl << endl;

		if (analysis.type == Analysis::Type::LINEAR_MODAL) {
            linearModal.markAsWritten();
			break;
		}

		LinearDynaModalFreq& linearDynaModalFreq = dynamic_cast<LinearDynaModalFreq&>(analysis);

		if (linearDynaModalFreq.residual_vector) {
			comm_file_ofs << "MOSTA" << linearDynaModalFreq.getId() << "=MODE_STATIQUE(MATR_RIGI=RIGI"
					<< linearDynaModalFreq.getId() << "," << endl;
			comm_file_ofs << "                     MATR_MASS=MASS" << linearDynaModalFreq.getId() << ","
					<< endl;
			comm_file_ofs << "                     FORCE_NODALE=(" << endl;
			for (const auto& loadSet : linearDynaModalFreq.getLoadSets()) {
				for (const auto& loading : loadSet->getLoadings()) {
					if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
                        const auto& dynamicExcitation =
                            static_pointer_cast<DynamicExcitation>(loading);
						const auto& nodalForces =
								dynamicExcitation->getLoadSet()->getLoadingsByType(
										Loading::Type::NODAL_FORCE);
						for (const auto& loading2 : nodalForces) {
							const auto& nodal_force = dynamic_pointer_cast<NodalForce>(
									loading2);
                            for(const int nodePosition : nodal_force->nodePositions()) {
                                const auto& force = nodal_force->getForceInGlobalCS(nodePosition);
                                const auto& moment = nodal_force->getMomentInGlobalCS(nodePosition);
                                comm_file_ofs << "                                    _F(NOEUD='"
                                        << Node::MedName(nodePosition) << "'," << endl;
                                comm_file_ofs << "                                      AVEC_CMP=(";
                                if (!is_equal(force.x(),0))
                                    comm_file_ofs << "'DX',";
                                if (!is_equal(force.y(),0))
                                    comm_file_ofs << "'DY',";
                                if (!is_equal(force.z(),0))
                                    comm_file_ofs << "'DZ',";
                                if (!is_equal(moment.x(),0))
                                    comm_file_ofs << "'DRX',";
                                if (!is_equal(moment.y(),0))
                                    comm_file_ofs << "'DRY',";
                                if (!is_equal(moment.z(),0))
                                    comm_file_ofs << "'DRZ',";
                                comm_file_ofs << "))," << endl;
                            }
                            dynamicExcitation->markAsWritten();
                            loadSet->markAsWritten();
						}
					}
				}
			}
			comm_file_ofs << "                                   )" << endl;
			comm_file_ofs << "                     );" << endl << endl;

			comm_file_ofs << "RESVE" << linearDynaModalFreq.getId()
					<< "=DEFI_BASE_MODALE(RITZ =(_F(MODE_MECA=MODES" << linearDynaModalFreq.getId()
					<< ",)," << endl;
			comm_file_ofs << "                               _F(MODE_INTF=MOSTA"
					<< linearDynaModalFreq.getId() << ",)," << endl;
			comm_file_ofs << "                               )," << endl;
			comm_file_ofs << "                        NUME_REF=NUMDDL" << linearDynaModalFreq.getId() << ","
					<< endl;
			comm_file_ofs << "                        MATRICE=MASS" << linearDynaModalFreq.getId() << ","
					<< endl;
			comm_file_ofs << "                        ORTHO='OUI'," << endl;
			comm_file_ofs << "                        );" << endl << endl;

			comm_file_ofs << "modes" << linearDynaModalFreq.getId() << "=" << "RESVE"
					<< linearDynaModalFreq.getId() << endl;
		} else {
			comm_file_ofs << "modes" << linearDynaModalFreq.getId() << "=" << "MODES"
					<< linearDynaModalFreq.getId() << endl;
		}

        comm_file_ofs << "PROJ_BASE(BASE=modes" << linearDynaModalFreq.getId() << "," << endl;
        comm_file_ofs << "          MATR_ASSE_GENE=(_F(MATRICE=CO('MASSG" << linearDynaModalFreq.getId()
                << "')," << endl;
        comm_file_ofs << "                             MATR_ASSE=MASS" << linearDynaModalFreq.getId() << ",),"
                << endl;
        comm_file_ofs << "                          _F(MATRICE=CO('RIGIG" << linearDynaModalFreq.getId()
                << "')," << endl;
        comm_file_ofs << "                             MATR_ASSE=RIGI" << linearDynaModalFreq.getId() << ",),"
                << endl;
        if (linearDynaModalFreq.getModalDamping() == nullptr) {
            comm_file_ofs << "                          _F(MATRICE=CO('AMORG" << linearDynaModalFreq.getId()
                    << "')," << endl;
            comm_file_ofs << "                             MATR_ASSE=AMOR" << linearDynaModalFreq.getId() << ",),"
                    << endl;
        }
        comm_file_ofs << "                          )," << endl;
        comm_file_ofs << "          VECT_ASSE_GENE=(" << endl;
        for (const auto& loadSet : linearDynaModalFreq.getLoadSets()) {
            for (const auto& loading : loadSet->getLoadings()) {
                if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
                    const auto& dynamicExcitation = dynamic_pointer_cast<DynamicExcitation>(loading);
                    comm_file_ofs << "                          _F(VECTEUR=CO('VG"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation->getId() << "')," << endl;
                    comm_file_ofs << "                             VECT_ASSE=FX"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation->getId() << ",";
                    comm_file_ofs << "TYPE_VECT=";
                    switch(dynamicExcitation->excitType) {
                    case DynamicExcitation::DynamicExcitationType::LOAD: {
                        comm_file_ofs << "'FORC',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::DISPLACEMENT: {
                        comm_file_ofs << "'DEPL',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::VELOCITY: {
                        comm_file_ofs << "'VITE',";
                        break;
                    };
                    case DynamicExcitation::DynamicExcitationType::ACCELERATION: {
                        comm_file_ofs << "'ACCE',";
                        break;
                    };
                    default:
                        handleWritingError("Dynamic excitation type " + to_string(static_cast<int>(dynamicExcitation->excitType)) + " not (yet) implemented");
                    }
                    comm_file_ofs << ")," << endl;
                    dynamicExcitation->markAsWritten();
                    loadSet->markAsWritten();
                }
            }
        }
        comm_file_ofs << "                         )," << endl;
        comm_file_ofs << "          );" << endl << endl;

		comm_file_ofs << "LIMODE" << linearDynaModalFreq.getId() << "=RECU_TABLE(CO=MODES"
				<< linearDynaModalFreq.getId() << "," << endl;
		comm_file_ofs << "                  NOM_PARA = 'FREQ');" << endl << endl;

		comm_file_ofs << "pfreq" << linearDynaModalFreq.getId() << "= LIMODE" << linearDynaModalFreq.getId()
				<< ".EXTR_TABLE().values()['FREQ']" << endl;

    if (linearDynaModalFreq.getModalDamping() != nullptr) {
      comm_file_ofs << "AMMO_I" << linearDynaModalFreq.getId() << "=CALC_FONC_INTERP(FONCTION = FCT"
          << setfill('0') << setw(5)
          << linearDynaModalFreq.getModalDamping()->getFunctionTable()->getId() << ","
          << endl;
      comm_file_ofs << "                         VALE_PARA = pfreq" << linearDynaModalFreq.getId() << endl;
      comm_file_ofs << "                         );" << endl << endl;

      comm_file_ofs << "AMMO_T" << linearDynaModalFreq.getId()
          << "=CREA_TABLE(FONCTION=_F(FONCTION = AMMO_I" << linearDynaModalFreq.getId()
          << ")," << endl;
      comm_file_ofs << "                   );" << endl << endl;

      comm_file_ofs << "AMMO" << linearDynaModalFreq.getId() << "=AMMO_T" << linearDynaModalFreq.getId()
          << ".EXTR_TABLE().values()['TOUTRESU']" << endl;
    }

    comm_file_ofs << "GENE" << linearDynaModalFreq.getId() << " = DYNA_VIBRA(" << endl;
    comm_file_ofs << "                   TYPE_CALCUL='HARM'," << endl;
    comm_file_ofs << "                   BASE_CALCUL='GENE'," << endl;
    comm_file_ofs << "                   MATR_MASS  = MASSG" << linearDynaModalFreq.getId() << ","
            << endl;
    comm_file_ofs << "                   MATR_RIGI  = RIGIG" << linearDynaModalFreq.getId() << ","
            << endl;
    if (linearDynaModalFreq.getModalDamping() != nullptr) {
        comm_file_ofs << "                   AMOR_MODAL = _F(AMOR_REDUIT = AMMO"
            << linearDynaModalFreq.getId() << ",)," << endl;
    } else {
        // LD MATR_AMOR nullifies AMOR_MODAL
        comm_file_ofs << "                   MATR_AMOR  = AMORG" << linearDynaModalFreq.getId() << ","
            << endl;
    }
    comm_file_ofs << "                   LIST_FREQ  = LST" << setfill('0') << setw(5)
        << linearDynaModalFreq.getExcitationFrequencies()->getValue()->getId() << "," << endl;
		comm_file_ofs << "                   EXCIT      = (" << endl;
		for (const auto& loadSet : linearDynaModalFreq.getLoadSets()) {
			for (const auto& loading : loadSet->getLoadings()) {
				if (loading->type == Loading::Type::DYNAMIC_EXCITATION) {
					const auto& dynamicExcitation =
							dynamic_pointer_cast<DynamicExcitation>(loading);
					comm_file_ofs << "                                 _F(" << endl;
                    comm_file_ofs << "                                    VECT_ASSE_GENE = VG"
                            << linearDynaModalFreq.getId() << "_"
                            << dynamicExcitation->getId() << "," << endl;
					comm_file_ofs << "                                    FONC_MULT = FCT" << setfill('0')
							<< setw(5) << dynamicExcitation->getFunctionTableB()->getId() << ","
							<< endl;
					comm_file_ofs << "                                    PHAS_DEG = "
							<< dynamicExcitation->getDynaPhase()->get() << ",)," << endl;
                    dynamicExcitation->markAsWritten();
                    loadSet->markAsWritten();
				}
			}
		}
		comm_file_ofs << "                                 )," << endl;
		comm_file_ofs << "                   #SOLVEUR=_F(RENUM='PORD',METHODE='MUMPS',NPREC=8)," << endl; // MUMPS: Error in function orderMinPriority no valid number of stages in multisector (#stages = 2)
		comm_file_ofs << "                   );" << endl << endl;

        comm_file_ofs << "RESU" << linearDynaModalFreq.getId() << " = REST_GENE_PHYS(RESU_GENE = GENE"
                << linearDynaModalFreq.getId() << "," << endl;
        comm_file_ofs << "                       TOUT_ORDRE = 'OUI'," << endl;
        comm_file_ofs << "                       NOM_CHAM = ('DEPL','VITE','ACCE')," << endl;
        comm_file_ofs << "                       );" << endl << endl;
        linearDynaModalFreq.markAsWritten();
		break;
	}
    case Analysis::Type::LINEAR_BUCKLING: {
        auto& linearBuckling = static_cast<LinearBuckling&>(analysis);

        comm_file_ofs << "FSIG" << linearBuckling.getId() << " = CREA_CHAMP(OPERATION='EXTR'," << endl;
        comm_file_ofs << "            TYPE_CHAM='ELGA_SIEF_R'," << endl;
        comm_file_ofs << "            RESULTAT=RESU" << linearBuckling.previousAnalysis->getId() << "," << endl;
        //comm_file_ofs << "            NUME_ORDRE=1," << endl;
        comm_file_ofs << "            NOM_CHAM='SIEF_ELGA',)" << endl << endl;

        writeAssemblage( analysis);

        comm_file_ofs << "RESU" << linearBuckling.getId() << "=CALC_MODES(MATR_RIGI=RIGI" << linearBuckling.getId() << "," << endl;
        comm_file_ofs << "                 MATR_RIGI_GEOM=RIGE" << linearBuckling.getId() << "," << endl;
        comm_file_ofs << "                 TYPE_RESU='MODE_FLAMB'," << endl;
		if (linearBuckling.use_power_iteration) {
            comm_file_ofs << "                       SOLVEUR_MODAL=_F(OPTION_INV='DIRECT')," << endl;
		} else {
            comm_file_ofs << "                       SOLVEUR_MODAL=_F(METHODE='TRI_DIAG')," << endl;
		}
        writeCalcFreq( linearBuckling);
        comm_file_ofs << "             VERI_MODE=_F(STOP_ERREUR='NON',)," << endl;
        comm_file_ofs << "             SOLVEUR=_F(METHODE='MUMPS',)," << endl;
        comm_file_ofs << "       )" << endl << endl;

        comm_file_ofs << "RESU" << linearBuckling.getId() << " = NORM_MODE(reuse=RESU"<< linearBuckling.getId() << ",MODE=RESU" << linearBuckling.getId() << ",NORME='TRAN_ROTA',)" << endl;
        comm_file_ofs << "TBCRT" << linearBuckling.getId() << " = RECU_TABLE(CO=RESU" << linearBuckling.getId() << ",NOM_PARA='CHAR_CRIT')" << endl;
        linearBuckling.markAsWritten();
        break;
    }
	default:
		handleWritingError(
				"Analysis " + Analysis::stringByType.at(analysis.type) + " not (yet) implemented");
	}
	return debut;
}

void AsterWriter::writeNodalDisplacementAssertion(const NodalDisplacementAssertion& nda) {

	bool relativeComparison = abs(nda.value) >= SMALLEST_RELATIVE_COMPARISON;
	comm_file_ofs << "                     CRITERE = " << (relativeComparison ? "'RELATIF'," : "'ABSOLU',") << endl;
	comm_file_ofs << "                     NOEUD='" << Node::MedName(nda.nodePosition) << "'," << endl;
	comm_file_ofs << "                     NOM_CMP    = '" << AsterModel::DofByPosition.at(nda.dof.position) << "'," << endl;
	comm_file_ofs << "                     NOM_CHAM   = 'DEPL'," << endl;
	if (!is_equal(nda.instant, -1)) {
		comm_file_ofs << "                     INST = " << nda.instant << "," << endl;
	} else {
		comm_file_ofs << "                     NUME_ORDRE = 1," << endl;
	}
    comm_file_ofs << "                     REFERENCE = 'SOURCE_EXTERNE'," << endl;
    comm_file_ofs << "                     PRECISION = " << nda.tolerance << "," << endl;
	comm_file_ofs << "                     VALE_REFE = " << nda.value << "," << endl;
	comm_file_ofs << "                     VALE_CALC = " << (is_zero(nda.value) ? 1e-10 : nda.value) << "," << endl;
	comm_file_ofs << "                     TOLE_MACHINE = (" << nda.tolerance << "," << 1e-5 << ")," << endl;

}

void AsterWriter::writeNodalComplexDisplacementAssertion(const NodalComplexDisplacementAssertion& nda) {

    bool relativeComparison = abs(nda.value) >= SMALLEST_RELATIVE_COMPARISON;
    comm_file_ofs << "                     CRITERE = "
            << (relativeComparison ? "'RELATIF'," : "'ABSOLU',") << endl;
    comm_file_ofs << "                     NOEUD='" << Node::MedName(nda.nodePosition) << "'," << endl;
    comm_file_ofs << "                     NOM_CMP = '" << AsterModel::DofByPosition.at(nda.dof.position)
            << "'," << endl;
    comm_file_ofs << "                     NOM_CHAM = 'DEPL'," << endl;
    comm_file_ofs << "                     FREQ = " << nda.frequency << "," << endl;
    comm_file_ofs << "                     VALE_CALC_C = " << nda.value.real() << "+" << nda.value.imag()
            << "j," << endl;
    comm_file_ofs << "                     TOLE_MACHINE = (" << (relativeComparison ? nda.tolerance : 1e-5) << "," << 1e-5 << ")," << endl;
}

void AsterWriter::writeNodalCellVonMisesAssertion( const NodalCellVonMisesAssertion& ncvmisa) {

    bool relativeComparison = abs(ncvmisa.value) >= SMALLEST_RELATIVE_COMPARISON;
    comm_file_ofs << "                     CRITERE = "
            << (relativeComparison ? "'RELATIF'," : "'ABSOLU',") << endl;
    comm_file_ofs << "                     NOEUD='" << Node::MedName(ncvmisa.nodePosition) << "'," << endl;
    comm_file_ofs << "                     GROUP_MA='" << Cell::MedName(ncvmisa.cellPosition) << "'," << endl;
    comm_file_ofs << "                     NOM_CMP = 'VMIS'," << endl;
	comm_file_ofs << "                     NUME_ORDRE = 1," << endl;
    comm_file_ofs << "                     NOM_CHAM = 'SIEQ_ELNO'," << endl;
    comm_file_ofs << "                     VALE_CALC = " << ncvmisa.value << "," << endl;
    comm_file_ofs << "                     TOLE_MACHINE = (" << (relativeComparison ? ncvmisa.tolerance : 1e-5) << "," << 1e-5 << ")," << endl;
}

void AsterWriter::writeFrequencyAssertion(const Analysis& analysis, const FrequencyAssertion& frequencyAssertion) {
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
    comm_file_ofs << "                  _F(RESULTAT=" << resuName << "," << endl;
	comm_file_ofs << "                     CRITERE = " << critere << endl;
    if (isBuckling) {
        comm_file_ofs << "                     PARA = 'CHAR_CRIT'," << endl;
        comm_file_ofs << "                     NUME_MODE = " << analysis.getAssertions().size() - frequencyAssertion.number + 1 << "," << endl;
        comm_file_ofs << "                     VALE_CALC = " << -frequencyAssertion.eigenValue << "," << endl;
    } else {
        comm_file_ofs << "                     PARA = 'FREQ'," << endl;
        comm_file_ofs << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
        comm_file_ofs << "                     VALE_CALC = " << frequencyAssertion.cycles << "," << endl;
    }
	comm_file_ofs << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
    comm_file_ofs << "                     )," << endl;

    if (not isBuckling) {
        comm_file_ofs << "                  _F(RESULTAT=" << resuName << "," << endl;
        comm_file_ofs << "                     CRITERE = " << critere << endl;
        comm_file_ofs << "                     PARA = 'MASS_GENE'," << endl;
        comm_file_ofs << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
        comm_file_ofs << "                     VALE_CALC = " << frequencyAssertion.generalizedMass << "," << endl;
        comm_file_ofs << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
        comm_file_ofs << "                     )," << endl;
        if (not is_equal(frequencyAssertion.generalizedMass, 1.0)) {
            // Do not check generalized stiffness k_g when generalize mass m_g is normalized
            // since it is the same as checking the frequency : (2*pi*f)**2=k_g/m_g
            // but the error would be squared
            comm_file_ofs << "                  _F(RESULTAT=" << resuName << "," << endl;
            comm_file_ofs << "                     CRITERE = " << critere << endl;
            comm_file_ofs << "                     PARA = 'RIGI_GENE'," << endl;
            comm_file_ofs << "                     NUME_MODE = " << frequencyAssertion.number << "," << endl;
            comm_file_ofs << "                     VALE_CALC = " << frequencyAssertion.generalizedStiffness << "," << endl;
            comm_file_ofs << "                     TOLE_MACHINE = " << frequencyAssertion.tolerance << "," << endl;
            comm_file_ofs << "                     )," << endl;
        }
    }

}
}
} //end of namespace vega

