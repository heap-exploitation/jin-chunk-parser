#define _GNU_SOURCE
#include <link.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#define DEBUG_SYMBOL_PATH_PREFIX  "/usr/lib/debug/.build-id"
#define DEBUG_SYMBOL_EXTENTION    ".debug"
#define TARGET_LIBRARY_PATH       "/lib/x86_64-linux-gnu/libc.so.6"
#define TARGET_SYMBOL_NAME        "main_arena"
 
static void *my_main_arena = NULL;
 
static size_t map_file(const char *path, char **buf) {
  int fd = -1;
  int ret;
  struct stat stat;
 
  *buf = MAP_FAILED;
 
  if ((fd = open(path, O_RDONLY)) < 0)
    return 0;
 
  ret = fstat(fd, &stat);
  if (ret)
    goto error;
 
  *buf = (char *)mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (*buf == MAP_FAILED)
    goto error;
 
  close(fd);
  return stat.st_size;
 
 error:
  if (fd != -1)
    close(fd);
  return 0;
}
 
static void unmap_file(void *buf, size_t size) {
  munmap(buf, size);
}
 
static void generate_symbol_file_path(char **path, const char *build_id, size_t size) {
  int i;
  char tmp[4];
  char *p = *path;
 
  sprintf(p, "%s/%x/", DEBUG_SYMBOL_PATH_PREFIX, build_id[0]);
  build_id++;
  size--;
 
  for (i = 0; i < size; i++) {
    sprintf(tmp, "%02x", (unsigned char)build_id[i]);
    strcat(p, tmp);
  }
  strcat(p, DEBUG_SYMBOL_EXTENTION);
  printf("debug symbol file: path=%s\n", p);
}
 
static size_t find_symbol_offset(const char *symfile, const char *symname) {
  char *base = MAP_FAILED;
  size_t size;
  Elf64_Ehdr *header;
  Elf64_Shdr *secs;
  Elf64_Sym  *symtab;
  char *names;
  int secidx, symidx;
  unsigned symcnt;
  size_t offset = (size_t)-1;
 
  size = map_file(symfile, &base);
  if (size == 0 || base == MAP_FAILED)
    goto error;
 
  header = (Elf64_Ehdr *)base;
  secs   = (Elf64_Shdr *)(base + header->e_shoff);
 
  for (secidx = 0; secidx < header->e_shnum; secidx++) {
    if (secs[secidx].sh_type != SHT_SYMTAB)
      continue;
    if (secs[secidx].sh_entsize == 0)
      continue;
 
    symtab = (Elf64_Sym *)(base + secs[secidx].sh_offset);
    names  = (char *)(base + secs[secs[secidx].sh_link].sh_offset);
    symcnt = secs[secidx].sh_size / secs[secidx].sh_entsize;
 
    for (symidx = 0; symidx < symcnt; symidx++) {
      if (strcmp(names + symtab[symidx].st_name, symname) == 0) {
        offset = symtab[symidx].st_value;
        goto final;
      }
    }
  }
 
 final:
  unmap_file(base, size);
  return offset;
 
 error:
  if (base != MAP_FAILED)
    unmap_file(base, size);
  return (size_t)-1;
}
 
static Elf64_Nhdr *find_build_id_note(const char *base) {
  Elf64_Ehdr *header = (Elf64_Ehdr *)base;
  Elf64_Shdr *secs   = (Elf64_Shdr *)(base + header->e_shoff);
  Elf64_Nhdr *note = NULL;
  int secidx;
  char *name;
 
  for (secidx = 0; secidx < header->e_shnum; secidx++) {
    if (secs[secidx].sh_type != SHT_NOTE)
      continue;
 
    note = (Elf64_Nhdr *)(base + secs[secidx].sh_offset);
    if (note->n_type != NT_GNU_BUILD_ID)
      continue;
 
    name = (char *)(base + secs[secidx].sh_offset + sizeof(Elf64_Nhdr));
    if (note->n_namesz == 4
        && note->n_descsz != 0
        && memcmp(name, "GNU", 4) == 0) {
      return note;
    }
  }
  return NULL;
}
 
int callback(struct dl_phdr_info *info, size_t size, void *data) {
  char *base = MAP_FAILED;
  size_t sz;
  Elf64_Nhdr *note;
  char *build_id;
  char *path = alloca(256);
  size_t offset;
 
  if (strcmp(info->dlpi_name, TARGET_LIBRARY_PATH) != 0)
    return 0;
 
  sz = map_file(info->dlpi_name, &base);
  if (sz == 0 || base == MAP_FAILED)
    return 0;
 
  note = find_build_id_note(base);
  if (!note)
    goto error;
 
  build_id = ((char *)note) + sizeof(Elf64_Nhdr) + note->n_namesz;
  generate_symbol_file_path(&path, build_id, note->n_descsz);
 
  offset = find_symbol_offset(path, TARGET_SYMBOL_NAME);
  if (offset == (size_t)-1)
    goto error;
  
  my_main_arena = (void *)(((char *)info->dlpi_addr) + offset);
  printf("%s found: %p\n", TARGET_SYMBOL_NAME, my_main_arena);
 
  unmap_file(base, sz);
  return 0;
 
 error:
  if (base != MAP_FAILED)
    unmap_file(base, sz);
  return 0;
}
 
int main(void) {
  dl_iterate_phdr(callback, NULL);
  return 0;
}