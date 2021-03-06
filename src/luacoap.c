#include <smcp/smcp.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <luacoap/client.h>

#define CLIENT_MT_NAME "coap_client"
#define COAP_METHOD_OBSERVE 5

typedef struct lcoap_userdata { smcp_t smcp; } lcoap_userdata;

static int coap_create_client(lua_State *L) {
  lcoap_userdata *cud;
  cud = (lcoap_userdata *)lua_newuserdata(L, sizeof(lcoap_userdata));
  luaL_getmetatable(L, CLIENT_MT_NAME);
  lua_setmetatable(L, -2);

  // Creates the client
  cud->smcp = smcp_create(61616);
  return 1;
}

static int coap_client_gc(lua_State *L) {
  int stack = 1;
  lcoap_userdata *cud =
      (lcoap_userdata *)luaL_checkudata(L, stack, CLIENT_MT_NAME);
  luaL_argcheck(L, cud, stack, "Server/Client expected");
  if (cud != NULL) {
    free(cud->smcp);
  }
  return 0;
}

static int coap_client_send_request(coap_code_t method, lua_State *L) {
  coap_transaction_type_t tt = COAP_TRANS_TYPE_CONFIRMABLE;
  coap_content_type_t ct = COAP_CONTENT_TYPE_TEXT_PLAIN;

  // Get the coap client
  int stack = 1;
  lcoap_userdata *cud =
      (lcoap_userdata *)luaL_checkudata(L, stack, CLIENT_MT_NAME);
  luaL_argcheck(L, cud, stack, "Client expected");
  if (cud == NULL) {
    return luaL_error(L, "First argument is not of class Client");
  }
  stack++;

  // Get transaction type
  if (lua_isnumber(L, stack)) {
    tt = lua_tointeger(L, stack);
    stack++;

    if ((tt != COAP_TRANS_TYPE_CONFIRMABLE) &&
        tt != (COAP_TRANS_TYPE_NONCONFIRMABLE)) {
      return luaL_error(L,
                        "Invalid transaction type, use coap.CON or coap.NON");
    }
  }

  // Get the url
  size_t ln;
  const char *url = luaL_checklstring(L, stack, &ln);
  stack++;

  if (url == NULL) return luaL_error(L, "Invalid URL");

  size_t payload_len;
  const char *payload = NULL;

  // Optional content type and payload
  if (lua_isnumber(L, stack)) {
    ct = lua_tointeger(L, stack);
    stack++;

    // get the payload
    size_t payload_len;
    const char *payload = luaL_checklstring(L, stack, &payload_len);
    stack++;
  }

  char return_content[2048];
  size_t return_content_size = 0;

  if (method == COAP_METHOD_OBSERVE) {
    if (send_request(cud->smcp, COAP_METHOD_GET, tt, url, ct, payload,
                     payload_len, true, &return_content[0],
                     &return_content_size) != 0) {
      luaL_error(L, "Error sending request");
    }
  } else {
    if (send_request(cud->smcp, method, tt, url, ct, payload, payload_len,
                     false, &return_content[0], &return_content_size) != 0) {
      luaL_error(L, "Error sending request");
    }
  }

  if (return_content_size > 0) {
    lua_pushlstring(L, return_content, return_content_size);
    return 1;
  }

  return 0;
}

static int coap_client_get(lua_State *L) {
  return coap_client_send_request(COAP_METHOD_GET, L);
}
static int coap_client_put(lua_State *L) {
  return coap_client_send_request(COAP_METHOD_PUT, L);
}
static int coap_client_post(lua_State *L) {
  return coap_client_send_request(COAP_METHOD_POST, L);
}
static int coap_client_observe(lua_State *L) {
  return coap_client_send_request(COAP_METHOD_OBSERVE, L);
}

static const struct luaL_Reg luacoap_client_map[] = {
    {"get", coap_client_get},   {"put", coap_client_put},
    {"post", coap_client_post}, {"observe", coap_client_observe},
    {"__gc", coap_client_gc},   {NULL, NULL}};

static const struct luaL_Reg luacoap_map[] = {{"Client", coap_create_client},
                                              {NULL, NULL}};

int luaopen_coap(lua_State *L) {
  // Declare the client metatable
  luaL_newmetatable(L, CLIENT_MT_NAME);
  luaL_setfuncs(L, luacoap_client_map, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -1, "__index");

  // Register the coap library
  luaL_newlib(L, luacoap_map);
  luaL_setfuncs(L, luacoap_map, 0);
  lua_pushnumber(L, COAP_TRANS_TYPE_CONFIRMABLE);
  lua_setfield(L, -2, "CON");
  lua_pushnumber(L, COAP_TRANS_TYPE_NONCONFIRMABLE);
  lua_setfield(L, -2, "NON");

  return 1;
}
