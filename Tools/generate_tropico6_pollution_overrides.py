from __future__ import annotations

import argparse
import csv
from pathlib import Path


DEFAULT_SOURCE_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\BuildingCatalog.tsv"
)
DEFAULT_OUTPUT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\Tropico6PollutionOverrides.tsv"
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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate structured Tropico 6 pollution overrides from BuildingCatalog.tsv."
    )
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE_TSV)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_TSV)
    return parser.parse_args()


def build_id(category_name: str, local_index_text: str) -> str:
    category_index = CATEGORY_ORDER.index(category_name) + 1
    local_index = int(local_index_text) + 1
    return f"build_{category_index}_{local_index}"


def extract_base_pollution_summary(detail_text: str) -> str:
    summary_lines: list[str] = []

    for raw_line in detail_text.split("\\n"):
        stripped = raw_line.strip()
        if stripped.startswith("효과:") or stripped.startswith("비고:"):
            summary_lines.append(stripped)

    return "\n".join(summary_lines)


def extract_pollution_output(detail_text: str) -> int:
    summary_text = extract_base_pollution_summary(detail_text)

    if "많은 공해 배출" in summary_text:
        return 32

    if "건물 자체는 공해 배출" in summary_text:
        return 18

    if "적은 공해 배출" in summary_text or "적은 공해" in summary_text:
        return 8

    if "공해 배출" in summary_text:
        return 18

    return 0


def extract_pollution_mitigation(detail_text: str) -> int:
    summary_text = extract_base_pollution_summary(detail_text)

    if "범위 내 다른 건물 공해 감소" in summary_text:
        return 20

    if "주변 공해 감소" in summary_text:
        return 12

    if "공해 감소" in summary_text:
        return 12

    return 0


def main() -> None:
    args = parse_args()

    with args.source.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        rows = list(reader)

    output_lines = [
        "# BuildingId\tPollutionOutput\tPollutionMitigation\tSourceDisplayName"
    ]
    row_count = 0

    for row in rows:
        detail_text = row.get("DetailText", "")
        pollution_output = extract_pollution_output(detail_text)
        pollution_mitigation = extract_pollution_mitigation(detail_text)

        if pollution_output <= 0 and pollution_mitigation <= 0:
            continue

        output_lines.append(
            "\t".join(
                [
                    build_id(row["# Category"], row["LocalIndex"]),
                    str(pollution_output) if pollution_output > 0 else "",
                    str(pollution_mitigation) if pollution_mitigation > 0 else "",
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
