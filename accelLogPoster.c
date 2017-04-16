#include <pebble.h>
 
static Window* window;
static TextLayer   *accel_layer;
static DictationSession *s_dictation_session;

 bool message = true;
static void window_load(Window *window)
{

  
  
  //Setup Accel Layer
  accel_layer = text_layer_create(GRect(5, 45, 144, 30));
  text_layer_set_font(accel_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(accel_layer, "Accel tap: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(accel_layer));
}
 
static void window_unload(Window *window)
{
  
  text_layer_destroy(accel_layer);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  // Print the results of a transcription attempt
  APP_LOG(APP_LOG_LEVEL_INFO, "Dictation status: %d", (int)status);
  
  if(status == DictationSessionStatusSuccess) {
  // Display the dictated text
  APP_LOG(APP_LOG_LEVEL_INFO, "transcription %s", transcription);
    DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

    dict_write_cstring(iter, "voice", transcription);


  dict_write_end(iter);
  app_message_outbox_send();
} else {
  // Display the reason for any error
}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  APP_LOG(APP_LOG_LEVEL_INFO, "Button clicked %i", 3);
  
  // Declare a buffer for the DictationSession
  static char s_last_text[512];
  
  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text),
                                               dictation_session_callback, NULL);
  
  // Start dictation UI
  dictation_session_start(s_dictation_session);
}

static void click_config_provider(void *context) {
  // Subcribe to button click events here
  ButtonId id = BUTTON_ID_SELECT;  // The Select button

  window_single_click_subscribe(id, select_click_handler);

}



static void accel_raw_handler(AccelData *data, uint32_t num_samples)
{
  static char buffer[] = "XYZ: 9999 / 9999 / 9999";
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;
    // Largest expected inbox and outbox message sizes
const uint32_t inbox_size = 32;
const uint32_t outbox_size = 32;

// Open AppMessage
app_message_open(inbox_size, outbox_size);

  
  // Declare the dictionary's iterator
DictionaryIterator *out_iter;

// Prepare the outbox buffer for this message
AppMessageResult result = app_message_outbox_begin(&out_iter);
if(result == APP_MSG_OK) {
  // Add an item to ask for weather data
  
  dict_write_int16(out_iter, 1 , x);
  dict_write_int16(out_iter, 2 , y);
  dict_write_int16(out_iter, 3 , z);

  // Send this message
  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
  }
} else {
  // The outbox cannot be used right now
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
}
  dict_write_end(out_iter);
  snprintf(buffer, sizeof("XYZ: 9999 / 9999 / 9999"), "XYZ: %d / %d / %d", x, y, z);
  APP_LOG(APP_LOG_LEVEL_INFO, "x: %d, y: %d, z: %d",
                                                          x, y, z);
  text_layer_set_text(accel_layer, buffer);
  message = false;

 
}
//  static void inbox_received_callback(DictionaryIterator *iter, void *context) {
//   // A new message has been successfully received
// message = true;
//   accel_data_service_subscribe(1, accel_raw_handler);
// }
static void init()
{
  window = window_create();
  // Use this provider to add button click subscriptions
  window_set_click_config_provider(window, click_config_provider);
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, (WindowHandlers) handlers);
  window_stack_push(window, true);
//  app_message_register_inbox_received(inbox_received_callback);
  
  accel_data_service_subscribe(5, accel_raw_handler);

 // accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
}
 
static void deinit()
{
  window_destroy(window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}
