#include "simply_msg.h"

#include "simply_accel.h"
#include "simply_res.h"
#include "simply_stage.h"
#include "simply_menu.h"
#include "simply_ui.h"
#include "simply_window_stack.h"

#include "simply.h"

#include "util/dict.h"
#include "util/list1.h"
#include "util/math.h"
#include "util/memory.h"
#include "util/string.h"
#include "util/window.h"

#include <pebble.h>

#define SEND_DELAY_MS 10

typedef enum Command Command;

enum Command {
  CommandWindowShow = 1,
  CommandWindowHide,
  CommandWindowProps,
  CommandWindowButtonConfig,
  CommandWindowActionBar,
  CommandClick,
  CommandLongClick,
  CommandImagePacket,
  CommandCardClear,
  CommandCardText,
  CommandCardImage,
  CommandCardStyle,
  CommandVibe,
  CommandAccelPeek,
  CommandAccelConfig,
  CommandMenuClear,
  CommandMenuClearSection,
  CommandMenuProps,
  CommandMenuSection,
  CommandMenuItem,
  CommandMenuGetSelection,
  CommandMenuSetSelection,
  CommandStageClear,
  CommandElementInsert,
  CommandElementRemove,
  CommandElementCommon,
  CommandElementRadius,
  CommandElementText,
  CommandElementTextStyle,
  CommandElementImage,
  CommandElementAnimate,
};

typedef enum WindowType WindowType;

enum WindowType {
  WindowTypeWindow = 0,
  WindowTypeMenu,
  WindowTypeCard,
  WindowTypeLast,
};

typedef enum VibeType VibeType;

enum VibeType {
  VibeShort = 0,
  VibeLong = 1,
  VibeDouble = 2,
};

typedef struct Packet Packet;

struct __attribute__((__packed__)) Packet {
  Command type:16;
  uint16_t length;
};

typedef struct WindowShowPacket WindowShowPacket;

struct __attribute__((__packed__)) WindowShowPacket {
  Packet packet;
  WindowType type:8;
  bool pushing;
};

typedef struct WindowHidePacket WindowHidePacket;

struct __attribute__((__packed__)) WindowHidePacket {
  Packet packet;
  uint32_t id;
};

typedef struct WindowPropsPacket WindowPropsPacket;

struct __attribute__((__packed__)) WindowPropsPacket {
  Packet packet;
  uint32_t id;
  GColor background_color:8;
  bool fullscreen;
  bool scrollable;
};

typedef struct WindowButtonConfigPacket WindowButtonConfigPacket;

struct __attribute__((__packed__)) WindowButtonConfigPacket {
  Packet packet;
  uint8_t button_mask;
};

typedef struct WindowActionBarPacket WindowActionBarPacket;

struct __attribute__((__packed__)) WindowActionBarPacket {
  Packet packet;
  uint32_t image[3];
  GColor background_color:8;
  bool action;
};

typedef struct ClickPacket ClickPacket;

struct __attribute__((__packed__)) ClickPacket {
  Packet packet;
  ButtonId button:8;
};

typedef ClickPacket LongClickPacket;

typedef struct ImagePacket ImagePacket;

struct __attribute__((__packed__)) ImagePacket {
  Packet packet;
  uint32_t id;
  int16_t width;
  int16_t height;
  uint32_t pixels[];
};

typedef struct CardClearPacket CardClearPacket;

struct __attribute__((__packed__)) CardClearPacket {
  Packet packet;
  uint8_t flags;
};

typedef struct CardTextPacket CardTextPacket;

struct __attribute__((__packed__)) CardTextPacket {
  Packet packet;
  uint8_t index;
  char text[];
};

typedef struct CardImagePacket CardImagePacket;

struct __attribute__((__packed__)) CardImagePacket {
  Packet packet;
  uint32_t image;
  uint8_t index;
};

typedef struct CardStylePacket CardStylePacket;

struct __attribute__((__packed__)) CardStylePacket {
  Packet packet;
  uint8_t style;
};

typedef struct VibePacket VibePacket;

struct __attribute__((__packed__)) VibePacket {
  Packet packet;
  VibeType type:8;
};

typedef Packet AccelPeekPacket;

typedef struct AccelConfigPacket AccelConfigPacket;

struct __attribute__((__packed__)) AccelConfigPacket {
  Packet packet;
  uint16_t num_samples;
  AccelSamplingRate rate:8;
  bool data_subscribed;
};

typedef Packet MenuClearPacket;

typedef struct MenuClearSectionPacket MenuClearSectionPacket;

struct __attribute__((__packed__)) MenuClearSectionPacket {
  Packet packet;
  uint16_t section;
};

typedef struct MenuPropsPacket MenuPropsPacket;

struct __attribute__((__packed__)) MenuPropsPacket {
  Packet packet;
  uint16_t num_sections;
};

typedef struct MenuSectionPacket MenuSectionPacket;

struct __attribute__((__packed__)) MenuSectionPacket {
  Packet packet;
  uint16_t section;
  uint16_t num_items;
  uint16_t title_length;
  char title[];
};

typedef struct MenuItemPacket MenuItemPacket;

struct __attribute__((__packed__)) MenuItemPacket {
  Packet packet;
  uint16_t section;
  uint16_t item;
  uint32_t icon;
  uint16_t title_length;
  uint16_t subtitle_length;
  char buffer[];
};

typedef Packet MenuGetSelectionPacket;

typedef struct MenuSetSelectionPacket MenuSetSelectionPacket;

struct __attribute__((__packed__)) MenuSetSelectionPacket {
  Packet packet;
  uint16_t section;
  uint16_t item;
  MenuRowAlign align:8;
  bool animated;
};

typedef Packet StageClearPacket;

typedef struct ElementInsertPacket ElementInsertPacket;

struct __attribute__((__packed__)) ElementInsertPacket {
  Packet packet;
  uint32_t id;
  SimplyElementType type:8;
  uint16_t index;
};

typedef struct ElementRemovePacket ElementRemovePacket;

struct __attribute__((__packed__)) ElementRemovePacket {
  Packet packet;
  uint32_t id;
};

typedef struct ElementCommonPacket ElementCommonPacket;

struct __attribute__((__packed__)) ElementCommonPacket {
  Packet packet;
  uint32_t id;
  GRect frame;
  GColor background_color:8;
  GColor border_color:8;
};

typedef struct ElementRadiusPacket ElementRadiusPacket;

struct __attribute__((__packed__)) ElementRadiusPacket {
  Packet packet;
  uint32_t id;
  uint16_t radius;
};

typedef struct ElementTextPacket ElementTextPacket;

struct __attribute__((__packed__)) ElementTextPacket {
  Packet packet;
  uint32_t id;
  TimeUnits time_units:8;
  char text[];
};

typedef struct ElementTextStylePacket ElementTextStylePacket;

struct __attribute__((__packed__)) ElementTextStylePacket {
  Packet packet;
  uint32_t id;
  GColor color:8;
  GTextOverflowMode overflow_mode:8;
  GTextAlignment alignment:8;
  uint32_t custom_font;
  char system_font[];
};

typedef struct ElementImagePacket ElementImagePacket;

struct __attribute__((__packed__)) ElementImagePacket {
  Packet packet;
  uint32_t id;
  uint32_t image;
  GCompOp compositing:8;
};

typedef struct ElementAnimatePacket ElementAnimatePacket;

struct __attribute__((__packed__)) ElementAnimatePacket {
  Packet packet;
  uint32_t id;
  GRect frame;
  uint32_t duration;
  AnimationCurve curve:8;
};

typedef enum SimplyACmd SimplyACmd;

enum SimplyACmd {
  SimplyACmd_setWindow = 0,
  SimplyACmd_windowShow,
  SimplyACmd_windowHide,
  SimplyACmd_setUi,
  SimplyACmd_click,
  SimplyACmd_longClick,
  SimplyACmd_accelTap,
  SimplyACmd_vibe,
  SimplyACmd_accelData,
  SimplyACmd_getAccelData,
  SimplyACmd_configAccelData,
  SimplyACmd_configButtons,
  SimplyACmd_setMenu,
  SimplyACmd_setMenuSection,
  SimplyACmd_getMenuSection,
  SimplyACmd_setMenuItem,
  SimplyACmd_getMenuItem,
  SimplyACmd_menuSelect,
  SimplyACmd_menuLongSelect,
  SimplyACmd_menuSelection,
  SimplyACmd_image,
  SimplyACmd_setStage,
  SimplyACmd_stageElement,
  SimplyACmd_stageRemove,
  SimplyACmd_stageAnimate,
  SimplyACmd_stageAnimateDone,
};

typedef enum SimplySetUiParam SimplySetUiParam;

enum SimplySetUiParam {
  SetUi_clear = 1,
  SetUi_title,
  SetUi_subtitle,
  SetUi_body,
  SetUi_icon,
  SetUi_subicon,
  SetUi_image,
  SetUi_style,
};

typedef enum SimplySetMenuParam SimplySetMenuParam;

enum SimplySetMenuParam {
  SetMenu_clear = 1,
  SetMenu_sections,
  SetMenu_selectionSection,
  SetMenu_selectionItem,
  SetMenu_selectionAlign,
  SetMenu_selectionAnimated,
};

typedef enum ElementParam ElementParam;

enum ElementParam {
  ElementId = 1,
  ElementType,
  ElementIndex,
  ElementX,
  ElementY,
  ElementWidth,
  ElementHeight,
  ElementBackgroundColor,
  ElementBorderColor,
  ElementRadius,
  ElementText,
  ElementTextFont,
  ElementTextColor,
  ElementTextOverflow,
  ElementTextAlignment,
  ElementTextUpdateTimeUnit,
  ElementImage,
  ElementCompositing,
};

static bool s_has_communicated = false;

bool simply_msg_has_communicated() {
  return s_has_communicated;
}

static SimplyWindow *get_top_simply_window(Simply *simply) {
  Window *base_window = window_stack_get_top_window();
  SimplyWindow *window = window_get_user_data(base_window);
  if (!window || (void*) window == simply->splash) {
    return NULL;
  }
  return window;
}

static void handle_window_show_packet(Simply *simply, Packet *data) {
  WindowShowPacket *packet = (WindowShowPacket*) data;
  SimplyWindow *window = simply->windows[MIN(WindowTypeLast - 1, packet->type)];
  simply_window_stack_show(simply->window_stack, window, packet->pushing);
}

static void handle_window_hide_packet(Simply *simply, Packet *data) {
  WindowHidePacket *packet = (WindowHidePacket*) data;
  SimplyWindow *window = get_top_simply_window(simply);
  if (!window) {
    return;
  }
  if (window->id == packet->id) {
    simply_window_stack_pop(simply->window_stack, window);
  }
}

static void handle_window_props_packet(Simply *simply, Packet *data) {
  WindowPropsPacket *packet = (WindowPropsPacket*) data;
  SimplyWindow *window = get_top_simply_window(simply);
  if (!window) {
    return;
  }
  window->id = packet->id;
  simply_window_set_background_color(window, packet->background_color);
  simply_window_set_fullscreen(window, packet->fullscreen);
  simply_window_set_scrollable(window, packet->scrollable);
}

static void handle_window_button_config_packet(Simply *simply, Packet *data) {
  WindowButtonConfigPacket *packet = (WindowButtonConfigPacket*) data;
  SimplyWindow *window = get_top_simply_window(simply);
  if (!window) {
    return;
  }
  window->button_mask = packet->button_mask;
}

static void handle_window_action_bar_packet(Simply *simply, Packet *data) {
  WindowActionBarPacket *packet = (WindowActionBarPacket*) data;
  SimplyWindow *window = get_top_simply_window(simply);
  if (!window) {
    return;
  }
  for (unsigned int i = 0; i < ARRAY_LENGTH(packet->image); ++i) {
    simply_window_set_action_bar_icon(window, i, packet->image[i]);
  }
  simply_window_set_action_bar_background_color(window, packet->background_color);
  simply_window_set_action_bar(window, packet->action);
}

static void handle_image_packet(Simply *simply, Packet *data) {
  ImagePacket *packet = (ImagePacket*) data;
  simply_res_add_image(simply->res, packet->id, packet->width, packet->height, packet->pixels);
}

static void handle_card_clear_packet(Simply *simply, Packet *data) {
  CardClearPacket *packet = (CardClearPacket*) data;
  simply_ui_clear(simply->ui, packet->flags);
}

static void handle_card_text_packet(Simply *simply, Packet *data) {
  CardTextPacket *packet = (CardTextPacket*) data;
  simply_ui_set_text(simply->ui, MIN(NumUiTextfields - 1, packet->index), packet->text);
}

static void handle_card_image_packet(Simply *simply, Packet *data) {
  CardImagePacket *packet = (CardImagePacket*) data;
  simply->ui->ui_layer.imagefields[MIN(NumUiImagefields - 1, packet->index)] = packet->image;
  window_stack_schedule_top_window_render();
}

static void handle_card_style_packet(Simply *simply, Packet *data) {
  CardStylePacket *packet = (CardStylePacket*) data;
  simply_ui_set_style(simply->ui, packet->style);
}

static void handle_vibe_packet(Simply *simply, Packet *data) {
  VibePacket *packet = (VibePacket*) data;
  switch (packet->type) {
    case VibeShort: vibes_short_pulse(); break;
    case VibeLong: vibes_short_pulse(); break;
    case VibeDouble: vibes_double_pulse(); break;
  }
}

static void accel_peek_timer_callback(void *context) {
  Simply *simply = context;
  AccelData data = { .x = 0 };
  simply_accel_peek(simply->accel, &data);
  if (!simply_msg_accel_data(simply->msg, &data, 1, 0)) {
    app_timer_register(10, accel_peek_timer_callback, simply);
  }
}

static void handle_accel_peek_packet(Simply *simply, Packet *data) {
  app_timer_register(10, accel_peek_timer_callback, simply);
}

static void handle_accel_config_packet(Simply *simply, Packet *data) {
  AccelConfigPacket *packet = (AccelConfigPacket*) data;
  simply->accel->num_samples = packet->num_samples;
  simply->accel->rate = packet->rate;
  simply_accel_set_data_subscribe(simply->accel, packet->data_subscribed);
}

static void handle_menu_clear_packet(Simply *simply, Packet *data) {
  simply_menu_clear(simply->menu);
}

static void handle_menu_clear_section_packet(Simply *simply, Packet *data) {
  MenuClearSectionPacket *packet = (MenuClearSectionPacket*) data;
  simply_menu_clear_section_items(simply->menu, packet->section);
}

static void handle_menu_props_packet(Simply *simply, Packet *data) {
  MenuPropsPacket *packet = (MenuPropsPacket*) data;
  simply_menu_set_num_sections(simply->menu, packet->num_sections);
}

static void handle_menu_section_packet(Simply *simply, Packet *data) {
  MenuSectionPacket *packet = (MenuSectionPacket*) data;
  SimplyMenuSection *section = malloc(sizeof(*section));
  *section = (SimplyMenuSection) {
    .section = packet->section,
    .num_items = packet->num_items,
    .title = packet->title_length ? strdup2(packet->title) : NULL,
  };
  simply_menu_add_section(simply->menu, section);
}

static void handle_menu_item_packet(Simply *simply, Packet *data) {
  MenuItemPacket *packet = (MenuItemPacket*) data;
  SimplyMenuItem *item = malloc(sizeof(*item));
  *item = (SimplyMenuItem) {
    .section = packet->section,
    .item = packet->item,
    .title = packet->title_length ? strdup2(packet->buffer) : NULL,
    .subtitle = packet->subtitle_length ? strdup2(packet->buffer + packet->title_length + 1) : NULL,
    .icon = packet->icon,
  };
  simply_menu_add_item(simply->menu, item);
}

static void handle_menu_get_selection_packet(Simply *simply, Packet *data) {
  simply_msg_send_menu_selection(simply->msg);
}

static void handle_menu_set_selection_packet(Simply *simply, Packet *data) {
  MenuSetSelectionPacket *packet = (MenuSetSelectionPacket*) data;
  MenuIndex menu_index = {
    .section = packet->section,
    .row = packet->item,
  };
  simply_menu_set_selection(simply->menu, menu_index, packet->align, packet->animated);
}

static void handle_stage_clear_packet(Simply *simply, Packet *data) {
  simply_stage_clear(simply->stage);
}

static void handle_element_insert_packet(Simply *simply, Packet *data) {
  ElementInsertPacket *packet = (ElementInsertPacket*) data;
  SimplyElementCommon *element = simply_stage_auto_element(simply->stage, packet->id, packet->type);
  if (!element) {
    return;
  }
  simply_stage_insert_element(simply->stage, packet->index, element);
  simply_stage_update(simply->stage);
}

static void handle_element_remove_packet(Simply *simply, Packet *data) {
  ElementInsertPacket *packet = (ElementInsertPacket*) data;
  SimplyElementCommon *element = simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  simply_stage_remove_element(simply->stage, element);
  simply_stage_update(simply->stage);
}

static void handle_element_common_packet(Simply *simply, Packet *data) {
  ElementCommonPacket *packet = (ElementCommonPacket*) data;
  SimplyElementCommon *element = simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  simply_stage_set_element_frame(simply->stage, element, packet->frame);
  element->background_color = packet->background_color;
  element->border_color = packet->border_color;
  simply_stage_update(simply->stage);
}

static void handle_element_radius_packet(Simply *simply, Packet *data) {
  ElementRadiusPacket *packet = (ElementRadiusPacket*) data;
  SimplyElementRect *element = (SimplyElementRect*) simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  element->radius = packet->radius;
  simply_stage_update(simply->stage);
};

static void handle_element_text_packet(Simply *simply, Packet *data) {
  ElementTextPacket *packet = (ElementTextPacket*) data;
  SimplyElementText *element = (SimplyElementText*) simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  if (element->time_units != packet->time_units) {
    element->time_units = packet->time_units;
    simply_stage_update_ticker(simply->stage);
  }
  strset(&element->text, packet->text);
  simply_stage_update(simply->stage);
}

static void handle_element_text_style_packet(Simply *simply, Packet *data) {
  ElementTextStylePacket *packet = (ElementTextStylePacket*) data;
  SimplyElementText *element = (SimplyElementText*) simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  element->text_color = packet->color;
  element->overflow_mode = packet->overflow_mode;
  element->alignment = packet->alignment;
  if (packet->custom_font) {
    element->font = simply_res_get_font(simply->res, packet->custom_font);
  } else if (packet->system_font[0]) {
    element->font = fonts_get_system_font(packet->system_font);
  }
  simply_stage_update(simply->stage);
}

static void handle_element_image_packet(Simply *simply, Packet *data) {
  ElementImagePacket *packet = (ElementImagePacket*) data;
  SimplyElementImage *element = (SimplyElementImage*) simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  element->image = packet->image;
  element->compositing = packet->compositing;
  simply_stage_update(simply->stage);
}

static void handle_element_animate_packet(Simply *simply, Packet *data) {
  ElementAnimatePacket *packet = (ElementAnimatePacket*) data;
  SimplyElementCommon *element = simply_stage_get_element(simply->stage, packet->id);
  if (!element) {
    return;
  }
  SimplyAnimation *animation = malloc0(sizeof(*animation));
  animation->duration = packet->duration;
  animation->curve = packet->curve;
  simply_stage_animate_element(simply->stage, element, animation, packet->frame);
}

static void handle_packet(Simply *simply, uint8_t *buffer, uint16_t length) {
  Packet *packet = (Packet*) buffer;
  switch (packet->type) {
    case CommandWindowShow:
      handle_window_show_packet(simply, packet);
      break;
    case CommandWindowHide:
      handle_window_hide_packet(simply, packet);
      break;
    case CommandWindowProps:
      handle_window_props_packet(simply, packet);
      break;
    case CommandWindowButtonConfig:
      handle_window_button_config_packet(simply, packet);
      break;
    case CommandWindowActionBar:
      handle_window_action_bar_packet(simply, packet);
      break;
    case CommandClick:
      break;
    case CommandLongClick:
      break;
    case CommandImagePacket:
      handle_image_packet(simply, packet);
      break;
    case CommandCardClear:
      handle_card_clear_packet(simply, packet);
      break;
    case CommandCardText:
      handle_card_text_packet(simply, packet);
      break;
    case CommandCardImage:
      handle_card_image_packet(simply, packet);
      break;
    case CommandCardStyle:
      handle_card_style_packet(simply, packet);
      break;
    case CommandVibe:
      handle_vibe_packet(simply, packet);
      break;
    case CommandAccelPeek:
      handle_accel_peek_packet(simply, packet);
      break;
    case CommandAccelConfig:
      handle_accel_config_packet(simply, packet);
      break;
    case CommandMenuClear:
      handle_menu_clear_packet(simply, packet);
      break;
    case CommandMenuClearSection:
      handle_menu_clear_section_packet(simply, packet);
      break;
    case CommandMenuProps:
      handle_menu_props_packet(simply, packet);
      break;
    case CommandMenuSection:
      handle_menu_section_packet(simply, packet);
      break;
    case CommandMenuItem:
      handle_menu_item_packet(simply, packet);
      break;
    case CommandMenuGetSelection:
      handle_menu_get_selection_packet(simply, packet);
      break;
    case CommandMenuSetSelection:
      handle_menu_set_selection_packet(simply, packet);
      break;
    case CommandStageClear:
      handle_stage_clear_packet(simply, packet);
      break;
    case CommandElementInsert:
      handle_element_insert_packet(simply, packet);
      break;
    case CommandElementRemove:
      handle_element_remove_packet(simply, packet);
      break;
    case CommandElementCommon:
      handle_element_common_packet(simply, packet);
      break;
    case CommandElementRadius:
      handle_element_radius_packet(simply, packet);
      break;
    case CommandElementText:
      handle_element_text_packet(simply, packet);
      break;
    case CommandElementTextStyle:
      handle_element_text_style_packet(simply, packet);
      break;
    case CommandElementImage:
      handle_element_image_packet(simply, packet);
      break;
    case CommandElementAnimate:
      handle_element_animate_packet(simply, packet);
      break;
  }
}

static void received_callback(DictionaryIterator *iter, void *context) {
  Tuple *tuple = dict_find(iter, 0);
  if (!tuple) {
    return;
  }

  s_has_communicated = true;

  handle_packet(context, tuple->value->data, tuple->length);
}

static void dropped_callback(AppMessageResult reason, void *context) {
}

static void sent_callback(DictionaryIterator *iter, void *context) {
}

static void failed_callback(DictionaryIterator *iter, AppMessageResult reason, Simply *simply) {
  SimplyUi *ui = simply->ui;
  if (reason == APP_MSG_NOT_CONNECTED) {
    simply_ui_clear(ui, ~0);
    simply_ui_set_text(ui, UiSubtitle, "Disconnected");
    simply_ui_set_text(ui, UiBody, "Run the Pebble Phone App");
    simply_window_stack_show(simply->window_stack, &ui->window, true);
  }
}

SimplyMsg *simply_msg_create(Simply *simply) {
  SimplyMsg *self = malloc(sizeof(*self));
  *self = (SimplyMsg) { .simply = simply };

  simply->msg = self;

  const uint32_t size_inbound = 2048;
  const uint32_t size_outbound = 512;
  app_message_open(size_inbound, size_outbound);

  app_message_set_context(simply);

  app_message_register_inbox_received(received_callback);
  app_message_register_inbox_dropped(dropped_callback);
  app_message_register_outbox_sent(sent_callback);
  app_message_register_outbox_failed((AppMessageOutboxFailed) failed_callback);

  return self;
}

void simply_msg_destroy(SimplyMsg *self) {
  if (!self) {
    return;
  }

  app_message_deregister_callbacks();

  self->simply->msg = NULL;

  free(self);
}

static void destroy_packet(SimplyPacket *packet) {
  if (!packet) {
    return;
  }
  free(packet->buffer);
  packet->buffer = NULL;
  free(packet);
}

static bool send_msg(SimplyPacket *packet) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  if (packet->is_dict) {
    dict_copy_from_buffer(iter, packet->buffer, packet->length);
  } else {
    dict_write_data(iter, 0, packet->buffer, packet->length);
  }
  return (app_message_outbox_send() == APP_MSG_OK);
}

static void send_msg_retry(void *data) {
  SimplyMsg *self = data;
  SimplyPacket *packet = (SimplyPacket*) self->queue;
  if (!packet) {
    return;
  }
  if (!send_msg(packet)){
    self->send_delay_ms *= 2;
    app_timer_register(self->send_delay_ms, send_msg_retry, self);
    return;
  }
  list1_remove(&self->queue, &packet->node);
  destroy_packet(packet);
  self->send_delay_ms = SEND_DELAY_MS;
}

static SimplyPacket *add_packet(SimplyMsg *self, Packet *buffer, Command type, size_t length) {
  SimplyPacket *packet = malloc0(sizeof(*packet));
  if (!packet) {
    free(buffer);
    return NULL;
  }
  *buffer = (Packet) {
    .type = type,
    .length = length,
  };
  *packet = (SimplyPacket) {
    .length = length,
    .buffer = buffer,
  };
  list1_append(&self->queue, &packet->node);
  send_msg_retry(self);
  return packet;
}

static SimplyPacket *add_dict(SimplyMsg *self, void *buffer, size_t length) {
  SimplyPacket *packet = malloc0(sizeof(*packet));
  if (!packet) {
    free(buffer);
    return NULL;
  }
  *packet = (SimplyPacket) {
    .is_dict = true,
    .length = length,
    .buffer = buffer,
  };
  list1_append(&self->queue, &packet->node);
  send_msg_retry(self);
  return packet;
}

static bool send_click(SimplyMsg *self, Command type, ButtonId button) {
  size_t length;
  ClickPacket *packet = malloc0(length = sizeof(*packet));
  if (!packet) {
    return false;
  }
  packet->button = button;
  return add_packet(self, (Packet*) packet, type, length);
}

bool simply_msg_single_click(SimplyMsg *self, ButtonId button) {
  return send_click(self, CommandClick, button);
}

bool simply_msg_long_click(SimplyMsg *self, ButtonId button) {
  return send_click(self, CommandLongClick, button);
}

bool send_window(SimplyMsg *self, SimplyACmd type, uint32_t id) {
  size_t length = dict_calc_buffer_size(2, 1, 4);
  void *buffer = malloc0(length);
  if (!buffer) {
    return false;
  }
  DictionaryIterator iter;
  dict_write_begin(&iter, buffer, length);
  dict_write_uint8(&iter, 0, type);
  dict_write_uint32(&iter, 1, id);
  return add_dict(self, buffer, length);
}

bool simply_msg_window_show(SimplyMsg *self, uint32_t id) {
  return send_window(self, SimplyACmd_windowShow, id);
}

bool simply_msg_window_hide(SimplyMsg *self, uint32_t id) {
  return send_window(self, SimplyACmd_windowHide, id);
}

bool simply_msg_accel_tap(SimplyMsg *self, AccelAxisType axis, int32_t direction) {
  size_t length = dict_calc_buffer_size(3, 1, 1, 1);
  void *buffer = malloc0(length);
  if (!buffer) {
    return false;
  }
  DictionaryIterator iter;
  dict_write_begin(&iter, buffer, length);
  dict_write_uint8(&iter, 0, SimplyACmd_accelTap);
  dict_write_uint8(&iter, 1, axis);
  dict_write_int8(&iter, 2, direction);
  return add_dict(self, buffer, length);
}

bool simply_msg_accel_data(SimplyMsg *self, AccelData *data, uint32_t num_samples, int32_t transaction_id) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  dict_write_uint8(iter, 0, SimplyACmd_accelData);
  if (transaction_id >= 0) {
    dict_write_int32(iter, 1, transaction_id);
  }
  dict_write_uint8(iter, 2, num_samples);
  dict_write_data(iter, 3, (uint8_t*) data, sizeof(*data) * num_samples);
  return (app_message_outbox_send() == APP_MSG_OK);
}

static void write_menu_item(DictionaryIterator *iter, SimplyACmd type, uint16_t section, uint16_t index) {
  dict_write_uint8(iter, 0, type);
  dict_write_uint16(iter, 1, section);
  dict_write_uint16(iter, 2, index);
}

static bool send_menu_item(SimplyMsg *self, SimplyACmd type, uint16_t section, uint16_t index) {
  DictionaryIterator *iter = NULL;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    return false;
  }
  write_menu_item(iter, type, section, index);
  return (app_message_outbox_send() == APP_MSG_OK);
}

static bool send_menu_item_retry(SimplyMsg *self, SimplyACmd type, uint16_t section, uint16_t index) {
  size_t length = dict_calc_buffer_size(3, 1, 2, 2);
  void *buffer = malloc0(length);
  if (!buffer) {
    return false;
  }
  DictionaryIterator iter;
  dict_write_begin(&iter, buffer, length);
  write_menu_item(&iter, type, section, index);
  return add_dict(self, buffer, length);
}

bool simply_msg_menu_get_section(SimplyMsg *self, uint16_t index) {
  return send_menu_item(self, SimplyACmd_getMenuSection, index, 0);
}

bool simply_msg_menu_get_item(SimplyMsg *self, uint16_t section, uint16_t index) {
  return send_menu_item(self, SimplyACmd_getMenuItem, section, index);
}

bool simply_msg_menu_select_click(SimplyMsg *self, uint16_t section, uint16_t index) {
  return send_menu_item_retry(self, SimplyACmd_menuSelect, section, index);
}

bool simply_msg_menu_select_long_click(SimplyMsg *self, uint16_t section, uint16_t index) {
  return send_menu_item_retry(self, SimplyACmd_menuLongSelect, section, index);
}

bool simply_msg_send_menu_selection(SimplyMsg *self) {
  MenuIndex menu_index = simply_menu_get_selection(self->simply->menu);
  return send_menu_item_retry(self, SimplyACmd_menuSelection, menu_index.section, menu_index.row);
}

bool simply_msg_animate_element_done(SimplyMsg *self, uint16_t index) {
  size_t length = dict_calc_buffer_size(2, 1, 2);
  void *buffer = malloc0(length);
  if (!buffer) {
    return false;
  }
  DictionaryIterator iter;
  dict_write_begin(&iter, buffer, length);
  dict_write_uint8(&iter, 0, SimplyACmd_stageAnimateDone);
  dict_write_uint16(&iter, 1, index);
  return add_dict(self, buffer, length);
}

