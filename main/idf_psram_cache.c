#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#define ALLOC_SIZE (64 * 1024)
#define ITER_NUM 1000

void fill_memory(uint8_t *addr, size_t length, uint8_t value) {
  for (size_t i = 0; i < length; i++) {
    addr[i] = value;
  }
}

void app_main(void) {
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  /* Re-configure the task watchdog timer to 30 seconds */
  esp_task_wdt_deinit();
  esp_task_wdt_init(30, false);

  /* malloc with larger than 8 bytes will allocate memory on the PSRAM */
	heap_caps_malloc_extmem_enable(128*1024);

  /* allocate 64KB of memory on the PSRAM to demonstrate cache hit (at 32KB max)
   * and cache miss */
  uint8_t *data = (uint8_t *)heap_caps_malloc(ALLOC_SIZE, MALLOC_CAP_SPIRAM);

  int64_t start;
  int64_t end;
  float average_time;

  start = esp_timer_get_time();
  for (int i = 0; i < ITER_NUM; i++) {
    fill_memory(data, ALLOC_SIZE / 2, 255);
  }
  end = esp_timer_get_time();
  average_time = (end - start) / (ITER_NUM * 1.0f);
  printf("Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n", average_time,
         ITER_NUM, ALLOC_SIZE / 2, data);

  start = esp_timer_get_time();
  for (int i = 0; i < ITER_NUM; i++) {
    fill_memory(data, ALLOC_SIZE, 255);
  }
  end = esp_timer_get_time();
  average_time = (end - start) / (ITER_NUM * 1.0f);
  printf("Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n", average_time,
				 ITER_NUM, ALLOC_SIZE, data);

	uint8_t *internal_data = (uint8_t *)malloc(ALLOC_SIZE);
	start = esp_timer_get_time();
	for (int i = 0; i < ITER_NUM; i++) {
		fill_memory(internal_data, ALLOC_SIZE / 2, 255);
	}
	end = esp_timer_get_time();
	average_time = (end - start) / (ITER_NUM * 1.0f);
	printf("Average time: %f us, n_iter: %d, write_size: %d, memory address: %p\n", average_time,
				 ITER_NUM, ALLOC_SIZE / 2, internal_data);

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
