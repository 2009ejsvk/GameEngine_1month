#include "Widget.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Shader/ShaderManager.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Mesh/MeshManager.h"
#include "../World/World.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"

FMatrix CWidget::mUIProjMatrix;

bool CWidget::SortRender(const std::shared_ptr<CWidget>& Src,
	const std::shared_ptr<CWidget>& Dest)
{
	return Src->GetZOrder() < Dest->GetZOrder();
}

bool CWidget::SortCollision(const std::shared_ptr<CWidget>& Src,
	const std::shared_ptr<CWidget>& Dest)
{
	return Src->GetZOrder() > Dest->GetZOrder();
}

CWidget::CWidget()
{
}

CWidget::CWidget(const CWidget& ref)
{
	mEnable = ref.mEnable;
	mAlive = ref.mAlive;
	mWorld = ref.mWorld;
	mUIManager = ref.mUIManager;
	mShader = ref.mShader;
	mMesh = ref.mMesh;

	mTransformCBuffer.reset(new CCBufferTransform);

	mTransformCBuffer->Init();

	mUIDefaultCBuffer.reset(new CCBufferUIDefault);

	mUIDefaultCBuffer->Init();

	mName = ref.mName;
	mPos = ref.mPos;
	mRenderPos = ref.mRenderPos;
	mSize = ref.mSize;
	mPivot = ref.mPivot;
	mAngle = ref.mAngle;
	mZOrder = ref.mZOrder;
	mPrevZOrder = ref.mPrevZOrder;
	mWidgetColor = ref.mWidgetColor;
}

CWidget::~CWidget()
{
}

void CWidget::SetParentAll()
{
}

void CWidget::SetShader(const std::weak_ptr<class CShader>& Shader)
{
	mShader = Shader;
}

void CWidget::SetShader(const std::string& Name)
{
	std::shared_ptr<CShaderManager>   ShaderMgr =
		CAssetManager::GetInst()->GetShaderManager().lock();

	mShader = ShaderMgr->FindShader(Name);
}

bool CWidget::Init()
{
	auto	World = mWorld.lock();

	std::weak_ptr<CMesh>	Mesh;

	if (World)
	{
		auto	AssetMgr = World->GetWorldAssetManager().lock();

		Mesh = AssetMgr->FindMesh("UIRectTex");

		if (Mesh.expired())
			return false;

		mMesh = Mesh;
	}

	else
	{
		std::weak_ptr<CMeshManager> Weak_MeshMgr =
			CAssetManager::GetInst()->GetMeshManager();

		std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

		mMesh = MeshMgr->FindMesh("Mesh_UIRectTex");
	}

	SetShader("UIDefault");

	mTransformCBuffer.reset(new CCBufferTransform);

	mTransformCBuffer->Init();

	mUIDefaultCBuffer.reset(new CCBufferUIDefault);

	mUIDefaultCBuffer->Init();

	return true;
}

void CWidget::Update(float DeltaTime)
{
}

void CWidget::Render()
{
	auto	Parent = mParent.lock();

	if (Parent)
	{
		mRenderPos = Parent->mRenderPos + mPos;
	}

	else
	{
		mRenderPos = mPos;
	}

	mUIDefaultCBuffer->SetWidgetColor(mWidgetColor);
}

void CWidget::Destroy()
{
	mAlive = false;
}

bool CWidget::CollisionMouse(std::weak_ptr<CWidget>& Result, 
	const FVector2& MousePos)
{
	auto	Parent = mParent.lock();

	if (Parent)
	{
		mRenderPos = Parent->mRenderPos + mPos;
	}

	else
	{
		mRenderPos = mPos;
	}

	// 회전이 없을 경우
	if (mAngle == 0.f)
	{
		FVector3	Min = mRenderPos - mSize * mPivot;
		FVector3	Max = Min + mSize;

		if (MousePos.x < Min.x)
		{
			if (mMouseOn)
			{
				mMouseOn = false;
				MouseUnHovered();
			}

			return false;
		}

		else if (MousePos.x > Max.x)
		{
			if (mMouseOn)
			{
				mMouseOn = false;
				MouseUnHovered();
			}

			return false;
		}

		else if (MousePos.y < Min.y)
		{
			if (mMouseOn)
			{
				mMouseOn = false;
				MouseUnHovered();
			}

			return false;
		}

		else if (MousePos.y > Max.y)
		{
			if (mMouseOn)
			{
				mMouseOn = false;
				MouseUnHovered();
			}

			return false;
		}

		Result = mSelf;
		mMouseOn = true;
		MouseHovered();

		return true;
	}

	// 회전이 있을 경우 OBB로 처리한다.
	FVector3	Axis[2];
	FVector3	HalfSize = mSize * 0.5f;
	FVector3	Center = mRenderPos - mSize * mPivot + HalfSize;

	Axis[0] = FVector3::Axis[EAxis::X];
	Axis[1] = FVector3::Axis[EAxis::Y];

	// 축을 각도만큼 회전시킨다.
	FMatrix	RotationMat;
	RotationMat.RotationZ(mAngle);

	// 축을 회전행렬에 곱해서 회전시킨다.
	Axis[0] = Axis[0].TransformNormal(RotationMat);
	Axis[1] = Axis[1].TransformNormal(RotationMat);

	Axis[0].Normalize();
	Axis[1].Normalize();

	FVector3	ConvertMousePos;
	ConvertMousePos.x = MousePos.x;
	ConvertMousePos.y = MousePos.y;

	FVector3	CenterLine = Center - ConvertMousePos;

	float	Dist = abs(CenterLine.Dot(Axis[0]));

	if (Dist > HalfSize.x)
	{
		if (mMouseOn)
		{
			mMouseOn = false;
			MouseUnHovered();
		}

		return false;
	}

	Dist = abs(CenterLine.Dot(Axis[1]));

	if (Dist > HalfSize.y)
	{
		if (mMouseOn)
		{
			mMouseOn = false;
			MouseUnHovered();
		}

		return false;
	}

	Result = mSelf;
	mMouseOn = true;
	MouseHovered();

	return true;
}

void CWidget::MouseHovered()
{
}

void CWidget::MouseUnHovered()
{
}

bool CWidget::MouseDragStart(const FVector2& MousePos,
	std::shared_ptr<CWidget>& DragOperator)
{
	return false;
}

bool CWidget::MouseDrag(const FVector2& MousePos,
	const FVector2& MouseMove)
{
	return false;
}

bool CWidget::MouseDragEnd(const FVector2& MousePos)
{
	return false;
}

void CWidget::RenderBrush(const FUIBrush& Brush, 
	const FVector3& RenderPos, const FVector3& Size)
{
	std::shared_ptr<CShader>    Shader = mShader.lock();
	std::shared_ptr<CMesh>    Mesh = mMesh.lock();

	FMatrix	ScaleMat, RotationMat, TranslateMat, WorldMat;

	ScaleMat.Scaling(Size);
	RotationMat.RotationZ(mAngle);
	TranslateMat.Translation(RenderPos);

	WorldMat = ScaleMat * RotationMat * TranslateMat;

	auto	World = mWorld.lock();

	mTransformCBuffer->SetWorldMatrix(WorldMat);
	mTransformCBuffer->SetViewMatrix(FMatrix::IdentityMatrix);
	mTransformCBuffer->SetProjMatrix(mUIProjMatrix);

	FVector3	PivotSize = mPivot * Mesh->GetMeshSize();

	mTransformCBuffer->SetPivotSize(PivotSize);

	mTransformCBuffer->UpdateBuffer();

	mUIDefaultCBuffer->SetBrusnTint(Brush.Tint);

	FVector2	TexSize;

	if (!Brush.Texture.expired())
	{
		mUIDefaultCBuffer->SetTextureEnable(true);

		auto	Texture = Brush.Texture.lock();

		TexSize.x = (float)Texture->GetTexture()->Width;
		TexSize.y = (float)Texture->GetTexture()->Height;

		if (!Brush.FrameAnimationEnable)
			Texture->SetShader(0, EShaderBufferType::Pixel, 0);

		else
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, Brush.Frame);
		}
	}

	else
	{
		mUIDefaultCBuffer->SetTextureEnable(false);
	}

	if (Brush.AnimationEnable)
	{
		mUIDefaultCBuffer->SetAnimEnable(true);

		int	Frame = Brush.Frame;

		FTextureFrame	FrameInfo = Brush.AnimationFrames[Frame];

		mUIDefaultCBuffer->SetBrushLTUV(FrameInfo.Start / TexSize);

		FVector2	FrameRB;
		FrameRB.x = FrameInfo.Start.x + FrameInfo.Size.x;
		FrameRB.y = FrameInfo.Start.y + FrameInfo.Size.y;

		mUIDefaultCBuffer->SetBrushRBUV(FrameRB / TexSize);
	}

	else
	{
		mUIDefaultCBuffer->SetAnimEnable(false);
	}

	mUIDefaultCBuffer->UpdateBuffer();

	Shader->SetShader();

	Mesh->Render();
}

void CWidget::RenderBrush(const FUIBrush& Brush, 
	const FVector3& RenderPos, const FVector3& Size, 
	const FVector2& FrameStart,
	const FVector3& FrameSize)
{
	std::shared_ptr<CShader>    Shader = mShader.lock();
	std::shared_ptr<CMesh>    Mesh = mMesh.lock();

	FMatrix	ScaleMat, TranslateMat, WorldMat;

	ScaleMat.Scaling(Size);
	TranslateMat.Translation(RenderPos);

	WorldMat = ScaleMat * TranslateMat;

	auto	World = mWorld.lock();

	mTransformCBuffer->SetWorldMatrix(WorldMat);
	mTransformCBuffer->SetViewMatrix(FMatrix::IdentityMatrix);
	mTransformCBuffer->SetProjMatrix(mUIProjMatrix);

	//FVector3	PivotSize = mPivot * Mesh->GetMeshSize();

	//mTransformCBuffer->SetPivotSize(PivotSize);

	mTransformCBuffer->UpdateBuffer();

	mUIDefaultCBuffer->SetBrusnTint(Brush.Tint);

	FVector2	TexSize;

	if (!Brush.Texture.expired())
	{
		mUIDefaultCBuffer->SetTextureEnable(true);

		auto	Texture = Brush.Texture.lock();

		TexSize.x = (float)Texture->GetTexture()->Width;
		TexSize.y = (float)Texture->GetTexture()->Height;

		if (!Brush.FrameAnimationEnable)
			Texture->SetShader(0, EShaderBufferType::Pixel, 0);

		else
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, Brush.Frame);
		}
	}

	else
	{
		mUIDefaultCBuffer->SetTextureEnable(false);
	}

	mUIDefaultCBuffer->SetAnimEnable(true);
	mUIDefaultCBuffer->SetBrushLTUV(FrameStart / TexSize);

	FVector2	FrameRB;
	FrameRB.x = FrameStart.x + FrameSize.x;
	FrameRB.y = FrameStart.y + FrameSize.y;

	mUIDefaultCBuffer->SetBrushRBUV(FrameRB / TexSize);

	mUIDefaultCBuffer->UpdateBuffer();

	Shader->SetShader();

	Mesh->Render();
}
