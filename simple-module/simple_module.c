#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

//#include <stdint.h>
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

struct list_head phonebook_head;

int add_user_entry(const user_entry_t entry) {
  printk(KERN_INFO "Start adding user with surname %s \n", entry.surname);

  //найти такого же
  phonebook_t * new_user = kmalloc(sizeof(phonebook_t), GFP_KERNEL);
  new_user->user.age = entry.age;
  strncpy(new_user->user.name, entry.name, CHAR_BUF_LEN);
  strncpy(new_user->user.surname, entry.surname, CHAR_BUF_LEN);
  strncpy(new_user->user.email, entry.email, CHAR_BUF_LEN);
  strncpy(new_user->user.phone, entry.phone, CHAR_BUF_LEN);
  INIT_LIST_HEAD(&new_user->next_list);

  list_add(&new_user->next_list, &phonebook_head);

  printk(KERN_INFO "Added user with surname %s \n", entry.surname);

  return 0;
}

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

int del_user_entry(phonebook_t * deleted) {
  printk(KERN_INFO "Start deleting user with surname %s \n", deleted->user.surname);

  list_del(&deleted->next_list);
  kfree(deleted);

  printk(KERN_INFO "Deleted user \n");

  return 0;
}

void delete_all(struct list_head *head)
{
  struct list_head * iter;
  phonebook_t * objPtr;
    
  redo:
  	list_for_each(iter, &phonebook_head) {
      objPtr = list_entry(iter, phonebook_t, next_list);
      list_del(&objPtr->next_list);
      kfree(objPtr);
      goto redo;
  	}
}

//--проверки на exist!--
//отдать юзера полностью?
//функции для устройства - разнести их с самой структурой


static int __init phonebook_module_init(void) {
  

  INIT_LIST_HEAD(&phonebook_head);
  printk(KERN_INFO "Test! %s\n", &phonebook_head);

  user_entry_t u1;
  u1.name[0] = 'a';
  u1.name[1] = '\0';

  u1.surname[0] = 'k';
  u1.surname[1] = '\0';


  user_entry_t u2;
  u2.name[0] = 'b';
  u2.name[1] ='\0';

  u2.surname[0] = 'l';
  u2.surname[1] = '\0';


  user_entry_t u3;
  u3.name[0] = 'c';
  u3.name[1] = '\0';

  u3.surname[0] = 'm';
  u3.surname[1] = '\0';

  add_user_entry(u1);
  add_user_entry(u2);
  add_user_entry(u3);

  phonebook_t * tt;

  find_user_entry_by_surname("k\0", &tt);
  //printk(KERN_INFO "I find! %s \n", tt.user.name);
  printk(KERN_INFO "wtf2 %s \n", tt);

  del_user_entry(tt);


  find_user_entry_by_surname("t\0", NULL);

  printk(KERN_INFO "Test! %s\n", &phonebook_head);

  return 0;
}

static void __exit phonebook_module_exit(void) {


  find_user_entry_by_surname("tested\0", NULL);
  delete_all(&phonebook_head);

  printk(KERN_INFO "Goodbye, test! %s\n", &phonebook_head);
}

module_init(phonebook_module_init);
module_exit(phonebook_module_exit);