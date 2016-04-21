#include <nginx.h>
#include <ngx_http.h>
#include <ngx_http_variables.h>
#include "ngx_http_txid120_logic.h"

/* format spec / base64 encode breakdown
c = seconds (used starting in year 2106), s = seconds (used now/soon), u = microseconds, r = random
val cccsssssssssssssssssssssssssssssssssuuuuuuuuuuuuuuuuuuuurrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
bin 0.......1.......2.......3.......4.......5.......6.......7.......8.......9.......0.......1.......2.......3.......4.......
b64 0.....1.....2.....3.....0.....1.....2.....3.....0.....1.....2.....3.....0.....1.....2.....3.....0.....1.....2.....3.....
blk 0.......................1.......................2.......................3.......................4.......................
*/

static FILE* urandom = NULL;

static ngx_int_t ngx_http_txid120_gen(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) {
  u_char *txid120 = ngx_pnalloc(r->pool, 20);
  if (txid120 == NULL) {
    v->valid = 0;
    v->not_found = 1;
    return NGX_ERROR;
  }

  ngx_http_txid120_logic(urandom, txid120);

  v->len = 20;
  v->data = txid120;
  v->valid = 1;
  v->not_found = 0;
  v->no_cacheable = 0;

  return NGX_OK;
}

static ngx_int_t ngx_http_txid120_init_module(ngx_cycle_t *cycle) {
  urandom = fopen("/dev/urandom", "r");
  if (urandom == NULL) {
    ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "could not open /dev/urandom device for \"$txid120\"");
    return NGX_ERROR;
  }

  ngx_log_error(NGX_LOG_DEBUG, cycle->log, 0, "opened /dev/urandom %d for pid %d", urandom, ngx_pid);

  return NGX_OK;
}

static ngx_str_t ngx_http_txid120_variable_name = ngx_string("txid120");

static ngx_int_t ngx_http_txid120_preconf(ngx_conf_t *cf) {
  ngx_http_variable_t* var = ngx_http_add_variable(cf, &ngx_http_txid120_variable_name, NGX_HTTP_VAR_NOHASH);

  if (var == NULL) {
      return NGX_ERROR;
  }

  var->get_handler = ngx_http_txid120_gen;

  return NGX_OK;
}

static ngx_http_module_t ngx_http_txid120_module_ctx = {
  ngx_http_txid120_preconf, /* preconfiguration */
  NULL, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  NULL, /* create location configuration */
  NULL /* merge location configuration */
};

static ngx_command_t ngx_http_txid120_module_commands[] = {
  ngx_null_command
};

ngx_module_t ngx_http_txid120_module = {
  NGX_MODULE_V1,
  &ngx_http_txid120_module_ctx, /* module context */
  ngx_http_txid120_module_commands, /* module directives */
  NGX_HTTP_MODULE, /* module type */
  NULL, /* init master */
  ngx_http_txid120_init_module, /* init module */
  NULL, /* init process */
  NULL, /* init thread */
  NULL, /* exit thread */
  NULL, /* exit process */
  NULL, /* exit master */
  NGX_MODULE_V1_PADDING
};
