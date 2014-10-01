#include <crtdbg.h>
#define assert(x) _ASSERT(x)
#include <stdio.h>
#include <Windows.h>

long long getDiskFree(char zDisk)
{
  char zRootPath[] = "C:\\";
  zRootPath[0] = zDisk;
  DWORD dwSpc, dwBps, dwnfc, dwnc;
  int rc = GetDiskFreeSpaceA(
    zRootPath,
    &dwSpc,
    &dwBps,
    &dwnfc,
    &dwnc
    );
  if (!rc) {
    rc = GetLastError();
    return 0;
  }
  long long freeSpace = dwnfc;
  //auto cast dw to long long
  freeSpace *= dwSpc;
  freeSpace *= dwBps;
  return freeSpace;
}
long long getDiskUsed(char zDisk)
{
  char zRootPath[] = "C:\\";
  zRootPath[0] = zDisk;
  DWORD dwSpc, dwBps, dwnfc, dwnc;
  int rc = GetDiskFreeSpaceA(
    zRootPath,
    &dwSpc,
    &dwBps,
    &dwnfc,
    &dwnc
    );
  if (!rc) {
    rc = GetLastError();
    return 0;
  }
  //auto cast dw to long long
  long long used = dwnc - dwnfc;
  used *= dwSpc;
  used *= dwBps;
  return used;
}
int main(int argc, char **argv)
{
  if (argc == 3) {
    char zExport[MAX_PATH];
    char zImport[MAX_PATH];
    char desDisk = argv[1][0];
    char srcDisk;
    char alias = 'A';
    //
    long long freeSpace = getDiskFree(desDisk);
    //
    long long used = 0;
    int nDisks = strlen(argv[2]);
    int len = 0;
    zExport[0] = 0;
    zImport[0] = 0;
    for (int i = 0; i < nDisks; i++, alias++) {
      //get disk size
      srcDisk = argv[2][i];
      if (srcDisk == desDisk) {
        assert(0);
        continue;
      }
      used += getDiskUsed(srcDisk);
      //make zParams
      sprintf_s(zExport + len, MAX_PATH - len, "<%C:\\|%C|%C:\\%C>",
        srcDisk, alias, desDisk, alias);
      len += sprintf_s(zImport + len, MAX_PATH - len, "<%C:\\%C|%C|%C:\\>",
        desDisk, alias, alias, srcDisk);
    }
    double diskFree = (double)freeSpace;
    double diskUsed = (double)used;
    diskFree /= 1024*1024*1024;
    diskUsed /= 1024*1024*1024;
    printf("space %I64d bytes used %I64d bytes\n", freeSpace, used);
    printf("diskFree %.3f GB diskUsed %.3f GB\n", diskFree, diskUsed);
    printf("export %s\n", zExport);
    printf("import %s\n", zImport);
  }
  else {
    printf("<des disk> <src disks>\n");
  }
  return 0;
}
