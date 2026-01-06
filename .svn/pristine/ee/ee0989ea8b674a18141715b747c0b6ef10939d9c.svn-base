#include "Object.h"

std::unordered_map<size_t, std::shared_ptr<CObject>> CObject::mCDOMap;

CObject::CObject()
{
}

CObject::CObject(const CObject& ref)
{
}

CObject::CObject(CObject&& ref)	noexcept
{
}

CObject::~CObject()
{
}

void CObject::DestroyCDO()
{
	mCDOMap.clear();
}
