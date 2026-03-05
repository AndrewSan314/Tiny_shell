/*
 * ai.c - Native AI Integration via WinHTTP
 * Replaces subprocess calls with direct HTTP API calls
 * Supports: conversation memory, system prompt, streaming output
 */

#include "ai.h"
#include "colors.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winhttp.h>

/*============================================================
 * GLOBALS
 *============================================================*/

static int g_ai_mode_enabled = 0;
static AiConversation g_conversation;

static const char *DEFAULT_SYSTEM_PROMPT =
    "You are the AI assistant built into MSH Shell (a custom Windows terminal). "
    "You help users with: shell commands and scripting, file management and "
    "system tasks, programming and debugging, general knowledge questions. "
    "Keep responses concise and terminal-friendly. "
    "When suggesting commands, format them clearly. "
    "You are running on Windows.";

/*============================================================
 * AI MODE STATE
 *============================================================*/

int ai_is_mode_enabled(void) { return g_ai_mode_enabled; }

void ai_set_mode(int enabled) { g_ai_mode_enabled = enabled ? 1 : 0; }

/*============================================================
 * INITIALIZATION
 *============================================================*/

void ai_init(void) {
  memset(&g_conversation, 0, sizeof(g_conversation));
  strncpy(g_conversation.system_prompt, DEFAULT_SYSTEM_PROMPT,
          AI_SYSTEM_PROMPT_LEN - 1);
  g_conversation.system_prompt[AI_SYSTEM_PROMPT_LEN - 1] = '\0';
  g_conversation.count = 0;
}

void ai_cleanup(void) { memset(&g_conversation, 0, sizeof(g_conversation)); }

void ai_clear_history(void) {
  g_conversation.count = 0;
  memset(g_conversation.messages, 0, sizeof(g_conversation.messages));
}

void ai_set_system_prompt(const char *prompt) {
  if (!prompt || prompt[0] == '\0') {
    strncpy(g_conversation.system_prompt, DEFAULT_SYSTEM_PROMPT,
            AI_SYSTEM_PROMPT_LEN - 1);
  } else {
    strncpy(g_conversation.system_prompt, prompt, AI_SYSTEM_PROMPT_LEN - 1);
  }
  g_conversation.system_prompt[AI_SYSTEM_PROMPT_LEN - 1] = '\0';
}

/*============================================================
 * HELPERS
 *============================================================*/

static int ai_get_stream_delay_ms(void) {
  char value[16];
  DWORD len = GetEnvironmentVariableA("MSH_AI_DELAY_MS", value, sizeof(value));
  int delayMs;

  if (len == 0 || len >= sizeof(value))
    return 6;

  delayMs = atoi(value);
  if (delayMs < 0)
    delayMs = 0;
  if (delayMs > 100)
    delayMs = 100;
  return delayMs;
}

static void ai_print_slow(const char *text, int delayMs) {
  size_t i;

  if (!text)
    return;

  for (i = 0; text[i] != '\0'; i++) {
    putchar(text[i]);
    fflush(stdout);
    if (delayMs <= 0)
      continue;

    if (text[i] == '\n') {
      Sleep(delayMs * 2);
    } else if (text[i] != '\r') {
      Sleep(delayMs);
    }
  }
}

static int trim_env_value(char *value) {
  char *start = value;
  char *end;

  while (*start && isspace((unsigned char)*start))
    start++;
  end = start + strlen(start);
  while (end > start && isspace((unsigned char)*(end - 1)))
    end--;
  *end = '\0';

  if ((end - start) >= 2 &&
      ((start[0] == '"' && end[-1] == '"') ||
       (start[0] == '\'' && end[-1] == '\''))) {
    start++;
    end--;
    *end = '\0';
  }

  if (start != value)
    memmove(value, start, strlen(start) + 1);

  return value[0] != '\0';
}

static int ai_get_api_key(char *out, size_t outLen) {
  DWORD len;

  /* Primary provider key */
  len = GetEnvironmentVariableA("OPENROUTER_API_KEY", out, (DWORD)outLen);
  if (len > 0 && len < outLen && trim_env_value(out))
    return 1;

  /* Backward compatibility with old variables */
  len = GetEnvironmentVariableA("GEMINI_API_KEY", out, (DWORD)outLen);
  if (len > 0 && len < outLen && trim_env_value(out))
    return 1;

  len = GetEnvironmentVariableA("GOOGLE_API_KEY", out, (DWORD)outLen);
  if (len > 0 && len < outLen && trim_env_value(out))
    return 1;

  return 0;
}

static void ai_get_model(char *out, size_t outLen) {
  DWORD len = GetEnvironmentVariableA("MSH_AI_MODEL", out, (DWORD)outLen);
  if (len == 0 || len >= outLen || !trim_env_value(out)) {
    strncpy(out, "z-ai/glm-4.5-air:free", outLen - 1);
    out[outLen - 1] = '\0';
  }
}

static void ai_get_api_path(char *out, size_t outLen) {
  DWORD len = GetEnvironmentVariableA("MSH_AI_URL", out, (DWORD)outLen);
  if (len == 0 || len >= outLen || !trim_env_value(out)) {
    strncpy(out, "/api/v1/chat/completions", outLen - 1);
    out[outLen - 1] = '\0';
  }
}

/*============================================================
 * JSON HELPERS (minimal, no external dependency)
 *============================================================*/

/* Escape a string for JSON: handle \, ", and control chars */
static void json_escape_append(char *dest, size_t destSize, const char *src) {
  size_t pos = strlen(dest);
  size_t i;

  for (i = 0; src[i] != '\0' && pos < destSize - 6; i++) {
    unsigned char c = (unsigned char)src[i];
    if (c == '"') {
      dest[pos++] = '\\';
      dest[pos++] = '"';
    } else if (c == '\\') {
      dest[pos++] = '\\';
      dest[pos++] = '\\';
    } else if (c == '\n') {
      dest[pos++] = '\\';
      dest[pos++] = 'n';
    } else if (c == '\r') {
      dest[pos++] = '\\';
      dest[pos++] = 'r';
    } else if (c == '\t') {
      dest[pos++] = '\\';
      dest[pos++] = 't';
    } else if (c < 0x20) {
      /* Skip other control chars */
    } else {
      dest[pos++] = (char)c;
    }
  }
  dest[pos] = '\0';
}

/* Build JSON request for OpenRouter (OpenAI-compatible chat API) */
static char *ai_build_request_json(const char *model, const char *userPrompt) {
  size_t bufSize = (AI_MAX_MSG_LEN * (g_conversation.count + 2) * 3) + 8192;
  char *json = (char *)calloc(bufSize, 1);
  int i;
  int first = 1;

  if (!json)
    return NULL;

  strcat(json, "{\"model\":\"");
  json_escape_append(json, bufSize, model);
  strcat(json, "\",\"messages\":[");

  if (g_conversation.system_prompt[0] != '\0') {
    strcat(json, "{\"role\":\"system\",\"content\":\"");
    json_escape_append(json, bufSize, g_conversation.system_prompt);
    strcat(json, "\"}");
    first = 0;
  }

  for (i = 0; i < g_conversation.count; i++) {
    const char *role = (g_conversation.messages[i].role == AI_ROLE_USER)
                           ? "user"
                           : "assistant";
    if (!first)
      strcat(json, ",");
    strcat(json, "{\"role\":\"");
    strcat(json, role);
    strcat(json, "\",\"content\":\"");
    json_escape_append(json, bufSize, g_conversation.messages[i].text);
    strcat(json, "\"}");
    first = 0;
  }

  if (!first)
    strcat(json, ",");
  strcat(json, "{\"role\":\"user\",\"content\":\"");
  json_escape_append(json, bufSize, userPrompt);
  strcat(json, "\"}],\"temperature\":0.7}");

  return json;
}

/* Extract assistant text from OpenRouter/OpenAI-style JSON response */
static char *json_extract_response_text(const char *json) {
  /*
   * Preferred: choices[0].message.content
   * Backward-compatible fallback: first "text" after candidates
   */
  const char *choicesPos;
  const char *contentKey;
  const char *candidatesPos;
  const char *textKey;
  const char *start;
  const char *end;
  size_t len;
  char *result;

  if (!json)
    return NULL;

  choicesPos = strstr(json, "\"choices\"");
  if (choicesPos) {
    contentKey = strstr(choicesPos, "\"content\"");
    if (contentKey) {
      start = contentKey + 9; /* length of "content" */
      while (*start && (*start == ':' || *start == ' '))
        start++;
      if (*start == '"') {
        start++;
        goto parse_value;
      }
    }
  }

  candidatesPos = strstr(json, "\"candidates\"");
  if (!candidatesPos)
    return NULL;
  textKey = strstr(candidatesPos, "\"text\"");
  if (!textKey)
    return NULL;

  start = textKey + 6; /* length of "text" */
  while (*start && (*start == ':' || *start == ' '))
    start++;

  if (*start != '"')
    return NULL;
  start++;

parse_value:
  /* Find the closing quote - handle escaped quotes */
  end = start;
  while (*end != '\0') {
    if (*end == '\\') {
      end += 2; /* skip escaped char */
      continue;
    }
    if (*end == '"')
      break;
    end++;
  }

  if (*end != '"')
    return NULL;

  len = (size_t)(end - start);
  result = (char *)malloc(len + 1);
  if (!result)
    return NULL;

  memcpy(result, start, len);
  result[len] = '\0';

  /* Unescape basic sequences */
  {
    char *r = result;
    char *w = result;
    while (*r) {
      if (*r == '\\' && *(r + 1)) {
        r++;
        switch (*r) {
        case 'n':
          *w++ = '\n';
          break;
        case 'r':
          *w++ = '\r';
          break;
        case 't':
          *w++ = '\t';
          break;
        case '"':
          *w++ = '"';
          break;
        case '\\':
          *w++ = '\\';
          break;
        default:
          *w++ = '\\';
          *w++ = *r;
          break;
        }
        r++;
      } else {
        *w++ = *r++;
      }
    }
    *w = '\0';
  }

  return result;
}

/*============================================================
 * CONVERSATION MEMORY
 *============================================================*/

static void ai_add_message(AiRole role, const char *text) {
  if (g_conversation.count >= AI_MAX_HISTORY * 2) {
    /* Shift: remove oldest pair (2 messages) */
    memmove(&g_conversation.messages[0], &g_conversation.messages[2],
            sizeof(AiMessage) * (AI_MAX_HISTORY * 2 - 2));
    g_conversation.count -= 2;
  }

  g_conversation.messages[g_conversation.count].role = role;
  strncpy(g_conversation.messages[g_conversation.count].text, text,
          AI_MAX_MSG_LEN - 1);
  g_conversation.messages[g_conversation.count].text[AI_MAX_MSG_LEN - 1] =
      '\0';
  g_conversation.count++;
}

/*============================================================
 * WINHTTP API CALL
 *============================================================*/

static char *winhttp_post_json(const char *host, const char *path,
                               const char *apiKey, const char *jsonBody) {
  HINTERNET hSession = NULL;
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;
  DWORD bytesRead = 0;
  DWORD totalSize = 0;
  DWORD bufSize = 8192;
  char *response = NULL;
  char chunk[4096];
  wchar_t wHost[256];
  wchar_t wPath[2048];
  char headersA[1024];
  wchar_t wHeaders[1024];
  int headerLen;

  /* Convert to wide strings */
  MultiByteToWideChar(CP_UTF8, 0, host, -1, wHost, 256);
  MultiByteToWideChar(CP_UTF8, 0, path, -1, wPath, 2048);

  hSession = WinHttpOpen(L"MSH-Shell/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                         WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession)
    goto cleanup;

  /* Set timeouts: connect=10s, send=30s, receive=45s */
  WinHttpSetTimeouts(hSession, 10000, 10000, 30000, 45000);

  hConnect = WinHttpConnect(hSession, wHost, INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!hConnect)
    goto cleanup;

  hRequest = WinHttpOpenRequest(hConnect, L"POST", wPath, NULL,
                                WINHTTP_NO_REFERER,
                                WINHTTP_DEFAULT_ACCEPT_TYPES,
                                WINHTTP_FLAG_SECURE);
  if (!hRequest)
    goto cleanup;

  /* Send request */
  {
    DWORD bodyLen = (DWORD)strlen(jsonBody);
    _snprintf(headersA, sizeof(headersA),
              "Content-Type: application/json\r\n"
              "Authorization: Bearer %s\r\n",
              apiKey);
    headersA[sizeof(headersA) - 1] = '\0';
    headerLen = MultiByteToWideChar(CP_UTF8, 0, headersA, -1, wHeaders, 1024);
    if (headerLen <= 0)
      goto cleanup;

    if (!WinHttpSendRequest(hRequest, wHeaders, (DWORD)-1, (LPVOID)jsonBody,
                            bodyLen, bodyLen, 0)) {
      goto cleanup;
    }
  }

  if (!WinHttpReceiveResponse(hRequest, NULL))
    goto cleanup;

  /* Check status code */
  {
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize,
                        WINHTTP_NO_HEADER_INDEX);

    if (statusCode == 429) {
      response = _strdup("[ERROR] HTTP 429: Rate limit exceeded. Wait a moment "
                         "and try again.");
      goto cleanup;
    }
    if (statusCode == 401 || statusCode == 403) {
      response = _strdup("[ERROR] HTTP 401/403: Authentication failed. Check "
                         "your OPENROUTER_API_KEY.");
      goto cleanup;
    }
    if (statusCode < 200 || statusCode >= 300) {
      char errBuf[128];
      snprintf(errBuf, sizeof(errBuf),
               "[ERROR] HTTP %lu: Request failed.", statusCode);
      response = _strdup(errBuf);
      goto cleanup;
    }
  }

  /* Read response body */
  response = (char *)calloc(bufSize, 1);
  if (!response)
    goto cleanup;

  while (WinHttpReadData(hRequest, chunk, sizeof(chunk) - 1, &bytesRead)) {
    if (bytesRead == 0)
      break;

    chunk[bytesRead] = '\0';

    if (totalSize + bytesRead >= bufSize - 1) {
      bufSize *= 2;
      {
        char *newBuf = (char *)realloc(response, bufSize);
        if (!newBuf) {
          free(response);
          response = NULL;
          goto cleanup;
        }
        response = newBuf;
      }
    }

    memcpy(response + totalSize, chunk, bytesRead);
    totalSize += bytesRead;
    response[totalSize] = '\0';
  }

cleanup:
  if (hRequest)
    WinHttpCloseHandle(hRequest);
  if (hConnect)
    WinHttpCloseHandle(hConnect);
  if (hSession)
    WinHttpCloseHandle(hSession);

  return response;
}

/*============================================================
 * CORE AI FUNCTION
 *============================================================*/

int ai_chat_line(const char *prompt) {
  char apiKey[256];
  char model[128];
  char path[512];
  char *requestJson = NULL;
  char *rawResponse = NULL;
  char *responseText = NULL;
  int delayMs;
  int retry;

  if (!prompt || prompt[0] == '\0') {
    print_info("Usage: ai <message>");
    return MSH_CONTINUE;
  }

  /* Handle sub-commands */
  if (_stricmp(prompt, "clear") == 0) {
    ai_clear_history();
    print_success("Conversation history cleared");
    return MSH_CONTINUE;
  }

  if (_strnicmp(prompt, "system ", 7) == 0) {
    const char *newPrompt = prompt + 7;
    while (*newPrompt == ' ')
      newPrompt++;
    if (*newPrompt == '\0') {
      print_info("Current system prompt:");
      printf("  %s\n", g_conversation.system_prompt);
    } else {
      ai_set_system_prompt(newPrompt);
      print_success("System prompt updated");
    }
    return MSH_CONTINUE;
  }

  if (_stricmp(prompt, "history") == 0) {
    if (g_conversation.count == 0) {
      print_info("No conversation history");
    } else {
      int i;
      set_color(CLR_HEADER);
      printf("  [CONVERSATION HISTORY] (%d messages)\n", g_conversation.count);
      reset_color();
      for (i = 0; i < g_conversation.count; i++) {
        const char *role =
            (g_conversation.messages[i].role == AI_ROLE_USER) ? "YOU" : "AI";
        set_color(g_conversation.messages[i].role == AI_ROLE_USER ? CLR_PROMPT
                                                                  : CLR_SUCCESS);
        printf("  [%s] ", role);
        reset_color();
        /* Print first 80 chars */
        printf("%.80s%s\n", g_conversation.messages[i].text,
               strlen(g_conversation.messages[i].text) > 80 ? "..." : "");
      }
    }
    return MSH_CONTINUE;
  }

  /* Check API key */
  if (!ai_get_api_key(apiKey, sizeof(apiKey))) {
    print_warning("OPENROUTER_API_KEY is not set");
    print_info("Use: export OPENROUTER_API_KEY=<your_key>");
    return MSH_CONTINUE;
  }

  ai_get_model(model, sizeof(model));
  ai_get_api_path(path, sizeof(path));

  /* Build JSON body with conversation history */
  requestJson = ai_build_request_json(model, prompt);
  if (!requestJson) {
    print_error("Failed to build AI request");
    return MSH_CONTINUE;
  }

  set_color(CLR_HEADER);
  printf("  [AI RESPONSE]\n");
  reset_color();

  /* API call with auto-retry for rate limiting (429) */
  {
    int maxRetries = 3;
    int backoffSec = 3; /* start at 3 seconds, doubles each retry */

    for (retry = 0; retry <= maxRetries; retry++) {
      if (rawResponse) {
        free(rawResponse);
        rawResponse = NULL;
      }

      rawResponse = winhttp_post_json("openrouter.ai", path, apiKey, requestJson);

      if (!rawResponse) {
        /* Network failure - retry once */
        if (retry == 0) {
          set_color(CLR_WARNING);
          printf("  Connection failed, retrying...\n");
          reset_color();
          Sleep(1000);
          continue;
        }
        break;
      }

      /* Check if it's a 429 rate limit error */
      if (strstr(rawResponse, "[ERROR] HTTP 429") != NULL) {
        if (retry < maxRetries) {
          int i;
          set_color(CLR_WARNING);
          printf("  Rate limited. Auto-retry in ");
          for (i = backoffSec; i > 0; i--) {
            printf("%d...", i);
            fflush(stdout);
            Sleep(1000);
          }
          printf("\n");
          reset_color();
          backoffSec *= 2; /* exponential backoff */
          continue;
        }
        /* All retries exhausted */
        break;
      }

      /* Not a 429 error - either success or other error, stop retrying */
      break;
    }
  }

  free(requestJson);

  if (!rawResponse) {
    print_error("Failed to connect to AI service. Check your internet.");
    return MSH_CONTINUE;
  }

  /* Check for error responses */
  if (strncmp(rawResponse, "[ERROR]", 7) == 0) {
    printf("  ");
    print_warning(rawResponse + 8);
    free(rawResponse);
    return MSH_CONTINUE;
  }

  /* Parse response */
  responseText = json_extract_response_text(rawResponse);
  free(rawResponse);

  if (!responseText || responseText[0] == '\0') {
    print_warning("AI returned empty response");
    if (responseText)
      free(responseText);
    return MSH_CONTINUE;
  }

  /* Print response with typing effect */
  delayMs = ai_get_stream_delay_ms();
  printf("  ");
  ai_print_slow(responseText, delayMs);

  /* Ensure newline at end */
  {
    size_t len = strlen(responseText);
    if (len > 0 && responseText[len - 1] != '\n')
      printf("\n");
  }

  /* Save to conversation memory */
  ai_add_message(AI_ROLE_USER, prompt);
  ai_add_message(AI_ROLE_MODEL, responseText);

  free(responseText);
  return MSH_CONTINUE;
}

/*============================================================
 * BUILTIN COMMANDS
 *============================================================*/

int msh_ai(char **args) {
  char prompt[MAX_CMD_LEN * 2];
  int i;

  prompt[0] = '\0';

  if (args[1] == NULL) {
    print_info("Usage: ai <message>");
    print_info("       ai clear     - Clear conversation history");
    print_info("       ai history   - Show conversation history");
    print_info("       ai system <prompt> - Set custom system prompt");
    return MSH_CONTINUE;
  }

  for (i = 1; args[i] != NULL; i++) {
    if (i > 1) {
      strncat(prompt, " ", sizeof(prompt) - strlen(prompt) - 1);
    }
    strncat(prompt, args[i], sizeof(prompt) - strlen(prompt) - 1);
  }

  return ai_chat_line(prompt);
}

int msh_aimode(char **args) {
  if (args[1] == NULL || _stricmp(args[1], "status") == 0) {
    if (ai_is_mode_enabled())
      print_success("AI mode is ON");
    else
      print_info("AI mode is OFF");

    print_info("Usage: aimode <on|off|status>");
    print_info("In AI mode: type text to chat, prefix shell commands with '!'");
    return MSH_CONTINUE;
  }

  if (_stricmp(args[1], "on") == 0) {
    ai_set_mode(1);
    print_success("AI mode enabled");
    if (!ai_get_api_key((char[256]){0}, 256)) {
      print_warning("OPENROUTER_API_KEY is not set yet");
      print_info("Use: export OPENROUTER_API_KEY=<your_key>");
    }
    print_info("Type normal text to chat. Use !<command> for shell commands.");
    return MSH_CONTINUE;
  }

  if (_stricmp(args[1], "off") == 0) {
    ai_set_mode(0);
    print_success("AI mode disabled");
    return MSH_CONTINUE;
  }

  print_info("Usage: aimode <on|off|status>");
  return MSH_CONTINUE;
}
