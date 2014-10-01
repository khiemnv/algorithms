#include "moveDataMng.h"

//-----------------transport client--------------
class TransportClient
  :public Transport_IF
{
private:
  void* mCnnClientHandle;
public:
  //transport IF
  int Init(TransportParams*);
  int Start();
  int GetStatus(TransportStatus*);
  int Cancel();
  int Final();
  //connection IF
  int Send();
  int Recv();
  //
  int Connect();
  int Disconnect();
  int Close();
};
//-----------------transport server--------------
class TransportServer
{
private:
  void* mTransporterHandle;
  void* mCnnServerHandle;
public:
  //connection IF
  int Send();
  int Recv();
  //
  int Start();
  int Listen();
  int Disconnect();
  int Stop();
};
