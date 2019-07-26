/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 *
 * NastranWriter.h
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
    static int newlineCounter;
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
	Line& add(const char*);
	Line& add(int);
	Line& add(const std::vector<int>);
	Line& add(const std::vector<double>);
	Line& add(const DOFS);
	Line& add(const VectorialValue);
};

std::ostream &operator<<(std::ostream &out, const Line& line);

class NastranWriter final : public Writer {
    enum class Dialect {
        COSMIC95,
        MODERN,
    };
public:
    NastranWriter() = default;
	NastranWriter(const NastranWriter& that) = delete;
	std::string writeModel(Model&, const ConfigurationParameters&) override;
    const std::string toString() const override;
private:
    static const std::unordered_map<CellType::Code, std::vector<int>, EnumClassHash> med2nastranNodeConnectByCellType; /**< see NastranParser.h */
    Dialect dialect;
	std::string getNasFilename(const Model& model, const std::string& outputPath) const;
	void writeSOL(const Model& model, std::ofstream& out) const;
	void writeCells(const Model& model, std::ofstream& out) const;
	void writeNodes(const Model& model, std::ofstream& out) const;
	void writeMaterials(const Model& model, std::ofstream& out) const;
	void writeConstraints(const Model& model, std::ofstream& out) const;
	void writeLoadings(const Model& model, std::ofstream& out) const;
	void writeRuler(std::ofstream& out) const;
	void writeElements(const Model& model, std::ofstream& out) const;
};

}
}
#endif /* NASTRANWRITER_H */
