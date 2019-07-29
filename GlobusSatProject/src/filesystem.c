#include <at91/utility/trace.h>
#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>
#include <hal/errors.h>

#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>

#include <stdlib.h>
#include <string.h>

#include "fileSystem.h"
#include "GlobalStandards.h"

#define SKIP_FILE_TIME_SEC 1000000
#define _SD_CARD 0
#define FIRST_TIME -1
#define FILE_NAME_WITH_INDEX_SIZE MAX_F_FILE_NAME_SIZE+sizeof(int)*2

//struct for filesystem info
typedef struct
{
	int num_of_files;
} FS;

//struct for chain file info
typedef struct
{
	int size_of_element;
	char name[FILE_NAME_WITH_INDEX_SIZE];
	unsigned long int creation_time;
	unsigned int last_time_modified;
	int num_of_files;

} C_FILE;
#define C_FILES_BASE_ADDR (FSFRAM+sizeof(FS))

// return -1 for FRAM fail
static int getNumOfFilesInFS()
{
	FS fs;
	if (FRAM_read((unsigned char*) &fs, FSFRAM, sizeof(FS)) != 0) {
		return -1;
	}
	return fs.num_of_files;
}

//return -1 on fail
static int setNumOfFilesInFS(int new_num_of_files)
{
	FS fs;
	fs.num_of_files = new_num_of_files;
	if (FRAM_write((unsigned char*) &fs, FSFRAM, sizeof(FS)) != 0) {
		return -1;
	}
	return 0;
}

FileSystemResult InitializeFS(Boolean first_time)
{
	int ret;
	hcc_mem_init(); // Initialize the memory to be used by filesystem

	ret = fs_init(); // Initialize the filesystem
	if (ret != F_NO_ERROR) {
		TRACE_ERROR("fs_init pb: %d\n\r", ret);
		return FS_FAT_API_FAIL;
	}

	ret = f_enterFS(); // Register this task with filesystem
	if (ret != F_NO_ERROR) {
		TRACE_ERROR("f_enterFS pb: %d\n\r", ret);
		return FS_FAT_API_FAIL;
	}

	ret = f_initvolume(0, atmel_mcipdc_initfunc, _SD_CARD); // Initialize volID as safe

	if ( F_ERR_NOTFORMATTED == ret) {
		TRACE_ERROR("Filesystem not formated!\n\r");
		return FS_FAT_API_FAIL;
	} else if ( F_NO_ERROR != ret) {
		TRACE_ERROR("f_initvolume pb: %d\n\r", ret);
		return FS_FAT_API_FAIL;
	}
	if (first_time) {
		FS fs = { 0 };
		if (FRAM_write((unsigned char*) &fs, FSFRAM, sizeof(FS)) != 0) {
			return FS_FAT_API_FAIL;
		}
	}

	F_SPACE space;
	// get free space on current drive
	ret = f_getfreespace(f_getdrive(), &space);
	if (F_NO_ERROR != ret)
		printf("\nError %d reading drive\n", ret);


	return FS_SUCCSESS;
}

//only register the chain, files will create dynamically
FileSystemResult c_fileCreate(char* c_file_name, int size_of_element)
{
	if (strlen(c_file_name) > MAX_F_FILE_NAME_SIZE) //check len
	{
		return FS_TOO_LONG_NAME;
	}

	C_FILE c_file; //chain file descriptor
	strcpy(c_file.name, c_file_name);
	Time_getUnixEpoch(&c_file.creation_time); //get current time
	c_file.size_of_element = size_of_element;
	c_file.last_time_modified = FIRST_TIME; //no written yet
	int num_of_files_in_FS = getNumOfFilesInFS();
	if (num_of_files_in_FS == -1) {
		return FS_FRAM_FAIL;
	}
	int c_file_address = C_FILES_BASE_ADDR
			+ num_of_files_in_FS * sizeof(C_FILE);
	if (FRAM_write((unsigned char*) &c_file, c_file_address, sizeof(C_FILE))
			!= 0) //write c_file struct in FRAM
			{
		return FS_FRAM_FAIL;
	}
	if (setNumOfFilesInFS(num_of_files_in_FS + 1) != 0) //TODO change to c_fil
			{
		return FS_FRAM_FAIL;
	}

	return FS_SUCCSESS;
}

//write element with timestamp to file
static void writewithEpochtime(F_FILE* file, char* data, int size,
		unsigned int time)
{
	int number_of_writes;
	number_of_writes = f_write(&time, sizeof(unsigned int), 1, file);
	number_of_writes += f_write(data, size, 1, file);
	printf("writing element, time is: %u\n", time);
	if (number_of_writes != 2) {
		printf("writewithEpochtime error\n");
	}
	f_flush(file); /* only after flushing can data be considered safe */
	f_close(file); /* data is also considered safe when file is closed */
}

// get C_FILE struct from FRAM by name
static Boolean get_C_FILE_struct(char* name, C_FILE* c_file,
		unsigned int *address)
{
	int i;
	unsigned int c_file_address = 0;
	int err_read = 0;
	int num_of_files_in_FS = getNumOfFilesInFS();
	for (i = 0; i < num_of_files_in_FS; i++)	//search correct c_file struct
			{
		c_file_address = C_FILES_BASE_ADDR + sizeof(C_FILE) * (i);
		err_read = FRAM_read((unsigned char*) c_file, c_file_address,
				sizeof(C_FILE));
		if (0 != err_read) {
			printf("FRAM error in 'get_C_FILE_struct()' error = %d\n",
					err_read);
			return FALSE;
		}

		if (!strcmp(c_file->name, name)) {
			if (address != NULL) {
				*address = c_file_address;
			}
			return TRUE;			//stop when found
		}
	}
	return FALSE;
}

//calculate index of file in chain file by time
static int getFileIndex(unsigned int creation_time, unsigned int current_time)
{
	return ((current_time - creation_time) / SKIP_FILE_TIME_SEC);
}

//write to curr_file_name
void get_file_name_by_index(char* c_file_name, int index, char* curr_file_name)
{
	sprintf(curr_file_name, "%s%d", c_file_name, index);
}

FileSystemResult c_fileReset(char* c_file_name)
{
	C_FILE c_file;
	unsigned int addr;			//FRAM ADDRESS

	char curr_file_name[MAX_F_FILE_NAME_SIZE + sizeof(int) * 2];
	unsigned long int curr_time;
	Time_getUnixEpoch(&curr_time);
	if (get_C_FILE_struct(c_file_name, &c_file, &addr) != TRUE)		//get c_file
	{
		return FS_NOT_EXIST;
	}
	for (int i = 0; i < c_file.num_of_files; i++) {
		get_file_name_by_index(c_file_name, i, curr_file_name);
		f_delete(c_file_name);
	}
	c_file.last_time_modified = -1;
	c_file.creation_time = curr_time;
	return FS_SUCCSESS;
}

FileSystemResult c_fileWrite(char* c_file_name, void* element)
{
	C_FILE c_file;
	F_FILE *file;
	unsigned int addr;			//FRAM ADDRESS
	unsigned long int curr_time;
	char curr_file_name[MAX_F_FILE_NAME_SIZE + sizeof(int) * 2];

	Time_getUnixEpoch(&curr_time);
	if (get_C_FILE_struct(c_file_name, &c_file, &addr) != TRUE)		//get c_file
	{
		return FS_NOT_EXIST;
	}
	int index_current = getFileIndex(c_file.creation_time, curr_time);
	get_file_name_by_index(c_file_name, index_current, curr_file_name);
	file = f_open(curr_file_name, "a+");
	writewithEpochtime(file, element, c_file.size_of_element, curr_time);
	c_file.last_time_modified = curr_time;
	if (FRAM_write((unsigned char *) &c_file, addr, sizeof(C_FILE)) != 0)//update last written
			{
		return FS_FRAM_FAIL;
	}
	f_close(file);
	return FS_SUCCSESS;
}

FileSystemResult c_fileRead(char* c_file_name, char* buffer, int size_of_buffer,
		unsigned long from_time, unsigned long to_time, int* read,
		unsigned long* last_read_time)
{
	unsigned int addr;			//FRAM ADDRESS
	unsigned int buffer_index = 0;

	C_FILE c_file;
	char curr_file_name[MAX_F_FILE_NAME_SIZE + sizeof(int) * 2];
	void* element;

	if (get_C_FILE_struct(c_file_name, &c_file, &addr) != TRUE)		//get c_file
	{
		return FS_NOT_EXIST;
	}
	if (from_time < c_file.creation_time) {
		from_time = c_file.creation_time;
	}
	F_FILE* current_file;
	int index_current = getFileIndex(c_file.creation_time, from_time);
	get_file_name_by_index(c_file_name, index_current, curr_file_name);
	unsigned int size_elementWithTimeStamp = c_file.size_of_element
			+ sizeof(unsigned int);
	element = malloc(size_elementWithTimeStamp);//store element and his timestamp
	do {
		get_file_name_by_index(c_file_name, index_current++, curr_file_name);
		current_file = f_open(curr_file_name, "r");
		unsigned int length = f_filelength(curr_file_name)
				/ (size_elementWithTimeStamp);//number of elements in currnet_file

		f_seek(current_file, 0L, SEEK_SET);
		for (unsigned int j = 0; j < length; j++)
		{
			f_read(element, (size_t )size_elementWithTimeStamp,
					(size_t )1, current_file);

			unsigned int element_time = *((unsigned int*) element);
			printf("read element, time is %u\n", element_time);
			if (element_time > to_time) {
				break;
			}

			if (element_time >= from_time) {
				*last_read_time = element_time;
				if (buffer_index > (unsigned int) size_of_buffer) {
					free(element);
					return FS_BUFFER_OVERFLOW;
				}
				(*read)++;
				memcpy(buffer + buffer_index, element,
						size_elementWithTimeStamp);
				buffer_index += size_elementWithTimeStamp;
			}
		}
		f_close(current_file);
	}
	while (getFileIndex(c_file.creation_time, c_file.last_time_modified)
			>= index_current);

	free(element);

	return FS_SUCCSESS;
}

void DeInitializeFS(void)
{
	printf("deinitializig file system \n");
	int err = f_delvolume(_SD_CARD); /* delete the volID */

	if (err != 0) {
		printf("f_delvolume err %d\n", err);
	}

	f_releaseFS(); /* release this task from the filesystem */

	err = fs_delete(); /* delete the filesystem */

	if (err != 0) {
		printf("fs_delete err , %d\n", err);
	}
	err = hcc_mem_delete(); /* free the memory used by the filesystem */

	if (err != 0) {
		printf("hcc_mem_delete err , %d\n", err);
	}
	printf("deinitializig file system end \n");
}
