Provides the necessary geometric objects and algorithms needed to implement a
CAD system.

Provides a `Topology` data structure that can be used to manage topological
entities in a CAD application.

Can be built as follows:

This project can be built using cmake:

```sh
mkdir build
cd build
cmake ..
make
```

If you want to run the unit tests, you'll need to install the [Catch2][1]
library. If you don't want to (or can't) install it, then you can have cmake
fetch a local copy with:

```sh
cmake -DMYCAD_FETCH_CATCH=ON ..
```

[1]: https://github.com/catchorg/Catch2
