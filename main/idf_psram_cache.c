#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/dport_reg.h>
#include <stdint.h>
#include <stdio.h>

#define ALLOC_SIZE (64 * 1024)
#define ITER_NUM 1000

void fill_memory(uint8_t *addr, size_t length, uint8_t value) {
  for (size_t i = 0; i < length; i++) {
    addr[i] = value;
  }
}

float test_memory_access(uint8_t *ptr, size_t size, size_t n_iter,
                         uint8_t value) {
  int64_t start = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    fill_memory(ptr, size, value);
  }
  int64_t end = esp_timer_get_time();
  return (end - start) / (n_iter * 1.0f);
}

void app_main(void) {
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  /* Refer to the ESP32 Technical Reference Manual for the cache mode register.
   * If the mode is 0, then each CPU core uses its own cache pool
   */
  // uint64_t cache_mode = DPORT_REG_READ(DPORT_CACHE_MUX_MODE_REG);
  // printf("Cache mode: %llu\n", cache_mode);

  /* Re-configure the task watchdog timer to 30 seconds */
  esp_task_wdt_deinit();
  esp_task_wdt_init(30, false);

  /* malloc with larger than 128 KB will allocate memory on the PSRAM */
  heap_caps_malloc_extmem_enable(128 * 1024);

  /* allocate 64KB of memory on the PSRAM to demonstrate cache hit (at 32KB max)
   * and cache miss */
  uint8_t *data = (uint8_t *)heap_caps_malloc(ALLOC_SIZE, MALLOC_CAP_SPIRAM);

  float average_time;

  average_time = test_memory_access(data, ALLOC_SIZE / 2, ITER_NUM, 255);
  printf(
      "Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n",
      average_time, ITER_NUM, ALLOC_SIZE / 2, data);

  average_time = test_memory_access(data, ALLOC_SIZE, ITER_NUM, 255);
  printf(
      "Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n",
      average_time, ITER_NUM, ALLOC_SIZE, data);

  uint8_t *internal_data = (uint8_t *)malloc(ALLOC_SIZE);
  average_time =
      test_memory_access(internal_data, ALLOC_SIZE / 2, ITER_NUM, 255);
  printf(
      "Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n",
      average_time, ITER_NUM, ALLOC_SIZE / 2, internal_data);

  average_time = test_memory_access(internal_data, ALLOC_SIZE, ITER_NUM, 255);
  printf(
      "Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n",
      average_time, ITER_NUM, ALLOC_SIZE, internal_data);
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
