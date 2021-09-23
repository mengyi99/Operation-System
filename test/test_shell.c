/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "screen.h"
#include "stdio.h"
#include "syscall.h"
#include "test.h" 
#include "common.h"
#include "string.h"
#include "fs.h"
#define P6_TEST

#ifdef P3_TEST

struct task_info task1 = {"task1", (uint64_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {"task2", (uint64_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {"task3", (uint64_t)&wait_exit_task, USER_PROCESS};

struct task_info task4 = {"task4", (uint64_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {"task5", (uint64_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {"task6", (uint64_t)&semaphore_add_task3, USER_PROCESS};

struct task_info task7 = {"task7", (uint64_t)&producer_task, USER_PROCESS};
struct task_info task8 = {"task8", (uint64_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {"task9", (uint64_t)&consumer_task2, USER_PROCESS};

struct task_info task10 = {"task10", (uint64_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {"task11", (uint64_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {"task12", (uint64_t)&barrier_task3, USER_PROCESS};

struct task_info task13 = {"SunQuan", (uint64_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {"LiuBei", (uint64_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {"CaoCao", (uint64_t)&CaoCao, USER_PROCESS};
#endif
#ifdef P4_TEST
struct task_info task16 = {"mem_test1", (uint64_t)&rw_task1, USER_PROCESS};
struct task_info task17 = {"plan", (uint64_t)&drawing_task1, USER_PROCESS};
#endif

#ifdef P5_TEST
struct task_info task18 = {"mac_send", (uint64_t)&mac_send_task, USER_PROCESS};
struct task_info task19 = {"mac_recv", (uint64_t)&mac_recv_task, USER_PROCESS};
#endif

#ifdef P6_TEST

struct task_info task19 = {"fs_test", (uint64_t)&test_fs, USER_PROCESS};
struct task_info task20 = {"bf_test", (uint64_t)&test_bf, USER_PROCESS};
#endif
//struct task_info task16 = {"multcore", (uint64_t)&test_multicore, USER_PROCESS};
//static struct task_info *test_tasks[NUM_MAX_TASK] = {
//    &task19,
//};

//#endif
static char read_uart_ch(void)
{
   char ch = 0;
   unsigned char *read_port = (unsigned char *)((0xffffffff1fe00000 | 0xa0000000)+0x00);
   unsigned char *stat_port = (unsigned char *)((0xffffffff1fe00000 | 0xa0000000)+0x05);
   
   if((*stat_port & 0x01))
   {
      ch = *read_port;
   }
   return ch;
}

char *command_boundary = "> -------------- COMMAND --------------";
char *user_name = "> ROOT@UCAS_OS$ ";
char argv[3][30]={0};
void test_shell()
{ 
  int i=0,j=0,k;
	kprintf("%s\n", command_boundary); // show boundary
	kprintf("%s", user_name); 
  char argv[4][30]={0};
  
	char in_buf[40]= { 0 };
 	int in_id = 0;

	while (1) {
		char ch = read_uart_ch();
		if (ch == 0) // do not recv ch
      {
         continue;
			}
		else if (ch != '\r') {
			if (ch == 8) { // backspace
				if (in_id > 0) {
					kprintf("%c", ch);
					in_buf[in_id--] = '\0';
				}
			} else if (in_id < 40) { // not backspace
				kprintf("%c", ch);
				in_buf[in_id++] = ch;
			}
		} else { // parsing buffer when '\r'
			kprintf("%c", ch);
			in_buf[in_id] = '\0';

 		if (memcmp(in_buf,"ps",2)==0 && in_buf[2] == '\0')
      { // ps
        do_process_show();
        kprintf("%s",user_name);
   			} 
    else if (memcmp(in_buf,"clear",5)==0 && in_buf[5] == '\0')
      { // clear
				screen_clear_shell(18,SCREEN_HEIGHT);
   	    screen_move_cursor(0, 18);
        kprintf("%s\n", command_boundary); // show boundary
	      kprintf("%s", user_name); 
			} 
    else if (memcmp(in_buf,"clear all",9)==0 &&in_buf[9] == '\0')
      { // clear all
				screen_clear(1,SCREEN_HEIGHT);
   	    screen_move_cursor(0, 18);
        kprintf("%s\n", command_boundary); // show boundary
	      kprintf("%s", user_name); 
			} 
   else if (memcmp(in_buf,"kill",4)==0)
      { //kill
         int be_kill; 
         if(in_buf[6]>='0' && in_buf[6]<='9')
         be_kill=10*(in_buf[5]-'0')+in_buf[6]-'0';
         else
           be_kill=in_buf[5]-'0';
				int has_killed =(be_kill > 0) ?do_kill((pid_t)be_kill) :(-1);
				if (has_killed == 0)
					kprintf("PROCESS (pid=%d) has been KILLED.\n",be_kill);
				else 
	  	    kprintf("PROCESS %d NOT EXISTED.\n",be_kill);

         kprintf("%s", user_name); 
			} 
      else if (memcmp(in_buf,"wait",4)==0)
      { //wait
				int wait_who;
        if(in_buf[6]>='0' && in_buf[6]<='9')
           wait_who=10*(in_buf[5]-'0')+in_buf[6]-'0';
        else
           wait_who=in_buf[5]-'0';
				int has_waited =(wait_who > 0) ?do_waitpid((pid_t)wait_who) :(-1);
				if (has_waited == -1) 
					kprintf("PROCESS %d NOT EXISTED.\n",wait_who);
        else
          kprintf("PROCESS %d WAIT SUCCESS.\n",wait_who);
                        
        kprintf("%s", user_name);
			} 
      else if (memcmp(in_buf,"exec 0",6)==0)
      { //exec
	//			int be_exec;
   //     if(in_buf[6]>='0' && in_buf[6]<='9')
    //       be_exec=10*(in_buf[5]-'0')+in_buf[6]-'0';
    //    else
     //      be_exec=in_buf[5]-'0';
		//		int has_exec;
 			//	if (be_exec >= 0 && be_exec < 16
		  //     has_exec =do_spawn(&task19);
			//	else
			//		 has_exec = -1; // invalid task
                       
		//		if (has_exec == -1)
		//			kprintf("Task %d EXEC FAILED/NOT EXISTED.\n",be_exec);
    //    else 
		//		  kprintf("Task %d EXEC SUCCESS.\n",be_exec);
        do_spawn(&task19);
        kprintf("%s",user_name);
		   	} 
      else if (memcmp(in_buf,"exec 1",6)==0)
      { 
        do_spawn(&task20);
        kprintf("%s",user_name);
		   	} 
       else 
       if(memcmp(in_buf,"mkfs",4)==0) 
       {
       init_fs();
       kprintf("%s",user_name);
       }
       else if(memcmp(in_buf,"statfs",6)==0)
       {
       print_fs_info();
       kprintf("%s",user_name);
       }
       
       
       else if(memcmp(in_buf,"cd",2)==0)
       {
       for(k=3;k<in_id;k++)
       {
       argv[1][j++] = in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       enter_dir(argv[1]);
       
       kprintf("%s",user_name);
       }
       else if(memcmp(in_buf,"mkdir",5)==0)
       {
       for(k=6;k<in_id;k++)
       {
       argv[1][j++] = in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       mkdir(argv[1]);
       kprintf("%s",user_name);
       }
       else if(memcmp(in_buf,"rmdir",5)==0)
       {
       for(k=6;k<in_id;k++)
       {
       argv[1][j++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       rmdir(argv[1]);
       kprintf("%s",user_name);
       }
       
       else if(memcmp(in_buf,"rm",2)==0  && memcmp(in_buf,"rmdir",5)!=0)
       {
       for(k=3;k<in_id;k++)
       {
       argv[1][j++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       rm(argv[1]);
       kprintf("%s",user_name);
  
       }
       
       else if(memcmp(in_buf,"ls",2)==0)
       {
       readdir();
       kprintf("%s",user_name);
  
       }
       else if(memcmp(in_buf,"touch",5)==0)
       {
       for(k=6;k<in_id;k++)
       {
       argv[1][j++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       
       mknod(argv[1]);
       kprintf("%s",user_name);
  
       }
       else if(memcmp(in_buf,"cat",3)==0)
       {
       for(k=4;k<in_id;k++)
       {
       argv[1][j++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       cat(argv[1]);
       kprintf("%s",user_name);
    
       }
       else if(memcmp(in_buf,"ln",2)==0  && memcmp(in_buf,"ln -s",5)!=0  )
       {
       for(k=3;k<in_id;k++)
       {
       if(in_buf[k]==' ')
         break;
       else
       argv[1][j++]=in_buf[k];
       }
       k++;
       for(;k<in_id;k++)
       {
       argv[2][i++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       for(;i<30;)
       {
       argv[2][i++]=0;
       }
       if (!do_link(argv[1], argv[2], 0)) {
          kprintf("Successfully create hard link: %s -> %s\n", argv[2], argv[1]);
       }

       kprintf("%s",user_name);
       }
       
       else if(memcmp(in_buf,"ln -s",5)==0)
       {
       for(k=6;k<in_id;k++)
       {
       if(in_buf[k]==' ')
         break;
       else
       argv[1][j++]=in_buf[k];
       }
       k++;
       for(;k<in_id;k++)
       {
       argv[2][i++]=in_buf[k];
       }
       for(;j<30;)
       {
       argv[1][j++]=0;
       }
       for(;i<30;)
       {
       argv[2][i++]=0;
       }
       if (!do_link(argv[1], argv[2], 1)) {
          kprintf("Successfully create soft link: %s -> %s\n", argv[2], argv[1]);
       }
       
       
       kprintf("%s",user_name);
  
       }
       else if(memcmp(in_buf,"run",3)==0)
       {
          while(1)
          {
          do_scheduler();          
          char c = read_uart_ch();
          if (c != '\r')
             continue;
          else
             break;
          }
          kprintf("%s",user_name);

          }
       else { // error
	      kprintf("[ERROR] Error Input!\n");
        kprintf("%s",user_name);
		   	}
	  	  in_id = 0;
        j=0;
        i=0;
		}

    do_scheduler();
	}
}