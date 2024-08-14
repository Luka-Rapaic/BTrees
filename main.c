#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STEP 10


char* read_line(FILE *File) {
    if (File == NULL) {
        printf("ERROR: File not open!");
        exit(0);
    }

    char *line = NULL;
    int i = 0;
    while(1) {
        if (i % STEP == 0) {
            line = (char*)realloc(line, i + STEP);
            if (line == NULL) {
                printf("ERROR: Failed to allocate memory!");
                exit(0);
            }
        }

        char c = (char)fgetc(File);

        if (c == '\n' || c == EOF) {
            line[i++] = '\0';
            line = (char*)realloc(line, i);
            return line;
        }

        line[i++] = c;
    }
}

char** read_lines(char* FileName) {
    FILE *File = fopen(FileName, "r");
    if (File == NULL) {
        printf("ERROR: Document failed to open!");
        exit(0);
    }

    char **lines = NULL;
    int i = 0;
    while(1) {
        if (i % STEP == 0) {
            lines = realloc(lines, (i + STEP) * sizeof(char*));
            if (lines == NULL) {
                printf("ERROR: Failed to allocate memory!");
                exit(0);
            }
        }

        if (feof(File)) {
            lines[i++] = NULL;
            lines = realloc(lines, i * sizeof(char*));
            break;
        }

        char *line = read_line(File);
        lines[i++] = line;
    }

    fclose(File);
    return lines;
}

void free_lines(char** lines) {
    for (int i = 0; lines[i]; i++) {
        free(lines[i]);
    }
    free(lines);
}

void print_lines(char** lines) {
    for (int i = 0; lines[i]; i++) {
        printf("%s\n", lines[i]);
    }
}


typedef struct Customer {
    long long int C_ID;
    char C_F_NAME[30], C_L_NAME[30], C_EMAIL[30];
    long long int C_AD_ID;
} Customer;

Customer* line_to_customer(char* line) {
    if (line == NULL) return NULL;

    Customer *customer = (Customer*)malloc(sizeof(Customer));
    if (customer == NULL) {
        printf("ERROR: Failed to allocate memory!");
        exit(0);
    }

    sscanf(line, "%lld|%[^|]|%[^|]|%[^|]|%lld\n", &customer->C_ID, customer->C_F_NAME, customer->C_L_NAME, customer->C_EMAIL, &customer->C_AD_ID);
    return customer;
}

typedef struct CustomerAccount {
    long long int CA_ID, CA_B_ID, CA_C_ID;
    char CA_NAME[40];
    int CA_TAX_ST;
    float CA_BAL;
} CustomerAccount;

CustomerAccount* line_to_customerAccount(char* line) {
    if (line == NULL) return NULL;

    CustomerAccount *cAcc = (CustomerAccount*)malloc(sizeof(CustomerAccount));
    if (cAcc == NULL) {
        printf("ERROR: Failed to allocate memory!");
        exit(0);
    }

    sscanf(line, "%lld|%lld|%lld|%[^|]|%d|%f", &cAcc->CA_ID, &cAcc->CA_B_ID, &cAcc->CA_C_ID, cAcc->CA_NAME, &cAcc->CA_TAX_ST, &cAcc->CA_BAL);
    return cAcc;
}

void print_customerAccount(CustomerAccount ca) {
    printf("%lld|%lld|%lld|%s|%d|%f\n", ca.CA_ID, ca.CA_B_ID, ca.CA_C_ID, ca.CA_NAME, ca.CA_TAX_ST, ca.CA_BAL);
}

int compare_keys(CustomerAccount ca1, CustomerAccount ca2) {
    if (ca1.CA_C_ID < ca2.CA_C_ID) return -1;
    if (ca1.CA_C_ID > ca2.CA_C_ID) return 1;
    if (ca1.CA_ID < ca2.CA_ID) return -1;
    if (ca1.CA_ID > ca2.CA_ID) return 1;
    return 0;
}

typedef struct Node {
    int m, n, isLeaf;
    CustomerAccount *CustomerAccounts;
    struct Node **children;
    struct Node *parent;
} Node;

typedef struct BTree {
    int m;
    Node *root;
} BTree;

typedef struct ListNode {
    CustomerAccount data;
    struct ListNode *next;
} ListNode;

ListNode* create_listNode() {
    ListNode *newListNode = (ListNode*)malloc(sizeof(ListNode));
    newListNode->next = NULL;
    return newListNode;
}

typedef struct QNode {
    Node *data;
    struct QNode *next;
} QNode;

typedef struct Queue {
    QNode *front, *back;
} Queue;

QNode* create_qnode() {
    QNode *newQNode = (QNode*)malloc(sizeof(QNode));
    newQNode->data = NULL;
    newQNode->next = NULL;
    return newQNode;
}

void delete_qnode(QNode *q) {
    free(q);
}

int queue_empty(Queue *q) {
    return q->front == NULL;
}

void queue_insert(Queue *q, Node *data) {
    QNode *newNode = create_qnode();
    newNode->data = data;
    newNode->next = q->front;

    q->front = newNode;
    if (queue_empty(q)) {
        q->back = newNode;
    }
}


Node* queue_pop(Queue *q) {
    QNode *firstNode = q->front;
    Node *node = firstNode->data;

    q->front = firstNode->next;
    if (!firstNode->next) q->back = NULL;

    delete_qnode(firstNode);
    return node;
}

Queue* create_queue() {
    Queue *newQueue = (Queue*)malloc(sizeof(Queue));
    newQueue->front = NULL;
    newQueue->back = NULL;
    return newQueue;
}

Node* create_node(int m, int isLeaf) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("ERROR: Failed to allocate memory!");
        exit(0);
    }
    newNode->m = m;
    newNode->n = 0;
    newNode->isLeaf = isLeaf;

    newNode->CustomerAccounts = (CustomerAccount *)malloc((m-1) * sizeof(CustomerAccount));

    if (!isLeaf) newNode->children = (Node**)malloc(m * sizeof(Node*));
    newNode->parent = NULL;
    return newNode;
}

void delete_node(Node *node) {
    free(node->CustomerAccounts);
    free(node->children);
    free(node);
}

void leaf_split(Node* leaf, CustomerAccount ca, Node** argChildLeft, Node** argChildRight, CustomerAccount* argCa) {
    CustomerAccount sorted[leaf->m];

    for (int i = 0; i < leaf->n; i++) sorted[i] = leaf->CustomerAccounts[i];
    sorted[leaf->n] = ca;

    for (int i = 0; i < leaf->n; i++) if (compare_keys(ca, sorted[i]) == -1) {
        sorted[i] = ca;
        break;
    }

    Node* newLeaf = create_node(leaf->m, 1);
    newLeaf->parent = leaf->parent;

    int mid = (leaf->m+1)/2-1;

    for (int i = 0; i < mid; i++) leaf->CustomerAccounts[i] = sorted[i];
    leaf->n = mid;

    int j = 0;
    for (int i = mid+1; i < leaf->m; i++) newLeaf->CustomerAccounts[j++] = sorted[i];
    newLeaf->n = newLeaf->m - (mid+1);

//    printf("Leaf-Split:\n");
//    printf("Left-Child->n: %d\n", leaf->n);
//    printf("Right-Child->n: %d\n", newLeaf->n);
//    printf("Mid: %d\n", mid);

    *argChildLeft = leaf;
    *argChildRight = newLeaf;
    *argCa = sorted[mid];
}

void node_split(Node* node, Node* childRight, CustomerAccount ca, Node** argChildLeft, Node** argChildRight, CustomerAccount *argCa) {
    CustomerAccount sorted[node->m];
    Node *sortedChildren[node->m+1];

    for (int i = 0; i < node->n; i++) sorted[i] = node->CustomerAccounts[i];
    for (int i = 0; i < node->m; i++) sortedChildren[i] = node->children[i];
    sorted[node->n] = ca;
    sortedChildren[node->n+1] = childRight;

    for (int i = 0; i < node->n; i++) if (compare_keys(ca, sorted[i]) == -1) {
        for (int j = node->n; j > i; j--) sorted[j] = sorted[j-1];
        sorted[i] = ca;
        for (int j = node->m; j > i+1; j--) sortedChildren[j] = sortedChildren[j-1];
        sortedChildren[i+1] = childRight;
        break;
    }

    Node* newNode = create_node(node->m, 0);
    newNode->parent = node->parent;

    int mid = (node->m+1)/2-1;

    for (int i = 0; i < mid; i++) node->CustomerAccounts[i] = sorted[i];
    for (int i = 0; i <= mid; i++) node->children[i] = sortedChildren[i];
    node->n = mid;

    int j = 0;
    for (int i = mid+1; i < node->m; i++)  newNode->CustomerAccounts[j++] = sorted[i];
    j = 0;
    for (int i = mid+1; i <= node->m; i++) newNode->children[j++] = sortedChildren[i];
    newNode->n = newNode->m - (mid+1);

    *argChildLeft = node;
    *argChildRight = newNode;
    *argCa = sorted[mid];
}

void leaf_insert(Node *node, CustomerAccount ca) {
//    node->CustomerAccounts[node->n] = ca;
    int i = 0;
    for (; i < node->n; i++) if (compare_keys(ca, node->CustomerAccounts[i]) == -1) break;
    for (int j = node->n; j > i; j--) node->CustomerAccounts[j] = node->CustomerAccounts[j-1];
    node->CustomerAccounts[i] = ca;
    node->n++;
}

void node_insert(Node *node, CustomerAccount ca, Node *childRight) {
    int i = 0;
    for (; i < node->n; i++) if (compare_keys(ca, node->CustomerAccounts[i]) == -1) break;
    for (int j = node->n; j > i; j--) node->CustomerAccounts[j] = node->CustomerAccounts[j-1];
    for (int j = node->n+1; j > i+1; j--) node->children[j] = node->children[j-1];
    node->CustomerAccounts[i] = ca;
    node->children[i+1] = childRight;
    node->n++;
}

void tree_insert(BTree *bTree, Node* node, CustomerAccount ca) {
    Node *childLeft = NULL, *childRight = NULL;
    while (1) {
        if (node == NULL) {
            node = create_node(bTree->m, 0);
            node->children[0] = childLeft;
            node->children[1] = childRight;
            childLeft->parent = node;
            childRight->parent = node;
            node->CustomerAccounts[0] = ca;
            node->n++;
            bTree->root = node;
            return;
        }

        if (node->n == node->m-1) {
            if (node->isLeaf) {
                leaf_split(node, ca, &childLeft, &childRight, &ca);
                node = node->parent;
            } else {
                node_split(node, childRight, ca, &childLeft, &childRight, &ca);
                node = node->parent;
            }
        } else {
            if (node->isLeaf) leaf_insert(node, ca);
            else node_insert(node, ca, childRight);
            return;
        }
    }
}

//INDEX DOBAR
void merge_leafs(Node* parent, Node* leftLeaf, Node* rightLeaf, int keyIndex) {
    leftLeaf->CustomerAccounts[leftLeaf->n++] = parent->CustomerAccounts[keyIndex];
    for (int i = 0; i < rightLeaf->n; i++) leftLeaf->CustomerAccounts[leftLeaf->n++] = rightLeaf->CustomerAccounts[i];
    delete_node(rightLeaf);

    for (int i = keyIndex; i < parent->n-1; i++) {
        parent->CustomerAccounts[i] = parent->CustomerAccounts[i+1];
        parent->children[i+1] = parent->children[i+2];
    }
}

void merge_nodes(Node* parent, Node* leftNode, Node* rightNode, int keyIndex) {
    leftNode->CustomerAccounts[leftNode->n++] = parent->CustomerAccounts[keyIndex];
    leftNode->children[leftNode->n] = rightNode->children[0];
    for (int i = 0; i < rightNode->n; i++) {
        leftNode->CustomerAccounts[leftNode->n++] = rightNode->CustomerAccounts[i];
        leftNode->children[leftNode->n] = rightNode->children[i+1];
    }
    delete_node(rightNode);

    for (int i = keyIndex; i < parent->n-1; i++) {
        parent->CustomerAccounts[i] = parent->CustomerAccounts[i+1];
        parent->children[i+1] = parent->children[i+2];
    }
}

//INDEX DOBAR
void leaf_delete(Node* leaf, int keyIndex) {
    for (int i = keyIndex; i < leaf->n-1; i++) {
        leaf->CustomerAccounts[i] = leaf->CustomerAccounts[i+1];
    }
    leaf->n--;
}

//INDEX DOBAR
int leaf_loan(Node* parent, Node* leaf, Node* leftBrother, Node* rightBrother, int nodeIndex) {
    if (rightBrother != NULL && rightBrother->n >= (leaf->m+1)/2 - 1) {
        leaf->CustomerAccounts[leaf->n++] = parent->CustomerAccounts[nodeIndex];
        parent->CustomerAccounts[nodeIndex] = rightBrother->CustomerAccounts[0];
        leaf_delete(rightBrother, 0);
        return 1;
    } else if (leftBrother != NULL && leftBrother->n >= (leaf->m+1)/2 - 1) {
        for (int i = leaf->n++; i > 0; i--) leaf->CustomerAccounts[i] = leaf->CustomerAccounts[i-1];
        leaf->CustomerAccounts[0] = parent->CustomerAccounts[nodeIndex-1];
        parent->CustomerAccounts[nodeIndex-1] = leftBrother->CustomerAccounts[leftBrother->n-1];
        leaf_delete(leftBrother, leftBrother->n-1);
        return 1;
    } else return 0;
}

//INDEX DOBAR
int node_loan(Node* parent, Node* node, Node* leftBrother, Node* rightBrother, int nodeIndex) {
    if (rightBrother != NULL && rightBrother->n >= (node->m+1)/2 - 1) {
        node->CustomerAccounts[node->n++] = parent->CustomerAccounts[nodeIndex];
        parent->CustomerAccounts[nodeIndex] = rightBrother->CustomerAccounts[0];
        node->children[node->n] = rightBrother->children[0];
        for (int i = 0; i < rightBrother->n; i++) rightBrother->children[i] = rightBrother->children[i+1];
        for (int i = 0; i < rightBrother->n-1; i++) rightBrother->CustomerAccounts[i] = rightBrother->CustomerAccounts[i+1];
        rightBrother->n--;
        return 1;
    } else if (leftBrother != NULL && leftBrother->n >= (node->m+1)/2 - 1) {
        for (int i = node->n++; i > 0; i--) node->CustomerAccounts[i] = node->CustomerAccounts[i-1];
        for (int i = node->n; i > 0; i--) node->children[i] = node->children[i-1];
        node->CustomerAccounts[0] = parent->CustomerAccounts[nodeIndex-1];
        parent->CustomerAccounts[nodeIndex-1] = leftBrother->CustomerAccounts[leftBrother->n-1];
        node->children[0] = leftBrother->children[leftBrother->n--];
        return 1;
    } else return 0;
}

void node_delete(BTree* bTree, Node* node, int keyIndex) {
    if (!node->isLeaf) {
        Node *donatorNode = node->children[keyIndex];
        while (!donatorNode->isLeaf) {
            donatorNode = donatorNode->children[donatorNode->n];
        }
        node->CustomerAccounts[keyIndex] = donatorNode->CustomerAccounts[donatorNode->n - 1];
        leaf_delete(donatorNode, donatorNode->n - 1);
        node = donatorNode;
    } else { //is leaf
        leaf_delete(node, keyIndex);
    }

    while (1) {
        if (node->n < (node->m + 1) / 2 - 1) {
            if (node == bTree->root) {
                if (node->n == 0) {
                    bTree->root = node->children[0];
                    delete_node(node);
                }
                return;
            }
            Node *rightBrother, *leftBrother, *parent = node->parent;

            int i = 0;
            for (; i < parent->n + 1; i++) if (parent->children[i] == node) break;
            rightBrother = i == parent->n ? NULL : parent->children[i + 1];
            leftBrother = i == 0 ? NULL : parent->children[i - 1];
            if (leaf_loan(parent, node, leftBrother, rightBrother, i)) {
                return;
            }

            if (leftBrother != NULL) merge_leafs(parent, leftBrother, node, i - 1);
            else merge_leafs(parent, node, rightBrother, i);

            node = parent;
        } else return;
    }
}

void jump_to_prev(Node** argNode, int* argIndex, int *argCnt) {
    Node *node = *argNode;
    int keyIndex = *argIndex;
    int cnt = 0;

    if (!node->isLeaf) {
        node = node->children[keyIndex];
        while (!node->isLeaf) {
            node = node->children[node->n];
            cnt++;
        }
        *argNode = node;
        *argIndex = node->n-1;
        *argCnt += cnt;
        return;
    } else {
        if (keyIndex != 0) {
            (*argIndex)--;
            return;
        } else {
            while (node->parent != NULL) {
                Node *parent = node->parent;
                int i = 0;
                for (; i < parent->n + 1; i++) {
                    if (parent->children[i] == node) break;
                    cnt++;
                }

                if (i != 0) {
                    *argNode = parent;
                    *argIndex = i-1;
                    *argCnt += cnt;
                    return;
                }

                node = parent;
            }

            *argNode = NULL;
            *argIndex = -1;
            *argCnt += cnt;
            return;
        }
    }
}

void jump_to_next(Node** argNode, int* argIndex, int *argCnt) {
    Node *node = *argNode;
    int keyIndex = *argIndex;
    int cnt = 0;

    if (!node->isLeaf) {
        node = node->children[keyIndex+1];
        while (!node->isLeaf) {
            node = node->children[0];
            cnt++;
        }
        *argNode = node;
        *argIndex = 0;
        *argCnt += cnt;
        return;
    } else {
        if (keyIndex != node->n) {
            (*argIndex)++;
            return;
        } else {
            while (node->parent != NULL) {
                Node *parent = node->parent;
                int i = 0;
                for (; i < parent->n + 1; i++) {
                    if (parent->children[i] == node) break;
                    cnt++;
                }

                if (i != 0) {
                    *argNode = parent;
                    *argIndex = i;
                    *argCnt += cnt;
                    return;
                }

                node = parent;
            }

            *argNode = NULL;
            *argIndex = -1;
            *argCnt += cnt;
            return;
        }
    }
}

void btree_search(BTree *btree, CustomerAccount ca, Node **argNode, int *argIndex) {
    Node *node = btree->root;
    outsideLoop: while (!node->isLeaf) {
        for(int i = 0; i < node->n; i++) {
            if (compare_keys(ca, node->CustomerAccounts[i]) == 0 || node->CustomerAccounts[i].CA_ID == ca.CA_ID) {
                *argNode = node;
                *argIndex = i;
                return;
            } else if (compare_keys(ca, node->CustomerAccounts[i]) == -1) {
                node = node->children[i];
                goto outsideLoop;
            }
        }
        node = node->children[node->n];
    }
    for (int i = 0; i < node->n; i++) {
        if (compare_keys(ca, node->CustomerAccounts[i]) == 0) {
            *argNode = node;
            *argIndex = i;
            return;
        }
    }
    *argNode = node;
    *argIndex = -1;
}

Customer* find_customer(char **lines, long long int customerID) {
    for (char *line = *lines; line; line++) {
        Customer *customer = line_to_customer(line);
        if (customer->C_ID == customerID) {
            return customer;
        }
        free(customer);
    }
    return NULL;
}

void seach_by_customer(BTree *btree, long long int customerID, Node **argNode, int *argIndex, int *argCnt) {
    Node *node = btree->root; int cnt = 0;
    outsideLoop: while (!node->isLeaf) {
        for(int i = 0; i < node->n; i++) {
            if (customerID == node->CustomerAccounts[i].CA_C_ID) {
                *argNode = node;
                *argIndex = i;
                *argCnt += cnt;
                return;
            } else if (customerID < node->CustomerAccounts[i].CA_C_ID) {
                node = node->children[i];
                goto outsideLoop;
            }
            cnt += 2;
        }
        node = node->children[node->n];
    }
    for (int i = 0; i < node->n; i++) {
        if (customerID == node->CustomerAccounts[i].CA_C_ID) {
            *argNode = node;
            *argIndex = i;
            *argCnt += cnt;
            return;
        }
        cnt++;
    }
    *argCnt += cnt;
    *argNode = node;
    *argIndex = -1;
}

ListNode* find_all_customer(BTree *btree, long long int customerID) {
    Node *targetNode, *prevNode, *nextNode;
    int targetIndex, prevIndex, nextIndex;
    int cnt = 0;

    seach_by_customer(btree, customerID, &targetNode, &targetIndex, &cnt);

    prevNode = targetNode, nextNode = targetNode;
    prevIndex = targetIndex, nextIndex = targetIndex;

    ListNode *listNode = create_listNode();
    listNode->data = targetNode->CustomerAccounts[targetIndex];

    jump_to_prev(&prevNode, &prevIndex, &cnt);
    jump_to_next(&nextNode, &nextIndex, &cnt);

    while (prevNode != NULL && prevNode->CustomerAccounts[prevIndex].CA_C_ID == customerID) {
        ListNode *newListNode = create_listNode();
        newListNode->data = prevNode->CustomerAccounts[prevIndex];
        newListNode->next = listNode;
        listNode = newListNode;
        jump_to_prev(&prevNode, &prevIndex, &cnt);
    }
    while (nextNode != NULL && nextNode->CustomerAccounts[nextIndex].CA_C_ID == customerID) {
        ListNode *newListNode = create_listNode();
        newListNode->data = nextNode->CustomerAccounts[nextIndex];
        newListNode->next = listNode;
        listNode = newListNode;
        jump_to_next(&nextNode, &nextIndex, &cnt);
    }

    return listNode;
}

void kreiraj_datoteku(BTree *btree, char *fileName, long long int customerID) {
    char **lines = read_lines("Customer.txt");
    Customer *cst = find_customer(lines, customerID);
    ListNode *listNode = find_all_customer(btree, customerID);
    FILE *file = fopen(fileName, "ab+");

    while (listNode != NULL) {
        CustomerAccount ca = listNode->data;
        fprintf(file, "%lld|%s|%s|%s|%lld|%lld|%d|%f\n", cst->C_ID, cst->C_F_NAME, cst->C_L_NAME, cst->C_EMAIL, ca.CA_ID, ca.CA_C_ID, ca.CA_TAX_ST, ca.CA_BAL);

        ListNode *tmp = listNode;
        listNode = listNode->next;
        free(tmp);
    }
    fclose(file);
}

void delete_all_customer(BTree *btree, long long int customerID) {
    Node *targetNode;
    int targetIndex;
    int cnt = 0;

    while (1) {
        seach_by_customer(btree, customerID, &targetNode, &targetIndex, &cnt);
        if (targetIndex == -1) {
            printf("Ukupan broj koraka: %d\n\n", cnt);
            return;
        }
        print_customerAccount(targetNode->CustomerAccounts[targetIndex]);
        node_delete(btree, targetNode, targetIndex);
    }
}

void btree_print(BTree* btree) {
    Queue *q = create_queue();
    queue_insert(q, btree->root);
    int expected = 1;
    int expectedNext = 0;

    while (1) {
        Node *node = queue_pop(q);
        for (int i = 0; i < node->n; i++) {
            printf("%lld-%lld", node->CustomerAccounts[i].CA_C_ID, node->CustomerAccounts[i].CA_ID);
            printf("   ");
        }
        if (!node->isLeaf) for (int i = 0; i < node->n+1; i++) {
            queue_insert(q, node->children[i]);
            expectedNext++;
        }
        expected--;
        if (expected == 0) {
            printf("\n");
            if (expectedNext == 0) return;
            else {
                expected = expectedNext;
                expectedNext = 0;
            }
        }
    }
}

BTree* create_btree(int m) {
    BTree *newBTree = (BTree*)malloc(sizeof(BTree));
    if (newBTree == NULL) {
        printf("ERROR: Failed to allocate memory!");
        exit(0);
    }
    newBTree->m = m;
    newBTree->root = create_node(m, 1);
    return newBTree;
}

BTree* customerAccount_table_to_btree(char *fileName, int m) {
    char **lines = read_lines(fileName);

    BTree *btree = create_btree(m);
    int i = 0;
    while (lines[i] != NULL) {
        CustomerAccount *ca = line_to_customerAccount(lines[i]);
        Node *insertionNode;
        int insertionKey;
        btree_search(btree, *ca, &insertionNode, &insertionKey);
        tree_insert(btree, insertionNode, *ca);
        i++;
    }

    return btree;
}

void print_btree(BTree *bTree) {
    int max = 80;

    Queue *q = create_queue();
    queue_insert(q, bTree->root);

    int lvl = 0;
    int offset = (max - 1) / 2;
    int parity = 0;
    int InThisLvl = 1;
    int InNextLvl = 0;

    while (!queue_empty(q)) {
        Node *newNode = queue_pop(q);

        if (!InThisLvl) {
            printf("\n");

            offset = (max - (2 * bTree->m - 1) * InNextLvl) / 2;
            InThisLvl = InNextLvl;
            InNextLvl = 0;
            parity = 0;
        }

        for (int i = 0; i < offset; i++) {
            printf(" ");
        }
        if (parity) printf(" ");
        parity = !parity;

        printf("(");
        for (int i = 0; i < newNode->n; i++) {
            printf("%lld", newNode->CustomerAccounts[i].CA_ID%100);
            if (i != bTree->m-2) {
                printf("|");
            } else printf(")");
        }
        for (int i = newNode->n; i < bTree->m-1; i++) {
            printf("x");
            if (i != bTree->m - 2) {
                printf("|");
            } else printf(")");
        }

        if (!newNode->isLeaf) {
            for (int i = 0; i < newNode->n+1; i++) {
                queue_insert(q, newNode->children[i]);
                InNextLvl++;
            }
        }
        InThisLvl--;

//        membInLvl--;
    }
}

int main() {
    char fileName[100] = "CustomerAccount20.txt";
    int m = 0;
    BTree *btree = NULL;
    CustomerAccount ca;
    int keyIndex;
    Node *keyNode;
    long long int customerID;
    while (1) {
        printf("~~~~~Meni~~~~~\n");
        printf("Izaberite funkciju za izvrsavanje: \n");
        printf("1. Izaberi dokument za citanje\n");
        printf("2. Formiraj tabelu nad dokumentom\n");
        printf("3. Prikazi stablo\n");
        printf("4. Ispisi podatke o korisniku i njegovim racunima\n");
        printf("5. Unesi novi zapis u indeks\n");
        printf("6. Izbrisi zapis iz indeksa\n");
        printf("7. Obrisi sve racune korisnika\n");
        printf("8. Prekini program\n");

        int choice;
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1:
                printf("Unesite naziv dokumenta:\n");
                scanf("%s", fileName);
                printf("\n");
                break;
            case 2:
                printf("Unesite red stabla:\n");
                scanf("%d", &m);
                if (m < 3 || m > 10) {
                    printf("Niz stabla mora pripadati intervalu [3, 10]!\n\n");
                    continue;
                }
                printf("\n");
                btree = customerAccount_table_to_btree(fileName, m);
                break;
            case 3:
                if (btree == NULL) {
                    printf("Prvo inicijalizujte stablo!\n\n");
                    continue;
                }
                print_btree(btree);
                printf("\n\n");
                break;
            case 4:
                if (btree == NULL) {
                    printf("Prvo inicijalizujte stablo!\n\n");
                    continue;
                }
                printf("Unesite ID korisnika:\n");
                scanf("%lld", &customerID);
                kreiraj_datoteku(btree, "Ispis.txt", customerID);
                printf("\n");
                break;
            case 5:
                if (btree == NULL) {
                    printf("Prvo inicijalizujte stablo!\n\n");
                    continue;
                }
                printf("Unesite zapis:\n");
                if (scanf("%lld|%lld|%lld|%[^|]|%d|%f", &ca.CA_ID, &ca.CA_B_ID, &ca.CA_C_ID, ca.CA_NAME, &ca.CA_TAX_ST, &ca.CA_BAL) != 6) {
                    printf("Uneti zapis nije validan!\n\n");
                    continue;
                };
                printf("\n");
                btree_search(btree, ca, &keyNode, &keyIndex);
                if (keyIndex != -1) {
                    printf("Uneti zapis vec postoji unutar indeksa!\n\n");
                    continue;
                }
                tree_insert(btree, keyNode, ca);
                printf("Zapis je uspesno unet!\n\n");
                break;
            case 6:
                if (btree == NULL) {
                    printf("Prvo inicijalizujte stablo!\n\n");
                    continue;
                }
                printf("Unesite zapis:\n");
                if (scanf("%lld|%lld|%lld|%[^|]|%d|%f", &ca.CA_ID, &ca.CA_B_ID, &ca.CA_C_ID, ca.CA_NAME, &ca.CA_TAX_ST, &ca.CA_BAL) != 6) {
                    printf("Uneti zapis nije validan!\n\n");
                    continue;
                };
                printf("\n");
                btree_search(btree, ca, &keyNode, &keyIndex);
                if (keyIndex == -1) {
                    printf("Uneti zapis ne postoji u indeksu!\n\n");
                    continue;
                }
                node_delete(btree, keyNode, keyIndex);
                printf("Zapis je uspesno obrisan!\n\n");
                break;
            case 7:
                if (btree == NULL) {
                    printf("Prvo inicijalizujte stablo!\n\n");
                    continue;
                }
                printf("Unesite id korisnika:\n");
                scanf("%lld", &customerID);
                delete_all_customer(btree, customerID);
                break;
            case 8:
                exit(0);
            default:
                printf("Vas izbor nije validan!\n\n");
        }
    }

//    BTree *btree = customerAccount_table_to_btree("CustomerAccount20.txt", 10);
//    int cnt = 0;
//    btree_print(btree);
//    free_lines(lines);
    return 0;
}
