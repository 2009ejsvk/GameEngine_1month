from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path


DEFAULT_SOURCE_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\BuildingCatalog.tsv"
)
DEFAULT_OUTPUT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6PowerOverrides.tsv"
)

CATEGORY_ORDER = [
    "Infrastructure",
    "FoodResource",
    "Industry",
    "Housing",
    "Entertainment",
    "MediaEducation",
    "Tourism",
    "PublicService",
    "LuxuryEntertainment",
    "Military",
    "GovernmentFinance",
]

POWER_PATTERN = re.compile(r"([+-]?\d+)")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate structured Tropico 6 power overrides from BuildingCatalog.tsv."
    )
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE_TSV)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_TSV)
    return parser.parse_args()


def extract_power(detail_text: str, prefixes: tuple[str, ...]) -> int:
    for line in detail_text.split("\\n"):
        stripped = line.strip()
        if not any(stripped.startswith(prefix) for prefix in prefixes):
            continue

        match = POWER_PATTERN.search(stripped)
        if match:
            return max(0, int(match.group(1)))

    return 0


def build_id(category_name: str, local_index_text: str) -> str:
    category_index = CATEGORY_ORDER.index(category_name) + 1
    local_index = int(local_index_text) + 1
    return f"build_{category_index}_{local_index}"


def main() -> None:
    args = parse_args()

    with args.source.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        rows = list(reader)

    output_lines = [
        "# BuildingId\tProducedPowerMW\tRequiredPowerMW\tSourceDisplayName"
    ]
    row_count = 0

    for row in rows:
        detail_text = row.get("DetailText", "")
        produced_power = extract_power(detail_text, ("생산 전력:", "발전량:"))
        required_power = extract_power(detail_text, ("필요 전력:",))

        if produced_power <= 0 and required_power <= 0:
            continue

        output_lines.append(
            "\t".join(
                [
                    build_id(row["# Category"], row["LocalIndex"]),
                    str(produced_power) if produced_power > 0 else "",
                    str(required_power) if required_power > 0 else "",
                    row.get("DisplayName", "").replace("\t", " "),
                ]
            )
        )
        row_count += 1

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(output_lines) + "\n", encoding="utf-8")
    print(f"Wrote {row_count} override rows to {args.output}")


if __name__ == "__main__":
    main()
