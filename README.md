Dungeon Crawler
=======

Dungeon Crawler is a 3D action RPG/adventure game, built using my Gumshoe game engine.
It currently only supports Windows and DirectX 11 (I have no porting plans).

The main purpose of the game is to serve as a functional project used to test and build up the engine.  The current design is to make a 3D isometric game (in the vein of Diablo), with action RPG elements and some puzzle elements (a la Resident Evil).

My goals include:

* Fully functional 3D game using the custom engine
* Procedurally generated maps and (maybe) puzzles
* AI
* Physics/collision system
* Action RPG & puzzle gameplay mechanics


Current State of the Project
----------------------------

 - What features are currently implemented?

See [docs/status.md](/docs/status.md).

 - What's the plan?

See [docs/milestones.md](/docs/milestones.md). I also plan to keep a list of random ideas here [docs/ideas](/docs/ideas).


Dependencies, Building and Running
----------------------------------

The engine only supports Windows right now (using DirectX 11 and Visual Studio compiler).

To build the game, source the shell.bat script in the misc folder (this sets up the path to the compiler).  Then, source the build.bat script (this builds the whole game).

I have not included the assets (audio, textures, models, etc.) in the repository.


License
-------

**The MIT License**; see [license.md](license.md).
