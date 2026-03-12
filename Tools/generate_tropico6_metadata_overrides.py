from __future__ import annotations

import argparse
import csv
from pathlib import Path


DEFAULT_SOURCE_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv"
)
DEFAULT_OUTPUT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6SourceMetadataOverrides.tsv"
)


MATCHES = [
    ("build_3_4", "DA_T6TanneryBP"),
    ("build_5_13", "DA_T6Theater"),
    ("build_5_14", "DA_T6OperaHouseBP"),
    ("build_5_15", "DA_Cabaret"),
    ("build_5_16", "DA_Casino"),
    ("build_5_17", "DA_CocktailBar"),
    ("build_5_18", "DA_T6GolfCourse"),
    ("build_5_19", "DA_T6GourmetRestaurant"),
    ("build_5_20", "DA_NightClub"),
    ("build_5_21", "DA_SnorkelBay"),
    ("build_5_22", "DA_BeachResort"),
    ("build_5_23", "DA_T6HangGlider"),
    ("build_5_24", "DA_MuseumOfModernArt"),
    ("build_5_25", "DA_T6YachtClub"),
    ("build_6_6", "DA_T6ChildhoodMuseum"),
    ("build_6_7", "DA_T6Mausoleum"),
    ("build_7_12", "DA_T6Airport"),
    ("build_8_14", "DA_CustomsOffice"),
    ("build_10_4", "DA_WatchtowerModern"),
    ("build_10_7", "DA_T6CommandoGarrison"),
    ("build_11_5", "DA_ImmigrationOffice"),
    ("build_11_7", "DA_Embassy"),
    ("build_11_8", "DA_Bank"),
    ("build_11_12", "DA_T6Office"),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate Tropico 6 era/category override data for the runtime building catalog."
    )
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE_TSV)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_TSV)
    return parser.parse_args()


def read_source_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        header_line = ""

        for raw_line in handle:
            if raw_line.startswith("#"):
                header_line = raw_line[2:].rstrip("\r\n")
                break

        if not header_line:
            return []

        fieldnames = header_line.split("\t")
        reader = csv.DictReader(handle, fieldnames=fieldnames, delimiter="\t")
        return [row for row in reader if row.get("AssetName")]


def main() -> None:
    args = parse_args()
    rows = read_source_rows(args.source)
    rows_by_asset = {row["AssetName"]: row for row in rows}

    output_lines = [
        "# BuildingId\tUnlockEra\tBuildMenuCategory\tSourceAssetName\tSourceDisplayName"
    ]

    for building_id, asset_name in MATCHES:
        row = rows_by_asset.get(asset_name)
        if row is None:
            raise SystemExit(f"Missing source asset: {asset_name}")

        output_lines.append(
            "\t".join(
                [
                    building_id,
                    row.get("ProjectEraHint", "").strip(),
                    row.get("ProjectCategoryHint", "").strip(),
                    asset_name,
                    row.get("DisplayName", "").replace("\t", " "),
                ]
            )
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(output_lines) + "\n", encoding="utf-8")
    print(f"Wrote {len(MATCHES)} override rows to {args.output}")


if __name__ == "__main__":
    main()
