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

BOOST_AUTO_TEST_CASE( test_NodeGroup2Families )
{
    Mesh mesh(LogLevel::INFO, "test");
    vector<shared_ptr<NodeGroup>> nodeGroups;
    shared_ptr<NodeGroup> gn1 = mesh.findOrCreateNodeGroup("GN1");
    gn1->addNodeByPosition(0);
    gn1->addNodeByPosition(3);
    gn1->addNodeByPosition(4);
    nodeGroups.push_back(gn1);
    shared_ptr<NodeGroup> gn2 = mesh.findOrCreateNodeGroup("GN2");
    gn2->addNodeByPosition(0);
    gn2->addNodeByPosition(1);
    nodeGroups.push_back(gn2);
    NodeGroup2Families ng(5, nodeGroups);
    int expected[] = { 2, 3, 0, 1, 1, 0 };
    vector<int> result = ng.getFamilyOnNodes();
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected, expected + 5);
    vector<Family> families = ng.getFamilies();
    BOOST_CHECK_EQUAL(static_cast<size_t>(3), families.size());
    bool famGN1_GN2_found = false;
    for (Family fam : families)
    {
        famGN1_GN2_found |= fam.name == "GN1_GN2";
    }
    BOOST_CHECK(famGN1_GN2_found);
}

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
    node.buildGlobalXYZ();
    BOOST_CHECK_CLOSE(1.0, node.x, Globals::DOUBLE_COMPARE_TOLERANCE);
    for (i = 0; nodeIterator.hasNext(); i++)
    {
        Node node = nodeIterator.next();
        node.buildGlobalXYZ();
        cout << node << endl;
    }
    BOOST_CHECK_EQUAL(mesh.countNodes(), i);
    BOOST_CHECK(!mesh.nodes.end().hasNext());
    cout << "bef finish" << endl;
//FINISH----------
    mesh.finish();
    BOOST_TEST_CHECKPOINT("after finish");

    BOOST_CHECK(!mesh.nodes.end().hasNext());

    i = 0;
    for (auto& node : mesh.nodes) {
        i++;
        cout << node << endl;
    }
    BOOST_CHECK_EQUAL(4, i);

    nodeIterator = mesh.nodes.begin();
    for (i = 0; nodeIterator.hasNext(); i++)
    {
        const Node node = nodeIterator.next();
        cout << node << endl;
    }
    BOOST_CHECK_EQUAL(mesh.countNodes(), i);
}

BOOST_AUTO_TEST_CASE( test_CellGroup2Families )
{
    Mesh mesh(LogLevel::INFO, "test");
    vector<shared_ptr<CellGroup>> cellGroups;
    shared_ptr<CellGroup> gn1 = mesh.createCellGroup("GMA1");
    mesh.addCell(1, CellType::TRI3, {1,2,3});
    mesh.addCell(2, CellType::SEG2, {1,3});
    mesh.addCell(3, CellType::TRI3, {3,4,5});
    gn1->addCellId(1);
    gn1->addCellId(2);
    gn1->addCellId(3);
    cellGroups.push_back(gn1);
    shared_ptr<CellGroup> gn2 = mesh.createCellGroup("GMA2");
    gn2->addCellId(1);
    cellGroups.push_back(gn2);
    unordered_map<CellType::Code, int,hash<int>> cellCountByType;
    cellCountByType[CellType::SEG2_CODE] = 3;
    cellCountByType[CellType::TRI3_CODE] = 3;
    CellGroup2Families cg2fam(mesh, cellCountByType, cellGroups);
    auto& result = cg2fam.getFamilyOnCells();

    int expectedTri3[] = { -2, -1, 0 };
    auto& tri3 = result.find(CellType::TRI3_CODE)->second;
    BOOST_CHECK_EQUAL_COLLECTIONS(tri3->begin(), tri3->end(), expectedTri3,
                                  expectedTri3 + 3);

    int expectedSeg2[] = { -1, 0, 0 };
    auto& seg2 = result.find(CellType::SEG2_CODE)->second;
    BOOST_CHECK_EQUAL_COLLECTIONS(seg2->begin(), seg2->end(), expectedSeg2,
                                  expectedSeg2 + 3);

    vector<Family> families = cg2fam.getFamilies();
    BOOST_CHECK_EQUAL(2, families.size());
    bool famGMA1_GMA2_found = false;
    for (Family fam : families)
    {
        famGMA1_GMA2_found |= fam.name == "GMA1_GMA2";
    }
    BOOST_CHECK(famGMA1_GMA2_found);

}
