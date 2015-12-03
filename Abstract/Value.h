/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Value.h
 *
 *  Created on: 6 mars 2014
 *      Author: siavelis
 */

#ifndef VALUE_H_
#define VALUE_H_

#include "Object.h"

#include <climits>
#include <map>
#include <vector>
#include <memory>

using namespace std;

namespace vega {

class Model;

/**
 * The generic vega::Value class is useful to store information on simple values such as a table or a function
 */
class Value: public Identifiable<Value> {
private:
	friend ostream &operator<<(ostream&, const Value&);    //output
public:
	enum Type {
		STEP_RANGE,
		FUNCTION_TABLE,
		DYNA_PHASE
	};
	enum ParaName {
		NO_PARA_NAME,
		FREQ,
		INST,
		AMOR
	};
	protected:
	const Model& model;
	public:
	const Type type;
	static const string name;
	static const map<Type, string> stringByType;
	protected:
	ParaName paraX;
	ParaName paraY;
	Value(const Model&, Type, int original_id = NO_ORIGINAL_ID, ParaName paraX = NO_PARA_NAME,
			ParaName paraY = NO_PARA_NAME);
	public:
	void setParaX(ParaName para) {
		paraX = para;
	}

	void setParaY(ParaName para) {
		paraY = para;
	}

	bool hasParaX() const {
		return paraX != NO_PARA_NAME;
	}
	bool hasParaY() const {
		return paraY != NO_PARA_NAME;
	}

	ParaName getParaX() const {
		return paraX;
	}

	ParaName getParaY() const {
		return paraY;
	}

	virtual bool isPlaceHolder() const {
		return false;
	}

	virtual std::shared_ptr<Value> clone() const =0;
};

/**
 * Hold parameter names of a Value
 */
class ValuePlaceHolder: public Value {
public:
	ValuePlaceHolder(const Model&, Type, int original_id, ParaName paraX, ParaName paraY =
			NO_PARA_NAME);
	bool isPlaceHolder() const {
		return true;
	}
	;
	std::shared_ptr<Value> clone() const;

};

class ValueRange: public Value {
protected:
	ValueRange(const Model&, Type, int original_id = NO_ORIGINAL_ID);
	public:
	std::shared_ptr<Value> clone() const;
};

class StepRange: public ValueRange {
public:
	const double start;
	double step;
	int count;
	double end;
	public:
	StepRange(const Model& model, double start, double step, double end, int original_id =
			NO_ORIGINAL_ID);
	StepRange(const Model& model, double start, int count, double end, int original_id =
			NO_ORIGINAL_ID);
	StepRange(const Model& model, double start, double step, int count, int original_id =
			NO_ORIGINAL_ID);
	std::shared_ptr<Value> clone() const;
};

class Function: public Value {
protected:
	Function(const Model&, Type, int original_id = NO_ORIGINAL_ID);
	public:
	std::shared_ptr<Value> clone() const;
};

class FunctionTable: public Function {
protected:
	std::vector<std::pair<double, double> > valuesXY;
	public:
	enum Interpolation {
		LINEAR,
		LOGARITHMIC,
		CONSTANT,
		NONE
	};

	const Interpolation parameter;
	const Interpolation value;
	const Interpolation left;
	const Interpolation right;

public:
	FunctionTable(const Model&, Interpolation parameter = LINEAR, Interpolation value = LINEAR,
			Interpolation left = NONE, Interpolation right = NONE,
			int original_id = NO_ORIGINAL_ID);
	void setXY(const double X, const double Y);
	const std::vector<std::pair<double, double> >::const_iterator getBeginValuesXY() const;
	const std::vector<std::pair<double, double> >::const_iterator getEndValuesXY() const;
	std::shared_ptr<Value> clone() const;
};

class ConstantValue: public Value {
protected:
	double value;
	ConstantValue(const Model&, Type, double value, int original_id = NO_ORIGINAL_ID);
	public:
	virtual double get() {
		return value;
	}
	std::shared_ptr<Value> clone() const {
		return std::shared_ptr<Value>(new ConstantValue(*this));
	}
};

class DynaPhase: public ConstantValue {
public:
	DynaPhase(const Model&, double value, int original_id = NO_ORIGINAL_ID);
	std::shared_ptr<Value> clone() const {
		return std::shared_ptr<Value>(new DynaPhase(*this));
	}
};

} /* namespace vega */

#endif /* VALUE_H_ */
