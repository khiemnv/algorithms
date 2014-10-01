#include "transport_client.h"

//-----------------transport client--------------
Transport_IF* transport_load_transport_client() {
  return new TransportClient();
}
int TransportClient::Init(TransportParams*) {return 0;}
int TransportClient::Start() {return 0;}
int TransportClient::GetStatus(TransportStatus*) {return 0;}
int TransportClient::Cancel() {return 0;}
int TransportClient::Final() {return 0;}
//-----------------transport server--------------