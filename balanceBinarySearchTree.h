#ifndef __BALANCE_BINARY_SEARCH_TREE_H_
#define __BALANCE_BINARY_SEARCH_TREE_H_
#include <stdio.h>

#define ELEMENTTYPE void *

/* 树的结点 (保留之前二叉搜索树结点结构体) */
typedef struct BSTreeNode
{
    ELEMENTTYPE val;
    struct BSTreeNode * left;
    struct BSTreeNode * right;
    struct BSTreeNode * parent;
} BSTreeNode;

/* 树的结点 */
typedef struct AVLTreeNode
{
    ELEMENTTYPE val;
    /* AVL树维护了一个高度的属性, 用于计算当前结点的平衡因子 */
    int height;     
    struct AVLTreeNode * left;
    struct AVLTreeNode * right;
    struct AVLTreeNode * parent;
} AVLTreeNode;

typedef struct BinarySearchTree
{
    int size;       /* 结点的个数 */
    int height;     /* 树的高度 */

    /* 钩子🪝函数放在树内部 */
    int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE);
    AVLTreeNode * root;
} BinarySearchTree;

/* 将二叉搜索树的结点 重命名为平衡二叉搜索树的结点 */
typedef BinarySearchTree BalanceBinarySearchTree;

/* 二叉搜索树初始化 */
int balanceBinarySearchTreeInit(BalanceBinarySearchTree **pBSTree, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE));

/* 二叉搜索树新增元素 */
int balanceBinarySearchTreeInsert(BalanceBinarySearchTree *pBSTree, ELEMENTTYPE val);

/* 二叉搜索树删除元素 */
int balanceBinarySearchTreeRemove(BalanceBinarySearchTree *pBSTree, ELEMENTTYPE val);

/* 二叉搜索树中是否包含指定元素 */
int balanceBinarySearchTreeIsContainVal(BalanceBinarySearchTree *pBSTree, ELEMENTTYPE val);

/* 获取二叉搜索树中指定元素的值*/
int balanceBinarySearchTreeIsContainValVal(BalanceBinarySearchTree *pBSTree, ELEMENTTYPE val, int *sockfd, int (*obtainFunc)(void *));

/* 二叉搜索树是否为空树 */
int balanceBinarySearchTreeIsNull(BalanceBinarySearchTree *pBSTree);

/* 二叉搜索树元素的个数 */
int balanceBinarySearchTreeGetSize(BalanceBinarySearchTree *pBSTree, int *pSize);

/* 前序遍历 */
int balanceBinarySearchTreePreOrderTravel(BalanceBinarySearchTree *pBSTree, int (*printFunc)(void *));

/* 中序遍历 */
int balanceBinarySearchTreeInOrderTravel(BalanceBinarySearchTree *pBSTree, int (*printFunc)(void *));

/* 后序遍历 */
int balanceBinarySearchTreePostOrderTravel(BalanceBinarySearchTree *pBSTree, int (*printFunc)(void *));

/* 层序遍历 */
int balanceBinarySearchTreeLevelOrderTravel(BalanceBinarySearchTree *pBSTree, int (*printFunc)(void *));

/* 获取树的高度 */
int balanceBinarySearchTreeGetHeight(BalanceBinarySearchTree *pBSTree, int *pHeight);

/* 二叉树的打印器 */
int balanceBinarySearchTreeFormatPrintOut(BalanceBinarySearchTree *pBSTree);

/* 判断树是否是完全二叉树 */
int balanceBinarySearchTreeIsComplete(BalanceBinarySearchTree *pBSTree);

/* 二叉搜索树的销毁 */
int balanceBinarySearchTreeDestroy(BalanceBinarySearchTree *pBSTree);

/* 二叉树的保存 */
int balanceBinarySearchTreeSave2File(BalanceBinarySearchTree *pBSTree, const char *pathname);
#endif  // __BINARY_SEARCH_TREE_H_