/*
 *
 *
 *
 *
 */

#include <iostream>
#include <vector>

using namespace std;

int n[9] = {1,2,3,4,5,6,7,8,9};
vector<int*> unused;

void print(int **np) 
{
    for (int i = 0; i < 3; i ++) {
        cout << "R" << i << ": ";
        for (int j = 2; j >= 0; j --) {
            int * p = np[3*i + j];
            if (p == NULL) {
                cout << "  ";
              continue;
            }
            cout << *(np[i*3 + j]) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void printv(vector<int *> v)
{
    int i = 0;
    for (vector<int *>::iterator it = unused.begin(); it < unused.end(); it ++, i ++) {
        cout << **it  << " ";
    }
    cout << endl;
}



int rule(int **np)
{
     /*
      * num[2] + num[1] - num[0] = 4;
      *  +        -        -
      * num[5] - num[4] * num[3] = 4;
      *  /        *        -
      * num[8] + num[7] - num[6] = 4;
      *  ||       ||       ||
      *  4        4        4
      */

    
    if (np[0] != NULL && np[1] != NULL & np[2] != NULL) {
        if (*np[2] + *np[1] - *np[0] != 4) {
            return -1;
        }
    }
    if (np[5] != NULL && np[4] != NULL & np[3] != NULL) {
        if (*np[5] - (*np[4]) * (*np[3]) != 4) {
            return -1;
        }
    }
    if (np[8] != NULL && np[7] != NULL & np[6] != NULL) {
        if (*np[8] + (*np[7]) - (*np[6]) != 4) {
            return -1;
        }
    }


    if (np[2] != NULL && np[5] != NULL & np[8] != NULL) {
        if (*np[2] + (*np[5]) / (*np[8]) != 4) {
            return -1;
        }
    }

    if (np[1] != NULL && np[4] != NULL & np[7] != NULL) {
        if (*np[1] - (*np[4]) * (*np[7]) != 4) {
            return -1;
        }
    }

    if (np[0] != NULL && np[3] != NULL & np[6] != NULL) {
        if (*np[0] - *np[3] - *np[6] != 4) {
            return -1;
        }
    }

    return 0;
}


int tree(int **np, int index, vector<int *> &unused)
{
    int ret = -1;
    if (unused.empty()) {
        cout << "unused queue is empty" << endl;
        return 0;
    }

    if (index >= 9) {
        cout << "too many number" << endl;
        return -1;
    }
    if (np == NULL) {
        cout << "np is invalid" << endl;
        return -1;
    }

    cout << "np index :" << index << ",======= unused vector ==== :";
    printv(unused);
    cout << "Current numbers:" << endl;
    print(np);
    if (np[index] == NULL) {
        for (vector<int *>::iterator it = unused.begin(); it < unused.end(); it ++) {
            np[index] = *it;
            if (rule(np) == 0) {
                unused.erase(it);
                ret = tree(np, index + 1, unused);
                if (ret == 0) {
                    break;
                }
                unused.insert(it, np[index]);
                np[index ] = NULL;
            }
        }
        if (ret < 0) {
            np[index] = NULL;
        }
    }

    return ret;
}



int main(void) {
    int *np[9] = {0};

    for (int i = 0; i < 9; i ++) {
        unused.push_back(&n[i]);
    }

    np[0] = unused.back();
    unused.pop_back();
    cout << "All unused numbers: " ;
    printv(unused);

    int ret = tree(&np[0], 1, unused);
    if (ret < 0) {
        cout << "Error: Fail to find numbers!" << endl;
    } else {
        cout << "Find numbers OK!" << endl;
    }
}
