#include <midi.h>
#include <ringbuffer.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

const char* note_name[] = {"c","c#","d","d#","e","f","f#","g","g#","a","a#","h"};

struct note {
  bool done;
  enum midi_channel channel;
  uint64_t time;
  uint64_t duration;
  uint8_t tone;
  uint8_t velocity_start;
  uint8_t velocity_end;
};

unsigned note_count = 0, note_offset = 0;
#define NOTE_INDEX_MASK 0xFF
struct note note_list[NOTE_INDEX_MASK+1];

bool start_note(enum midi_channel channel, uint64_t time, uint8_t tone, uint8_t velocity_start){
  if(note_count >= sizeof(note_list)/sizeof(*note_list))
    return false;
  note_list[(note_offset + note_count++) & NOTE_INDEX_MASK] = (struct note){
    .channel = channel,
    .time = time,
    .tone = tone,
    .velocity_start = velocity_start,
  };
  return true;
}

bool stop_note(enum midi_channel channel, uint64_t time, uint8_t tone, uint8_t velocity_end){
  for(unsigned i=note_offset,n=note_offset+note_count; i<n; i++){
    {
      struct note*restrict const note = &note_list[i & NOTE_INDEX_MASK];
      if(note->done || note->channel != channel || note->tone != tone)
        continue;
      note->done = true;
      note->duration = time - note->time;
      note->velocity_end = velocity_end;
    }
    while(note_count && note_list[note_offset & NOTE_INDEX_MASK].done){
      struct note*restrict const note = &note_list[note_offset & NOTE_INDEX_MASK];
      static uint64_t last_time = 0;
      static uint64_t natural_duration = 0;
      static unsigned notes_same_time = 0; // Note, this is only the ones starting at the same time, there may still be some active.
      const uint64_t diff = note->time - last_time;
      notes_same_time += 1;
      if(diff){
        printf(notes_same_time <= 1 ? " " : "\n");
        notes_same_time = 0;
        if(natural_duration == diff){
          printf(">>");
        }else{
          printf(">> %llu", (long long unsigned)diff);
        }
        natural_duration = natural_duration > diff ? natural_duration - diff : 0;
        last_time = note->time;
      }
      if(natural_duration < note->duration)
        natural_duration = note->duration;
      printf("\nn  %-2s %d  %4u", note_name[note->tone % 12], note->tone/12-2, (unsigned)note->duration);
      note_count -= 1;
      note_offset += 1;
    }
    return true;
  }
  return false;
}

int main(int argc, char* argv[]){
  setbuf(stdout, 0);
  puts(
    ":tune c 4 261.63\n"
    ":intonation equal\n"
    ":speed 1\n"
    ":tempo 2ms\n"
    "\n"
  );
  struct ringbuffer* rb = ringbuffer_create();
  if(!rb){
    fprintf(stderr, "ringbuffer_create failed");
    return 1;
  }
  struct midi_event_parser mep = {
    .has_timing = true
  };
  while(true){
    while(true){
      const struct buffer_wo wo = ringbuffer_get_write_buffer(rb);
      if(!wo.length) break;
      ssize_t s = read(0, wo.v, wo.length);
      if(!s) break;
      if(s == -1){
        if(errno == EINTR)
          continue;
        perror("read failed");
        return 1;
      }
      ringbuffer_commit(rb, s);
    }
    const struct buffer_ro ro = ringbuffer_get_read_buffer(rb);
    ssize_t res = midi_event_parser_parse(&mep, ro.length, ro.v, !ro.length);
    if(res < 0){
      fprintf(stderr, "midi_event_parser_parse failed\n");
      return 1;
    }
    if(mep.got_event){
      const struct midi_event*restrict const e = &mep.event;
      fprintf(stderr, "%u dispatch_midi_event 0x%02X %u %d %s\n", (unsigned)mep.time, e->type, e->len, e->channel, lookup_midi_message(e->type));
      switch(e->type){
        case MIDI_MESSAGE_NOTE_ON_EVENT: {
          if(e->len < 2){
            fprintf(stderr, "Invalid MIDI note on message, expected at least 2 data bytes (the note and \"velocity\" (essentially gain/volume))\n");
          }else{
            fprintf(stderr, "  %d %d\n", ((uint8_t*)e->data)[0], ((uint8_t*)e->data)[1]);
            if(!start_note(e->channel, mep.time, ((uint8_t*)e->data)[0], ((uint8_t*)e->data)[1])){
              fprintf(stderr, "Note overflow! Channel %u Note %u\n", note_list[note_offset & NOTE_INDEX_MASK].channel, note_list[note_offset & NOTE_INDEX_MASK].tone);
              return 1;
            }
          }
        } break;
        case MIDI_MESSAGE_NOTE_OFF_EVENT: {
          if(e->len < 2)
            fprintf(stderr, "Invalid MIDI note on message, expected at least 2 data bytes (the note and \"velocity\" (essentially gain/volume))\n");
          if(e->len >= 1){
            int note = ((uint8_t*)e->data)[0];
            int velocity = e->len >= 2 ? ((uint8_t*)e->data)[1] : 127;
            fprintf(stderr, "  %d %d\n", note, velocity);
            if(!stop_note(e->channel, mep.time, ((uint8_t*)e->data)[0], ((uint8_t*)e->data)[1])){
              fprintf(stderr, "Note not found!\n");
            }
          }
        } break;
        default: break;
      }
    }
    if(!ro.length) break;
    if(!res){
      fprintf(stderr, "midi_event_parser_parse failed to progress\n");
      return 1;
    }
    ringbuffer_discard(rb, res);
  }
  printf("\n");
  ringbuffer_destroy(rb);
  return 0;
}
