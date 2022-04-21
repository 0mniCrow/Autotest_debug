#ifndef SIGNALOBJECT_V2_H
#define SIGNALOBJECT_V2_H
#include"FuncObject_v2.h"

class SignalObject_v2:public DataObject
{
private:
    int signal_type;                                          //An attribute contains a type of the signal.
    QStringList data;                                                   //          A list contains attendant values.
    SignalObject_v2() = delete;                                         //         Default constructor is turned off.

public:


    enum signalType                                                 //    Basic signals (that need to be parsed).
    {
        SIG_parenthOpen=1, SIG_parenthClose=2, SIG_comma=3, SIG_feedEnd =4,
        SIG_strEnd=15, SIG_blcStart=16, SIG_blcEnd=17, SIG_scrptEnd=18,

        Cmd_err=0, Cmd_if=5, Cmd_elseif=6, Cmd_else=7, Cmd_endif=8, Cmd_while=9,
        Cmd_loop=10, Cmd_cont=11, Cmd_break=12, Cmd_for=13, Cmd_addScr=14,
        Cmd_incr = 19, Cmd_decr = 20, Cmd_aprop = 21, Cmd_glob = 22, Cmd_ret=23,
        Cmd_mstime=24, Cmd_ldvar=25, Cmd_msg=26, Cmd_usleep=27, Cmd_msleep=28,
        Cmd_sound=29, Cmd_stop=30, Cmd_get_setadr=31
    };
    SignalObject_v2(const SignalObject_v2 & other);
    ~SignalObject_v2();
    SignalObject_v2 & operator=(const SignalObject_v2 & other);
    SignalObject_v2(const char * data);
    SignalObject_v2(char type, const char * info=nullptr);                                                   //                                Destructor.
    QStringList getData();                                      //        Get the variable with number "num".
    int getType() const;                                            //           Get a signal type of the object.
    int objType() const override;                                       //     Get an object type (override vitrual).
    static QString signalName(int signal_type);
};

typedef SignalObject_v2::signalType SIGN;

typedef std::shared_ptr<SignalObject_v2> signal_ptr;

#endif // SIGNALOBJECT_V2_H
