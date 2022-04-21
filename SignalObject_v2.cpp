#include "SignalObject_v2.h"
SignalObject_v2::SignalObject_v2(const SignalObject_v2 & other):DataObject()
{
    signal_type=other.signal_type;
    data=other.data;
}

SignalObject_v2 & SignalObject_v2::operator=(const SignalObject_v2 & other)
{
    signal_type=other.signal_type;
    data=other.data;
    return *this;
}

SignalObject_v2::SignalObject_v2(const char * info)
{
    QString temp(info);
    data = std::move(temp.split(' ',QString::SkipEmptyParts));
    //data.removeFirst();
    bool aok=true;
    temp = data.first();
    signal_type=temp.toInt(&aok,0);
    if(!aok)
    {
        signal_type=Cmd_err;
        data.clear();
        data.append(QString("Incorrect signal"));
    }
    data.removeFirst();
}

SignalObject_v2::SignalObject_v2(char type, const char * info)
{
    signal_type=static_cast<int>(type);
    if(info)
    {
        QString temp(info);
        data = temp.split(' ',QString::SkipEmptyParts);
    }
}

SignalObject_v2::~SignalObject_v2()
{
}

QStringList SignalObject_v2::getData()
{
    return data;
}

int SignalObject_v2::getType() const
{
    return signal_type;
}

int SignalObject_v2::objType() const
{
    return Tsignal;
}

QString SignalObject_v2::signalName(int signal_type)
{
    /**/
    QString answer;
    switch(signal_type)
    {
    case SIG_parenthOpen:   {   answer = "Opening parenth";     } break;
    case SIG_parenthClose:  {   answer = "Closing parenth";     } break;
    case SIG_comma:         {   answer = "Comma";               } break;
    case SIG_feedEnd:       {   answer = "Group end";           } break;
    case SIG_strEnd:        {   answer = "End of the string";   } break;
    case SIG_blcStart:      {   answer = "Start of the block";  } break;
    case SIG_blcEnd:        {   answer = "End of the block";    } break;
    case SIG_scrptEnd:      {   answer = "End of the script";   } break;
    case Cmd_err:           {   answer = "Error";               } break;
    case Cmd_if:            {   answer = "If";                  } break;
    case Cmd_elseif:        {   answer = "ElseIf";              } break;
    case Cmd_else:          {   answer = "Else";                } break;
    case Cmd_endif:         {   answer = "EndIf";               } break;
    case Cmd_while:         {   answer = "While";               } break;
    case Cmd_loop:          {   answer = "Loop";                } break;
    case Cmd_cont:          {   answer = "Continue";            } break;
    case Cmd_break:         {   answer = "Break";               } break;
    case Cmd_for:           {   answer = "For";                 } break;
    case Cmd_addScr:        {   answer = "CallScript";          } break;
    case Cmd_incr:          {   answer = "Increment";           } break;
    case Cmd_decr:          {   answer = "Decrement";           } break;
    case Cmd_aprop:         {   answer = "Apropriation";        } break;
    case Cmd_glob:          {   answer = "MakeGlobal";          } break;
    case Cmd_ret:           {   answer = "Return";              } break;
    case Cmd_mstime:        {   answer = "Current time";        } break;
    case Cmd_ldvar:         {   answer = "Load variables";      } break;
    case Cmd_msg:           {   answer = "Message";             } break;
    case Cmd_usleep:        {   answer = "Sleep s";             } break;
    case Cmd_msleep:        {   answer = "Sleep ms";            } break;
    case Cmd_sound:         {   answer = "Make sound";          } break;
    case Cmd_stop:          {   answer = "Stop";                } break;
    case Cmd_get_setadr:    {   answer = "Load config address"; } break;
    default:                {   answer = "Unknown signal";      } break;
    }
    return answer;
}
