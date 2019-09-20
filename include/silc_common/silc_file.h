/*
 * silc_file.h
 *
 *  Created on: Dec 13, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_FILE_H_
#define SILC_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum silc_file_type_e
{
	SILC_FILE_TYPE_DIR = 1,
	SILC_FILE_TYPE_REG = 2,
	SILC_FILE_TYPE_OTH = 3,
}silc_file_type_t;

#define  silc_file_path_form2(tmp_path__, path1__, path2__)	\
		uint32_t len1__##tmp_path__ = strlen(path1__);	\
		uint32_t len2__##tmp_path__ = strlen(path2__);	\
		uint32_t len__##tmp_path__ = len1__##tmp_path__ + len2__##tmp_path__ + 2;	\
		silc_str_declare(tmp_path__, len__##tmp_path__);	\
		snprintf(tmp_path__.str, len__##tmp_path__, "%s/%s",path1__, path2__);	\
		tmp_path__.str[len__##tmp_path__-1] = 0;	\



int silc_file_path_remove(const silc_cstr path);
int silc_file_path_remove2(const silc_cstr path1, const silc_cstr path2);

int silc_file_dir_create(const silc_cstr path);
int silc_file_path_get_type(const silc_cstr path, silc_file_type_t* ret_type);

int silc_file_create_file(const silc_cstr path, uint64_t size, silc_bool fill_zero);
int silc_file_create_file2(const silc_cstr path1, const silc_cstr path2, uint64_t size, silc_bool fill_zero);
int silc_file_open_file(const silc_cstr path);

typedef struct silc_file_map_s
{
	uint8_t* p_map_usr;
	uint64_t usr_length;
	void* _p_map;
	uint64_t _map_length;
}silc_file_map_t;

int silc_file_map_file(int fd, uint64_t length, uint64_t offset, silc_file_map_t* p_map);
int silc_file_unmap_file(silc_file_map_t* p_map);

/**
 * @ingroup silc_file
 * @brief Read the whole content of a file into a allocated buffer, and return the buffer and size
 *
 * @param[in] fname 	        path to the file
 * @param[out] pp_ret_data 	    returned allocated buffer, containg the content of the file
 * @param[out] p_size			the size of the file and the content
 * @return 0 for success, non-zero for Error
 *
 * Read the whole content of a file into a allocated buffer, and return the buffer and size.
 * Note, if the file is a text file, the returned buffer will be null terminated, and can
 * be used to as a c string.
 **/
int silc_file_read_all(const char* fname, char** pp_ret_data, uint32_t* p_size);
int silc_file_write_all(const char* fname, const char* p_buf, uint32_t size);

typedef int (*silc_file_dir_for_each_cbf)(const silc_cstr path, const silc_cstr fname, const silc_cstr full_path, silc_file_type_t ftype, void* p_arg);
int silc_file_dir_for_each(const silc_cstr path, silc_file_dir_for_each_cbf p_cbf, void* p_arg);

#ifdef __cplusplus
}
#endif

#endif /* SILC_FILE_H_ */
