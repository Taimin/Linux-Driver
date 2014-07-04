#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
	int fd = -1;
	unsigned char key_vals[4] = {0, 0, 0, 0};
	int cnt = 0;
	
	fd = open("/dev/yang", O_RDWR);
	if(fd < 0)
	{
		printf("can not open yang\n");
		return -1;
	}

	while(1)
	{
		read(fd, key_vals, sizeof(key_vals));
		if(!key_vals[0]|!key_vals[1]|!key_vals[2]|!key_vals[3])
		{
			printf("%d key pressed: %d %d %d %d\n", cnt++, key_vals[0], key_vals[1], key_vals[2], key_vals[3]);
		}
	}
	
	return 0;
}
