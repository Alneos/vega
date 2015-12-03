/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr) 
 * Unauthorized copying of this file, via any medium is strictly prohibited. 
 * Proprietary and confidential.
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

	virtual ~Identifiable() {
	}

};
template<class T> const int Identifiable<T>::NO_ORIGINAL_ID = INT_MIN;
template<class T> int Identifiable<T>::auto_id = 0;

template<class T>
std::string to_str(const T& t) {
	std::ostringstream oss;
	std::string id, type;

	if (t.isOriginal())
		id = "original_id=" + std::to_string(t.getOriginalId());
	else
		id = "vega_id=" + std::to_string(t.getId());

	auto it = T::stringByType.find(t.type);
	if (it != T::stringByType.end())
		type = "type=" + it->second;
	else
		type = "unknown type";

	oss << T::name << "{" << type << "; " << id << "}";

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
