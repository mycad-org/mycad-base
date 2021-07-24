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
            auto addVertex(Point const p) -> topo::VertexID;

            auto getPoint(topo::VertexID const v) -> Point;

        private:
            std::map<topo::VertexID, Point> vertices = {};
            topo::Topology topo = topo::Topology();
    };
} // namespace mycad


#endif // MYCAD_ENTITY_HEADER
