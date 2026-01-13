#include "../libDaisy/src/daisy_seed.h"
#include "../wasmi-daisy/wasmi_daisy.h"
#include "SDRAM.hpp"

using namespace daisy;
static DaisySeed hardware;
Jaffx::SDRAM sdram_manager;

// Provide allocation functions for wasmi-daisy to use Jaffx SDRAM manager
extern "C" {
  void* jaffx_sdram_malloc(size_t size) {
    return sdram_manager.malloc(size);
  }
  
  void jaffx_sdram_free(void* ptr) {
    sdram_manager.free(ptr);
  }
}

// WebAssembly module that adds two numbers
// (module
//   (func (export "add") (param i32 i32) (result i32)
//     local.get 0
//     local.get 1
//     i32.add))
static const uint8_t wasm_add[] = {
  0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
  0x01, 0x07, 0x01, 0x60, 0x02, 0x7f, 0x7f, 0x01,
  0x7f, 0x03, 0x02, 0x01, 0x00, 0x07, 0x07, 0x01,
  0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09,
  0x01, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a,
  0x0b
};

class Timer {
private:
  bool mDone = false;
  unsigned int mStartTime = 0;
  unsigned int mEndTime = 0;
  unsigned int mTickFreq = 0; 

public:
  void start() {  
    mTickFreq = System::GetTickFreq();
    this->mStartTime = System::GetTick();
  }

  void end() {
    this->mEndTime = System::GetTick();
    mDone = true;
  }

  unsigned int ticksElapsed() {
    if (!mDone) {
      return 0;
    }
    return mEndTime - mStartTime;
  }

  float usElapsed() {
    if (!mDone) {
      return 0.f;
    }
    float ticksElapsed = float(mEndTime - mStartTime);
    return (ticksElapsed * 1e6f) / mTickFreq;
  }
};

int main() {
  // Wasmi stuff
  WasmiEngine* engine = nullptr;
  WasmiStore* store = nullptr;
  WasmiModule* module = nullptr;
  WasmiInstance* instance = nullptr;
  WasmiFunc* add_func = nullptr;

  bool initialized = false;
  bool test_complete = false;
  int32_t test_result = 0;
  unsigned int counter = 0;

  // Initialize hardware
  hardware.Init();
  sdram_manager.init();
  hardware.StartLog(true); // wait for serial connection
  System::Delay(200);
  hardware.PrintLine("===========================================");
  hardware.PrintLine("  Wasmi-Daisy Test Program");
  hardware.PrintLine("===========================================");
  hardware.PrintLine("");


  // Initialize WebAssembly
  hardware.PrintLine("[INIT] Creating wasmi engine...");
  engine = wasmi_engine_new();
  if (!engine) {
    hardware.PrintLine("[ERROR] Failed to create engine!");
    while (true) {}; // halt here
  }
  hardware.PrintLine("[OK] Engine created successfully");  


  // Create store
  hardware.PrintLine("[INIT] Creating wasmi store...");
  store = wasmi_store_new(engine);
  if (!store) {
    hardware.PrintLine("[ERROR] Failed to create store!");
    wasmi_engine_delete(engine);
    engine = nullptr;
    while (true) {}; // halt here
  }
  hardware.PrintLine("[OK] Store created successfully"); 
  

  // Load module
  hardware.PrintLine("[INIT] Loading WebAssembly module...");
  hardware.PrintLine("[INFO] Module size: %d bytes", sizeof(wasm_add));
  module = wasmi_module_new(engine, wasm_add, sizeof(wasm_add));
  if (!module) {
    hardware.PrintLine("[ERROR] Failed to load module!");
    wasmi_store_delete(store);
    wasmi_engine_delete(engine);
    store = nullptr;
    engine = nullptr;
    while (true) {}; // halt here
  }
  hardware.PrintLine("[OK] Module loaded successfully");


  // Instantiate module
  hardware.PrintLine("[INIT] Instantiating module...");
  instance = wasmi_instance_new(store, module);
  if (!instance) {
    hardware.PrintLine("[ERROR] Failed to instantiate module!");
    wasmi_module_delete(module);
    wasmi_store_delete(store);
    wasmi_engine_delete(engine);
    module = nullptr;
    store = nullptr;
    engine = nullptr;
    while (true) {}; // halt here
  }
  hardware.PrintLine("[OK] Module instantiated successfully");

  hardware.PrintLine("[INIT] Getting exported 'add' function...");
  const char* func_name = "add";
  add_func = wasmi_instance_get_func(
    store,
    instance,
    reinterpret_cast<const uint8_t*>(func_name),
    3  // strlen("add")
  );

  // Check if we got the function
  if (!add_func) {
    hardware.PrintLine("[ERROR] Failed to get 'add' function!");
    wasmi_instance_delete(instance);
    wasmi_module_delete(module);
    wasmi_store_delete(store);
    wasmi_engine_delete(engine);
    instance = nullptr;
    module = nullptr;
    store = nullptr;
    engine = nullptr;
    while (true) {}; // halt here
  }
  hardware.PrintLine("[OK] Got 'add' function successfully");  

  // Wasmi initialized successfully!
  initialized = true;
  hardware.PrintLine("");
  hardware.PrintLine("[SUCCESS] Wasmi initialized and ready!");
  hardware.PrintLine("");

  // Test the add function
  hardware.PrintLine("[TEST] Calling wasm add(42, 58)...");
  Timer mTimer;
  mTimer.start();
  test_result = wasmi_func_call_i32_i32_to_i32(store, add_func, 42, 58);
  mTimer.end();
  hardware.PrintLine("[RESULT] add(42, 58) = %d", test_result);
  hardware.PrintLine("[INFO] Wasm function call took %d ticks", mTimer.ticksElapsed());
  hardware.PrintLine("[INFO] Wasm function call took " FLT_FMT3 "us", FLT_VAR3(mTimer.usElapsed()));
  if (test_result == 100) {
    hardware.PrintLine("[SUCCESS] Wasm add function returned correct result!");
  } 
  else {
    hardware.PrintLine("[FAILURE] Wasm add function returned incorrect result!");
  }
  
  
  // Prepare for next test iteration 
  hardware.PrintLine("[STATUS] Test complete! Resetting to bootloader.");
  hardware.PrintLine("");
  hardware.PrintLine("===========================================");
  System::Delay(200);
  System::ResetToBootloader(System::BootloaderMode::DAISY_INFINITE_TIMEOUT);
  return 0;
}