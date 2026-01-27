#include "../libDaisy/src/daisy_seed.h"
#include "../wasmi-daisy/wasmi_daisy.h"
#include "../wasm-module/build/module_wasm.h"
#include "SDRAM.hpp"

using namespace daisy;
static DaisySeed hardware;

// Global SDRAM allocator instance
static Jaffx::SDRAM sdram;

// Wasmi runtime objects
WasmiEngine* engine = nullptr;
WasmiStore* store = nullptr;
WasmiModule* module = nullptr;
WasmiInstance* instance = nullptr;
WasmiFunc* process_func = nullptr;

// Macro for halting on errors
#define ERROR_HALT while (true) {}

// Macro for block size
#define BLOCK_SIZE 128

// Macro for enabling audio
// #define RUN_AUDIO

// Provide allocation functions for wasmi-daisy to use Jaffx SDRAM manager
extern "C" {
    void* jaffx_sdram_malloc(size_t size) {
        return sdram.malloc(size);
    }

    void jaffx_sdram_free(void* ptr) {
        if (ptr) sdram.free(ptr);
    }
}

// WebAssembly module that adds two numbers
// Generated from wasm-module/module.cpp

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

bool InitWasmi() {
  hardware.PrintLine("Initializing wasmi runtime with Daisy wrapper...");

  // Create wasmi engine
  hardware.PrintLine("[INIT] Creating wasmi engine...");
  engine = wasmi_engine_new();
  if (!engine) {
    hardware.PrintLine("[ERROR] Failed to create engine!");
    ERROR_HALT
  }
  hardware.PrintLine("[OK] Engine created successfully");  

  // Create store
  hardware.PrintLine("[INIT] Creating wasmi store...");
  store = wasmi_store_new(engine);
  if (!store) {
    hardware.PrintLine("[ERROR] Failed to create store!");
    wasmi_engine_delete(engine);
    engine = nullptr;
    ERROR_HALT
  }
  hardware.PrintLine("[OK] Store created successfully"); 

  // Load module
  hardware.PrintLine("[INIT] Loading WebAssembly module...");
  hardware.PrintLine("[INFO] Module size: %d bytes", build_module_wasm_len);
  module = wasmi_module_new(engine, build_module_wasm, build_module_wasm_len);
  if (!module) {
    hardware.PrintLine("[ERROR] Failed to load module!");
    wasmi_store_delete(store);
    wasmi_engine_delete(engine);
    store = nullptr;
    engine = nullptr;
    ERROR_HALT
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
    ERROR_HALT
  }
  hardware.PrintLine("[OK] Module instantiated successfully");

  hardware.PrintLine("[INIT] Getting exported 'process' function...");
  const char* func_name = "process";
  process_func = wasmi_instance_get_func(
    store,
    instance,
    reinterpret_cast<const uint8_t*>(func_name),
    7  // strlen("process")
  );

  // Check if we got the function
  if (!process_func) {
    hardware.PrintLine("[ERROR] Failed to get 'process' function!");
    wasmi_instance_delete(instance);
    wasmi_module_delete(module);
    wasmi_store_delete(store);
    wasmi_engine_delete(engine);
    instance = nullptr;
    module = nullptr;
    store = nullptr;
    engine = nullptr;
    ERROR_HALT
  }
  hardware.PrintLine("[OK] Got 'process' function successfully");  

  // Wasmi initialized successfully!
  hardware.PrintLine("");
  hardware.PrintLine("[SUCCESS] Wasmi initialized and ready!");
  hardware.PrintLine("");

  return true;
}

// Audio callback using buffer-based wasmi processing
static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
  // Process the entire buffer at once using the wasmi wrapper
  wasmi_func_call_buffer_process(store, instance, process_func, in[0], out[0], size);

  // Copy left channel to right channel for stereo output
  memcpy(out[1], out[0], size * sizeof(float));
}

int main() {
  hardware.Init();
  hardware.StartLog(true); // wait for serial connection

  System::Delay(200);
  hardware.PrintLine("===========================================");
  hardware.PrintLine("         Wasmi-Daisy Test Program          ");
  hardware.PrintLine("===========================================");
  hardware.PrintLine("");

  // Initialize SDRAM allocator
  hardware.PrintLine("Initializing SDRAM allocator...");
  sdram.init();
  hardware.PrintLine("SDRAM initialized (64MB at 0xC0000000)");
  hardware.PrintLine("");  

  // Initialize wasmi and load module
  if (!InitWasmi()) {
    hardware.PrintLine("FATAL: wasmi initialization failed");
    ERROR_HALT
  }

  hardware.PrintLine("=== Testing Process Function ===");

  // Generate a few test samples with different inputs
  hardware.PrintLine("Calling process() with various inputs...");
  for (int i = 0; i < 5; i++) {
    float input = (float)i * 0.1f;  // 0.0, 0.1, 0.2, 0.3, 0.4
    
    // prepare and process single-sample buffers
    float input_buffer[1] = {input};
    float output_buffer[1] = {0.0f};
    wasmi_func_call_buffer_process(store, instance, process_func, input_buffer, output_buffer, 1);
    float output = output_buffer[0];

    hardware.PrintLine("  process(" FLT_FMT3 ") = " FLT_FMT3, FLT_VAR3(input), FLT_VAR3(output));
  }  

  hardware.PrintLine("");
  hardware.PrintLine("=== Running Performance Benchmarks ===");
  System::Delay(100);  

  // Benchmark configuration (same as your example)
  const int WARMUP_RUNS = 10;
  const int BENCHMARK_RUNS = 100;

  // Warmup phase
  hardware.PrintLine("");
  hardware.PrintLine("[WARMUP] Running %d warmup iterations...", WARMUP_RUNS);
  volatile float warmup_result = 0.0f;
  for (int i = 0; i < WARMUP_RUNS; i++) {

    // prepare and process single-sample buffers
    float input = daisy::Random::GetFloat(-1.f, 1.f);
    float input_buffer[1] = {input};
    float output_buffer[1] = {0.0f};
    wasmi_func_call_buffer_process(store, instance, process_func, input_buffer, output_buffer, 1);
    float output = output_buffer[0];

    warmup_result += output;
  }
  hardware.PrintLine("[OK] Warmup complete (result=" FLT_FMT3 ")", FLT_VAR3(warmup_result));  

  // Benchmark phase
  hardware.PrintLine("");
  hardware.PrintLine("[BENCHMARK] Running %d iterations...", BENCHMARK_RUNS);
  
  float total_us = 0.0f;
  float min_us = 1e9f;
  float max_us = 0.0f;
  float total_ticks = 0.0f;
  float min_ticks = 1e9f;
  float max_ticks = 0.0f;
  volatile float checksum = 0.0f;
  
  for (int i = 0; i < BENCHMARK_RUNS; i++) {

    // prepare buffers
    float input_buffer[BLOCK_SIZE];
    float output_buffer[BLOCK_SIZE];
    for (int j = 0; j < BLOCK_SIZE; j++) {
      input_buffer[j] = daisy::Random::GetFloat(-1.f, 1.f);
      output_buffer[j] = 0.f;
    }
        
    Timer timer;
    timer.start();
    wasmi_func_call_buffer_process(store, instance, process_func, input_buffer, output_buffer, BLOCK_SIZE);
    timer.end();
    
    float elapsed_us = timer.usElapsed();
    float elapsed_ticks = (float)timer.ticksElapsed();
    
    total_us += elapsed_us;
    total_ticks += elapsed_ticks;
      
    if (elapsed_us < min_us) min_us = elapsed_us;
    if (elapsed_us > max_us) max_us = elapsed_us;
    if (elapsed_ticks < min_ticks) min_ticks = elapsed_ticks;
    if (elapsed_ticks > max_ticks) max_ticks = elapsed_ticks;
      
    // use checksum to prevent optimization
    for (int j = 0; j < BLOCK_SIZE; j++) {
      checksum += output_buffer[j];
    }        

  }
    
  float avg_us = total_us / BENCHMARK_RUNS;
  float avg_ticks = total_ticks / BENCHMARK_RUNS;
  
  hardware.PrintLine("");
  hardware.PrintLine("=== BENCHMARK RESULTS ===");
  hardware.PrintLine("Iterations: %d", BENCHMARK_RUNS);
  hardware.PrintLine("Average:    " FLT_FMT3 " us (%d ticks)", FLT_VAR3(avg_us), (int)avg_ticks);
  hardware.PrintLine("Minimum:    " FLT_FMT3 " us (%d ticks)", FLT_VAR3(min_us), (int)min_ticks);
  hardware.PrintLine("Maximum:    " FLT_FMT3 " us (%d ticks)", FLT_VAR3(max_us), (int)max_ticks);
  hardware.PrintLine("Checksum:   " FLT_FMT3 " (prevents optimization)", FLT_VAR3(checksum));
  
  // Calculate real-time performance
  float samples_per_us = (float)BLOCK_SIZE / avg_us;
  float samples_per_sec = samples_per_us * 1000000.0f;
  float realtime_factor_48k = samples_per_sec / 48000.0f;
  
  hardware.PrintLine("");
  hardware.PrintLine("=== REAL-TIME ANALYSIS ===");
  hardware.PrintLine("Sample rate: 48000 Hz");
  hardware.PrintLine("Throughput: " FLT_FMT3 " samples/sec", FLT_VAR3(samples_per_sec));
  hardware.PrintLine("Real-time factor: " FLT_FMT3 "x", FLT_VAR3(realtime_factor_48k));
    
  if (realtime_factor_48k >= 1.0f) {
    hardware.PrintLine("Result: CAN run in REAL-TIME! OK");
  } else {
    hardware.PrintLine("Result: Too slow for real-time X");
  }
  
  hardware.PrintLine("");
  hardware.PrintLine("[SUCCESS] wasmi benchmark complete!");

  hardware.PrintLine("Test Complete!");
  hardware.SetLed(true);
  System::Delay(200);

  #ifndef RUN_AUDIO
  // Prepare for next test
  System::ResetToBootloader(System::BootloaderMode::DAISY_INFINITE_TIMEOUT);
  #endif

  // Start Audio
  hardware.SetAudioBlockSize(BLOCK_SIZE); // number of samples handled per callback (buffer size)
	hardware.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ); // sample rate
  hardware.StartAudio(AudioCallback);
  return 0;
}