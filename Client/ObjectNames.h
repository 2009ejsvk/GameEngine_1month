#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// 게임 오브젝트 / 위젯 이름 상수
//
//   World::FindObject, WorldUIManager::FindWidget, CreateGameObject 등에서
//   문자열 리터럴을 직접 쓰는 대신 이 상수를 사용한다.
//   이름을 바꿀 때 이 파일만 수정하면 모든 참조가 함께 업데이트된다.
// ─────────────────────────────────────────────────────────────────────────────

// ── TileMap 오브젝트 ─────────────────────────────────────────────────────────
constexpr const char* GTileMapObjectName         = "TileMap";
constexpr const char* GTileMapFloorBlueName      = "TileMapFloorBlue";
constexpr const char* GTileMapFloorYellowName    = "TileMapFloorYellow";
constexpr const char* GTileMapRoadPreviewName    = "TileMapRoadPreview";
constexpr const char* GTileMapExpansionName      = "TileMapExpansion";

// ── 배치 시스템 오브젝트 ─────────────────────────────────────────────────────
constexpr const char* GMainCameraName          = "MainCamera";
constexpr const char* GPlacementControllerName  = "PlacementController";
// 동적 이름: GPlacedBuildingPrefix + std::to_string(N) 형태로 사용
constexpr const char* GPlacedBuildingPrefix     = "PlacedBuilding_";

// ── UI 위젯 ──────────────────────────────────────────────────────────────────
constexpr const char* GBuildMenuWidgetName      = "BuildMenuWidget";
constexpr const char* GAlmanacWidgetName        = "AlmanacWidget";
constexpr const char* GEdictWidgetName          = "EdictWidget";
constexpr const char* GTradeWidgetName          = "TradeWidget";
constexpr const char* GTopHudWidgetName         = "TopHudWidget";
constexpr const char* GCitizenInfoWidgetName    = "CitizenInfoWidget";
