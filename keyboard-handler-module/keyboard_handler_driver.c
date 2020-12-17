







#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Julia Akzhigitova");
MODULE_DESCRIPTION("Keyboard handler module");
MODULE_VERSION("0.01");


static const unsigned int KEYBOARD_IRQ = 1;

static atomic_long_t key_counter = ATOMIC_LONG_INIT(0);
static struct tasklet_struct counter_task;

struct task_struct * printer_thread_struct;


static irqreturn_t keyboard_counter_handler_top(int irq, void * dev_id) {
  tasklet_schedule(&counter_task);
  return IRQ_HANDLED;
}

void keyboard_counter_handler_bottom(unsigned long data) {
  atomic_long_inc(&key_counter);
}


int printer_thread(void * data) {
  unsigned long count;

  while(!kthread_should_stop()) {
    msleep(60000);
    count = atomic_long_xchg(&key_counter, 0);
    printk(KERN_INFO "Pressed %lu keys in the last minute", count);
  }

  do_exit(0);
}


static int __init keyboard_module_init(void) {
  printk(KERN_INFO "Keyboard module init \n");

  int res = request_irq(KEYBOARD_IRQ, keyboard_counter_handler_top, IRQF_SHARED, "keyboard_module", &counter_task);
  if (res < 0) {
    printk(KERN_ERR "Cannot register interrupt handler \n");
    return res;
  }

  tasklet_init(&counter_task, keyboard_counter_handler_bottom, 0);

  printer_thread_struct = kthread_run(printer_thread, NULL, "keyboard-counter-printer-thread");

  return 0;
}

static void __exit keyboard_module_exit(void) {
  kthread_stop(printer_thread_struct);
  tasklet_kill(&counter_task);
  free_irq(KEYBOARD_IRQ, &counter_task);

  printk(KERN_INFO "Exit from keyboard module! \n");
}

module_init(keyboard_module_init);
module_exit(keyboard_module_exit);
