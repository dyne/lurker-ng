#define longfn	find_email_end
#define scanner	find_email_starts

#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

extern char* scanner(const char* start, const char* end);
extern const char* longfn(const char* start, const char* end);

void dump_yes(const char* start, const char* end) {
  fwrite(start, 1, end-start, stdout);
  fwrite(start, 1, end-start, stderr);
  fputc('\n', stderr);
}

void dump_no(const char* start, const char* end) {
  fwrite(start, 1, end-start, stdout);
}

void process(const char* start, const char* end) {
  char* map_start = scanner(start, end);
  char* map_end = map_start + (end - start);
  
  const char* i = start; 
  char* map_i = map_start;
  
  while (map_i != map_end) {
    if (*map_i) {
      dump_no(i, start + (map_i-map_start));
      i = start + (map_i - map_start);
      
      const char* mid = longfn(i, end);
      dump_yes(i, mid);
      i = mid;
      map_i = map_start + (mid - start);
    } else {
      ++map_i;
    }
  }
  
  dump_no(i, end);
  free(map_start);
}

int main() {
  string buf;
  char sect[8192];  
  
  int got;
  while ((got = fread(sect, 1, sizeof(sect), stdin)) != 0)
    buf.append(sect, got);
  
  process(buf.c_str(), buf.c_str() + buf.length());
  return 0;
}
