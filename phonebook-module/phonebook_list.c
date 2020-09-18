#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "phonebook_list.h"

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