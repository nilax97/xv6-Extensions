#include "types.h"
#include "user.h"
#include "date.h"

int
main(int argc, char *argv[])
{
  toggle();
  // fork(); 
  int fd = open("arr", 0);
  close(fd);
  print_count();
  exit();
}
