- figure out if we can get rid of `mycad::geom::Point()` default constructor -
  this is currently needed for rapidcheck to work properly.
- do we need to add some sort of precision thingy?
- add ccache to github action
- split up github actions into separate configure → build → test jobs
    - this will allow for separate "build" and "test" badges, which could be
      helpful
    - it also will make it easier to track down where errors are occuring
