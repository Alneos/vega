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
	const std::string keyword = "";
	std::vector<std::string> fields;
public:
	Line(std::string);
	Line& add();
	Line& add(double);
	Line& add(std::string);
	Line& add(int);
	Line& add(const std::vector<int>);
	Line& add(const std::vector<double>);
	Line& add(const DOFS);
	Line& add(const VectorialValue);
};

std::ostream &operator<<(std::ostream &out, const Line& line);

class NastranWriter final : public Writer {
public:
	NastranWriter();
	std::string writeModel(const std::shared_ptr<Model> model_ptr, const ConfigurationParameters&) override;
    const std::string toString() const override;
private:
	std::string getDatFilename(const std::shared_ptr<vega::Model>& model, const std::string& outputPath) const;
	void writeSOL(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeCells(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeNodes(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeMaterials(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeConstraints(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeLoadings(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
	void writeRuler(std::ofstream& out) const;
	void writeElements(const std::shared_ptr<vega::Model>& model, std::ofstream& out) const;
};

}
}
#endif /* NASTRANWRITER_H */
