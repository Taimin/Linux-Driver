#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
	int fd = -1;
	unsigned char key_val;
	int cnt = 0;
	
	fd = open("/dev/yang", O_RDWR);
	if(fd < 0)
	{
		printf("can not open yang\n");
		return -1;
	}

	while(1)
	{
		read(fd, &key_val, 1);
		
		printf("%d key pressed: 0x%x\n", cnt++, key_val);
		
	}
	
	return 0;
}

