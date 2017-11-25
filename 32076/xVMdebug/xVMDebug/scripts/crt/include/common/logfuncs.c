#include <Windows.h>
#include <internal.h>

void printmem(unsigned char* plpmem,int pszmem,int pnlchr)
{
	int		i,j,k;
	char*	pstr;
	char* pstrascii;
	if (pszmem <= 0 || !plpmem)
		return;
	pstr = (char*)malloc(pszmem*3+pnlchr+5);
	if (!pstr)
		return;
	pstrascii = (char*)malloc(pnlchr+1);
	for (i=0,j=0,k=0;i<pszmem;i++)
	{
		if (pnlchr > 0)
		{
			if (i>=1 && i%pnlchr == 0)
			{
				print(pszmem,0,"%s%s",pstr,pstrascii);
				j=0;
				k=0;
			}
		}
		sprintf(pstr+j,"%02X ",plpmem[i]);
		j+=3;
		if (plpmem[i] >= 0x20 && plpmem[i] <= 0x7F)
			*(pstrascii+k) = plpmem[i];
		else
			*(pstrascii+k) = '.';
		k++;
		*(pstr+j) = 0;
		*(pstrascii+k) = 0;
	}
	if (i != 0)
	{
		for (;(i%pnlchr) != 0;i++)
			strcat(pstr,"   ");
	}
		
	print(pszmem,0,"%s%s",pstr,pstrascii);
	free(pstr);
	if (pstrascii)
		free(pstrascii);
}
