#include <bootloader/boot.h>
#include <bootloader/slot.h>
#include <ion.h>
#include <bootloader/interface.h>

#include <assert.h>

namespace Bootloader {

BootMode Boot::mode() {
  // We use the exam mode driver as storage for the boot mode
  uint8_t mode = Ion::ExamMode::FetchExamMode();

  if (mode > 3)
    return Unknown;
  
  return (BootMode) mode;
}

void Boot::setMode(BootMode mode) {
  BootMode currentMode = Boot::mode();
  if (currentMode == mode)
    return;

  assert(mode != BootMode::Unknown);
  int8_t deltaMode = (int8_t)mode - (int8_t)currentMode;
  deltaMode = deltaMode < 0 ? deltaMode + 4 : deltaMode;
  assert(deltaMode > 0);
  Ion::ExamMode::IncrementExamMode(deltaMode);
}

__attribute__((noreturn)) void Boot::boot() {
  assert(mode() != BootMode::Unknown);

  if (!Slot::A().kernelHeader()->isValid() && !Slot::B().kernelHeader()->isValid()) {
    // Bootloader if both invalid
    bootloader();
  } else if (!Slot::A().kernelHeader()->isValid()) {
    // If slot A is invalid and B valid, boot B
    setMode(BootMode::SlotB);
    Slot::B().boot();
  } else if (!Slot::B().kernelHeader()->isValid()) {
    // If slot B is invalid and A valid, boot A
    setMode(BootMode::SlotA);
    Slot::A().boot();
  } else {
    // Both valid, boot the selected one
    if (mode() == BootMode::SlotA) {
      Slot::A().boot();
    } else if (mode() == BootMode::SlotB) {
      Slot::B().boot();
    }
  }

  // Achivement unlocked: How Did We Get Here?
  bootloader();
}

__attribute__ ((noreturn)) void Boot::bootloader() {
  Bootloader::Interface::draw();
  for(;;) {
    Ion::USB::enable();
    while (!Ion::USB::isEnumerated());
    Ion::USB::DFU(false);
  }
}

}
