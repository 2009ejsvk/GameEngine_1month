#include "PathManager.h"

std::unordered_map<std::string, TCHAR*> CPathManager::mPathMap;

void CPathManager::Clear()
{
	auto	iter = mPathMap.begin();
	auto	iterEnd = mPathMap.end();

	for (; iter != iterEnd; ++iter)
	{
		delete[] iter->second;
	}
}

bool CPathManager::Init()
{
	// 루트 경로를 만들어둔다.
	// 실행파일 이름까지 포함한 실행파일이 있는 전체 경로를 얻어온다.
	TCHAR* Path = new TCHAR[MAX_PATH];
	GetModuleFileName(0, Path, MAX_PATH);

	// abc/defdd
	// D:\Lecture\UE20252\DirectX\GameEngine\Binary\Client_Debug.exe
	// 뒷 부분의 실행파일 이름은 제거한다.
	int	Length = (int)lstrlen(Path);

	for (int i = Length - 1; i >= 0; --i)
	{
		if (Path[i] == '\\' || Path[i] == '/')
		{
			memset(&Path[i + 1], 0, sizeof(TCHAR) * (Length - i - 1));
			break;
		}
	}

	mPathMap.insert(std::make_pair("Root", Path));

	CreatePath("Asset", TEXT("Asset\\"));
	CreatePath("Shader", TEXT("Shader\\"), "Asset");
	CreatePath("Texture", TEXT("Texture\\"), "Asset");
	CreatePath("EngineAsset", TEXT("EngineAsset\\"));
	CreatePath("EngineShader", TEXT("Shader\\"), "EngineAsset");
	CreatePath("EngineTexture", TEXT("Texture\\"), "EngineAsset");

	return true;
}

bool CPathManager::CreatePath(const std::string& Name, const TCHAR* Path, const std::string& BasePathName)
{
	if (FindPath(Name))
		return false;

	// BasePath를 얻어온다.
	const TCHAR* BasePath = FindPath(BasePathName);

	if (!BasePath)
		return false;

	TCHAR* FullPath = new TCHAR[MAX_PATH];

	memset(FullPath, 0, sizeof(TCHAR) * MAX_PATH);

	// 베이스 경로를 복사한다.
	lstrcpy(FullPath, BasePath);
	// 인자로 들어온 패스를 붙여준다.
	lstrcat(FullPath, Path);

	mPathMap.insert(std::make_pair(Name, FullPath));

	return true;
}

const TCHAR* CPathManager::FindPath(const std::string& Name)
{
	auto iter = mPathMap.find(Name);

	if (iter == mPathMap.end())
		return nullptr;

	return iter->second;
}
