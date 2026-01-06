#include "Texture.h"
#include "../PathManager.h"
#include "../../Device.h"

CTexture::CTexture()
{
	mAssetType = EAssetType::Texture;
}

CTexture::~CTexture()
{
	size_t	Size = mTextureList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		SAFE_DELETE(mTextureList[i]);
	}
}

bool CTexture::LoadTexture(const TCHAR* FileName, 
	const std::string& PathName)
{
	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR	FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	return LoadTexture(FullPath);
}

bool CTexture::LoadTexture(const TCHAR* FullPath)
{
	FTextureInfo* Texture = new FTextureInfo;

	// 파일 확장자마다 로딩하는 함수가 다르다.
	// 1. .dds 2. .tga 3. 그 외의 파일확장자
	// 전체 경로에서 파일 확장자를 얻어온다.
	TCHAR	FileExt[_MAX_EXT] = {};

	// 파일 확장자를 얻어온다.
	// _wsplitpath_s 함수는 경로를 나누어주는 함수이다.
	_wsplitpath_s(FullPath, nullptr, 0, nullptr, 0, nullptr, 0,
		FileExt, _MAX_EXT);

	// 위에서 얻어온 유니코드 문자열 파일 확장자를 멀티바이트 문자열로
	// 변경한다.
	char	ConvertExt[_MAX_EXT] = {};
	// 멀티바이트로 변경하기 위한 길이를 얻어온다.
	int	Count = WideCharToMultiByte(CP_ACP, 0, FileExt, -1,
		nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FileExt, -1, ConvertExt, Count,
		nullptr, nullptr);

	// 확장자를 무조건 대문자로 비교하기 위해 대문자로 변경한다.
	_strupr_s(ConvertExt);

	// 픽셀정보를 저장하기 위한 객체를 생성한다.
	DirectX::ScratchImage* Image = new DirectX::ScratchImage;

	// 확장자 체크
	if (strcmp(ConvertExt, ".DDS") == 0)
	{
		if (FAILED(DirectX::LoadFromDDSFile(FullPath,
			DirectX::DDS_FLAGS_NONE, nullptr, *Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);
			return false;
		}
	}

	else if (strcmp(ConvertExt, ".TGA") == 0)
	{
		if(FAILED(DirectX::LoadFromTGAFile(FullPath, nullptr,
			*Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);
			return false;
		}
	}

	else
	{
		if(FAILED(DirectX::LoadFromWICFile(FullPath,
			DirectX::WIC_FLAGS_NONE, nullptr, *Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);
			return false;
		}
	}

	Texture->Image = Image;

	mTextureList.push_back(Texture);

	// 로딩한 이미지 정보를 이용해서 출력용 ShaderResourceView를
	// 생성한다.
	return CreateResourceView((int)mTextureList.size() - 1);
}

bool CTexture::LoadTexture(const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	size_t	Size = FileName.size();

	for(size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FileName[i], PathName))
			return false;
	}

	return true;
}

bool CTexture::LoadTexture(const std::vector<const TCHAR*> FullPath)
{
	size_t	Size = FullPath.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FullPath[i]))
			return false;
	}

	return true;
}

void CTexture::SetShader(int Register, int ShaderBufferType,
	int TextureIndex)
{
	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}

	// PixelShader
	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}

	// DomainShader
	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}

	// HullShader
	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}

	// GeometryShader
	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}

	// ComputeShader
	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetShaderResources(
			Register, 1, &mTextureList[TextureIndex]->SRV);
	}
}

bool CTexture::CreateResourceView(int Index)
{
	if (FAILED(DirectX::CreateShaderResourceView(
		CDevice::GetInst()->GetDevice(),
		mTextureList[Index]->Image->GetImages(),
		mTextureList[Index]->Image->GetImageCount(),
		mTextureList[Index]->Image->GetMetadata(),
		&mTextureList[Index]->SRV)))
	{
		return false;
	}

	mTextureList[Index]->Width = (unsigned int)mTextureList[Index]->Image->GetImages()[0].width;
	mTextureList[Index]->Height = (unsigned int)mTextureList[Index]->Image->GetImages()[0].height;

	return true;
}
