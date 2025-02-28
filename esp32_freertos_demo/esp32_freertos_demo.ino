/**
 * @brief ESP32 FreeRTOS project
 * @author Chase E. Stewart
 * 
 * A basic project to brush up on some FreeRTOS concepts
 * using deferred processing from interrupts, concurrent tasks,
 * And a global variable shared by concurrent programs
 *
 * TODO make the operations on the shared variable both more intensive
 * and easier to glean, such as changing a string back and forth from 
 * "aaaaaaaaaa" to "bbbbbbbbbb" and back so that any failures of the 
 * concurrency protection would be very clear
 */

/**
 * Heard about this from Digikey tutorial video, trying to ensure
 * all tasks run on same ESP32 core
 */
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

/* ESP32 dev board pinout */
#define LED_PIN 2
#define USR_BUTTON_PIN 0

/* Convenience macros */
#define MSECS(x) (x / portTICK_PERIOD_MS)

/* FreeRTOS Objects */
SemaphoreHandle_t xSerialMutex = { 0 };
TimerHandle_t xTimer = { 0 };

/* Globals */
volatile bool isRecentButtonPress = false;
volatile bool isButtonNeedService = false;
volatile bool is_led_on = true;
int score = 1000;

/**
 * @brief toggle LED + write serial if button pressed within 1 sec
 */
void toggleLED(void *parameter) {
  (void)parameter;
  while (1) {
    if (isRecentButtonPress) {
      xSemaphoreTake(xSerialMutex, 0);
      Serial.println((is_led_on) ? "High!" : "Low!");
      xSemaphoreGive(xSerialMutex);
      digitalWrite(LED_PIN, (is_led_on) ? HIGH : LOW);
    }
    is_led_on = !is_led_on;
    vTaskDelay(MSECS(50));
  }
}

/**
 * @brief Inc counter and log over serial
 */
void serialHeartbeat(void *parameter) {
  (void)parameter;
  while (1) {
    xSemaphoreTake(xSerialMutex, 0);
    score++;
    Serial.print("++ ");
    Serial.println(score);
    xSemaphoreGive(xSerialMutex);
    vTaskDelay(100);
  }
}

/**
 * @brief Deferred button handler, dec counter and log serial
 * button also enables LED flash for 1 second via timer
 */
void buttonService(void *parameter) {
  (void)parameter;
  while (1) {
    if (isButtonNeedService) {
      xSemaphoreTake(xSerialMutex, 0);
      score--;
      isButtonNeedService = false;
      Serial.print("-- ");
      Serial.println(score);
      xSemaphoreGive(xSerialMutex);

      if (isRecentButtonPress) {
        if (!xTimerIsTimerActive(xTimer)) {
          xTimerStart(xTimer, 0);
        } else {
          xTimerReset(xTimer, 0);
        }
      }
    }
    vTaskDelay(5);
  }
}

/**
 * @brief Stop LED flash, LED task logging 1 sec after button int
 */
void timerCallback(void *unused) {
  (void)unused;
  isRecentButtonPress = false;
  is_led_on = false;
  digitalWrite(LED_PIN, LOW);
}

/**
 * @brief quickly set deferred handling vars and return
 */
void buttonPressISR() {
  isButtonNeedService = true;
  isRecentButtonPress = true;
  is_led_on = true;
}

/**
 * @brief Init objects and create tasks
 */
void setup() {

  /* Create mutex for Serial access */
  xSerialMutex = xSemaphoreCreateMutex();

  /* Create timer for button push */
  xTimer = xTimerCreate("led on timer", MSECS(1000), pdFALSE, 0, timerCallback);

  /* Init serial object for 115200 baud */
  Serial.begin(115200);
  Serial.println("Serial is up");

  /* Set pin mode for LED and BOOT button */
  pinMode(LED_PIN, OUTPUT);
  pinMode(USR_BUTTON_PIN, INPUT);

  /* Create a HW interrupt on the BOOT button attached to GPIO0*/
  attachInterrupt(digitalPinToInterrupt(USR_BUTTON_PIN), buttonPressISR, FALLING);

  /* Create toggle LED task*/
  xTaskCreatePinnedToCore(
    toggleLED,
    "LED Task",
    1024,
    NULL,
    1,
    NULL,
    app_cpu);

  /* Create serial heartbeat task*/
  xTaskCreatePinnedToCore(
    serialHeartbeat,
    "Heartbeat Task",
    1024,
    NULL,
    2,
    NULL,
    app_cpu);

  /* Create button servicer task */
  xTaskCreatePinnedToCore(
    buttonService,
    "Button Handler Task",
    1024,
    NULL,
    3,
    NULL,
    app_cpu);

  /**
   * NOTE that the scheduler does not need to be started
   * for expressif's implementation of FreeRTOS for ESP32 on Arduino
   */
}

/**
 * @brief unused default arduino superloop
 */
void loop() {
  /**
   * do nothing
   * expressif's implementation of FreeRTOS will start the scheduler here or before
   */
}
