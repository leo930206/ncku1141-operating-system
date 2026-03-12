/*
Step 1. proc_create(): Kernel 建立 /proc/Mythread_info 檔案窗口。

Step 2. Lock & Threads: User 建立執行緒並上鎖 (Spinlock)，確保一次只有一個人用 Kernel Buffer。

Step 3-4. Write (User -> Kernel): User 寫入 "Hello" (fwrite)，Kernel (Mywrite) 用 copy_from_user 收信。

Step 5. Kernel Processing: Kernel 在 "Hello" 後面加上 PID 和 Time。

Step 6-7. Read (Kernel -> User): User 讀取檔案 (fgets)，Kernel (Myread) 用 copy_to_user 把完整訊息回傳，並清空 Buffer。

Step 8-9. Unlock & Display: User 解鎖 (Unlock) 並印出結果。
*/


#include <asm/current.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#define procfs_name "Mythread_info"
#define BUFSIZE 1024
char buf[BUFSIZE];  // kernel buffer

// 當 User 呼叫 fwrite() 寫入 /proc/Mythread_info 時，Kernel 會執行這個函式。
static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset) {
    /*Your code here*/
    int cur_len = strlen(buf);  // 目前 kernel buffer 的長度

    int fail = copy_from_user(&buf[cur_len], ubuf, buffer_len);  // 把 user buffer 的資料複製到 kernel buffer
    if (fail) return -EFAULT;
    cur_len += buffer_len;

    int len = sprintf(&buf[cur_len], "PID: %d, TID: %d, Time: %llu\n",             // 在 kernel buffer 後面加上 PID TID Time
                      current->tgid, current->pid, current->utime / 100 / 1000);  
    cur_len += len;

    return buffer_len; 
}

// 當 User 呼叫 fgets() 讀取 /proc/Mythread_info 時，Kernel 會執行這個函式。
static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset) {
    /*Your code here*/
    int fail = copy_to_user(ubuf, buf, strlen(buf));   // 把 kernel buffer 的資料複製到 user buffer
    if (fail) return -EFAULT;

    *offset = strlen(buf);         // 更新 offset 為目前 kernel buffer 的長度
    memset(buf, 0, sizeof(buf));

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
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");