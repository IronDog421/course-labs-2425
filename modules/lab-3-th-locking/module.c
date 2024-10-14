/*
#include <linux/kthread.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

volatile int64_t shared_variable = 0;
volatile int vl = 10;
uint64_t iter = (1<<20);


static int add_thread_fn(void *data){
        uint64_t i;
        for(i=0;i<iter;i++){
                shared_variable += vl;

        }
        pr_info("[ADD] finished: %lld\n", shared_variable);
        return 0;
}

static int subtrack_thread_fn(void *data){
    uint64_t i;
    for(i=0; i<iter; i++){
        shared_variable -= vl;
    }
    pr_info("[SUB] finished: %lld\n", shared_variable);
    return 0;
}

void broken_share(void){
        kthread_run(add_thread_fn, NULL, "add_thread");
        kthread_run(subtrack_thread_fn, NULL, "sub_thread");
}

static int __init my_module_init(void){
        broken_share();
        pr_info("Module loaded and threads started\n");
        return 0;
}

static void __exit my_module_exit(void){pr_info("Module unloaded");}
module_init(my_module_init);
module_exit(my_module_exit);
*/

#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/atomic.h>

MODULE_LICENSE("GPL");

atomic64_t shared_variable;
volatile int vl = 10;
uint64_t iter = (1<<20);

static int add_thread_fn(void *data){
    uint64_t i;
    for(i = 0; i < iter; i++){
        atomic64_add(vl, &shared_variable);
    }
    pr_info("[ADD] finished: %lld\n", atomic64_read(&shared_variable));
    return 0;
}

static int subtrack_thread_fn(void *data){
    uint64_t i;
    for(i = 0; i < iter; i++){
        atomic64_sub(vl, &shared_variable);
    }
    pr_info("[SUB] finished: %lld\n", atomic64_read(&shared_variable));
    return 0;
}

void broken_share(void){
    kthread_run(add_thread_fn, NULL, "add_thread");
    kthread_run(subtrack_thread_fn, NULL, "sub_thread");
}

static int __init my_module_init(void){
    atomic64_set(&shared_variable, 0);  // Inicializa la variable atómica
    broken_share();
    pr_info("Module loaded and threads started\n");
    return 0;
}

static void __exit my_module_exit(void){
    pr_info("Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

