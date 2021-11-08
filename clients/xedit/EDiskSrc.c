 
#include "xedit.h"

XtTextSource makeStringSource(string)
  char *string;
{
    return (XtTextSource)TCreateApAsSource();
}


  
XtTextSource PseudoDiskSourceCreate(filename)
  char *filename;
{
#define chunk 2048
  XtTextSource source;
  XtTextBlock text;
  XtTextPosition pos;
  FILE *file;
  char *buf;
  int amount;
    if(!strlen(filename)){
	return( makeStringSource(""));
    }
    file = fopen(filename, "r");
    if(!file)
	return(makeStringSource(""));
    source = makeStringSource("");
    pos = 0;
    text.format = FMT8BIT;
    buf = malloc(chunk);
    text.ptr = buf;
    while(( amount = fread(buf, 1, chunk, file)) > 0){
        text.length = amount;
        (*source->Replace)(source, pos, pos, &text);
        pos += amount; 
    }
    fclose(file);
    free(buf);
    return(source);
}

void PseudoDiskSourceDestroy(source)
XtTextSource source;
{
	TDestroyApAsSource(source); 
}
