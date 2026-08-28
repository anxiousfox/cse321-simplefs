#include "simplefs.h"

void set_bit(unsigned char *bitmap, int index) { bitmap[index / 8] |= (1u << (index % 8)); }
int is_bit_set(unsigned char *bitmap, int index) { return bitmap[index / 8] & (1u << (index % 8)); }
long inode_offset(int inode_number) { return ((long)INODE_TABLE_BLOCK * BLOCK_SIZE) + ((long)(inode_number - 1) * sizeof(inode_t)); }
int find_free_inode(unsigned char *bitmap) { (void)bitmap; return -1; }
int find_free_data_block(unsigned char *bitmap) { (void)bitmap; return -1; }

int main(int argc, char *argv[])
{
    char *image_name = NULL;
    FILE *fp;
    unsigned char zero_block[BLOCK_SIZE] = {0};
    unsigned char inode_bitmap[BLOCK_SIZE] = {0};
    unsigned char data_bitmap[BLOCK_SIZE] = {0};
    superblock_t sb;
    inode_t root_inode;
    dirent_t dot, dotdot;

    if (argc != 3) { printf("Usage: %s --image <image_name>\n", argv[0]); return 1; }
    if (strcmp(argv[1], "--image") != 0) { printf("Error: expected --image option.\n"); return 1; }
    image_name = argv[2];

    fp = fopen(image_name, "wb+");
    if (!fp) { printf("Error: could not create image file.\n"); return 1; }

    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        if (fwrite(zero_block, BLOCK_SIZE, 1, fp) != 1) { printf("Error: could not initialize image.\n"); fclose(fp); return 1; }
    }

    /* TODO 1: Fill all superblock fields. */
    memset(&sb, 0, sizeof(sb));
    /* CODE EDITING START'S HERE */
    sb.magic = MAGIC_NUMBER;                    // 0x53465331
    sb.block_size = BLOCK_SIZE;                // 4096
    sb.total_blocks = TOTAL_BLOCKS;            // 64
    sb.inode_count = TOTAL_INODES;             // 32
    sb.inode_bitmap_block = INODE_BITMAP_BLOCK;// 1
    sb.data_bitmap_block = DATA_BITMAP_BLOCK;  // 2
    sb.inode_table_block = INODE_TABLE_BLOCK;  // 3
    sb.data_region_block = DATA_REGION_BLOCK;  // 4
    sb.root_inode = ROOT_INODE;                // 1
    /* TODO END */
    fseek(fp, SUPERBLOCK_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(&sb, sizeof(sb), 1, fp);

    /* TODO 2: Mark inode 1 allocated (inode bitmap index 0). */
     /* CODE EDITING START'S HERE */
    set_bit(inode_bitmap, ROOT_INODE - 1);     // Index 0 represents Inode 1
    /* TODO END */
    fseek(fp, INODE_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_bitmap, BLOCK_SIZE, 1, fp);

    /* TODO 3: Mark root data block allocated (data bitmap index 0). */
     /* CODE EDITING START'S HERE */
    set_bit(data_bitmap, 0);                  // Index 0 represents Block 4 (DATA_REGION_BLOCK)
    /* TODO END */
    fseek(fp, DATA_BITMAP_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(data_bitmap, BLOCK_SIZE, 1, fp);

    /* TODO 4: Initialize root inode according to the specification. */
    memset(&root_inode, 0, sizeof(root_inode));
     /* CODE EDITING START'S HERE */
    root_inode.type = TYPE_DIRECTORY;          // 2
    root_inode.links = 2;                     // '.' and '..'
    root_inode.size = 2 * DIRENT_SIZE;         // 128 bytes (2 x 64-byte directory entries)
    root_inode.direct[0] = ROOT_DATA_BLOCK;    // Block 4
    root_inode.direct[1] = 0;                  // Unused
    root_inode.direct[2] = 0;                  // Unused
    /* TODO END */
    fseek(fp, inode_offset(ROOT_INODE), SEEK_SET);
    fwrite(&root_inode, sizeof(root_inode), 1, fp);

    /* TODO 5: Initialize the '.' entry. */
    memset(&dot, 0, sizeof(dot));
     /* CODE EDITING START'S HERE */
    dot.inode_no = ROOT_INODE;                 // 1
    dot.type = TYPE_DIRECTORY;                 // 2
    strncpy(dot.name, ".", sizeof(dot.name) - 1);
    /* TODO END */

    /* TODO 6: Initialize the '..' entry. */
    memset(&dotdot, 0, sizeof(dotdot));
     /* CODE EDITING START'S HERE */
    dotdot.inode_no = ROOT_INODE;              // 1 (root directory's parent is itself)
    dotdot.type = TYPE_DIRECTORY;              // 2
    strncpy(dotdot.name, "..", sizeof(dotdot.name) - 1);
    /* TODO END */

    fseek(fp, ROOT_DATA_BLOCK * BLOCK_SIZE, SEEK_SET);
    fwrite(&dot, sizeof(dot), 1, fp);
    fwrite(&dotdot, sizeof(dotdot), 1, fp);

    fclose(fp);
    printf("SimpleFS image created successfully: %s\n", image_name);
    return 0;
}
