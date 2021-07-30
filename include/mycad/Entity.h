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
            auto addEdge(VertexID, VertexID) -> MaybeEdgeID{return std::nullopt;};

            auto getPoint(VertexID const v) -> Point;
            auto getLine(EdgeID) -> MaybeLine{return std::nullopt;};

        private:
            std::map<VertexID, Point> vertices = {};
            std::map<EdgeID, Line> edges = {};
            Topology topo = Topology();
    };
} // namespace mycad


#endif // MYCAD_ENTITY_HEADER
