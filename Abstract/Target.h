#ifndef TARGET_H_
#define TARGET_H_

#include <climits>
#include <memory>
#include <set>
#include <complex>
#include "Value.h"
#include "Object.h"
#include "Reference.h"
#include "Dof.h"

namespace vega {

class Model;

class Target: public Identifiable<Target> {
private:
    friend std::ostream &operator<<(std::ostream &out, const Target&);    //output
public:
    enum class Type {
        BOUNDARY_NODELINE,
        BOUNDARY_NODESURFACE,
    };
protected:
    const Model & model;

public:
    const Type type;
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
protected:
    Target(const Model&, Target::Type, int original_id = NO_ORIGINAL_ID);
public:
    virtual std::shared_ptr<Target> clone() const=0;
};

/**
 * Defines a line by couples of node ids, see Nastran BSSEG
 */
class BoundaryNodeLine: public Target {
public:
    BoundaryNodeLine(const Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
    std::shared_ptr<Target> clone() const;
};

/**
 * Defines a surface by four node ids (last one may be zero), see Nastran BSSURF
 */
class BoundaryNodeSurface: public Target {
public:
    BoundaryNodeSurface(const Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
    std::shared_ptr<Target> clone() const;
};

} /* namespace vega */

#endif /* TARGET_H_ */
