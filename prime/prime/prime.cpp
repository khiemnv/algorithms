// prime.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <intrin.h>
#include <crtdbg.h>
#include <math.h>

#define assert(expr) _ASSERT(expr)

#define block_len 8
#define getBlockIndex(iBit) ((iBit) >> 3)
#define getNumber(offset, iBlock,iBit) ( (offset) + ((((iBlock) << 3) | (iBit))<<1))
#define getRemain(iBit) ( (iBit) & 0x07 )

class myLst
{
public:
	UINT64 count;
	UINT64 lastPrime = 0;
private:
	UINT64* mBuff = 0;
	UINT64 mBuffSize = 0;
	UINT64 mMinIndex = 0;
	UINT64 mMaxIndex = 0;

	//file map
	HANDLE mFileHandle = 0;
	HANDLE mMapHandle = 0;
	void* mMapAddr = 0;
	UINT64 mFileOffset = 0;
	UINT64 mMaxViewSize = 1024*1024*1024;
	UINT64 mMaxViewCount = 8;
	bool mUseFileMap = false;
public:
	UINT64 mMoveViewCount =0;	//for debug
public:
	myLst(void* buff, UINT64 buffSize) {
		count = 0;
		if (buff == NULL) {
			void* mapAddr = NULL;
			int rc = MapFile(&mapAddr);
			if (rc != 0) {
				DisplayError(TEXT("myLst()"), rc);
				return;
			}
			mBuffSize = mMaxViewCount * mMaxViewSize;
			mBuff = (UINT64*)mapAddr;
			mUseFileMap = true;
		}
		else {
			mBuffSize = buffSize;
			mBuff = (UINT64*)buff;
			mMinIndex = 0;
			mMaxIndex = (mBuffSize >> 3) - 1;
		}

		//load list data
		UINT64 arr[] = {0, 0, 5,7,11,13,17,19};
		int i;
		for (i = 2; i < _countof(arr); i++) {
			if (mBuff[i] != arr[i]) {
				break;
			}
		}
		if (i == _countof(arr)) {
			count = mBuff[0];
			lastPrime = mBuff[1];
#if 0
			//manual fix error
			if ((count == 2) && (lastPrime == 3)) {
				count = 411523195;
				lastPrime = 8999999993;
			}
#endif
			mBuff[0] = 2;
			mBuff[1] = 3;
		}
	}
	~myLst() {
		assert(count <= (mMaxIndex+1));
		if ((mMaxIndex+1) < count) {
			int rc = moveView(count-1);
			if (rc != 0) {
				DisplayError(TEXT("~myLst()"), rc);
			}
		}
		lastPrime = mBuff[count - 1 - mMinIndex];

		if (mMinIndex != 0) {
			int rc = moveView(0);
			if (rc != 0) {
				DisplayError(TEXT("~myLst()"), rc);
			}
		}
		mBuff[0] = count;
		mBuff[1] = lastPrime;

		unMapFile();
	}
	UINT64& operator[] (UINT64 index) {
		if (index == count) {
			count++;
		}
		assert(index < count);
		if ((index < mMinIndex) || (index > mMaxIndex)) {
			int rc = moveView(index);
			if (rc != 0) {
				DisplayError(TEXT("operator[]()"),rc);
			}
		}
		UINT64 rIndex = index - mMinIndex;
		return mBuff[rIndex];
	}
	void add(UINT64 num) {
		assert(count < (mBuffSize >> 3));
		if (count > mMaxIndex) {
			int rc = moveView(count);
			if (rc != 0) {
				DisplayError(TEXT("add()"), rc);
			}
		}
		UINT64 rIndex = count - mMinIndex;
		mBuff[rIndex] = num;
		count++;
	}
	UINT64 findIndex(UINT64 num) {
		UINT64 iBegin = 0;
		while (num < mBuff[0]) {
			assert(mMinIndex > 0);
			int rc = moveView(mMinIndex - 1);
			if (rc != 0) {
				DisplayError(TEXT("find()"),rc);
			}
		}
		UINT64 iEnd;
		iEnd = min(count - 1, mMaxIndex) - mMinIndex;
		while (num > mBuff[iEnd]) {
			assert(mMaxIndex < count);
			int rc = moveView(mMaxIndex + 1);
			if (rc != 0) {
				DisplayError(TEXT("find()"), rc);
			}
			iEnd = min(count - 1, mMaxIndex) - mMinIndex;
		}
		UINT64 iMid;
		while ((iEnd - iBegin) > 1) {
			iMid = (iEnd + iBegin) / 2;
			if (num < mBuff[iMid]) {
				iEnd = iMid;
			}
			else {
				iBegin = iMid;
			}
		}
		return (iEnd + mMinIndex);
	}
private:
	void DisplayError(TCHAR* pszAPI, DWORD dwError)
	{
		LPVOID lpvMessageBuffer;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvMessageBuffer, 0, NULL);

		//... now display this string
		_tprintf(TEXT("ERROR: API        = %s\n"), pszAPI);
		_tprintf(TEXT("       error code = %d\n"), dwError);
		_tprintf(TEXT("       message    = %s\n"), (LPTSTR)lpvMessageBuffer);

		// Free the buffer allocated by the system
		LocalFree(lpvMessageBuffer);

		ExitProcess(GetLastError());
	}
	int moveView(UINT64 index) {
		assert(mUseFileMap);
		int rc;
		UINT64 newFileOffset = mFileOffset;
		UINT64 newMinIndex = mMinIndex;
		UINT64 newMaxIndex = mMaxIndex;
		for (; index < newMinIndex;) {
			newFileOffset -= mMaxViewSize;
			newMinIndex = newFileOffset >> 3;
			newMaxIndex = ((newFileOffset + mMaxViewSize) >> 3) - 1;
		}
		for (; index > newMaxIndex;) {
			newFileOffset += mMaxViewSize;
			newMinIndex = newFileOffset >> 3;
			newMaxIndex = ((newFileOffset + mMaxViewSize) >> 3) - 1;
		}

		rc = UnmapViewOfFile(mMapAddr);
		assert(rc);

		LARGE_INTEGER mapFileOffset;
		mapFileOffset.QuadPart = newFileOffset;
		assert((mMaxViewSize >> 32) == 0);
		mMapAddr = MapViewOfFile(
			mMapHandle,
			FILE_MAP_ALL_ACCESS,
			mapFileOffset.HighPart,
			mapFileOffset.LowPart,
			(UINT32)mMaxViewSize
		);
		if (mMapAddr != NULL) {
			mBuff = (UINT64*)mMapAddr;
			mFileOffset = newFileOffset;
			mMinIndex = newMinIndex;
			mMaxIndex = newMaxIndex;

			rc = 0;
		}
		else {
			rc = GetLastError();
		}

		mMoveViewCount++;	//for debug
		return rc;
	}
	void unMapFile() {
		if (mMapAddr) {
			UnmapViewOfFile(mMapAddr);
			mMapAddr = 0;
		}
		if (mMapHandle) {
			CloseHandle(mMapHandle);
			mMapHandle = 0;
		}
		if (mFileHandle) {
			CloseHandle(mFileHandle);
			mFileHandle = 0;
		}
	}
	//if success func return 0
	int MapFile(void** pMadAddr)
	{
		int rc;

#if 1
		TCHAR* zFile = TEXT("tmp.dat");

		HANDLE hFile;
		hFile = CreateFile(
			zFile,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			0
		);
		if (hFile != INVALID_HANDLE_VALUE) {
			mFileHandle = hFile;
			rc = 0;
		}
		else {
			rc = GetLastError();
		}
#endif
		LARGE_INTEGER mapSize;
		HANDLE hMap;
		void* mapAddr;

		mapSize.QuadPart = mMaxViewCount*mMaxViewSize;
		hMap = CreateFileMapping(
			//INVALID_HANDLE_VALUE,
			hFile,
			NULL,
			PAGE_READWRITE, 
			mapSize.HighPart, mapSize.LowPart,
			NULL
		);
		if (hMap != NULL) {
			LARGE_INTEGER fileOffset;
			fileOffset.QuadPart = 0;
			assert((mMaxViewSize >> 32) == 0);
			mapAddr = MapViewOfFile(
				hMap,
				FILE_MAP_ALL_ACCESS,
				fileOffset.HighPart,
				fileOffset.LowPart,
				(UINT32)mMaxViewSize
			);
			if (mapAddr != NULL) {
				mMapHandle = hMap;
				mMapAddr = mapAddr;
				*pMadAddr = mapAddr;

				//for move view
				mFileOffset = 0;
				mMinIndex = mFileOffset >> 3;
				mMaxIndex = ((mFileOffset + mMaxViewSize) >> 3) - 1;
				rc = 0;
			}
			else {
				rc = GetLastError();
				CloseHandle(hMap);
			}
		}		
		else {
			rc = GetLastError();
		}
		return rc;
	}
};

class myDict {
private:
	UINT8 * buff;
	UINT32 buffSize;
	UINT64 offset;
	UINT64 iFirst;
	UINT64 iLast;
public:
	UINT64 count;
public:
	myDict(void* buff, UINT32 buffSize) {
		this->buff = (UINT8*)buff;
		this->buffSize = buffSize;
		count = 0;
		iFirst = 0;
		iLast = 0;
		offset = 0;
	}
	bool fill(UINT64 minNum, UINT64 maxNum) {
		offset = minNum;
		iFirst = 0;
		iLast = (maxNum - minNum) >> 1;
		UINT64 remain = getRemain(iLast);
		UINT64 maxIndex = getBlockIndex(iLast);

		bool ret = true;
		if (maxIndex >= buffSize) {
			maxIndex = buffSize - 1;
			remain = block_len - 1;
			iLast = (maxIndex << 3) + remain;
			ret = false;
		}

		//init blocks
		assert((maxIndex >>32) == 0);
		for (UINT32 iBlock = 0; iBlock < maxIndex; iBlock++) {
			buff[iBlock] = 0xff;
		}
		UINT8 blockValue = 1;
		for (UINT8 iBit = 0; iBit < remain; iBit++) {
			blockValue = (blockValue << 1) | 1;
		}
		buff[maxIndex] = blockValue;

		count = iLast + 1;
		return ret;
	}
	UINT64 pop(UINT64* list, UINT64 listLen) {
		UINT32 iBlock = 0;
		UINT64 num;
		UINT64 i;
		UINT8 iBit;
		UINT8 blockValue;
		for (i = 0; i < count; iBlock++) {
			blockValue = buff[iBlock];
			for (iBit = 0;blockValue != 0; iBit++) {
				if ((blockValue) & 1) {
					num = getNumber(offset, (UINT64)iBlock, iBit);
					list[listLen + i] = num;
					i++;
				}
				blockValue >>= 1;
			}
		}
		count = 0;
		return i;
	}
	UINT64 pop(myLst& list, UINT64 listLen) {
		UINT32 iBlock = 0;
		UINT64 num;
		UINT64 i;
		UINT8 iBit;
		UINT8 blockValue;
		for (i = 0; i < count; iBlock++) {
			blockValue = buff[iBlock];
			for (iBit = 0; blockValue != 0; iBit++) {
				if ((blockValue) & 1) {
					num = getNumber(offset, (UINT64)iBlock, (UINT64)iBit);
					list[listLen + i] = num;
					i++;
				}
				blockValue >>= 1;
			}
		}
		count = 0;
		return i;
	}
	void remove(UINT64 num) {
		UINT64 iBit = (num - offset) >> 1;
		UINT64 remain = getRemain(iBit);
		UINT64 iBlock = getBlockIndex(iBit);
		UINT8 blockValue = buff[iBlock];
		UINT8 flag = 0x01 << remain;
		assert(blockValue & flag);
		buff[iBlock] = blockValue ^ flag;
		count--;
	}
	UINT64 getMinNum() {
		return offset;
	}
	UINT64 getMaxNum() {
		UINT64 iBlock = getBlockIndex(iLast);
		UINT64 remain = getRemain(iLast);
		return getNumber(offset, iBlock, remain);
	}
};

#define max_depth (32)
#define use_sqrt 1
//#pragma intrinsic(_umul128)
//#define primeLstLen primeLst.count
class myPrime {
public:
	myPrime(void* lstBuff, UINT64 lstBuffSize, void* dictBuff, UINT32 dictBuffSize):
		primeLst(lstBuff,lstBuffSize),
		numDict(dictBuff,dictBuffSize)
	{
		if (lstBuff == NULL) {
			primeLstLen = primeLst.count;
			//maxPrime = primeLst[primeLstLen - 1];
			maxPrime = primeLst.lastPrime;
		} else {
			primeLstLen = 0;
			maxPrime = 0;
		}
	}
	UINT64 sol(UINT64 maxNum) {
		UINT32 arr[] = { 2,3,5,7,11,13 };
		UINT32 i;
		UINT64 lastPrime = maxPrime;
		UINT64 lim;
		UINT64 maxPrimeIndex = 0;
		UINT64 added;
		bool bRet;

		//init
		if (primeLstLen < __crt_countof(arr)) {
			for (i = 0; i < __crt_countof(arr); i++) {
				primeLst[i] = arr[i];
			}
			primeLstLen = i;
			lastPrime = primeLst[primeLstLen - 1];
		}

		assert(lastPrime < (MAXUINT64 / 3));

		//start
		for (;;) {
			lim = ((UINT64)3 * (UINT64)lastPrime) - (UINT64)2;
			if (lim > maxNum) {
				lim = maxNum;
			}
#if use_sqrt
			UINT64 limSqrt = sqrt(lim);
			maxPrimeIndex = primeLst.findIndex(limSqrt);
			/*while (primeLst[maxPrimeIndex] < limSqrt)
			{
				maxPrimeIndex++;
			}*/
#endif
			lastPrime += 2;
			bRet = numDict.fill(lastPrime, lim);
			assert(bRet);
			if (!bRet) {
				lim = numDict.getMaxNum();
			}
			//for (i = 1; i < primeLstLen; i++) {
				//ignore(i);
			//}
#if use_sqrt
			for (UINT64 iPrime = 1; iPrime <= maxPrimeIndex; iPrime++) {
				filter2(iPrime, primeLst[iPrime]);
			}
#else
			filter2();
#endif
			added = numDict.pop(primeLst, primeLstLen);
			primeLstLen += added;
			lastPrime = primeLst[primeLstLen - 1];

			if (lim >= maxNum) break;
		}

		maxPrime = primeLst[primeLstLen - 1];
		moveViewCount = primeLst.mMoveViewCount;	//for debug
		return primeLstLen;
	}
private:
#if 0	//sol 1
	void ignore(u4 iBase) {
		node* curNode;
		u4 iPrime = iBase;
		int curLevel = 0;
		curNode = &(stack[curLevel]);
		curNode->value = primeLst[iPrime];
		curNode->iPrime = iPrime;

		u4 minNum = numDict.min();
		u4 maxNum = numDict.max();
		u8 curValue = curNode->value;
		for (;;) {
			u4 prime = primeLst[iPrime];
			curValue = curValue * prime;
			while (curValue < minNum) {
				curLevel++;
				curNode = &(stack[curLevel]);
				curNode->iPrime = iPrime;
				curNode->value = curValue;

				curValue = curValue * prime;
			}
			while (curValue <= maxNum) {
				numDict.remove(curValue);

				curLevel++;
				curNode = &(stack[curLevel]);
				curNode->iPrime = iPrime;
				curNode->value = curValue;

				curValue = curValue * prime;
			}
			for (; curLevel >= 0; ) {
				iPrime = curNode->iPrime - 1;
				curNode->iPrime = iPrime;	//debug
				if (iPrime >= 1) {
					break;
				}

				curLevel--;
				curNode = &(stack[curLevel]);
			}
			if (curLevel < 0) {
				break;
			}

			curNode->iPrime = iPrime;
			curValue = curNode->value;
		}
	}
#endif
	void filter2(UINT64 iBase = 1, UINT64 numBase = 1) {
		node* curNode;
		UINT64 iPrime = iBase;
		int curLevel = 0;
		curNode = &(stack[curLevel]);
		curNode->value = numBase;
		curNode->iPrime = iPrime;

		UINT64 minNum = numDict.getMinNum();
		UINT64 maxNum = numDict.getMaxNum();
		assert(maxNum < MAXUINT64);
		UINT64 c = 0;
		UINT64 curValue = curNode->value;
		for (;;) {
			UINT64 prime = primeLst[iPrime];
			curValue = _umul128(curValue , prime, &c);
			curValue = c == 0 ? curValue : MAXUINT64;
			while (curValue < minNum) {
				curLevel++;
				curNode = &(stack[curLevel]);
				curNode->iPrime = iPrime;
				curNode->value = curValue;

				curValue = _umul128(curValue, prime, &c);
				curValue = c == 0 ? curValue : MAXUINT64;
			}
			while (curValue <= maxNum) {
				numDict.remove(curValue);

				curLevel++;
				curNode = &(stack[curLevel]);
				curNode->iPrime = iPrime;
				curNode->value = curValue;

				curValue = _umul128(curValue, prime, &c);
				curValue = c == 0 ? curValue : MAXUINT64;
			}
			for (curLevel--; curLevel >= 0; curLevel--) {
				curNode = &(stack[curLevel]);
				iPrime = curNode->iPrime + 1;
#if _DEBUG
				curNode->iPrime = iPrime;	//debug
#endif
				if (iPrime < primeLstLen) {
					break;
				}
			}
			if (curLevel < 0) {
				break;
			}

			curNode->iPrime = iPrime;
			curValue = curNode->value;
		}
	}
private:
	struct node
	{
		UINT64 value;
		UINT64 iPrime;
	} stack[max_depth] = { 0 };
	myDict numDict;
	myLst primeLst;
	//u8* primeLst;
	UINT64 primeLstLen;
	//u4 bufferSize;
public:
	UINT64 maxPrime = 0;
	UINT64 moveViewCount = 0;	//for debug
};

#pragma region test
void testDict() {
	UINT8 buff[30];
	myDict dict(buff, sizeof(buff));
	bool bRet = dict.fill(1, 20);
	UINT64 lst[10];
	UINT64 len = dict.pop(lst, 0);
	assert(len == 10);
	
	dict.fill(1, 20);
	assert(dict.count == 10);
	dict.remove(3);
	assert(dict.count == 9);
	dict.remove(5);
	assert(dict.count == 8);
	dict.remove(7);
	assert(dict.count == 7);
	len = dict.pop(lst, 0);

	dict.fill(1, 20);
	assert(dict.count == 10);
	dict.remove(19);
	assert(dict.count == 9);
	dict.remove(17);
	assert(dict.count == 8);
	dict.remove(15);
	assert(dict.count == 7);
	len = dict.pop(lst, 0);
}
void testLst() {
	UINT64 arr[10];
	myLst lst(arr, sizeof(arr));

	for (UINT64 i = 0; i < 5; i++) {
		lst.add(i);
		assert(lst.count == (i+1));
		assert(lst[lst.count - 1] == i);
	}
	for (UINT64 i = 5; i < 10; i++) {
		lst[i] = i;
		assert(lst.count == (i+1));
		assert(lst[lst.count - 1] == i);
	}

	for (UINT64 i = 0; i < 10; i++) {
		lst[i] = i+10;
		assert(lst.count == 10);
		assert(lst[i] == i+10);
	}
}
void DisplayError(TCHAR* pszAPI, DWORD dwError);
void testFileMap() {
	int rc;

#if 1
	TCHAR* zFile = TEXT("tmp.dat");

	HANDLE hFile;

	hFile = CreateFile(
		zFile,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	if (hFile != INVALID_HANDLE_VALUE) {
		rc = 0;
	}
	else {
		rc = GetLastError();
	}
#endif
	LARGE_INTEGER mapSize;
	HANDLE hMap;
	void* mapAddr;

	UINT64 MaxViewSize = 1024 * 1024 * 1024;
	UINT64 MaxViewCount = 16;
	mapSize.QuadPart = MaxViewCount * MaxViewSize;
	hMap = CreateFileMapping(
		//INVALID_HANDLE_VALUE,
		hFile,
		NULL,
		PAGE_READWRITE,
		mapSize.HighPart, mapSize.LowPart,
		NULL
	);
	if (hMap != NULL) {
		LARGE_INTEGER fileOffset;
		fileOffset.QuadPart = 0;
		for (int i = 0; i < MaxViewCount; i++) {
			assert((MaxViewSize >> 32) == 0);
			mapAddr = MapViewOfFile(
				hMap,
				FILE_MAP_ALL_ACCESS,
				fileOffset.HighPart,
				fileOffset.LowPart,
				(UINT32)MaxViewSize
			);
			if (mapAddr != NULL) {
				UnmapViewOfFile(mapAddr);
				fileOffset.QuadPart += MaxViewSize;
			}
			else {
				rc = GetLastError();
				printf_s("i %d\n", i);
				break;
			}
		}
		CloseHandle(hMap);
		CloseHandle(hFile);
	}
	else {
		rc = GetLastError();
		CloseHandle(hFile);
	}

	DisplayError(TEXT(""),rc);
}
void DisplayError(TCHAR* pszAPI, DWORD dwError)
{
	LPVOID lpvMessageBuffer;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	//... now display this string
	_tprintf(TEXT("ERROR: API        = %s\n"), pszAPI);
	_tprintf(TEXT("       error code = %d\n"), dwError);
	_tprintf(TEXT("       message    = %s\n"), (LPTSTR)lpvMessageBuffer);

	// Free the buffer allocated by the system
	LocalFree(lpvMessageBuffer);

	ExitProcess(GetLastError());
}
#pragma endregion


UINT8 dictBuff[256 * 1024 * 1024];
//UINT64 listBuff[16*1024*1024];
int main()
{
	//testDict();
	//testLst();
	//testFileMap();

	UINT64 start = GetTickCount64();
	UINT64 end;
	//myPrime prm(listBuff, sizeof(listBuff), dictBuff, sizeof(dictBuff));
	myPrime prm(0, 0, dictBuff, sizeof(dictBuff));	//use file mapping
	//u8 nPrime = prm.sol(MAXUINT32<<1);
	UINT64 nPrime = prm.sol((UINT64)16000*1000*1000);
	//UINT64 nPrime = prm.sol(200*1000*1000);
	end = GetTickCount64();
	printf_s("nPrime %llu\nmaxPrime %llu\n", nPrime, prm.maxPrime);
	printf_s("moveViewCount %llu\n", prm.moveViewCount);
	printf_s("eleapsed %llu(s)\n", (end - start) / 1000);
}