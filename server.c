/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include <stdio.h> // For printf, etc.
#include <rpc/rpc.h> // For RPC facilities.
#include <string.h> // For strcpy, strcmp, strdup, strlen, etc.
#include "ssnfs.h" // Automatically generated rpc service header.
#include <unistd.h> // Needed for access, etc.
#include <fcntl.h> // Needed for ftruncate, etc.
#include <errno.h> // Need for errno, strerror, etc.
#include <stdlib.h>
#define MAX_FILES 20
#define BLOCK_SIZE 512
#define FILE_BLOCKS 64
#define DISK_SIZE (16*1024*1024)
#define TOTAL_BLOCKS (DISK_SIZE / BLOCK_SIZE)

typedef struct {
    char username[USER_NAME_SIZE];
    char filename[FILE_NAME_SIZE];
    int used;
    int blocks[FILE_BLOCKS];
} file_info;

typedef struct {
    int used;
    char username[USER_NAME_SIZE];
    char filename[FILE_NAME_SIZE];
    int file_table_index;
    int file_pointer;
} open_file_entry;

file_info file_table[MAX_FILES];
open_file_entry open_file_table[MAX_FILES];
unsigned char block_bitmap[TOTAL_BLOCKS / 8];

void init_virtual_disk() {
    static int disk_initialized = 0;
    if (disk_initialized)
        return;

    FILE *disk = fopen("virtual_disk.dat", "rb");
    if (!disk) {
        disk = fopen("virtual_disk.dat", "wb");
        if (!disk) {
            perror("Error: Unable to create virtual disk.");
            exit(1);
        }
        fseek(disk, DISK_SIZE - 1, SEEK_SET);
        fputc('\0', disk);
    }
    fclose(disk);
    disk_initialized = 1;
}

int read_from_disk(char *buffer, file_info *file, int offset, int size) {
    init_virtual_disk();

    FILE *disk = fopen("virtual_disk.dat", "rb");
    if (!disk) {
        perror("Error: Unable to open virtual disk for reading.");
        return -1;
    }

    int block_start = offset / BLOCK_SIZE;
    int block_offset = offset % BLOCK_SIZE;
    int bytes_remaining = size;
    int bytes_read = 0;

    while (bytes_remaining > 0 && block_start < FILE_BLOCKS) {
        int block_index = file->blocks[block_start];
        fseek(disk, block_index * BLOCK_SIZE + block_offset, SEEK_SET);
        int to_read = BLOCK_SIZE - block_offset;
        if (to_read > bytes_remaining)
            to_read = bytes_remaining;

        int read = fread(buffer + bytes_read, 1, to_read, disk);
        if (read <= 0) {
            fclose(disk);
            return bytes_read;
        }

        bytes_read += read;
        bytes_remaining -= read;
        block_start++;
        block_offset = 0;
    }

    fclose(disk);
    return bytes_read;
}

int write_to_disk(char *buffer, file_info *file, int offset, int size) {
    init_virtual_disk();

    FILE *disk = fopen("virtual_disk.dat", "rb+");
    if (!disk) {
        perror("Error: Unable to open virtual disk for writing.");
        return -1;
    }

    int block_start = offset / BLOCK_SIZE;
    int block_offset = offset % BLOCK_SIZE;
    int bytes_remaining = size;
    int bytes_written = 0;

    while (bytes_remaining > 0 && block_start < FILE_BLOCKS) {
        int block_index = file->blocks[block_start];
        fseek(disk, block_index * BLOCK_SIZE + block_offset, SEEK_SET);
        int to_write = BLOCK_SIZE - block_offset;
        if (to_write > bytes_remaining)
            to_write = bytes_remaining;

        int written = fwrite(buffer + bytes_written, 1, to_write, disk);
        if (written <= 0) {
            fclose(disk);
            return bytes_written;
        }

        bytes_written += written;
        bytes_remaining -= written;
        block_start++;
        block_offset = 0;
    }

    fclose(disk);
    return bytes_written;
}

void init_disk() {
    static int initialized = 0;
    if (initialized)
        return;

    init_virtual_disk();

    memset(block_bitmap, 0, sizeof(block_bitmap));
    memset(file_table, 0, sizeof(file_table));
    memset(open_file_table, 0, sizeof(open_file_table));

    initialized = 1;
}

int file_exists(char *user_name, char *file_name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].username, user_name) == 0 &&
            strcmp(file_table[i].filename, file_name) == 0) {
            return 1;
        }
    }
    return 0;
}

int allocate_fd(char *user_name, char *file_name) {
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].username, user_name) == 0 &&
            strcmp(file_table[i].filename, file_name) == 0) {
            file_index = i;
            break;
        }
    }
    if (file_index == -1) {
        return -1; 
    }
    for (int i = 0; i < MAX_FILES; i++) {
        if (!open_file_table[i].used) {
            open_file_table[i].used = 1;
            strcpy(open_file_table[i].username, user_name);
            strcpy(open_file_table[i].filename, file_name);
            open_file_table[i].file_table_index = file_index;
            open_file_table[i].file_pointer = 0; 
            return i;
        }
    }
    return -1; 
}

int get_existing_fd(char *user_name, char *file_name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (open_file_table[i].used &&
            strcmp(open_file_table[i].username, user_name) == 0 &&
            strcmp(open_file_table[i].filename, file_name) == 0) {
            return i;
        }
    }
    return allocate_fd(user_name, file_name);
}

int get_free_blocks(int num_blocks, int *blocks) {
    for (int i = 0; i <= TOTAL_BLOCKS - num_blocks; i++) {
        int j;
        for (j = 0; j < num_blocks; j++) {
            int block_index = i + j;
            int byte_index = block_index / 8;
            int bit_index = block_index % 8;
            if (block_bitmap[byte_index] & (1 << bit_index)) {
                break;
			}
        }
        if (j == num_blocks) {
            for (j = 0; j < num_blocks; j++) {
                int block_index = i + j;
                int byte_index = block_index / 8;
                int bit_index = block_index % 8;
                block_bitmap[byte_index] |= (1 << bit_index);
                blocks[j] = block_index;
            }
            return 1; 
        }
    }
    return 0;
}

int add_file(file_info * new_file) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i] = *new_file;
            file_table[i].used = 1;
            return 1;
        }
    }
    return -1;
}

open_output *
open_file_1_svc(open_input *argp, struct svc_req *rqstp)
{
    static open_output result;
    char *user_name = argp->user_name;
    char *file_name = argp->file_name;
    int fd;

    result.fd = -1;
    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
    }
    result.out_msg.out_msg_val = NULL;
    result.out_msg.out_msg_len = 0;

    printf("open_file_1_svc: User '%s' requested file '%s'.\n", user_name, file_name);

    init_disk();
    init_virtual_disk();

    if (file_exists(user_name, file_name)) {
        fd = get_existing_fd(user_name, file_name);
        if (fd == -1) {
            result.fd = -1;
            result.out_msg.out_msg_val = strdup("Error: Unable to open existing file.");
            result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
            return &result;
        }
        result.fd = fd;
        result.out_msg.out_msg_val = strdup("File opened successfully.");
    } else {
        file_info new_file;
        memset(&new_file, 0, sizeof(file_info));
        strncpy(new_file.username, user_name, USER_NAME_SIZE - 1);
        strncpy(new_file.filename, file_name, FILE_NAME_SIZE - 1);
        new_file.used = 1;

        int blocks[FILE_BLOCKS];
        if (!get_free_blocks(FILE_BLOCKS, blocks)) {
            result.fd = -1;
            result.out_msg.out_msg_val = strdup("Error: No free blocks available to create a new file.");
            result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
            return &result;
        }
        memcpy(new_file.blocks, blocks, sizeof(blocks));

        int file_index = add_file(&new_file);
        if (file_index == -1) {
            result.fd = -1;
            result.out_msg.out_msg_val = strdup("Error: File table is full.");
            result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
            return &result;
        }

        fd = allocate_fd(user_name, file_name);
        if (fd == -1) {
            result.fd = -1;
            result.out_msg.out_msg_val = strdup("Error: Unable to allocate file descriptor.");
            result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
            return &result;
        }
        result.fd = fd;
        result.out_msg.out_msg_val = strdup("File created and opened successfully.");
    }

    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
    return &result;
}

read_output *
read_file_1_svc(read_input *argp, struct svc_req *rqstp)
{

	static read_output  result;
	char *user_name = argp->user_name;
	int fd = argp->fd;
	int size = argp->numbytes;
	char *buffer;

	result.success = -1;
	if (result.out_msg.out_msg_val != NULL){
		free(result.out_msg.out_msg_val);
	}
	result.out_msg.out_msg_val = NULL;
	result.out_msg.out_msg_len = 0;
	if (result.buffer.buffer_val != NULL){
		free(result.buffer.buffer_val);
	}
	result.buffer.buffer_val = NULL;
	result.buffer.buffer_len = 0;

	if (fd < 0 || fd >= MAX_FILES || !open_file_table[fd].used || strcmp(open_file_table[fd].username, user_name) != 0){
		result.success = -1;
		result.out_msg.out_msg_val = strdup("Error: Invalid file descriptor or user mismatch.");
		result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
		return &result;
	}

	open_file_entry *file_entry = &open_file_table[fd];
	file_info *file = &file_table[file_entry->file_table_index];

	int offset = file_entry->file_pointer;
	int max_size = FILE_BLOCKS * BLOCK_SIZE - offset;
	if (size > max_size){
		size = max_size;
	}

	buffer = malloc(size);
	if (!buffer){
		result.success = -1;
		result.out_msg.out_msg_val = strdup("Error: Memory allocation failed.");
		result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
		return &result;
	}
	
	int bytes_read = read_from_disk(buffer, file, offset, size);
	if (bytes_read < 0){
		free(buffer);
		result.success = -1;
		result.out_msg.out_msg_val = strdup("Error: Failed to read from disk.");
		result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
		return &result;
	}

	file_entry->file_pointer += bytes_read;

    result.success = bytes_read;
    result.buffer.buffer_val = buffer;
    result.buffer.buffer_len = bytes_read;
    result.out_msg.out_msg_val = strdup("Read successful.");
    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;

	return &result;
}

write_output *
write_file_1_svc(write_input *argp, struct svc_req *rqstp)
{
    static write_output result;
    char *user_name = argp->user_name;
    int fd = argp->fd;
    int size = argp->numbytes;
    char *buffer = argp->buffer.buffer_val;

    result.success = -1;
    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
    }
    result.out_msg.out_msg_val = NULL;
    result.out_msg.out_msg_len = 0;

    if (fd < 0 || fd >= MAX_FILES || !open_file_table[fd].used ||
        strcmp(open_file_table[fd].username, user_name) != 0) {
        result.success = -1;
        result.out_msg.out_msg_val = strdup("Error: Invalid file descriptor or user mismatch.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    open_file_entry *file_entry = &open_file_table[fd];
    file_info *file = &file_table[file_entry->file_table_index];

    int offset = file_entry->file_pointer;
    int max_size = FILE_BLOCKS * BLOCK_SIZE - offset;
    if (size > max_size) {
        size = max_size;
    }

    int bytes_written = write_to_disk(buffer, file, offset, size);
    if (bytes_written < 0) {
        result.success = -1;
        result.out_msg.out_msg_val = strdup("Error: Failed to write to disk.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    file_entry->file_pointer += bytes_written;

    result.success = bytes_written;
    result.out_msg.out_msg_val = strdup("Write successful.");
    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;

    return &result;
}

list_output *
list_files_1_svc(list_input *argp, struct svc_req *rqstp)
{
    static list_output result;
    char *user_name = argp->user_name;
    char *file_list_str = NULL;
    size_t total_length = 0;

    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
        result.out_msg.out_msg_val = NULL;
    }
    result.out_msg.out_msg_len = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].username, user_name) == 0) {

            char *file_name = file_table[i].filename;
            size_t name_length = strlen(file_name);

            char *new_str = realloc(file_list_str, total_length + name_length + 2);
            if (!new_str) {
                free(file_list_str);
                result.out_msg.out_msg_val = strdup("Error: Memory allocation failed.");
                result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
                return &result;
            }
            file_list_str = new_str;

            strcpy(file_list_str + total_length, file_name);
            total_length += name_length;
            file_list_str[total_length] = '\n';
            total_length += 1;
            file_list_str[total_length] = '\0';
        }
    }

    if (file_list_str == NULL) {
        result.out_msg.out_msg_val = strdup("No files found.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
    } else {
        result.out_msg.out_msg_val = file_list_str;
        result.out_msg.out_msg_len = total_length + 1;
    }

    return &result;
}

delete_output *
delete_file_1_svc(delete_input *argp, struct svc_req *rqstp)
{
    static delete_output result;
    char *user_name = argp->user_name;
    char *file_name = argp->file_name;

    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
        result.out_msg.out_msg_val = NULL;
    }
    result.out_msg.out_msg_len = 0;

    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].username, user_name) == 0 &&
            strcmp(file_table[i].filename, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        result.out_msg.out_msg_val = strdup("Error: File not found.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    for (int i = 0; i < FILE_BLOCKS; i++) {
        int block_index = file_table[file_index].blocks[i];
        int byte_index = block_index / 8;
        int bit_index = block_index % 8;
        block_bitmap[byte_index] &= ~(1 << bit_index);
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (open_file_table[i].used &&
            strcmp(open_file_table[i].username, user_name) == 0 &&
            strcmp(open_file_table[i].filename, file_name) == 0) {
            open_file_table[i].used = 0;
        }
    }

    file_table[file_index].used = 0;

    result.out_msg.out_msg_val = strdup("File deleted successfully.");
    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;

    return &result;
}

close_output *
close_file_1_svc(close_input *argp, struct svc_req *rqstp)
{
    static close_output result;
    char *user_name = argp->user_name;
    int fd = argp->fd;

    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
        result.out_msg.out_msg_val = NULL;
    }
    result.out_msg.out_msg_len = 0;

    if (fd < 0 || fd >= MAX_FILES ||
        !open_file_table[fd].used ||
        strcmp(open_file_table[fd].username, user_name) != 0) {
        result.out_msg.out_msg_val = strdup("Error: Invalid file descriptor or user mismatch.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    open_file_table[fd].used = 0;

    result.out_msg.out_msg_val = strdup("File closed successfully.");
    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;

    return &result;
}

seek_output *
seek_position_1_svc(seek_input *argp, struct svc_req *rqstp)
{
    static seek_output result;
    char *user_name = argp->user_name;
    int fd = argp->fd;
    int position = argp->position;

    if (result.out_msg.out_msg_val != NULL) {
        free(result.out_msg.out_msg_val);
        result.out_msg.out_msg_val = NULL;
    }
    result.out_msg.out_msg_len = 0;
    result.success = -1;

    if (fd < 0 || fd >= MAX_FILES ||
        !open_file_table[fd].used ||
        strcmp(open_file_table[fd].username, user_name) != 0) {
        result.out_msg.out_msg_val = strdup("Error: Invalid file descriptor or user mismatch.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    if (position < 0 || position > FILE_BLOCKS * BLOCK_SIZE) {
        result.out_msg.out_msg_val = strdup("Error: Invalid position.");
        result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;
        return &result;
    }

    open_file_table[fd].file_pointer = position;

    result.success = 0;
    result.out_msg.out_msg_val = strdup("Seek successful.");
    result.out_msg.out_msg_len = strlen(result.out_msg.out_msg_val) + 1;

    return &result;
}
