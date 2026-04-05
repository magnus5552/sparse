#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEFAULT_BLOCK_SIZE 4096

int main(int argc, char *argv[])
{
    int input_fd, output_fd;
    char *input_file = NULL;
    char *output_file;
    size_t block_size = DEFAULT_BLOCK_SIZE;
    char *block;
    ssize_t bytes_read;
    off_t total_written = 0;
    off_t hole_start = -1;
    off_t hole_size = 0;
    off_t data_start = -1;
    off_t data_size = 0;

    if (argc == 2)
    {
        output_file = argv[1];
        input_fd = STDIN_FILENO;
    }
    else if (argc == 3)
    {
        input_file = argv[1];
        output_file = argv[2];
        input_fd = open(input_file, O_RDONLY);
        if (input_fd < 0)
        {
            perror("Error opening input file");
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        fprintf(stderr, "   or: %s <input_file> <output_file>\n", argv[0]);
        exit(1);
    }

    char *bs_env = getenv("BLOCK_SIZE");
    if (bs_env != NULL)
    {
        block_size = atoi(bs_env);
        if (block_size <= 0)
        {
            block_size = DEFAULT_BLOCK_SIZE;
        }
    }

    block = malloc(block_size);
    if (block == NULL)
    {
        perror("Error allocating memory");
        exit(1);
    }

    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0)
    {
        perror("Error opening output file");
        free(block);
        if (input_file != NULL)
            close(input_fd);
        exit(1);
    }

    while ((bytes_read = read(input_fd, block, block_size)) > 0)
    {
        for (ssize_t i = 0; i < bytes_read; i++)
        {
            if (block[i] == 0)
            {
                if (data_size > 0)
                {
                    if (write(output_fd, block + data_start, data_size) < 0)
                    {
                        perror("Error writing to output file");
                        free(block);
                        close(output_fd);
                        if (input_file != NULL)
                            close(input_fd);
                        exit(1);
                    }
                    data_start = -1;
                    data_size = 0;
                }

                if (hole_start == -1)
                {
                    hole_start = total_written;
                }
                hole_size++;
            }
            else
            {
                if (hole_size > 0)
                {
                    if (lseek(output_fd, hole_start + hole_size, SEEK_SET) < 0)
                    {
                        perror("Error seeking in output file");
                        free(block);
                        close(output_fd);
                        if (input_file != NULL)
                            close(input_fd);
                        exit(1);
                    }
                    hole_start = -1;
                    hole_size = 0;
                }

                if (data_start == -1)
                {
                    data_start = i;
                }
                data_size++;
            }
            total_written++;
        }
        
        if (data_size > 0)
        {
            if (write(output_fd, block + data_start, data_size) < 0)
            {
                perror("Error writing to output file");
                free(block);
                close(output_fd);
                if (input_file != NULL)
                    close(input_fd);
                exit(1);
            }
            data_start = -1;
            data_size = 0;
        }       
    }

    if (bytes_read < 0)
    {
        perror("Error reading input");
        free(block);
        close(output_fd);
        if (input_file != NULL)
            close(input_fd);
        exit(1);
    }

    if (hole_size > 0)
    {
        if (ftruncate(output_fd, hole_start + hole_size) < 0)
        {
            perror("Error setting final file size");
        }
    }

    free(block);
    close(output_fd);
    if (input_file != NULL)
        close(input_fd);

    return 0;
}