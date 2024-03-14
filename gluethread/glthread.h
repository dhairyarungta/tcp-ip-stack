#ifndef __GLTHREAD__
#define __GLTHREAD__

typedef struct _glthread{
    struct _glthread* left;
    struct _glthread* right;
}glthread_t;

/*base_glthread referes to the dummy head of the list*/

void
init_glthread(glthread_t *glthread);

void 
glthread_add_next(glthread_t *curr_glthread, glthread_t *new_glthread);

void 
glthread_add_before(glthread_t *curr_glthread, glthread_t *new_glthread);

void 
remove_glthread(glthread_t *base_glthread);

void 
glthread_add_last(glthread_t *base_glthread, glthread_t *new_glthread);

void 
delete_glthread_list(glthread_t *base_glthread);

void 
glthread_priority_insert(glthread_t *base_glthread,
    glthread_t* glthread, int(*comp_fn)(void*, void*),int offset);


unsigned int 
get_glthread_list_count(glthread_t *base_glthread);


#define IS_GLTHREAD_LIST_EMPTY(glthreadptr)\
    (glthreadptr->right==NULL && glthreadptr->left ==NULL)

#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name)\
    static inline structure_name* fn_name(glthread_t *glthreadptr)\
    { return (structure_name*)((char*)(glthreadptr)-(char*)&(((structure_name*)0)->field_name));}

/*Macro to move to first node after dummy head node*/
#define BASE(glthreadptr)((glthreadptr)->right)

/*glthreadptrstart is a dummy head pointer*/
#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart,glthreadptr)\
    {   glthread_t *_glthread_ptr = NULL;\
        glthreadptr = BASE(glthreadptrstart);\
        for (;glthreadptr!=NULL;glthreadptr = _glthread_ptr)\
        {   _glthread_ptr = (glthreadptr)->right;\


#define ITERATE_GLTHREAD_END(glthreadptrstart,gltreadptr)}}

#define GLTHREAD_GET_USER_DATE_FROM_OFFSET(glthreadptr, offset)\
    (void*)((char*)(glthreadptr)-offset)

#endif