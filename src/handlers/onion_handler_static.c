/*
	Onion HTTP server library
	Copyright (C) 2010 David Moreno Montero

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	*/

#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <regex.h>

#include <onion_handler.h>
#include <onion_response.h>

#include "onion_handler_static.h"

struct onion_handler_static_data_t{
	int code;
	const char *data;
	regex_t path;
};

typedef struct onion_handler_static_data_t onion_handler_static_data;

/**
 * @short Performs the real request: checks if its for me, and then write the data.
 */
int onion_handler_static_handler(onion_handler *handler, onion_request *request){
	onion_handler_static_data *d=handler->priv_data;

	if (regexec(&d->path, request->path, 0, NULL, 0)!=0)
		return 0;
	
	onion_response *res=onion_response_new(request);

	int length=strlen(d->data);
	onion_response_set_length(res, length);
	onion_response_set_code(res, d->code);
	
	onion_response_write_headers(res);
	
	onion_response_write(res, d->data, length);

	onion_response_free(res);
	
	return 1;
}

/// Removes internal data for this handler.
void onion_handler_static_delete(onion_handler_static_data *d){
	free((char*)d->data);
	regfree(&d->path);
	free(d);
}

/**
 * @short Creates a static handler that just writes some static data.
 *
 * Path is a regex for the url, as arrived here.
 */
onion_handler *onion_handler_static(const char *path, const char *text, int code){
	onion_handler *ret;
	ret=malloc(sizeof(onion_handler));
	memset(ret,0,sizeof(onion_handler));
	
	onion_handler_static_data *priv_data=malloc(sizeof(onion_handler_static_data));

	priv_data->code=code;
	priv_data->data=strdup(text);
	
	// Path is a little bit more complicated, its an regexp.
	int err;
	if (path)
		err=regcomp(&priv_data->path, path, REG_EXTENDED);
	else
		err=regcomp(&priv_data->path, "", REG_EXTENDED); // empty regexp, always true. should be fast enough. 
	if (err){
		char buffer[1024];
		regerror(err, &priv_data->path, buffer, sizeof(buffer));
		fprintf(stderr, "%s:%d Error analyzing regular expression '%s': %s.\n", __FILE__,__LINE__, path, buffer);
		onion_handler_static_delete(priv_data);
		return NULL;
	}
	
	ret->handler=onion_handler_static_handler;
	ret->priv_data=priv_data;
	ret->priv_data_delete=(onion_handler_private_data_free)onion_handler_static_delete;
	
	return ret;
}

