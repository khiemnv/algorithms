#ifndef __MOVE_DATA_MNG__
#define __MOVE_DATA_MNG__

typedef struct TransportParams TransportParams;
typedef struct TransportStatus TransportStatus;

//-----------------structure define--------------
typedef struct TransporterParams TransporterParams;

struct TransportParams{
  enum ModuleType {
    singleTransporter,
    transportClientServer,
  };
  ModuleType modType;
  TransporterParams *exParams;
};

struct TransportStatus{
};

class Transport_IF {
public:
  virtual int Init(TransportParams*) = 0;
  virtual int Start() = 0;
  virtual int GetStatus(TransportStatus*) = 0;
  virtual int Cancel() = 0;
  virtual int Final() = 0;
};

//C API
void* TransportCreate(TransportParams*);
int TransportStart(void* transporterHandle);
int TransportGetStatus(void* transporterHandle, TransportStatus* pStatus);
int TransportCancel(void* transporterHandle);
int TransportFinal(void* transporterHandle);

Transport_IF* transport_load_transporter();
Transport_IF* transport_load_transport_client();
int transport_init(Transport_IF*, TransportParams*);
int transport_final(Transport_IF*);
int transport_release(Transport_IF*);

#endif //__MOVE_DATA_MNG__
