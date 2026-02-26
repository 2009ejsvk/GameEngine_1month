#include "Tile.h"

CTile::CTile()
{
}

CTile::CTile(const CTile& ref)
{
}

CTile::CTile(CTile&& ref) noexcept
{
}

CTile::~CTile()
{
}

void CTile::Save(FILE* File)
{
	fwrite(&mType, sizeof(ETileType), 1, File);
	fwrite(&mPos, sizeof(FVector2), 1, File);
	fwrite(&mSize, sizeof(FVector2), 1, File);
	fwrite(&mCenter, sizeof(FVector2), 1, File);
	fwrite(&mTextureFrame, sizeof(int), 1, File);
	fwrite(&mFrameStart, sizeof(FVector2), 1, File);
	fwrite(&mFrameEnd, sizeof(FVector2), 1, File);

	fwrite(&mRender, sizeof(bool), 1, File);
	fwrite(&mOutLineRender, sizeof(bool), 1, File);

	fwrite(&mOutLineColor, sizeof(FVector4), 1, File);

	fwrite(&mIndexX, sizeof(int), 1, File);
	fwrite(&mIndexY, sizeof(int), 1, File);
	fwrite(&mIndex, sizeof(int), 1, File);
}

void CTile::Load(FILE* File)
{
	fread(&mType, sizeof(ETileType), 1, File);
	fread(&mPos, sizeof(FVector2), 1, File);
	fread(&mSize, sizeof(FVector2), 1, File);
	fread(&mCenter, sizeof(FVector2), 1, File);
	fread(&mTextureFrame, sizeof(int), 1, File);
	fread(&mFrameStart, sizeof(FVector2), 1, File);
	fread(&mFrameEnd, sizeof(FVector2), 1, File);

	fread(&mRender, sizeof(bool), 1, File);
	fread(&mOutLineRender, sizeof(bool), 1, File);

	fread(&mOutLineColor, sizeof(FVector4), 1, File);

	fread(&mIndexX, sizeof(int), 1, File);
	fread(&mIndexY, sizeof(int), 1, File);
	fread(&mIndex, sizeof(int), 1, File);
}
