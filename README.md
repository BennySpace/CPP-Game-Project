# Echoes of the Majestic

Console text RPG / dungeon crawler written in C++.

The project features:
- navigation through a hostile world of connected locations
- collectible and usable items loaded from external data files
- enemies and turn-based combat
- a clear win condition
- a console interface

Story content, items, enemies, and locations are stored in `data/*.txt`.
The station map is generated in code from the location graph so it stays consistent with real room connections.