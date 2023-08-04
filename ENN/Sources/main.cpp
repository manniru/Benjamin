#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include "enn_chapar.h"
#include <QCommandLineParser>

#ifdef WIN32
    #include "../PNN/aj_dllgen.h"
#endif

EnnCmdOptions* parseClOptions(QCoreApplication *app);

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#ifdef WIN32
    aj_dllGen();
#endif
    app.setOrganizationName("Binaee");
    app.setOrganizationDomain("Binaee.com");
    app.setApplicationName("ENN");

    qDebug() << "thread support"
             << std::thread::hardware_concurrency();

    EnnCmdOptions *options = parseClOptions(&app);
    EnnChapar chapar(&app, options);

    return 0;
}

EnnCmdOptions* parseClOptions(QCoreApplication *app)
{
    QCommandLineParser parser;
    EnnCmdOptions *ret_opt = new EnnCmdOptions;

    // -u UI_MODE
    QCommandLineOption ui_mode("u", "Set UI Mode");
    parser.addOption(ui_mode);

    // -t TEST_MODE
    QCommandLineOption test_mode("t", "Set Test Mode");
    parser.addOption(test_mode);

    // -tf TEST_FULL_MODE
    QCommandLineOption test_full_mode("tf", "Set Test Full Mode");
    parser.addOption(test_full_mode);

    // -f FILE_MODE
    QCommandLineOption file_mode("f", "Set File Mode");
    parser.addOption(file_mode);

    // -l learning rate
    QStringList option_lr;
    option_lr << "l" << "learning_rate";
    QCommandLineOption lr_option(option_lr,
                       "Set Learning Rate", "Learning Rate", ENN_LEARN_RATE);
    parser.addOption(lr_option);

    // -w word
    QStringList option_word;
    option_word << "w" << "word";
    QCommandLineOption word_option(option_word,
                       "Set Word", "Word", "");
    parser.addOption(word_option);

    parser.process(*app);

    ret_opt->mode = ENN_LEARN_MODE;
    if( parser.isSet(test_mode) )
    {
        ret_opt->mode = ENN_TEST_MODE;
    }
    else if( parser.isSet(ui_mode) )
    {
        ret_opt->mode = ENN_UI_MODE;
    }
    else if( parser.isSet(test_full_mode) )
    {
        ret_opt->mode = ENN_TF_MODE;
    }
    else if( parser.isSet(file_mode) )
    {
        ret_opt->mode = ENN_FILE_MODE;
    }

    ret_opt->learning_rate = parser.value(lr_option).toFloat();
    ret_opt->word = parser.value(word_option);

    return ret_opt;
}
