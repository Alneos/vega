/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterModel.h
 *
 *  Created on: Aug 30, 2013
 *      Author: devel
 */

#ifndef ASTERMODEL_H_
#define ASTERMODEL_H_

#include "../Abstract/Value.h"
#include "../Abstract/ConfigurationParameters.h"

namespace vega {
class Model;
class ElementSet;

namespace aster {

class AsterModel {
public:
	const vega::Model& model;
	const vega::ConfigurationParameters configuration;
	std::string phenomene;
	AsterModel(const vega::Model& model, const vega::ConfigurationParameters &configuration);
	AsterModel(const AsterModel& that) = delete;
	virtual ~AsterModel();
	const std::string getOutputFileName(std::string extension, bool absolute = true) const;
	const std::string getAsterVersion() const;
	double getMemjeveux() const;
	double getTpmax() const;
	const std::string getModelisations(const std::shared_ptr<ElementSet>) const;
	static const std::map<Function::ParaName, std::string> NomParaByParaName;
	static const std::map<FunctionTable::Interpolation, std::string> InterpolationByInterpolation;
	static const std::map<FunctionTable::Interpolation, std::string> ProlongementByInterpolation;
	static const std::vector<std::string> DofByPosition;
};

}
}
#endif /* ASTERMODEL_H_ */
