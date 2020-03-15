#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int *temp;

void barrier(int pipes[][2], int pipe_id, int type)
{
  int i;
  /* ---------------- BARRIER ---------------- */

  if (type == -1)
  {
    for (i = 1; i < 6; i++)
    {
      read(pipes[0][0], temp, sizeof(temp));
    }
    for (i = 1; i < 6; i++)
    {
      write(pipes[i][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[0][1], temp, sizeof(temp));
    read(pipes[pipe_id][0], temp, sizeof(temp));
  }
}

/* Seperate unit test for copy on write testing 
 * Also included in bigger test case below 
 * no need to uncomment */

// int main(int argc, char *argv[])
// {
//   // int *temp = malloc(sizeof(int));
//   int fd = open("my_file", O_CREATE | O_RDWR);
//   char text[15] = "Modified by:0\n";
//   write(fd, text, 14);
//   close(fd);

//   fd = open("my_file", O_RDONLY);
//   cat(fd);
//   close(fd);
//   if (fork() == 0)
//   {

//     int container1 = create_container();
//     join_container(container1); // called only by child created by preceeding fork call.
//     // printf(1, "Joined Container\n", x);

//     fd = open("my_file", O_CREATE | O_RDWR);
//     // printf(1, "Opened\n", x);

//     text[12] = '1';
//     while (read(fd, temp, sizeof(temp)) > 0)
//     {
//     }
//     // printf(1, "Seek done\n", x);

//     write(fd, text, 14);
//     // printf(1, "Written\n", x);

//     close(fd);

//     fd = open("my_file", O_RDONLY);
//     cat(fd);
//     close(fd);

//     ls(".");

//     exit();
//   }
//   else
//   {
//     wait();
//     ls(".");
//     fd = open("my_file", O_RDONLY);
//     cat(fd);
//     close(fd);
//     exit();
//   }
// }

int main(int argc, char *argv[])
{
  temp = (int *)malloc(sizeof(int));
  int i = -1, pid = -1, type = -1, pipe_id = 0, pipes[6][2], container1, container2, container3, local_pid = -1, mycontainer = -1;

  printf(1, "Creating initial my_file2 not in any container\n");
  int fd = open("my_file2", O_CREATE | O_RDWR);
  char text[15] = "Modified by:0\n";
  write(fd, text, 14);
  close(fd);
  fd = open("my_file2", O_RDONLY);
  printf(1, "Reading my_file2\n");
  cat(fd);

  /* Setting up communication */

  for (i = 0; i < 6; i++)
    if (pipe(pipes[i]) < 0)
      exit();

  /* Creating the containers . */
  container1 = create_container();
  printf(1, "Created Container ID = %d\n", container1);
  container2 = create_container();
  printf(1, "Created Container ID = %d\n", container2);
  container3 = create_container();
  printf(1, "Created Container ID = %d\n", container3);

  /* Three container managers ( user programs ) should be running now */

  /* Multiple process creation to test the scheduler */
  for (i = 0; i < 3; i++)
  {
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      type = 1;
      local_pid = i;
      mycontainer = container1;
      join_container(container1); // called only by child created by preceeding fork call.
      break;
    }
  }

  /* Testing of resource isolation */
  if (pid != 0)
  {
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      type = 2;
      local_pid = 0;
      mycontainer = container2;
      join_container(container2); // called only by child created by preceeding fork call.
    }
  }

  if (pid != 0)
  {
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      type = 3;
      local_pid = 0;
      mycontainer = container3;
      join_container(container3); // called only by child created by preceeding fork call.
    }
    else
    {
      pipe_id = 0;
    }
  }

  barrier(pipes, pipe_id, type);

  /* ---------------- PROCESS ISOLATION ---------------- */
  // called by atmost one process in every container .
  if (pid == 0 && (type != 1 || (type == 1 && local_pid == 1)))
    ps();

  barrier(pipes, pipe_id, type);

  /* ---------------- SCHEDULER TEST ---------------- */

  scheduler_log(1); // This will enable logs from the container scheduler

  sleep(1);
  /* Messaging so that every process is scheduled at least once */

  if (type == -1)
  {
    for (i = 1; i < 6; i++)
    {
      read(pipes[0][0], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[0][1], temp, sizeof(temp));
  }

  /* Print statements of form ( without the quotes ): */
  // " Container + <container_id > : Scheduling process + <pid >"

  scheduler_log(0); // Disable the scheduler log after scheduling all the child process atleast once.

  /* ---------------- MEMORY ISOLATION TEST ---------------- */

  memory_log(1); // This will enable the logs from the memory allocator . It will print the mapping from GVA to HVA everytime a malloc request comes in.
  // Executed by all the child processes across conatiners .
  void *m = malloc(4);

  printf(1, "", m);
  // print the GVA and HVA , i.e. the mapping created in the container ’s
  // page table . "GVA -> HVA"

  memory_log(0); // Disable the memory logs after printing mapping from all the containers atleast once.

  /* ---------------- FILE SYSTEM TEST ---------------- */

  /* Executed by all the child processes in all the containers */
  if (pid == 0)
  {
    read(pipes[pipe_id][0], temp, sizeof(temp));
    printf(1, "Running ls for pid:%d container:%d\n", getpid(), mycontainer);
    ls("."); // This will print the host file system
    if (pipe_id < 5)
    {
      write(pipes[pipe_id + 1][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[1][1], temp, sizeof(temp));
  }

  barrier(pipes, pipe_id, type);

  if (pid == 0)
  {
    char file_name[7] = "file_0";
    file_name[5] += getpid();
    open(file_name, O_CREATE); // pid can be same across containers , however ,in this case they will be different as they are created using fork and join a container later.
  }

  barrier(pipes, pipe_id, type);

  // All the process should finish the create system call
  if (pid == 0)
  {
    read(pipes[pipe_id][0], temp, sizeof(temp));
    printf(1, "Running ls after creating files for pid:%d container:%d\n", getpid(), mycontainer);
    ls("."); // the container should see files created by processes running inside in it along with the original files from the host system.
    if (pipe_id < 5)
    {
      write(pipes[pipe_id + 1][1], temp, sizeof(temp));
    }
    else
    {
      write(pipes[0][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[1][1], temp, sizeof(temp));
  }

  if (pid != 0)
  {
    read(pipes[pipe_id][0], temp, sizeof(temp));
  }
  /* Executed by only one child process in every container */

  if (pid == 0)
  {
    read(pipes[pipe_id][0], temp, sizeof(temp));
    printf(1, "Creating/Writing my_file by pid:%d container:%d\n", getpid(), mycontainer);
    int fd = open("my_file", O_CREATE | O_RDWR);
    while (read(fd, temp, sizeof(temp)) > 0)
    {
    }

    char text[15] = "Modified by:0\n";
    text[12] += getpid();
    write(fd, text, 14);
    close(fd);
    printf(1, "Running cat\n");
    fd = open("my_file", O_RDONLY);
    cat(fd); // The contents of the file should be different for process running across containers.
    if (pipe_id < 5)
    {
      write(pipes[pipe_id + 1][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[1][1], temp, sizeof(temp));
  }

  barrier(pipes, pipe_id, type);

  if (pid == 0)
  {
    read(pipes[pipe_id][0], temp, sizeof(temp));
    printf(1, "Creating/Writing my_file by pid:%d container:%d\n", getpid(), mycontainer);

    int fd = open("my_file2", O_CREATE | O_RDWR);
    while (read(fd, temp, sizeof(temp)) > 0)
    {
    }

    char text[15] = "Modified by:0\n";
    text[12] += getpid();
    write(fd, text, 14);
    close(fd);
    printf(1, "Running cat\n");
    fd = open("my_file2", O_RDONLY);
    cat(fd); // The contents of the file should be different for process running across containers.
    if (pipe_id < 5)
    {
      write(pipes[pipe_id + 1][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[1][1], temp, sizeof(temp));
  }
  // ==========================================================

  barrier(pipes, pipe_id, type);

  if (pid == 0)
  {
    /* Executed by all the child processes */
    leave_container();
    exit();
  }
  else
  {
    for (int i = 0; i < 5; i++)
    {
      wait();
    }

    // // Executed by the main process
    destroy_container(container1);
    destroy_container(container2);
    destroy_container(container3);

    exit();
  }
}