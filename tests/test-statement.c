#include <glib.h>
#include "ghbci/ghbci-statement.h"
#include "ghbci/ghbci-statement-private.h"

static void
test_remove_newlines(void)
{
    gchar* str1 = g_strdup("hello world\nfoo \nbar");
    ghbci_statement_remove_newlines(str1);
    g_assert_cmpstr(str1, ==, "hello worldfoo bar");
    g_free(str1);

    gchar* str2 = g_strdup("\nhello\nworld\n\nfoo\nbar\n\n");
    ghbci_statement_remove_newlines(str2);
    g_assert_cmpstr(str2, ==, "helloworldfoobar");
    g_free(str2);
}

static void
test_prettify_diba(void)
{
    GHbciStatement* statement = g_object_new(GHBCI_TYPE_STATEMENT,
            "reference", "EREF+0003920452\n"
                         "MREF+000000002943\n"
                         "CRED+DE18ZZZ00000180162\n"
                         "SVWZ+Brot fuer die Welt-Vie\n"
                         "len Dank fuerIhre Spende",
            NULL);

    ghbci_statement_prettify_statement(G_OBJECT(statement));

    gchar* reference;
    gchar* eref;
    gchar* mref;
    gchar* cred;
    g_object_get(statement,
                 "reference", &reference,
                 "eref", &eref,
                 "mref", &mref,
                 "cred", &cred,
                 NULL);
    g_assert_cmpstr(reference, ==, "Brot fuer die Welt-Vielen Dank fuerIhre Spende");
    g_assert_cmpstr(eref, ==, "0003920452");
    g_assert_cmpstr(mref, ==, "000000002943");
    g_assert_cmpstr(cred, ==, "DE18ZZZ00000180162");

    g_object_unref(statement);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/statement/remove-new-lines", test_remove_newlines);
    g_test_add_func ("/statement/prettify-diba", test_prettify_diba);
    return g_test_run ();
}


//vim: expandtab sw=4
