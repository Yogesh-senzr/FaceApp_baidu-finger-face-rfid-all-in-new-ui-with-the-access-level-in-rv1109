#include "id_data_decode.h"

#ifndef LOCAL_DECODE_IMG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

//#define POSTURL	"http://www.baidu.com/img/logo_gif_be7507c6c688ba2bd085da2ad1015e7d.gif?v=19489737.gif"

//#define FILENAME   "logo.gif"

size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream)
{
	FILE *fptr = (FILE*)stream;
	fwrite(buffer,size,nmemb,fptr);
			return size*nmemb;
}

int get_bmp_by_url(char *post_url, char *file_name){
	CURL *curl;
	CURLcode res;
	FILE* fptr;
	struct curl_slist *http_header = NULL;
	printf("get bmp start\n");
//	printf*("the post_url:%s, the file_name:%s\n", post_url, file_name);
	if ((fptr = fopen(file_name,"wb+")) == NULL)
	{
//		printf("fopen file error:%s\n",file_name);
		return -1;
	}

	curl = curl_easy_init();
	if (!curl)
	{
		printf("curl init failed\n");
		return -1;
	}
	printf("curl init ok\n");
	curl_easy_setopt(curl,CURLOPT_URL, post_url);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr);
	res = curl_easy_perform(curl);
	printf("res:%d\n", res);
	if (res != CURLE_OK)
	{
		switch(res)
		{
			case CURLE_UNSUPPORTED_PROTOCOL:
				fprintf(stderr,"不支持的协议,由URL的头部指定\n");
				break;
			case CURLE_COULDNT_CONNECT:
				fprintf(stderr,"不能连接到remote主机或者代理\n");
				break;
			case CURLE_HTTP_RETURNED_ERROR:
				fprintf(stderr,"http返回错误\n");
				break;
			case CURLE_READ_ERROR:fprintf(stderr,"读本地文件错误\n");
				break;
			default:
				fprintf(stderr,"返回值:%d\n",res);
				break;																																																									}																													return -1;
	}
	curl_easy_cleanup(curl);
	return 0;
}
#endif
