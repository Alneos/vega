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
 * Reference.h
 *
 *  Created on: 15 mai 2014
 *      Author: siavelis
 */

#ifndef REFERENCE_H_
#define REFERENCE_H_

#include <memory>
#include <climits>
#include <boost/functional/hash.hpp>
#include <fstream>
#include <ostream>
#include <algorithm>

namespace vega {

class InputContext {
public:
    InputContext(int lineNumber, std::string fileName, std::string line) : lineNumber{lineNumber}, fileName{fileName}, line{line} {
    };
    InputContext() = default;
    InputContext(const InputContext&) = default;
    InputContext& operator=(const InputContext&) = default;
    int lineNumber = -1;
    std::string fileName = "";
    std::string line = "";
};

/**
 * An object that need to reference to a vega class can use this template to create a reference to an object of that class
 */
template<class T>
class Reference final {
    InputContext inputContext;
public:
    template<class T2> friend std::string to_str(const Reference<T2>&);
    template<class T2> friend std::ostream& operator<<(std::ostream&, const Reference<T2>&);
    static constexpr int NO_ID = INT_MIN;
    // TODO : make type, original_id and id private
    typename T::Type type;
    int original_id;
    int id;
    Reference(const typename T::Type, const int original_id = NO_ID, const int id = NO_ID) noexcept;
    Reference(const T&) noexcept;
    Reference(const std::shared_ptr<T>&) noexcept;
    bool has_original_id() const noexcept {
        return original_id != NO_ID;
    }
    bool has_id() const noexcept {
        return id != NO_ID;
    }
    /**
     * Add some context about the input where it has been found
     */
    void setInputContext(const InputContext& context) noexcept {
        inputContext = context;
    }

    /**
     * Get the context about the input where it has been found
     */
    InputContext getInputContext() const noexcept {
        return inputContext;
    }
};

template<class T>
Reference<T>::Reference(const typename T::Type type, const int original_id, const int id) noexcept :
        type(type), original_id(original_id), id(id) {
}

template<class T>
Reference<T>::Reference(const T& t) noexcept :
        inputContext{t.getInputContext()}, type(t.type), original_id(t.getOriginalId()), id(t.getId()) {
}

template<class T>
Reference<T>::Reference(const std::shared_ptr<T>& ptr) noexcept :
        inputContext{ptr->getInputContext()}, type(ptr->type), original_id(ptr->getOriginalId()), id(ptr->getId()) {
}

template<class T>
bool operator==(const Reference<T>& left, const Reference<T>& right) noexcept {
    if (left.type != right.type) {
        return false;
    }
    if (left.original_id == T::NO_ORIGINAL_ID && right.original_id == T::NO_ORIGINAL_ID) {
        return (left.id == right.id);
    }
    return left.original_id == right.original_id;
}

template<class T>
bool operator!=(const Reference<T>& left, const Reference<T>& right) noexcept {
    return not (left == right);
}

template<class T>
bool operator <(const Reference<T>& left, const Reference<T>& right) noexcept {
    if (left.type != right.type) {
        return left.type < right.type; // first ordering : type
    } else if (left.original_id != T::NO_ORIGINAL_ID && right.original_id != T::NO_ORIGINAL_ID) {
        return left.original_id < right.original_id; // second ordering : original ids
    } else if (left.original_id != T::NO_ORIGINAL_ID || right.original_id != T::NO_ORIGINAL_ID) {
        return left.original_id != T::NO_ORIGINAL_ID; // third ordering : original_id wins over autoid
    } else {
        return left.id < right.id; // last ordering : compare autoids
    }
}

template<class T>
std::string to_str(const Reference<T>& reference) {
    std::ostringstream oss;
    std::string id, type;

    if (reference.has_original_id())
        id = "original_id=" + std::to_string(reference.original_id);
    else
        id = "vega_id=" + std::to_string(reference.id);

    auto it = T::stringByType.find(reference.type);
    if (it != T::stringByType.end())
        type = "type=" + T::name + "<" + it->second + ">";
    else
        type = "type " + T::name + "<" + std::to_string(static_cast<int>(reference.type)) + "> not yet mapped in stringByType";

    oss << "Reference[" << type << "; " << id;
    if (reference.inputContext.lineNumber >= 1) {
        auto contextLine = reference.inputContext.line;
        std::replace( contextLine.begin(), contextLine.end(), '\n', '|');
        oss << ";input " << reference.inputContext.lineNumber << " " << contextLine;
    }
    oss << "]";

    return oss.str();
}

template<class T>
std::ostream &operator<<(std::ostream &out, const Reference<T>& reference) {
	out << to_str(reference);
	return out;
}

}/* namespace vega */

namespace std {

template<class T>
struct hash<vega::Reference<T>> {
    size_t operator()(const vega::Reference<T>& reference) const noexcept {
        size_t seed = 0;
        // LD : hash should be different between, for example, Reference<LoadSet> and Reference<ConstraintSet>
        // but LoadSet::Type and ConstraintSet::Type enums both begin with 0 value, so I use the string value instead.
        boost::hash_combine(seed,hash<string>()(T::stringByType.find(reference.type)->second));
        boost::hash_combine(seed,hash<int>()(reference.original_id));
        boost::hash_combine(seed,hash<int>()(reference.id));
        return seed;
    }
};

} /* namespace std */

#endif /* REFERENCE_H_ */
