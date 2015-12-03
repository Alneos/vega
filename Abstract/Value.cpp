/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
 *
 * Value.cpp
 *
 *  Created on: 6 mars 2014
 *      Author: siavelis
 */

#include "Value.h"
#include "Model.h"

using namespace std;

namespace vega {

Value::Value(const Model& model, Value::Type type, int original_id, ParaName paraX, ParaName paraY) :
		Identifiable(original_id), model(model), type(type), paraX(paraX), paraY(paraY) {
}

const string Value::name = "Value";

const map<Value::Type, string> Value::stringByType = { { STEP_RANGE, "STEP_RANGE" }, {
		FUNCTION_TABLE, "FUNCTION_TABLE" }, { DYNA_PHASE, "DYNA_PHASE" }, };

ostream &operator<<(ostream &out, const Value& value) {
	out << to_str(value);
	return out;
}

ValuePlaceHolder::ValuePlaceHolder(const Model& model, Type type, int original_id, ParaName paraX,
		ParaName paraY) :
		Value(model, type, original_id, paraX, paraY) {
}

shared_ptr<Value> ValuePlaceHolder::clone() const {
	return shared_ptr<Value>(new ValuePlaceHolder(*this));
}

ValueRange::ValueRange(const Model& model, Type type, int original_id) :
		Value(model, type, original_id) {
}

shared_ptr<Value> ValueRange::clone() const {
	return shared_ptr<Value>(new ValueRange(*this));
}

StepRange::StepRange(const Model& model, double start, double step, double end, int original_id) :
		ValueRange(model, STEP_RANGE, original_id), start(start), step(step), end(end) {
	assert(step > 0);
	count = int((end - start) / step);
}

StepRange::StepRange(const Model& model, double start, int count, double end, int original_id) :
		ValueRange(model, STEP_RANGE, original_id), start(start), count(count), end(end) {
	assert(count > 0);
	step = (end - start) / count;
}
StepRange::StepRange(const Model& model, double start, double step, int count, int original_id) :
		ValueRange(model, STEP_RANGE, original_id), start(start), step(step), count(count) {
	end = start + step * count;
}

shared_ptr<Value> StepRange::clone() const {
	return shared_ptr<Value>(new StepRange(*this));
}

Function::Function(const Model& model, Type type, int original_id) :
		Value(model, type, original_id) {
}

shared_ptr<Value> Function::clone() const {
	return shared_ptr<Value>(new Function(*this));
}

FunctionTable::FunctionTable(const Model& model, Interpolation parameter, Interpolation value,
		Interpolation left, Interpolation right, int original_id) :
		Function(model, FUNCTION_TABLE, original_id), parameter(parameter), value(value), left(
				left), right(right) {
}

void FunctionTable::setXY(const double X, const double Y) {
	valuesXY.push_back(pair<double, double>(X, Y));
}

const vector<pair<double, double>>::const_iterator FunctionTable::getBeginValuesXY() const {
	return valuesXY.begin();
}

const vector<pair<double, double>>::const_iterator FunctionTable::getEndValuesXY() const {
	return valuesXY.end();
}

shared_ptr<Value> FunctionTable::clone() const {
	return shared_ptr<Value>(new FunctionTable(*this));
}

ConstantValue::ConstantValue(const Model&, Type type, double value, int original_id) :
		Value(model, type, original_id), value(value) {
}

DynaPhase::DynaPhase(const Model&, double value, int original_id) :
		ConstantValue(model, DYNA_PHASE, value, original_id) {
}

} /* namespace vega */
