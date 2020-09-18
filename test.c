#include <linux/kernel.h>
#include <sys/syscall.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>
//436 - add_user
//437 - get_user
//438 - del_user

struct user_data {
  uint32_t age;
  char name[32];
  char surname[32];
  char email[32];
  char phone[32];
};


//require sudo!!!
int main() {
  struct user_data user;
  struct user_data output_user;

  memset(&user, 0, sizeof(struct user_data));

  sprintf(user.name, "%s", "test");
  sprintf(user.surname, "%s", "surname");
  sprintf(user.email, "%s", "sobaka");
  sprintf(user.phone, "%s", "88005553535");
  user.age = 20;

  long res = syscall(436, &user);
  printf("%d", res);
//  res = syscall(437, user.surname, strlen(user.surname), &output_user);
//  res = syscall(438, user.surname, strlen(user.surname));
  return 0;
}
