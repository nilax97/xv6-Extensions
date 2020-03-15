#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	// toggle();	// toggle the system trace on or off
	int pid = fork();
	if(pid==0)
		printf(1,"I am a child\n");
	ps();
	wait();
	exit();
}
