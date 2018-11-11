#ifndef TARGET_H_
#define TARGET_H_

#include <climits>
#include <memory>
#include <set>
#include <complex>
#include "Value.h"
#include "Object.h"
#include "Reference.h"
#include "MeshComponents.h"
#include "Dof.h"

namespace vega {

class Model;

class Target: public Identifiable<Target> {
private:
    friend std::ostream &operator<<(std::ostream &out, const Target&);    //output
public:
    enum class Type {
        BOUNDARY_NODECLOUD,
        BOUNDARY_NODELINE,
        BOUNDARY_NODESURFACE,
        BOUNDARY_SURFACE,
        CONTACT_BODY,
        BOUNDARY_ELEMENTFACE,
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
    const std::string to_str() const;
};

/**
 * Defines a could of node ids to be used in contact problems, see Nastran BCGRID
 */
class BoundaryNodeCloud: public Target {
public:
    BoundaryNodeCloud(const Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
    std::shared_ptr<Target> clone() const;
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
 * Defines a surface by four node ids (last one may be zero in triangles), see Nastran BSSEG
 */
class BoundaryNodeSurface: public Target {
public:
    BoundaryNodeSurface(const Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
    std::shared_ptr<Target> clone() const;
};

/**
 * Defines a surface by element ids, see Nastran BSURF
 */
class BoundarySurface: public Target {
public:
    BoundarySurface(const Model& model, std::shared_ptr<CellGroup> cellGroup, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<CellGroup> cellGroup;
    std::shared_ptr<Target> clone() const;
};

/**
 * Defines a contact body, see Nastran BCBODY
 */
class ContactBody: public Target {
public:
    ContactBody(const Model& model, Reference<Target> boundary, int original_id =
            NO_ORIGINAL_ID);
    Reference<Target> boundary;
    std::shared_ptr<Target> clone() const;
};

/**
 * Defines a surface by an element and a couple of node ids, see Optistruct SURF
 */
class BoundaryElementFace: public Target {
public:
    class ElementFaceByTwoNodes final {
    public:
        ElementFaceByTwoNodes(int cellId, int nodeid1 = Globals::UNAVAILABLE_INT, int nodeid2 = Globals::UNAVAILABLE_INT, bool swapNormal = false);
        const int cellId;
        const int nodeid1;
        const int nodeid2;
        const bool swapNormal;
    };
    BoundaryElementFace(const Model& model, std::list<ElementFaceByTwoNodes> faceInfos, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<CellGroup> cellGroup;
    std::list<ElementFaceByTwoNodes> faceInfos;
    std::shared_ptr<Target> clone() const;
};

} /* namespace vega */

#endif /* TARGET_H_ */
