#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

#define CHAR_BUF_LEN 64
#define REQUEST_SIZE 512

#define DEV_NAME "/dev/phonebook_dev"

struct user_data {
  uint32_t age;
  char name[CHAR_BUF_LEN];
  char surname[CHAR_BUF_LEN];
  char email[CHAR_BUF_LEN];
  char phone[CHAR_BUF_LEN];
};


asmlinkage long sys_add_user(struct user_data * input_data) {
  mm_segment_t prev_fs = get_fs();
  set_fs(KERNEL_DS);
  
  int fd = ksys_open(DEV_NAME, O_RDWR, 0); 
  if (0 > fd) {
    set_fs(prev_fs);
    return -1;
  }

  struct user_data new_user;

  if (copy_from_user(new_user.name, input_data->name, strlen(input_data->name))
          || copy_from_user(new_user.surname, input_data->surname, strlen(input_data->surname))
          || copy_from_user(new_user.email, input_data->email, strlen(input_data->email))
          || copy_from_user(new_user.phone, input_data->phone, strlen(input_data->phone))
          || copy_from_user(&(new_user.age), &(input_data->age), sizeof(input_data->age))) {
    ksys_close(fd);
    set_fs(prev_fs);
    return -1;
  }

  printk(KERN_INFO "Start request add");
  char request[REQUEST_SIZE];
  //memset(request, 0, REQUEST_SIZE);
  //name, surname, email, phone, age
  sprintf(request, "add %s %s %s %s %u", 
        new_user.name, new_user.surname, new_user.email, new_user.phone, new_user.age);

  int written = ksys_write(fd, request, strlen(request));
  ksys_read(fd, request, REQUEST_SIZE);

  ksys_close(fd);

  set_fs(prev_fs);
  if (0 > written) {
    return -1;
  }

  return written;
}

asmlinkage long sys_get_user(const char * surname, unsigned int len, struct user_data * output_data) {
  mm_segment_t prev_fs = get_fs();
  set_fs(KERNEL_DS);
  
  int fd = ksys_open(DEV_NAME, O_RDWR, 0); 
  if (0 > fd) {
    set_fs(prev_fs);
    return -1;
  }

  char surname_kernel[CHAR_BUF_LEN];
  //memset(surname_kernel, 0, CHAR_BUF_LEN);
  if (copy_from_user(surname_kernel, surname, len)) {
    ksys_close(fd);
    set_fs(prev_fs);
    return -1;
  }

  char request[REQUEST_SIZE];
  //memset(request, 0, REQUEST_SIZE);
  sprintf(request, "get %s", surname_kernel);

  int written = ksys_write(fd, request, strlen(request));
  if (0 > written) {
    ksys_close(fd);
    set_fs(prev_fs);
    return -1;
  }

  //memset(request, 0, REQUEST_SIZE);
  if (0 > ksys_read(fd, request, REQUEST_SIZE)) {
    ksys_close(fd);
    set_fs(prev_fs);
    return -1;
  }
  ksys_close(fd);

  struct user_data getting_user;
  //memset(&getting_user, 0, sizeof(struct user_data));
  //name, surname, email, phone, age
  sscanf(request, "%s %s %s %s %u", 
        getting_user.name, getting_user.surname, getting_user.email, getting_user.phone, &getting_user.age);

  if (copy_to_user(output_data->name, getting_user.name, strlen(getting_user.name))
          || copy_to_user(output_data->surname, getting_user.surname, strlen(getting_user.surname))
          || copy_to_user(output_data->email, getting_user.email, strlen(getting_user.email))
          || copy_to_user(output_data->phone, getting_user.phone, strlen(getting_user.phone))
          || copy_to_user(&(output_data->age), &(getting_user.age), sizeof(getting_user.age))) {
    set_fs(prev_fs);
    return -1;
  }
  
  set_fs(prev_fs);
  return 1;
}

asmlinkage long sys_del_user(const char * surname, unsigned int len) {
  mm_segment_t prev_fs = get_fs();
  set_fs(KERNEL_DS);
  
  int fd = ksys_open(DEV_NAME, O_RDWR, 0); 
  if (0 > fd) {
    set_fs(prev_fs);
    return -1;
  }

  char surname_kernel[CHAR_BUF_LEN];
  //memset(surname_kernel, 0, CHAR_BUF_LEN);
  if (copy_from_user(surname_kernel, surname, len)) {
    ksys_close(fd);
    set_fs(prev_fs);
    return -1;
  }

  char request[REQUEST_SIZE];
  //memset(request, 0, REQUEST_SIZE);
  sprintf(request, "del %s", surname_kernel);

  int written = ksys_write(fd, request, strlen(request));
  ksys_read(fd, request, REQUEST_SIZE);

  ksys_close(fd);

  set_fs(prev_fs);
  if (0 > written) {
    return -1;
  }

  return 1;
}