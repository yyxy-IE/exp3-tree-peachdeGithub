/**
  * @file    tree.c
  * @brief   目录树形查看器 完整功能版
  * @func    1.创建节点 2.构建目录树 3.树形输出 4.统计节点总数
  *          5.统计叶子节点数 6.计算树高度 7.统计目录文件数
  *          8.释放整棵树 9.获取当前目录名
  */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <dirent.h>
 #include <sys/stat.h>
 #include <errno.h>
 #define MAX_PATH_LEN  1024
 #define MAX_PREFIX_LEN 512
 // 树节点结构：左孩子右兄弟
 typedef struct FileTreeNode
 {
     char fileName[MAX_PATH_LEN];
     int isDirectory;
     struct FileTreeNode *child;
     struct FileTreeNode *next;
 } FileTreeNode;
 // 1. 创建节点
 FileTreeNode* CreateFileNode(const char *name, int isDir)
 {
     FileTreeNode *node = (FileTreeNode *)malloc(sizeof(FileTreeNode));
     if (node == NULL)
     {
         perror("malloc error");
         exit(EXIT_FAILURE);
     }
     memset(node, 0, sizeof(FileTreeNode));
     strncpy(node->fileName, name, MAX_PATH_LEN - 1);
     node->isDirectory = isDir;
     node->child = NULL;
     node->next = NULL;
     return node;
 }
 // 节点名称比较
 int StringCompare(const char *str1, const char *str2)
 {
     return strcmp(str1, str2);
 }
 // 同级节点冒泡排序
 void SortFileTreeBrother(FileTreeNode *head)
 {
     if (head == NULL || head->next == NULL)
         return;
     FileTreeNode *p, *q;
     char tempName[MAX_PATH_LEN];
     int tempDirFlag;
     for (p = head; p != NULL; p = p->next)
     {
         for (q = p->next; q != NULL; q = q->next)
         {
             if (StringCompare(p->fileName, q->fileName) > 0)
             {
                 strcpy(tempName, p->fileName);
                 tempDirFlag = p->isDirectory;
                 strcpy(p->fileName, q->fileName);
                 p->isDirectory = q->isDirectory;
                 strcpy(q->fileName, tempName);
                 q->isDirectory = tempDirFlag;
             }
         }
     }
 }
 // 2. 递归构建目录树
 FileTreeNode* BuildDirectoryTree(const char *path)
 {
     DIR *dirStream = opendir(path);
     if (dirStream == NULL)
     {
         fprintf(stderr, "无法打开目录: %s\n", path);
         return NULL;
     }
     struct dirent *dirEntry;
     FileTreeNode *brotherHead = NULL;
     FileTreeNode **pTail = &brotherHead;
     while ((dirEntry = readdir(dirStream)) != NULL)
     {
         if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
             continue;
         char fullPath[MAX_PATH_LEN] = {0};
         snprintf(fullPath, MAX_PATH_LEN, "%s/%s", path, dirEntry->d_name);
         struct stat fileStat;
         if (stat(fullPath, &fileStat) == -1)
             continue;
         FileTreeNode *newNode = NULL;
         if (S_ISDIR(fileStat.st_mode))
         {
             newNode = CreateFileNode(dirEntry->d_name, 1);
             newNode->child = BuildDirectoryTree(fullPath);
         }
         else if (S_ISREG(fileStat.st_mode))
         {
             newNode = CreateFileNode(dirEntry->d_name, 0);
         }
         if (newNode != NULL)
         {
             *pTail = newNode;
             pTail = &newNode->next;
         }
     }
     closedir(dirStream);
     SortFileTreeBrother(brotherHead);
     FileTreeNode *rootNode = CreateFileNode(path, 1);
     rootNode->child = brotherHead;
     return rootNode;
 }
 // 3. 树形输出
 void PrintFileTree(FileTreeNode *node, char *prefix, int isLast)
 {
     if (node == NULL)
         return;
     printf("%s", prefix);
     if (isLast)
         printf("└─ ");
     else
         printf("├─ ");
     printf("%s\n", node->fileName);
     char newPrefix[MAX_PREFIX_LEN] = {0};
     snprintf(newPrefix, MAX_PREFIX_LEN, "%s%s", prefix, isLast ? "    " : "│   ");
     FileTreeNode *child = node->child;
     while (child != NULL)
     {
         int lastFlag = (child->next == NULL) ? 1 : 0;
         PrintFileTree(child, newPrefix, lastFlag);
         child = child->next;
     }
 }
 // 4. 统计节点总数
 int CountTotalNode(FileTreeNode *root)
 {
     if (root == NULL)
         return 0;
     return 1 + CountTotalNode(root->child) + CountTotalNode(root->next);
 }
 // 5. 统计叶子节点数
 int CountLeafNode(FileTreeNode *root)
 {
     if (root == NULL)
         return 0;
     // 没有子节点即为叶子
     if (root->child == NULL)
         return 1 + CountLeafNode(root->next);
     return CountLeafNode(root->child) + CountLeafNode(root->next);
 }
 // 6. 计算树的高度
 int GetTreeHeight(FileTreeNode *root)
 {
     if (root == NULL)
         return 0;
     int childH = GetTreeHeight(root->child);
     int siblingH = GetTreeHeight(root->next);
     return 1 + (childH > siblingH ? childH : siblingH);
 }
 // 7. 统计目录和文件数量
 void CountDirAndFile(FileTreeNode *root, int *dirCount, int *fileCount)
 {
     if (root == NULL)
         return;
     if (root->isDirectory)
         (*dirCount)++;
     else
         (*fileCount)++;
     CountDirAndFile(root->child, dirCount, fileCount);
     CountDirAndFile(root->next, dirCount, fileCount);
 }
 // 8. 释放整棵树
 void FreeFileTree(FileTreeNode *root)
 {
     if (root == NULL)
         return;
     FreeFileTree(root->child);
     FreeFileTree(root->next);
     free(root);
 }
 // 获取当前目录名
 void GetBaseNameFromPath(const char *path, char *outName)
 {
     char *lastSlash = strrchr(path, '/');
     if (lastSlash != NULL)
         strcpy(outName, lastSlash + 1);
     else
         strcpy(outName, path);
 }
 int main(int argc, char *argv[])
 {
     char targetPath[MAX_PATH_LEN] = {0};
     if (argc > 1)
         strcpy(targetPath, argv[1]);
     else
         strcpy(targetPath, ".");
     struct stat testStat;
     if (stat(targetPath, &testStat) == -1)
     {
         printf("路径不存在！\n");
         return EXIT_FAILURE;
     }
     if (!S_ISDIR(testStat.st_mode))
     {
         printf("指定路径不是目录！\n");
         return EXIT_FAILURE;
     }
     FileTreeNode *treeRoot = BuildDirectoryTree(targetPath);
     if (treeRoot == NULL)
     {
         printf("目录树构建失败！\n");
         return EXIT_FAILURE;
     }
     char baseName[MAX_PATH_LEN];
     GetBaseNameFromPath(targetPath, baseName);
     printf("目录树形结构：%s/\n\n", baseName);
     FileTreeNode *firstChild = treeRoot->child;
     while (firstChild != NULL)
     {
         int last = (firstChild->next == NULL) ? 1 : 0;
         PrintFileTree(firstChild, "", last);
         firstChild = firstChild->next;
     }
     // 输出所有统计信息
     int total = 0, leaf = 0, height = 0;
     int dirNum = 0, fileNum = 0;
     total = CountTotalNode(treeRoot);
     leaf = CountLeafNode(treeRoot);
     height = GetTreeHeight(treeRoot);
     CountDirAndFile(treeRoot, &dirNum, &fileNum);
     printf("\n================ 统计信息 ================\n");
     printf("1. 节点总数：%d\n", total);
     printf("2. 叶子节点数：%d\n", leaf);
     printf("3. 树的高度：%d\n", height);
     printf("4. 目录数量：%d\n", dirNum - 1);
     printf("5. 文件数量：%d\n", fileNum);
     printf("==========================================\n");
     FreeFileTree(treeRoot);
     return 0;
 }