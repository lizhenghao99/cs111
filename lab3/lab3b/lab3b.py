# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934


import sys
import csv

rc = 0
total_blocks = 0
total_inodes = 0
block_size = 0
inode_size = 0
reserved_blocks = list(range(1, 3))
free_blocks = list()
allo_blocks = list()
reserved_inodes = list
free_inodes = list()
allo_inodes = list()
link_count = dict()
dir_link = dict()
dir_name = dict()
dir_inode = dict()
dir_parent = dict()

def readsuper(row):
	global total_blocks 
	total_blocks = int(row[1])
	global total_inodes
	total_inodes = int(row[2])
	global block_size
	block_size = int(row[3])
	global inode_size 
	inode_size = int(row[4])
	global reserved_inodes
	reserved_inodes = list(range(1, int(row[7])))

def readgroup(row):
	global reserved_blocks 
	inode_table_size = int(total_inodes * inode_size / block_size)
	if total_inodes * inode_size % block_size > 0:
		inode_table_size += 1
	reserved_blocks.append(int(row[5]))
	reserved_blocks.append(int(row[6]))
	reserved_blocks.extend(range(int(row[7]), int(row[7])+inode_table_size))
	

def readinode(row):
	global rc
	blocks = row[12:]
	allo_inodes.append(int(row[1]))
	link_count[int(row[1])] = int(row[6])
	
	for i, blocknum, in enumerate(blocks, start=0):
		num = int(blocknum)
		output = ['ERROR','BLOCK',blocknum,'IN INODE',row[1],'AT OFFSET',i]
		if num != 0:
			allo_blocks.append(num)
		#invalid blocks
		if num < 0 or num > total_blocks:
			output[0] = 'INVALID'
			if i == 12:
				output.insert(1, 'INDIRECT')
			if i == 13:
				output[6] = 256 + 12
				output.insert(1, 'DOUBLE INDIRECT')
			if i == 14:
				output[6] = 256*256 + 256 + 12
				output.insert(1, 'TRIPLE INDIRECT')
			print(*output)
			rc = 2
		#reserved blocks
		for reserved in reserved_blocks:
			if num == int(reserved):
				output[0] = 'RESERVED'
				if i == 12:
					output.insert(1, 'INDIRECT')
				if i == 13:
					output[6] = 256 + 12
					output.insert(1, 'DOUBLE INDIRECT')
				if i == 14:
					output[6] = 256*256 + 256 + 12
					output.insert(1, 'TRIPLE INDIRECT')
				print(*output)
				rc = 2

def readdirent(row):	
	global rc
	inode = int(row[3])
	if inode < 1 or inode > total_inodes:
		print('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(row[1],row[6],row[3]))
		rc = 2
	
	if row[6] == "'.'" and row[1] != row[3]:
		print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(row[1],row[6],row[3],row[1]))
		rc = 2

	if row[6] == "'..'" and row[1] == '2' and row[3] != '2':
		print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(row[1],row[6],row[3],row[1]))
		rc = 2



	if int(row[3]) not in dir_link:
		dir_link[int(row[3])] = 1
	else:
		dir_link[int(row[3])] += 1
	dir_name[int(row[1])+int(row[2])] = row[6]
	dir_inode[int(row[1])+int(row[2])] = int(row[3])
	dir_parent[int(row[1])+int(row[2])] = int(row[1])

def readindirect(row):
	global rc
	allo_blocks.append(int(row[5]))
	num = int(row[5])
	if num < 0 or num > total_blocks:	
		print('INVALID BLOCK {} IN INODE {} AT OFFSET {}'.format(row[5],row[1],row[3]))
		rc = 2


def readfreeblock(row):
	free_blocks.append(int(row[1]))

def readfreeinode(row):
	free_inodes.append(int(row[1]))

def checkblocks(input_file):
	global rc
	input_file.seek(0)
	reader = csv.reader(input_file, delimiter=',')
	current_blocks = free_blocks + allo_blocks + reserved_blocks
	for i in range(1, total_blocks):
		if i not in current_blocks:
			print('UNREFERENCED BLOCK {}'.format(i))
			rc = 2
	for j in allo_blocks:
		if j in free_blocks:
			print('ALLOCATED BLOCK {} ON FREELIST'.format(j))
			rc = 2
	for k in range(1, total_blocks):
		if allo_blocks.count(k) > 1:
			for row in reader:
				if row[0] == 'INODE':
					blocks = row[12:]
					for i, blocknum, in enumerate(blocks, start=0):
						num = int(blocknum)
						output = ['ERROR','BLOCK',blocknum,'IN INODE',row[1],'AT OFFSET',i]
						if num == k:
							output[0] = 'DUPLICATE'
							if i == 12:
								output.insert(1, 'INDIRECT')
							if i == 13:
								output[6] = 256 + 12
								output.insert(1, 'DOUBLE INDIRECT')
							if i == 14:
								output[6] = 256*256 + 256 + 12
								output.insert(1, 'TRIPLE INDIRECT')
							print(*output)
							rc = 2
				if row[0] == 'INDIRECT':
					num = int(row[5])
					if num == k:	
						print('DUPLICATE BLOCK {} IN INODE {} AT OFFSET {}'.format(row[5],row[1],row[3]))
						rc = 2

def checkinodes():
	global rc
	current_inodes = free_inodes + allo_inodes + reserved_inodes
	for i in range(1, total_inodes):
		if i not in current_inodes:
			print('UNALLOCATED INODE {} NOT ON FREELIST'.format(i))
			rc = 2
	for j in allo_inodes:
		if j in free_inodes:
			print('ALLOCATED INODE {} ON FREELIST'.format(j))
			rc = 2
				
def checkdirs():
	global rc
	current_inodes = free_inodes + allo_inodes + reserved_inodes
	for i in allo_inodes:
		if i not in dir_link:
			print('INODE {} HAS 0 LINKS BUT LINKCOUNT IS {}'.format(i,link_count[i]))
			rc = 2
		elif link_count[i] != dir_link[i]:
			print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(i,dir_link[i],link_count[i]))
			rc = 2
	
	for j in dir_name:
		if dir_inode[j] not in allo_inodes:
			if (dir_inode[j] > 1 and dir_inode[j] < total_inodes):
				print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(dir_parent[j],dir_name[j],dir_inode[j]))
				rc = 2

	for k in dir_name:
		if dir_name[k] != "'.'" and dir_name[k] != "'..'":
			if dir_inode[k] in dir_parent.values():
				for x in dir_parent:
					if dir_parent[x] == dir_inode[k] and dir_name[x] == "'..'":
						if dir_inode[x] != dir_parent[k]:
							print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dir_parent[x],dir_name[x],dir_inode[x],dir_parent[k]))
							rc = 2
			
		
			
		
def readcsv():
	try:
		input_file = open(sys.argv[1], mode='r')
	except OSError:
		print("Error: failure while opening input file", file=sys.stderr)
		exit(1)
	# iterate through csv
	reader = csv.reader(input_file, delimiter=',')
	for row in reader:
		if row[0] == 'SUPERBLOCK':
			readsuper(row)
		if row[0] == 'GROUP':
			readgroup(row)
		if row[0] == 'INODE':
			readinode(row)
		if row[0] == 'DIRENT':
			readdirent(row)
		if row[0] == 'INDIRECT':
			readindirect(row)
		if row[0] == 'BFREE':
			readfreeblock(row)
		if row[0] == 'IFREE':
			readfreeinode(row)
	checkblocks(input_file)
	checkinodes()
	checkdirs()

def main():
	if len(sys.argv) != 2:
		print("Error: invalid argument", file=sys.stderr)
		exit(1)
	
	readcsv()
	exit(rc)



if __name__ == '__main__':
	main()
