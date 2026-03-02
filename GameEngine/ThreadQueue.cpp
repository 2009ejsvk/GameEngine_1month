#include "ThreadQueue.h"

CThreadQueue::CThreadQueue()
{
	InitializeCriticalSection(&mCrt);
}

CThreadQueue::~CThreadQueue()
{
	DeleteCriticalSection(&mCrt);
}

bool CThreadQueue::push(int Header, int Size, unsigned char* Data)
{
	CSync	sync(&mCrt);

	if (mSize >= THREAD_QUEUE_CAPACITY)
		return false;

	if (Size < 0 || Size > THREAD_QUEUE_DATA_SIZE)
		return false;

	mPush = (mPush + 1) % THREAD_QUEUE_CAPACITY;

	mData[mPush].Header = Header;
	mData[mPush].Size = Size;

	if (Data)
		memcpy(mData[mPush].Data, Data, Size);

	++mSize;
	return true;
}

void CThreadQueue::pop(int& Header, int& Size, unsigned char* Data)
{
	CSync	sync(&mCrt);

	if (mSize == 0)
		return;

	mPop = (mPop + 1) % THREAD_QUEUE_CAPACITY;

	Header = mData[mPop].Header;
	Size = mData[mPop].Size;

	if (Size > 0)
		memcpy(Data, mData[mPop].Data, Size);

	--mSize;
}

int CThreadQueue::size()
{
	CSync	sync(&mCrt);

	return mSize;
}

bool CThreadQueue::full()
{
	CSync	sync(&mCrt);

	return mSize >= THREAD_QUEUE_CAPACITY;
}

bool CThreadQueue::empty()
{
	CSync	sync(&mCrt);

	return mSize == 0;
}
