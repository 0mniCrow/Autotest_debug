#include "ScripterCore_v2.h"

ScripterCore_v2::ScripterCore_v2(FuncManager_v2 *func_manager)
{
    startLog();
    QString msg("(ScripterCore constructor): ");
    message(msg+"Started;");
    if(func_manager)
    {
        functor = func_manager;
        message(msg+"User function manager was load;");
    }
    else
    {
        functor = new FuncManager_v2();
        message(msg+"Default function manager was load;");
    }
    QString errtxt(functor->getError());
    if(!errtxt.isEmpty())
    {
        message(msg+"Function manager was load with issues ["+errtxt+"];",scr_ERROR);
    }
    message(msg+"Core starts to read function manager's messages;");
    loadMessages(functor->readLog());
    message(msg+"Function manager's messages was read;");
    message(msg+"Reset flags;");
    char tempflags =ModifyVars;
    if(scrFlags&LogFileIsSet)
    {
        tempflags|=LogFileIsSet;
    }
    scrFlags=0;
    scrFlags=tempflags;
    message(msg+"Core starts to load scanner expressions;");
    scanner.insert(t_bool,std::move(QRegExp("^(true|false)$"))),
    scanner.insert(t_int,std::move(QRegExp("^(?:(?:\\-?\\d+)|(?:0[xX][0-9A-Fa-f]+))$"))),
    scanner.insert(t_double,std::move(QRegExp("^(\\-?\\d+\\.\\d+(?:[eE][\\-\\+]?\\d+)?)$"))),     //old ver: "^(\\-?\\d+\\.\\d+)$"
    scanner.insert(t_string,std::move(QRegExp("^\"([^\"]+)\"$"))),
    scanner.insert(t_void,std::move(QRegExp("^\\$(\\w+)$"))),
    scanner.insert(t_func,std::move(QRegExp("^([^\\$\\d]\\w*)$"))), //old ver: "^([^\\$][A-Z\\_a-z]\\w*)$"
    scanner.insert(t_match,std::move(QRegExp("^(?:\\+|\\-|\\/|\\*|\\^|\\!|\\=|\\>|\\<|\\&|\\|)$"))),
    scanner.insert(t_signal,std::move(QRegExp("^(?:\\(|\\)|\\,|\\{|\\})$")));
    message(msg+"Core is ready;");
    __inScr__set_state(IsFree);
    message(msg+"Finished;");
}

ScripterCore_v2::~ScripterCore_v2()
{
    message("Scripter Core destructor starts;");
    scrPool.clear();
    add_on_blocks.clear();
    runSeq.clear();
    globVars.clear();
    if(functor)
    {
        delete functor;
    }
    finishLog();
}

//It extracts and returns a data from the string
var_ptr ScripterCore_v2::__inScr__get_variable(const QString & info)
{
    QString msg("(__inScr__get_variable): ");
    message(msg+"Started;");
    message(msg+"Core tries to extract data from a string ["+info+"];");
    QMap<inner_types,QRegExp>::iterator it = __inScr__get_token_type(info);
    var_ptr token;

    if(it==scanner.end())
    {
       message(msg+"ERROR! Core doesn't find data in a string ["+info+"];",scr_ERROR);
       return token;
    }
    message(msg+"Data is successfully extracted;");
    QRegExp & curReg = it.value();
    QString capture = curReg.cap(1);
    if(capture.isEmpty())
    {
        capture = curReg.cap();
    }
    message(msg+"Exctracted data is ["+capture+"];");
    message(msg+"Core tries to analyse extracted data;");
    bool allok=true;
    switch(it.key())
    {
    case t_int:
    {
        message(msg+"Exctracted data is integer type;");
        int value = capture.toInt(&allok,0);
        if(allok)
        {
            message(msg+"Exctracted data successfully transformed;");
            token = std::make_shared<VarObject_v2>(value);
        }
        else
        {
            message(msg+"ERROR! Exctracted data transform fail;",scr_ERROR);
        }
    }
        break;
    case t_double:
    {
        message(msg+"Exctracted data is double type;");
        double value = capture.toDouble(&allok);
        if(allok)
        {
            message(msg+"Exctracted data successfully transformed;");
            token = std::make_shared<VarObject_v2>(value);
        }
        else
        {
            message(msg+"ERROR! Exctracted data transform fail;",scr_ERROR);
        }
    }
        break;
    case t_bool:
    {
        message(msg+"Exctracted data is boolean type;");
        token = std::make_shared<VarObject_v2>(capture.contains("true"));
    }
        break;
    case t_string:
    {
        message(msg+"Exctracted data is string type;");
        token = std::make_shared<VarObject_v2>(capture.toLatin1().data());
    }
        break;
    default:
        message(msg+"ERROR! Irregular type mismatch;",scr_ERROR);
        break;
    }
    message(msg+"Finished;");
    return token;
}

//Auxilary method, it appends a new string into a string array
char ** ScripterCore_v2::__inScr__appendstring(char ** oldchar, int size, char * newchar) const
{
    if(newchar)
    {
        char ** newsome = new char *[size+1];
        if(oldchar)
        {
            for(int i = 0; i < size;i++)
            {
                newsome[i]=oldchar[i];
            }
        }
        newsome[size]=newchar;
        delete [] oldchar;
        return newsome;
    }
    return nullptr;
}

//Auxiliary method, it copies a part of string and returns a pointer to it
char * ScripterCore_v2::__inScr__partofstring(const char * string, int position, int size) const
{
    if(!string)
        return nullptr;
    char * newstr = new char[size+1];
    newstr[size]='\0';
    for(int i = 0; i <size;i++)
    {
        newstr[i]=string[position+i];
    }
    return newstr;
}

//It splits a text to lexemes and returns the number of the error string or -1 if all ok
int ScripterCore_v2::__inScr__code_splitter(const QStringList &script_text,
                              QLinkedList<QStringList *> &container)
{
    QString msg("(__inScr__code_splitter): ");
    message(msg+"Started;");
    message(msg+"Core starts to separate the text into lexemes;");
    int i=0,end=0;
    int parenthes_counter = 0;
    int block_counter = 0;
    for(i=0,end = script_text.size(); i<end;i++)
    {
        QString data = script_text.at(i);
        if(data.isEmpty())
        {
            message("\t"+msg+"String N["+QString::number(i)+"] is empty;");
            container.append(nullptr);
            continue;
        }
        message("\t"+msg+"String N["+QString::number(i)+"] data is ["+data+"];");
        QByteArray bytearray = data.toLatin1();
        const char * splitstring = bytearray.data();
        char ** folder = nullptr;                                       //Main folder of the lexemes
        int count = 0, size = 0, startpoint = 0, elements = 0;          //Counter / length / start position / number of the elements
        bool nonstop = false, stop = false, no_error = true;            //Parentheses flag(for string elem.)/interrupt flag/error flag
        char ch=splitstring[count];
        message("\t"+msg+"Core starts to analyse string characters;");
        while(no_error)
        {
            while(ch!='\0'&&(!stop))
            {
                message("\t\t"+msg+"Current character value is ["+
                        QString::number(ch)+"];");
                switch(ch)
                {
                case'\t':
                {
                    if(nonstop)
                        size++;
                }
                    break;
                case'#':                                                    //Если это символ решетки, и если это не
                {                                                           //часть строкового элемента, флаг прерыва-
                    if(!nonstop)                                            //ния поднимается. Если это часть строко-
                        stop=true;                                          //вого токена, прерывание не включается.
                    else                                                    //И элемент засчитывается как часть длины
                        size++;                                             //строки.
                }
                    break;
                case'/':                                                    //Если это символ обратной косой черты, и
                {                                                           //если он не является частью строки, а так
                    if((!nonstop)&&(splitstring[count+1]=='/'))             //же следующий за ним элемент является
                    {                                                       //аналогичной чертой, поднимается флаг
                        stop=true;                                          //прерывания.
                        break;
                    }
                }
                case'+':
                case'-':
                case'*':
                case'%':
                case'^':
                case'!':
                case'|':
                case'&':
                case'>':
                case'<':
                case'=':
                case'{':
                case'}':
                case'(':
                case',':
                case')':                                                    //Если это один из рабочих символов скрипта
                {
                    if(!nonstop)                                            //и  если не поднят флаг строкового элемента
                    {
                        if(size>0)                                          //В случае, если длина последнего элемента
                        {                                                   //больше нуля, этот элемент помещается в
                            char * string = __inScr__partofstring(splitstring,startpoint,size);  //хранилище строк.
                            folder = __inScr__appendstring(folder,elements,string);
                            elements++;
                            if(!folder)
                            {
                                no_error = false;
                                break;
                            }
                            size=0;
                            startpoint=0;
                        }
                        char * match = new char[2];                         //Рабочий символ помещается в хранилище строк
                        match[0]=ch;                                        //как отдельная строка.
                        match[1]='\0';
                        folder = __inScr__appendstring(folder,elements,match);
                        elements++;
                    }
                    else                                                    //Если флаг строкового элемента поднят,
                    {                                                       //символ считается частью строкового токена.
                        size++;
                    }
                }
                    break;
                case'"':                                                    //Если символ является символом двойных кавычек
                {
                    if(!nonstop)                                            //И если флаг строкового элемента не поднят
                    {                                                       //начинается отсчет нового строкового элемента
                        startpoint=count;                                   //и поднимается флаг строкового элемента.
                        size++;
                        nonstop=true;
                    }
                    else                                                    //Если флаг строкового элемента поднят,
                    {                                                       //флаг строкового элемента опускается, и
                        nonstop=false;                                      //строковый токен помещается в хранилище
                        size++;                                             //строк.
                        char * string = __inScr__partofstring(splitstring,startpoint,size);
                        folder = __inScr__appendstring(folder,elements,string);
                        elements++;
                        if(!folder)
                        {
                            no_error = false;
                            break;
                        }
                        size=0;
                        startpoint=0;
                    }
                }
                    break;
                case' ':                                                    //Если данный символ - символ пробела
                {
                    if(!nonstop)                                            //и если не поднят флаг строкового эле-
                    {                                                       //мента.
                        if(size>0)                                                      //Если длина последнего эле-
                        {                                                               //мента больше нуля, то эле-
                            char * string = __inScr__partofstring(splitstring,startpoint,size);  //мент помещается в хранилище
                            folder = __inScr__appendstring(folder,elements,string);              //строк.
                            elements++;
                            if(!folder)
                            {
                                no_error = false;
                                break;
                            }
                            size=0;
                            startpoint=0;
                        }                                                               //В противном случае символ
                    }                                                                   //игнорируется.
                    else                                                    //Если флаг строкового элемента поднят
                        size++;                                             //пробел считается частью строкового токена
                }
                    break;
                default:                                                    //Любой другой символ считается за обычный
                {                                                           //символ, и если длина предыдущего элемента
                    if(size==0&&(!nonstop))                                 //равна нулю, и флаг строкового элемента опу-
                    {                                                       //щен, то стартовая позиция нового элемента
                        startpoint=count;                                   //заполняется индексом данного символа.
                    }
                    size++;
                }
                    break;
                }
                if(!no_error)
                    break;
                ch=splitstring[++count];                                    //Следующий элемент.
            }
            if(no_error&&size>0)                                                        //Если после обработки цикла остался не-
            {                                                                           //добавленный элемент, здесь он добавля-
                char * string = __inScr__partofstring(splitstring,startpoint,size);     //ется.
                folder = __inScr__appendstring(folder,elements,string);
                elements++;
                if(!folder)
                {
                    no_error = false;
                    break;
                }
            }
            if(nonstop)                                                     //Если не была закрыта скобка строкового
                no_error=false;                                            //элемента, вызывается сообщение об ошибке.
            break;
        }
        if(!no_error)
        {
            for(int h = 0; h < elements;h++)
            {
                delete [] folder[h];
            }
            delete [] folder;
            message(msg+"ERROR! Core can't split text into lexemes at line N"+QString::number(i)+"];",scr_ERROR);
            return i;
        }


        QStringList * result = new QStringList();
        if(elements!=0)                                                 //Если рабочих элементов не оказалось,
        {                                                               //возвращается пустой указатель.
            for(int h = 0; h < elements;h++)                                //В противном случае из массива строк
            {                                                               //Создается Qt-ориентированный вектор
                QString elem(folder[h]);                         //строк и освобождается память.
                message("\n"+msg+"Lexeme ["+elem+"] was append;");
                result->append(elem);
                delete [] folder[h];
            }
            delete [] folder;
        }
        container.append(result);
    }
    if((parenthes_counter!=0)||(block_counter!=0))
    {
        message(msg+"WARNING! Not all parenthes was been closed;",scr_WARNING);
        return 0;
    }
    message(msg+"Finished;",scr_INFO);
    return -1;
}

//It analyses lexeme lines, that was separated in the splitter method and writes script tokens into temporary container
bool ScripterCore_v2::__inScr__token_analyser(const QStringList & cur_line, QLinkedList<data_ptr> & temporary,
                                    var_map & local_variables,
                                    block_ptr & containBlock, int & blockPos)
{
    QString msg("(__inScr__token_analyser): ");
    message(msg+"Started;");
    if(containBlock)
    {
        message(msg+"WARNING! Script block, that will be used for record, wasn't empty. It will be cleaned;", scr_WARNING);
    }
    containBlock.reset();
    blockPos=-1;
    if(!temporary.isEmpty())
    {
        message(msg+"WARNING! Container, that will be used for record, wasn't empty. It will be cleaned;", scr_WARNING);
        temporary.clear();
    }
    message(msg+"Core starts to analyse lexemes and transform those into tokens;",scr_INFO);
    bool isnegative=false, first_element=true, allok=true;
    for(int step = 0; step<cur_line.size();step++)
    {
        if(containBlock||(blockPos>=0))
        {
            break;
        }
        QString cur_token = cur_line.at(step);
        message("\t"+msg+"Lexeme ["+cur_token+"] is analysing now;",scr_INFO);
        QMap<inner_types,QRegExp>::iterator cur_iter = __inScr__get_token_type(cur_token);
        if(cur_iter==scanner.end())
        {
            allok=false;
            message(msg+"ERROR! Lexeme ["+cur_token+"] indefined by the scaner tool;", scr_ERROR);
            return false;
        }
        QRegExp & curReg = cur_iter.value();
        switch(cur_iter.key())
        {
        case t_bool:
        case t_int:
        case t_double:
        case t_string:
        case t_void:
        {
            message("\t\t"+msg+"Lexeme indefied as a variable;",scr_INFO);
            var_ptr token;
            QString capture = curReg.cap(1);
            if(capture.isEmpty())
            {
                capture = curReg.cap();
                if(capture.isEmpty())
                {
                    capture = curReg.cap(2);
                    if(capture.isEmpty())
                    {
                        capture=curReg.cap(3);                              //This is just a crutch.
                    }                                                       //For some reason lexemes aren't
                }                                                           //capture by regexp from the fist
            }                                                               //position.
            VarObject_v2::Visibility_v toVis = VarObject_v2::Vline;
            switch(cur_iter.key())
            {
            case t_int:
            {
                int value = capture.toInt(&allok,0);
                if(allok)
                {
                    message("\t\t"+msg+"Lexeme transformed to integer type;",scr_INFO);
                    if(isnegative)
                    {
                        value*=-1;
                    }
                        token = std::make_shared<VarObject_v2>(value);
                }
            }
                break;
            case t_double:
            {
                double value = capture.toDouble(&allok);
                if(allok)
                {
                    message("\t\t"+msg+"Lexeme transformed to double type;",scr_INFO);
                    if(isnegative)
                    {
                        value *=-1.0;
                    }
                        token = std::make_shared<VarObject_v2>(value);
                    }
            }
                break;
            case t_bool:
            {
                token = std::make_shared<VarObject_v2>(capture.contains("true"));
                message("\t\t"+msg+"Lexeme transformed to boolean type;",scr_INFO);
            }
                break;
            case t_string:
            {
                token = std::make_shared<VarObject_v2>(capture.toLatin1().data());
                message("\t\t"+msg+"Lexeme transformed to string type;",scr_INFO);
            }
                break;
            case t_void:
            {
                if(local_variables.contains(capture))
                {
                    var_map::iterator temp = local_variables.find(capture);
                    token = temp.value();
                    message("\t\t"+msg+"Lexeme transformed to existing user variable;",scr_INFO);
                }
                else
                {
                    token = std::make_shared<VarObject_v2>(capture, Type::Tvoid , Vision::Vlocal);
                    local_variables.insert(capture,token);
                    message("\t\t"+msg+"Lexeme transformed to a new user variable;",scr_INFO);

                }
                toVis=VarObject_v2::Vlocal;

            }
                break;
            default:
            {
                message(msg+"ERROR! Lexeme wasn't transformed;",scr_ERROR);
                allok=false;
            }
                break;
            }

            token->setVision(toVis);
            if(allok)
            {
                isnegative = false;
                first_element = false;
                temporary.append(std::static_pointer_cast<DataObject>(token));
                message("\t\t"+msg+"Lexeme was added to the folder as a token;",scr_INFO);
            }
            else
            {
                message(msg+"ERROR! Scanner can't pharse this lexeme ["+cur_token+"];",scr_ERROR);
                return false;
            }
        }
            break;
        case t_func:
        case t_match:
        {
            message("\t\t"+msg+"Lexeme indefied as a function;",scr_INFO);
            if(!(functor->contains(cur_token)))
            {
                message(msg+"WARNING! Token ["+cur_token+"] defined as a function, but there is no such function;",scr_WARNING);
                return false;
            }

            if(first_element&&(cur_token=="-"))
            {
                isnegative=true;
                first_element=false;
                break;
            }
            first_element=true;
            isnegative=false;
            while(functor->isOperator(cur_token))
            {
                if(functor->op_is_math(cur_token))
                {
                    first_element=false;
                }
                else
                {
                    first_element=true;
                }
                int jstep = step+1;
                if(jstep<cur_line.size())
                {
                    QString next_token = cur_line.at(jstep);
                    QMap<inner_types,QRegExp>::iterator next_iter = __inScr__get_token_type(next_token);
                    if((next_iter.key()==t_match)||(next_iter.key()==t_func))
                    {
                        next_token.prepend(cur_token);
                        if(functor->isOperator(next_token))
                        {
                            cur_token=next_token;
                            step++;
                            continue;
                        }
                        break;
                    }
                    break;
                }
                break;
            }

            std::shared_ptr<FuncObject_v2> token = std::make_shared<FuncObject_v2>(cur_token);
            temporary.append(std::static_pointer_cast<DataObject>(token));
            message("\t\t"+msg+"Lexeme was added to the folder as a token;",scr_INFO);
        }
            break;
        case t_signal:
        {
            message("\t\t"+msg+"Lexeme indefied as a signal;",scr_INFO);
            isnegative = false;
            std::shared_ptr<SignalObject_v2> token;
            if(cur_token=="(")
            {
                token =  std::make_shared<SignalObject_v2>(static_cast<unsigned char>(SignalObject_v2::SIG_parenthOpen));
                temporary.append(std::static_pointer_cast<DataObject>(token));
                first_element = true;
            }
            else if(cur_token==")")
            {
                token = std::make_shared<SignalObject_v2>(static_cast<unsigned char>(SignalObject_v2::SIG_parenthClose));
                temporary.append(std::static_pointer_cast<DataObject>(token));
                first_element = false;
            }
            else if(cur_token==",")
            {
                token = std::make_shared<SignalObject_v2>(static_cast<unsigned char>(SignalObject_v2::SIG_comma));
                temporary.append(std::static_pointer_cast<DataObject>(token));
                first_element = true;
            }
            else if(cur_token=="{")
            {
                //In this case new script block is creating and returning trought function interface
                containBlock = std::make_shared<BlockObject_v2>();
                temporary.append(std::static_pointer_cast<DataObject>(containBlock));
                blockPos=step; //This position checks at what element in the line the block was started

            }
            else if(cur_token=="}")
            {
                blockPos=step; //This position checks at what element in the line the block was finished
            }
            else
            {
                message(msg+"ERROR! Token ["+cur_token+"] defined as a signal, but there is no such signal;",scr_ERROR);
                return false;
            }
            message("\t\t"+msg+"Signal successfully processed;",scr_INFO);
        }
            break;
        default:
        {
            message("\t"+msg+"ERROR! Token ["+cur_token+"] indefined;",scr_ERROR);
            return false;
        }
            break;
        }
    }
    message(msg+"Finished;",scr_INFO);
    return true;
}

//Method uses regular expressions to define a type of the lexeme
QMap<ScripterCore_v2::inner_types,QRegExp>::iterator
                        ScripterCore_v2::__inScr__get_token_type(const QString & data)
{
    QMap<inner_types,QRegExp>::iterator iter = scanner.begin();
    while(iter!=scanner.end())
    {
        QRegExp & curReg = iter.value();
        if(curReg.indexIn(data)>=0)
        {
            return iter;
        }
        iter++;
    }
    return iter;
}

//It prosesses a whole lexemes list and creates sequance of the script tokens.
//It works at more higher level that token analyser - it creates blocks and logic elements
block_ptr ScripterCore_v2::__inScr__code_analyser(const QLinkedList<QStringList *> & data,
                                        block_ptr * parent)
{
    QString msg("(__inScr__code_analyser): ");
    message(msg+"Started;");
    QStack<block_ptr> charger;
    block_ptr cur_block;
    var_map current_local;
    if(parent)
    {
        message(msg+"This script block has a parent block;", scr_INFO);
        cur_block = std::make_shared<BlockObject_v2>();
        current_local = (*parent)->getLocalVariables();
        cur_block->setParentVariables(current_local);
    }
    else
    {
        message(msg+"This script block is a main block;", scr_INFO);
        cur_block =  std::make_shared<BlockObject_v2>(QString("mainBlock"));
    }
    charger.push(cur_block);

    message(msg+"Core starts to analyse the data;", scr_INFO);
    int stringNum = 0;
    bool allok=true;
    QLinkedList<QStringList*>::const_iterator scrIter = data.begin();
    while(scrIter!=data.end())
    {
        if(!(*scrIter))
        {
            message("\t"+msg+"Current block is empty;", scr_INFO);
            stringNum++;
            scrIter++;
            continue;
        }
        QStringList cur_line = (*(*scrIter));
        message("\t"+msg+"Current lexemes line N["+QString::number(stringNum+1)+"] is ["+cur_line.join(" ")+"];", scr_INFO);
        if(cur_line.isEmpty())
        {
            message("\t"+msg+"Current string is empty;", scr_INFO);
            stringNum++;
            scrIter++;
            continue;
        }
        message("\t"+msg+"Core strarts to analyse this string;", scr_INFO);
        QLinkedList<data_ptr> temporary;
        int blockPos = -1;
        block_ptr tempBlock;
        while(__inScr__token_analyser(cur_line,temporary,current_local,tempBlock,blockPos))
        {
            message("\t"+msg+"Current string was analysed;", scr_INFO);
            bool not_sorted = true;
            if(!temporary.isEmpty())
            {
                message("\t"+msg+"Core sends a token list to sorting;", scr_INFO);
                if(!__inScr__code_sort(temporary))
                {
                    message(msg+"ERROR! Core can't sort this list of tokens;",scr_ERROR);
                    allok = false;
                    break;
                }
                message("\t"+msg+"The token list was sorted;", scr_INFO);
                not_sorted = false;
            }

            if(tempBlock||(blockPos>=0))
            {
                message("\t"+msg+"Operations with the block started;", scr_INFO);
                if(not_sorted)
                {
                    message("\t"+msg+"Core sends a token list to sorting;", scr_INFO);
                    if(!__inScr__code_sort(temporary))
                    {
                        message(msg+"ERROR! Core can't sort this list of tokens;",scr_ERROR);
                        allok = false;
                        break;
                    }
                }
                if(!temporary.isEmpty())
                {
                    cur_block->appendString(stringNum,std::move(temporary));
                    message("\t"+msg+"Analysed tokens appended to the current block;", scr_INFO);
                }
                if(!cur_block->appendLocalVariables(current_local))
                {
                    message(msg+"ERROR! Local variables can't be appended to the current block", scr_ERROR);
                    allok=false;
                    break;
                }
                if(tempBlock)
                {
                    message("\t"+msg+"New block has arrived;", scr_INFO);
                    tempBlock->setParentVariables(std::move(cur_block->getLocalVariables()));
                    charger.push(tempBlock);
                    cur_block=tempBlock;
                    tempBlock.reset();
                    message("\t"+msg+"New block is set as a current block;", scr_INFO);
                }
                else if(blockPos>=0)
                {
                    message("\t"+msg+"Block position has changed;", scr_INFO);
                    charger.pop();
                    if(charger.isEmpty())
                    {
                        allok=false;
                        message(msg+"ERROR! Core tries to get a non-existent block (please check parenthes in script);", scr_ERROR);
                        break;
                    }
                    cur_block=charger.top();
                    current_local.clear();
                    current_local = cur_block->getLocalVariables();
                    message("\t"+msg+"Current block was changed from a child to a parent;", scr_INFO);
                }
                blockPos++;
                if(blockPos>=cur_line.size())
                {
                    message(msg+"ERROR! Block position exceeds the length of the token line;", scr_INFO);
                    break;
                }
                cur_line = cur_line.mid(blockPos);
                message("\t"+msg+"Token list was separated between blocks;", scr_INFO);
                continue;
            }
            else
            {   /*
                if(!__inScr__code_sort(temporary))
                {
                    message("Fail when trying to sort list of tokens;",scr_ERROR);
                    allok = false;
                    break;
                }
                */
                cur_block->appendString(stringNum,std::move(temporary));
                message("\t"+msg+"Token list was append to the block;", scr_INFO);
                break;
            }
        }
        if(!allok)
        {
            message(msg+"ERROR! The core was finish analysing with issues;",scr_ERROR);
            break;
        }
        stringNum++;
        scrIter++;
    }
    if(allok&&((!charger.isEmpty())&&(charger.size()==1)))
    {
        cur_block->appendLocalVariables(current_local);
        message(msg+"Analysis was finished successful;",scr_INFO);
        stringNum++;
        block_ptr ubertemp = charger.pop();
        ubertemp->toBlockBegin();
        message(msg+"Finished;",scr_INFO);
        return ubertemp;

    }
    else
    {
        message(msg+"ERROR! Analysis was finished with issues;",scr_ERROR);
        block_ptr fail_ptr;
        message(msg+"Finished;",scr_INFO);
        return fail_ptr;
    }
}

//It sorts incoming tokens using Polish notation.
bool ScripterCore_v2::__inScr__code_sort(QLinkedList<data_ptr> & code_line)
{
    QString msg("(__inScr__code_sort): ");
    message(msg+"Started;");
    if(code_line.isEmpty())
    {
        message(msg+"The line is empty;");
        return true;
    }

    message(msg+"Now core is starting to sort the line of tokens;",scr_INFO);
    bool aok = true;
    while(aok)
    {
        QLinkedList<data_ptr> ordered;
        QStack<data_ptr> funcStack;
        int itemcount = 0;
        for(QLinkedList<data_ptr>::iterator curItem = code_line.begin(); curItem!= code_line.end();curItem++)
        {
            itemcount++;
            message("\t"+msg+"Item N["+QString::number(itemcount)+"] is current;",scr_INFO);
            switch((*curItem)->objType())
            {
            case Tvariable:
            {
                message("\t\t"+msg+"Current item is a variable. It is appending to ordered list;",scr_INFO);
                ordered.append(*curItem);
            }
                break;
            case Tfunction:
            {
                message("\t\t"+msg+"Current item is function;",scr_INFO);
                std::shared_ptr<FuncObject_v2> function = std::dynamic_pointer_cast<FuncObject_v2>(*curItem);
                if(funcStack.isEmpty())
                {
                    message("\t\t\t"+msg+"The function stack is empty. The function is pushing to the function stack;",scr_INFO);
                    funcStack.push(*curItem);
                }
                else if(!functor->isOperator(function->fName()))
                {
                    message("\t\t\t"+msg+"This function isn't an operator. The function is pushing to the function stack;",scr_INFO);
                    funcStack.push(*curItem);
                }
                else
                {
                    message("\t\t\t"+msg+"This function is an operator. The core is start to sorting functions in the stack;",scr_INFO);
                    while((funcStack.size()>0)&&(funcStack.top()->objType()==Tfunction))
                    {
                        std::shared_ptr<FuncObject_v2> tp = std::dynamic_pointer_cast<FuncObject_v2>(funcStack.top());
                        /*
                             * Пока на вершине стека присутствует также математическая функция,
                             * а также пока поступающая функция лево-ассоциативна, и её приоритет
                             * меньше либо такой же, как и у функции на вершине стека, либо если
                             * функция право-ассоциативная и её приоритет меньше чем у функции стека
                            */
                        if(functor->isOperator(tp->fName())&&
                                ((functor->op_left_assoc(function->fName())&&
                                  (functor->op_preced(function->fName())<=functor->op_preced(tp->fName())))||
                                 (!functor->op_left_assoc(function->fName())&&
                                  (functor->op_preced(function->fName())<functor->op_preced(tp->fName())))))
                        {
                            message("\t\t\t"+msg+"Last pushed function is apending to the ordered list;",scr_INFO);
                            ordered.append(funcStack.pop());  //Переложить функцию из стека в очередь вывода
                        }
                        else
                        {
                            break;
                        }
                    }
                    message("\t\t\t"+msg+"Current operator is pushing to the function stack;",scr_INFO);
                    funcStack.push(*curItem);                                    //Затем переложить в стек поступившую функцию.
                }
            }
                break;
            case Tsignal:
            {
                message("\t\t"+msg+"Current item is a signal;",scr_INFO);
                signal_ptr signal = std::dynamic_pointer_cast<SignalObject_v2>(*curItem);
                if(signal->getType()==SignalObject_v2::SIG_comma)
                {
                    message("\t\t\t"+msg+"This signal is a comma;",scr_INFO);
                    bool endus = false;                                                //символ "(",перекладывать аргументы из
                    while(funcStack.size()>0)                                                         //стека в очередь вывода
                    {

                        if(funcStack.top()->objType()==Tsignal)
                        {
                            signal_ptr tempor = std::dynamic_pointer_cast<SignalObject_v2>(funcStack.top());
                            if(tempor->getType()==SignalObject_v2::SIG_parenthOpen)
                            {
                                message("\t\t\t"+msg+"Signal the opening parenth is found;",scr_INFO);
                                endus = true;
                                break;
                            }
                        }
                        else
                        {
                            message("\t\t\t"+msg+"Last pushed function is appending to the ordered list;",scr_INFO);
                            ordered.append(funcStack.pop());
                        }
                    }
                    if(!endus)                                                      //Если дно стека нашлось раньше символа "("
                    {                                                        //в скрипте обнаружилась ошибка последовательности
                        aok=false;
                        message(msg+"ERROR! Comma without parenthes is found (please check parenthes);",scr_ERROR);
                        break;
                    }
                }
                else if(signal->getType()==SignalObject_v2::SIG_parenthOpen)        //Если функция - символ "(", положить её в стек
                {
                    message("\t\t\t"+msg+"This signal is an open parenth;",scr_INFO);
                    funcStack.push(*curItem);
                    ordered.append(*curItem);
                }
                else if(signal->getType()==SignalObject_v2::SIG_parenthClose)                           //Если функция - символ ")"
                {                                                                      //Перекладывать функции из стека функций

                    //ordered.append(std::static_pointer_cast<DataObject>(sig_ptr));
                    message("\t\t\t"+msg+"This signal is a close parenth;",scr_INFO);
                    bool endus = false;                                             //в очередь вывода, покуда на вершине стека
                    while((funcStack.size()>0)/*(funcStack.top()->objType()==Tsignal)*/)                                                          //не появится символ "("
                    {
                        if(funcStack.top()->objType()==Tsignal)
                        {
                        std::shared_ptr<SignalObject_v2> tp =
                                std::dynamic_pointer_cast<SignalObject_v2>(funcStack.top());
                            if(tp->getType()==SignalObject_v2::SIG_parenthOpen)
                            {
                                 message("\t\t\t"+msg+"Signal the opening parenth is found;",scr_INFO);
                                endus = true;
                                break;
                            }
                            else
                            {
                                message("\t\t\t"+msg+"Last pushed function is appending to the ordered list;",scr_INFO);
                                ordered.append(funcStack.pop());
                            }
                        }
                        else
                        {
                            message("\t\t\t"+msg+"Last pushed function is appending to the ordered list;",scr_INFO);
                            ordered.append(funcStack.pop());
                        }
                    }
                    if(!endus)                                                         //Если дно стека достигнуто раньше, чем
                    {                                                       //обнаружился символ "(", значит обнаружена ошибка
                        aok = false;
                        message(msg+"ERROR! A closed parenth without an open parenth is found (please check parenthes);",scr_ERROR);
                        break;
                    }
                    funcStack.pop();                                                        //Выкидываем оператор "(" из стека
                    bool isfunc = false;
                    if(funcStack.size()>0)                                    //Если на вершине стека оператор функции, кладем
                    {                                                                                   //его в очередь вывода.

                        if((funcStack.top()->objType()==Tfunction)&&
                                (!functor->isOperator(funcStack.top()->getName())))
                        {
                            message("\t\t\t"+msg+"Last pushed function is appending to the ordered list;");
                            ordered.append(funcStack.pop());
                            isfunc=true;
                        }
                    }
                    QLinkedList<data_ptr>::iterator zae_al = (ordered.end());
                    zae_al--;
                    message("\t\t\t"+msg+"Core is setting a marker for the current function parameters end;");
                    while(zae_al!=ordered.end())
                    {
                        if(((*zae_al)->objType())==Tsignal)
                        {
                            signal_ptr sig_ptr = std::dynamic_pointer_cast<SignalObject_v2>(*zae_al);
                            if(sig_ptr->getType()==SignalObject_v2::SIG_parenthOpen)
                            {
                                if(isfunc)
                                {
                                signal_ptr sig_end = std::make_shared<SignalObject_v2>(
                                            static_cast<char>(SignalObject_v2::SIG_feedEnd));
                                *zae_al=std::static_pointer_cast<DataObject>(sig_end);
                                }
                                else
                                {
                                    zae_al=ordered.erase(zae_al);
                                }
                                break;
                            }
                        }
                        if(zae_al==ordered.begin())
                        {
                            zae_al=ordered.end();
                            break;
                        }
                        zae_al--;
                    }
                    if(zae_al==ordered.end())
                    {
                        aok = false;
                        message(msg+"ERROR! There is no parenthes for a function(please check parenthes);",scr_ERROR);
                        break;
                    }
                }
            }
                break;
            case Tblock:
            {
                message("\t\t"+msg+"Current item is a scipt block;",scr_INFO);
                ordered.append(*curItem);
            }
                break;
            }
            if(!aok)
            {
                break;
            }
        }
        if(!aok)
        {
            ordered.clear();
            message(msg+"ERROR! Sorting finished with issues;",scr_ERROR);
            return false;
        }
        else
        {
            message("\t\t"+msg+"Sorting was finished;",scr_INFO);
            message("\t\t"+msg+"Checking for remaining functions;",scr_INFO);
            while(funcStack.size()>0)                   //Когда не осталось операторов на выходе
            {                                           //и если в стеке остались токены,
                if(funcStack.top()->objType()==Tsignal)
                {
                    signal_ptr tp = std::dynamic_pointer_cast<SignalObject_v2>(funcStack.top());
                    if((tp->getType() == SignalObject_v2::SIG_parenthOpen)||
                            (tp->getType() == SignalObject_v2::SIG_parenthClose)||
                            (tp->getType() == SignalObject_v2::SIG_comma))
                    {
                        ordered.clear();
                        message(msg+"ERROR! There are unused signals stay at the line;",scr_ERROR);
                        return false;
                    }
                }
                message("\t\t\t"+msg+"Last pushed function is appending to the ordered list;",scr_INFO);
                ordered.append(funcStack.pop());
            }
            code_line.clear();
            code_line = ordered;
            message("\t\t"+msg+"Input line was replaced by ordered;",scr_INFO);
        }
        break;
    }
    if(aok)
    {
        message(msg+"Sorting ends with success;",scr_INFO);
        return true;
    }
    message(msg+"ERROR! Sorting fails",scr_ERROR);
    return false;
}

//It is the main calculation method - the one line run.
data_ptr ScripterCore_v2::__inScr__calculate_step(block_ptr &data)
{
    QString msg("(__inScr__calculate_step): ");
    message(msg+"Started;");

    data_ptr answer;
    if(!data)
    {
        message(msg+"ERROR! Incoming block is empty;",scr_ERROR);
        return answer;
    }
    QLinkedList<data_ptr> inLineVars;
    bool allok = true;
    QStack<data_ptr> varPool;
    QLinkedList<data_ptr> & current_line = data->getCurrentLine();
    QLinkedList<data_ptr>::iterator it = current_line.begin();
    message(msg+"Calculation is starting;",scr_INFO);
    while((it!=current_line.end())&&(scrState!=IsBreak))
    {
        data_ptr cur_item = *it;
        message("\t"+msg+"Current item name is ["+cur_item->getName()+"];",scr_INFO);
        switch(cur_item->objType())                                                  // The switch checks a type of the current token.
        {
        case Tsignal:                                                                //                     For a signals will be one
        {                                                                            //            outcome - they will be pushed into
            message("\t"+msg+"Current item is a signal;");
            signal_ptr sign = std::dynamic_pointer_cast<SignalObject_v2>(cur_item);
            message("\t"+msg+"Signal code is ["+QString::number(sign->getType())+"];");
            message("\t"+msg+"It goes to the variable stack;",scr_INFO);
            varPool.push(cur_item);                                                  //                              a variable stack.
        }
            break;
        case Tvariable:
        {
            message("\t"+msg+"Current item is a variable;");
            var_ptr variable = std::dynamic_pointer_cast<VarObject_v2>(cur_item);
            char * logdata = variable->operator char *();
            message("\t"+msg+"Variable value is ["+QString(logdata)+"];",scr_INFO);
            delete [] logdata;
            if(variable->getVision()==VarObject_v2::Vlocal)
            {
                auto temp = globVars.find(variable->getName());
                if(temp!=globVars.end())
                {
                    variable = temp.value();
                    message("\t\t"+msg+"There is already global variable with that name. Global one replaces local;",scr_INFO);
                }
            }
            varPool.push(std::static_pointer_cast<DataObject>(variable));
            message("\t"+msg+"Variable goes to the stack;",scr_INFO);
        }
            break;
        case Tfunction:
        {
            message("\t"+msg+"Current item is a function;",scr_INFO);
            func_ptr function = std::dynamic_pointer_cast<FuncObject_v2>(cur_item);
            QString func_name = function->fName();
            message("\t"+msg+"Function name is ["+func_name+"];",scr_INFO);
            QVector<var_ptr> for_send;
            data_ptr temp;
            if(functor->isOperator(func_name))
            {
                message("\t\t"+msg+"Current function is an operator;",scr_INFO);
                message("\t\t"+msg+"Core is preparing to call operator;",scr_INFO);
                int size = functor->op_arg_count(func_name);
                message("\t\t"+msg+"Current operator has ["+size+"] arguments;",scr_INFO);
                if(size==0)
                {
                    message("\t\t"+msg+"Core is starting to call an operator;",scr_INFO);
                    temp = functor->callFunction(function,for_send);
                }
                else
                {
                    for_send.resize(size);
                    message("\t\t"+msg+"Extracting arguments from stack;",scr_INFO);
                    for(int i = size-1; i>=0; i--)
                    {
                        if((!varPool.isEmpty())&&(varPool.top()->objType()==Tvariable))
                        {
                            for_send[i] = std::dynamic_pointer_cast<VarObject_v2>(varPool.pop());
                            char * logdata = for_send[i]->operator char *();
                            message("\t"+msg+"Argument N["+QString::number(i+1)+"] = ["+QString(logdata)+"];",scr_INFO);
                            delete [] logdata;
                        }
                        else
                        {
                            allok = false;
                            message(msg+"ERROR! Not enought parameters for the operator ["+func_name+"];",scr_ERROR);
                            break;
                        }
                    }
                    message("\t\t"+msg+"Arguments are extracted;",scr_INFO);
                    if(!allok)
                    {
                        break;
                    }
                    message("\t\t"+msg+"Core is starting to call the operator;",scr_INFO);
                    temp = functor->callFunction(function,for_send);
                }
            }
            else
            {
                message("\t\t"+msg+"Current function isn't an operator;",scr_INFO);
                message("\t\t"+msg+"Extracting arguments from stack;",scr_INFO);
                while(true)
                {
                    if(varPool.isEmpty())
                    {
                        allok=false;
                        message(msg+"ERROR! Not enought parameters for function ["+func_name+"];",scr_ERROR);
                        break;
                    }
                    else if(varPool.top()->objType()==Tvariable)
                    {
                        for_send.append(std::dynamic_pointer_cast<VarObject_v2>(varPool.pop()));
                        char * logdata = for_send.last()->operator char *();
                        message("\t\t\t"+msg+"Variable ["+QString(logdata)+"] was appended to the argument list;");
                        delete [] logdata;
                    }
                    else if(varPool.top()->objType()==Tsignal)
                    {
                        signal_ptr sign = std::dynamic_pointer_cast<SignalObject_v2>(varPool.top());
                        int type = sign->getType();
                        if(type!=SignalObject_v2::SIG_feedEnd)
                        {
                           message(msg+"ERROR! Undefined signal with code ["+QString::number(type)+"] in the stack;",scr_ERROR);
                           allok=false;
                        }
                        else
                        {
                            message("\t\t\t"+msg+"That was all arguments for the function;");
                            if(for_send.size()>1)                                                           //Arguments are in reverse order
                            {                                                                               //after being in the stack container
                                message("\t\t\t"+msg+"Core starts to reversing function's arguments;");
                                QVector<var_ptr> revers_for_send;
                                while(!for_send.isEmpty())
                                {
                                    revers_for_send.append(for_send.takeLast());
                                }
                                swap(revers_for_send,for_send);
                                message("\t\t\t"+msg+"Reversing is complete;");
                            }
                        }
                        varPool.pop();
                        break;
                    }
                    else
                    {
                        allok=false;
                        message(msg+"ERROR! Type mismatch. Wrong token in the stack;",scr_ERROR);
                        break;
                    }
                }
                if(!allok)
                {
                    message(msg+"ERROR! Arguments was extract with an issue;",scr_ERROR);
                    break;
                }
                message("\t\t"+msg+"Core is starting to call the function;",scr_INFO);
                temp = functor->callFunction(function,for_send);
            }
            message("\t"+msg+"Loading messages form the function manager;",scr_INFO);
            loadMessages(functor->readLog());
            message("\t"+msg+"Messages form the function manager are loaded;",scr_INFO);
            if((!allok)||(!temp))                                                   //                        checks an importatn values state
            {
                allok=false;
                message(msg+"ERROR! Function ["+func_name+"] returns without responce;",scr_ERROR);
                break;
            }
            message("\t"+msg+"Core is starting to analyse function responce;",scr_INFO);
            switch(temp->objType())                                                 //            Checking a type of the returned basic object
            {
            case Tvariable:                                                         //     If it is a variable, throught a dynamic cast checks
            {                                                                       //      a vision of the variable, and if this is temporary
                message("\t\t"+msg+"Function returned a variable;",scr_INFO);
                var_ptr tempvar = std::dynamic_pointer_cast<VarObject_v2>(temp);    //         object, the variable is appended to the list of
                char * logdata = tempvar->operator char *();
                message("\t\t"+msg+"Variable value is["+QString(logdata)+"];");
                delete [] logdata;
                if(tempvar->getVision()==Vision::Vtemp)                             //the temporary variables. In the any case the variable is
                {
                    message("\t\t"+msg+"This variable is temporary and will be destroy at the end of line run;",scr_INFO);
                    inLineVars.append(tempvar);                                     //                            pushes to the variable stack;
                }
                varPool.push(std::static_pointer_cast<DataObject>(tempvar));
                message("\t\t"+msg+"Variable is appending to the stack;",scr_INFO);
            }
                break;
            case Tsignal:                                              //               If this is a signal,checks an answer value,
            {                                                          //                          and if a variable already taken
                message("\t\t"+msg+"Function returned a signal;",scr_INFO);
                signal_ptr tempvar = std::dynamic_pointer_cast<SignalObject_v2>(temp);
                message("\t\t"+msg+"Signal code is ["+QString::number(tempvar->getType())+"];",scr_INFO);
                if(tempvar->getType()!=SignalObject_v2::Cmd_err)
                {
                    if(!__inScr__change_current_state(tempvar,&for_send))
                    {
                        allok = false;
                        message(msg+"ERROR! Undefinded returning signal from the function ["+func_name+"];",scr_ERROR);
                        break;
                    }
                    message("\t\t"+msg+"Signal analysed;",scr_INFO);
                }
                else
                {
                    message("\t\t"+msg+"Function returned an error signal;",scr_INFO);
                    QStringList dll_msg = tempvar->getData();
                    message(msg+"ERROR! The function ["+func_name+
                            "] returned an error signal with message (\""+
                            dll_msg.join(' ')+"\");",scr_ERROR);
                    allok = false;
                    //answer = temp;
                    break;
                }
            }
                break;
            default:
            {
                message(msg+"ERROR! Type mismatch. Function ["+func_name+"] returned data with undefined type;",scr_ERROR);
                allok = false;
            }
                break;
            }
        }
            break;
        case Tblock:
        {
            message("\t"+msg+"Current item is a block;",scr_INFO);
            answer = cur_item;
            message("\t"+msg+"New block will unfold in the next iteration;",scr_INFO);
        }
            break;
        }
        if(!allok)
        {
            message(msg+"ERROR! Line iteration completed with an issues;",scr_ERROR);
            signal_ptr error = std::make_shared<SignalObject_v2>(static_cast<char>(SignalObject_v2::Cmd_err),
                                                                 "Problem during a line calculation!");
            answer = std::static_pointer_cast<DataObject>(error);
            break;
        }
        it++;
    }
    message(msg+"Line calculation was finished;",scr_INFO);
    message(msg+"Finished;",scr_INFO);
    return answer;
}

//It changes state of the scrips. It uses when libraries return a signal.
bool ScripterCore_v2::__inScr__change_current_state(signal_ptr & signal, QVector<var_ptr> *data)
{
    QString msg("(__inScr__change_current_state): ");
    message(msg+"Started;");
    /*

        Cmd_err=0, Cmd_if=5, Cmd_elseif=6, Cmd_else=7, Cmd_endif=8, Cmd_while=9,
        Cmd_loop=10, Cmd_cont=11, Cmd_break=12, Cmd_for=13, Cmd_addScr=14,
        Cmd_incr = 19, Cmd_decr = 20, Cmd_aprop = 21, Cmd_glob = 22, Cmd_ret=23,
        Cmd_setup=24, Cmd_ldvar=25, Cmd_msg=26
*/

    //block_ptr cur_block = runSeq.top();
    message(msg+"Core is extracting an actual script block to work;");
    block_ptr cur_block =__inScr__script_get_actual_block();
    switch(signal->getType())
    {
    case SIGN::Cmd_if:
    {
        message("\t"+msg+"Current signal type is [IF];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"if\" - wrong argument count;",scr_ERROR);
            return false;
        }
        VarObject_v2 * info = data->at(0).get();
        if(static_cast<bool>(*info))
        {
            scriptManager.StartCondition(true);
        }
        else
        {
            cur_block->nextString();
            scriptManager.StartCondition(false);
        }
    }
        break;
    case SIGN::Cmd_elseif:
    {
        message("\t"+msg+"Current signal type is [ELSEIF];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"elseif\" - wrong argument count;",scr_ERROR);
            return false;
        }
        VarObject_v2 * info = data->at(0).get();
        scriptManager.NextCondition(static_cast<bool>(*info));
        if(!scriptManager.ConditionValid())
        {
            cur_block->nextString();
        }
    }
        break;
    case SIGN::Cmd_else:
    {
        message("\t"+msg+"Current signal type is [ELSE];");
        scriptManager.NextCondition(true);
        if(!scriptManager.ConditionValid())
        {
            cur_block->nextString();
        }
    }
        break;
    case SIGN::Cmd_endif:
    {
        message("\t"+msg+"Current signal type is [ENDIF];");
        scriptManager.EndCondition();
    }
        break;
    case SIGN::Cmd_while:
    {
        message("\t"+msg+"Current signal type is [WHILE];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"while\" - wrong argument count;",scr_ERROR);
            return false;
        }
        VarObject_v2 * info = data->at(0).get();
        if(static_cast<bool>(*info))
        {
            scriptManager.StartCycle(true);
        }
        else
        {
            cur_block->nextString();
            scriptManager.StartCycle(false);
        }
    }
        break;
    case SIGN::Cmd_for:
    {
        message("\t"+msg+"Current signal type is [FOR];");
        if(data->size()!=3)
        {
            message(msg+"ERROR! COMMAND \"for\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr counter = data->at(2);
        VarObject_v2 * condition = data->at(1).get();
        int incr = (static_cast<int>(*(data->at(0).get())));

        if(static_cast<bool>(*condition))
        {
            scriptManager.StartForCycle(counter,true,incr);
        }
        else
        {
            cur_block->nextString();
            scriptManager.StartForCycle(counter,false,incr);
        }
    }
        break;
    case SIGN::Cmd_loop:
    {
        message("\t"+msg+"Current signal type is [LOOP];");
        if(scriptManager.CycleActive())
        {
            if(scriptManager.CycleValid())
            { 
                cur_block->prevString();
                cur_block->prevString();
                __inScr__set_flag(MissOneStep);
            }
            scriptManager.EndCycle();
        }
        else
        {
            message(msg+"ERROR! COMMAND \"loop\" - trying to stop not-existing cycle;",scr_ERROR);
            return false;
        }
    }
        break;
    case SIGN::Cmd_cont:
    case SIGN::Cmd_break:
    {
        bool last = false;

        while(true)
        {
            last = scriptManager.IsCycleOnTop();

            block_ptr temp = __inScr__script_eject_current_block();
            if(!temp)
            {
                message(msg+"ERROR! COMMAND \"continue\" - trying to exit from unexisting cycle;",scr_ERROR);
                return false;
            }
            block_ptr metatemp = __inScr__script_get_actual_block();

            if(!metatemp)
            {
                message(msg+"ERROR! COMMAND \"continue\" - trying to exit from unexisting cycle;",scr_ERROR);
                return false;
            }
            if(last)
            {
                if(signal->getType()==SIGN::Cmd_cont)
                {
                    message("\t"+msg+"Current signal type is [CONTINUE];");
                    scriptManager.ContinueCycle();
                }
                else
                {
                    message("\t"+msg+"Current signal type is [BREAK];");
                    scriptManager.BreakCycle();
                }
                break;
            }
            else
            {
                scriptManager.EndCondition();
                continue;
            }
        }
    }
        break;
    case SIGN::Cmd_addScr:
    {
        message("\t"+msg+"Current signal type is [MANUAL LOAD SCRIPT];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"loadscript\" - wrong argument count;",scr_ERROR);
            return false;
        }
        char * addr = data->at(0)->getData();
        QString addrstr(addr);
        delete [] addr;
        return __inScr__append_extern_block(addrstr);
    }
        break;
    case SIGN::Cmd_incr:
    {
        message("\t"+msg+"Current signal type is [INCREMENT];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! Operator \"++\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr incr = data->at(0);
        switch(incr->getType())
        {
        case Type::Tint:
        {
            incr->operator =(static_cast<int>(*(incr.get()))+1);
        }
            break;
        case Type::Tdouble:
        {
            incr->operator =(static_cast<double>(*(incr.get()))+1.0);
        }
            break;
        default:
        {
            return false;
        }
            break;
        }
    }
        break;
    case SIGN::Cmd_decr:
    {
        message("\t"+msg+"Current signal type is [DECREMENT];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! Operator \"--\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr incr = data->at(0);
        switch(incr->getType())
        {
        case Type::Tint:
        {
            incr->operator =(static_cast<int>(*(incr.get()))-1);
        }
            break;
        case Type::Tdouble:
        {
            incr->operator =(static_cast<double>(*(incr.get()))-1.0);
        }
            break;
        default:
        {
            return false;
        }
            break;
        }
    }
        break;
    case SIGN::Cmd_aprop:
    {
        message("\t"+msg+"Current signal type is [APROPRIATION];");
        if(data->size()!=2)
        {
            message(msg+"ERROR! Operator \"=\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr right = data->at(0);
        //var_ptr left = data->at(1);
        QStringList info = signal->getData();
        QString cominfo = info.join(" ");
        QMap<inner_types,QRegExp>::iterator iter = __inScr__get_token_type(cominfo);
        var_ptr incr = std::make_shared<VarObject_v2>();
        switch(iter.key())
        {
        case t_int:
        {
            incr->operator =(cominfo.toInt());
        }
            break;
        case t_double:
        {
           incr->operator =(cominfo.toDouble());
        }
            break;
        case t_bool:
        {
            incr->operator =(cominfo.contains("true")||(cominfo.at(0)=='1'));
        }
            break;
        default:
        {
            incr->operator =(cominfo.toLatin1().data());
        }

            break;
        }
        right.operator *() = incr.operator *();//left.operator *();
    }
        break;
    case SIGN::Cmd_glob:
    {

        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"makeglobal\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr incr = data->at(0);
        const QStringList & info = signal->getData();
        QString answer= info.at(0);
        if((incr->getVision()==Vision::Vlocal)||(incr->getVision()==Vision::Vglobal))
        {
            if((answer=="true")||answer.at(0)=='1')
            {
                message("\t"+msg+"Current signal type is [MAKEGLOBAL];");
                if(!globVars.contains(incr->getName()))
                {
                    var_ptr glob = std::make_shared<VarObject_v2>(*incr.get());
                    globVars.insert(incr->getName(),/*incr*/glob);
                    /*incr*/glob->setVision(Vision::Vglobal);
                }
            }
            else
            {
                message("\t"+msg+"Current signal type is [DELETEGLOBAL];");
                var_map::iterator iter = globVars.find(incr->getName());
                if(iter!=globVars.end())
                {
                    incr->setVision(Vision::Vlocal);
                    globVars.erase(iter);
                }
            }
        }
        else
        {
            message(msg+"ERROR! Type miscast. Object ["+incr->getName()+" is not a variable;",scr_ERROR);
            return false;
        }
    }
        break;
    case SIGN::Cmd_ret:
    {
        message("\t"+msg+"Current signal type is [RETURN];");
        if(data->size()!=1)
        {
            message(msg+"ERROR! COMMAND \"return\" - wrong argument count;",scr_ERROR);
            return false;
        }
        var_ptr incr = data->at(0);
        return_value = __inScr__get_value(incr);
    }
        break;
    case SIGN::Cmd_mstime:
    {
        message("\t"+msg+"Current signal type is [MSTIME];");
        QTime cur_time = QTime::currentTime();
        int result = script_timer.msecsTo(cur_time);
        var_ptr incr = data->at(0);
        incr->operator =(result);
    }
        break;
    case SIGN::Cmd_ldvar:
    {
        message("\t"+msg+"Current signal type is [LOAD EXCEL VARIABLES];");
        int i = load_vars.size();
        if(i > data->size())
        {
            i = data->size();
        }
        for(int j=0;j<i;j++)
        {
            //int v = data->size()-1;
            data->at(j)->operator =(load_vars.at(j).operator *());
        }
    }
        break;
    case SIGN::Cmd_msg:
    {
        message("\t"+msg+"Current signal type is [MESSAGE];");
        var_ptr incr = data->at(0);
        char * c_str = incr->getData();
        message(c_str,scr_MESSAGE);
        delete [] c_str;
    }
        break;
    case SIGN::Cmd_msleep:
    case SIGN::Cmd_usleep:
    {
        var_ptr incr = data->at(0);
        VarObject_v2 * temp = incr.get();
        int slep = static_cast<int>(*temp);
        if(slep>0)
        {
            if(signal->getType()==SIGN::Cmd_msleep)
            {
                message("\t"+msg+"Current signal type is [MSLEEP];");
                QThread::msleep(slep);
            }
            else
            {
                message("\t"+msg+"Current signal type is [USLEEP];");
                QThread::sleep(slep);
            }
        }
        else
        {
            message(msg+"ERROR! Delay value can't be negative number!", scr_ERROR);
            return false;
        }
    }
        break;
    case SIGN::Cmd_stop:
    {
        message("\t"+msg+"Current signal type is [STOP];");
        __inScr__set_state(IsBreak);
    }
        break;
    case SIGN::Cmd_sound:
    {
        message("\t"+msg+"Current signal type is [SOUND];");
        QApplication::beep();
    }
        break;
    case SIGN::Cmd_get_setadr:
    {
        message("\t"+msg+"Current signal type is [GET SETTING ADDRESS];");
        var_ptr incr = data->at(0);
        incr->operator =(config_file.toLatin1().data());
    }
        break;
    default:
    {
        if(!this->changeAdditionalState(signal,data))
        {
            message(msg+"ERROR! Undefined signal;",scr_ERROR);
            return false;
        }
        message("\t"+msg+"Current signal is usertype;");
    }
        break;
    }
    message(msg+"Finished;");
    return true;
}

//It is an optional method for overload. It can be used for additional signals with no need to rewrite main code.
bool ScripterCore_v2::changeAdditionalState(signal_ptr & funcAnswer, QVector<var_ptr> *data)
{
    Q_UNUSED( funcAnswer);
    Q_UNUSED (data);
    return false;
}

//It writes messages for a log.
void ScripterCore_v2::message(const QString & text, int importancy)
{
    //scripter_log.append(text);

    switch(importancy)
    {
    case scr_MESSAGE:
    case scr_ERROR:
    {
        emit __signal_script_message(text,importancy);
    }
        break;
    case scr_WARNING:
    {
        if(scrFlags&SendWarningMessages)
        {
        emit __signal_script_message(text,importancy);
        }
    }
        break;
    case scr_INFO:
    {
        if(scrFlags&SendInfoMessages)
        {
        emit __signal_script_message(text,importancy);
        }
    }
        break;
    }
    if(scrFlags&LogFileIsSet)
    {
        QString cur_time = QTime::currentTime().toString("hh:mm:ss.zzz ");
        inlogstream<<cur_time+text+"\n";
    }
}

//This is overload method. It is same as previous.
void ScripterCore_v2::message(const char *text, int inportancy)
{
    const QString data(text);
    message(data,inportancy);
}

//It initialises a log for writing.
void ScripterCore_v2::startLog()
{
    if(scrFlags&LogFileIsSet)
    {
        return;
    }
    QString path(QDir::currentPath()+"/"+SCRIPTER_LOG_ADDR+"/");
    QDir krk;
    krk.mkpath(path);
    QString curDataTime = QDateTime::currentDateTime().toString("dd_MMMM_yyyy(hh.mm.ss)");
    inlogfile.setFileName(path+curDataTime+"_Script.log");
    if(inlogfile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        inlogstream.setDevice(&inlogfile);
        __inScr__set_flag(LogFileIsSet);
        message("New scripter core journal is starting;");
    }
}

//It finishes a log and flushes the data that remain in the buffer.
void ScripterCore_v2::finishLog()
{
    message("Scripter core journal is end;");
    inlogstream.flush();
    inlogfile.close();
    __inScr__remove_flag(LogFileIsSet);
}

//Auxiliary method writes multiply info messages.
void ScripterCore_v2::loadMessages(const QStringList & msgs)
{
    foreach(QString str, msgs)
    {
        message(str);
    }
}


void ScripterCore_v2::__inScr__set_state(states new_state)
{
    scrState = new_state;
}

void ScripterCore_v2::__inScr__set_flag(flags new_flag)
{
        scrFlags|=new_flag;
}

void ScripterCore_v2::__inScr__remove_flag(flags rem_flag)
{
        scrFlags&=~rem_flag;
}

//It translites in-script-variable into QVariant variable
QVariant ScripterCore_v2::__inScr__get_value(var_ptr & variable)
{
    QString msg("(__inScr__get_value): ");
    message(msg+"Started;");
    QVariant answer;
    VarObject_v2 * dat = variable.get();
    switch(dat->getType())
    {
    case Type::Tint:
    {
        answer = static_cast<int>(*dat);
    }
    break;
    case Type::Tdouble:
    {
        answer = static_cast<double>(*dat);
    }
        break;
    case Type::Tstring:
    {
        char * temp = dat->getData();
        QString temp2(temp);
        answer = temp2;
        delete [] temp;
    }
        break;
    case Type::Tbool:
    {
        answer = static_cast<bool>(*dat);
    }
        break;
    case Type::Tvoid:
    {
        QString tempo("\"empty\"");
        answer = tempo;
    }
        break;
    default:
    {
        message(msg+"ERROR! Type miscast. Variable name ["+dat->getName()+"];",scr_ERROR);
    }
        break;
    }
    message(msg+"Variable value is ["+answer.toString()+"];");
    message(msg+"Finished;");
    return answer;
}

//It loads and prepares script by the file address.
bool ScripterCore_v2::__inScr__set_current_script(const QString & address)
{
    QString msg("(__inScr__set_current_script): ");
    message(msg+"Started;");
    QString rname;
    block_ptr cur_script;
    char ch = 0;

    if(address.contains('\\'))
    {
        ch = '\\';
    }
    else if(address.contains('/'))
    {
        ch = '/';
    }

    if(ch==0)
    {
        rname = address;
    }
    else
    {
        rname = address.mid(address.lastIndexOf(ch)+1);
    }
    message(msg+"Script name is ["+rname+"];");
    if(scrPool.contains(rname))
    {
        message(msg+"Script pool already contains ["+rname+"];");
        cur_script = scrPool.find(rname).value();
        message(msg+"It will be load from core's memory (to clear the memory use \"settings\");");
    }
    else
    {
        message(msg+"There is a new script;");
        message(msg+"Core is starting to process new script;");
        cur_script = __inScr__code_disposer(address);
        if(!cur_script)
        {
            message(msg+"ERROR! Core can't load script with addres ["+address+"];",scr_ERROR);
            return false;
        }
        message(msg+"New script was sucessfully processed;");
        scrPool.insert(rname,cur_script);
    }
    runSeq.push(cur_script);
    cur_script->toBlockBegin();
    message(msg+"Running sequance is set;");
    message(msg+"Finished;");
    return true;
}

QStringList ScripterCore_v2::__inScr__open_file(const QString &file_name)
{
    QString msg("(__inScr__open_file): ");
    message(msg+"Started;");
    message(msg+"Core is trying to open file ["+file_name+"];");
    QFile file(file_name);
    QStringList data;
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        message(msg+"ERROR! Core can't open file ["+file_name+"];",scr_ERROR);
        return data;
    }
    QTextStream reader(&file);
    message(msg+"Core is starting to read text file with a script;");
    while(!reader.atEnd())
    {
        QString line(reader.readLine());
        data.append(line);
        message(msg+"Core is reading line ["+line+"];");
    }
    file.close();
    message(msg+"Finished;");
    return data;
}

//It is a main script load, a text file transletes to the token sequanse.
block_ptr ScripterCore_v2::__inScr__code_disposer(const QString & address, block_ptr *parent)
{
    QString msg("(__inScr__code_disposer): ");
    message(msg+"Started;");
    message(msg+"Main analysing process started;");
    message(msg+"The first step: reading file ["+address+"];");
    QStringList data = __inScr__open_file(address);
    message(msg+"The first step over. Core was read ["+QString::number(data.size())+"] lines in script;");

    message(msg+"The second step: splitting text data into lexemes;");
    QLinkedList<QStringList*> splitdata;
    block_ptr answer;
    int errstr = __inScr__code_splitter(data,splitdata);
    if(errstr!=-1)
    {
        message(msg+"ERROR! Splitting script - problem with line N["+QString::number(errstr+1)+"];", scr_ERROR);
        return answer;
    }
    message(msg+"The second step over. Elements was splitted into lexemes;");
    message(msg+"The third step: analysing lexemes;");
    answer = __inScr__code_analyser(splitdata,parent);
    qDeleteAll(splitdata);
    if(!answer)
    {
        message(msg+"ERROR! Analysing script was failed;", scr_ERROR);
    }
    message(msg+"The third step over. Lexemes was analysed;");
    message(msg+"Finished;");
    return answer;
}

//This is slot. It starts the script sequance.
bool ScripterCore_v2::__inScr__script_start()
{
    QString msg("(__inScr__code_disposer): ");
    message(msg+"Started;");
    if(scrState!=IsReady)
    {
        message(msg+"ERROR! Scripter core isn't ready to start!",scr_ERROR);
        return false;
    }
    __inScr__set_state(IsRun);
    message(msg+"Core state is changed to RUN;");
    block_ptr cur_block = __inScr__script_get_actual_block();
    message(msg+"Getting an actual script block to run;");
    if(!cur_block)
    {
        __inScr__set_state(IsError);
        message(msg+"ERROR! Trying to invoke an ended script", scr_ERROR);
        return __inScr__script_end();
    }
    bool allok = true;
    message(msg+"Core is starting to run the script;");
    while(cur_block)
    {
        message("\t"+msg+"Line calculating is starting;");
        data_ptr answer = __inScr__calculate_step(cur_block);
        if(scrState == IsBreak)
        {
            message("\t"+msg+"Break state is active. Script running is stopping;");
            __slot_break_script();
            return true;
        }
        if(answer)
        {
            message("\t"+msg+"Start to scanning calculation answer;");
            allok = __inScr__scan_calculator_answer(answer);
        }

        if(!allok)
        {
            message("\t"+msg+"ERROR! Answer calculation returns error;");
            __inScr__set_state(IsError);
            return __inScr__script_end();
        }

        if(scrFlags&MissOneStep)
        {
            __inScr__remove_flag(MissOneStep);
        }
        else
        {
            //! ______________________!Check!_________________________
            if(!cur_block->nextString())
            {
                message("\t"+msg+"WARNING! There is no more lines in block;",scr_WARNING);
                /*
                message("\t"+msg+"There is no more lines in block;",scr_ERROR);
                __inScr__set_state(IsError);
                return __inScr__script_end();
                */
            }
            //!______________________!Check!___________________________
        }

        if(__inScr__check_end())
        {
            message("\t"+msg+"Running is over;");
            return __inScr__script_end();
        }
        if((add_on_blocks.isEmpty())&&(scrFlags&StepModeOn))
        {
            message("\t"+msg+"Core is ready for the next step;");
            block_ptr temp_block = __inScr__script_get_actual_block();
            return __inScr__script_continue(temp_block->curStrNum());
        }
        cur_block = __inScr__script_get_actual_block();
    }
    message("\t"+msg+"Script running is ends;");
    message(msg+"Finished;");
    return __inScr__script_end();
}

//It checks if any token lines remain in the current block.
bool ScripterCore_v2::__inScr__check_end()
{
    if(add_on_blocks.isEmpty())
    {
        if(runSeq.size()==1)
        {
            if(runSeq.top()->isBlockEnd())
            {
                return true;
            }
        }
    }
    return false;
}

//It analyses a calculation method answer. It works above current script line
bool ScripterCore_v2::__inScr__scan_calculator_answer(data_ptr & answer)
{
    QString msg("(__inScr__scan_calculator_answer): ");
    message(msg+"Started;");
    message(msg+"Core is starting to analyse calculation answer;");
    switch(answer->objType())
    {
    case Tblock:
    {
        message("\t"+msg+"Calculation answer is a block;");
        block_ptr new_block = std::dynamic_pointer_cast<BlockObject_v2>(answer);
        new_block->toBlockBegin();
        if(!add_on_blocks.isEmpty())
        {
            message("\t"+msg+"This block is not a part of the script;");
            add_on_blocks.push(new_block);
        }
        else
        {
            message("\t"+msg+"This is a standard script block;");
            runSeq.push(new_block);
        }
        return true;
    }
        break;
    case Tsignal:
    {
        message("\t"+msg+"Calculation answer is a signal;");
        signal_ptr signal = std::dynamic_pointer_cast<SignalObject_v2>(answer);
        QString errorText;
        const QStringList & errorLines = signal->getData();
        for(int i = 0; i<errorLines.size();i++)
        {
            errorText.append(errorLines.at(i));
        }
        if(signal->getType()==SIGN::Cmd_err)
        {
            message("\t"+msg+"This signal is an error signal;");
            message(msg+"ERROR! Error text \""+errorText+"\";",scr_ERROR);
            return false;
        }
        else
        {
            message(msg+"ERROR! Unknown signal with message \""+errorText+"\";",scr_ERROR);
            return false;
        }
    }
        break;
    default:
    {
        message(msg+"ERROR! Type mismatch. Calculator returns a value with unknown type;",scr_ERROR);
        return false;
    }
        break;
    }
    message(msg+"Finished;");
    return true;
}

block_ptr ScripterCore_v2::__inScr__script_eject_current_block()
{
    return __inScr__script_get_actual_block(true);
}

//It finds a current using block and returns a pointer to it / ejects from the source
block_ptr ScripterCore_v2::__inScr__script_get_actual_block(bool secret)
{
    block_ptr cur_block;
    while(!add_on_blocks.isEmpty())
    {
        if(add_on_blocks.top()->isBlockEnd())
        {
            block_ptr temp_block = add_on_blocks.pop();
            temp_block->toBlockBegin();
            QMap<QString,var_ptr> map_variables;
            if(!add_on_blocks.isEmpty())
            {
                map_variables = add_on_blocks.top()->getLocalVariables();
            }
            else
            {
                map_variables = runSeq.top()->getLocalVariables();
            }
            temp_block->refresh(map_variables);
            continue;
        }
        if(secret)
        {
            cur_block = add_on_blocks.pop();
        }
        else
        {
            cur_block = add_on_blocks.top();
        }
        break;
    }

    if(!cur_block)
    {
        while(!runSeq.isEmpty())
        {
            if(runSeq.top()->isBlockEnd())
            {
                block_ptr temp_bl = runSeq.pop();
                temp_bl->toBlockBegin();
                QMap<QString,var_ptr> map_variables;
                if(!runSeq.isEmpty())
                {
                    map_variables = runSeq.top()->getLocalVariables();
                }
                temp_bl->refresh(map_variables);
                continue;
            }
            if(secret)
            {
                cur_block = runSeq.pop();
            }
            else
            {
                cur_block = runSeq.top();
            }
            break;
        }
    }
    return cur_block;
}

//This is slot. It informes a user that the script line is calculated and the script is ready
//to the next step
bool ScripterCore_v2::__inScr__script_continue(int line)
{
    if(scrState==IsBreak)
    {
        return false;
    }
    else if(scrFlags&ModifyVars)
    {
        QMap<QString,QVariant> cementary;
        var_map variables;
        variables = runSeq.top()->getLocalVariables();
        for(var_map::iterator iter = variables.begin(); iter!= variables.end();iter++)
        {
            var_map::iterator gl_iter = globVars.find(iter.key());
            if(gl_iter!=globVars.end())
            {
                cementary.insert(gl_iter.key(),__inScr__get_value(gl_iter.value()));
            }
            else
            {
                cementary.insert(iter.key(), __inScr__get_value(iter.value()));
            }
        }
        emit __signal_script_endline(cementary);
    }
    __inScr__set_state(IsReady);
    emit __signal_script_ready(line);
    return true;
}

//This is slot. It clears a temporary information from the ended script.
//Also it informes user that script is finished
bool ScripterCore_v2::__inScr__script_end()
{
    QString msg("(__inScr__script_end): ");
    message(msg+"Started;");
    add_on_blocks.clear();
    message(msg+"Core starts to clear script temporary information;");
    while(!runSeq.isEmpty())
    {
        var_map vars;
        block_ptr block = runSeq.pop();
        if(!runSeq.isEmpty())
        {
            vars = runSeq.top()->getLocalVariables();
        }
        block->refresh(vars);
        block->toBlockBegin();
    }
    scriptManager.toDefault();
    load_vars.clear();
    config_file.clear();
    __inScr__flush_log_in_file();
    if(scrState==IsError)
    {
        message(msg+"ERROR! Script was finished with errors;",scr_ERROR);
    }
    else if(scrState == IsBreak)
    {
        message(msg+"WARNING! Script execution was interrupted by user;",scr_WARNING);
    }
    message(msg+"Temporary information is cleared;");
    __inScr__set_state(IsFree);
    __signal_script_end(return_value);
    message(msg+"Finisheded;");
    return true;
}

void ScripterCore_v2::__inScr__flush_log_in_file()
{
    /*
    QString path(QDir::currentPath()+"/Journal/");
    QDir krk;
    krk.mkpath(path);
    QFile Log(path+"LastLogFile.txt");
    if(Log.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&Log);
        for(int i = 0; i<scripter_log.size();i++)
        {
            stream<<"Event #"+QString::number(i+1)+": "+scripter_log.at(i)+'\n';
        }
        Log.flush();
        Log.close();
    }
    */
}

//It appends additional script above current and sets it to run
bool ScripterCore_v2::__inScr__append_extern_block(const QString & addr)
{
    QString msg("(__inScr__append_extern_block): ");
    message(msg+"Started;");
    if(runSeq.isEmpty())
    {
        message(msg+"ERROR! There is no main script. Core can't load additional script address ["+addr+
                "];",scr_ERROR);
        return false;
    }
    block_ptr run_seq = runSeq.top();
    block_ptr add_block = __inScr__code_disposer(addr,&run_seq);
    if(add_block)
    {
        message(msg+"Additional script is load. Address ["+addr+"];",scr_INFO);
        add_block->toBlockBegin();
        add_on_blocks.push(add_block);
        message(msg+"Finished;");
        return true;
    }
    else
    {
        message(msg+"ERROR! Core can't load additional script name address ["+addr+"];",scr_ERROR);
    }
    return false;
}

//It collects variables for sending.
var_map ScripterCore_v2::__inScr__actual_variables(var_map & local_map)
{
    var_map actual_map = globVars;
    var_map::iterator iter = local_map.begin();
    while(iter!=local_map.end())
    {
        if(!(actual_map.contains(iter.key())))
        {
            actual_map.insert(iter.key(),iter.value());
        }
    }
    return actual_map;
}


//____________________________SLOTS______________________________________


//This slot takes an information from a user and prepares a script to start.
void ScripterCore_v2::__slot_load_script(const QString & name_addr,
                                   const QString & setting_addr,
                                   const QString & variables)
{
    QString msg("(__slot_load_script): ");
    message(msg+"Started;");
    if(scrState!=IsFree)
    {
        message(msg+"ERROR! Core can't load new script when previous is still running. ["+name_addr+
                "];",scr_ERROR);
        return;
    }

    if(name_addr.isEmpty())
    {
        message(msg+"ERROR! Core can't append a new script without name;",scr_ERROR);
        return;
    }


    __inScr__set_state(IsLoading);
    message(msg+"Script is loading;");


    if(name_addr.contains('$'))                           //Check that user asks a variable value
    {                                                     //not an entire script.
        message(msg+"User asks for a global variable name ["+name_addr+"];",scr_INFO);
        QString var_name = name_addr.mid(1);
        if(globVars.contains(var_name))
        {
            QMap<QString,var_ptr>::iterator temp = globVars.find(var_name);
            if(temp==globVars.end())
            {
                message(msg+"ERROR! A global variable [$"+var_name+"] is not exist;",scr_ERROR);
                emit __signal_script_end(QVariant());
            }
            QVariant answer(__inScr__get_value(temp.value()));
            message(msg+"A global variable with name ["+name_addr+"] is found. Value is ["+answer.toString()+"];",scr_INFO);
            emit __signal_script_end(answer);
        }
        __inScr__set_state(IsFree);
        message(msg+"Finished;");
        return;
    }
    else
    {
        message(msg+"Core is starting to load script ["+name_addr+"];", scr_INFO);
        if(!setting_addr.isEmpty())
        {
            message(msg+"User was set a setting file ["+setting_addr+"];", scr_INFO);
            QFileInfo setadr(setting_addr);
            if((!setadr.exists())||(!setadr.isFile()))
            {
                message("WARNING! Setting file ["+setting_addr+"] doesn't exist;", scr_WARNING);
                config_file.clear();
            }
            else
            {
                message(msg+"Setting file address is saved;", scr_INFO);
                config_file = setadr.absoluteFilePath();
            }
        }
    }
    if(!variables.isEmpty())
    {
        message(msg+"User was set variables for this script. ["+variables+"];", scr_INFO);
        QStringList varlist = variables.split(',',QString::SkipEmptyParts);
        if(varlist.isEmpty())
        {
            message(msg+"WARNING! Core can't split variables by  \",\" separator;",scr_WARNING);
        }
        QRegExp varparser("\\[(\\w+)\\]\\(([^\\)]+)\\)");
        for(QStringList::iterator it = varlist.begin();it!=varlist.end();it++)
        {
            QString & current = *it;
            message("\t"+msg+"Current variable is ["+current+"];", scr_INFO);
            var_ptr load_var;

            if(varparser.indexIn(current)>=0)
            {
                QString type = varparser.cap(1);
                QString value = varparser.cap(2);
                if(type=="v")
                {
                    message("\t"+msg+"Current variable is a script variable;", scr_INFO);
                    //QString value = varparser.cap(2);
                    value=value.remove('$');
                    message("\t"+msg+"Variable name is ["+value+"];", scr_INFO);   
                    var_map::iterator tempiter = globVars.find(value);
                    if(tempiter!=globVars.end())
                    {
                        load_var = tempiter.value();
                    }
                    else
                    {
                        message("WARNING! Core can't find a variable ["+value+"] in the memory;",scr_WARNING);
                    }
                }
                else if(type=="b")
                {
                    message("\t"+msg+"Current variable is a boolean;", scr_INFO);
                    //QString value = varparser.cap(2);
                    load_var = std::make_shared<VarObject_v2>(value.contains("true"));
                }
                else if(type=="s")
                {
                    message("\t"+msg+"Current variable is a string;", scr_INFO);
                    //QString value = varparser.cap(2);
                    message("\t"+msg+"String text is ["+value+"];", scr_INFO);
                    load_var = std::make_shared<VarObject_v2>(value.toLatin1().data());
                }
                else if(type=="d")
                {
                    message("\t"+msg+"Current variable is double;", scr_INFO);
                    bool aok = true;
                    //QString value = varparser.cap(2);
                    double fl_val = value.toDouble(&aok);
                    if(aok)
                    {
                        message("\t"+msg+"Double value is ["+QString::number(fl_val)+"];", scr_INFO);
                        load_var = std::make_shared<VarObject_v2>(fl_val);
                    }
                    else
                    {
                        message("\t"+msg+"ERROR! Core can't cast variable ["+value+"] to double;", scr_ERROR);
                    }
                }
                else if(type=="i")
                {
                    message("\t"+msg+"Current variable is integer;", scr_INFO);
                    bool aok = true;
                    //QString value = varparser.cap(2);
                    int int_val = value.toInt(&aok,0);
                    if(aok)
                    {
                        message("\t"+msg+"Integer value is ["+QString::number(int_val)+"];", scr_INFO);
                        load_var = std::make_shared<VarObject_v2>(int_val);
                    }
                    else
                    {
                        message("\t"+msg+"ERROR! Core can't cast variable ["+value+"] to integer;", scr_ERROR);
                    }
                }

                if(load_var)
                {
                    message("\t"+msg+"Variable is transformed and load;", scr_INFO);
                    load_vars.append(load_var);
                }
                else
                {
                    message(msg+"ERROR! Core can't find a needed type for ["+current+"] variable;",scr_ERROR);
                }
            }
            else
            {
                message(msg+"ERROR! Core can't translate ["+current+"] into variable;",scr_ERROR);
            }
        }
        message(msg+"Variables is analysed;", scr_INFO);
        /*
        for(QStringList::iterator it = varlist.begin();it!=varlist.end();it++)
        {
            QString current = *it;
            if(current.contains('"'))
            {
                current = current.mid(current.indexOf('"'));
                current.chop(current.size()-current.lastIndexOf('"')-1);
            }
            else
            {
                current.remove(' ');
            }
            *it = current;
        }

        for(QStringList::iterator it = varlist.begin();it!=varlist.end();it++)
        {
            var_ptr load_var;
            if((*it).contains('$'))
            {
                QString tempvar = *it;
                tempvar.remove(0,1);
                var_map::iterator tempiter = globVars.find(tempvar);
                if(tempiter!=globVars.end())
                {
                    load_var = tempiter.value();
                }
                else
                {
                    message("Can't find variable ["+*it+"] into globals;",scr_WARNING);
                }
            }
            else
            {
                load_var = __inScr__get_variable(*it);
            }

            if(load_var)
            {
                load_vars.append(load_var);
            }
            else
            {
                message("Can't translate text["+*it+"] into variable;",scr_WARNING);
            }
        }
        */
    }
    message(msg+"Core is starting to set current script;", scr_INFO);
    curLine=0;
    return_value.clear();
    if(!__inScr__set_current_script(name_addr))
    {
        message(msg+"ERROR! Current script isn't set;", scr_ERROR);
        __inScr__set_state(IsError);
        __inScr__script_end();
        return;
    }
    script_timer = QTime::currentTime();
    if(scrFlags&StepModeOn)
    {
        message(msg+"Step mode is on, script is controlled manually;", scr_INFO);
        __inScr__set_state(IsReady);
        emit __signal_script_ready(curLine);
    }
    else
    {
        message(msg+"Step mode is off, script is controlled automatic;", scr_INFO);
        __inScr__set_state(IsReady);
        __inScr__script_start();
    }
    message(msg+"Finished;");
}

//This slot waits for the user command to continue the script
void ScripterCore_v2::__slot_start_script()
{
    QString msg("(__slot_start_script): ");
    message(msg+"Started;");
    if(scrState==IsFree)
    {
        message(msg+"ERROR! Core can't start the script. Script wasn't load;",scr_ERROR);
    }
    else if(scrState==IsReady)
    {
        message(msg+"Starting script;");
        if(!__inScr__script_start())
        {
            message(msg+"ERROR! Core can't continue to run the script;",scr_ERROR);
        }
        message(msg+"Script was started;");
    }
    else
    {
        message(msg+"ERROR! Script is already running;",scr_ERROR);
    }
    message(msg+"Finished;");
}

void ScripterCore_v2::__slot_load_library(const QString & address)
{
    QString msg("(__slot_load_library): ");
    message(msg+"Started;");
    message(msg+"Core tries to load plugin ["+address+"];");
    if(scrState!=IsFree)
    {
        message(msg+"ERROR! Core can't load plugin when script is still running;",scr_ERROR);
        return;
    }
    bool aok = functor->loadNewLibrary(address);
    loadMessages(functor->readLog());
    if(!aok)
    {
        message(msg+"ERROR!Core can't load plugin. Message from function manager [" + functor->getError()+"];", scr_ERROR);
        return;
    }
    message(msg+"Plugin succesfully installed;", scr_INFO);
    message(msg+"Finished;");
}


//This slot sets user's function manager into the scripter core.
void ScripterCore_v2::__slot_set_function_core(FuncManager_v2 * newcore)
{
    QString msg("(__slot_set_function_core): ");
    message(msg+"Started;");
    if(scrState!=IsFree)
    {
        message(msg+"ERROR! Core can't load a new function manager when script is running;",scr_ERROR);
        return;
    }
    if(newcore)
    {
        delete functor;
        functor = newcore;
        message(msg+"User's function manager is installed;",scr_INFO);
    }
    message(msg+"Finished;");
}

//This and few future slot changes user communication flags
void ScripterCore_v2::__slot_turn_pause(bool logic)
{
    if(scrState!=IsRun)
    {
        if(logic)
        {
            __inScr__set_flag(StepModeOn);
        }
        else
        {
            __inScr__remove_flag(StepModeOn);
        }
    }
}

void ScripterCore_v2::__slot_turn_warnings(bool logic)
{
    if(logic)
    {
        __inScr__set_flag(SendWarningMessages);
    }
    else
    {
        __inScr__remove_flag(SendWarningMessages);
    }
}

void ScripterCore_v2::__slot_turn_info_msg(bool logic)
{
    if(logic)
    {
        __inScr__set_flag(SendInfoMessages);
    }
    else
    {
        __inScr__remove_flag(SendInfoMessages);
    }
}

void ScripterCore_v2::__slot_turn_modify_vars(bool logic)
{
    if(logic)
    {
        __inScr__set_flag(ModifyVars);
    }
    else
    {
        __inScr__remove_flag(ModifyVars);
    }
}

//Slot waits for the user reqest and sends a data
void ScripterCore_v2::__slot_ask_for_global_vars()
{
    QMap<QString,QVariant> globals;
    auto iter = globVars.begin();
    while(iter!=globVars.end())
    {
        globals.insert(iter.key(),__inScr__get_value(iter.value()));
    }
    emit __signal_global_vars(globals);
}

void ScripterCore_v2::__slot_break_script()
{
    __inScr__set_state(IsBreak);
    return_value=QString("!userBreak!");
    __inScr__script_end();
}

//Slot loads additional code without any else information (for testing)
void ScripterCore_v2::__slot_addit_code(const QStringList & data)
{
    QString msg("(__slot_addit_code): ");
    message(msg+"Started;");
    if(scrState!=IsFree)
    {
        message(msg+"ERROR! Core can't load additional script when script is running;",scr_ERROR);
        return;
    }
    __inScr__set_state(IsLoading);
    QLinkedList<QStringList*> app_data;
    int err = __inScr__code_splitter(data, app_data);
    message(msg+"Starting to load additional script;");
    if(err<0)
    {
        block_ptr add_block = __inScr__code_analyser(app_data);
        if(add_block)
        {
            runSeq.push(add_block);
            __inScr__set_state(IsReady);
            if(scrFlags&StepModeOn)
            {
               emit __signal_script_ready(add_block->curStrNum());
               return;
            }
            else
            {
               __inScr__script_start();
               return;
            }
        }
        else
        {
            message(msg+"ERROR! Core can't analyse user script;",scr_ERROR);
        }
    }
    else
    {
        message(msg+"ERROR! Core can't split user scipt into tokens;",scr_ERROR);
    }
    __inScr__script_end();
    message(msg+"Finished;");
}

void ScripterCore_v2::__slot_load_variables(const QMap<QString,QVariant> & data)
{
    QMap<QString,QVariant>::const_iterator iter = data.begin();
    while(iter!=data.end())
    {
        __slot_load_variable(iter.key(),iter.value());
        iter++;
    }
}

//This slot loads user variables to script
void ScripterCore_v2::__slot_load_variable(const QString & name, const QVariant & data)
{
    QString msg("(__slot_load_variable): ");
    message(msg+"Started;");
    message(msg+"Core tries to find a variable ["+name+"] in a global variables list;");
    var_map::iterator iter = globVars.find(name);
    if(iter!=globVars.end())
    {
        iter.value()->operator =(data);
        message(msg+"Global variable ["+iter.key()+"] found. New value is ["+data.toString()+"];",scr_INFO);
        message(msg+"Finished;");
        return;
    }
    else if(scrState==IsReady)
    {
        message(msg+"There is no such variable ["+name+"] in a global variables list;");
        while(true)
        {
            block_ptr cur_block = __inScr__script_get_actual_block();
            if(!cur_block)
            {
                message(msg+"Can't find a variable because the current block is missing;",scr_WARNING);
                break;
            }
            var_map curMap = cur_block->getLocalVariables();
            message(msg+"Core starts to search it in a local variables list;");
            iter = curMap.find(name);
            if(iter!=curMap.end())
            {
                iter.value()->operator =(data);
                message(msg+"Local variable ["+iter.key()+"] found. New value is ["+data.toString()+"];",scr_INFO);
                message(msg+"Finished;");
                return;
            }
            message(msg+"Variable ["+name+"] doesn't exists in the script. It will be created as a new variable;");
            break;
        }
    }
    var_ptr temp = std::make_shared<VarObject_v2>();
    temp->operator =(data);
    temp->setVision(Vision::Vglobal);
    globVars.insert(name,temp);
    message(msg+"Global variable ["+name+"] with value ["+data.toString()+"] created;",scr_INFO);
    message(msg+"Finished;");
}


void ScripterCore_v2::__slot_clear_loading_script(const QString & name)
{
    QString msg("(__slot_clear_loading_script): ");
    message(msg+"Started;");
    if(scrState!=IsFree)
    {
        __slot_break_script();
        message(msg+"Current script is breaking;",scr_WARNING);
    }
    if(name.isEmpty())
    {
        runSeq.clear();
        scrPool.clear();
        message(msg+"Recorded scripts erased;",scr_WARNING);
    }
    else
    {
        runSeq.clear();
        scrPool.remove(name);
        message(msg+"Recorded script ["+name+"] erased;",scr_WARNING);
    }
    message(msg+"Finished;");
}

//These slots read function interfaces and return it to user with a signal
void ScripterCore_v2::__slot_is_library_load(const QString & libname)
{
    emit __signal_is_library_load(functor->isLibLoad(libname));
    loadMessages(functor->readLog());
}

void ScripterCore_v2::__slot_get_function(const QString &libname, const QString & name)
{
    emit __signal_get_function(functor->getFunctionInterface(libname,name));
    loadMessages(functor->readLog());
}

void ScripterCore_v2::__slot_get_all_functions()
{
    emit __signal_get_all_functions(functor->getAllInterfaces());
    loadMessages(functor->readLog());
}

//This slot is for the custom function call
void ScripterCore_v2::__slot_call_function(const QString & libname, const QString & name, const QVector<QVariant> & data)
{
    QString msg("(__slot_call_function): ");
    message(msg+"Started;");
    QString params;
    foreach(QVariant var, data)
    {
        params.append(var.toString()+", ");
    }

    message(msg+"Function ["+name+"] with parameters ["+params+"] would be call from library ["+libname+"];");
    QVariant answer = functor->manualFuncCall(libname, name, data);
    loadMessages(functor->readLog());
    if(answer.isNull())
    {
        message(msg+"ERROR!["+functor->getError()+"]",scr_ERROR);
    }
    emit __signal_call_function(answer);
    message(msg+"Finished;");
    return;
}
