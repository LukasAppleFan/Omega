#include "interface.h"

namespace Ion {
namespace USB {
namespace Device {

static inline uint16_t min(uint16_t x, uint16_t y) { return (x<y ? x : y); }

bool Interface::processSetupInRequest(SetupPacket * request, uint8_t * transferBuffer, uint16_t * transferBufferLength, uint16_t transferBufferMaxLength) {
  if (request->requestType() != SetupPacket::RequestType::Standard) {
    return false;
  }
  switch (request->bRequest()) {
    case (int) Request::GetStatus:
      return getStatus(transferBuffer, transferBufferLength, transferBufferMaxLength);
    case (int) Request::SetInterface:
      return setInterface(request, transferBufferLength);
    case (int) Request::GetInterface:
      return getInterface(transferBuffer, transferBufferLength);
    case (int) Request::ClearFeature:
      return clearFeature(transferBufferLength);
    case (int) Request::SetFeature:
      return setFeature(transferBufferLength);
  }
}

bool Interface::getStatus(uint8_t * transferBuffer, uint16_t * transferBufferLength, uint16_t transferBufferMaxLength) {
  *transferBufferLength = min(2, transferBufferMaxLength);
  for (int i = 0; i<*transferBufferLength; i++) {
    transferBuffer[i] = 0; // By specification
  }
  return true;
}

bool Interface::getInterface(uint8_t * transferBuffer, uint16_t * transferBufferLength) {
  *transferBufferLength = 1;
  transferBuffer[0] = getActiveInterfaceAlternative();
  return true;
}

bool Interface::setInterface(SetupPacket * request, uint16_t * transferBufferLength) {
  // We support one interface only
  setActiveInterfaceAlternative(request->wValue());
  /* There is one interface alternative only, we no need to set it again, just
   * reset the endpoint. */
  //m_ep0->reset(); //TODO Needed?
  *transferBufferLength = 0;
  return true;
}

bool Interface::clearFeature(uint16_t * transferBufferLength) {
  //TODO ?
  *transferBufferLength = 0;
  return true;
}

bool Interface::setFeature(uint16_t * transferBufferLength) {
  //TODO ?
  *transferBufferLength = 0;
  return true;
}

}
}
}
