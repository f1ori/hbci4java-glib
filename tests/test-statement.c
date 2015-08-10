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
            "reference", "EREF+42       \n"
                         "              \n"
                         "MREF+C5D043E1A2C847988DF9F3\n"
                         "5F005785EB    \n"
                         "CRED+DE05ZZZ00000205131    \n"
                         "              \n"
                         "SVWZ+1.840000 Max Musterman\n"
                         "n Lastschrift Miete\n",
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
    g_assert_cmpstr(reference, ==, "1.840000 Max Mustermann Lastschrift Miete");
    g_assert_cmpstr(eref, ==, "42");
    g_assert_cmpstr(mref, ==, "C5D043E1A2C847988DF9F35F005785EB");
    g_assert_cmpstr(cred, ==, "DE05ZZZ00000205131");

    g_object_unref(statement);
}

static void
test_prettify_volksbank(void)
{
    GHbciStatement* statement = g_object_new(GHBCI_TYPE_STATEMENT,
            "reference", "Brot fuer die Welt-Vielen D\n"
                         "ank fuer Ihre Spende EREF: \n"
                         "0002958342 MREF: 0000000253\n"
                         "74 CRED: DE18ZZZ00000180162\n"
                         " IBAN: DE093702050000041084\n"
                         "05 BIC: BFSWDE33\n",
            NULL);

    ghbci_statement_prettify_statement(G_OBJECT(statement));

    gchar* reference;
    gchar* other_iban;
    gchar* other_bic;
    gchar* eref;
    gchar* mref;
    gchar* cred;
    g_object_get(statement,
                 "reference", &reference,
                 "other-iban", &other_iban,
                 "other-bic", &other_bic,
                 "eref", &eref,
                 "mref", &mref,
                 "cred", &cred,
                 NULL);
    g_assert_cmpstr(reference, ==, "Brot fuer die Welt-Vielen Dank fuer Ihre Spende");
    g_assert_cmpstr(other_iban, ==, "DE09370205000004108405");
    g_assert_cmpstr(other_bic, ==, "BFSWDE33");
    g_assert_cmpstr(eref, ==, "0002958342");
    g_assert_cmpstr(mref, ==, "000000025374");
    g_assert_cmpstr(cred, ==, "DE18ZZZ00000180162");

    g_object_unref(statement);
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/statement/remove-new-lines", test_remove_newlines);
    g_test_add_func ("/statement/prettify-diba", test_prettify_diba);
    g_test_add_func ("/statement/prettify-volksbank", test_prettify_volksbank);
    return g_test_run ();
}


//vim: expandtab sw=4
