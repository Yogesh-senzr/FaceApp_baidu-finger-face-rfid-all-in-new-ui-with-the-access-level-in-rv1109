#include "Utils.h"
#include "utils/Utils.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>

bool CheckUsbStorageExist()
{
	FILE *pFile = NULL;
	char buf[256] = { 0 };
	if ((pFile = popen("mount|grep /udisk", "r")) == NULL)
	{
		return false;
	}
	fgets(buf, sizeof(buf), pFile);
	fclose(pFile);
	if (strlen(buf) > 10)
	{
		return true;
	}
	return false;
}

int get_GpioValue(unsigned int gpio_chip_num,unsigned int gpio_offset_num)
{
	FILE *fp;
	char file_name[50];
	unsigned int gpio_num;
	unsigned char buf[10];
	
	gpio_num = gpio_chip_num * 8 + gpio_offset_num;

	sprintf(file_name, "/sys/class/gpio/export");
	fp = fopen(file_name, "w");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "%d", gpio_num);
	fclose(fp);
	
	sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
	fp = fopen(file_name, "rb+");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "in");
	fclose(fp);
	
	sprintf(file_name, "/sys/class/gpio/gpio%d/value", gpio_num);
	fp = fopen(file_name, "rb+");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	memset(buf, 0, 10);
	fread(buf, sizeof(char), sizeof(buf) - 1, fp);

	fclose(fp);

	sprintf(file_name, "/sys/class/gpio/unexport");
	fp = fopen(file_name, "w");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "%d", gpio_num);
	fclose(fp);
	
	return (int)(buf[0]-48);

}
int set_GpioValue(unsigned int gpio_chip_num,unsigned int gpio_offset_num,unsigned int gpio_out_val)
{
	FILE *fp;
	char file_name[50];
	char buf[10];
	unsigned int gpio_num;
	gpio_num = gpio_chip_num * 8 + gpio_offset_num;

	sprintf(file_name, "/sys/class/gpio/export");
	fp = fopen(file_name, "w");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "%d", gpio_num);
	fclose(fp);

	sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
	fp = fopen(file_name, "rb+");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "out");
	fclose(fp);

	sprintf(file_name, "/sys/class/gpio/gpio%d/value", gpio_num);
	fp = fopen(file_name, "rb+");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	if (gpio_out_val)
			strcpy(buf,"1");
	else
			strcpy(buf,"0");

	fwrite(buf, sizeof(char), sizeof(buf) - 1, fp);
	printf("%s: gpio%d_%d = %s\n", __func__,
					gpio_chip_num, gpio_offset_num, buf);
	fclose(fp);

	sprintf(file_name, "/sys/class/gpio/unexport");
	fp = fopen(file_name, "w");
	if (fp == NULL) {
			printf("Cannot open %s.\n", file_name);
			return -1;
	}
	fprintf(fp, "%d", gpio_num);
	fclose(fp);

	return 0;

}

static char *mypem = ISC_NULL;
static int pemsize = 0;
int write_data(void *buffer, size_t sz, size_t nmemb, void *ResInfo)
{
	std::string* psResponse = (std::string*) ResInfo; //强制转换
	psResponse->append((char*) buffer, sz * nmemb); //sz*nmemb表示接受数据的多少
	return sz * nmemb;  //返回接受数据的多少
}

static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
	CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
	if(mypem == ISC_NULL)
	{
		pemsize = Utils_getFileSize("/etc/ssl/cacert.pem");
		mypem = (char*) malloc(pemsize);
		if (mypem == ISC_NULL)
		{
			return rv;
		}
		int fd = open("/etc/ssl/cacert.pem", O_RDONLY, 0666);
		if (fd <= 0)
		{
			return rv;
		}
		read(fd, (void*)mypem, pemsize);
		close(fd);
	}

	BIO *cbio = BIO_new_mem_buf(mypem, pemsize);
	X509_STORE *cts = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);
	int i;
	STACK_OF(X509_INFO) *inf;
	(void) curl;
	(void) parm;

	if (!cts || !cbio)
	{
		return rv;
	}

	inf = PEM_X509_INFO_read_bio(cbio, ISC_NULL, ISC_NULL, ISC_NULL);

	if (!inf)
	{
		BIO_free(cbio);
		return rv;
	}

	for (i = 0; i < sk_X509_INFO_num(inf); i++)
	{
		X509_INFO *itmp = sk_X509_INFO_value(inf, i);
		if (itmp->x509)
		{
			X509_STORE_add_cert(cts, itmp->x509);
		}
		if (itmp->crl)
		{
			X509_STORE_add_crl(cts, itmp->crl);
		}
	}

	sk_X509_INFO_pop_free(inf, X509_INFO_free);
	BIO_free(cbio);

	rv = CURLE_OK;
	return rv;
}

std::string Utils_DoPostRawJson(std::string url, Json::Value &data)
{
	std::string ResString;
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = ISC_NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //数据请求到以后的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ResString);
		std::string requestData = data.toStyledString();
		LogD("%s %d %s", __FUNCTION__, __LINE__, requestData.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
	}
	if (res != CURLE_OK)
	{
		LogE("%s %s[%d] curl_easy_perform() failed: %s\n", __FILE__, __FUNCTION__, __LINE__, curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return "";
	}
	curl_easy_cleanup(curl);
	return ResString;
}
