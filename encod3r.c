#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>
#include <glib/gprintf.h>

// Base64 implementation (simple, no external lib needed)
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const char* input) {
    size_t len = strlen(input);
    size_t out_len = 4 * ((len + 2) / 3);
    char* output = malloc(out_len + 1);
    if (!output) return NULL;

    size_t i, j = 0;
    for (i = 0; i < len; i += 3) {
        unsigned int val = 0;
        val |= (unsigned char)input[i] << 16;
        if (i + 1 < len) val |= (unsigned char)input[i + 1] << 8;
        if (i + 2 < len) val |= (unsigned char)input[i + 2];

        output[j++] = b64_table[(val >> 18) & 0x3F];
        output[j++] = b64_table[(val >> 12) & 0x3F];
        output[j++] = (i + 1 < len) ? b64_table[(val >> 6) & 0x3F] : '=';
        output[j++] = (i + 2 < len) ? b64_table[val & 0x3F] : '=';
    }
    output[j] = '\0';
    return output;
}

// URL encode (basic, handles common chars)
char* url_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        unsigned char c = *input;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            g_string_append_c(result, c);
        } else {
            g_string_append_printf(result, "%%%02X", c);
        }
    }
    return g_string_free(result, FALSE);
}

char* double_url_encode(const char* input) {
    char* once = url_encode(input);
    char* twice = url_encode(once);
    free(once);
    return twice;
}

char* html_escape(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        switch (*input) {
            case '&':  g_string_append(result, "&amp;"); break;
            case '<':  g_string_append(result, "&lt;"); break;
            case '>':  g_string_append(result, "&gt;"); break;
            case '"':  g_string_append(result, "&quot;"); break;
            case '\'': g_string_append(result, "&#x27;"); break;
            default:   g_string_append_c(result, *input);
        }
    }
    return g_string_free(result, FALSE);
}

char* html_hex_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        g_string_append_printf(result, "&#x%X;", (unsigned char)*input);
    }
    return g_string_free(result, FALSE);
}

char* html_dec_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        g_string_append_printf(result, "&#%d;", (unsigned char)*input);
    }
    return g_string_free(result, FALSE);
}

char* hex_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        g_string_append_printf(result, "\\x%02x", (unsigned char)*input);
    }
    return g_string_free(result, FALSE);
}

char* octal_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        g_string_append_printf(result, "\\%o", (unsigned char)*input);
    }
    return g_string_free(result, FALSE);
}

char* rot13(const char* input) {
    char* output = strdup(input);
    for (char* p = output; *p; p++) {
        if ('A' <= *p && *p <= 'Z') *p = ((*p - 'A' + 13) % 26) + 'A';
        else if ('a' <= *p && *p <= 'z') *p = ((*p - 'a' + 13) % 26) + 'a';
    }
    return output;
}

char* mixed_case(const char* input) {
    char* output = strdup(input);
    for (int i = 0; output[i]; i++) {
        output[i] = (i % 2 == 0) ? tolower(output[i]) : toupper(output[i]);
    }
    return output;
}

char* unicode_encode(const char* input) {
    GString* result = g_string_new("");
    for (; *input; input++) {
        g_string_append_printf(result, "\\u%04x", (unsigned char)*input);
    }
    return g_string_free(result, FALSE);
}

char* encode_string(const char* input, const char* method) {
    if (strcmp(method, "url") == 0) return url_encode(input);
    if (strcmp(method, "double_url") == 0) return double_url_encode(input);
    if (strcmp(method, "html") == 0) return html_escape(input);
    if (strcmp(method, "html_hex") == 0) return html_hex_encode(input);
    if (strcmp(method, "html_dec") == 0) return html_dec_encode(input);
    if (strcmp(method, "base64") == 0) return base64_encode(input);
    if (strcmp(method, "hex") == 0) return hex_encode(input);
    if (strcmp(method, "octal") == 0) return octal_encode(input);
    if (strcmp(method, "rot13") == 0) return rot13(input);
    if (strcmp(method, "mixed_case") == 0) return mixed_case(input);
    if (strcmp(method, "unicode") == 0) return unicode_encode(input);

    return strdup("Unknown encoding method.");
}

// Callback for Encode button
void on_encode_clicked(GtkWidget *widget, gpointer data) {
    GtkEntry *entry = GTK_ENTRY(data);
    GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(g_object_get_data(G_OBJECT(widget), "combo"));
    GtkTextView *output_view = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(widget), "output"));

    const char* input = gtk_entry_get_text(entry);
    const char* method = gtk_combo_box_text_get_active_text(combo);

    char* result = encode_string(input, method);

    // Update output
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(output_view);
    gtk_text_buffer_set_text(buffer, result, -1);

    // Copy to clipboard using xclip
    gchar *command;
    g_shell_parse_argv("xclip -selection clipboard", NULL, &command, NULL);
    FILE *pipe = popen(command, "w");
    if (pipe) {
        fwrite(result, 1, strlen(result), pipe);
        pclose(pipe);
    }
    g_strfreev(command);

    free(result);
    g_free((gchar*)method);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Encod3r");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Input
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Input String:"), FALSE, FALSE, 0);
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Method selector
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Encoding Method:"), FALSE, FALSE, 0);
    GtkWidget *combo = gtk_combo_box_text_new();
    const char* methods[] = {"url", "double_url", "html", "html_hex", "html_dec",
                             "base64", "hex", "octal", "rot13", "mixed_case", "unicode", NULL};
    for (int i = 0; methods[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), methods[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);

    // Encode button
    GtkWidget *button = gtk_button_new_with_label("Encode");
    g_object_set_data(G_OBJECT(button), "combo", combo);
    g_object_set_data(G_OBJECT(button), "output", NULL); // will set later
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    // Output
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Encoded Output:"), FALSE, FALSE, 0);
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled), output_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    // Connect button
    g_object_set_data(G_OBJECT(button), "output", output_view);
    g_signal_connect(button, "clicked", G_CALLBACK(on_encode_clicked), entry);

    // Allow Enter key to trigger encode
    g_signal_connect(entry, "activate", G_CALLBACK(on_encode_clicked), entry);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}