#ifndef NEFIT_EASY_H
#define NEFIT_EASY_H

#include <sys/queue.h>

#include <json-c/json.h>
#include <openssl/aes.h>
#include <strophe.h>

#define EASY_UNUSED(var) (void)(var)

struct nefit_easy;

typedef void (netif_easy_callback)(struct nefit_easy *easy, json_object *new_obj);

struct request {
	STAILQ_ENTRY(request) next;
	char *http_req;					/* HTTP request to send to the thermostat */
};

struct nefit_easy
{
	int connected;
	int busy;
	AES_KEY aesKeyEnc;
	AES_KEY aesKeyDec;
	xmpp_conn_t *xmpp_conn;
	xmpp_ctx_t *xmpp_ctx;
	char *gateway;
	STAILQ_HEAD(requests, request) requests;
	netif_easy_callback *cb;
};

int easy_connect(struct nefit_easy *easy, const char *serial, char const *access_key,
				 char const *password, netif_easy_callback *cb);
int easy_get(struct nefit_easy *easy, char const *url);
int easy_put_object(struct nefit_easy *easy, char const *url, json_object *obj);
int easy_put_double(struct nefit_easy *easy, char const *url, double value);
int easy_put_string(struct nefit_easy *easy, char const *url, char const *value);

#endif
