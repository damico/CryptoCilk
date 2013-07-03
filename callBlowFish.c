/*
 * callBlowFish.c
 *
 *  Created on: Jul 2, 2013
 *      Author: root
 */


#include <openssl/blowfish.h>
#include <openssl/evp.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cilk/cilk.h>



#define IP_SIZE 1024
#define OP_SIZE 1032

unsigned char key[16];
unsigned char iv[8];

double startTime,endTime;

int decrypt (int infd, int outfd) {

	unsigned char outbuf[IP_SIZE];
	int olen, tlen, n;
	char inbuff[OP_SIZE];
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init (&ctx);
	EVP_DecryptInit (&ctx, EVP_bf_cbc (), key, iv);

	for (;;)
	{
		bzero (&inbuff, OP_SIZE);
		if ((n = read (infd, inbuff, OP_SIZE)) == -1)
		{
			perror ("read error");
			break;
		}
		else if (n == 0)
			break;

		bzero (&outbuf, IP_SIZE);

		if (EVP_DecryptUpdate (&ctx, outbuf, &olen, inbuff, n) != 1)
		{
			printf ("error in decrypt update\n");
			return 0;
		}

		if (EVP_DecryptFinal (&ctx, outbuf + olen, &tlen) != 1)
		{
			printf ("error in decrypt final\n");
			return 0;
		}
		olen += tlen;
		if ((n = write (outfd, outbuf, olen)) == -1)
			perror ("write error");
	}

	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int encrypt (int infd, int outfd) {

	unsigned char outbuf[OP_SIZE];
	int olen, tlen, n;
	char inbuff[IP_SIZE];
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init (&ctx);
	EVP_EncryptInit (&ctx, EVP_bf_cbc (), key, iv);

	for (;;)
	{
		bzero (&inbuff, IP_SIZE);

		if ((n = read (infd, inbuff, IP_SIZE)) == -1)
		{
			perror ("read error");
			break;
		}
		else if (n == 0)
			break;

		if (EVP_EncryptUpdate (&ctx, outbuf, &olen, inbuff, n) != 1)
		{
			printf ("error in encrypt update\n");
			return 0;
		}

		if (EVP_EncryptFinal (&ctx, outbuf + olen, &tlen) != 1)
		{
			printf ("error in encrypt final\n");
			return 0;
		}
		olen += tlen;
		if ((n = write (outfd, outbuf, olen)) == -1)
			perror ("write error");
	}
	EVP_CIPHER_CTX_cleanup (&ctx);
	return 1;
}

int main (int argc, char *argv[]) {


	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);


	long double total_time;
	clock_t start, end;
	start = clock();

	key[0] = 150;
	key[1] = 56;
	key[2] = 47;
	key[3] = 134;
	key[4] = 227;
	key[5] = 62;
	key[6] = 74;
	key[7] = 102;
	key[8] = 178;
	key[9] = 156;
	key[10] = 45;
	key[11] = 99;
	key[12] = 222;
	key[13] = 212;
	key[14] = 84;
	key[15] = 190;

	iv[0] = 238;
	iv[1] = 25;
	iv[2] = 170;
	iv[3] = 1;
	iv[4] = 227;
	iv[5] = 70;
	iv[6] = 47;
	iv[7] = 70;

	char* files[] = {"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian0.iso",
			"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian1.iso",
			"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian2.iso",
			"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian3.iso",
			"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian4.iso",
			"/media/d267fe98-4fc0-4484-891d-8263c9849c18/tests/debian5.iso"
			};

	int i = 0;
	for(i=0; i<4; i++){
		printf("Enc: %d\n", i);
		encFile(files[i], key, iv);

	}


//http://stackoverflow.com/questions/5248915/execution-time-of-c-program


	end = clock();//time count stops
	total_time = ((long double) (end - start)) / (long double) CLOCKS_PER_SEC;//calulate total time
	//printf("\nTime taken: %f", total_time);

	printf("start time: %.16g\n",(double) start);
	printf("end time: %Lf\n", (long double) end);
	//printf("CLOCKS_PER_SEC %2.16e\n", (double) CLOCKS_PER_SEC);


	//if(total_time<100000000) printf("TOTAL TIME %.5f seconds\n", (total_time/10));
	//else printf("TOTAL TIME (/10) %f seconds\n", (total_time/10));

	printf("TOTAL TIME  %Lf\n", (total_time));


	printf("Time taken: %.2fs\n", (double)(clock() - start)/CLOCKS_PER_SEC);


	gettimeofday(&tv2, NULL);


	printf ("Total time = %f seconds\n",
	             (double) (tv2.tv_usec - tv1.tv_usec)/1000000 +
	             (double) (tv2.tv_sec - tv1.tv_sec));


	return 0;
}

int encFile(const char *filename, unsigned char *key[16], unsigned char *iv[8]){
	int flags1 = 0, flags2 = 0, outfd, infd, decfd;
	mode_t mode;
	char choice, temp;
	int done = 0, n, olen;

	bzero (&key, 16);
	bzero (&iv, 8);
	bzero (&mode, sizeof (mode));

	flags1 = flags1 | O_RDONLY;
	flags2 = flags2 | O_RDONLY;
	flags2 = flags2 | O_WRONLY;
	flags2 = flags2 | O_CREAT;

	mode = mode | S_IRUSR;
	mode = mode | S_IWUSR;

	char encExt[] = ".enc";
	char encFilename[200];

	snprintf(encFilename, 200, "%s%s", filename, encExt);

	if ((infd = open (filename, flags1, mode)) == -1) printf("IO Error: %s\n",filename);
	if ((outfd = open (encFilename, flags2, mode)) == -1) perror ("open output file error");

	encrypt (infd, outfd);

	close (infd);
	close (outfd);
	return 0;
}

