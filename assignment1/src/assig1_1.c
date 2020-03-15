#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	toggle(); // This toggles the system trace on or off
	printf(1,"This is for test \n" );
	int cid = fork();
	if(cid!=0){
		print_count();
		toggle();		
	}
	exit();
}
