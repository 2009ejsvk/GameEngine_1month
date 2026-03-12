# Tropico 6 Building Source Inventory

## Summary

- Mod Kit unique building data assets imported: 284
- Existing runtime build catalog rows: 159
- Source TSV: `E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv`
- Runtime `BuildingCatalog.tsv` was left untouched to avoid breaking hard-coded `build_x_y` references in the current game logic.
- This inventory is meant to be the project-side source reference for future catalog expansion, DLC import, and category/era mapping work.

## Counts by Era or Pack

- Cold War: 58
- Colonial: 53
- Drones: 15
- Festival: 8
- Future Era: 12
- Landmarks: 1
- Modern Times: 51
- Nature: 17
- Waterborne: 15
- World Wars: 45
- Zombie: 9

## Counts by Source Category

- Decoration: 16
- Entertainment: 23
- FoodAndResources: 19
- Government: 18
- Housing: 19
- Industry: 30
- Infrastructure: 27
- Landmarks: 23
- LuxuryEntertainment: 17
- MediaAndEducation: 15
- Military: 21
- PublicServices: 28
- Tourism: 21
- Unknown: 7

## Counts by Source Pack

- Cold War: 58
- Colonial: 53
- Drones: 15
- Festival: 8
- Future: 12
- Landmarks: 1
- Modern Times: 51
- Nature: 17
- Waterborne: 15
- World Wars: 45
- Zombie: 9

## Landmark Entries

- Landmark-tagged entries: 23
- Colosseum (Cold War, Landmarks)
- Great Sphinx (Cold War, Landmarks)
- Jesus Statue (Cold War, Landmarks)
- Saint Basil Cathedral (Cold War, Landmarks)
- Statue Of Liberty (Cold War, Landmarks)
- The Ball (Cold War, Landmarks)
- Brandenburg Gate (Colonial, Landmarks)
- Hagia Sophia (Colonial, Landmarks)
- Registanof Samarkand (Colonial, Landmarks)
- Stonehenge (Colonial, Landmarks)
- Eternal Flame (Landmarks, Landmarks)
- Moonlander (Modern Times, Landmarks)
- Neuschwanstein Castle (Modern Times, Landmarks)
- Temple Of Heaven (Modern Times, Landmarks)
- The Great Pyramid At Giza (Modern Times, Landmarks)
- White House (Modern Times, Landmarks)
- Winter Palace (Modern Times, Landmarks)
- World Tree (Nature, Landmarks)
- Ahu Akivi Moai Heads (World Wars, Landmarks)
- Eiffel Tower (World Wars, Landmarks)

## Highest Build Cost Entries

| Building | Era | Category | BuildCost |
| --- | --- | --- | ---: |
| Space Port Complex | Future Era | Military | 55000 |
| Space Program | Cold War | MediaAndEducation | 42000 |
| Fusion Reactor | Future Era | Infrastructure | 36000 |
| Nuclear Program | Cold War | MediaAndEducation | 35000 |
| Aircraft Carrier | Cold War | Military | 35000 |
| Nuclear Power Plant | Cold War | Infrastructure | 32000 |
| Mausoleum | Cold War | MediaAndEducation | 28000 |
| Pharmaceutical Company | Modern Times | Industry | 28000 |
| Electronics Factory | Modern Times | Industry | 25000 |
| Smart Furniture Studio | Modern Times | Industry | 25000 |
| Winter Palace | Modern Times | Landmarks | 25000 |
| El Prez Yacht Shipyard | Waterborne | Industry | 25000 |
| Vehicle Factory | Cold War | Industry | 22000 |
| Stadium | Modern Times | Entertainment | 21000 |
| Oil Rig | Cold War | FoodAndResources | 20000 |

## TSV Columns

- `DisplayName`: extracted building name
- `AssetName`: Tropico asset data name
- `SourceEra`: Mod Kit era or pack bucket
- `ProjectEraHint`: current project 4-era hint (`Colonial`, `WorldWars`, `ColdWar`, `Modern`)
- `SourceCategory`: original Tropico source category
- `ProjectCategoryHint`: current project category hint where mapping is straightforward
- `GameplayCategory`: lower-level gameplay tag found in the asset
- `BuildCost` / `BlueprintCost`: numeric values extracted from the data asset
- `IsLandmark`: original landmark flag from the data asset
- `AssetPath` / `BlueprintPath`: original Mod Kit references

## Integration Notes

- Some entries are special-purpose or internal data assets rather than player-buildable menu items. Keep them in the source inventory, but filter them before importing into the runtime build menu.
- DLC packs such as `Nature`, `Waterborne`, `Zombie`, `Drones`, and `Future Era` are currently mapped to `Modern` only as a project hint, not as a definitive gameplay-era decision.
- `Landmarks` do not map cleanly to a single runtime category in the current project. Use `SourceCategory` plus design intent when folding them into `BuildingCatalog.tsv`.
