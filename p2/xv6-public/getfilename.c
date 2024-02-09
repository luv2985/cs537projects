#include "types.h"
#include "user.h"

int main(int argc, char* argv[]) 
{
  if (argc != 2)
  {
	printf(1, "Usage: getfilename <name_of_file>\n");
	exit();
  }
  
  int f = open(argv[1], 0);

  char name[256];
  getfilename(f, name, 256);
  
  printf(1, "XV6_TEST_OUTPUT Open filename: %s\n", name);
  exit();
}

