#include "AlmanacPageData.h"
#include "RuntimeConfigRegistry.h"
#include "StringUtils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

namespace AlmanacPageData
{
    namespace
    {
        constexpr const wchar_t* GConfigId = L"UI.AlmanacPageData";

        void TrimString(std::string& Value)
        {
            Value.erase(
                Value.begin(),
                std::find_if(
                    Value.begin(),
                    Value.end(),
                    [](unsigned char Character)
                    {
                        return !std::isspace(Character);
                    }));
            Value.erase(
                std::find_if(
                    Value.rbegin(),
                    Value.rend(),
                    [](unsigned char Character)
                    {
                        return !std::isspace(Character);
                    }).base(),
                Value.end());
        }

        std::vector<std::string> Split(const std::string& Value, char Delimiter)
        {
            std::vector<std::string> Parts;
            std::string Current;

            for (char Character : Value)
            {
                if (Character == Delimiter)
                {
                    Parts.push_back(Current);
                    Current.clear();
                    continue;
                }

                Current.push_back(Character);
            }

            Parts.push_back(Current);
            return Parts;
        }

        std::wstring ParseTextValue(const std::string& Value)
        {
            std::string Unescaped;
            Unescaped.reserve(Value.size());

            for (size_t Index = 0; Index < Value.size(); ++Index)
            {
                if (Value[Index] == '\\' &&
                    Index + 1 < Value.size() &&
                    Value[Index + 1] == 'n')
                {
                    Unescaped.push_back('\n');
                    ++Index;
                    continue;
                }

                Unescaped.push_back(Value[Index]);
            }

            return StringUtils::Utf8ToWide(Unescaped);
        }

        int ParseInt(const std::string& Value, int Fallback = 0)
        {
            try
            {
                return std::stoi(Value);
            }
            catch (...)
            {
                return Fallback;
            }
        }

        float ParseFloat(const std::string& Value, float Fallback = 0.f)
        {
            try
            {
                return std::stof(Value);
            }
            catch (...)
            {
                return Fallback;
            }
        }

        void ParseModifierList(
            const std::string& Value,
            FModifierEntry* Entries,
            int EntryCount)
        {
            const std::vector<std::string> Items = Split(Value, ';');

            for (int Index = 0; Index < EntryCount; ++Index)
            {
                Entries[Index].Label.clear();
                Entries[Index].Value = 0;

                if (Index >= static_cast<int>(Items.size()))
                    continue;

                std::string Item = Items[static_cast<size_t>(Index)];
                TrimString(Item);

                if (Item.empty())
                    continue;

                const std::vector<std::string> Parts = Split(Item, ',');

                if (!Parts.empty())
                    Entries[Index].Label = ParseTextValue(Parts[0]);
                if (Parts.size() > 1)
                    Entries[Index].Value = ParseInt(Parts[1]);
            }
        }

        void ParseLabelValueList(
            const std::string& Value,
            FLabelValueEntry* Entries,
            int EntryCount)
        {
            const std::vector<std::string> Items = Split(Value, ';');

            for (int Index = 0; Index < EntryCount; ++Index)
            {
                Entries[Index].Label.clear();
                Entries[Index].Value.clear();

                if (Index >= static_cast<int>(Items.size()))
                    continue;

                std::string Item = Items[static_cast<size_t>(Index)];
                TrimString(Item);

                if (Item.empty())
                    continue;

                const std::vector<std::string> Parts = Split(Item, ',');

                if (!Parts.empty())
                    Entries[Index].Label = ParseTextValue(Parts[0]);
                if (Parts.size() > 1)
                    Entries[Index].Value = ParseTextValue(Parts[1]);
            }
        }

        std::array<int, GPoliticsNeutralCount> ParseIntList(
            const std::string& Value)
        {
            std::array<int, GPoliticsNeutralCount> Result = {};
            const std::vector<std::string> Parts = Split(Value, ',');

            for (int Index = 0; Index < GPoliticsNeutralCount; ++Index)
            {
                if (Index >= static_cast<int>(Parts.size()))
                    break;

                Result[static_cast<size_t>(Index)] = ParseInt(Parts[static_cast<size_t>(Index)]);
            }

            return Result;
        }

        FVector4 ParseTint(const std::string& Value, const FVector4& Fallback)
        {
            const std::vector<std::string> Parts = Split(Value, ',');

            if (Parts.size() != 4)
                return Fallback;

            return FVector4(
                ParseFloat(Parts[0], Fallback.x),
                ParseFloat(Parts[1], Fallback.y),
                ParseFloat(Parts[2], Fallback.z),
                ParseFloat(Parts[3], Fallback.w));
        }

        void SetDefaultData(FDataSet& Data)
        {
            Data.PoliticsNeutralCounts = { 124, 148, 134, 163 };
            Data.PoliticsSupportRows =
            {{
                { L"유권자", L"982" },
                { L"지지율", L"42%" },
                { L"미정", L"20%" },
                { L"반대", L"38%" }
            }};
            Data.PoliticsElectionText = L"다음 선거\n1월, 2029";

            Data.PoliticsFactions =
            {{
                { L"자본주의자", 422, 57, L"Adrián López", 192, 230, 0, 12,
                    FVector4(0.34f, 0.78f, 0.40f, 0.96f),
                    {{{ L"에펠 탑", 1 }, { L"대식당", 1 }, { L"노이슈반슈타인 성", 2 },
                      { L"패스트푸드 체인점", 2 }, { L"거주용 동", -7 }, { L"공항", 7 }}} },
                { L"공산주의자", 436, 71, L"Camila Ortega", 204, 232, 0, 9,
                    FVector4(0.84f, 0.24f, 0.18f, 0.96f),
                    {{{ L"공동 주택", 3 }, { L"대학교", 2 }, { L"복지 사무소", 2 },
                      { L"은행", -2 }, { L"공항", -1 }, { L"보건소", 5 }}} },
                { L"종교인", 420, 41, L"Jacob Hughes", 173, 247, 0, 0,
                    FVector4(0.90f, 0.86f, 0.78f, 0.96f),
                    {{{ L"에펠 탑", 1 }, { L"대성당", 1 }, { L"노이슈반슈타인 성", 2 },
                      { L"카바레", -1 }, { L"공항", 1 }, { L"고급 호텔", -1 }}} },
                { L"군국주의자", 414, 65, L"General Batista", 186, 228, 0, 8,
                    FVector4(0.56f, 0.44f, 0.16f, 0.96f),
                    {{{ L"막사", 3 }, { L"경비 초소", 2 }, { L"요새", 4 },
                      { L"교회", -1 }, { L"방송국", -2 }, { L"군항", 2 }}} },
                { L"환경주의자", 447, 83, L"Elena Verde", 219, 228, 0, 11,
                    FVector4(0.22f, 0.70f, 0.48f, 0.96f),
                    {{{ L"국립공원", 4 }, { L"풍력 발전소", 3 }, { L"정원", 2 },
                      { L"광산", -3 }, { L"공장", -2 }, { L"폐기물 처리장", 7 }}} },
                { L"실업가", 401, 62, L"Rafael Cruz", 183, 218, 0, 7,
                    FVector4(0.56f, 0.46f, 0.18f, 0.96f),
                    {{{ L"공장", 3 }, { L"무역항", 2 }, { L"은행", 2 },
                      { L"환경청", -3 }, { L"파업", -1 }, { L"공항", 4 }}} },
                { L"지식인", 427, 75, L"Prof. Lucía Vega", 211, 216, 0, 10,
                    FVector4(0.92f, 0.72f, 0.22f, 0.96f),
                    {{{ L"고등학교", 2 }, { L"대학교", 4 }, { L"도서관", 3 },
                      { L"막사", -2 }, { L"검문소", -1 }, { L"방송국", 4 }}} },
                { L"보수주의자", 392, 10, L"María del Sol", 180, 212, 0, 3,
                    FVector4(0.46f, 0.52f, 0.60f, 0.96f),
                    {{{ L"성당", 1 }, { L"경찰서", 2 }, { L"패스트푸드 체인점", -1 },
                      { L"공항", -2 }, { L"궁전", 2 }, { L"현대 예술관", -3 }}} }
            }};

            Data.ForeignPowers =
            {{
                { L"중국", 97, L"97", L"미확인", 0, 5, 39, 0, 3, 0,
                    {{{ L"대사관", 5 }, { L"", 0 }, { L"", 0 }}},
                    {{{ L"무역항 특별 파티", 39 }, { L"", 0 }}},
                    {{{ L"역사적 관문", 3 }, { L"", 0 }}} },
                { L"러시아", 97, L"97", L"미확인", 0, 5, 39, 0, 3, 0,
                    {{{ L"대사관", 5 }, { L"", 0 }, { L"", 0 }}},
                    {{{ L"무역항 특별 파티", 39 }, { L"", 0 }}},
                    {{{ L"역사적 관문", 3 }, { L"", 0 }}} },
                { L"미국", 104, L">100", L"우호적", 10000, 6, 14, 3, 1, 8,
                    {{{ L"대사관", 5 }, { L"관광객 유입", 1 }, { L"", 0 }}},
                    {{{ L"무역항 특별 파티", 10 }, { L"긴급 원조", 4 }}},
                    {{{ L"역사적 관문", 1 }, { L"", 0 }}} },
                { L"중동", 76, L"76", L"호의적", 0, 2, 5, 0, 1, 3,
                    {{{ L"대사관", 2 }, { L"", 0 }, { L"", 0 }}},
                    {{{ L"석유 거래", 5 }, { L"", 0 }}},
                    {{{ L"역사적 관문", 1 }, { L"", 0 }}} },
                { L"유럽연합", 49, L"49", L"경계", 0, 3, 0, -2, 0, 1,
                    {{{ L"대사관", 3 }, { L"", 0 }, { L"", 0 }}},
                    {{{ L"", 0 }, { L"", 0 }}},
                    {{{ L"역사적 관문", 1 }, { L"", 0 }}} }
            }};

            Data.BuildingCategories =
            {{
                { L"교통 & 기반시설", 96,
                    {{{ L"정류장", 54 }, { L"운송사업자 건물", 19 }, { L"태양광 발전소", 10 },
                      { L"해상 터미널", 7 }, { L"변전소", 2 }, { L"재건설 건물", 2 },
                      { L"부두", 1 }, { L"수상 공항", 1 }}} },
                { L"음식 & 자원", 65,
                    {{{ L"대규모 환경 목장", 39 }, { L"양식장", 13 }, { L"가공장과 목장", 12 },
                      { L"목장", 1 }, { L"", 0 }, { L"", 0 }, { L"", 0 }, { L"", 0 }}} },
                { L"공업", 0,
                    {{{ L"가구 공장", 0 }, { L"제약회사", 0 }, { L"조립 공장", 0 },
                      { L"제조소", 0 }, { L"자동차 공장", 0 }, { L"전자 공장", 0 },
                      { L"무기 공장", 0 }, { L"석유 공장", 0 }}} },
                { L"주거지", 50,
                    {{{ L"기숙사", 14 }, { L"아파트", 10 }, { L"펜트하우스", 7 },
                      { L"공동주택", 6 }, { L"호텔", 5 }, { L"테르마", 4 },
                      { L"컨트리하우스", 2 }, { L"시티하우스", 2 }}} },
                { L"오락", 39,
                    {{{ L"술집", 10 }, { L"레스토랑", 8 }, { L"극장", 6 },
                      { L"놀이공원", 5 }, { L"사이클링", 4 }, { L"영화관", 3 },
                      { L"클럽", 2 }, { L"야외 무대", 1 }}} },
                { L"문화 오락", 17,
                    {{{ L"카지노", 4 }, { L"골프장", 3 }, { L"스파", 3 },
                      { L"요트 클럽", 2 }, { L"카바레", 2 }, { L"고급 레스토랑", 1 },
                      { L"스페이스하우스", 1 }, { L"고급 클럽", 1 }}} },
                { L"복지 & 교육", 12,
                    {{{ L"초등학교", 3 }, { L"고등학교", 2 }, { L"대학교", 1 },
                      { L"도서관", 2 }, { L"라디오 방송국", 1 }, { L"TV 방송국", 1 },
                      { L"신문사", 1 }, { L"사이버 운영 센터", 1 }}} },
                { L"관광업", 124,
                    {{{ L"호텔", 45 }, { L"고급 호텔", 19 }, { L"관광객 유입", 14 },
                      { L"야외 별장", 12 }, { L"관광객 식당", 11 }, { L"아날로그 베이", 9 },
                      { L"해양 리조트", 8 }, { L"관광 공항", 6 }}} },
                { L"공익 서비스", 9,
                    {{{ L"병원", 2 }, { L"경찰서", 2 }, { L"소방서", 1 },
                      { L"수도원", 1 }, { L"교회", 1 }, { L"묘지", 1 },
                      { L"진료소", 1 }, { L"법원", 0 }}} },
                { L"공격 & 군사", 16,
                    {{{ L"막사", 5 }, { L"요새", 3 }, { L"경비 초소", 2 },
                      { L"검문소", 2 }, { L"군항", 1 }, { L"군수 공장", 1 },
                      { L"무기고", 1 }, { L"공군 기지", 1 }}} },
                { L"행정", 12,
                    {{{ L"궁전", 1 }, { L"세무국", 2 }, { L"의회", 2 },
                      { L"고등 법원", 2 }, { L"호텔", 2 }, { L"부동산청", 1 },
                      { L"노동부", 1 }, { L"관광청", 1 }}} },
                { L"장식물", 40,
                    {{{ L"공원", 10 }, { L"분수", 8 }, { L"동상", 7 },
                      { L"광장", 5 }, { L"화단", 4 }, { L"나무길", 3 },
                      { L"기념비", 2 }, { L"조각상", 1 }}} },
                { L"설계 불가사의", 4,
                    {{{ L"하늘섬", 1 }, { L"에이펠타워", 1 }, { L"자유의여신상", 1 },
                      { L"브루게르크 문", 1 }, { L"", 0 }, { L"", 0 }, { L"", 0 }, { L"", 0 }}} }
            }};
        }

        bool ApplyRecord(const std::string& Key, const std::string& Value, FDataSet& Data)
        {
            if (Key == "PoliticsNeutralCounts")
            {
                Data.PoliticsNeutralCounts = ParseIntList(Value);
                return true;
            }

            if (Key == "PoliticsSupportRows")
            {
                ParseLabelValueList(
                    Value,
                    Data.PoliticsSupportRows.data(),
                    GPoliticsSupportRowCount);
                return true;
            }

            if (Key == "PoliticsElectionText")
            {
                Data.PoliticsElectionText = ParseTextValue(Value);
                return true;
            }

            if (Key.rfind("PoliticsFaction", 0) == 0)
            {
                const int Index = ParseInt(Key.substr(15), -1);

                if (Index < 0 || Index >= GPoliticsFactionCount)
                    return false;

                std::vector<std::string> Parts = Split(Value, '|');
                auto& Entry = Data.PoliticsFactions[static_cast<size_t>(Index)];

                if (Parts.size() > 0) Entry.Label = ParseTextValue(Parts[0]);
                if (Parts.size() > 1) Entry.Count = ParseInt(Parts[1], Entry.Count);
                if (Parts.size() > 2) Entry.Favor = ParseInt(Parts[2], Entry.Favor);
                if (Parts.size() > 3) Entry.Leader = ParseTextValue(Parts[3]);
                if (Parts.size() > 4) Entry.ModerateCount = ParseInt(Parts[4], Entry.ModerateCount);
                if (Parts.size() > 5) Entry.StrongCount = ParseInt(Parts[5], Entry.StrongCount);
                if (Parts.size() > 6) Entry.FerventCount = ParseInt(Parts[6], Entry.FerventCount);
                if (Parts.size() > 7) Entry.BuildingModifierTotal = ParseInt(Parts[7], Entry.BuildingModifierTotal);
                if (Parts.size() > 8) Entry.IconTint = ParseTint(Parts[8], Entry.IconTint);
                if (Parts.size() > 9)
                {
                    ParseModifierList(
                        Parts[9],
                        Entry.Modifiers.data(),
                        GPoliticsModifierCount);
                }

                return true;
            }

            if (Key.rfind("ForeignPower", 0) == 0)
            {
                const int Index = ParseInt(Key.substr(12), -1);

                if (Index < 0 || Index >= GForeignPowerCount)
                    return false;

                std::vector<std::string> Parts = Split(Value, '|');
                auto& Entry = Data.ForeignPowers[static_cast<size_t>(Index)];

                if (Parts.size() > 0) Entry.Name = ParseTextValue(Parts[0]);
                if (Parts.size() > 1) Entry.Relation = ParseInt(Parts[1], Entry.Relation);
                if (Parts.size() > 2) Entry.DisplayValue = ParseTextValue(Parts[2]);
                if (Parts.size() > 3) Entry.Status = ParseTextValue(Parts[3]);
                if (Parts.size() > 4) Entry.EconomicAid = ParseInt(Parts[4], Entry.EconomicAid);
                if (Parts.size() > 5) Entry.BuildingModifier = ParseInt(Parts[5], Entry.BuildingModifier);
                if (Parts.size() > 6) Entry.EdictModifier = ParseInt(Parts[6], Entry.EdictModifier);
                if (Parts.size() > 7) Entry.ConstitutionModifier = ParseInt(Parts[7], Entry.ConstitutionModifier);
                if (Parts.size() > 8) Entry.MiscModifier = ParseInt(Parts[8], Entry.MiscModifier);
                if (Parts.size() > 9) Entry.TradeModifier = ParseInt(Parts[9], Entry.TradeModifier);
                if (Parts.size() > 10)
                {
                    ParseModifierList(
                        Parts[10],
                        Entry.BuildingLines.data(),
                        GForeignBuildingLineCount);
                }
                if (Parts.size() > 11)
                {
                    ParseModifierList(
                        Parts[11],
                        Entry.EdictLines.data(),
                        GForeignEdictLineCount);
                }
                if (Parts.size() > 12)
                {
                    ParseModifierList(
                        Parts[12],
                        Entry.MiscLines.data(),
                        GForeignMiscLineCount);
                }

                return true;
            }

            if (Key.rfind("BuildingCategory", 0) == 0)
            {
                const int Index = ParseInt(Key.substr(16), -1);

                if (Index < 0 || Index >= GBuildingCategoryCount)
                    return false;

                std::vector<std::string> Parts = Split(Value, '|');
                auto& Entry = Data.BuildingCategories[static_cast<size_t>(Index)];

                if (Parts.size() > 0) Entry.Label = ParseTextValue(Parts[0]);
                if (Parts.size() > 1) Entry.Count = ParseInt(Parts[1], Entry.Count);
                if (Parts.size() > 2)
                {
                    ParseModifierList(
                        Parts[2],
                        Entry.Details.data(),
                        GBuildingDetailCount);
                }

                return true;
            }

            return false;
        }

        class FPageDataRepository
        {
        public:
            static FPageDataRepository& Get()
            {
                static FPageDataRepository Repository;
                return Repository;
            }

            void ResetToDefaults()
            {
                SetDefaultData(CurrentData);
            }

            bool LoadFile(const std::wstring& Path)
            {
                std::ifstream File(Path);

                if (!File.is_open())
                    return false;

                FDataSet Data;
                SetDefaultData(Data);

                std::string Line;

                while (std::getline(File, Line))
                {
                    if (Line.empty() || Line[0] == '#' || Line[0] == ';')
                        continue;

                    const size_t EqPos = Line.find('=');

                    if (EqPos == std::string::npos)
                        continue;

                    std::string Key = Line.substr(0, EqPos);
                    std::string Value = Line.substr(EqPos + 1);
                    TrimString(Key);
                    TrimString(Value);

                    if (Key.empty())
                        continue;

                    ApplyRecord(Key, Value, Data);
                }

                CurrentData = std::move(Data);
                return true;
            }

            const FDataSet& GetData() const
            {
                return CurrentData;
            }

        private:
            FPageDataRepository()
            {
                ResetToDefaults();
            }

            FDataSet CurrentData;
        };

        void ResetRepositoryToDefaults()
        {
            FPageDataRepository::Get().ResetToDefaults();
        }

        bool LoadRepositoryFile(const std::wstring& Path)
        {
            return FPageDataRepository::Get().LoadFile(Path);
        }

    }

    void RegisterRuntimeConfig()
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                RuntimeConfigRegistry::BuildExeRelativePath(
                    L"AlmanacPageData.ini"),
                0.5f,
                &ResetRepositoryToDefaults,
                &LoadRepositoryFile,
                nullptr
            });
    }

    const FDataSet& Get()
    {
        RegisterRuntimeConfig();
        return FPageDataRepository::Get().GetData();
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::PollSource(GConfigId, DeltaTime);
    }
}
