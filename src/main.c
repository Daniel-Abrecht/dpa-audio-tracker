#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <attr/xattr.h>

#define COMMON_SAMPLE_RATE_44_1 44100
#define COMMON_SAMPLE_RATE_48   48000

#ifndef M_PIl
#define M_PIl 3.141592653589793238462643383279502884L
#endif
#define C_2_POW_1_12 1.0594630943592952645618252949463417007792043174941856285592084314L

enum output_format {
  F_FLOAT_64,
  F_FLOAT_32,
  F_INT_32,
};

struct settings {
  long double c4;
  long double tempo;
  long double speed;
  const struct intonation* intonation;
};

struct note {
  const char* name;
  long double factor;
};

struct intonation {
  const char* name;
  size_t note_count;
  const struct note* note_map;
};

enum intonation_index {
  INTONATION_EQUAL,
  INTONATION_PYTHAGOREAN,
  INTONATION_JUST,
  INTONATION_MEAN_TONE_FIFTH,
};

const struct intonation intonation[] = {
  {
    .name = "equal",
    .note_count = 12,
    .note_map = (const struct note[]){
      {"c" , 1},
      {"c#", C_2_POW_1_12},
      {"d" , C_2_POW_1_12 * C_2_POW_1_12},
      {"d#", C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"e" , C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"f" , C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"f#", C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"g" , C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"g#", C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"a" , C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"a#", C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
      {"h" , C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12 * C_2_POW_1_12},
    },
  },
  {
    .name = "pythagorean",
    .note_count = 7,
    .note_map = (const struct note[]){
      {"c",         1},
      {"d",   9.l/  8},
      {"e",  81.l/ 64},
      {"f",   4.l/  3},
      {"g",   3.l/  2},
      {"a",  27.l/ 16},
      {"h", 143.l/128},
    },
  },
  {
    .name = "just",
    .note_count = 7,
    .note_map = (const struct note[]){
      {"c",      1},
      {"d",  9.l/8},
      {"e",  5.l/4},
      {"f",  4.l/3},
      {"g",  3.l/2},
      {"a",  5.l/3},
      {"h", 15.l/8},
    },
  },
  {
    .name = "mean-tone-fifth",
    .note_count = 12,
    .note_map = (const struct note[]){
      {"c" , 1},
      {"c#", 1.0449067265256594125050516769666374006063049869569961949530991455L},
      {"d" , 1.1180339887498948482045868343656381177203091798057628621354486227L},
      {"d#", 1.1962790249769764335295191953127307162907678060917650764132748000L},
      {"e" , 5.L/4},
      {"f" , 1.3374806099528440480064661465172958727760703833049551295399669062L},
      {"f#", 1.3975424859373685602557335429570476471503864747572035776693107783L},
      {"g" , 1.4953487812212205419118989941409133953634597576147063455165935000L},
      {"g#", 8.L/5},
      {"a" , 1.6718507624410550600080826831466198409700879791311939119249586328L},
      {"a#", 1.7888543819998317571273389349850209883524946876892205794167177963L},
      {"h" , 1.8691859765265256773898737426761417442043246970183829318957418750L},
    },
  },
};
enum { INTONATION_COUNT = sizeof(intonation) / sizeof(*intonation) };

typedef int16_t sample_generator_t(long double f);
extern sample_generator_t sg_sin;

struct wav_header { unsigned char data[44]; };
struct wav_header mk_wav(uint32_t channels, uint16_t sample_rate, enum output_format format){
  const uint16_t bits_per_sample = format == F_FLOAT_64 ? 64 : 32;
  const uint64_t sbcb = ((int64_t)sample_rate * bits_per_sample * channels + 7) / 8;
  const uint64_t bcb = ((int64_t)bits_per_sample * channels + 7) / 8;
  const bool isfloat = format != F_INT_32;
  struct wav_header h = {{
    'R','I','F','F',
     ~0, ~0, ~0, ~0, // file size
    'W','A','V','E',
    'f','m','t',' ',
     16,  0,  0,  0,
    isfloat?3:1,  0,
    channels, channels>>8,
    sample_rate, sample_rate>>8, sample_rate>>16, sample_rate>>24,
    sbcb, sbcb>>8, sbcb>>16, sbcb>>24,
    bcb, bcb>>8,
    bits_per_sample, bits_per_sample>>8,
    'd','a','t','a',
     ~0, ~0, ~0, ~0, // data size (file size - 44)
  }};
  return h;
}

long double parse_time(const struct settings*const s, const char*const restrict input);

int16_t sg_sin(long double f){
  return sinl(2 * M_PIl * f) * 0x7FFFu;
}

int16_t sg_triangle(long double f){
  int32_t x = (int32_t)0x7FFFl*4 * f;
  if(x > 0x7FFF)
    x = -(x-0x7FFF);
  if(x < 0x7FFF)
    x = -(x+0x7FFF);
  return x;
}

int16_t sg_square(long double f){
  return f > 0.5 ? 0x7FFF : -0x7FFF;
}

struct tracker {
  struct settings settings;
  struct {
    int32_t min;
    int32_t max;
    uint64_t samples_total;
    long double square_sum, abs_sum;
  } stats;
  unsigned long line;
  enum output_format format;
  uint32_t samples_per_second;
  struct generator* generator_list;
};

struct tone {
  const struct tracker* tracker;
  sample_generator_t* waveform;
  uint32_t duration; // in samples
  uint32_t phase;    // in samples
};

struct generator {
  struct generator* next;
  uint64_t duration;
  uint64_t time;
  struct tone tone;
};

int16_t tone_get_sample(struct tone* tone){
  uint32_t phase = tone->phase + 1;
  if(phase >= tone->duration)
    phase = 0;
  tone->phase = phase;
  return tone->waveform((long double)tone->phase / tone->duration);
}

long double intonation_get_note_factor(const struct intonation*const intonation, const char* name){
  for(size_t i=0; i<intonation->note_count; i++)
    if(!strcmp(name, intonation->note_map[i].name))
      return intonation->note_map[i].factor;
  return 0;
}

void state_set(struct settings* s, int argc, char* argv[argc]){
  if(argc < 1)
    return;
  if(!strcmp(argv[0], "speed")){
    if(argc != 2)
      return;
    s->speed = strtold(argv[1], 0);
    return;
  }
  if(!strcmp(argv[0], "tempo")){
    if(argc != 2)
      return;
    long double o = s->tempo;
    s->tempo = 1;
    long double n = parse_time(s, argv[1]);
    if(n && n > 0){
      s->tempo = n;
    }else{
      s->tempo = o;
    }
    return;
  }
  if(!strcmp(argv[0], "intonation")){
    if(argc != 2)
      return;
    for(size_t i=0; i<INTONATION_COUNT; i++){
      if(strcmp(intonation[i].name, argv[1]))
        continue;
      s->intonation = &intonation[i];
      return;
    }
    fprintf(stderr, "unknown intonation: %s\n", argv[1]);
  }
}

void tracker_remove_generator(struct generator **pit){
  struct generator *it = *pit;
  *pit = it->next;
  free(it);
}

int64_t tracker_generate_sample(struct tracker* tracker){
  int64_t amplitude = 0;
  for(struct generator **pit=&tracker->generator_list; *pit; pit=(*pit)?&(*pit)->next:pit){
    struct generator *it = *pit;
    const uint64_t time = it->time++;
    if(time >= it->duration){
      tracker_remove_generator(pit);
      continue;
    }
    const long double half_time = 0.1;
    int32_t a = tracker->samples_per_second * half_time / (time+tracker->samples_per_second*half_time) * 0x7FFF;
    amplitude += (int64_t)tone_get_sample(&it->tone) * a / 0x7FFF;
  }
  return amplitude;
}

void write_sample(const struct tracker* tracker, int64_t sample){
  switch(tracker->format){
    case F_FLOAT_64: {
      double f = (long double)sample / 0x8000;
      static_assert(sizeof(f) == 8);
      write(1, &f, 4);
    } break;
    case F_FLOAT_32: {
      float f = (long double)sample / 0x8000;
      static_assert(sizeof(f) == 4);
      write(1, &f, 4);
    } break;
    case F_INT_32: {
      if(sample > 0x7FFFFFFF)
        sample = 0x7FFFFFFF;
      if(sample < -0x7FFFFFFF)
        sample = -0x7FFFFFFF;
      write(1, (int32_t[]){sample,sample>>8,sample>>16,sample>>24}, 4);
    } break;
  }
}

void tracker_generate(struct tracker* tracker, int argc, char* argv[argc]){
  uint64_t time = ~0;
  if(argc)
    time = (uint64_t)tracker->samples_per_second * parse_time(&tracker->settings, argv[0]) / tracker->settings.speed;
  while(tracker->generator_list){
    if(time != (uint64_t)~0)
      if(!time--)
        break;
    int64_t amplitude = tracker_generate_sample(tracker);
    if(tracker->stats.min > amplitude)
      tracker->stats.min = amplitude;
    if(tracker->stats.max < amplitude)
      tracker->stats.max = amplitude;
    tracker->stats.abs_sum += amplitude < 0 ? -amplitude : amplitude;
    tracker->stats.square_sum += amplitude * amplitude;
    tracker->stats.samples_total++;
    write_sample(tracker, amplitude);
  }
}

long double parse_time(const struct settings*const s, const char*const restrict input){
  double nominator=1, denominator=1;
  char unit[3] = {0};
  int ret = sscanf(input, "%lf/%lf%2s", &nominator, &denominator, unit);
  if(ret == 1)
    ret = sscanf(input, "%lf%2s", &nominator, unit);
  if(ret == EOF || ret <= 0)
    return 0;
  long double time = (long double)nominator / denominator;
  if(!*unit){
    time *= s->tempo;
  }else if(!strcmp(unit, "s")){
  }else if(!strcmp(unit, "ms")){
    time /= 1000;
  }else if(!strcmp(unit, "m")){
    time *= 60;
  }else if(!strcmp(unit, "h")){
    time *= 60 * 60;
  }else return 0;
  return time;
}

void tracker_add_generator(struct generator** list, const struct generator*restrict const entry){
  struct generator* e = malloc(sizeof(*entry));
  *e = *entry;
  e->next = *list;
  *list = e;
}

void tracker_add_note(struct tracker* tracker, int argc, char* argv[argc]){
  const int oargc = argc;
  char**const oargv = argv;
  if(!argc) goto error;
  const struct settings*const s = &tracker->settings;
  const long double factor = intonation_get_note_factor(s->intonation, argv[0]);
  if(!factor) goto error;
  if(!--argc) goto error;
  argv += 1;
  int octave = atoi(argv[0]);
  const long double frequency = s->c4 * (powl(2, octave) / 16) * factor;
  if(!frequency) goto error;
  if(!--argc) goto error;
  argv += 1;
  struct generator g = {0};
  g.tone.tracker = tracker;
  g.tone.waveform = sg_sin;
  g.tone.duration = tracker->samples_per_second / frequency;
  g.duration = tracker->samples_per_second * parse_time(s, argv[0]) / s->speed;
  g.duration = (g.duration + g.tone.duration - 1) / g.tone.duration * g.tone.duration; // Round up to whole wave
  tracker_add_generator(&tracker->generator_list, &g);
  return;
error:
  fprintf(stderr, "%lu: tracker_add_note failed:", tracker->line);
  for(int i=0; i<oargc; i++)
    fprintf(stderr, " %s", oargv[i]);
  fprintf(stderr, "\n");
}

typedef void cmd_func(struct tracker* tracker, int argc, char* argv[argc]);
struct cmd {
  const char* name;
  cmd_func* call;
};

const struct cmd cmd_list[] = {
  { ">>", tracker_generate },
  { "n", tracker_add_note }
};

void setattri(int fd, const char* name, long long value){
  char buf[64];
  ssize_t s = snprintf(buf, sizeof(buf), "%lld", value);
  if(s >= 0){
    fsetxattr(fd, name, buf, s, 0);
  }
}

int main(int argc, char* argv[]){
  (void)argc;
  (void)argv;
  struct tracker tracker = {
    .settings = {
      .c4 = 261.6,
      .speed = 1,
      .tempo = 1,
      .intonation = &intonation[INTONATION_EQUAL],
    },
    .line = 1,
    .stats.min =  INFINITY,
    .stats.max = -INFINITY,
    .format = F_INT_32,
    .samples_per_second = COMMON_SAMPLE_RATE_48,
  };
  { write(1, mk_wav(1, tracker.samples_per_second, tracker.format).data, sizeof(struct wav_header)); };
  for(char buf[256]; fgets(buf, sizeof(buf), stdin);){
    char* pch = strtok(buf," \t\r\n");
    if(pch && *pch && *pch != '#')
    do {
      int cmdargc = 0;
      char* cmdargv[32];
      for(; pch; pch = strtok(0, " \t\r\n")){
        if(pch[0] == '#'){
          pch = 0;
          break;
        }
        if(cmdargc && (!strcmp(pch, ">>") || pch[0] == ':'))
          break;
        cmdargv[cmdargc++] = pch;
        if((unsigned)cmdargc >= sizeof(cmdargv)/sizeof(*cmdargv))
          break;
      }
      if(!cmdargc) continue;
      if(cmdargv[0][0] == ':'){
        cmdargv[0] += 1;
        state_set(&tracker.settings, cmdargc, cmdargv);
      }else{
        const struct cmd* cmd = 0;
        for(size_t i=0; i<sizeof(cmd_list)/sizeof(*cmd_list); i++){
          if(strcmp(cmd_list[i].name, cmdargv[0]))
            continue;
          cmd = &cmd_list[i];
          break;
        }
        if(cmd){
          cmd->call(&tracker, cmdargc-1, cmdargv+1);
        }else{
          fprintf(stderr, "unknown command: %s\n", cmdargv[0]);
        }
      }
    } while(pch);
    tracker.line += 1;
  }
  tracker_generate(&tracker, 0, 0);
  double average_abs_volume = tracker.stats.abs_sum / tracker.stats.samples_total;
  double average_square_volume = sqrtl(tracker.stats.square_sum / tracker.stats.samples_total);
  setattri(1, "user.stats.min", tracker.stats.min);
  setattri(1, "user.stats.max", tracker.stats.max);
  setattri(1, "user.stats.abs_avg" , average_abs_volume);
  setattri(1, "user.stats.qsum_avg", average_square_volume);
  setattri(1, "user.stats.factor", (long double)0x80000000 / average_square_volume);
  return 0;
}
