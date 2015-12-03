/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * SystusModel.h
 *
 *  Created on: Oct 2, 2013
 *      Author: devel
 */

#ifndef SYSTUSMODEL_H_
#define SYSTUSMODEL_H_

#include "../Abstract/Model.h"
#include "../Abstract/ConfigurationParameters.h"

namespace vega {

class SystusModel {
public:
	SystusModel(const vega::Model* model, const vega::ConfigurationParameters &configuration);
	virtual ~SystusModel();
	const vega::Model* model;
	const vega::ConfigurationParameters configuration;
	/*string phenomene;*/
	/*const string memjeveux;*/
	const string getName() const;
	const string getOutputFileName(string extension) const;
	/*const string getSystusVersion() const;*/
};

}

#endif /* SYSTUSMODEL_H_ */
