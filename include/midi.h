#ifndef SOUND_MIDI_H
#define SOUND_MIDI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#define MIDI_MESSAGES \
  /* Control Change, 0b1011nnnn, next byte 0b0XXXXXXX mapped to 0x0000-0x007F inclusive */ \
  X(MIDI_MESSAGE_BANK_SELECT_MSB) \
  X(MIDI_MESSAGE_MODULATION_WHEEL_MSB) \
  X(MIDI_MESSAGE_BREATH_CONTROL_MSB) \
  X(UNDEFINED_4) \
  X(MIDI_MESSAGE_FOOT_CONTROLLER_MSB) \
  X(MIDI_MESSAGE_PORTAMENTO_TIME_MSB) \
  X(MIDI_MESSAGE_DATA_ENTRY_MSB) \
  X(MIDI_MESSAGE_CHANNEL_VOLUME_MSB) \
  X(MIDI_MESSAGE_BALANCE_MSB) \
  X(UNDEFINED_10) \
  X(MIDI_MESSAGE_PAN_MSB) \
  X(MIDI_MESSAGE_EXPRESSION_CONTROLLER_MSB) \
  X(MIDI_MESSAGE_EFFECT_CONTROL_MSB_1) \
  X(MIDI_MESSAGE_EFFECT_CONTROL_MSB_2) \
  X(UNDEFINED_15) \
  X(UNDEFINED_16) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_MSB_1) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_MSB_2) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_MSB_3) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_MSB_4) \
  X(UNDEFINED_21) \
  X(UNDEFINED_22) \
  X(UNDEFINED_23) \
  X(UNDEFINED_24) \
  X(UNDEFINED_25) \
  X(UNDEFINED_26) \
  X(UNDEFINED_27) \
  X(UNDEFINED_28) \
  X(UNDEFINED_29) \
  X(UNDEFINED_30) \
  X(UNDEFINED_31) \
  X(UNDEFINED_32) \
  \
  X(MIDI_MESSAGE_BANK_SELECT_LSB) /* 0x00 */ \
  X(MIDI_MESSAGE_MODULATION_WHEEL_LSB) \
  X(MIDI_MESSAGE_BREATH_CONTROL_LSB) \
  X(UNDEFINED_36) \
  X(MIDI_MESSAGE_FOOT_CONTROLLER_LSB) \
  X(MIDI_MESSAGE_PORTAMENTO_TIME_LSB) \
  X(MIDI_MESSAGE_DATA_ENTRY_LSB) \
  X(MIDI_MESSAGE_CHANNEL_VOLUME_LSB) \
  X(MIDI_MESSAGE_BALANCE_LSB) \
  X(UNDEFINED_42) \
  X(MIDI_MESSAGE_PAN_LSB) \
  X(MIDI_MESSAGE_EXPRESSION_CONTROLLER_LSB) \
  X(MIDI_MESSAGE_EFFECT_CONTROL_LSB_1) \
  X(MIDI_MESSAGE_EFFECT_CONTROL_LSB_2) \
  X(UNDEFINED_47) \
  X(UNDEFINED_48) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_LSB_1) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_LSB_2) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_LSB_3) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_LSB_4) \
  X(UNDEFINED_53) \
  X(UNDEFINED_54) \
  X(UNDEFINED_55) \
  X(UNDEFINED_56) \
  X(UNDEFINED_57) \
  X(UNDEFINED_58) \
  X(UNDEFINED_59) \
  X(UNDEFINED_60) \
  X(UNDEFINED_61) \
  X(UNDEFINED_62) \
  X(UNDEFINED_63) \
  X(UNDEFINED_64) \
  \
  X(MIDI_MESSAGE_DAMPER_PEDAL) /* ON/OFF (SUSTAIN) */ \
  X(MIDI_MESSAGE_PORTAMENTO) /* ON/OFF */ \
  X(MIDI_MESSAGE_SUSTENUTO) /* ON/OFF */ \
  X(MIDI_MESSAGE_SOFT_PEDAL) /* ON/OFF */ \
  X(MIDI_MESSAGE_LEGATO_FOOTSWITCH) \
  X(MIDI_MESSAGE_HOLD_2) \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_1) /* (SOUND_VARIATION) */ \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_2) /* (TIMBRE) */ \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_3) /* (RELEASE_TIME) */ \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_4) /* (ATTACK_TIME) */ \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_5) /* (BRIGHTNESS) */ \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_6) \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_7) \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_8) \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_9) \
  X(MIDI_MESSAGE_SOUND_CONTROLLER_10) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_5) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_6) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_7) \
  X(MIDI_MESSAGE_GENERAL_PURPOSE_CONTROLLER_8) \
  X(MIDI_MESSAGE_PORTAMENTO_CONTROL) \
  X(UNDEFINED_86) \
  X(UNDEFINED_87) \
  X(UNDEFINED_88) \
  X(UNDEFINED_89) \
  X(UNDEFINED_90) \
  X(UNDEFINED_91) \
  X(MIDI_MESSAGE_EFFECTS_1_DEPTH) \
  X(MIDI_MESSAGE_EFFECTS_2_DEPTH) \
  X(MIDI_MESSAGE_EFFECTS_3_DEPTH) \
  X(MIDI_MESSAGE_EFFECTS_4_DEPTH) \
  X(MIDI_MESSAGE_EFFECTS_5_DEPTH) \
  \
  X(MIDI_MESSAGE_DATA_ENTRY_p1) \
  X(MIDI_MESSAGE_DATA_ENTRY_m1) \
  X(MIDI_MESSAGE_NON_REGISTERED_PARAMETER_NUMBER_LSB) \
  X(MIDI_MESSAGE_NON_REGISTERED_PARAMETER_NUMBER_MSB) \
  X(MIDI_MESSAGE_REGISTERED_PARAMETER_NUMBER_LSB) \
  X(MIDI_MESSAGE_REGISTERED_PARAMETER_NUMBER_MSB) \
  X(UNDEFINED_103) \
  X(UNDEFINED_104) \
  X(UNDEFINED_105) \
  X(UNDEFINED_106) \
  X(UNDEFINED_107) \
  X(UNDEFINED_108) \
  X(UNDEFINED_109) \
  X(UNDEFINED_110) \
  X(UNDEFINED_111) \
  X(UNDEFINED_112) \
  X(UNDEFINED_113) \
  X(UNDEFINED_114) \
  X(UNDEFINED_115) \
  X(UNDEFINED_116) \
  X(UNDEFINED_117) \
  X(UNDEFINED_118) \
  X(UNDEFINED_119) \
  X(UNDEFINED_120) \
  \
  X(MIDI_MESSAGE_ALL_SOUND_OFF) \
  X(MIDI_MESSAGE_RESET_ALL_CONTROLLERS) \
  X(MIDI_MESSAGE_LOCAL_CONTROL)    /* ON/OFF */ \
  X(MIDI_MESSAGE_ALL_NOTES_OFF) \
  X(MIDI_MESSAGE_OMNI_MODE_OFF)    /* (+ ALL_NOTES_OFF) */ \
  X(MIDI_MESSAGE_OMNI_MODE_ON)     /* (+ ALL_NOTES_OFF) */ \
  X(MIDI_MESSAGE_POLY_MODE_ON_OFF) /* (+ ALL_NOTES_OFF) */ \
  X(MIDI_MESSAGE_POLY_MODE_ON)     /* (INCL_MONO=OFF_+ALL_NOTES OFF) */ \
  \
  /* 0b1XXXnnnn  Channel Voice Messages. Mapped to 0x80-0x87 inclusive */ \
  X(MIDI_MESSAGE_NOTE_OFF_EVENT) \
  X(MIDI_MESSAGE_NOTE_ON_EVENT) \
  X(MIDI_MESSAGE_POLYPHONIC_KEY_PRESSURE) /* (Aftertouch). */ \
  X(MIDI_MESSAGE_CONTROL_CHANGE) /* 0b1011nnnn  Control Change / Channel Modes. Won't be emmited. I give those their own message entry, see above. */ \
  X(MIDI_MESSAGE_PROGRAM_CHANGE) \
  X(MIDI_MESSAGE_CHANNEL_PRESSURE) /* (After-touch) */ \
  X(MIDI_MESSAGE_PITCH_WHEEL_CHANGE) \
  X(UNDEFINED) /* 0x1111  System Common Messages, 0b1111XXXX */ \
  \
  /* Nothing, just 8 unused values */ \
  X(UNDEFINED_137) \
  X(UNDEFINED_138) \
  X(UNDEFINED_139) \
  X(UNDEFINED_140) \
  X(UNDEFINED_141) \
  X(UNDEFINED_142) \
  X(UNDEFINED_143) \
  X(UNDEFINED_144) \
  \
  /* 0b1111XXXX  Mapped to 0x90-0x9F inclusive */ \
  /* System Common Messages. 0x90-0x97 inclusive */ \
  X(MIDI_MESSAGE_SYSTEM_EXCLUSIVE) \
  X(UNDEFINED_146) \
  X(MIDI_MESSAGE_SONG_POSITION_POINTER) \
  X(MIDI_MESSAGE_SONG_SELECT) \
  X(UNDEFINED_149) \
  X(UNDEFINED_150) \
  X(MIDI_MESSAGE_TUNE_REQUEST) \
  X(MIDI_MESSAGE_END_OF_EXCLUSIVE) \
  \
  /* System Real-Time Messages. 0x98-0x9F inclusive */ \
  X(MIDI_MESSAGE_TIMING_CLOCK) \
  X(UNDEFINED_154) \
  X(MIDI_MESSAGE_START) \
  X(MIDI_MESSAGE_CONTINUE) \
  X(MIDI_MESSAGE_STOP) \
  X(UNDEFINED_158) \
  X(MIDI_MESSAGE_ACTIVE_SENSING) \
  X(MIDI_MESSAGE_RESET) /* This is the reset message, but in midi files, it's instead reused for meta events... */ \
  \
  \
  /* meta events: 0-X(127) mapped to 0xA0 to 0x11F */ \
  X(MIDI_MESSAGE_META_SEQUENCE_NUMBER) \
  X(MIDI_MESSAGE_META_TEXT_EVENT) \
  X(MIDI_MESSAGE_META_COPYRIGHT_NOTICE) \
  X(MIDI_MESSAGE_META_SEQUENCE_TRACK_NAME) \
  X(MIDI_MESSAGE_META_INSTRUMENT_NAME) \
  X(MIDI_MESSAGE_META_LYRIC) \
  X(MIDI_MESSAGE_META_MARKER) \
  X(MIDI_MESSAGE_META_CUE_POINT)

enum midi_message {
#define X(N) N,
  MIDI_MESSAGES
#undef X
  MIDI_MESSAGE_COUNT,
  MIDI_MESSAGE_META_MIDI_CHANNEL_PREFIX = 0xA0 + 0x20, \
  MIDI_MESSAGE_META_END_OF_TRACK = 0xA0 + 0x2F, \
  MIDI_MESSAGE_META_SET_TEMPO = 0xA0 + 0x51, /* (in microseconds per MIDI quarter-note) */ \
  MIDI_MESSAGE_META_SMPTE_OFFSET = 0xA0 + 0x54, \
  MIDI_MESSAGE_META_TIME_SIGNATURE = 0xA0 + 0x58, \
  MIDI_MESSAGE_META_KEY_SIGNATURE, \
  MIDI_MESSAGE_META_SEQUENCER_SPECIFIC = 0xA0 + 0x7F,
};

extern const char*const midi_message_s[MIDI_MESSAGE_COUNT];

static inline const char* lookup_midi_message(enum midi_message m){
  if(m >= MIDI_MESSAGE_COUNT || m < 0)
    return "INVALID";
  return midi_message_s[m];
}

enum midi_channel {
  MIDI_CHANNEL_NONE,
  MIDI_CHANNEL_1,
  MIDI_CHANNEL_2,
  MIDI_CHANNEL_3,
  MIDI_CHANNEL_4,
  MIDI_CHANNEL_5,
  MIDI_CHANNEL_6,
  MIDI_CHANNEL_7,
  MIDI_CHANNEL_8,
  MIDI_CHANNEL_9,
  MIDI_CHANNEL_10,
  MIDI_CHANNEL_11,
  MIDI_CHANNEL_12,
  MIDI_CHANNEL_13,
  MIDI_CHANNEL_14,
  MIDI_CHANNEL_15,
  MIDI_CHANNEL_16,
};

static_assert(MIDI_MESSAGE_NOTE_OFF_EVENT       == 0x80, "MIDI_MESSAGE_NOTE_OFF_EVENT has unexpected value");
static_assert(MIDI_MESSAGE_SYSTEM_EXCLUSIVE     == 0x90, "MIDI_MESSAGE_SYSTEM_EXCLUSIVE has unexpected value");
static_assert(MIDI_MESSAGE_TIMING_CLOCK         == 0x98, "MIDI_MESSAGE_TIMING_CLOCK has unexpected value");
static_assert(MIDI_MESSAGE_META_SEQUENCE_NUMBER == 0xA0, "MIDI_MESSAGE_META_SEQUENCE_NUMBER has unexpected value");

struct midi_event {
  enum midi_message type;
  uint32_t len;
  const void* data;
  enum midi_channel channel;
  uint64_t time;
};

struct midi_event_parser {
  uint64_t time;
  uint32_t tmp;
  uint8_t state;
  bool has_timing;
  struct midi_event event;
  bool got_event;
};

#define midi_message_has_channel(X) ((X) < 0x90)

ssize_t midi_event_parser_parse(struct midi_event_parser* midi, size_t len, const uint8_t data[len], bool eof);

#endif