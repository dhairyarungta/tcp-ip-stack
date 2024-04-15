#include "glthread.h"
#include <stdlib.h>

void
init_glthread(glthread_t *glthread){
    glthread->left = NULL;
    glthread->right = NULL;
}

void 
glthread_add_next(glthread_t *curr_glthread, glthread_t *new_glthread){
    if(curr_glthread->right==NULL){
        curr_glthread->right = new_glthread;
        new_glthread->left = curr_glthread;
        return;
    }
    new_glthread->right = curr_glthread->right;
    curr_glthread->right = new_glthread;
    new_glthread->left = curr_glthread;
    new_glthread->right->left = new_glthread;
    return;
}

void 
glthread_add_before(glthread_t *curr_glthread, glthread_t *new_glthread){
    if(curr_glthread->left == NULL){
        new_glthread->left = NULL;
        new_glthread->right = curr_glthread;
        curr_glthread->left = new_glthread;
        return;
    }
    glthread_t *temp = curr_glthread->left;
    temp->right = new_glthread;
    curr_glthread->left = new_glthread;
    new_glthread->right = curr_glthread;
    new_glthread->left = temp;
    return;
}

void 
remove_glthread(glthread_t *glthread){
    if(glthread->right == NULL){
        if(glthread->left !=NULL){
            glthread->left->right = NULL;
            glthread->left = NULL;
        }
         return;
    }

    if(glthread->left == NULL){
        glthread->right->left = NULL;
        glthread->right = NULL;
    }

    glthread->left->right = glthread->right;
    glthread->right->left = glthread->left;
    glthread->left = NULL;
    glthread->right = NULL;
    return;

}

void glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread){
    glthread_t *glthreadptr = NULL;
    glthread_t *prev_glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread,glthreadptr){
        prev_glthreadptr = glthreadptr;
    }ITERATE_GLTHREAD_END(base_glthread,glthreadptr);

    if(prev_glthreadptr)
        glthread_add_next(prev_glthreadptr,new_glthread);
    else
    glthread_add_next(base_glthread,new_glthread);

}

void 
delete_glthread_list(glthread_t *base_glthread){
    glthread_t *glthreadptr = NULL;
    ITERATE_GLTHREAD_BEGIN(base_glthread,glthreadptr){
        remove_glthread(glthreadptr);
    }ITERATE_GLTHREAD_END(base_glthread,glthreadptr)
}

unsigned int
get_glthread_list_count(glthread_t *base_glthread){
    unsigned int count = 0;
    glthread_t *glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread,glthreadptr){
        count++;
    }ITERATE_GLTHREAD_END(base_glthread,glthreadptr);
    return count;
}

void
glthread_priority_insert(glthread_t *base_glthread,
    glthread_t *glthread, int(*comp)(void*, void*),int offset){

        glthread_t *cur = NULL;
        glthread_t *prev = NULL;
        init_glthread(glthread);

        if(IS_GLTHREAD_LIST_EMPTY(base_glthread)){
            glthread_add_next(base_glthread,glthread);
            return;
        }

        prev = base_glthread;
        ITERATE_GLTHREAD_BEGIN(base_glthread,cur){
            if(comp(GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread,offset),
            GLTHREAD_GET_USER_DATA_FROM_OFFSET(cur,offset))==-1){
                glthread_add_next(prev, glthread);
                return;
            }
            prev = cur;
        }ITERATE_GLTHREAD_END(base_glthread,cur);
        glthread_add_next(prev,glthread);
    }
