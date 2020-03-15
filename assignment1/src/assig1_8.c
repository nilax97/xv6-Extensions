#include "types.h"
#include "stat.h"
#include "user.h"

int
msg_prep(int value, char *msg, int type) //Function to process the int and convert it to a string
{
  char *message = (char *)msg;
  int return_val = -1;

  if(type == 0)
  {
    int temp = value;
    int index = 0;
    while(temp > 0)
    {
      *(message + index) = '0' + temp%10;
      temp = temp/10;
      index = index + 1;
    }
    *(message+ index) = '\0';
    return_val = 0;
  }
  else
  {
    value = 0;
    int index = 0;
    while(*(message + index) != '\0')
    {
      index = index + 1;
    }
    index = index - 1;
    while(index >= 0)
    {
      value = value * 10;
      value = value + (*(message + index) - '0');
      index = index - 1;
    }
    return_val = value;
  }
  return return_val;
}

int send_mult(int parent_id, int *child_id, void *msg1, int len)
{
  for(int i=0; i<len; ++i)
  {
    send(parent_id, child_id[i], msg1);
  }
  return 0;
}


int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	int tot_sum = 0;
	float variance = 0.0;

	int size=1000;
	short arr[size];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);

  	//----FILL THE CODE HERE for unicast sum and multicast variance
  int parent_id = getpid();
  int child_id[8];

  for(int i=0; i<process_count; ++i)
  {
    child_id[i] = fork();
    if(child_id[i] == 0)
    {
      int sum = 0;
      int start_loc = (size/process_count) * i;
      int end_loc = (size/process_count) * (i+1);
      if(i == process_count - 1)
      {
        end_loc = size;
      }
      for(int j = start_loc; j<end_loc; ++j)
      {
        sum = sum + arr[j];
      }
      char *msg = (char *)malloc(MSGSIZE);
      msg_prep(sum, msg, 0);
      send(getpid(), parent_id, msg);

      if(type == 0)
      {
        exit();
      }
      recv(msg);
      int avg = 0;
      avg = msg_prep(avg, msg, 1);
      int sigma = 0;
      for(int j = start_loc; j<end_loc; ++j)
      {
        sigma = sigma + (arr[j]-avg)*(arr[j]-avg);
      }
      msg_prep(sigma, msg, 0);
      send(getpid(), parent_id, msg);
      exit();
    }
  }
  for(int i=0; i<process_count; ++i)
  {
    char *msg = (char *)malloc(MSGSIZE);
    recv(msg);
    int sum = 0;
    sum = msg_prep(sum, msg, 1);
    tot_sum = tot_sum + sum;
  }
  if(type == 1)
  {
    int avg = tot_sum/size;
    char *msg1 = (char *)malloc(MSGSIZE);
    msg_prep(avg, msg1, 0);

    send_mult(parent_id, child_id, msg1, process_count);
    for(int i=0; i<process_count; ++i)
    {
      char *msg = (char *)malloc(MSGSIZE);
      recv(msg);
      int sigma = 0;
      sigma = msg_prep(sigma, msg, 1);
      variance = variance + sigma;
    }

    variance = variance/size;
  }
  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		printf(1,"Variance of array for file %s is %d\n", filename,(int)variance);
	}
  for(int i=0; i<process_count; ++i)
  {
    wait();
  }
	exit();
}