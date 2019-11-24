// prime.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <crtdbg.h>

#define assert(expr) _ASSERT(expr)

typedef unsigned long long u8;
typedef unsigned long u4;
typedef unsigned char u1;

#define block_len 8
#define getBlockIndex(iBit) ((iBit) >> 3)
#define getNumber(offset, iBlock,iBit) ( (offset) + ((((iBlock) << 3) | (iBit))<<1))
#define getRemain(iBit) ( (iBit) & 0x07 )

class myDict {
public:
	myDict(void* buff, u4 buffSize) {
		this->buff = (u1*)buff;
		this->buffSize = buffSize;
		count = 0;
		iFirst = 0;
		iLast = 0;
		offset = 0;
	}
	bool fill(u4 minNum, u4 maxNum) {
		offset = minNum;
		iFirst = 0;
		iLast = (maxNum - minNum) >> 1;
		u4 remain = getRemain(iLast);
		u4 maxIndex = getBlockIndex(iLast);

		if (maxIndex >= buffSize) return false;

		//init blocks
		for (u4 i = 0; i < maxIndex; i++) {
			buff[i] = 0xff;
		}
		u1 blockValue = 1;
		for (u4 i = 0; i < remain; i++) {
			blockValue = (blockValue << 1) | 1;
		}
		buff[maxIndex] = blockValue;

		count = iLast + 1;
		return true;
	}
	u4 pop(u4* list, u4 listLen) {
		u4 iBlock = 0;
		u4 iBit;
		u4 num;
		u4 i;
		u1 blockValue;
		for (i = 0; i < count; iBlock++) {
			blockValue = buff[iBlock];
			for (iBit = 0;blockValue != 0; iBit++) {
				if ((blockValue) & 1) {
					num = getNumber(offset, iBlock, iBit);
					list[listLen + i] = num;
					i++;
				}
				blockValue >>= 1;
			}
		}
		count = 0;
		return i;
	}
	void remove(u4 num) {
		u4 iBit = (num - offset) >> 1;
		u4 remain = getRemain(iBit);
		u4 iBlock = getBlockIndex(iBit);
		u1 blockValue = buff[iBlock];
		u1 flag = 0x01 << remain;
		assert(blockValue & flag);
		buff[iBlock] = blockValue ^ flag;
		count--;
	}
	u4 count;
	u4 min() {
		return offset;
	}
	u4 max() {
		u4 iBlock = getBlockIndex(iLast);
		u4 remain = getRemain(iLast);
		return getNumber(offset, iBlock, remain);
	}
private:
	u1* buff;
	u4 buffSize;
	u4 offset;
	u4 iFirst;
	u4 iLast;
};

#define max_depth (32)
class myPrime {
public:
	myPrime(void* lstBuff, u4 lstBuffSize, void* dictBuff, u4 dictBuffSize) {
		numDict = new myDict(dictBuff, dictBuffSize);
		primeLst = (u4*)lstBuff;
		bufferSize = lstBuffSize;
		primeLstLen = 0;
	}
	u4 sol(u4 maxNum) {
		u4 arr[] = { 2,3,5,7,11,13 };
		u4 i;
		u4 maxPrime;
		u8 lim;
		u4 added;
		bool bRet;

		//init
		for (i = 0; i < __crt_countof(arr); i++) {
			primeLst[i] = arr[i];
		}
		primeLstLen = i;

		//start
		for (;;) {
			maxPrime = primeLst[primeLstLen - 1];
			lim = ((u8)3 * (u8)maxPrime) - (u8)2;
			if (lim > maxNum) {
				lim = maxNum;
			}
			bRet = numDict->fill(maxPrime + 2, lim);
			assert(bRet);
			//for (i = 1; i < primeLstLen; i++) {
				//ignore(i);
			//}
			filter2();
			added = numDict->pop(primeLst, primeLstLen);
			primeLstLen += added;

			if (lim >= maxNum) break;
		}
		return primeLstLen;
	}
private:
	struct node
	{
		u8 value;
		u4 iPrime;
	} stack[max_depth];
	void ignore(u4 iBase) {
		node* curNode;
		u4 iPrime = iBase;
		int curLevel = 0;
		curNode = &(stack[curLevel]);
		curNode->value = primeLst[iPrime];
		curNode->iPrime = iPrime;

		u4 minNum = numDict->min();
		u4 maxNum = numDict->max();
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
				numDict->remove(curValue);

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
	void filter2(u4 numBase = 1) {
		node* curNode;
		u4 iPrime = 1;
		int curLevel = 0;
		curNode = &(stack[curLevel]);
		curNode->value = numBase;
		curNode->iPrime = iPrime;

		u4 minNum = numDict->min();
		u4 maxNum = numDict->max();
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
				numDict->remove(curValue);

				curLevel++;
				curNode = &(stack[curLevel]);
				curNode->iPrime = iPrime;
				curNode->value = curValue;

				curValue = curValue * prime;
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
	myDict* numDict;
	u4* primeLst;
	u4 primeLstLen;
	u4 bufferSize;
};

void testDict() {
	u1 buff[30];
	myDict dict(buff, sizeof(buff));
	bool bRet = dict.fill(1, 20);
	u4 lst[10];
	u4 len = dict.pop(lst, 0);
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


#include <Windows.h>

u1 dictBuff[512 * 1024 * 1024];
u4 listBuff[256 * 1024 * 1024];
int main()
{
	//testDict();

	DWORD start = GetTickCount();
	DWORD end;
	myPrime prm(listBuff, sizeof(listBuff), dictBuff, sizeof(dictBuff));
	u4 nPrime = prm.sol(MAXUINT32);
	end = GetTickCount();
	printf_s("eleapsed %d(s)\n", (end - start) / 1000);
	printf_s("nPrime %lu\nmaxPrime %lu\n", nPrime, listBuff[nPrime - 1]);
}