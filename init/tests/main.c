#include <check.h>
#include "control.h"
#include "../src/log.h"


Suite * init_suite(void)
{
    Suite *s;

    s = suite_create("init");

    suite_add_tcase(s, tcontrol_create_test_case());

    return s;
}


int main()
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    log_name = "test";
    log_level = LOG_ERROR;

    s = init_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}
