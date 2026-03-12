from __future__ import annotations

import argparse
import csv
from pathlib import Path


DEFAULT_SOURCE_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6SourceBuildingInventory.tsv"
)
DEFAULT_OUTPUT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6WorkforceOverrides.tsv"
)


MATCHES = [
    ("build_1_6", "DA_T6LandingBP", 0, "Uneducated"),
    ("build_1_7", "DA_T6StorageQuayBP", 0, "Uneducated"),
    ("build_1_8", "DA_T6Warehouse_BuildingBP", 0, "Uneducated"),
    ("build_1_9", "DA_ElectricSubstation", 0, "Uneducated"),
    ("build_1_13", "DA_BusStop", 0, "Uneducated"),
    ("build_1_14", "DA_Tunnel", 0, "Uneducated"),
    ("build_1_18", "DA_WindFarm", 0, "Uneducated"),
    ("build_1_19", "DA_OffshoreWindTurbine", 0, "Uneducated"),
    ("build_6_9", "DA_T6InspiringBillboard", 0, "Uneducated"),
    ("build_6_10", "DA_T6InspiringStatue", 0, "Uneducated"),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate Tropico 6 workforce override data for the runtime building catalog."
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
        "# BuildingId\tCapacity\tRequiredEducation\tSourceAssetName\tSourceDisplayName"
    ]

    for building_id, asset_name, capacity, education in MATCHES:
        row = rows_by_asset.get(asset_name)
        if row is None:
            raise SystemExit(f"Missing source asset: {asset_name}")

        output_lines.append(
            "\t".join(
                [
                    building_id,
                    str(capacity),
                    education,
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
