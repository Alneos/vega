/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * This file is part of Vega.
 *
 *   Vega is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Vega is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Vega.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Value.h
 *
 *  Created on: 6 mars 2014
 *      Author: siavelis
 */

#ifndef VALUE_H_
#define VALUE_H_

#include "Object.h"
#include "Reference.h"
#include "Value.h"
#include "Utility.h"

#include <climits>
#include <map>
#include <vector>
#include <memory>

namespace ublas = boost::numeric::ublas;

namespace vega {

class Model;

/**
 * The generic vega::Value class is useful to store information on simple values such as a table or a function
 */
class Value {
public:
    enum Type {
        FLOAT,
        BAND_RANGE,
        STEP_RANGE,
        SPREAD_RANGE,
        FUNCTION_TABLE,
        LIST,
        DYNA_PHASE,
        VECTOR,
        VECTORFUNCTION
    };
protected:
    Value(Value::Type);
public:
    static const std::map<Type, std::string> stringByType;
    const Value::Type type;
public:

    virtual bool isPlaceHolder() const {
        return false;
    }

    virtual void scale(double factor) = 0;
    virtual bool iszero() const = 0;
    virtual bool isfunction() const {
        return false;
    }
};

/**
 * The generic vega::Value class is useful to store information on simple values such as a table or a function
 */
class NamedValue: public Value, public Identifiable<NamedValue> {
public:
    enum ParaName {
        NO_PARA_NAME,
        FREQ,
        INST,
        AMOR,
        PARAX
    };

private:
    friend std::ostream &operator<<(std::ostream&, const NamedValue&);    //output
protected:
    const Model& model;
public:
    static const std::string name;
protected:
    ParaName paraX;
    ParaName paraY;
    NamedValue(const Model&, Type, int original_id = NO_ORIGINAL_ID, ParaName paraX = NO_PARA_NAME,
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

    virtual std::shared_ptr<NamedValue> clone() const =0;
};

/**
 * Hold parameter names of a Value
 * TODO LD: understand why this is not a Reference ??
 */
class ValuePlaceHolder: public NamedValue {
public:
    ValuePlaceHolder(const Model&, Type, int original_id, ParaName paraX, ParaName paraY =
            NO_PARA_NAME);
    bool isPlaceHolder() const {
        return true;
    }
    ;
    std::shared_ptr<NamedValue> clone() const;
    virtual bool iszero() const {
        throw std::logic_error("Should not check placeholders for being zero");
    }
    virtual void scale(double factor) {
        UNUSEDV(factor);
        throw std::logic_error("Should not try to scale placeholders");
    }
};

class ListValue: public NamedValue {
    std::list<double> alist;
public:
    ListValue(const Model&, std::list<double> values, int original_id = NO_ORIGINAL_ID);
    const std::list<double> getList() const;
    std::shared_ptr<NamedValue> clone() const override;
    virtual void scale(double factor) override;
    virtual bool iszero() const override;
};

class FloatValue: public NamedValue {
public:
    FloatValue(const Model&, double number, int original_id = NO_ORIGINAL_ID);
    double number;
    std::shared_ptr<NamedValue> clone() const override;
    void scale(double factor) override;
    bool iszero() const override;
};

class ValueRange: public NamedValue {
protected:
    ValueRange(const Model&, Type, int original_id = NO_ORIGINAL_ID);
};

/**
 * band, searching for root inside a range
 * see Nastran EIGRL
 */
class BandRange: public ValueRange {
public:
    const double start; /**< start of band */
    const int maxsearch; /**< see EIGRL ND */
    const double end; /**< end of band */
public:
    BandRange(const Model& model, double start, int searchcount, double end, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<NamedValue> clone() const override;
    void scale(double factor) override;
    bool iszero() const override;
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
    std::shared_ptr<NamedValue> clone() const override;
    void scale(double factor) override;
    bool iszero() const override;
};

/**
 * spread, +/- the factional amount specified for each value which occurs in the range.
 * see Nastran FREQ4
 */
class SpreadRange: public ValueRange {
public:
    const double start;
    int count;
    double end;
    double spread;
public:
    SpreadRange(const Model& model, double start, int count, double end, double spread, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<NamedValue> clone() const override;
    void scale(double factor) override;
    bool iszero() const override;
};

class Function: public NamedValue {
protected:
    Function(const Model&, Type, int original_id = NO_ORIGINAL_ID);
public:
    bool isfunction() const override {
        return true;
    }
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
    std::shared_ptr<NamedValue> clone() const override;
    bool iszero() const override;
    void scale(double factor) override;
};

class ConstantValue: public NamedValue {
protected:
    double value;
    ConstantValue(const Model&, Type, double value, int original_id = NO_ORIGINAL_ID);
public:
    virtual double get() {
        return value;
    }
    bool iszero() const override {
        return is_zero(value);
    }
    void scale(double factor) override {
        value *= factor;
    }
};

class DynaPhase: public ConstantValue {
public:
    DynaPhase(const Model&, double value, int original_id = NO_ORIGINAL_ID);
    std::shared_ptr<NamedValue> clone() const override {
        return std::make_shared<DynaPhase>(*this);
    }

};

/*
 * Placeholder class, put here all the methods to operate on a vector.
 * @see vega expression.pyx VectorialValue
 */
class VectorialValue final : public Value {//: public TriValue {
	private:
		ublas::vector<double> value;
		VectorialValue(ublas::vector<double>& value);
		friend std::ostream& operator<<(std::ostream& os, const VectorialValue& obj);
    public:
        VectorialValue();
		VectorialValue(double x, double y, double z);
		VectorialValue(std::initializer_list<double> c);
		inline double x() const {
			return value[0];
		}

		inline double y() const {
			return value[1];
		}

		inline double z() const {
			return value[2];
		}

		VectorialValue normalized() const;
		void scale(double factor) override;
		VectorialValue scaled(double factor) const;
		double dot(const VectorialValue &v) const;
		double norm() const;
		bool iszero() const override;
		/**
		 *  Cross product with another VectorialValue. Result is ("this" x "other")
		 **/
		VectorialValue cross(const VectorialValue& other) const;
		/**
		 *  Return a vector orthonormal to "other", computed from this->value
		 **/
		VectorialValue orthonormalized(const VectorialValue& other) const;
		friend const VectorialValue operator+(const VectorialValue&, const VectorialValue&);
		friend const VectorialValue operator-(const VectorialValue&, const VectorialValue&);
		friend const VectorialValue operator*(const double&, const VectorialValue&);
		friend const VectorialValue operator/(const VectorialValue&, const double&);
		VectorialValue& operator=(const VectorialValue&);
		friend bool operator==(const VectorialValue&, const VectorialValue&);
		friend bool operator!=(const VectorialValue&, const VectorialValue&);

        static const VectorialValue O;
		static const VectorialValue X;
		static const VectorialValue Y;
		static const VectorialValue Z;
		static const VectorialValue XYZ[3];
	};

/**
 * Adapter class: it can represent a double value, a reference or an empty value.
 * It can be used in every place where a solver can put a number or a function, list etc.
 */
class ValueOrReference
    final {
        boost::variant<double, Reference<NamedValue>> storage;
    public:
        static const ValueOrReference EMPTY_VALUE;
        ValueOrReference();
        ValueOrReference(double);
        ValueOrReference(const boost::variant<double, Reference<NamedValue>>& value);
        double getValue() const;
        Reference<NamedValue> getReference() const;
        bool isReference() const;
        bool isEmpty() const;
        bool operator==(const ValueOrReference& rhs) const;
        bool operator!=(const ValueOrReference& rhs) const {
            return !(*this == rhs);
        }
        ;
        bool operator<(const ValueOrReference& rhs) const;
    };

std::ostream& operator<<(std::ostream &out, const ValueOrReference& valueOrReference);

class VectorialFunction final : public NamedValue {//: public TriValue {
private:
    Function& _fx;
    Function& _fy;
    Function& _fz;
public:
    VectorialFunction(const Model&, Function& fx, Function& fy, Function& fz, int original_id = NO_ORIGINAL_ID);
    inline Function& x() const { return _fx;};
    inline Function& y() const { return _fy;};
    inline Function& z() const { return _fz;};
    void scale(double factor) override;
    bool iszero() const override;
    std::shared_ptr<NamedValue> clone() const override {
        return std::make_shared<VectorialFunction>(*this);
    }
};

} /* namespace vega */

#endif /* VALUE_H_ */
