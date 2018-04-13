#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifdef NORAND
#define RAND_MAX 2796203
static unsigned int seed = 100001;
void srand(unsigned int s) { seed = s; }
unsigned int rand() {
  seed = (seed * 125) % RAND_MAX;
  return seed / RAND_MAX;
}
#endif

int mkstemp(char *tmplate)
{
  static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  int iLen, iRnd;
  char *pChr;

  errno = 0;

  iLen = strlen(tmplate);
  if(iLen >= 6)
  {
    pChr = tmplate + iLen - 6;
    srand((unsigned int) time(0));

    if(strncmp(pChr, "XXXXXX", 6) == 0)
    {
      int iChr;
      for(iChr = 0; iChr < 6; iChr++)
      {
        /* 528.5 = RAND_MAX / letters */
        iRnd = rand() / 528.5;
        *(pChr++) = letters[iRnd > 0 ? iRnd - 1 : 0];
      }
    }
    else
    {
      errno = EINVAL;
      return -1;
    }
  }
  else
  {
    errno = EINVAL;
    return -1;
  }
  
  return open(tmplate, O_CREAT | O_EXCL | O_RDWR, 0666);
}
