/*
 * callBlowFish.c
 *
 *  Created on: Jul 2, 2013
 *      Author: Jose Damico (jd.comment@gmail.com)
 */


#include <openssl/blowfish.h>
#include <openssl/evp.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cilk/cilk.h>
#include "libpq-fe.h"
#include <unistd.h>


#define IP_SIZE 512
#define OP_SIZE 1024*1024

unsigned char key[16];
unsigned char iv[8];

double startTime,endTime;
int intP = -1;

static void exit_nicely(PGconn *conn){
	PQfinish(conn);
	exit(1);
}

int decrypt (int infd, int outfd) {

	unsigned char           *inbuff, *outbuf;

	int             olen=0, tlen=0, n=0;
	EVP_CIPHER_CTX  ctx;
	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit(&ctx, EVP_bf_cbc(), key, iv);

	outbuf = (unsigned char *) malloc(sizeof(unsigned char) * OP_SIZE);
	inbuff = (unsigned char *) malloc(sizeof(unsigned char) * IP_SIZE);

	for (;;) {
		memset(inbuff, 0, IP_SIZE);

		if ((n = read(infd, inbuff, IP_SIZE)) == -1) {
			perror("read error");
			break;
		} else if (n == 0)
			break;

		if (EVP_EncryptUpdate(&ctx, outbuf, &olen, inbuff, n) != 1) {
			printf("error in encrypt update\n");
			return 0;
		}

		if ((n = write(outfd, outbuf, olen)) == -1)
			perror("write error");
	}
	tlen=0;
	if (EVP_EncryptFinal(&ctx, outbuf + olen, &tlen) != 1) {
		printf("error in encrypt final\n");
		return 0;
	}

	if ((n = write(outfd, outbuf+olen, tlen)) == -1)
		perror("write error");

	EVP_CIPHER_CTX_cleanup(&ctx);
	close (infd);
	fsync (outfd);
	return 1;
}

int encrypt (int infd, int outfd) {

	unsigned char           *inbuff, *outbuf;
	int             olen=0, tlen=0, n=0;
	EVP_CIPHER_CTX  ctx;
	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit(&ctx, EVP_bf_cbc(), key, iv);

	outbuf = (unsigned char *) malloc(sizeof(unsigned char) * OP_SIZE);
	inbuff = (unsigned char *) malloc(sizeof(unsigned char) * IP_SIZE);

	/* keep reading until a break */
	for (;;) {
		memset(inbuff, 0, IP_SIZE);
		if ((n = read(infd, inbuff, IP_SIZE)) == -1) {
			perror("read error");
			break;
		} else if (n == 0)
			break;

		memset(outbuf, 0, OP_SIZE);

		if (EVP_DecryptUpdate(&ctx, outbuf, &olen, inbuff, n) != 1) {
			printf("error in decrypt update\n");
			return 0;
		}
		if ((n = write(outfd, outbuf, olen)) == -1)
			perror("write error");
	}

	tlen=0;
	EVP_DecryptFinal(&ctx, outbuf + olen, &tlen);

	if ((n = write(outfd, outbuf+olen, tlen)) == -1)
		perror("write error");

	EVP_CIPHER_CTX_cleanup(&ctx);
	close (infd);
	close (outfd);
	return 1;
}

int main (int argc, char *argv[]) {

	char *p = "n";

	if(argc > 1) p = argv[1]; 
	intP = strcmp(p, "y");

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


	int   i=0,j=0;

	PGconn    *conn;
	PGresult   *res;

	int infd = -1;
	int inFiles[6];
	int flags1 = 0;
	mode_t mode;


	conn = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'CryptoCilk' user = 'jinwork' password = 'ignition' connect_timeout = '10' options='-c search_path=crypto'");


	if (PQstatus(conn) == CONNECTION_BAD)
	{
		fprintf(stderr, "%s", PQerrorMessage(conn));
		exit_nicely(conn);
	}

	res = PQexec(conn, "SELECT fqueue_id, fqueue_name FROM crypto.file_queue_t WHERE FQUEUE_TIME IS NULL");


	/* first, print out the attribute names */
	int nFields = PQnfields(res);

	char * arr  = (char *) _mm_malloc(nFields * sizeof(char), 64);

	int r  = PQntuples(res);

	if(intP == 0){

		cilk_for (i = 0; i < r; i++){
			arr =PQgetvalue(res, i, 1);
			printf("Starting encryption: %d\n", i);
			cilk_spawn encFile(arr);
		}

	}else{

		for (i = 0; i < r; i++){
			arr =PQgetvalue(res, i, 1);
			printf("Starting encryption: %d\n", i);
			encFile(arr, key, iv);
		}

	}

	//	char encExt[] = ".enc";
	//	char encFilename[200];
	//	for (i = 0; i < PQntuples(res); i++){
	//		arr =PQgetvalue(res, i, 1);
	//
	//		snprintf(encFilename, 200, "%s%s", arr, encExt);
	//		printf("Starting decryption: %d %s\n", i, encFilename);
	//		decFile(encFilename, key, iv);
	//	}




	PQclear(res);
	PQfinish(conn);

	end = clock();
	total_time = ((long double) (end - start)) / (long double) CLOCKS_PER_SEC;

	printf("CLOCK() Start time: %.5f\n",(double) start);
	printf("CLOCK() End time: %.5Lf\n", (long double) end);
	printf("CLOCK() Total Time  %.5Lf\n", (total_time));


	gettimeofday(&tv2, NULL);

	double gTime = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	printf ("[gettimeofday()] Total time = %f seconds\n", gTime);


	conn = PQconnectdb("hostaddr = '127.0.0.1' port = '5432' dbname = 'CryptoCilk' user = 'jinwork' password = 'ignition' connect_timeout = '10' options='-c search_path=crypto'");
	if (PQstatus(conn) == CONNECTION_BAD){
		fprintf(stderr, "%s", PQerrorMessage(conn));
		exit_nicely(conn);
	}
	char preSql[100];
	sprintf (preSql, "INSERT INTO crypto.proc_log_t (proc_log_time, proc_log_paral)VALUES(%.5f, %d)",gTime, intP);
	res = PQexec(conn, preSql);
	PQclear(res);
	PQfinish(conn);

	//_mm_free(arr);
	return 0;
}

int encFile(const char *filename){
	int flags1 = 0, flags2 = 0, infd, outfd, decfd;
	mode_t mode;
	char choice, temp;
	int done = 0, n, olen;

	bzero(&infd, sizeof(int));
	bzero(&outfd, sizeof(int));

	//	bzero (&key, 16);
	//	bzero (&iv, 8);
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

	if( remove(encFilename) != 0 ) perror( "Error deleting file" );
	if ((infd = open (filename, flags1, mode)) == -1) printf("IO Error: %s\n", filename);
	if ((outfd = open (encFilename, flags2, mode)) == -1) perror ("open output file error");

	printf("infd: %d\n", infd);

	if(intP == 0) cilk_spawn encrypt (infd, outfd);
	else encrypt (infd, outfd);


	return 0;
}

int decFile(const char *filename, unsigned char *key[16], unsigned char *iv[8]){
	int flags1 = 0, flags2 = 0, infd, outfd, decfd;
	mode_t mode;
	char choice, temp;
	int done = 0, n, olen;

	bzero(&infd, sizeof(int));
	bzero(&outfd, sizeof(int));

	//	bzero (&key, 16);
	//	bzero (&iv, 8);
	bzero (&mode, sizeof (mode));

	flags1 = flags1 | O_RDONLY;
	flags2 = flags2 | O_RDONLY;
	flags2 = flags2 | O_WRONLY;
	flags2 = flags2 | O_CREAT;

	mode = mode | S_IRUSR;
	mode = mode | S_IWUSR;

	char encExt[] = ".iso";
	char encFilename[200];

	snprintf(encFilename, 200, "%s%s", filename, encExt);

	if( remove(encFilename) != 0 ) perror( "Error deleting file" );
	if ((infd = open (filename, flags1, mode)) == -1) printf("IO Error: %s\n", filename);
	if ((outfd = open (encFilename, flags2, mode)) == -1) perror ("open output file error");

	printf("infd: %d\n", infd);

	if(intP == 0) cilk_spawn decrypt(infd, outfd);
	else decrypt(infd, outfd);


	return 0;
}



