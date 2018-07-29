#include "common.h"
#include <chibi/eval.h>
#include <stdio.h>

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
 *  - arguments: a group string: "X/X/X"
 *               a state: on = #t, off = #f
 */
static sexp func_knx_send(sexp ctx, sexp self, sexp_sint_t n, 
                              sexp group, sexp state) {
	char url[200];

	if (!sexp_stringp(group) || !sexp_booleanp(state))
		error("knx:send: invalid arguments");
	
	lookup_knx_url(ctx, url);
	knx_send(url, sexp_string_data(group), 
			 sexp_unbox_boolean(state));

	return SEXP_TRUE;
}

/*
 *  func_knx_read: a scheme function wrapper for the knx_read function
 *	- argument: a group string: "X/X/X"
 */
static sexp func_knx_read(sexp ctx, sexp self, sexp_sint_t n, sexp group) {
	char url[200];
	int res;

	if (!sexp_stringp(group))
		error("knx:read: invalid argument");

	lookup_knx_url(ctx, url);

	res = knx_read(url, sexp_string_data(group));

	return sexp_make_boolean(res);
}

/*
 *  func_knx_watch: subscribe functions to knx group events
 *  - argument: an associated list of: groupname:function
 */
static sexp func_knx_watch(sexp ctx, sexp self, sexp_sint_t n, sexp rules) {
	char url[200];
	char group[200];
	int res, i;

	if (!sexp_listp(ctx, rules))
		error("knx:read: invalid argument");

	sexp_gc_var3(ev_group, rulesv, rulec);
	sexp_gc_preserve3(ctx, ev_group, rulesv, rulec);
	rulesv = sexp_list_to_vector(ctx, rules);

	lookup_knx_url(ctx, url);

	while(1) {
		res = knx_watch(url, group);
		ev_group = sexp_c_string(ctx, group, strlen(group));

		for (i = 0; i < sexp_unbox_fixnum(sexp_length(ctx, rules)); i++) {
			rulec = sexp_vector_ref(rulesv, sexp_make_fixnum(i));
			if(sexp_unbox_boolean(sexp_equalp(ctx, ev_group, sexp_car(rulec)))) {
				sexp_apply(ctx, sexp_cdr(rulec),
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

	sexp ctx;
	ctx = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);
	sexp_load_standard_env(ctx, NULL, SEXP_SEVEN);
	sexp_load_standard_ports(ctx, NULL, stdin, stdout, stderr, 1);
	
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
