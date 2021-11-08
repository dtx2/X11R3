/* 
 * crtstub : stubs for displays not linked into the server
 */

void notinstalled(name) char *name;
{
  printf("%s is not installed in server\n",name);
  puts("Run ??? to install the display");
  exit(1);
}

