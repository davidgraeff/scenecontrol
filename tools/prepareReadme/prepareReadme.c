#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[])
{
   if (argc != 2)
    return 1;
   
   FILE *file = fopen ( argv[1], "r" );
   if ( file != NULL )
   {
      char line [ 256 ];
      char lineWithChanges [ 258 ];
      while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
      {
	 if (strlen(line)==1)
             memcpy(lineWithChanges, " .\n\0", 4);
	 else {
	     lineWithChanges[0] = ' '; lineWithChanges[1] = ' ';
	     memcpy(lineWithChanges+2, line, sizeof line);
	 }
	 fputs ( lineWithChanges, stdout ); /* write the line */
      }
      fclose ( file );
   }
   else
   {
      perror ( argv[1] );
   }
   return 0;
}
