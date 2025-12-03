/*
 * cmd_ai.c - AI assistant for shell commands (REFACTORED)
 *
 * Makes HTTP requests to OpenAI API to get shell command suggestions
 */

#include "picobox.h"
#include "cmd_spec.h"
#include <curl/curl.h>
#include <json-c/json.h>

#define OPENAI_API_URL "https://api.openai.com/v1/chat/completions"
#define MAX_RESPONSE_SIZE 4096

/*
 * Structure to hold HTTP response data
 */
struct response_buffer {
    char *data;
    size_t size;
};

/*
 * Callback for curl to write response data
 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct response_buffer *mem = (struct response_buffer *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Not enough memory for response\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

/*
 * Make OpenAI API request and return the response
 */
static char *make_openai_request(const char *query)
{
    CURL *curl;
    CURLcode res;
    struct response_buffer response = {0};
    char *api_key;
    char auth_header[512];
    struct curl_slist *headers = NULL;
    char *json_request;
    char *result = NULL;

    /* Get API key from environment */
    api_key = getenv("AI_SHELL");
    if (!api_key) {
        fprintf(stderr, "Error: AI_SHELL environment variable not set\n");
        fprintf(stderr, "Set it with: export AI_SHELL='your-api-key'\n");
        return NULL;
    }

    /* Initialize curl */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }

    /* Build JSON request */
    json_object *root = json_object_new_object();
    json_object *messages = json_object_new_array();
    json_object *system_msg = json_object_new_object();
    json_object *user_msg = json_object_new_object();

    json_object_object_add(system_msg, "role", json_object_new_string("system"));
    json_object_object_add(system_msg, "content",
        json_object_new_string(
            "You are a helpful Unix shell assistant for PicoBox, a BNFC-powered shell implementation.\n\n"
            "Shell Capabilities:\n"
            "- Simple commands: echo hello, ls, pwd, cat file.txt\n"
            "- Pipelines: cat file | grep pattern | wc -l\n"
            "- Redirections: echo test > file.txt, cat < input.txt, cmd >> append.txt\n"
            "- Built-in commands: cd, exit, help, plus 27+ Unix utilities\n"
            "- Command sequences: cmd1 ; cmd2 ; cmd3\n\n"
            "Important Limitations:\n"
            "- When a command in a pipeline has output redirection (>), it breaks the pipe chain\n"
            "  Example: 'ls | grep test > file.txt | wc' - wc gets empty input because grep writes to file\n"
            "- Use full paths for external commands in pipelines for reliability\n"
            "- No background jobs (&), job control, or command substitution yet\n\n"
            "Response Format:\n"
            "- For 'how do I' questions: Provide ONLY the command, no explanation\n"
            "- For 'what is' or 'explain' questions: Brief, friendly explanation\n"
            "- No markdown formatting, no code blocks, just plain text\n"
            "- Be concise and beginner-friendly\n\n"
            "Examples:\n"
            "Q: how do I list all files\n"
            "A: ls -la\n\n"
            "Q: what does grep do\n"
            "A: grep searches for text patterns in files. Use: grep 'pattern' filename"));

    json_object_object_add(user_msg, "role", json_object_new_string("user"));
    json_object_object_add(user_msg, "content", json_object_new_string(query));

    json_object_array_add(messages, system_msg);
    json_object_array_add(messages, user_msg);

    json_object_object_add(root, "model", json_object_new_string("gpt-3.5-turbo"));
    json_object_object_add(root, "messages", messages);
    json_object_object_add(root, "temperature", json_object_new_double(0.3));
    json_object_object_add(root, "max_tokens", json_object_new_int(150));

    json_request = strdup(json_object_to_json_string(root));

    /* Set up headers */
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* Configure curl */
    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    /* Make the request */
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(res));
    } else if (response.data) {
        /* Parse JSON response */
        json_object *response_obj = json_tokener_parse(response.data);
        if (response_obj) {
            /* Check for API error */
            json_object *error_obj = json_object_object_get(response_obj, "error");
            if (error_obj) {
                json_object *error_msg = json_object_object_get(error_obj, "message");
                if (error_msg) {
                    fprintf(stderr, "API Error: %s\n", json_object_get_string(error_msg));
                }
            } else {
                /* Success path */
                json_object *choices = json_object_object_get(response_obj, "choices");
                if (choices && json_object_get_type(choices) == json_type_array) {
                    json_object *first_choice = json_object_array_get_idx(choices, 0);
                    if (first_choice) {
                        json_object *message = json_object_object_get(first_choice, "message");
                        if (message) {
                            json_object *content = json_object_object_get(message, "content");
                            if (content) {
                                result = strdup(json_object_get_string(content));
                            }
                        }
                    }
                }
            }
            json_object_put(response_obj);
        }
    }

    /* Cleanup */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(response.data);
    free(json_request);
    json_object_put(root);

    return result;
}

/*
 * AI command implementation - run function
 */
static int cmd_ai_run(int argc, char **argv)
{
    char query[2048] = "";
    size_t len = 0;
    char *response;
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: AI <question>\n");
        fprintf(stderr, "Example: AI how do I list all files\n");
        return EXIT_ERROR;
    }

    /* Combine all arguments into a single query string */
    for (i = 1; i < argc; i++) {
        if (len > 0 && len < sizeof(query) - 1) {
            query[len++] = ' ';
        }
        size_t word_len = strlen(argv[i]);
        if (len + word_len < sizeof(query) - 1) {
            strcpy(query + len, argv[i]);
            len += word_len;
        }
    }
    query[len] = '\0';

    printf("🤔 Thinking...\n");
    fflush(stdout);

    /* Make API request */
    response = make_openai_request(query);

    if (!response) {
        fprintf(stderr, "Failed to get AI response\n");
        return EXIT_ERROR;
    }

    /* Print the response */
    printf("✨ %s\n", response);
    fflush(stdout);

    free(response);
    return EXIT_OK;
}

/*
 * Print usage information
 */
static void cmd_ai_print_usage(FILE *out)
{
    fprintf(out, "Usage: AI <question>\n");
    fprintf(out, "\n");
    fprintf(out, "Ask the AI assistant for help with shell commands.\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  AI how do I list all files\n");
    fprintf(out, "  AI what command shows disk usage\n");
    fprintf(out, "  AI explain the grep command\n");
    fprintf(out, "\n");
    fprintf(out, "Note: Requires AI_SHELL environment variable to be set with OpenAI API key.\n");
}

/*
 * Command specification
 */
static const cmd_spec_t cmd_ai_spec = {
    .name = "AI",
    .summary = "Ask AI assistant for shell command help",
    .long_help = NULL,
    .run = cmd_ai_run,
    .print_usage = cmd_ai_print_usage
};

/*
 * Registration function (called from main)
 */
#ifdef BUILTIN_ONLY
void register_ai_command(void)
{
    register_command(&cmd_ai_spec);
}
#else
/*
 * Standalone mode - for testing
 */
int main(int argc, char **argv)
{
    return cmd_ai_run(argc, argv);
}
#endif
