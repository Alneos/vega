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
    enum class Type {
        KEYWORD,
        SCALAR,
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

    //virtual const std::string str() const = 0;
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
 * Placeholder for special values
 */
class KeywordValue: public Value {
    public:
    enum class Keyword {
        RIGID //< The keyword RIGID may be used in place of a stiffness value. When RIGID is defined, a very high relative stiffness (relative to the surrounding structure) is selected for that degree-of-freedom simulating a rigid connection.
    };

private:
    friend std::ostream &operator<<(std::ostream&, const KeywordValue&);    //output
protected:
    const Model& model;
public:
    static const std::map<Keyword, std::string> stringByType;
protected:
    KeywordValue(const Model&, Keyword keyword);
public:
    const Keyword keyword;
    const std::string str() const;// override;
    virtual std::shared_ptr<KeywordValue> clone() const =0;
};

/**
 * The generic vega::Value class is useful to store information on simple values such as a table or a function
 */
class NamedValue: public Value, public Identifiable<NamedValue> {
private:
    friend std::ostream &operator<<(std::ostream&, const NamedValue&);    //output
protected:
    const Model& model;
public:
    static const std::string name;
protected:
    NamedValue(const Model&, Type, int original_id = NO_ORIGINAL_ID);
public:
    virtual std::shared_ptr<NamedValue> clone() const =0;
};

class ListValueBase: public NamedValue {
public:
    ListValueBase(const Model&, int original_id = NO_ORIGINAL_ID):
        NamedValue(model, Value::Type::LIST, original_id) {
    }
    virtual bool isintegral() const = 0;
};


template<class T> class ListValue: public ListValueBase {
    std::list<T> alist;
public:
    ListValue(const Model&, std::list<T> values, int original_id = NO_ORIGINAL_ID):
        ListValueBase(model, original_id), alist(values) {
    }
    const std::list<T> getList() const {
        return alist;
    }
    bool empty() const {
        return alist.size() == 0;
    }
    std::shared_ptr<NamedValue> clone() const override {
        return std::make_shared<ListValue<T>>(*this);
    }
    void scale(double factor) override {
        std::transform(alist.begin(), alist.end(), alist.begin(), [factor](T d) -> T { return static_cast<T>(d * factor); });
    }
    bool iszero() const override {
        return alist.empty();
    }
    bool isintegral() const override {
        return std::is_integral<T>::value;
    }
};

template<class T> class ScalarValue: public NamedValue {
public:
    ScalarValue(const Model&, T number, int original_id = NO_ORIGINAL_ID) :
        NamedValue(model, Value::Type::SCALAR, original_id), number(number) {
    }
    T number;
    std::shared_ptr<NamedValue> clone() const override{
        return std::make_shared<ScalarValue<T>>(*this);
    }
    void scale(T factor) override {
        number *= factor;
    }
    bool iszero() const override {
        return is_zero(number);
    }
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
public:
    enum class ParaName {
        NO_PARA_NAME,
        FREQ,
        STRESS,
        STRAIN,
        INST,
        AMOR,
        PARAX
    };
protected:
    ParaName paraX;
    ParaName paraY;
    Function(const Model&, Type, int original_id = NO_ORIGINAL_ID, ParaName paraX = ParaName::NO_PARA_NAME, ParaName paraY =
            ParaName::NO_PARA_NAME);
public:
    void setParaX(ParaName para) {
        paraX = para;
    }

    void setParaY(ParaName para) {
        paraY = para;
    }

    bool hasParaX() const {
        return paraX != ParaName::NO_PARA_NAME;
    }
    bool hasParaY() const {
        return paraY != ParaName::NO_PARA_NAME;
    }

    ParaName getParaX() const {
        return paraX;
    }

    ParaName getParaY() const {
        return paraY;
    }
    bool isfunction() const override {
        return true;
    }
};

/**
 * Hold parameter names of a Function (when the Function itself has not been defined yet)
 */
class FunctionPlaceHolder: public Function {
public:
    FunctionPlaceHolder(const Model&, Type, int original_id, ParaName paraX, ParaName paraY =
            ParaName::NO_PARA_NAME);
    bool isPlaceHolder() const {
        return true;
    };
    std::shared_ptr<NamedValue> clone() const;
    virtual bool iszero() const {
        throw std::logic_error("Should not check placeholders for being zero");
    }
    virtual void scale(double factor) {
        UNUSEDV(factor);
        throw std::logic_error("Should not try to scale placeholders");
    }
};

class FunctionTable: public Function {
protected:
    std::vector<std::pair<double, double> > valuesXY;
    public:
    enum class Interpolation {
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
    FunctionTable(const Model&, Interpolation parameter = Interpolation::LINEAR, Interpolation value = Interpolation::LINEAR,
            Interpolation left = Interpolation::NONE, Interpolation right = Interpolation::NONE,
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
