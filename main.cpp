#include "gui/mainForm/mainwindow.h"

#ifdef ENABLE_TESTS_IN_APP
#include <catch2/catch_session.hpp>
#endif

#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
#ifdef ENABLE_TESTS_IN_APP
    bool skipTests = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--no-tests") { skipTests = true; break; }
    }
    // Also allow environment variable to skip tests
    const char* env_skip = std::getenv("REMINDER_SKIP_TESTS");
    if (env_skip && std::string(env_skip) == "1") skipTests = true;

    if (!skipTests) {
        Catch::Session session;
        // You can pass Catch args via argc/argv if desired
        int testResult = session.run(argc, argv);
        if (testResult != 0) {
            std::cerr << "Unit tests failed (code=" << testResult << "). Exiting.\n";
            return testResult;
        }
        std::cout << "Unit tests passed.\n";
    } else {
        std::cout << "Skipping unit tests (--no-tests or REMINDER_SKIP_TESTS=1).\n";
    }
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
