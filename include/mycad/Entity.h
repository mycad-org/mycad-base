#ifndef MYCAD_ENTITY_HEADER
#define MYCAD_ENTITY_HEADER

#include "Geometry.h"
#include "Topology.h"

#include <map>

namespace mycad
{
    class Entity
    {
        public:
            auto addVertex(Point const p) -> VertexID;
            auto addEdge(VertexID const v1, VertexID const v2) -> MaybeEdgeID;

            auto getPoint(VertexID const v) const -> Point;
            auto getLine(EdgeID const e) const -> MaybeLine;

        private:
            std::map<VertexID, Point> vertices = {};
            std::map<EdgeID, Line> edges = {};
            Topology topo = Topology();
    };
} // namespace mycad


#endif // MYCAD_ENTITY_HEADER
