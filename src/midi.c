#include <midi.h>
#include <stdio.h>

const char*const midi_message_s[MIDI_MESSAGE_COUNT] = {
#define X(N) [N] = #N,
  MIDI_MESSAGES
#undef X
};

static inline int parse_variable_length_quantity(size_t len, const uint8_t data[len], uint32_t* res){
  if(len > 4)
    len = 4;
  uint32_t result = 0;
  for(unsigned i=0; i<len; i++){
    uint8_t byte = data[i];
    result <<= 7;
    result |= byte & 0x7F;
    if(!(byte & 0x80)){
      *res = result;
      return i + 1;
    }
  }
  if(len == 4)
    return -1;
  return 0;
}

static inline void dispatch_midi_event(struct midi_event_parser* midi, enum midi_message type, uint32_t len, const uint8_t data[len], enum midi_channel channel){
  midi->event = (struct midi_event){
    .type = type,
    .time = midi->time,
    .len = len,
    .data = data,
    .channel = channel,
  };
  midi->got_event = true;
}

enum parser_state {
  PS_TIMING,
  PS_EVENT_TYPE,
  PS_EVENT_SYSEX,
  PS_SKIP,
  PS_EVENT_META,
};

ssize_t midi_event_parser_parse(struct midi_event_parser* midi, size_t len, const uint8_t data[len], bool eof){
  midi->got_event = false;
  const size_t old_len = len;
  bool got_next = false;
#define NEXT(...) { got_next=true; dispatch_midi_event(midi, __VA_ARGS__); }
  while(len && !got_next){

    switch((enum parser_state)midi->state){
      case PS_TIMING: {
        if(midi->has_timing){
          uint32_t timing = 0;
          int vlen = parse_variable_length_quantity(len, data, &timing);
          if(vlen == -1)
            return -1;
          if(vlen == 0)
            goto out;
          midi->time += timing;
          data += vlen;
          len  -= vlen;
        }
        midi->state = PS_EVENT_TYPE;
      } break;

      case PS_EVENT_TYPE: {
        if(len <= 1)
          goto out;
        uint8_t type = data[0];
        if(!(type & 0x80))
          return -1;
        if(type == 0xF7 || type == 0xF0){
          midi->state = PS_EVENT_SYSEX;
          midi->tmp = type == 0xF0 ? MIDI_MESSAGE_SYSTEM_EXCLUSIVE : MIDI_MESSAGE_END_OF_EXCLUSIVE;
          len -= 1;
          data += 1;
        }else if(type == 0xFF && midi->has_timing){
          if(len < 2)
            goto out;
          midi->state = PS_EVENT_META;
          if(data[1] & 0x80)
            return -1;
          midi->tmp = data[1] + 0xA0;
          len  -= 2;
          data += 2;
        }else if((type & 0xF0) != 0xF0){
          uint8_t channel = type & 0x0F;
          enum midi_message message = ((type & 0x70)>>4) | 0x80;
          uint32_t needed = (message == MIDI_MESSAGE_PROGRAM_CHANGE || message == MIDI_MESSAGE_CHANNEL_PRESSURE) ? 2 : 3;
          if(needed > len)
            goto out;
          if(message == MIDI_MESSAGE_CONTROL_CHANGE){
            message = data[1];
            if(message & 0x80)
              return -1;
            NEXT(message, 1, &data[2], MIDI_CHANNEL_1 + channel);
          }else{
            NEXT(message, needed-1, data+1, MIDI_CHANNEL_1 + channel);
          }
          len -= needed;
          data += needed;
          midi->state = PS_TIMING;
        }else{
          enum midi_message message = (type & 0x0F) | 0x90;
          uint32_t i, l;
          for(i=1,l=(len<32?len:32); i<l; i++)
            if(data[i] & 0x80)
              break;
          if(i == 32)
            return -1;
          if(i == len && !eof)
            goto out;
          NEXT(message, i-1, data+1, MIDI_CHANNEL_NONE);
          data += i;
          len  -= i;
          midi->state = PS_TIMING;
        }
      }; break;

      case PS_EVENT_META:
      case PS_EVENT_SYSEX: {
        if(len < 1)
          goto out;
        uint32_t dlen = 0;
        int vlen = parse_variable_length_quantity(len, data, &dlen);
        if(vlen == -1)
          return -1;
        if(vlen == 0)
          goto out;
        if(dlen > 254){
          midi->state = PS_SKIP;
          len += vlen;
          data -= vlen;
          midi->tmp = dlen;
        }else{
          if(len-vlen < dlen)
            goto out;
          data += vlen;
          len  -= vlen;
          NEXT(midi->tmp, dlen, data, MIDI_CHANNEL_NONE);
          data += dlen;
          len  -= dlen;
          midi->state = PS_TIMING;
        }
      } break;

      case PS_SKIP: {
        if(len < (size_t)midi->tmp){
          midi->tmp -= len;
          data += len;
          len = 0;
        }else{
          len  -= midi->tmp;
          data += midi->tmp;
          midi->state = PS_TIMING;
        }
      } break;

    }
#undef NEXT
  }

out:
  return old_len - len;
}
