/*
 * NAME: 	Zhenghao Li
 * EMAIL:	lizhenghao99@g.ucla.edu
 * ID:		704971934
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<errno.h>

#include "ext2_fs.h"

#define SUPER_OFFSET 1024

struct ext2_super_block super_block;
struct ext2_group_desc *group;
struct ext2_inode inode;
struct ext2_dir_entry dir;
int fd;
int rc;

void format_time(unsigned input, char *output)
{
	char result[32];
	time_t time = input;
	struct tm* better_time = gmtime(&time);
	strftime(result, 32, "%m/%d/%y %H:%M:%S", better_time);
	strcpy(output, result);
}

void print_super_block()
{
    rc = pread(fd, &super_block, sizeof(struct ext2_super_block), SUPER_OFFSET);
    if (rc == -1)
    {
        perror("pread");
        exit(2);
    }
	printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", 
			super_block.s_blocks_count, 		
			super_block.s_inodes_count, 
			EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size,
			super_block.s_inode_size,
			super_block.s_blocks_per_group, 
			super_block.s_inodes_per_group, 
			super_block.s_first_ino);
}

void print_dirent(struct ext2_inode *inode, int BLOCK_SIZE, int j)
{
	if (inode->i_mode & 0x4000)
	{
        for (int k = 0; k < EXT2_NDIR_BLOCKS; k++)
        {
        	if (inode->i_block[k] == 0)
                continue;

            int dir_offset = 0;
            while (dir_offset < BLOCK_SIZE)
            {
                rc = pread(fd, &dir, sizeof(struct ext2_dir_entry),
                            inode->i_block[k]*BLOCK_SIZE + dir_offset);
                if (rc == -1)
                {
                   perror("pread");
                   exit(2);
                }
                if (dir.rec_len == 0)
	                break;
                if (dir.inode != 0)
                {
                    printf("DIRENT,%d,%u,%u,%d,%d,'%s'\n",
                            j+1,
                            k*BLOCK_SIZE + dir_offset,
                            dir.inode,
                            dir.rec_len,
                            dir.name_len,
                            dir.name);
                }
                dir_offset += dir.rec_len;
            }
        }
	}
}

void print_indirect(int ptr, int BLOCK_SIZE, int j, int rec, int rec_offset, 
					struct ext2_inode *inode)
{
	if (rec == 1)
		rec_offset += 12;
	if (rec == 2)
		rec_offset += 256 + 12;
    if (rec == 3)
		rec_offset += 65536 + 256 + 12;

	for (int offset = 0; offset < BLOCK_SIZE; offset += sizeof(unsigned))
	{
		unsigned block;
		rc = pread(fd, &block, sizeof(unsigned), 
								ptr*BLOCK_SIZE + offset);
		if (rc == -1)
        {
        	perror("pread");
        	exit(2);
        }

		if (block == 0)
			continue;

		unsigned logic_offset;
		logic_offset = rec_offset + offset/sizeof(unsigned);	
		
		printf("INDIRECT,%d,%d,%u,%u,%u\n", 
					j+1, 
					rec, 
					logic_offset,
					ptr, 
					block);
		
		if (rec == 1 && inode->i_mode & 0x4000)
		{
			int dir_offset = 0;
			while (dir_offset < BLOCK_SIZE)
            {
                rc = pread(fd, &dir, sizeof(struct ext2_dir_entry),
                            block*BLOCK_SIZE + dir_offset);
                if (rc == -1)
                {
                   perror("pread");
                   exit(2);
                }
                if (dir.rec_len == 0)
                    break;
                if (dir.inode != 0)
                {
                    printf("DIRENT,%d,%u,%u,%d,%d,'%s'\n",
                            j+1,
                            logic_offset*BLOCK_SIZE + dir_offset,
                            dir.inode,
                            dir.rec_len,
                            dir.name_len,
                            dir.name);
                }
                dir_offset += dir.rec_len;
            }
		}




		
		if (rec == 3)
			rec_offset -= 256 + 12;
		if (rec == 2)
			rec_offset -= 12;
		if (rec != 1)
			print_indirect(block, BLOCK_SIZE, j, rec-1, rec_offset, inode);
	}
}


void print_groups()
{
	int num_groups = super_block.s_blocks_count/super_block.s_blocks_per_group
						+1;
	int res_blocks = super_block.s_blocks_count%super_block.s_blocks_per_group;
	int res_inodes = super_block.s_inodes_count%super_block.s_inodes_per_group;
		
	int num_blocks = super_block.s_blocks_per_group;
    int num_inodes = super_block.s_inodes_per_group;

	int BLOCK_SIZE = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
		
	group = malloc(sizeof(struct ext2_group_desc)*num_groups);
	if (group == NULL)
	{
		perror("malloc");
		exit(2);
	}

	for (int i = 0; i < num_groups; i++)
	{
		int group_offset = SUPER_OFFSET + BLOCK_SIZE 
			+ i * super_block.s_blocks_per_group * BLOCK_SIZE;
		rc = pread(fd, &group[i], sizeof(struct ext2_group_desc), group_offset);
		if (rc == -1)
    	{
        	perror("pread");
        	exit(2);
    	}

	
		if ( i == num_groups - 1)
		{
			if (res_blocks != 0)
				num_blocks = res_blocks;
			if (res_inodes != 0)
				num_inodes = res_inodes;
		}
		
		/* GROUP */
		printf("GROUP,%d,%d,%d,%d,%d,%u,%u,%u\n",
				i, num_blocks, num_inodes,
				group[i].bg_free_blocks_count,
				group[i].bg_free_inodes_count, 
				group[i].bg_block_bitmap, 
				group[i].bg_inode_bitmap, 
				group[i].bg_inode_table);
		

		/* BFREE and IFREE */
		unsigned char bbuf;
		unsigned char ibuf;	
		for (int j = 0; j < num_blocks/8 + 1; j++)
		{
			rc = pread(fd, &bbuf, 1, group[i].bg_block_bitmap*BLOCK_SIZE + j);
			if (rc == -1)
        	{
            	perror("pread");
            	exit(2);
       	 	}
			for (int k = 0; k < 8; k++)
			{
				unsigned char test = 1 << k;
				if ((bbuf & test) == 0)
					printf("BFREE,%d\n", j*8 + k + 1);
			}
		}
		for (int j = 0; j < num_inodes/8 + 1; j++)
        {
            rc = pread(fd, &ibuf, 1, group[i].bg_inode_bitmap*BLOCK_SIZE + j);
            if (rc == -1)
            {
                perror("pread");
                exit(2);
            }
            for (int k = 0; k < 8; k++)
            {
                unsigned char test = 1 << k;
                if ((ibuf & test) == 0)
                    printf("IFREE,%d\n", j*8 + k + 1);
            }
        }

		
		/* INODE */
		for (unsigned j = 0; j < super_block.s_inodes_per_group; j++)
		{
			rc = pread(fd, &inode, super_block.s_inode_size, 
			  		group[i].bg_inode_table*BLOCK_SIZE + 
					j*super_block.s_inode_size);
            if (rc == -1)
            {
                perror("pread");
                exit(2);
            }
			
			if (inode.i_mode != 0 && inode.i_links_count != 0)
			{
				char type;
				if (inode.i_mode & 0x8000)
					type = 'f';
				else if (inode.i_mode & 0x4000)
					type = 'd';
				else if (inode.i_mode & 0xA000)
					type = 's';
				else
					type = '?';
				
				char ctime[32];
				format_time(inode.i_ctime, ctime);
				char mtime[32];
				format_time(inode.i_mtime, mtime);
				char atime[32];
				format_time(inode.i_atime, atime);
				printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%u,%u", 
						j+1, type, 
						inode.i_mode & 0xFFF, 
						inode.i_uid, inode.i_gid, 
						inode.i_links_count, 
						ctime, mtime, atime, 
						inode.i_size,
						inode.i_blocks);
				if (type == 'f' || type == 'd' ||
						(type == 's' && inode.i_size > 60) )
				{
					for(int k = 0 ; k < EXT2_N_BLOCKS; k++)
					{
						printf(",%u", inode.i_block[k]);
					}
					printf("\n");
				}
				else
					printf("\n");
			}

			print_dirent(&inode, BLOCK_SIZE, j);
			if (inode.i_mode & 0x4000 || inode.i_mode & 0x8000)
   			{	unsigned inblock = inode.i_block[EXT2_IND_BLOCK];
				unsigned dinblock = inode.i_block[EXT2_DIND_BLOCK];
				unsigned tinblock = inode.i_block[EXT2_TIND_BLOCK];
				if (inblock != 0)
					print_indirect(inblock, BLOCK_SIZE, j, 1, 0, &inode);
				if (dinblock != 0)
					print_indirect(dinblock, BLOCK_SIZE, j, 2, 0, &inode);
				if (tinblock != 0)
					print_indirect(tinblock, BLOCK_SIZE, j, 3, 0, &inode);
			}
		}
	}
}


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Invalid Arugment\n");
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }	

	print_super_block();
	print_groups();

	free(group);	
	return 0;
}
