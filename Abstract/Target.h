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
    Model& model;

public:
    const Type type;
    static const std::string name;
    static const std::map<Type, std::string> stringByType;
protected:
    Target(Model&, Target::Type, int original_id = NO_ORIGINAL_ID);
    Target(const Target& that) = delete;
public:
	virtual bool isNodeTarget() const {
		return false;
	}
	virtual bool isCellTarget() const {
		return false;
	}
    const std::string to_str() const;
};

class NodeTarget: public Target {
protected:
    NodeTarget(Model&, Target::Type, int original_id = NO_ORIGINAL_ID);
public:
	bool isNodeTarget() const override final {
		return true;
	}
};

/**
 * Defines a could of node ids to be used in contact problems, see Nastran BCGRID
 */
class BoundaryNodeCloud: public NodeTarget {
public:
    BoundaryNodeCloud(Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
};

/**
 * Defines a line by couples of node ids, see Nastran BSSEG
 */
class BoundaryNodeLine: public NodeTarget {
public:
    BoundaryNodeLine(Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
};

/**
 * Defines a surface by four node ids (last one may be zero in triangles), see Nastran BSSEG
 */
class BoundaryNodeSurface: public NodeTarget {
public:
    BoundaryNodeSurface(Model& model, std::list<int> nodeids, int original_id =
            NO_ORIGINAL_ID);
    std::list<int> nodeids;
};

class CellTarget: public Target {
protected:
    CellTarget(Model&, Target::Type, int original_id = NO_ORIGINAL_ID);
public:
	bool isCellTarget() const override final {
		return true;
	}
	virtual void createSkin() = 0;
};

/**
 * Defines a surface by element ids, see Nastran BSURF
 */
class BoundarySurface: public CellTarget, public CellContainer {
public:
    BoundarySurface(Model& model, int original_id =
            NO_ORIGINAL_ID);
    void createSkin() override { /* Already a skin */ };
};

/**
 * Defines a contact body, see Nastran BCBODY
 */
class ContactBody: public Target {
public:
    ContactBody(Model& model, Reference<Target> boundary, int original_id =
            NO_ORIGINAL_ID);
    Reference<Target> boundary;
};

/**
 * Defines a surface by an element and a couple of node ids, see Optistruct SURF
 */
class BoundaryElementFace: public CellTarget {
public:
    class ElementFaceByTwoNodes final {
    public:
        ElementFaceByTwoNodes(int cellId, int nodeid1 = Globals::UNAVAILABLE_INT, int nodeid2 = Globals::UNAVAILABLE_INT, bool swapNormal = false);
        const int cellId;
        const int nodeid1;
        const int nodeid2;
        const bool swapNormal;
    };
    BoundaryElementFace(Model& model, std::list<ElementFaceByTwoNodes> faceInfos, int original_id =
            NO_ORIGINAL_ID);
    std::shared_ptr<CellGroup> surfaceCellGroup;
    std::shared_ptr<CellGroup> elementCellGroup;
    std::list<ElementFaceByTwoNodes> faceInfos;
    void createSkin() override;
};

} /* namespace vega */

#endif /* TARGET_H_ */
