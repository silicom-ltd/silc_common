#include <dirent.h>
#include <sys/ioctl.h>
#include "silc_common.h"

int silc_gpio_get_chip_num()
{
	const struct dirent *ent;
	DIR *dp;
	int ret;
	char chip_num = 0;
	if(ret > 0)
		;
	/* List all GPIO devices one at a time */
	dp = opendir("/dev");
	if (!dp)
	{
		ret = -errno;
		SILC_ERR("Failed to open directory%s\n",strerror(errno));
		goto error_out;
	}
	ret = -ENOENT;
	while ((ent = readdir(dp)) != NULL)
	{
		if(strlen(ent->d_name) > strlen("gpiochip") &&
		  strncmp(ent->d_name, "gpiochip", strlen("gpiochip")) == 0)
		{
			chip_num ++;
		}
	}
error_out:
	if (closedir(dp) == -1)
	{
		ret = -errno;
		SILC_ERR("Failed to close directory%s\n",strerror(errno));
	}
	return chip_num;
}

int silc_gpio_get_by_name(const char*name,char*bank_num,char*pin_num)
{
	struct gpiochip_info cinfo;
	char *chrdev_name;
	int fd = -1;
	char bank = 0;
	int pin = 0;
	int chip_num = 0,ret = 0;
	chip_num = silc_gpio_get_chip_num();
	if(chip_num < 0)
	{
		SILC_ERR("get_gpio_chip_num fail \n");
		return -1;
	}
	for(bank = 0; bank < chip_num; bank ++)
	{
		ret = asprintf(&chrdev_name, "/dev/gpiochip%d", bank);
		if (ret < 0)
				return -ENOMEM;
		fd = open(chrdev_name, 0);
		if (fd == -1)
		{
			ret = -errno;
			SILC_ERR("Failed to open %s:%s\n", chrdev_name,strerror(errno));
			goto exit_close_error;
		}
		ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &cinfo);
		if (ret == -1)
		{
			ret = -errno;
			SILC_ERR("Failed to issue CHIPINFO IOCTL:%s\n",strerror(errno));
			goto exit_close_error;
		}
		//fprintf(stdout, "GPIO chip: %s, \"%s\", %u GPIO lines\n",cinfo.name, cinfo.label, cinfo.lines);
		for (pin = 0; pin < cinfo.lines; pin ++)
		{
			struct gpioline_info linfo;
			memset(&linfo, 0, sizeof(linfo));
			linfo.line_offset = pin;
			ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &linfo);
			if (ret == -1)
			{
				ret = -errno;
				SILC_ERR("Failed to issue LINEINFO IOCTL\n");
				goto exit_close_error;
			}
			if (linfo.name[0])
			{
				if(strcmp(linfo.name,name) == 0)
				{
					//ret = SILC_GPIO_TO_PIN(bank,pin);
					*bank_num = bank;
					*pin_num = pin;
					//fprintf(stderr,"gpio pin is %d bank is %d ,pin is %d name is %s\n",ret,*bank_num,*pin_num,linfo.name);
					return 0;
				}
			}
			else
			{
				;//unamed
			}
		}
	}
	ret = -ENXIO;
	SILC_ERR("can not find the gpio :%s",name);
exit_close_error:
	if (close(fd) == -1)
		SILC_ERR("Failed to close GPIO character device file");
	free(chrdev_name);
	return ret;
}
