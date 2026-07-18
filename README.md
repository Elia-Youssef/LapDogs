Exit code: 0
Wall time: 1 seconds
Output:
# LapDogs

LapDogs is a multiplayer online kart racing game built in Unreal Engine 5 for the Shiba Inu ecosystem. Shiboshi and Sheboshi avatars compete in ability-driven races featuring Grand Prix events, online matchmaking, AI competitors, live leaderboards, and tournament integrations.

## Visuals

[View screenshots and gameplay videos on the Rebel Art Studios project page.](https://rebelartstudios.org/project/lap-dogs)

## Features

- Networked client and dedicated-server racing
- Epic Online Services matchmaking and session management
- Grand Prix events with configurable tracks and race rounds
- Live race, final race, and Grand Prix leaderboards
- AI racers that can fill available session slots
- Character classes with primary and secondary abilities
- Pickups, obstacles, checkpoints, lap timing, and race statistics
- Custom drifting with network-predicted movement
- Tournament, player statistics, and reward-service integrations
- Spectator cameras and synchronized race-state transitions

## Technology

- Unreal Engine 5.4
- C++ with Blueprint extension points
- Enhanced Input
- Gameplay Tags and Gameplay Tasks
- Niagara and UMG
- Epic Online Services through Redpoint EOS
- Client, game, editor, and dedicated-server build targets

## Project structure

| Path | Purpose |
| --- | --- |
| `Source/ShibRun/` | Core racing, character, AI, multiplayer, and game-state code |
| `Plugins/AbilitySystem/` | Abilities, effects, attributes, and modifiers |
| `Plugins/ShibMetaverseEOS/` | Authentication, matchmaking, and session management |
| `Plugins/ShibAPIs/` | Player statistics, tournament, payment, and metaverse APIs |
| `Plugins/ShibUiNavigation/` | UI navigation and save-game systems |
| `Plugins/ShibAsyncLoadingScreen/` | Startup and map loading screens |
| `Plugins/ShibChunkingSystem/` | Runtime content chunk management |
| `Config/` | Engine, input, packaging, platform, and gameplay-tag configuration |

## Repository scope

This repository is a source-code and portfolio showcase. It contains the game's C++ modules, configuration, and selected custom plugins.

The complete game content, proprietary assets, private service configuration, and some licensed Marketplace plugins are not distributed here. As a result, this checkout is not a standalone playable build.

## Development requirements

Authorized contributors need:

- Unreal Engine 5.4
- A compatible C++ development toolchain
- The private `Content/` directory
- Team-managed EOS and backend configuration
- The external plugins referenced by `ShibRun.uproject`, including:
  - Redpoint EOS Online Subsystem
  - Online Subsystem Blueprints
  - Matchmaking
  - DragonIK
  - Sky Creator

After restoring the private dependencies:

1. Generate project files from `ShibRun.uproject`.
2. Build the `ShibRunEditor` target.
3. Open the project in Unreal Editor.
4. Use the appropriate LapDogs or ShibRun client/server target for packaged multiplayer builds.

## Ownership and licensing

Copyright notices in the source identify Shiba Inu Games LLC.

No open-source license is currently included. Unless a separate license grants permission, the source is provided for viewing and portfolio reference only.

