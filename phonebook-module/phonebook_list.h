#include <linux/list.h>

#define CHAR_BUF_LEN 32

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

int find_user_data_by_surname(const char * surname, phonebook_t ** returned);
int add_user_data(user_data_t * entry);
int del_user_data(phonebook_t * deleted);
void delete_all(void);
