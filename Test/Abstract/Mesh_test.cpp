/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */

#define BOOST_TEST_MODULE mesh_test
#include <boost/test/unit_test.hpp>
#include "../../Abstract/MeshComponents.h"
#include "../../Abstract/Mesh.h"

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_faceIds )
{
    vector<int> nodeIds = { 101, 102, 103, 104, 105, 106, 107, 108 };
    Mesh mesh(LogLevel::INFO, "test");
    int cellPosition = mesh.addCell(1, CellType::HEXA8, nodeIds);
    const Cell&& hexa = mesh.findCell(cellPosition);
    vector<int> face1NodeIds = hexa.faceids_from_two_nodes(101, 104);
    vector<int> expectedFace1NodeIds = { 101, 102, 103, 104 };
    BOOST_CHECK_EQUAL_COLLECTIONS(face1NodeIds.begin(), face1NodeIds.end(),
                                  expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
    vector<int> face2NodeIds = hexa.faceids_from_two_nodes(105, 107);
    vector<int> expectedFace2NodeIds = { 105, 106, 107, 108 };
    BOOST_CHECK_EQUAL_COLLECTIONS(face2NodeIds.begin(), face2NodeIds.end(),
                                  expectedFace2NodeIds.begin(), expectedFace2NodeIds.end());
}

BOOST_AUTO_TEST_CASE( test_NodeGroup )
{
    Mesh mesh(LogLevel::INFO, "test");
    vector<int> nodeIds = { 101, 102, 103, 104 };
    double coords[12] = { 1.0, 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
    for (int i = 0; i < 12; i += 3)
    {
        mesh.addNode(nodeIds[i / 3], coords[i], coords[i + 1], coords[i + 2]);
    }
    shared_ptr<NodeGroup> nodes = mesh.findOrCreateNodeGroup("test", 5);
    std::for_each(nodeIds.begin(), nodeIds.end(), [&nodes](int &nodeId)
    {
        nodes->addNodeId(nodeId);
    });
    mesh.finish();
    //find the group by name
    shared_ptr<NodeGroup> testGroup = dynamic_pointer_cast<NodeGroup>(mesh.findGroup("test"));
    BOOST_ASSERT_MSG(testGroup != nullptr, "Group found by name");
    BOOST_CHECK_EQUAL(testGroup->nodePositions().size(), static_cast<size_t>(4));
    const set<int> nodeIds1 = testGroup->getNodeIds();
    for (int originalId : nodeIds)
    {
        string message = string("Node id ") + to_string(originalId) + " not found";
        BOOST_CHECK_MESSAGE(nodeIds1.find(originalId) != nodeIds1.end(), message.c_str());
    }
    //find the group by id
    shared_ptr<NodeGroup> groupById = dynamic_pointer_cast<NodeGroup>(mesh.findGroup(5));
    BOOST_ASSERT_MSG(groupById != nullptr, "Group found by id");
}

BOOST_AUTO_TEST_CASE( test_node_iterator )
{
    Mesh mesh(LogLevel::INFO, "test");
    double coords[12] = { 1.0, 250., 0., 433., 250., 0., 0., -500., 0., 0., 0., 1000. };
    int j = 1;
    for (int i = 0; i < 12; i += 3)
    {
        mesh.addNode(j, coords[i], coords[i + 1], coords[i + 2]);
        j++;
    }

    int i = 0;
    NodeIterator nodeIterator = mesh.nodes.begin();
    cout << "NODE" << *nodeIterator << endl;
    //(*nodeIterator).buildGlobalXYZ();
    Node node = *nodeIterator;
    BOOST_CHECK_CLOSE(1.0, node.x, Globals::DOUBLE_COMPARE_TOLERANCE);
    for (i = 0; nodeIterator.hasNext(); i++)
    {
        Node node2 = nodeIterator.next();
        cout << node2 << endl;
    }
    BOOST_CHECK_EQUAL(mesh.countNodes(), i);
    BOOST_CHECK(!mesh.nodes.end().hasNext());
    cout << "bef finish" << endl;
//FINISH----------
    mesh.finish();
    BOOST_TEST_CHECKPOINT("after finish");

    BOOST_CHECK(!mesh.nodes.end().hasNext());

    i = 0;
    for (const auto& node2 : mesh.nodes) {
        i++;
        cout << node2 << endl;
    }
    BOOST_CHECK_EQUAL(4, i);

    nodeIterator = mesh.nodes.begin();
    for (i = 0; nodeIterator.hasNext(); i++)
    {
        const Node node2 = nodeIterator.next();
        cout << node2 << endl;
    }
    BOOST_CHECK_EQUAL(mesh.countNodes(), i);
}
