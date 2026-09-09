#include "../../src/3D/Grid/_OctreeGrid.h"
#include "../../src/3D/Viewer/_ImGUIviewer.h"
#include <cassert>
#include <iostream>

using namespace kai;
class Grid : public _OctreeGrid {
public:
    using _OctreeGrid::updateDrawAssets;
    using _OctreeGrid::deleteExpiredCells;
    void age(OCTREE_CELL<OCTGRID_PCL_CELL>* cell = nullptr) {
        if (!cell) cell = m_pCell;
        if (cell->getT()) cell->getT()->m_tStamp = 1;
        for (auto child : cell->m_pChild) if (child) age(child);
    }
};
class Viewer : public _ImGUIviewer {
public:
    using _ImGUIviewer::collectGeometry;
};
int main() {
    Grid grid;
    grid.setName("grid");
    assert(grid.init(json{{"class","_OctreeGrid"}, {"thread",{{"FPS",30}}}, {"nP",16},
        {"nMaxLevel",40}, {"nMaxCells",100}, {"dTexpireCell",1000},
        {"vPorigin",{10,-20,30}}, {"vRootCellSize",{8,4,2}}}));
    OCTGRID_CELLS snapshot;
    assert(grid.get(&snapshot) == 0 && snapshot.m_header.m_tStamp == 0);
    GEOMETRY_POINT point{vFloat3(13.25,-18.375,30.8125),vFloat3(1,0,0),getApproxTbootUs()};
    assert(grid.addCellPoint(point, point.m_tStamp));
    for (int depth = 0; depth <= 40; ++depth) {
        auto cell = grid.getCell(point.m_vP, depth);
        assert(cell && (cell->m_ID.m_uint64[0]&63) == uint64_t(depth));
        assert(grid.getCell(cell->m_ID) == cell);
    }
    assert(!grid.getCell(UUID128(64))); // root with unused path bits
    assert(!grid.getCell(UUID128(41)));
    grid.updateDrawAssets();
    assert(grid.get(&snapshot) == 41);
    const auto stableCells = snapshot.m_vCell;
    assert(stableCells[0].id() == uint64_t(0));
    for (const auto &cell : stableCells) assert(cell.m_vC[0] == 255 && cell.m_vC[1] == 0 && cell.m_vC[2] == 0);
    assert(grid.get(&snapshot,0,2) == 2);
    assert(grid.get(&snapshot,0,0) == 0 && snapshot.m_header.m_tStamp);
    assert(grid.get(&snapshot,snapshot.m_header.m_tStamp) == 0);
    point.m_vP = vFloat3(13.375,-18.25,30.875);
    point.m_vC = vFloat3(0,0,1);
    grid.addCellPoint(point,getApproxTbootUs());
    for (const auto &record : stableCells) {
        auto id = record.id();
        auto cell = grid.getCell(id);
        assert(cell && cell->m_ID.m_uint64[0] == id.m_uint64[0] && cell->m_ID.m_uint64[1] == id.m_uint64[1]);
    }
    grid.updateDrawAssets();
    grid.get(&snapshot);
    assert(snapshot.m_vCell[0].m_vC[0] == 128 && snapshot.m_vCell[0].m_vC[2] == 128);
    Viewer viewer;
    IMGUI_VIEWER_OBJ object;
    object.m_pGB = &grid;
    object.m_nCbuf = 2;
    viewer.collectGeometry(&grid,&object);
    assert(object.m_vBox.size() == 2 && object.m_vL.empty());
    assert(object.m_vBox[0].m_ID == uint64_t(0));
    assert(object.m_vBox[0].m_vCenter.x == 10 && object.m_vBox[0].m_vSize.y == 4);
    int edges = 0;
    object.m_vBox[0].forEachEdge([&](const vFloat3 &a, const vFloat3 &b) {
        assert((a.x != b.x) + (a.y != b.y) + (a.z != b.z) == 1);
        ++edges;
    });
    assert(edges == 12);
    GEOMETRY_RINGBUF<GEOMETRY_LINE> lines;
    assert(grid.get(&lines) == 0);
    grid.age(); grid.deleteExpiredCells(); grid.updateDrawAssets();
    assert(grid.get(&snapshot) == 0 && snapshot.m_vCell.empty());
    object.clearGeometry(); viewer.collectGeometry(&grid,&object);
    assert(object.m_vBox.empty() && object.m_vL.empty());
    std::cout << "PASS: native ID stability/lookup at depths 0-40, RGB averaging, caps/expiry and ImGUI boxes/clearing\n";
}
