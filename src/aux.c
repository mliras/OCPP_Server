#include <stdio.h>
#include <string.h>
#include <time.h>
#include "aux.h"

int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * ipow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

int charCounter(char* pString, char c)
{
    int count = 0;
    char* pTemp = pString;

    while(pTemp != NULL)
    {
        pTemp = strchr(pTemp, c);
        if( pTemp ) {
            pTemp++;
            count++;
        }
    }

    return count;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = calloc(1,sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

//Returns the same string but in lower letters. The String remains in lower letter
char * strlwr(char * s)
{
        char *t = s;

        if (!s)
        {
                return 0;
        }

        int i = 0;
        while ( *t != '\0' )
        {
                if (*t >= 'A' && *t <= 'Z' )
                {
                        *t = *t + ('a' - 'A');
                }
                t++;
        }

        return s;
}

//Returns a copy of the string in lower letters. The String remains unmodified
char * strlwr_ex(char * s)
{
        if (!s)
        {
                return 0;
        }


        char *t, *u;
        t=strdup(s);
        u=t;
        int i = 0;
        while ( *t != '\0' )
        {
                if (*t >= 'A' && *t <= 'Z' )
                {
                        *t = *t + ('a' - 'A');
                }
                t++;
        }

        return u;
}


char *convertF(float myFloat)
{
	char *b=(char *)calloc(64,sizeof(char));
	//char buffer[64];
	//for (int i=0; i<64; i++) buffer[i]='\0';

	int ret = snprintf(b, 64, "%.1f", myFloat);

	if (ret < 0) {
	    return NULL;
	}

	//char *b=(char *)calloc(strlen(buffer)+1,sizeof(char));
	//strncpy(b, buffer, strlen(buffer));

	return b;
}


char *convert(int i)
{
	int digitos=1;
	int temp=10;

	while ((i/temp)>0){
		digitos++;
		temp*=10;
	}

	char *b=(char *)calloc(digitos+1,sizeof(char));
	temp/=10;

	for (int k=0; k<digitos; k++){
		b[k]=48+(i/temp);  //48 es 30h
		i=i%temp;
		temp/=10;
	}

	//b[digitos]='\0';

	return b;
}



char *getPayloadFromMessage(int uniqueId)
{
	queue_node *p=Message_queue;
	while (mutex) usleep(100);
	mutex=1;

	while (p && (uniqueId != p->UniqueId))
	{
			p=p->next;
	}


	mutex=0;

	if (p) return p->payload;

	return NULL;
}

///////////////////////////////////////////////////////////////////////////
//             MESSAGE QUEUE
///////////////////////////////////////////////////////////////////////////

void Dequeue()
{
		//if (debug) show_queue_contents();

		queue_node *p=Message_queue;
		while (mutex) usleep(10);
		mutex=1;
		// Cola vacia
        if(!Message_queue)
        {
                if (debug) printf("Queue is Empty\n");
                //return;
        }
        else
        {
        	//Lo quita del principio
        	if (p)
        	{
        		//printf("\nFREEING %s", p->payload);
        		Message_queue=p->next;

        		//NO SE POR QUE PERO DA UN DOUBLE FREE VULNERABILITY!!
        	//	if (p->payload) free(p->payload);
    			free(p);
        	}
        }
        mutex=0;
}

void Dequeue_i(int uniqueId)
{
		//if (debug) show_queue_contents();

		queue_node *p=Message_queue;
		queue_node *prev=p;
		while (mutex) usleep(10);
		mutex=1;
		// Cola vacia
        if(!Message_queue)
        {
                if (debug) printf("Queue is Empty\n");
                //return;
        }
        else
        {
        	//Lo quita del principio
        	while (p)
        	{
        		if (p->UniqueId==uniqueId)
        		{
        			if (Message_queue==p)
        			{
        				Message_queue=p->next;
        			}
        			else
        			{
        				prev->next=p->next;
        			}

        			if (p->payload) free(p->payload);

        			free(p);

        			break;
        		}

        		prev=p;
        		p=p->next;
        	}
        }
        mutex=0;
}


void Enqueue(queue_node *new_node)
{
	//    if (debug) show_queue_contents();
		queue_node *p=Message_queue;
		queue_node *r=new_node;
		r->next=NULL;

		while (mutex) usleep(100);
		mutex=1;

		//Si la cola esta vacia, hago que Message_queue apunte al nuevo nodo y borro el que hacia que estuviera vacio.
		if (!Message_queue)
		{
			Message_queue=r;
			Message_queue->next=NULL;
		}
		else
		{
			//Lo añade al final
			while (p->next!=NULL) {
				p=p->next;
			}
			p->next=r;
		}
		mutex=0;

//		if (Message_queue) printf("Message queue->next es: %x", Message_queue->next);
		if (debug) show_queue_contents();
}

void show_queue_contents()
{
	queue_node *p=Message_queue;
	printf("\n");

	while (p)
	{
	//	printf("%s con uniqueID %d en %p-->",p->payload, p->UniqueId, p);
		p=p->next;
	}

	if (!p) printf("NULL");
}

char * replace(char const * const original,char const * const pattern,char const * const replacement)
{
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string,
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

int getMessageIdFromUniqueId(const char* UniqueId_str)
{
	queue_node *p=Message_queue;
	int n=atoi(UniqueId_str);

	while (mutex) usleep(100);
	mutex=1;

	while (p && (n != p->UniqueId))
	{
	//	printf("%d", p->Messagetype);
		p=p->next;
	}

	mutex=0;
	if (p)
		return p->Messagetype;
	else
		return p;
}




char *getCurrentTime()
{
    //Recuperamos fecha y hora
    time_t now;
    time(&now);
    struct tm* now_tm;
    now_tm = localtime(&now);
    char *currentTime=(char *)calloc(1,sizeof(char)*80);
    strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
    currentTime[strlen(currentTime)]='\0';

    return currentTime;
}

//NO SE PERMITEN EXPIRY DATES DE SEGUNDOS
char *getExpiryTime(int n, char type)
{
	if (n<0) return -1;

	//Solo se permite indicar dia, hora o minuto
	if ((type!='h') && (type!='H') && (type!='d') && (type!='D') && (type!='m') && (type!='M')) return -1;

	//No mas de 365 dias, 24 horas o 60 minutos
	if (((type=='d') || (type=='D')) && n>365) return -1;
	if (((type=='h') || (type=='H')) && n>23) return -1;
	if (((type=='m') || (type=='M')) && n>59) return -1;

	time_t now;
	time(&now);
	struct tm* now_tm;
	now_tm = localtime(&now);

    int horas=now_tm->tm_hour;
    int minutos=now_tm->tm_min;
    int dias=now_tm->tm_yday;

    if (type=='d'||type=='D')
    {
    	dias+=n;
    	if (dias>365) now_tm->tm_year+=1;
    	now_tm->tm_yday=dias%365;
    }

    if (type=='h'||type=='H')
    {

    	horas+=n;
    	//printf("HORAS ES: %d",horas);
    	if (horas>23)
    	{
    			dias+=1;
    			if (dias>365) now_tm->tm_year+=1;
    			now_tm->tm_yday=dias%365;
    	}
    	now_tm->tm_hour=horas%24;
    }

    if (type=='m'||type=='M')
    {
    	minutos+=n;
    	if (minutos>60)
    	{
    		horas+=1;
    		if (horas>23)
    		{
    			dias+=1;
    			if (dias>365) now_tm->tm_year+=1;
    			now_tm->tm_yday=dias%365;
    		}
    		now_tm->tm_hour=horas%24;
    	}
    	now_tm->tm_min=minutos%60;
    }

    char *expiryTime=(char *)calloc(1,sizeof(char)*80);
    strftime (expiryTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
    expiryTime[strlen(expiryTime)]='\0';

    return expiryTime;
}

//Esta funcion encripta un texto, con una clave, que es un entero. Esta clave es solo parte de la clave final de cifrado.
//Devuelve un texto ya cifrado.
char *encrypt(char *text, int last){
	if (!text) return NULL;

	char *temp=(char *)calloc(1,sizeof(char)*strlen(text));

	char secret1[4] = { 53, 44, 71, 66};
	int i;
	char secret2[4] = { 177, 253, 122, last};

	    for(i = 0; i < strlen(text); i++)
	    {
	    	if ((i%8)<4)
			{
	    		temp[i] = (text[i] ^ secret1[i%4])+secret2[i%4];
	    		if (temp[i]>=0) temp[i]++;
	    	}
	    	else
	    	{
	    		temp[i] = (text[i] ^ secret2[i%4])+secret1[i%4];
	    		if (temp[i]>=0) temp[i]++;
	    	}
	    }
	 temp[i]='\0';
	 return temp;
}

char *decrypt(char *text, int last)
{
	if (!text) return NULL;

	char *temp=(char *)calloc(1,sizeof(char)*strlen(text));

	char secret1[4] = { 53, 44, 71, 66};
	int i;
	char secret2[4] = { 177, 253, 122, last};

	    for(i = 0; i < strlen(text); i++)
	    {
	    	if ((i%8)<4)
			{
	    		if (text[i]>=0) text[i]--;
	    		temp[i] = (text[i] - secret2[i%4]) ^ secret1[i%4];
	    	}
	    	else
	    	{
	    		if (text[i]>=0) text[i]--;
	    		temp[i] = (text[i] - secret1[i%4]) ^ secret2[i%4];
	    	}
	    }
	 temp[i]='\0';
	 return temp;
}

int getNextUniqueID(){
		return UniqueId++;
}

char *getNextUniqueID_char(){
	char *b=(char *) calloc(1,sizeof(char) * 8);
	sprintf(b, "%d", UniqueId++);
	return b;
}

int checkIsNumber(char *tmp)
{
	int j=0;
	int isDigit=0;
	while(j<strlen(tmp)){
	  isDigit = isdigit(tmp[j]);
	  if (isDigit == 0) break;
	  j++;
	}

	return isDigit;
}


char *getRandomString(int length, int mins, int mays, int nums)
{
	int idTokenLength=length;
	char *idToken=(char *)calloc(1,sizeof(char) * idTokenLength);

	//Here we should prepare the idTag. It's not well explained neither in 4.8 nor in 6.45, nor in 7.28 sections

	int l=0;
	if (mins) l+=26;
	if (mays) l+=26;
	if (nums) l+=10;

	if (l>0)
	{
		char charset[l];
		if (mins) strcat(charset, "abcdefghijklmnopqrstuvwxyz");
		if (mays) strcat(charset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		if (nums) strcat(charset, "0123456789");

		if (idToken) {
		 for (int n = 0;n < idTokenLength; n++) {
	    	int key = rand() % (int)(sizeof(charset) -1);
	        idToken[n] = charset[key];
		 }
	            idToken[idTokenLength] = '\0';
		}

	return idToken;
	}

	return NULL;
}

