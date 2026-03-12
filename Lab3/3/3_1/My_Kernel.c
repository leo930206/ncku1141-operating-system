/*
Step 1. proc_create(): Kernel 建立 /proc/Mythread_info 檔案窗口。

Step 2. Create threads: User 程式建立兩個執行緒。

Step 3. read(): User 呼叫 fgets() 讀取檔案，觸發 Kernel 的 Myread。

Step 4. print info: Kernel (Myread) 遍歷 task_struct 抓取 PID, TID, State 等資訊。

Step 5. copy_to_user: Kernel 把抓到的資料，安全複製給 User Buffer。

Step 6-7. Display: 資料回到 User 程式，printf 印出結果。
*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE 1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset) {
    /* Do nothing */
    return 0;
}

// 當 User 呼叫 fgets() 讀取 /proc/Mythread_info 時，Kernel 會執行這個函式。
static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset) {
    /*Your code here*/
    if (*offset > 0) {   // 開始位置應該是 0
        return 0;  // EOF
    }

    struct task_struct *thread;
    for_each_thread(current, thread) {      // 遍歷目前 process 的所有 threads
        if (current->pid == thread->pid) {  // skip main thread
            continue;
        }
        int len = sprintf(&buf[*offset], "PID: %d, TID: %d, Priority: %d, State: %d\n",   // 把抓到的資訊寫進 kernel buffer
                          current->pid, thread->pid, thread->prio, thread->__state);
        *offset += len;  // 每次讀取後，更新 offset
    }

    int fail = copy_to_user(ubuf, buf, *offset);    // 把 kernel buffer 的資料複製到 user buffer
    if (fail) return -EFAULT; 

    return *offset;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void) {
    proc_create(procfs_name, 0644, NULL, &Myops);
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void) {
    remove_proc_entry(procfs_name, NULL);
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
