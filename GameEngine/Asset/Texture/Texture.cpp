#include "Texture.h"
#include "../PathManager.h"
#include "../../Device.h"
#include "TextureManager.h"

CTexture::CTexture()
{
	mAssetType = EAssetType::Texture;
}

CTexture::~CTexture()
{
	SAFE_RELEASE(mArraySRV);

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
	Texture->FullPath = FullPath;

	mTextureList.push_back(Texture);

	// 로딩한 이미지 정보를 이용해서 출력용 ShaderResourceView를
	// 생성한다.
	return CreateResourceView((int)mTextureList.size() - 1);
}

bool CTexture::LoadTexture(const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	mTextureType = ETextureType::Frame;

	size_t	Size = FileName.size();

	for(size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FileName[i], PathName))
			return false;
	}

	return true;
}

bool CTexture::LoadTexture(
	const std::vector<const TCHAR*>& FullPath)
{
	mTextureType = ETextureType::Frame;

	size_t	Size = FullPath.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FullPath[i]))
			return false;
	}

	return true;
}

bool CTexture::LoadTextureArray(
	const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	mTextureType = ETextureType::Array;

	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	std::vector<const TCHAR*>	FullPathArray;

	size_t	Size = FileName.size();

	for (size_t i = 0; i < Size; ++i)
	{
		TCHAR* FullPath = new TCHAR[MAX_PATH];

		lstrcpy(FullPath, BasePath);
		lstrcat(FullPath, FileName[i]);

		FullPathArray.push_back(FullPath);
	}
	
	bool Result = LoadTextureArray(FullPathArray);

	Size = FullPathArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		SAFE_DELETE_ARRAY(FullPathArray[i]);
	}

	return Result;
}

bool CTexture::LoadTextureArray(
	const std::vector<const TCHAR*>& FullPath)
{
	mTextureType = ETextureType::Array;

	size_t	Size = FullPath.size();

	mArrayCount = (int)Size;

	for (size_t i = 0; i < Size; ++i)
	{
		FTextureInfo* Texture = new FTextureInfo;

		TCHAR	FileExt[_MAX_EXT] = {};

		_wsplitpath_s(FullPath[i], nullptr, 0, nullptr, 0,
			nullptr, 0, FileExt, _MAX_EXT);

		char	ConvertExt[_MAX_EXT] = {};
		// 멀티바이트로 변경하기 위한 길이를 얻어온다.
		int	Count = WideCharToMultiByte(CP_ACP, 0, FileExt, -1,
			nullptr, 0, nullptr, nullptr);
		WideCharToMultiByte(CP_ACP, 0, FileExt, -1,
			ConvertExt, Count, nullptr, nullptr);

		// 확장자를 무조건 대문자로 비교하기 위해 대문자로 변경한다.
		_strupr_s(ConvertExt);

		// 픽셀정보를 저장하기 위한 객체를 생성한다.
		DirectX::ScratchImage* Image = new DirectX::ScratchImage;

		// 확장자 체크
		if (strcmp(ConvertExt, ".DDS") == 0)
		{
			if (FAILED(DirectX::LoadFromDDSFile(FullPath[i],
				DirectX::DDS_FLAGS_NONE, nullptr, *Image)))
			{
				SAFE_DELETE(Texture);
				SAFE_DELETE(Image);
				return false;
			}
		}

		else if (strcmp(ConvertExt, ".TGA") == 0)
		{
			if (FAILED(DirectX::LoadFromTGAFile(FullPath[i],
				nullptr, *Image)))
			{
				SAFE_DELETE(Texture);
				SAFE_DELETE(Image);
				return false;
			}
		}

		else
		{
			if (FAILED(DirectX::LoadFromWICFile(FullPath[i],
				DirectX::WIC_FLAGS_NONE, nullptr, *Image)))
			{
				SAFE_DELETE(Texture);
				SAFE_DELETE(Image);
				return false;
			}
		}

		Texture->Image = Image;
		Texture->FullPath = FullPath[i];

		Texture->Width = (unsigned int)Image->GetImages()[0].width;
		Texture->Height = (unsigned int)Image->GetImages()[0].height;

		mTextureList.push_back(Texture);
	}

	// 로딩한 이미지 정보를 이용해서 출력용 ShaderResourceView를
	// 생성한다.
	return CreateResourceViewArray();
}

void CTexture::SetShader(int Register, int ShaderBufferType,
	int TextureIndex)
{
	ID3D11ShaderResourceView* SRV =
		mTextureList[TextureIndex]->SRV;

	if (mTextureType == ETextureType::Array)
	{
		SRV = mArraySRV;
	}

	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetShaderResources(
			Register, 1, &SRV);
	}

	// PixelShader
	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetShaderResources(
			Register, 1, &SRV);
	}

	// DomainShader
	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetShaderResources(
			Register, 1, &SRV);
	}

	// HullShader
	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetShaderResources(
			Register, 1, &SRV);
	}

	// GeometryShader
	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetShaderResources(
			Register, 1, &SRV);
	}

	// ComputeShader
	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetShaderResources(
			Register, 1, &SRV);
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

bool CTexture::CreateResourceViewArray()
{
	DirectX::ScratchImage* ImageArray = 
		new DirectX::ScratchImage;

	size_t	MipLevel = mTextureList[0]->Image->GetMetadata().mipLevels;
	size_t	Count = mTextureList.size();

	if (FAILED(ImageArray->Initialize2D(
		mTextureList[0]->Image->GetMetadata().format,
		(size_t)mTextureList[0]->Width,
		(size_t)mTextureList[0]->Height,
		Count, MipLevel)))
		return false;

	for (size_t i = 0; i < Count; ++i)
	{
		const DirectX::Image* Images = mTextureList[i]->Image->GetImages();

		MipLevel = mTextureList[i]->Image->GetMetadata().mipLevels;

		for (size_t j = 0; j < MipLevel; ++j)
		{
			const DirectX::Image* Src = &ImageArray->GetImages()[i * MipLevel + j];
			const DirectX::Image* Dest = &Images[j];

			// 픽셀 복사
			memcpy(Src->pixels, Dest->pixels, Src->slicePitch);
		}

		CreateResourceView((int)i);
	}

	ID3D11Device* Device = CDevice::GetInst()->GetDevice();

	ID3D11Texture2D* Texture = nullptr;

	if (FAILED(DirectX::CreateTextureEx(
		Device,
		ImageArray->GetImages(),
		ImageArray->GetImageCount(),
		ImageArray->GetMetadata(),
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
		0, 0, DirectX::CREATETEX_FLAGS::CREATETEX_DEFAULT,
		(ID3D11Resource**)&Texture)))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC	Desc = {};

	Desc.Format = mTextureList[0]->Image->GetMetadata().format;
	Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	Desc.Texture2DArray.MostDetailedMip = 0;
	Desc.Texture2DArray.MipLevels =
		mTextureList[0]->Image->GetMetadata().mipLevels;
	Desc.Texture2DArray.FirstArraySlice = 0;
	Desc.Texture2DArray.ArraySize = (UINT)Count;

	if (FAILED(Device->CreateShaderResourceView(Texture, &Desc,
		&mArraySRV)))
		return false;

	SAFE_DELETE(ImageArray);

	SAFE_RELEASE(Texture);

	return true;
}

void CTexture::Save(FILE* File)
{
	size_t	Size = mTextureList.size();

	fwrite(&Size, sizeof(size_t), 1, File);

	for (size_t i = 0; i < Size; ++i)
	{
		size_t	Count = mTextureList[i]->FullPath.length();

		fwrite(&Count, sizeof(size_t), 1, File);
		fwrite(mTextureList[i]->FullPath.c_str(),
			sizeof(wchar_t), Count, File);
	}
}
