#ifndef EXCEPT_H
#define EXCEPT_H
#include <setjmp.h>
#include <assert.h>

typedef struct _Except_t {  
    char *reason;
} Except_t;

typedef struct Except_Frame Except_Frame;
struct Except_Frame {
    Except_Frame *prev;
    jmp_buf env;
    const char *file;
    int line;
    const Except_t *exception;
};

enum { Except_entered=0, Except_raised,
       Except_handled,   Except_finalized };
       
extern Except_Frame *Except_stack;

extern const Except_t Assert_Failed;

Except_Frame *Except_stack = NULL;

void Except_raise(const Except_t *e, const char *file,
    int line) {
    Except_Frame *p = Except_stack;
    assert(e);
    if (p == NULL) {
        fprintf(stderr, "Uncaught exception");
        if (e->reason)
            fprintf(stderr, " %s", e->reason);
        else
            fprintf(stderr, " at 0x%p", e);
        if (file && line > 0)
            fprintf(stderr, " raised at %s:%d\n", file, line);
        fprintf(stderr, "aborting...\n");
        fflush(stderr);
        abort();
    }
    p->exception = e;
    p->file = file;
    p->line = line;
    Except_stack = Except_stack->prev;
    longjmp(p->env, Except_raised);
}

void Except_raise(const Except_t *e, const char *file,int line);

#define RAISE(e) Except_raise(&(e), __FILE__, __LINE__)

#define RERAISE Except_raise(Except_frame.exception, \
    Except_frame.file, Except_frame.line)
    
#define RETURN switch (Except_stack = Except_stack->prev,0) default: return

#define TRY do { \
    volatile int Except_flag; \
    Except_Frame Except_frame; \
    Except_frame.prev = Except_stack; \
    Except_stack = &Except_frame;  \
    Except_flag = setjmp(Except_frame.env); \
    if (Except_flag == Except_entered) {
    
#define EXCEPT(e) \
        if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } else if (Except_frame.exception == &(e)) { \
        Except_flag = Except_handled;
        
#define ELSE \
        if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } else { \
        Except_flag = Except_handled;
        
#define FINALLY \
        if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } { \
        if (Except_flag == Except_entered) \
            Except_flag = Except_finalized;
            
#define END_TRY \
        if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
        } if (Except_flag == Except_raised) RERAISE; \
} while (0);

#endif
