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
 * Object.h
 *
 *  Created on: 3 juin 2014
 *      Author: siavelis
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include "Reference.h"
#include <climits>
#include <string>
#include <sstream>

namespace vega {

/**
 * Base template class for a vega identifiable class
 */
template<class T> class Identifiable {
    static int auto_id;
    int original_id;
    int id;
public:
    static const int NO_ORIGINAL_ID;

    /**
     * Were the Object in the original study ?
     */
    bool isOriginal() const {
        return original_id != NO_ORIGINAL_ID;
    }

    int getId() const {
        return id;
    }

    int bestId() const {
        if (original_id != NO_ORIGINAL_ID) {
            return original_id;
        } else {
            return id;
        }
    }

    int getOriginalId() const {
        return original_id;
    }

    static int lastAutoId() {
        return auto_id;
    }

    void resetId() {
        id = ++auto_id;
        original_id = NO_ORIGINAL_ID;
    }

    Identifiable(int original_id = NO_ORIGINAL_ID) :
            original_id(original_id), id(++auto_id) {
    }

    virtual bool validate() const {
        return true;
    }

    Reference<T> getReference() const {
        int no_id = Reference<T>::NO_ID;
        return Reference<T>(static_cast<const T * const >(this)->type,
                (isOriginal() ? original_id : no_id), id);
    }

    virtual std::map<std::string, std::string> to_map() const {
        // default, to be overriden to add details in to_str by subclasses
        std::map<std::string, std::string> infos;

        if (isOriginal())
            infos["original_id"] = std::to_string(original_id);
        else
            infos["vega_id"] = std::to_string(id);

        return infos;
    }

    virtual ~Identifiable() {
    }

};
template<class T> const int Identifiable<T>::NO_ORIGINAL_ID = INT_MIN;
template<class T> int Identifiable<T>::auto_id = 0;


template<class T>
std::string to_str(const std::shared_ptr<T>& ptr) {
    return to_str(*ptr);
}

template<class T>
std::string to_str(const T& t) {
    std::ostringstream oss;
    std::string type;

    auto it = T::stringByType.find(t.type);
    if (it != T::stringByType.end())
        type = it->second;
    else
        type = "unknown type";

    oss << T::name << "<" << type << ">" << "{";
    bool first = true;
    for (const auto& kv : t.to_map()) {
        if (!first)
            oss << ";";
        else
            first = false;
        oss << kv.first << "=" << kv.second;
    }
    oss << "}";

    return oss.str();
}

template<class T>
int operator==(const Identifiable<T>& left, const Identifiable<T>& right) {
    return left.getReference() == right.getReference();
}

template<class T>
int operator <(const Identifiable<T>& left, const Identifiable<T>& right) {
    return left.getReference() < right.getReference();
}

} /* namespace vega */

namespace std {

template<class T>
struct hash<vega::Identifiable<T>> {
    size_t operator()(const vega::Identifiable<T>& identifiable) const {
        return hash<vega::Reference<T>>()(identifiable.getReference());
    }
};

} /* namespace std */

#endif /* OBJECT_H_ */
