from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path


DEFAULT_INPUT = Path(
    r"C:\Users\2009e\OneDrive\Documents\Playground\reports\tropico6_buildings_inventory.csv"
)
DEFAULT_OUTPUT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv"
)
DEFAULT_OUTPUT_MD = Path(
    r"E:\GameEngine_1month\Docs\Tropico6_Building_Source_Inventory.md"
)
RUNTIME_CATALOG_PATH = Path(
    r"E:\GameEngine_1month\Client\Building\Data\BuildingCatalog.tsv"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Import Tropico 6 Mod Kit building inventory into this project."
    )
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output-tsv", type=Path, default=DEFAULT_OUTPUT_TSV)
    parser.add_argument("--output-md", type=Path, default=DEFAULT_OUTPUT_MD)
    return parser.parse_args()


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        return list(reader)


def read_runtime_catalog_count(path: Path) -> int:
    if not path.exists():
        return 0

    count = 0
    with path.open("r", encoding="utf-8-sig") as handle:
        for raw_line in handle:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue
            count += 1
    return count


def normalize_project_era_hint(source_era: str) -> str:
    mapping = {
        "Colonial": "Colonial",
        "World Wars": "WorldWars",
        "Cold War": "ColdWar",
        "Modern Times": "Modern",
        "Drones": "Modern",
        "Future Era": "Modern",
        "Festival": "Modern",
        "Nature": "Modern",
        "Waterborne": "Modern",
        "Zombie": "Modern",
        "Landmarks": "Modern",
    }
    return mapping.get(source_era, "")


def normalize_project_category_hint(source_category: str) -> str:
    mapping = {
        "Decoration": "Infrastructure",
        "Entertainment": "Entertainment",
        "FoodAndResources": "FoodResource",
        "Government": "GovernmentFinance",
        "Housing": "Housing",
        "Industry": "Industry",
        "Infrastructure": "Infrastructure",
        "LuxuryEntertainment": "LuxuryEntertainment",
        "MediaAndEducation": "MediaEducation",
        "Military": "Military",
        "PublicServices": "PublicService",
        "Tourism": "Tourism",
    }
    return mapping.get(source_category, "")


def as_int(text: str) -> int:
    if not text:
        return 0
    try:
        return int(round(float(text)))
    except ValueError:
        return 0


def write_tsv(rows: list[dict[str, str]], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        handle.write(
            "# DisplayName\tAssetName\tSourceEra\tProjectEraHint\tSourcePack\t"
            "SourceGroup\tSourceCategory\tProjectCategoryHint\tGameplayCategory\t"
            "BuildCost\tBlueprintCost\tConnectionCostPerCell\tIsLandmark\t"
            "KeybindAction\tAssetPath\tBlueprintPath\n"
        )
        for row in rows:
            fields = [
                row["display_name"],
                row["asset_name"],
                row["era"],
                normalize_project_era_hint(row["era"]),
                row["source_pack"],
                row["source_group"],
                row["category"],
                normalize_project_category_hint(row["category"]),
                row["gameplay_category"],
                str(as_int(row["build_cost"])),
                str(as_int(row["blueprint_cost"])),
                str(as_int(row["connection_cost_per_cell"])),
                row["is_landmark"],
                row["keybind_action"],
                row["asset_path"],
                row["blueprint_path"],
            ]
            handle.write("\t".join(field.replace("\t", " ").replace("\n", " ") for field in fields))
            handle.write("\n")


def write_markdown(
    rows: list[dict[str, str]],
    path: Path,
    runtime_catalog_count: int,
    output_tsv_path: Path,
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)

    era_counts = Counter(row["era"] or "Unknown" for row in rows)
    category_counts = Counter(row["category"] or "Unknown" for row in rows)
    pack_counts = Counter(row["source_pack"] or "Unknown" for row in rows)
    landmark_rows = [row for row in rows if row["is_landmark"] == "True"]
    top_cost_rows = sorted(rows, key=lambda row: as_int(row["build_cost"]), reverse=True)[:15]

    with path.open("w", encoding="utf-8") as handle:
        handle.write("# Tropico 6 Building Source Inventory\n\n")
        handle.write("## Summary\n\n")
        handle.write(
            f"- Mod Kit unique building data assets imported: {len(rows)}\n"
        )
        handle.write(
            f"- Existing runtime build catalog rows: {runtime_catalog_count}\n"
        )
        handle.write(
            f"- Source TSV: `{output_tsv_path}`\n"
        )
        handle.write(
            "- Runtime `BuildingCatalog.tsv` was left untouched to avoid breaking "
            "hard-coded `build_x_y` references in the current game logic.\n"
        )
        handle.write(
            "- This inventory is meant to be the project-side source reference for "
            "future catalog expansion, DLC import, and category/era mapping work.\n"
        )

        handle.write("\n## Counts by Era or Pack\n\n")
        for key, count in sorted(era_counts.items()):
            handle.write(f"- {key}: {count}\n")

        handle.write("\n## Counts by Source Category\n\n")
        for key, count in sorted(category_counts.items()):
            handle.write(f"- {key}: {count}\n")

        handle.write("\n## Counts by Source Pack\n\n")
        for key, count in sorted(pack_counts.items()):
            handle.write(f"- {key}: {count}\n")

        handle.write("\n## Landmark Entries\n\n")
        handle.write(f"- Landmark-tagged entries: {len(landmark_rows)}\n")
        for row in landmark_rows[:20]:
            handle.write(
                f"- {row['display_name']} ({row['era']}, {row['category'] or 'Unknown'})\n"
            )

        handle.write("\n## Highest Build Cost Entries\n\n")
        handle.write("| Building | Era | Category | BuildCost |\n")
        handle.write("| --- | --- | --- | ---: |\n")
        for row in top_cost_rows:
            handle.write(
                f"| {row['display_name']} | {row['era']} | "
                f"{row['category'] or 'Unknown'} | {as_int(row['build_cost'])} |\n"
            )

        handle.write("\n## TSV Columns\n\n")
        handle.write("- `DisplayName`: extracted building name\n")
        handle.write("- `AssetName`: Tropico asset data name\n")
        handle.write("- `SourceEra`: Mod Kit era or pack bucket\n")
        handle.write("- `ProjectEraHint`: current project 4-era hint (`Colonial`, `WorldWars`, `ColdWar`, `Modern`)\n")
        handle.write("- `SourceCategory`: original Tropico source category\n")
        handle.write("- `ProjectCategoryHint`: current project category hint where mapping is straightforward\n")
        handle.write("- `GameplayCategory`: lower-level gameplay tag found in the asset\n")
        handle.write("- `BuildCost` / `BlueprintCost`: numeric values extracted from the data asset\n")
        handle.write("- `IsLandmark`: original landmark flag from the data asset\n")
        handle.write("- `AssetPath` / `BlueprintPath`: original Mod Kit references\n")

        handle.write("\n## Integration Notes\n\n")
        handle.write(
            "- Some entries are special-purpose or internal data assets rather than "
            "player-buildable menu items. Keep them in the source inventory, but "
            "filter them before importing into the runtime build menu.\n"
        )
        handle.write(
            "- DLC packs such as `Nature`, `Waterborne`, `Zombie`, `Drones`, and "
            "`Future Era` are currently mapped to `Modern` only as a project hint, "
            "not as a definitive gameplay-era decision.\n"
        )
        handle.write(
            "- `Landmarks` do not map cleanly to a single runtime category in the "
            "current project. Use `SourceCategory` plus design intent when folding "
            "them into `BuildingCatalog.tsv`.\n"
        )


def main() -> None:
    args = parse_args()
    rows = read_rows(args.input)
    runtime_count = read_runtime_catalog_count(RUNTIME_CATALOG_PATH)
    write_tsv(rows, args.output_tsv)
    write_markdown(rows, args.output_md, runtime_count, args.output_tsv)
    print(f"Imported {len(rows)} Tropico 6 source inventory rows.")
    print(f"TSV: {args.output_tsv}")
    print(f"Markdown: {args.output_md}")


if __name__ == "__main__":
    main()
