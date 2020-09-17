#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Julia Akzhigitova");
MODULE_DESCRIPTION("Phonebook module");
MODULE_VERSION("0.01");

#define CHAR_BUF_LEN 64

typedef struct {
  uint32_t age;
  char name[CHAR_BUF_LEN];
  char surname[CHAR_BUF_LEN];
  char email[CHAR_BUF_LEN];
  char phone[CHAR_BUF_LEN];
} user_data_t;


typedef struct {
  struct list_head next_list;
  user_data_t user;
} phonebook_t;

static struct list_head phonebook_head;


int find_user_data_by_surname(const char * surname, phonebook_t ** returned) {
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

int add_user_data(user_data_t * entry) {
  printk(KERN_INFO "Start adding user with surname %s \n", entry->surname);

  
  phonebook_t * find_user = NULL;
  int res = find_user_data_by_surname(entry->surname, &find_user);
  if (0 == res) {
  	printk(KERN_INFO "Surname already exists");
  	return -1;
  }

  phonebook_t * new_user = kmalloc(sizeof(phonebook_t), GFP_KERNEL);

  new_user->user.age = entry->age;
  strncpy(new_user->user.name, entry->name, CHAR_BUF_LEN);
  strncpy(new_user->user.surname, entry->surname, CHAR_BUF_LEN);
  strncpy(new_user->user.email, entry->email, CHAR_BUF_LEN);
  strncpy(new_user->user.phone, entry->phone, CHAR_BUF_LEN);

  INIT_LIST_HEAD(&new_user->next_list);

  list_add(&new_user->next_list, &phonebook_head);

  printk(KERN_INFO "Added user with surname %s \n", entry->surname);

  return 0;
}

int del_user_data(phonebook_t * deleted) {
  printk(KERN_INFO "Start deleting user with surname %s \n", deleted->user.surname);

  list_del(&deleted->next_list);
  kfree(deleted);

  printk(KERN_INFO "Deleted user \n");

  return 0;
}

void delete_all(void) {
  printk(KERN_INFO "Free list \n");

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

/*-------------------------------------------------------------*/

#define CLASS_NAME "phonebook_class"
#define DEV_NAME "phonebook_dev"

#define ADD_CMD "add "
#define GET_CMD "get "
#define DEL_CMD "del "

#define RET_SIZE 512

static const int CMD_LEN = 4;

static dev_t dev_type;
static struct cdev ph_cdev;
static struct class * phonebook_class;
static struct device * phonebook_device;

char returned[RET_SIZE];


static int ph_open(struct inode * inode, struct file * file) {
  printk(KERN_INFO "Dev opening \n");

  return 0;
}

static int ph_close(struct inode * inode, struct file * file) {
  printk(KERN_INFO "Dev closing \n");

  return 0;
}

static ssize_t ph_read(struct file * file, char __user * user_buffer, size_t size, loff_t * offset) {
  if (*offset > 0) return 0;
  
  if (0 == copy_to_user(user_buffer, returned, strlen(returned))) {
  	(*offset) += strlen(returned);
  	return strlen(returned);
  }
  return -1;
}

static ssize_t ph_write(struct file * file, const char __user * user_buffer, size_t size, loff_t * offset) {
  char buffer[CMD_LEN + sizeof(user_data_t)];
  memset(buffer, 0, CMD_LEN + sizeof(user_data_t));
  memset(returned, 0, RET_SIZE);

  if (size > CMD_LEN + sizeof(user_data_t)) {
  	printk(KERN_INFO "Too large request \n");
  	strcpy(returned, "too large\0");
  	return -1;
  }
  copy_from_user(buffer, user_buffer, size);


  if (strncmp(ADD_CMD, buffer, CMD_LEN) == 0) {
  	user_data_t adding_user;
  	//name, surname, email, phone, age
  	sscanf(buffer + CMD_LEN, "%s %s %s %s %u", 
  		adding_user.name, adding_user.surname, adding_user.email, adding_user.phone, &adding_user.age);

  	int res = add_user_data(&adding_user);
  	if (-1 == res) {
  	  strcpy(returned, "already exist\0");
  	  return -1;
  	}

  	strcpy(returned, "done\0");
  } else if (strncmp(GET_CMD, buffer, CMD_LEN) == 0) {
  	phonebook_t * entry;
  	if (0 > find_user_data_by_surname(buffer + CMD_LEN, &entry)) {
  	  strcpy(returned, "not found\0");
  	  return -1;
  	}

  	//name, surname, email, phone, age
  	sprintf(returned, "%s %s %s %s %u", 
  		entry->user.name, entry->user.surname, entry->user.email, entry->user.phone, entry->user.age);
  } else if (strncmp(DEL_CMD, buffer, CMD_LEN) == 0) {
  	phonebook_t * entry;
  	if (0 > find_user_data_by_surname(buffer + CMD_LEN, &entry)) {
  	  strcpy(returned, "not found\0");
  	  return -1;
  	}

  	del_user_data(entry);
  	strcpy(returned, "done\0");
  } else {
  	strcpy(returned, "bad command\0");
  	return -1;
  }

  return size;
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