#include <iostream>

#include "doctest/doctest.h"
//#include "app.h"

//bool AppTest() {
//    App::GetInstance()->Init();
//
//    return false;
//}
//
//TEST(app_test, AppTest) {
//    EXPECT_TRUE(AppTest());
//}

TEST_SUITE("test_app2") {
    TEST_CASE("test_app2_init") {
        CHECK(false);
        //    CHECK(App::GetInstance()->Init() == 1);
    }
}
