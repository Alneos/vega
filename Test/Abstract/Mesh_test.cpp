/*
 * Copyright (C) Alneos, s. a r. l. (contact@alneos.fr)
 * Released under the GNU General Public License
 */

#define BOOST_TEST_MODULE mesh_test
#include <boost/test/unit_test.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

#include "../../Abstract/MeshComponents.h"
#include "../../Abstract/Mesh.h"

using namespace std;
using namespace vega;

BOOST_AUTO_TEST_CASE( test_node_distance ) {
    Mesh mesh(LogLevel::INFO, "test_node_distance");
    int nodepos1 = mesh.addNode(1, 0.0, 0.0, 0.0);
    int nodepos2 = mesh.addNode(2, 10.0, 0.0, 0.0);
    const Node& node1 = mesh.findNode(nodepos1);
    const Node& node2 = mesh.findNode(nodepos2);
    BOOST_CHECK_EQUAL(node1.distance(node2), 10);
    BOOST_CHECK_EQUAL(node2.distance(node1), 10);
    BOOST_CHECK_EQUAL(boost::geometry::comparable_distance(node1, node2), 100);
    BOOST_CHECK_EQUAL(node1.square_distance(node2), 100);
}

BOOST_AUTO_TEST_CASE( test_mesh_stats ) {
    Mesh mesh(LogLevel::INFO, "test_mesh_stats");
    mesh.addNode(1, 0.0, 0.0, 0.0);
    mesh.addNode(2, 0.0, 10.0, 0.0);
    mesh.addNode(3, 20.0, 10.0, 0.0);
    mesh.addNode(4, 20.0, 0.0, 0.0);
    mesh.addNode(5, 20.0, 0.0, 0.0);
    mesh.addCell(101, CellType::SEG2, {1, 2});
    mesh.addCell(102, CellType::SEG2, {2, 3});
    mesh.addCell(103, CellType::SEG2, {3, 4});
    mesh.addCell(104, CellType::SEG2, {3, 4});
    mesh.addCell(105, CellType::SEG2, {4, 5});
    const auto& stats = mesh.calcStats();
    BOOST_CHECK_EQUAL(stats.minLength, 0);
    BOOST_CHECK_EQUAL(stats.minNonzeroLength, 10);
    BOOST_CHECK_EQUAL(stats.maxLength, 20);
    BOOST_CHECK(stats.quadraticMeanLength > 10);
    BOOST_CHECK(stats.quadraticMeanLength < 20);
    const auto& stats2 = mesh.calcStats();
    BOOST_CHECK_EQUAL(stats2.maxLength, 20);
}

BOOST_AUTO_TEST_CASE( test_faceIds )
{
    Mesh mesh(LogLevel::INFO, "test");
    int cellPosition = mesh.addCell(1, CellType::HEXA8, { 101, 102, 103, 104, 105, 106, 107, 108 });
    const Cell&& hexa = mesh.findCell(cellPosition);
    vector<int> face1NodeIds = hexa.faceids_from_two_nodes(101, 104);
    vector<int> expectedFace1NodeIds = { 101, 102, 103, 104 };
    BOOST_CHECK_EQUAL_COLLECTIONS(face1NodeIds.begin(), face1NodeIds.end(),
                                  expectedFace1NodeIds.begin(), expectedFace1NodeIds.end());
    vector<int> face2NodeIds = hexa.faceids_from_two_nodes(105, 107);
    vector<int> expectedFace2NodeIds = { 105, 108, 107, 106 };
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
