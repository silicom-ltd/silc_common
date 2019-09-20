/*
 * silc_file.h
 *
 *  Created on: Dec 13, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#include "silc_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/mman.h>

int silc_file_path_remove2(const silc_cstr path1, silc_cstr path2);

int silc_file_path_remove(const silc_cstr path)
{
	struct stat64 st;

	//important, use lstat, as we don't delete stuff from within symnlink dir
	if(0 == lstat64(path, &st))
	{
		if(!(st.st_mode & S_IFDIR))
		{
			return unlink(path);
		}
	}

	DIR * p_dir = opendir(path);
	struct dirent64 ent, *p_ent = &ent, *p_result;
	while(1)
	{
		int ret = readdir64_r(p_dir, p_ent, &p_result);
		if(0 == strcmp(p_ent->d_name, ".")  || 0 == strcmp(p_ent->d_name, "..") )
			continue;
		if(ret != 0||p_result == NULL)
			break;
		if(0 != silc_file_path_remove2(path, p_ent->d_name))
		{
			closedir(p_dir);
			return -1;
		}
	}
	closedir(p_dir);

	if(0 != rmdir(path))
		return -1;
	return 0;
}


int silc_file_path_remove2(const silc_cstr path1, const silc_cstr path2)
{
	silc_file_path_form2(tmp_path, path1, path2);

	return silc_file_path_remove(tmp_path.str);
}

int silc_file_dir_create(const silc_cstr path)
{
	struct stat64 st;
	if(0 == stat64(path, &st))
	{
		if(st.st_mode & S_IFDIR)
			return 0;
		else
		{//not a directory, remove so we can create
			if(0 != silc_file_path_remove(path))
				return -1;
		}
	}
	//create as 0775
	if(0 != mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH))
	{
		return -1;
	}
	return 0;
}

int silc_file_path_get_type(const silc_cstr path, silc_file_type_t* ret_type)
{
	struct stat64 st;
	if(0 == stat64(path, &st))
	{
		if(st.st_mode & S_IFDIR)
			*ret_type = SILC_FILE_TYPE_DIR;
		else if (st.st_mode & S_IFREG)
			*ret_type = SILC_FILE_TYPE_REG;
		else
			*ret_type = SILC_FILE_TYPE_OTH;
		return 0;
	}
	return -1;
}

#define SILC_FILE_FILL_SEG	(1<<20)
int silc_file_create_file(const silc_cstr path, uint64_t size, silc_bool fill_zero)
{
	int fd;
	fd = open(path, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if(fd < 0)
	{
		return -1;
	}
	if(fill_zero)
	{
		close(fd);
		return 0;
	}
	//now fill 0
	uint64_t loop = 0, loop_max = size/SILC_FILE_FILL_SEG;
	uint64_t written = 0;

	uint32_t alloc_size = size;
	if(loop_max)
		alloc_size = SILC_FILE_FILL_SEG;
	char* buf = malloc(alloc_size);
	if(buf == NULL)
	{
		close(fd);
		unlink(path);
		return -1;
	}
	memset(buf, 0, alloc_size);

	int ret;
	while(loop < loop_max)
	{
		ret = write(fd, buf, SILC_FILE_FILL_SEG);
		if(ret != SILC_FILE_FILL_SEG)
		{
			close(fd);
			unlink(path);
			free(buf);
			return -1;
		}
		loop++;
		written += SILC_FILE_FILL_SEG;
	}
	ret = write(fd, buf, size - written);
	if(ret != size - written)
	{
		close(fd);
		unlink(path);
		free(buf);
		return -1;
	}

	free(buf);
	close(fd);
	return 0;

}

int silc_file_open_file(const silc_cstr path)
{
	int fd;
	fd = open(path, O_RDWR);
	return fd;
}

int silc_file_create_file2(const silc_cstr path1, const silc_cstr path2, uint64_t size, silc_bool fill_zero)
{
	silc_file_path_form2(tmp_path, path1, path2);

	return silc_file_create_file(tmp_path.str, size, fill_zero);

}

int silc_file_map_file(int fd, uint64_t length, uint64_t offset, silc_file_map_t* p_map)
{
	void* p_ret;
	uint64_t real_off = SILC_MEM_PAGE_ALIGN_BASE(offset, uint64_t);
	uint64_t real_offend = SILC_MEM_PAGE_ALIGN_END((offset + length), uint64_t);
	uint64_t map_size = real_offend - real_off;
	p_ret = mmap64(NULL, map_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd,  real_off);
	if(p_ret == MAP_FAILED)
	{
		SILC_ERR("Map file failed, err:%s", strerror(errno));
		return -1;
	}

	uint64_t usr_off = offset - real_off;
	p_map->p_map_usr = (uint8_t*)p_ret + usr_off;
	p_map->usr_length = length;

	p_map->_p_map = p_ret;
	p_map->_map_length = map_size;

	return 0;
}

int silc_file_unmap_file(silc_file_map_t* p_map)
{
	if(0 != munmap(p_map->_p_map, p_map->_map_length))
	{
		SILC_ERR("unmap file failed, err:%s", strerror(errno));
		return -1;
	}

	p_map->_p_map = NULL;
	return 0;
}
int silc_file_write_all(const char* fname, const char* p_buf, uint32_t size)
{
	int fd = open(fname, O_CREAT|O_WRONLY|O_TRUNC,  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if(fd < 0)
		return -1;

	size_t to_write = size;
	size_t already_write = 0;
	int ret;
	while(to_write)
	{
		ret = write(fd, p_buf+already_write, to_write);
		if(ret<=0)
		{
			close(fd);
			return -1;
		}
		to_write -= ret;
		already_write += ret;
	}

	close(fd);
	return 0;
}

int silc_file_read_all(const char* fname, char** pp_ret_data, uint32_t* p_size)
{
	struct stat64 st;
	if(0 != stat64(fname, &st))
		return -1;
	if(st.st_size > (100*SILC_SIZE_MB))
	{//file too big
		return -1;
	}
	int fd = open(fname, O_RDONLY);
	if(fd < 0)
		return -1;
	char* p_buf = malloc(st.st_size+1024);

	size_t to_read = st.st_size;
	size_t already_read = 0;
	int ret;
	while(to_read)
	{
		ret = read(fd, p_buf+already_read, to_read);
		if(ret <= 0)
		{
			close(fd);
			free(p_buf);
			return -1;
		}
		to_read -= ret;
		already_read += ret;
	}
	close(fd);
	p_buf[st.st_size]=0;
	*pp_ret_data = p_buf;
	*p_size = st.st_size;
	return 0;
}

int silc_file_dir_for_each(const silc_cstr path, silc_file_dir_for_each_cbf p_cbf, void* p_arg)
{
	DIR* p_dir;
	struct dirent ent, * p_ent;
	uint32_t path_len = strlen(path);
	char buf[path_len + 512];

	p_dir = opendir(path);
	if(p_dir == NULL)
	{
		return -1;
	}

	silc_file_type_t ftype;

	while(1)
	{
		if(0!=readdir_r(p_dir, &ent, &p_ent))
		{
			closedir(p_dir);
			return -1;
		}
		if(p_ent == NULL)
		{
			closedir(p_dir);
			return 0;
		}
		if(0 == strcmp(ent.d_name, ".")||0 == strcmp(ent.d_name, ".."))
		{
			continue;
		}
		strcpy(buf, path);
		buf[path_len] = '/';
		strcpy(buf + path_len + 1, ent.d_name);
		if(0 != silc_file_path_get_type(buf, &ftype))
		{
			closedir(p_dir);
			return -1;
		}
		if(0 != p_cbf(path, ent.d_name, buf, ftype, p_arg))
		{
			closedir(p_dir);
			return -1;
		}
	}

	return 0;
}
