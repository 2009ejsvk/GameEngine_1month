from __future__ import annotations

import argparse
import csv
from pathlib import Path


DEFAULT_TSV = Path(
    r"E:\GameEngine_1month\Client\Building\Data\BuildingProductionRecipes.tsv"
)

FIELDNAMES = [
    "BuildingId",
    "ProducedType",
    "ProducedLabel",
    "Input1Type",
    "Input1Amount",
    "Input1Label",
    "Input2Type",
    "Input2Amount",
    "Input2Label",
    "Input3Type",
    "Input3Amount",
    "Input3Label",
    "Input4Type",
    "Input4Amount",
    "Input4Label",
    "VisitConsumptionType",
    "VisitAcceptedTypes",
]

TROPICO_UPSERTS = {
    "build_2_4": {
        "BuildingId": "build_2_4",
        "ProducedType": "Banana",
        "ProducedLabel": "바나나",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "Crops",
        "VisitAcceptedTypes": "Banana,Corn,Pineapple",
    },
    "build_2_6": {
        "BuildingId": "build_2_6",
        "ProducedType": "Meat",
        "ProducedLabel": "고기",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "AnimalProducts",
        "VisitAcceptedTypes": "Meat,Milk",
    },
    "build_2_7": {
        "BuildingId": "build_2_7",
        "ProducedType": "Coal",
        "ProducedLabel": "석탄",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "",
        "VisitAcceptedTypes": "",
    },
    "build_2_11": {
        "BuildingId": "build_2_11",
        "ProducedType": "Banana",
        "ProducedLabel": "바나나",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "HydroponicProduce",
        "VisitAcceptedTypes": "Banana,Corn,Pineapple",
    },
    "build_2_12": {
        "BuildingId": "build_2_12",
        "ProducedType": "Meat",
        "ProducedLabel": "고기",
        "Input1Type": "FeedCrops",
        "Input1Amount": "1",
        "Input1Label": "사료 작물",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "FactoryLivestock",
        "VisitAcceptedTypes": "Meat,Milk",
    },
    "build_3_1": {
        "BuildingId": "build_3_1",
        "ProducedType": "Coal",
        "ProducedLabel": "석탄",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "",
        "VisitAcceptedTypes": "",
    },
    "build_5_6": {
        "BuildingId": "build_5_6",
        "ProducedType": "None",
        "ProducedLabel": "-",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "Crops",
        "VisitAcceptedTypes": "Coconuts,Fish,FarmedFish,Banana,Corn,Pineapple,Meat,Milk,Cheese,CannedGoods,Juice",
    },
    "build_5_8": {
        "BuildingId": "build_5_8",
        "ProducedType": "None",
        "ProducedLabel": "-",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "FactoryLivestock",
        "VisitAcceptedTypes": "Meat,Cheese,CannedGoods",
    },
    "build_5_19": {
        "BuildingId": "build_5_19",
        "ProducedType": "None",
        "ProducedLabel": "-",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "Cheese",
        "VisitAcceptedTypes": "",
    },
    "build_8_2": {
        "BuildingId": "build_8_2",
        "ProducedType": "None",
        "ProducedLabel": "-",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "Crops",
        "VisitAcceptedTypes": "Coconuts,Fish,FarmedFish,Banana,Corn,Pineapple,Meat,Milk,Cheese,CannedGoods,Juice",
    },
    "build_8_10": {
        "BuildingId": "build_8_10",
        "ProducedType": "None",
        "ProducedLabel": "-",
        "Input1Type": "",
        "Input1Amount": "",
        "Input1Label": "",
        "Input2Type": "",
        "Input2Amount": "",
        "Input2Label": "",
        "Input3Type": "",
        "Input3Amount": "",
        "Input3Label": "",
        "Input4Type": "",
        "Input4Amount": "",
        "Input4Label": "",
        "VisitConsumptionType": "CannedGoods",
        "VisitAcceptedTypes": "",
    },
}

REMOVED_BUILDING_IDS = {
    "build_2_5",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Upsert Tropico-driven production recipes into BuildingProductionRecipes.tsv."
    )
    parser.add_argument("--tsv", type=Path, default=DEFAULT_TSV)
    return parser.parse_args()


def build_sort_key(building_id: str) -> tuple[int, int, str]:
    try:
        _, category, local = building_id.split("_", 2)
        return int(category), int(local), building_id
    except ValueError:
        return 999, 999, building_id


def main() -> None:
    args = parse_args()

    with args.tsv.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        rows = {
            row["# BuildingId"]: {
                "BuildingId": row["# BuildingId"],
                "ProducedType": row.get("ProducedType", ""),
                "ProducedLabel": row.get("ProducedLabel", ""),
                "Input1Type": row.get("Input1Type", ""),
                "Input1Amount": row.get("Input1Amount", ""),
                "Input1Label": row.get("Input1Label", ""),
                "Input2Type": row.get("Input2Type", ""),
                "Input2Amount": row.get("Input2Amount", ""),
                "Input2Label": row.get("Input2Label", ""),
                "Input3Type": row.get("Input3Type", ""),
                "Input3Amount": row.get("Input3Amount", ""),
                "Input3Label": row.get("Input3Label", ""),
                "Input4Type": row.get("Input4Type", ""),
                "Input4Amount": row.get("Input4Amount", ""),
                "Input4Label": row.get("Input4Label", ""),
                "VisitConsumptionType": row.get("VisitConsumptionType", ""),
                "VisitAcceptedTypes": row.get("VisitAcceptedTypes", ""),
            }
            for row in reader
            if row.get("# BuildingId")
        }

    for building_id in REMOVED_BUILDING_IDS:
        rows.pop(building_id, None)

    rows.update(TROPICO_UPSERTS)

    sorted_rows = [rows[key] for key in sorted(rows.keys(), key=build_sort_key)]

    args.tsv.parent.mkdir(parents=True, exist_ok=True)
    with args.tsv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["# " + FIELDNAMES[0], *FIELDNAMES[1:]])
        for row in sorted_rows:
            writer.writerow([row[field] for field in FIELDNAMES])

    print(f"Wrote {len(sorted_rows)} production recipe rows to {args.tsv}")


if __name__ == "__main__":
    main()
