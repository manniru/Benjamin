#include <QGuiApplication>
#include "bt_chapar.h"
#include "bt_state.h"
#ifdef WIN32
#include "../PNN/aj_dllgen.h"
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#ifdef WIN32
    aj_dllGen();
#endif

    BtState st;
    if( argc>1 )
    {
        QString in_state = argv[1];
        if( in_state=="e" )
        {
            st.state = BT_ENN_MODE;
        }
        else if( in_state=="t" )
        {
            st.state = BT_TEST_MODE;
        }
    }

    BtChapar chaper(&st);

    return app.exec();
}
