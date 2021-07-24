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

            auto getPoint(VertexID const v) -> Point;

        private:
            std::map<VertexID, Point> vertices = {};
            Topology topo = Topology();
    };
} // namespace mycad


#endif // MYCAD_ENTITY_HEADER
