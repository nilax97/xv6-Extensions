#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include<time.h>

#define REQUEST 1
#define INQUIRE 2
#define RELINQUISH 3
#define LOCK 4
#define RELEASE 5
#define FAIL 6
#define END 7

#define UNUSED -1

#define TYPE1 0
#define TYPE2 1
#define TYPE3 2

struct message
{
	int data;
	int sender;
};

void push_queue(struct message q[], struct message msg, int size)
{
	int i;
	for(i = size - 1; (i>=0 && q[i].sender > msg.sender); --i)
	{
		q[i+1] = q[i];
	}
	q[i+1] = msg;
	return;
}

struct message pop_queue(struct message q[], int size)
{
	struct message msg = q[0];
	int i;
	for(i = 0; i<size-1; ++i)
	{
		q[i] = q[i+1];
	}
	return msg;
}

int sum_array(int arr[], int n)
{
	int sum = 0;
	for(int i=0; i<n; ++i)
	{
		sum += arr[i];
	}
	return sum;
}

int sqrt_x(int x)
{
	for(int i=0; i*i<=x; ++i)
	{
		if(i*i == x)
			return i;
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int P,P1,P2,P3;

	FILE *f1 = fopen("assig2b.inp","r");
	fscanf(f1,"%d%d%d%d",&P,&P1,&P2,&P3);

	int K = sqrt_x(P);

	int i,j;
	int process_comm[P][2];
	int main_comm[2];
	for(i = 0; i<P; ++i)
		pipe(process_comm[i]);

	pipe(main_comm);

	int process_list[K][K];

	for(i=0; i<K; ++i)
		for(j=0; j<K; ++j)
			process_list[i][j] = i*K + j;

	int quorum[P][2*K - 1];
	int process_type[P];
	for(i=0; i<P; ++i)
	{
		int row = i/K;
		int col = i%K;
		int count = 0;
		quorum[i][count] = process_list[row][col];
		count++;
		for(j=0; j<K; ++j)
		{
			if(j != row)
			{
				quorum[i][count] = process_list[j][col];
				count++;
			}
			if( j!= col)
			{
				quorum[i][count] = process_list[row][j];
				count++;
			}
		}

		if(i<P1)
			process_type[i] = TYPE1;
		else if(i - P1 < P2)
			process_type[i] = TYPE2;
		else
			process_type[i] = TYPE3;
	}

	int pid_list[P];
	struct message process_queue[P][P];
	int queue_size[P];
	int status[P];

	for(i = 0; i<P; ++i)
	{
		queue_size[i] = 0;
		status[i] = UNUSED;
		pid_list[i] = fork();
		if(pid_list[i] == 0)
		{
			int inquiry = 0;
			int fail = 0;
			int accept[2*K - 1];
			int crit_section = 0;
			int release = 0;
			int ignore = 0;
			int target = UNUSED;
			struct message ret_msg;
			struct message msg;
			if(process_type[i] != TYPE1)
			{
				msg.data = REQUEST;
				msg.sender = i;
				for(j=0; j<2*K -1; ++j)
				{
					int val = quorum[i][j];
					write(process_comm[val][1], &msg, sizeof(msg));
				}
			}

			while(read(process_comm[i][0], &msg, sizeof(msg)))
			{
				if(msg.data == REQUEST)
				{
					if(status[i] == UNUSED)
					{
						target = msg.sender;
						status[i] = target;
						ret_msg.data = LOCK;
						ret_msg.sender = i;
						write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
					}
					else
					{
						push_queue(process_queue[i], msg, queue_size[i]);
						queue_size[i]++;
						target = msg.sender;
						int busy = 0;
						if(target > status[i] || (queue_size[i]>0 && process_queue[i][0].sender < target))
							busy = 1;
						if(busy)
						{
							ret_msg.data = FAIL;
							ret_msg.sender = i;
							write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
						}
						else
						{
							if(inquiry == 0)
							{
								ret_msg.data = INQUIRE;
								ret_msg.sender = i;
								int to = status[i];
								write(process_comm[to][1], &ret_msg, sizeof(ret_msg));
							}
						}
					}
				}
				else if(msg.data == INQUIRE)
				{
					if(fail == 1)
					{
						ret_msg.data = RELINQUISH;
						ret_msg.sender = i;
						for(j=0; j<2*K -1; ++j)
						{
							target = quorum[i][j];
							write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
						}
						inquiry = 0;
					}
					else
					{
						if(process_type[i] == TYPE1)
						{
							if(release == 0)
							{
								if(crit_section == 1 && sum_array(accept, 2*K-1) == 2*K -1)
								{
									while(release == 0)
									{
										;
									}
									ret_msg.data = RELEASE;
									ret_msg.sender = i;
									for(j=0; j<2*K -1; ++j)
									{
										target = quorum[i][j];
										write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
									}
									inquiry = 0;
								}
								else
								{
									ignore = 1;
								}
							}
						}
					}
					inquiry = 0;
				}

				else if(msg.data == RELINQUISH)
				{
					status[i] = UNUSED;
					if(queue_size[i]>0)
					{
						ret_msg = pop_queue(process_queue[i], queue_size[i]);
						queue_size[i]--;
						if(ret_msg.data == REQUEST)
						{
							target = ret_msg.sender;
							status[i] = target;
							ret_msg.data = LOCK;
							ret_msg.sender = i;
							write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
						}
					}

					ret_msg.data = REQUEST;
					ret_msg.sender = msg.sender;
					if(queue_size[i]>0)
					{
						push_queue(process_queue[i], ret_msg, queue_size[i]);
						queue_size[i]++;
					}
				}
				else if(msg.data == LOCK)
				{
					int j;
					for(j=0; j< 2*K-1; ++j)
					{
						if(quorum[i][j] == msg.sender)
							break;
					}
					accept[j] = 1;
					crit_section = 1;
					for(j=0; j< 2*K -1; ++j)
					{
						if(accept[j] == 0)
						{
							crit_section = 0;
							break;
						}
					}
					if(crit_section == 1)
					{
						printf("%d acquired the lock at time %lu\n",getpid(), time(NULL));
						if(process_type[i] == TYPE2)
						{
							sleep(2);
						}
						printf("%d released the lock at time %lu\n",getpid(), time(NULL));
						int completed = 1;
						write(main_comm[1], &completed, sizeof(int));

						ret_msg.data = RELEASE;
						ret_msg.sender = i;
						for(j = 0; j< 2*K -1; ++j)
						{
							target = quorum[i][j];
							write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
							accept[j] = 0;
						}
						fail = 0;
						crit_section = 0;
						release = 1;
					}
				}
				else if(msg.data == RELEASE)
				{
					status[i] = UNUSED;
					if(queue_size[i] > 0)
					{
						ret_msg = pop_queue(process_queue[i], queue_size[i]);
						target = ret_msg.sender;
						status[i] = target;
						ret_msg.data = LOCK;
						ret_msg.sender = i;
						write(process_comm[target][1], &ret_msg, sizeof(ret_msg));
					}
				}
				else if(msg.data == FAIL)
				{
					fail = 1;
					for(j=0; j< 2*K -1; ++j)
					{
						if(quorum[i][j] == msg.sender)
							break;	
					}
					accept[j] = 0;
					if(ignore == 1 && inquiry == 1)
					{
						ret_msg.data = RELINQUISH;
						ret_msg.sender = i;
						for(j=0; j< 2*K -1; ++j)
						{
							int temp = quorum[i][j];
							write(process_comm[temp][1], &ret_msg, sizeof(ret_msg));
						}
						ignore = 0;
						inquiry = 0;
					}
				}
				else if(msg.data == END)
				{
					break;
				}
			}
			exit(0);
		}
	}
	int comp_count = 0;
	int foo;
	while(read(main_comm[0], &foo, sizeof(int)))
	{
		comp_count++;
		if(comp_count == (P2 + P3))
			break;
	}
	struct message msg;
	msg.data = END;
	msg.sender = UNUSED;
	for(i = 0; i<P; ++i)
	{
		write(process_comm[i][1], &msg, sizeof(msg));
	}
	exit(0);
}