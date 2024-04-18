#ifndef OpenKAI_src_Net_HttpClient_H_
#define OpenKAI_src_Net_HttpClient_H_

#include "../Base/BASE.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED "application/json"
// (use for files: picture, mp3, tar-file etc.)
#define HTTP_CONTENT_TYPE_FORM_DATA "multipart/form-data"
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN "text/plain"

#define REQUEST_POST_FLAG 2
#define REQUEST_GET_FLAG 3

namespace kai
{

	struct http_request_get
	{
		struct evhttp_uri *uri;
		struct event_base *base;
		struct evhttp_connection *cn;
		struct evhttp_request *req;
		char **image;
		size_t imagesize;
		char *content_type;
		char *post_data;
	};

	// struct http_request_post {
	//	struct evhttp_uri *uri;
	//	struct event_base *base;
	//	struct evhttp_connection *cn;
	//	struct evhttp_request *req;
	//	char *content_type;
	//	char *post_data;
	// };

	class HttpClient
	{
	public:
		HttpClient();
		~HttpClient();

		int init();
		int uninit();
		int get_image(const char *url, char **image, size_t &imagesize);
		int post_imageinfo(const char *url, const char *databuffer);

	private:
		struct event_base *_base;
	};

}
#endif
