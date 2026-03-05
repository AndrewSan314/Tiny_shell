#include "ai.h"
#include "colors.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_ai_mode_enabled = 0;

int ai_is_mode_enabled(void) { return g_ai_mode_enabled; }

void ai_set_mode(int enabled) { g_ai_mode_enabled = enabled ? 1 : 0; }

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

static int env_var_has_nonempty_value(const char *name) {
  DWORD needed;
  char *value;
  char *start;
  char *end;
  int hasValue = 0;

  needed = GetEnvironmentVariableA(name, NULL, 0);
  if (needed <= 1)
    return 0;

  value = (char *)malloc(needed);
  if (!value)
    return 0;

  if (GetEnvironmentVariableA(name, value, needed) == 0) {
    free(value);
    return 0;
  }

  start = value;
  while (*start && isspace((unsigned char)*start))
    start++;

  end = start + strlen(start);
  while (end > start && isspace((unsigned char)*(end - 1)))
    *(--end) = '\0';

  if ((end - start) >= 2 &&
      ((start[0] == '"' && end[-1] == '"') ||
       (start[0] == '\'' && end[-1] == '\''))) {
    start++;
    end--;
    *end = '\0';
  }

  if (*start != '\0')
    hasValue = 1;

  free(value);
  return hasValue;
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

static int ai_has_api_key(void) {
  if (env_var_has_nonempty_value("GEMINI_API_KEY"))
    return 1;
  return env_var_has_nonempty_value("GOOGLE_API_KEY");
}

int ai_chat_line(const char *prompt) {
  FILE *pipe;
  char output[1024];
  int delayMs;
  int gotOutput = 0;
  int endsWithNewline = 1;
  int hitRateLimit = 0;
  int hitAuthError = 0;
  int statusCode;
  const char *psCommand =
      "powershell -NoProfile -ExecutionPolicy Bypass -Command "
      "\"$ErrorActionPreference='Stop'; "
      "try { "
      "$model=if($env:MSH_AI_MODEL){$env:MSH_AI_MODEL}else{'gemini-2.0-flash'}; "
      "$apiKey=if($env:GEMINI_API_KEY){$env:GEMINI_API_KEY}else{$env:GOOGLE_API_KEY}; "
      "$apiKey=$apiKey.Trim(); "
      "if($apiKey.StartsWith('\"') -and $apiKey.EndsWith('\"') -and $apiKey.Length -ge 2){$apiKey=$apiKey.Substring(1,$apiKey.Length-2)}; "
      "if($apiKey.StartsWith(\"'\") -and $apiKey.EndsWith(\"'\") -and $apiKey.Length -ge 2){$apiKey=$apiKey.Substring(1,$apiKey.Length-2)}; "
      "if([string]::IsNullOrWhiteSpace($apiKey)){throw 'GEMINI_API_KEY/GOOGLE_API_KEY is empty.'}; "
      "$base=if($env:MSH_AI_URL){$env:MSH_AI_URL}else{'https://generativelanguage.googleapis.com/v1beta/models'}; "
      "$uri=($base.TrimEnd('/') + '/' + $model + ':generateContent?key=' + $apiKey); "
      "$body=@{ contents=@(@{ parts=@(@{ text=$env:MSH_AI_PROMPT }) }) } | ConvertTo-Json -Depth 8 -Compress; "
      "$resp=Invoke-RestMethod -Method Post -Uri $uri -ContentType 'application/json' -Body $body -TimeoutSec 45; "
      "$parts=@(); "
      "if($resp.candidates -and $resp.candidates[0].content -and $resp.candidates[0].content.parts){$parts=$resp.candidates[0].content.parts}; "
      "$text=(($parts | ForEach-Object { $_.text }) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }) -join [Environment]::NewLine; "
      "if([string]::IsNullOrWhiteSpace($text)){Write-Output '[WARN] AI returned empty response.'}else{Write-Output $text}; "
      "} catch { "
      "$msg=$_.Exception.Message; "
      "$statusCode=''; "
      "if($_.Exception -and $_.Exception.Response -and $_.Exception.Response.StatusCode){$statusCode=[int]$_.Exception.Response.StatusCode}; "
      "if([string]::IsNullOrWhiteSpace($msg)){$msg='Unknown AI request error.'}; "
      "if($statusCode){Write-Output ('[ERROR] HTTP ' + $statusCode + ': ' + $msg)}else{Write-Output ('[ERROR] ' + $msg)}; "
      "exit 1 "
      "}\" 2>&1";

  if (!prompt || prompt[0] == '\0') {
    print_info("Usage: ai <message>");
    return MSH_CONTINUE;
  }

  if (!ai_has_api_key()) {
    print_warning("GEMINI_API_KEY is not set");
    print_info("Use: export GEMINI_API_KEY=<your_key>");
    return MSH_CONTINUE;
  }

  SetEnvironmentVariableA("MSH_AI_PROMPT", prompt);
  pipe = _popen(psCommand, "r");
  if (!pipe) {
    print_error("Failed to start PowerShell for AI request");
    SetEnvironmentVariableA("MSH_AI_PROMPT", NULL);
    return MSH_CONTINUE;
  }

  set_color(CLR_HEADER);
  printf("  [AI RESPONSE]\n");
  reset_color();

  delayMs = ai_get_stream_delay_ms();
  while (fgets(output, sizeof(output), pipe)) {
    size_t lineLen;
    gotOutput = 1;

    if (strstr(output, "HTTP 429") || strstr(output, "(429)") ||
        strstr(output, "Too Many Requests")) {
      hitRateLimit = 1;
    }
    if (strstr(output, "HTTP 401") || strstr(output, "HTTP 403") ||
        strstr(output, "(401)") || strstr(output, "(403)") ||
        strstr(output, "Forbidden") || strstr(output, "Unauthorized")) {
      hitAuthError = 1;
    }

    printf("  ");
    ai_print_slow(output, delayMs);
    lineLen = strlen(output);
    endsWithNewline = (lineLen > 0 && output[lineLen - 1] == '\n');
  }

  statusCode = _pclose(pipe);
  SetEnvironmentVariableA("MSH_AI_PROMPT", NULL);

  if (!gotOutput) {
    print_warning("No output received from AI");
  } else if (!endsWithNewline) {
    printf("\n");
  }

  if (statusCode != 0) {
    if (hitRateLimit) {
      print_warning(
          "AI rate limit reached (HTTP 429). Wait a bit or use another key/model.");
    } else if (hitAuthError) {
      print_warning(
          "AI authentication failed (HTTP 401/403). Re-check API key validity.");
    } else {
      print_warning("AI request failed. Check API key, model, and network.");
    }
  }

  return MSH_CONTINUE;
}

int msh_ai(char **args) {
  char prompt[MAX_CMD_LEN * 2];
  int i;

  prompt[0] = '\0';

  if (args[1] == NULL) {
    print_info("Usage: ai <message>");
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
    if (!ai_has_api_key()) {
      print_warning("GEMINI_API_KEY is not set yet");
      print_info("Use: export GEMINI_API_KEY=<your_key>");
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
