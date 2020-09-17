#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Julia Akzhigitova");
MODULE_DESCRIPTION("Phonebook module");
MODULE_VERSION("0.01");

#define CHAR_BUF_LEN 256

typedef struct {
  uint32_t age;
  char name[CHAR_BUF_LEN];
  char surname[CHAR_BUF_LEN];
  char email[CHAR_BUF_LEN];
  char phone[CHAR_BUF_LEN];
} user_entry_t;


typedef struct {
  struct list_head next_list;
  user_entry_t user;
} phonebook_t;

static struct list_head phonebook_head;


int find_user_entry_by_surname(const char * surname, phonebook_t ** returned) {
  printk(KERN_INFO "Start searching user with surname %s \n", surname);

  struct list_head * iter;
  phonebook_t * entry;

  list_for_each(iter, &phonebook_head) {
  	entry = list_entry(iter, phonebook_t, next_list);

  	printk(KERN_INFO "Look at entry with surname %s \n", entry->user.surname);
  	if (strcmp(entry->user.surname, surname) == 0) {
  		*returned = entry;

  		return 0;
  	}
  }

  return -1;
}

int add_user_entry(const char * name, const char * surname, const char * email, const char * phone, uint32_t age) {
  printk(KERN_INFO "Start adding user with surname %s \n", surname);

  
  phonebook_t * find_user = NULL;
  int res = find_user_entry_by_surname(surname, &find_user);
  if (-1 == res) {
  	printk(KERN_INFO "Surname already exists");
  	return -1;
  }

  phonebook_t * new_user = kmalloc(sizeof(phonebook_t), GFP_KERNEL);

  new_user->user.age = age;
  strncpy(new_user->user.name, name, CHAR_BUF_LEN);
  strncpy(new_user->user.surname, surname, CHAR_BUF_LEN);
  strncpy(new_user->user.email, email, CHAR_BUF_LEN);
  strncpy(new_user->user.phone, phone, CHAR_BUF_LEN);

  INIT_LIST_HEAD(&new_user->next_list);

  list_add(&new_user->next_list, &phonebook_head);

  printk(KERN_INFO "Added user with surname %s \n", surname);

  return 0;
}

int del_user_entry(phonebook_t * deleted) {
  printk(KERN_INFO "Start deleting user with surname %s \n", deleted->user.surname);

  list_del(&deleted->next_list);
  kfree(deleted);

  printk(KERN_INFO "Deleted user \n");

  return 0;
}

void delete_all(void) {
  struct list_head * iter;
  phonebook_t * entry;
    
  redo:
  	list_for_each(iter, &phonebook_head) {
      entry = list_entry(iter, phonebook_t, next_list);
      list_del(&entry->next_list);
      kfree(entry);
      goto redo;
  	}
}

//--проверки на exist!--
//отдать юзера полностью?
//функции для устройства - разнести их с самой структурой
/*-------------------------------------------------------------*/

#define CLASS_NAME "phonebook_class"
#define DEV_NAME "phonebook_dev"

static dev_t dev_type;
static struct cdev ph_cdev;
static struct class * phonebook_class;
static struct device * phonebook_device;


static int ph_open(struct inode * inode, struct file * file) {
  printk(KERN_INFO "Dev opening \n");

  return 0;
}

static int ph_close(struct inode * inode, struct file * file) {
  printk(KERN_INFO "Dev closing \n");

  return 0;
}

static ssize_t ph_read(struct file * file, char __user * user_buffer, size_t size, loff_t * offset) {
  return 0;
}

static ssize_t ph_write(struct file * file, const char __user * user_buffer, size_t size, loff_t * offset) {
  return 0;
}


static struct file_operations dev_ops =
{
    .owner        = THIS_MODULE,
    .release      = ph_close,
    .open         = ph_open,
    .read         = ph_read,
    .write        = ph_write,
};


static int __init phonebook_module_init(void) {
  printk(KERN_INFO "Phonebook module init \n");

  int res = alloc_chrdev_region(&dev_type, 0, 1, DEV_NAME);
  if (0 > res) {
    printk(KERN_INFO "Registration failed \n");
  	return -1;
  }

  phonebook_class = class_create(THIS_MODULE, CLASS_NAME);
  if (NULL == phonebook_class) {
    printk(KERN_INFO "Class creation failed \n");
  	unregister_chrdev_region(dev_type, 1);
    return -1;
  }

  cdev_init(&ph_cdev, &dev_ops);
  if (-1 == cdev_add(&ph_cdev, dev_type, 1)) {
    printk(KERN_INFO "Dev addition failed \n");
    class_destroy(phonebook_class);
  	unregister_chrdev_region(dev_type, 1);
    return -1;
  }

  phonebook_device = device_create(phonebook_class, NULL, dev_type, NULL, DEV_NAME);
  if (NULL == phonebook_device) {
    printk(KERN_INFO "Device creation failed \n");
    cdev_del(&ph_cdev);
    class_destroy(phonebook_class);
  	unregister_chrdev_region(dev_type, 1);
    return -1;
  }
  INIT_LIST_HEAD(&phonebook_head);

  return 0;
}

static void __exit phonebook_module_exit(void) {
  delete_all();
  device_destroy(phonebook_class, dev_type);
  cdev_del(&ph_cdev);
  class_destroy(phonebook_class);
  unregister_chrdev_region(dev_type, 1);

  printk(KERN_INFO "Exit from phonebook module! \n");
}

module_init(phonebook_module_init);
module_exit(phonebook_module_exit);