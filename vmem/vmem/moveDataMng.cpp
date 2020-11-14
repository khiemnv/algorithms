#include <crtdbg.h>
#define assert(x) _ASSERT(x)

#include "moveDataMng.h"

//-----------------C API-------------------------
int transport_init(Transport_IF* hTransporter, TransportParams* params) {
  return hTransporter->Init(params);
}
int transport_final(Transport_IF* hTransporter) {
  return hTransporter->Final();
}
int transport_release(Transport_IF* hTransporter) {
  delete hTransporter;
  return 0;
}

void* TransportCreate(TransportParams* params)
{
  int rc = 0;
  Transport_IF* hTransporter;
  if (params->modType == params->singleTransporter) {
    hTransporter = transport_load_transporter();
  } else {
    hTransporter = transport_load_transport_client();
  }
  if (hTransporter) {rc = transport_init(hTransporter, params);}
  //init error
  if (rc) {
    transport_final(hTransporter);
    transport_release(hTransporter);
    hTransporter = 0;
  }
  return hTransporter;
};
int TransportStart(void* transporterHandle)
{
  assert(transporterHandle);
  int rc;
  Transport_IF* p = (Transport_IF*)transporterHandle;
  rc = p->Start();
  return rc;
}
int TransportGetStatus(void* transporterHandle, TransportStatus* pStatus)
{
  assert(transporterHandle && pStatus);
  int rc;
  Transport_IF* p = (Transport_IF*)transporterHandle;
  rc = p->GetStatus(pStatus);
  return rc;
}
int TransportCancel(void* transporterHandle)
{
  assert(transporterHandle);
  int rc;
  Transport_IF* p = (Transport_IF*)transporterHandle;
  rc = p->Cancel();
  return rc;
}
//final and release transporter handle
int TransportFinal(void* transporterHandle)
{
  assert(transporterHandle);
  int rc;
  Transport_IF* p = (Transport_IF*)transporterHandle;
  rc = p->Final();
  delete p;
  return rc;
}

