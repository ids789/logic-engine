#include "common.h"
#include <chibi/eval.h>
#include <stdio.h>

/*
 *  func_def_item: create a new physical device item
 *  - arguments: a name symbol
 *               a knx address string
 */
static sexp func_def_item(sexp ctx, sexp self, sexp_sint_t n, 
                          sexp name, sexp addr) {
	item new;
	
	sexp_gc_var1(sname);
	sexp_gc_preserve1(ctx, sname);
	sname = sexp_symbol_to_string(ctx, name);

	new.name = malloc(sexp_unbox_fixnum(sexp_string_size(sname)));
	strcpy(new.name, sexp_string_data(sname));
	new.address = knx_convert_gaddr(sexp_string_data(addr));
	add_item(new);

	// link the item's scheme symbol to its registry location
	sexp_env_define(ctx, sexp_context_env(ctx), name, 
	                sexp_make_fixnum(item_registry_size-1));

	sexp_gc_release1(ctx);
	return SEXP_TRUE;
}

/*
 *  lookup_knx_url: get the url of the knxd server
 *  - the scheme script should save it to the 'knx-url' symbol
 */
void lookup_knx_url(sexp ctx, char* url) {
	sexp_gc_var2(url_str, url_sym);
	sexp_gc_preserve2(ctx, url_str, url_sym);
	
	url_sym = sexp_intern(ctx, "knx-url", 7);
	url_str = sexp_env_ref(ctx, sexp_context_env(ctx),
		    				url_sym, sexp_make_boolean(0));

	if (!sexp_unbox_boolean(url))
		error("Error: knxd url is not set");

	strcpy(url, sexp_string_data(url_str));

	sexp_gc_release2(ctx);
}

/*
 *  func_knx_send: a scheme function wrapper for the knx_send function
 *  - arguments: a group symbol
 *               a state: on = #t, off = #f
 */
static sexp func_knx_send(sexp ctx, sexp self, sexp_sint_t n, 
                              sexp dest, sexp state) {
	char url[200];
	item dest_item;

	if (!sexp_numberp(dest) || !sexp_booleanp(state))
		error("knx:send: invalid arguments");

	dest_item = get_item(sexp_unbox_fixnum(dest));
	lookup_knx_url(ctx, url);

	knx_send(url, dest_item.address, sexp_unbox_boolean(state));

	return SEXP_TRUE;
}

/*
 *  func_knx_read: a scheme function wrapper for the knx_read function
 *	- argument: a group symbol
 */
static sexp func_knx_read(sexp ctx, sexp self, sexp_sint_t n, sexp dest) {
	char url[200];
	int res;
	item dest_item;

	if (!sexp_numberp(dest))
		error("knx:read: invalid argument");

	dest_item = get_item(sexp_unbox_fixnum(dest));
	lookup_knx_url(ctx, url);

	res = knx_read(url, dest_item.address);

	return sexp_make_boolean(res);
}

/*
 *  func_knx_watch: subscribe functions to knx group events
 *  - argument: an associated list of: groupname:function
 */
static sexp func_knx_watch(sexp ctx, sexp self, sexp_sint_t n, sexp rules) {
	char url[200];
	int pos, res, i;
	eibaddr_t group;

	if (!sexp_listp(ctx, rules))
		error("knx:read: invalid argument");

	sexp_gc_var2(rule, rulesv);
	sexp_gc_preserve2(ctx, rule, rulesv);
	rulesv = sexp_list_to_vector(ctx, rules);

	lookup_knx_url(ctx, url);

	while(1) {
		knx_watch(url, &group, &res);
		pos = lookup_item(group);

		// unknown group recieved
		if(res == -1)
			continue;

		for (i = 0; i < sexp_unbox_fixnum(sexp_length(ctx, rules)); i++) {
			rule = sexp_vector_ref(rulesv, sexp_make_fixnum(i));
			if(sexp_unbox_boolean(sexp_equalp(ctx, sexp_make_fixnum(pos), 
			                                       sexp_car(rule)))) {
				sexp_apply(ctx, sexp_cdr(rule),
				           sexp_list1(ctx, sexp_make_fixnum(res)));
			}
		}
	}

	sexp_gc_release2(ctx);
	return SEXP_TRUE;
}

int main(int argc, char** argv) {

	if (argc != 2)
		error("usage: %s script", argv[0]);

	item_registry = NULL;
	item_registry_size = 0;

	sexp ctx;
	ctx = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);
	sexp_load_standard_env(ctx, NULL, SEXP_SEVEN);
	sexp_load_standard_ports(ctx, NULL, stdin, stdout, stderr, 1);
	
	sexp_define_foreign(ctx, sexp_context_env(ctx),
						"define-item", 2, (sexp_proc1)func_def_item);

	sexp_define_foreign(ctx, sexp_context_env(ctx),
						"knx:send", 2, (sexp_proc1)func_knx_send);

	sexp_define_foreign(ctx, sexp_context_env(ctx),
						"knx:read", 1, (sexp_proc1)func_knx_read);

	sexp_define_foreign(ctx, sexp_context_env(ctx),
						"knx:watch", 1, (sexp_proc1)func_knx_watch);

	// load the supplied scheme script
	sexp_gc_var1(script);
	sexp_gc_preserve1(ctx, script);
	script = sexp_c_string(ctx, argv[1], -1);
	sexp_load(ctx, script, NULL);

	sexp_gc_release1(ctx);
	sexp_destroy_context(ctx);
}
