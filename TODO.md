Open
====
- [ ] figure out if we can get rid of `mycad::geom::Point()` default constructor -
      this is currently needed for rapidcheck to work properly.
- [ ] do we need to add some sort of precision thingy?
- [ ] add ccache to github action
- [ ] split up github actions into separate configure → build → test jobs
    - this will allow for separate "build" and "test" badges, which could be
      helpful
    - it also will make it easier to track down where errors are occuring
- [ ] set up a `mycad::ErrorCode` enum so we can be a bit more expressive using our
      `tl::expected`
- [ ] add moar documentation
- [ ] integrate shader compiling and generating headers (with byte code stuff)
      into cmake
- [ ] create separate include/mycad/Renderer subdir and move all rendering stuff
      in there
- [ ] allow for different shaders (i.e. texture vs. flat color) at runtime

Closed
======
- [x] use east const
- [x] try to centralize the github actions yaml's so we don't duplicate it.
- [x] consider writing `mycad::expected` rather than using the one from
      https:://github.com/tartanllama/expected
    - look at https://github.com/ezzieyguywuf/expected
