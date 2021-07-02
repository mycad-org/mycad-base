#ifndef MYCAD_TOPOLOGY_HEADER
#define MYCAD_TOPOLOGY_HEADER

#include <iostream>
#include "tl/expected.hpp"

namespace mycad {
    namespace topo {
        class Topology{
            public:
                int addFreeVertex();

            private:
                int lastVertexID = 0;
        };
    } // namespace topo
}     // namespace mycad


#endif // MYCAD_TOPOLOGY_HEADER
