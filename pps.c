#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <linux/kdev_t.h>
#include <time.h>

void getstat(char *fpath,char *stat);
int getCmdline(char *fpath,int pid,char *buf);
void getTty(char *fpath,char *tty);
void getCPUandTIME(char *fpath,char *resultString,char *TIME);
void getMEM(char *fpath,char *memperc,unsigned long *vsz, long *rss);
void getSTART(char *fpath,char *start);

int main (int argc, char *argv[])
{
  int a_flag=0,u_flag=0,x_flag=0;

  if(argc==2){
	// option
  if(strchr(argv[1],'a')!=NULL)
	  a_flag =1;
  if(strchr(argv[1],'u')!=NULL)
	  u_flag =1;
  if(strchr(argv[1],'x')!=NULL)
	  x_flag =1;
  }



  //프로세스 정보를 가져오기 위해서 /proc/[PID]/stat파일을 읽어들이고 " "기준으로 파싱.
  DIR *dir;   // /proc/pid/를 가리킬 변수
  struct dirent *entry;
  struct stat statbuf;
  char tempPath[256];

  int pid;
  struct passwd *user,*currentuser;
  uid_t current,UID;
  char username[32];//u
  char MEM[64];//u
  char cmdline[300000];
  char tty[64];
  unsigned long vsz; //u
  long rss; //u
  char START[256];//u
  char STAT[256];
  char CPU[256]; //u
  char TIME[256];



  //pps
  if((!a_flag)&&(!u_flag)&&(!x_flag)) 
    printf(" %5s %7s %7s %s\n","PID","TTY","TIME","CMD");
  //pps aux
  else if(a_flag&&u_flag&&x_flag)
	   printf("%-7s %7s %7s %7s %7s %7s %4s %5s %5s %8s   %s\n","USER","PID","%%CPU","%%MEM","VSZ","RSS","TTY","STAT","START","TIME","COMMAND");
  //pps a
  else if(a_flag&&(!u_flag)&&(!x_flag))
	  printf(" %5s %7s %5s %7s %s\n","PID","TTY","STAT","TIME","COMMAND");
  //pps au 
	else if((!x_flag)&&u_flag&&a_flag)
	   printf("%-7s %7s %7s %7s %7s %7s %4s %5s %5s %8s    %s\n","USER","PID","%CPU","%MEM","VSZ","RSS","TTY","STAT","START","TIME","COMMAND");
  //pps x
  else if(x_flag&&(!a_flag)&&(!u_flag))
	  printf(" %5s %7s %5s %7s %s\n","PID","TTY","STAT","TIME","COMMAND");
  //pps u
	else if(u_flag && (!x_flag) && (!a_flag))
	   printf("%-7s %7s %7s %7s %7s %7s %4s %5s %5s %8s    %s\n","USER","PID","%CPU","%MEM","VSZ","RSS","TTY","STAT","START","TIME","COMMAND");
  //pps ax
  else if(a_flag && x_flag && (!u_flag))
	  printf(" %5s %7s %5s %7s %s\n","PID","TTY","STAT","TIME","COMMAND");
  //pps ux
  else if(u_flag && x_flag && (!a_flag))
	   printf("%-7s %7s %7s %7s %7s %7s %4s %5s %5s %8s    %s\n","USER","PID","%CPU","%MEM","VSZ","RSS","TTY","STAT","START","TIME","COMMAND");


  dir = opendir("/proc");
  while((entry= readdir(dir)) != NULL)
  {
	stat(entry ->d_name, &statbuf);
	if(!S_ISDIR(statbuf.st_mode))
	  continue;
	pid = atoi(entry->d_name);
	if(pid <= 0) continue;

	currentuser = getpwuid(getuid());
	current = geteuid();
	char currentUser[32]; 
	strcpy(currentUser,currentuser->pw_name);
	//username
	user = getpwuid(statbuf.st_uid);
	strcpy(username,user->pw_name);
	UID = user -> pw_uid;
	sprintf(tempPath,"/proc/%d/cmdline",pid);
	//cmdline
	getCmdline(tempPath,pid,cmdline); //정의한 함수
	sprintf(tempPath,"/proc/%d/stat",pid);
	//STAT = stat + (</N)+(L)+(S)+(l)+(+)
	getstat(tempPath,STAT);
	//tty
	getTty(tempPath,tty);
	//TIME(cpu time) & %CPU
	getCPUandTIME(tempPath,CPU,TIME);
	//%MEM & VSZ & RSS
	getMEM(tempPath,MEM,&vsz,&rss);
	//START
	getSTART(tempPath,START);

	if(!strcmp(cmdline,"")) continue;
	
	//pps aux
	if(a_flag&&u_flag&&x_flag)
     {
	   printf("%-7s %7d %7s %7s %7ld %7ld %4s %5s %5s %8s    %s\n",username,pid,CPU,MEM,vsz,rss,tty,STAT,START,TIME,cmdline);
	 }

	//pps 
	else if((!a_flag)&&(!u_flag)&&(!x_flag)) 
	{
	  if((strcmp(currentUser,username)==0)&&(STAT[0]=='R'))
	      printf(" %5d %7s %7s %s\n",pid,tty,TIME,cmdline);
	}
	//pps x
	else if(x_flag&&(!a_flag)&&(!u_flag))
	{
	  if(strcmp(currentUser,username)==0)
	      printf(" %5d %7s %5s %7s %s\n",pid,tty,STAT,TIME,cmdline);
	}
	//pps au
	else if((!x_flag)&&u_flag&&a_flag)
	{
	  if(tty[0] != '?')
	   printf("%-7s %7d %7s %7s %7ld %7ld %4s %5s %5s %8s    %s\n",username,pid,CPU,MEM,vsz,rss,tty,STAT,START,TIME,cmdline);
	}
	//pps u
	else if(u_flag && (!x_flag) && (!a_flag))
	{
	  if((strcmp(currentUser,username)==0)&&(tty[0] != '?'))
	   printf("%-7s %7d %7s %7s %7ld %7ld %4s %5s %5s %8s    %s\n",username,pid,CPU,MEM,vsz,rss,tty,STAT,START,TIME,cmdline);
	}
	//pps ax
	else if(a_flag && x_flag && (!u_flag))
	{
	  printf(" %5d %7s %5s %7s %s\n",pid,tty,STAT,TIME,cmdline);
	}
	//pps ux
    else if(u_flag && x_flag && (!a_flag))
	{
	  if(strcmp(currentUser,username)==0)
	   printf("%-7s %7d %7s %7s %7ld %7ld %4s %5s %5s %8s    %s\n",username,pid,CPU,MEM,vsz,rss,tty,STAT,START,TIME,cmdline);

	}
	//pps a
	else if(a_flag&&(!u_flag)&&(!x_flag))
	{
	  if(tty[0] != '?')
	     printf(" %5d %7s %5s %7s %s\n",pid,tty,STAT,TIME,cmdline);
	}
  }


  closedir(dir);
}

int getCmdline(char *fpath,int pid,char *buf)
{
  FILE *fp;
  int i;

  fp = fopen(fpath,"r");
  memset(buf,0,sizeof(buf));
  char temp[1024];
  memset(temp,0,sizeof(temp));
  fgets(temp,sizeof(temp),fp);
  //printf("temp:%s\n",temp);
  strcpy(buf,temp);
  if(strcmp(temp,"")==0)
  {
	FILE *fp2;
	char *ptr;
	char tempPath[256];
	sprintf(tempPath,"/proc/%d/stat",pid);
	fp2 = fopen(tempPath,"r");
	char Temp[1024];
	fgets(Temp,sizeof(Temp),fp2);
	ptr = strtok(Temp," ");
	ptr = strtok(NULL," ");
	strcpy(buf,ptr);
  }
  fclose(fp);
}
void getstat(char *fpath,char *stat)
{
  FILE *fp;
  int num=0;
  int i;
  char buf[1024];
  fp = fopen(fpath,"r");
  memset(buf,0,sizeof(buf));
  fgets(buf,sizeof(buf),fp);

  //공백으로 구분
  char *ptr = strtok(buf," ");
  int pid = atoi(ptr);
  pid_t pgid = getpgid(pid);
  //printf("pid=%d ",pid);
  int ppid,session,nice,num_threads;
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  //state
  char S = ptr[0];
  stat[num++] = S;
  //printf("state=%c ",S);
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  session = atoi(ptr);
  for(i=6;i<19;i++)
	ptr = strtok(NULL," ");
  nice = atoi(ptr);
  ptr = strtok(NULL," ");
  num_threads = atoi(ptr);
  //printf("ppid(%d)ses(%d)nice(%d)num_thread(%d)\n",ppid,session,nice,num_threads);



  //nice : < or N
  if(nice <0)
  {
	char N = '<';
	stat[num++] = N;
  }
  else if(nice >0)
  {
	char N = 'N';
	stat[num++] = N;
  }
  //L
  FILE *fp2;
  char tempPath[256] ;
  sprintf(tempPath,"/proc/%d/status",pid);
  fp2 = fopen(tempPath,"r");
  char buff[1024];
  char vml[256];
  memset(buff,0,sizeof(buff));
  int l_flag=0;
  while(!feof(fp2))
  {
    fgets(buff,sizeof(buff),fp2);
	char *finder;
	if((finder = strstr(buff,"VmLck:"))!= NULL)
	{
	  strcpy(vml,finder);
	  l_flag=1;
	  break;
	}
	else
	  continue;
  }
  if(l_flag ==1)
  { 
	char *ptr2;
	ptr2 = strtok(vml," ");
	if(ptr ==NULL)
	  printf("ptr2 NULL!!\n");
	int vmlck = atoi(ptr2);
	//printf("vmlck=%d\n",vmlck);
	if(vmlck >0)
	  stat[num++] = 'L';
  }

  //S
  if(pid == session)
  {
	stat[num++] = 's';
  }
  //l
  if(num_threads>=2)
  {
	stat[num++] = 'l';
  }
  //+
  if(pid == pgid)
  {
	stat[num++] = '+';
  }
  
  stat[num] = '\0';
  //printf("stat : %s\n",stat);
  fclose(fp);
  fclose(fp2);
}
void getTty(char *fpath,char *tty)
{
  FILE *fp;
  int i;
  char buf[64];

  fp = fopen(fpath,"r");
  memset(buf,0,sizeof(buf));
  fgets(buf,sizeof(buf),fp);
  //공백으로구분
  char *ptr = strtok(buf," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  ptr = strtok(NULL," ");
  int tty_nr = atoi(ptr);
  int major = MAJOR(tty_nr);
  int minor = MINOR(tty_nr);
  sprintf(tty,"tty%d",minor);
  if(minor == 0)
	sprintf(tty,"?");
  if(major ==136)
	sprintf(tty,"pts/%d",minor);
  fclose(fp);
}
void getCPUandTIME(char *fpath,char *resultString,char *TIME)
{
  //uptime():OS의 uptime을 구해온다.
  int seconds =0;
  FILE *fp;
  char buf[36];
  double stime_os;
  double idletime;

  if ((fp = fopen("/proc/uptime", "r")) == NULL)
  {
      perror("fopen error : ");
      exit(0);
  }
  fgets(buf, 36, fp);
  sscanf(buf, "%lf %lf", &stime_os, &idletime);
  fclose(fp);

  seconds = (int)stime_os;

  int i;
  char buff[1024];
  unsigned long  utime,stime,starttime;
  int pcpu=0;

  char timebuf[256];
  int h,m,s;

  fp = fopen(fpath,"r");
  memset(buff,0,sizeof(buff));
  fgets(buff,sizeof(buff),fp);

  char *ptr1 = strtok(buff," ");
  for(i=1;i<=14;i++)
	ptr1 = strtok(NULL," ");
  utime = atol(ptr1);
  ptr1 = strtok(NULL," ");
  stime= atol(ptr1);
  for(i=15;i<=22;i++)
   ptr1 = strtok(NULL," ");
  starttime = atol(ptr1);
  unsigned long long total_time = utime + stime;
  unsigned long cputime = (utime+stime)/100;
  h = cputime/3600;
  m = (cputime - h*3600)/60;
  s = (cputime- h*3600 - m*60);
  sprintf(TIME,"%02d:%02d:%02d",h,m,s);

  //**계산**//
  seconds = seconds - (int)(starttime/100.0);
  if(seconds)
  {
	pcpu = (int)(total_time * 100ULL/100.0)/seconds;
  }

  sprintf(resultString,"%2d.%0d",pcpu/10,pcpu%10);

  fclose(fp);
}
void getMEM(char *fpath,char *memperc,unsigned long *vsz, long *rss)
{
  FILE *fp;
  int i;
  char buf[1024];

  fp = fopen(fpath,"r");
  memset(buf,0,sizeof(buf));
  fgets(buf,sizeof(buf),fp);
  //공백으로 구분
  char *ptr=strtok(buf," ");
  for(i=1;i<=22;i++)
	ptr = strtok(NULL," ");
  unsigned long VSZ = atol(ptr);
  VSZ= VSZ/1024;
  *vsz = VSZ;
  ptr = strtok(NULL," ");
  long RSS = atol(ptr);
  RSS = RSS*4;
  *rss = RSS;

  fclose(fp);
  fp = fopen("/proc/meminfo","r");
  char mem[1024];
  memset(mem,0,sizeof(mem));
  fgets(mem,sizeof(mem),fp);
  ptr = strtok(mem," ");
  ptr = strtok(NULL," ");
  unsigned long memTotal = atol(ptr);
  memTotal = memTotal/100;
  sprintf(memperc,"%.1f",(double)RSS/memTotal);
  fclose(fp);
}
void getSTART(char *fpath,char *start)
{
  FILE *fp;
  int i;
  char buf[1024];
  char *ptr;
  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  char now_y[64],diff_y[64];
  char now_md[64],diff_md[64];
  strftime(now_y,64,"%Y",tm_now);
  strftime(now_md,64,"%b%d",tm_now);

  fp = fopen(fpath,"r");
  memset(buf,0,sizeof(buf));
  fgets(buf,sizeof(buf),fp);

  ptr = strtok(buf," ");
  for(i=1;i<22;i++)
  {
	ptr = strtok(NULL," ");
  }
  int startTime = atoi(ptr);
  startTime = startTime/100;
  FILE *fp2;
  char buff[1024];
  fp2 = fopen("/proc/uptime","r");
  fgets(buff,sizeof(buff),fp2);
  char *ptr2;
  ptr2 = strtok(buff," ");
  int uptime = atoi(ptr2);
  time_t diff = now - (uptime - startTime);
  
  struct tm *tm = localtime(&diff);
  char result[64];

  strftime(diff_y,64,"%Y",tm);
  strftime(diff_md,64,"%b%d",tm);

//  printf("n%s d%s n%s d%s\n",now_y,diff_y,now_md,diff_md);
  if(strcmp(now_y,diff_y)!=0)
  {//년도
	strcpy(start,diff_y);
  }
  else if(strcmp(now_md,diff_md)!=0)
  {//월일
	strcpy(start,diff_md);
  }
  else
  {//시:분
	strftime(result,sizeof(result),"%R",tm);
    strcpy(start,result);
  }
}
