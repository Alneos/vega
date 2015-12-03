/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * AsterBuilder.h
 *
 *  Created on: 5 mars 2013
 *      Author: dallolio
 */

#ifndef NASTRANWRITER_H
#define NASTRANWRITER_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include "../Abstract/Model.h"
#include "../Abstract/SolverInterfaces.h"
#include "../Abstract/ConfigurationParameters.h"

namespace vega {
namespace nastran {

class Line {
private:
	friend std::ostream &operator<<(std::ostream &out, const Line& line);
	unsigned int fieldLength = 0;
	unsigned int fieldNum = 0;
	const string keyword = "";
	std::vector<string> fields;
public:
	Line(string _keyword);
	Line& add();
	Line& add(double value);
	Line& add(string value);
	Line& add(int value);
	Line& add(const std::vector<int> values);
	Line& add(const std::vector<double> values);
	Line& add(const DOFS dofs);
	Line& add(const VectorialValue vector);
};

std::ostream &operator<<(std::ostream &out, const Line& line);

class NastranWriterImpl {
public:
	NastranWriterImpl();
	virtual ~NastranWriterImpl();
	string writeModel(const std::shared_ptr<Model> model_ptr, const ConfigurationParameters&);

private:
	string getDatFilename(const shared_ptr<vega::Model>& model, const string& outputPath) const;
	void writeSOL(const shared_ptr<vega::Model>& model, ofstream& out) const;
	void writeCells(const shared_ptr<vega::Model>& model, ofstream& out);
	void writeNodes(const shared_ptr<vega::Model>& model, ofstream& out);
	void writeMaterials(const shared_ptr<vega::Model>& model, ofstream& out);
	void writeConstraints(const shared_ptr<vega::Model>& model, ofstream& out);
	void writeLoadings(const shared_ptr<vega::Model>& model, ofstream& out);
	void writeRuler(ofstream& out);
	void writeElements(const shared_ptr<vega::Model>& model, ofstream& out);
};

}
}
#endif /* NASTRANWRITER_H */
